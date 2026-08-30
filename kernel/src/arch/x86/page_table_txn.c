#include "page_table_txn.h"

static int restore_applied(struct vmm_pt_transaction *transaction)
{
    unsigned i;

    for (i = transaction->applied; i > 0; i--)
        *transaction->changes[i - 1].entry = transaction->changes[i - 1].before;

    if (!transaction->flush(transaction->flush_context)) {
        transaction->state = VMM_PT_TXN_FAILED;
        return VMM_PT_TXN_ERR_ROLLBACK;
    }
    for (i = 0; i < transaction->applied; i++) {
        if (*transaction->changes[i].entry != transaction->changes[i].before) {
            transaction->state = VMM_PT_TXN_FAILED;
            return VMM_PT_TXN_ERR_ROLLBACK;
        }
    }
    transaction->state = VMM_PT_TXN_ROLLED_BACK;
    return VMM_PT_TXN_OK;
}

static int fail_after_restore(struct vmm_pt_transaction *transaction,
                              int original_status)
{
    if (restore_applied(transaction) != VMM_PT_TXN_OK)
        return VMM_PT_TXN_ERR_ROLLBACK;
    return original_status;
}

int vmm_pt_txn_begin(struct vmm_pt_transaction *transaction,
                     struct vmm_pt_change *changes, unsigned capacity,
                     vmm_pt_flush_fn flush, void *flush_context)
{
    if (!transaction || !changes || !capacity || !flush)
        return VMM_PT_TXN_ERR_ARGUMENT;
    transaction->changes = changes;
    transaction->capacity = capacity;
    transaction->count = 0;
    transaction->applied = 0;
    transaction->fail_after_write = 0;
    transaction->state = VMM_PT_TXN_RESERVING;
    transaction->flush = flush;
    transaction->flush_context = flush_context;
    return VMM_PT_TXN_OK;
}

int vmm_pt_txn_reserve(struct vmm_pt_transaction *transaction,
                       volatile vmm_pt_word *entry, vmm_pt_word after)
{
    unsigned i;
    struct vmm_pt_change *change;

    if (!transaction || transaction->state != VMM_PT_TXN_RESERVING || !entry)
        return VMM_PT_TXN_ERR_STATE;
    if (transaction->count == transaction->capacity)
        return VMM_PT_TXN_ERR_CAPACITY;
    for (i = 0; i < transaction->count; i++) {
        if (transaction->changes[i].entry == entry)
            return VMM_PT_TXN_ERR_DUPLICATE;
    }
    change = &transaction->changes[transaction->count++];
    change->entry = entry;
    change->before = *entry;
    change->after = after;
    return VMM_PT_TXN_OK;
}

void vmm_pt_txn_fail_after_write(struct vmm_pt_transaction *transaction,
                                 unsigned writes)
{
    if (transaction && transaction->state == VMM_PT_TXN_RESERVING)
        transaction->fail_after_write = writes;
}

int vmm_pt_txn_apply(struct vmm_pt_transaction *transaction)
{
    unsigned i;

    if (!transaction || transaction->state != VMM_PT_TXN_RESERVING
            || !transaction->count)
        return VMM_PT_TXN_ERR_STATE;

    for (i = 0; i < transaction->count; i++) {
        if (*transaction->changes[i].entry != transaction->changes[i].before)
            return VMM_PT_TXN_ERR_STALE;
    }

    transaction->applied = 0;
    for (i = 0; i < transaction->count; i++) {
        *transaction->changes[i].entry = transaction->changes[i].after;
        transaction->applied++;
        if (*transaction->changes[i].entry != transaction->changes[i].after)
            return fail_after_restore(transaction, VMM_PT_TXN_ERR_VERIFY);
        if (transaction->fail_after_write == transaction->applied)
            return fail_after_restore(transaction, VMM_PT_TXN_ERR_INJECTED);
    }

    if (!transaction->flush(transaction->flush_context))
        return fail_after_restore(transaction, VMM_PT_TXN_ERR_FLUSH);
    for (i = 0; i < transaction->count; i++) {
        if (*transaction->changes[i].entry != transaction->changes[i].after)
            return fail_after_restore(transaction, VMM_PT_TXN_ERR_VERIFY);
    }
    transaction->state = VMM_PT_TXN_APPLIED;
    return VMM_PT_TXN_OK;
}

int vmm_pt_txn_commit(struct vmm_pt_transaction *transaction)
{
    if (!transaction || transaction->state != VMM_PT_TXN_APPLIED)
        return VMM_PT_TXN_ERR_STATE;
    transaction->state = VMM_PT_TXN_COMMITTED;
    return VMM_PT_TXN_OK;
}

int vmm_pt_txn_rollback(struct vmm_pt_transaction *transaction)
{
    if (!transaction || transaction->state != VMM_PT_TXN_APPLIED)
        return VMM_PT_TXN_ERR_STATE;
    return restore_applied(transaction);
}
