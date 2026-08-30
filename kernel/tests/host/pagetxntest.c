#include <stdio.h>
#include <string.h>

#include "page_table_txn.h"

#define ENTRY_COUNT 512u

static int checks;
static int failures;

struct flush_probe {
    int calls;
    int fail_call;
    int corrupt_call;
    volatile vmm_pt_word *corrupt_entry;
};

static void ok(int condition, const char *message)
{
    checks++;
    if (!condition) {
        failures++;
        printf("  FAIL  %s\n", message);
    }
}

static int flush_probe(void *opaque)
{
    struct flush_probe *probe = opaque;
    probe->calls++;
    if (probe->calls == probe->corrupt_call && probe->corrupt_entry)
        *probe->corrupt_entry ^= 0x1000ULL;
    return probe->calls != probe->fail_call;
}

static void seed(vmm_pt_word *entries, vmm_pt_word *before)
{
    for (unsigned i = 0; i < ENTRY_COUNT; i++)
        entries[i] = before[i] = 0x100000ULL + (vmm_pt_word)i * 0x1000ULL;
}

static int reserve_full(struct vmm_pt_transaction *transaction,
                        struct vmm_pt_change *changes,
                        vmm_pt_word *entries, struct flush_probe *probe)
{
    int status = vmm_pt_txn_begin(
        transaction, changes, ENTRY_COUNT, flush_probe, probe
    );
    if (status != VMM_PT_TXN_OK) return status;
    for (unsigned i = 0; i < ENTRY_COUNT; i++) {
        status = vmm_pt_txn_reserve(
            transaction, &entries[i], 0x8000000000000003ULL | ((vmm_pt_word)i << 12)
        );
        if (status != VMM_PT_TXN_OK) return status;
    }
    return VMM_PT_TXN_OK;
}

int main(void)
{
    vmm_pt_word entries[ENTRY_COUNT];
    vmm_pt_word before[ENTRY_COUNT];
    struct vmm_pt_change changes[ENTRY_COUNT];
    struct vmm_pt_transaction transaction;
    struct flush_probe probe = {0};

    puts("pagetxntest - reserve/validate/apply/flush/commit or exact rollback\n");

    seed(entries, before);
    ok(reserve_full(&transaction, changes, entries, &probe) == VMM_PT_TXN_OK,
       "reserve a complete 512-entry range");
    ok(memcmp(entries, before, sizeof(entries)) == 0,
       "reservation does not mutate the table");
    ok(vmm_pt_txn_apply(&transaction) == VMM_PT_TXN_OK,
       "apply and flush the complete range");
    ok(probe.calls == 1 && transaction.state == VMM_PT_TXN_APPLIED,
       "successful apply flushes once and remains reversible");
    ok(vmm_pt_txn_commit(&transaction) == VMM_PT_TXN_OK
       && transaction.state == VMM_PT_TXN_COMMITTED,
       "commit makes the applied range final");
    ok(vmm_pt_txn_rollback(&transaction) == VMM_PT_TXN_ERR_STATE,
       "a committed transaction cannot be rolled back");

    seed(entries, before);
    probe = (struct flush_probe){0};
    ok(vmm_pt_txn_begin(&transaction, changes, 1, flush_probe, &probe)
       == VMM_PT_TXN_OK, "begin a bounded transaction");
    ok(vmm_pt_txn_reserve(&transaction, &entries[0], 3) == VMM_PT_TXN_OK,
       "reserve within capacity");
    ok(vmm_pt_txn_reserve(&transaction, &entries[1], 3)
       == VMM_PT_TXN_ERR_CAPACITY, "refuse a range beyond journal capacity");
    ok(vmm_pt_txn_begin(&transaction, changes, ENTRY_COUNT, flush_probe, &probe)
       == VMM_PT_TXN_OK, "restart after an un-applied reservation");
    ok(vmm_pt_txn_reserve(&transaction, &entries[7], 3) == VMM_PT_TXN_OK,
       "reserve one entry");
    ok(vmm_pt_txn_reserve(&transaction, &entries[7], 5)
       == VMM_PT_TXN_ERR_DUPLICATE, "refuse a duplicate entry reservation");

    seed(entries, before);
    probe = (struct flush_probe){0};
    ok(reserve_full(&transaction, changes, entries, &probe) == VMM_PT_TXN_OK,
       "reserve for stale-entry test");
    entries[200] ^= 0x2000ULL;
    ok(vmm_pt_txn_apply(&transaction) == VMM_PT_TXN_ERR_STALE,
       "validate the entire range before the first write");
    ok(memcmp(entries, before, 200 * sizeof(entries[0])) == 0
       && memcmp(entries + 201, before + 201,
                 (ENTRY_COUNT - 201) * sizeof(entries[0])) == 0,
       "stale validation leaves every other entry untouched");
    entries[200] = before[200];

    for (unsigned fail_after = 1; fail_after <= ENTRY_COUNT; fail_after++) {
        seed(entries, before);
        probe = (struct flush_probe){0};
        if (reserve_full(&transaction, changes, entries, &probe) != VMM_PT_TXN_OK) {
            ok(0, "reserve every injected-failure transaction");
            break;
        }
        vmm_pt_txn_fail_after_write(&transaction, fail_after);
        if (vmm_pt_txn_apply(&transaction) != VMM_PT_TXN_ERR_INJECTED
                || memcmp(entries, before, sizeof(entries)) != 0
                || transaction.state != VMM_PT_TXN_ROLLED_BACK) {
            ok(0, "every injected write failure restores the exact table");
            break;
        }
    }
    ok(1, "all 512 nth-write failures were exercised");

    seed(entries, before);
    probe = (struct flush_probe){.fail_call = 1};
    ok(reserve_full(&transaction, changes, entries, &probe) == VMM_PT_TXN_OK,
       "reserve for flush-failure test");
    ok(vmm_pt_txn_apply(&transaction) == VMM_PT_TXN_ERR_FLUSH
       && probe.calls == 2
       && memcmp(entries, before, sizeof(entries)) == 0,
       "a failed apply flush restores and re-flushes the exact table");

    seed(entries, before);
    probe = (struct flush_probe){
        .corrupt_call = 1,
        .corrupt_entry = &entries[317],
    };
    ok(reserve_full(&transaction, changes, entries, &probe) == VMM_PT_TXN_OK,
       "reserve for readback-corruption test");
    ok(vmm_pt_txn_apply(&transaction) == VMM_PT_TXN_ERR_VERIFY
       && probe.calls == 2
       && memcmp(entries, before, sizeof(entries)) == 0,
       "post-flush corruption is detected and exactly rolled back");

    seed(entries, before);
    probe = (struct flush_probe){0};
    ok(reserve_full(&transaction, changes, entries, &probe) == VMM_PT_TXN_OK,
       "reserve for explicit rollback test");
    ok(vmm_pt_txn_apply(&transaction) == VMM_PT_TXN_OK,
       "apply before caller validation");
    ok(vmm_pt_txn_rollback(&transaction) == VMM_PT_TXN_OK
       && probe.calls == 2
       && memcmp(entries, before, sizeof(entries)) == 0,
       "caller rejection restores and flushes the complete range");

    seed(entries, before);
    probe = (struct flush_probe){.fail_call = 2};
    ok(reserve_full(&transaction, changes, entries, &probe) == VMM_PT_TXN_OK,
       "reserve for rollback-flush failure test");
    ok(vmm_pt_txn_apply(&transaction) == VMM_PT_TXN_OK,
       "apply before rollback-flush failure");
    ok(vmm_pt_txn_rollback(&transaction) == VMM_PT_TXN_ERR_ROLLBACK
       && transaction.state == VMM_PT_TXN_FAILED
       && memcmp(entries, before, sizeof(entries)) == 0,
       "a failed rollback flush is reported instead of claiming exact rollback");

    printf("\n%d checks, %d failures\n", checks, failures);
    if (failures) return 1;
    puts("ok    full-range page-table transactions are exact under every injected failure");
    return 0;
}
