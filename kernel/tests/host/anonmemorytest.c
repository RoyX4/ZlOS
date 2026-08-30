#include <stdio.h>
#include <string.h>

#include "anon_memory.h"

#define TEST_PAGES 96U
#define TEST_FLAGS 0x8000000000000007ULL

static unsigned char backing[TEST_PAGES][PMM_PAGE_BYTES];
static int checks;
static int failures;

struct flush_control {
    unsigned int calls;
    unsigned int fail_call;
};

void zl_putc_pub(char c) { putchar(c); }

void *pmm_host_page_pointer(unsigned long long address)
{
    if (address < PMM_DYNAMIC_FLOOR) return NULL;
    unsigned long long page = (address - PMM_DYNAMIC_FLOOR) / PMM_PAGE_BYTES;
    return page < TEST_PAGES ? backing[page] : NULL;
}

static unsigned char *page_bytes(pmm_u64 address)
{
    return pmm_host_page_pointer(address);
}

static void expect(int condition, const char *message)
{
    checks++;
    if (!condition) {
        failures++;
        printf("FAIL: %s\n", message);
    }
}

static int flush(void *context)
{
    struct flush_control *control = context;
    control->calls++;
    return !control->fail_call || control->calls != control->fail_call;
}

static int setup_pages(unsigned int pages)
{
    struct pmm_range range = {
        PMM_DYNAMIC_FLOOR, (pmm_u64)pages * PMM_PAGE_BYTES, PMM_RANGE_USABLE
    };
    memset(backing, 0xa5, sizeof backing);
    return pmm_init_ranges(&range, 1);
}

static int page_is_zero(pmm_u64 page)
{
    unsigned char *bytes = page_bytes(page);
    if (!bytes) return 0;
    for (unsigned int i = 0; i < PMM_PAGE_BYTES; i++)
        if (bytes[i]) return 0;
    return 1;
}

static void test_reservation_and_collision(void)
{
    struct anon_memory memory = {0};
    struct flush_control control = {0};
    vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};

    expect(setup_pages(8) == PMM_OK, "reservation pool initialized");
    expect(anon_memory_init(NULL, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_E_ARGUMENT,
           "null initialization refused");
    expect(anon_memory_init(&memory, PMM_OWNER_FREE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_E_OWNER,
           "internal allocator owner refused");
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries,
                            TEST_FLAGS & ~1ULL, flush, &control) ==
           ANON_MEMORY_E_ARGUMENT, "non-present PTE flags refused");
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries,
                            TEST_FLAGS | 0x1000ULL, flush, &control) ==
           ANON_MEMORY_E_ARGUMENT, "physical address bits in flags refused");
    entries[31] = TEST_FLAGS;
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_E_COLLISION,
           "initial virtual collision refused");
    entries[31] = 0;
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_OK,
           "clean anonymous window initialized");
    expect(anon_memory_ready(&memory), "initialized window is ready");
    expect(anon_memory_reserve(&memory, 0, 0) == ANON_MEMORY_E_LIMIT,
           "zero-page reservation refused");
    expect(anon_memory_reserve(&memory, ANON_MEMORY_PAGE_COUNT, 1) ==
           ANON_MEMORY_E_LIMIT, "reservation past limit refused");
    expect(anon_memory_reserve(&memory, ANON_MEMORY_PAGE_COUNT - 1, 2) ==
           ANON_MEMORY_E_LIMIT, "wrapping reservation refused");

    entries[3] = TEST_FLAGS;
    expect(anon_memory_reserve(&memory, 3, 1) == ANON_MEMORY_E_COLLISION,
           "occupied PTE collision refused");
    expect(memory.reserved_count == 0 && pmm_used_pages() == 0,
           "collision leaves reservation and PMM unchanged");
    expect(anon_memory_check(&memory) == ANON_MEMORY_E_CORRUPT,
           "global check exposes hidden free-slot mapping");
    entries[3] = 0;

    expect(anon_memory_reserve(&memory, 4, 5) == ANON_MEMORY_OK,
           "five virtual pages reserved");
    expect(memory.reserved_count == 5 && memory.committed_count == 0 &&
           pmm_used_pages() == 0, "reservation consumes no physical memory");
    for (unsigned int i = 4; i < 9; i++) {
        expect(anon_memory_state(&memory, i) == ANON_MEMORY_RESERVED,
               "reserved state is distinct");
        expect(entries[i] == 0 && anon_memory_page(&memory, i) == 0,
               "reserved page has no PTE or frame");
    }
    expect(anon_memory_reserve(&memory, 6, 1) == ANON_MEMORY_E_STATE,
           "overlapping reservation refused");
    expect(anon_memory_release(&memory, 4, 5) == ANON_MEMORY_OK,
           "uncommitted reservation released");
    expect(memory.reserved_count == 0 && anon_memory_check(&memory) ==
           ANON_MEMORY_OK, "uncommitted release restores empty window");
    expect(anon_memory_destroy(&memory) == ANON_MEMORY_OK,
           "empty anonymous window destroyed");
    expect(!anon_memory_ready(&memory), "destroy clears initialization");
}

static void test_commit_zero_reclaim(void)
{
    struct anon_memory memory = {0};
    struct flush_control control = {0};
    vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};
    pmm_u64 original[4];

    expect(setup_pages(8) == PMM_OK, "commit pool initialized");
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_OK,
           "commit window initialized");
    expect(anon_memory_reserve(&memory, 2, 4) == ANON_MEMORY_OK,
           "commit range reserved");
    expect(anon_memory_commit(&memory, 2, 4) == ANON_MEMORY_OK,
           "four-page range committed");
    expect(control.calls == 1, "commit flushes once");
    expect(memory.reserved_count == 4 && memory.committed_count == 4 &&
           pmm_used_pages() == 4, "commit accounting exact");
    for (unsigned int i = 0; i < 4; i++) {
        unsigned int slot = 2 + i;
        original[i] = anon_memory_page(&memory, slot);
        expect(original[i] != 0, "committed slot has frame");
        expect(pmm_page_owner(original[i]) == ANON_MEMORY_OWNER_BASE,
               "committed frame has anonymous owner");
        expect(entries[slot] == (original[i] | TEST_FLAGS),
               "committed PTE binds exact frame and permissions");
        expect(page_is_zero(original[i]), "new anonymous frame is zero-filled");
        memset(page_bytes(original[i]), 0x5a, PMM_PAGE_BYTES);
    }
    entries[2] |= ANON_MEMORY_PTE_ACCESSED;
    entries[3] |= ANON_MEMORY_PTE_ACCESSED | ANON_MEMORY_PTE_DIRTY;
    expect(anon_memory_check(&memory) == ANON_MEMORY_OK,
           "hardware-managed accessed and dirty bits remain valid");
    expect(memory.committed_count == 4 && pmm_used_pages() == 4,
           "hardware-managed PTE bits do not change ownership accounting");
    expect(anon_memory_release(&memory, 2, 4) == ANON_MEMORY_OK,
           "committed range released");
    expect(control.calls == 2, "release flushes once");
    expect(memory.reserved_count == 0 && memory.committed_count == 0 &&
           pmm_used_pages() == 0 && pmm_free_pages() == 8,
           "release restores exact allocator baseline");
    for (unsigned int i = 2; i < 6; i++)
        expect(entries[i] == 0 && anon_memory_state(&memory, i) == ANON_MEMORY_FREE,
               "release removes PTE and reservation");

    expect(anon_memory_reserve(&memory, 2, 4) == ANON_MEMORY_OK,
           "replacement range reserved");
    expect(anon_memory_commit(&memory, 2, 4) == ANON_MEMORY_OK,
           "replacement range committed");
    for (unsigned int i = 0; i < 4; i++) {
        expect(anon_memory_page(&memory, 2 + i) == original[i],
               "replacement deterministically reuses frame");
        expect(page_is_zero(original[i]), "reused anonymous frame is re-zeroed");
    }
    expect(anon_memory_destroy(&memory) == ANON_MEMORY_OK,
           "live anonymous window destroyed");
    expect(pmm_used_pages() == 0 && pmm_free_pages() == 8,
           "destroy restores allocator baseline");
}

static void test_oom_atomicity(void)
{
    for (unsigned int pages = 1; pages < 6; pages++) {
        struct anon_memory memory = {0};
        struct flush_control control = {0};
        vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};
        expect(setup_pages(pages) == PMM_OK, "short anonymous pool initialized");
        expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries,
                                TEST_FLAGS, flush, &control) == ANON_MEMORY_OK,
               "short-pool window initialized");
        expect(anon_memory_reserve(&memory, 0, 6) == ANON_MEMORY_OK,
               "short-pool range reserved");
        expect(anon_memory_commit(&memory, 0, 6) == ANON_MEMORY_E_NOMEM,
               "nth-frame OOM reported");
        expect(memory.reserved_count == 6 && memory.committed_count == 0 &&
               pmm_used_pages() == 0 && pmm_free_pages() == pages,
               "OOM restores exact PMM baseline and reservation");
        for (unsigned int i = 0; i < 6; i++)
            expect(memory.states[i] == ANON_MEMORY_RESERVED && !memory.pages[i] &&
                   !entries[i], "OOM leaves every slot reserved but uncommitted");
        expect(anon_memory_release(&memory, 0, 6) == ANON_MEMORY_OK,
               "reservation remains releasable after OOM");
        expect(anon_memory_destroy(&memory) == ANON_MEMORY_OK,
               "short-pool window destroyed");
    }
}

static void test_transaction_failures(void)
{
    for (unsigned int write = 1; write <= 4; write++) {
        struct anon_memory memory = {0};
        struct flush_control control = {0};
        vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};
        expect(setup_pages(8) == PMM_OK, "write-failure pool initialized");
        expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries,
                                TEST_FLAGS, flush, &control) == ANON_MEMORY_OK,
               "write-failure window initialized");
        expect(anon_memory_reserve(&memory, 0, 4) == ANON_MEMORY_OK,
               "write-failure range reserved");
        anon_memory_fail_after_write(&memory, write);
        expect(anon_memory_commit(&memory, 0, 4) == ANON_MEMORY_E_TRANSACTION,
               "nth PTE write failure reported");
        expect(memory.committed_count == 0 && pmm_used_pages() == 0 &&
               pmm_free_pages() == 8, "nth write rolls PTEs and frames back");
        for (unsigned int i = 0; i < 4; i++)
            expect(!entries[i] && memory.states[i] == ANON_MEMORY_RESERVED,
                   "nth write preserves reservation only");
        expect(anon_memory_destroy(&memory) == ANON_MEMORY_OK,
               "write-failure window destroyed");
    }

    struct anon_memory memory = {0};
    struct flush_control control = {0, 1};
    vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};
    expect(setup_pages(8) == PMM_OK, "flush-failure pool initialized");
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_OK,
           "flush-failure window initialized");
    expect(anon_memory_reserve(&memory, 0, 3) == ANON_MEMORY_OK,
           "flush-failure range reserved");
    expect(anon_memory_commit(&memory, 0, 3) == ANON_MEMORY_E_TRANSACTION,
           "commit flush failure reported");
    expect(control.calls == 2 && pmm_used_pages() == 0,
           "failed flush performs successful rollback flush and frees frames");
    for (unsigned int i = 0; i < 3; i++)
        expect(!entries[i] && memory.states[i] == ANON_MEMORY_RESERVED,
               "flush failure restores every PTE");

    control.calls = control.fail_call = 0;
    expect(anon_memory_commit(&memory, 0, 3) == ANON_MEMORY_OK,
           "commit recovers after flush failure");
    pmm_u64 pages[3] = {memory.pages[0], memory.pages[1], memory.pages[2]};
    anon_memory_fail_after_write(&memory, 2);
    expect(anon_memory_release(&memory, 0, 3) == ANON_MEMORY_E_TRANSACTION,
           "release PTE failure reported");
    expect(memory.committed_count == 3 && pmm_used_pages() == 3,
           "failed release retains all frames");
    for (unsigned int i = 0; i < 3; i++)
        expect(entries[i] == (pages[i] | TEST_FLAGS) &&
               memory.states[i] == ANON_MEMORY_COMMITTED,
               "failed release restores all mappings");
    expect(anon_memory_release(&memory, 0, 3) == ANON_MEMORY_OK,
           "release recovers after injected failure");
    expect(anon_memory_destroy(&memory) == ANON_MEMORY_OK,
           "transaction-failure window destroyed");
}

static void test_fail_closed_rollback(void)
{
    struct anon_memory memory = {0};
    struct flush_control control = {0, 1};
    vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};
    expect(setup_pages(4) == PMM_OK, "rollback-failure pool initialized");
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_OK,
           "rollback-failure window initialized");
    expect(anon_memory_reserve(&memory, 0, 2) == ANON_MEMORY_OK,
           "rollback-failure range reserved");
    anon_memory_fail_after_write(&memory, 1);
    expect(anon_memory_commit(&memory, 0, 2) == ANON_MEMORY_E_CORRUPT,
           "failed rollback is a hard corruption result");
    expect(memory.broken_count == 2 && memory.committed_count == 0 &&
           pmm_used_pages() == 2, "rollback failure retains owned frames");
    for (unsigned int i = 0; i < 2; i++)
        expect(memory.states[i] == ANON_MEMORY_BROKEN && memory.pages[i] &&
               pmm_page_owner(memory.pages[i]) == ANON_MEMORY_OWNER_BASE,
               "uncertain mapping frame is never freed");
    expect(anon_memory_destroy(&memory) == ANON_MEMORY_E_CORRUPT,
           "broken mapping cannot be silently destroyed");
}

static void test_foreign_owner_and_two_processes(void)
{
    struct anon_memory first = {0}, second = {0};
    struct flush_control one = {0}, two = {0};
    vmm_pt_word first_entries[ANON_MEMORY_PAGE_COUNT] = {0};
    vmm_pt_word second_entries[ANON_MEMORY_PAGE_COUNT] = {0};
    expect(setup_pages(12) == PMM_OK, "two-process anonymous pool initialized");
    expect(anon_memory_init(&first, ANON_MEMORY_OWNER_BASE, first_entries,
                            TEST_FLAGS, flush, &one) == ANON_MEMORY_OK,
           "first anonymous process initialized");
    expect(anon_memory_init(&second, ANON_MEMORY_OWNER_BASE + 1, second_entries,
                            TEST_FLAGS, flush, &two) == ANON_MEMORY_OK,
           "second anonymous process initialized");
    expect(anon_memory_reserve(&first, 0, 4) == ANON_MEMORY_OK &&
           anon_memory_commit(&first, 0, 4) == ANON_MEMORY_OK,
           "first anonymous process committed");
    expect(anon_memory_reserve(&second, 0, 4) == ANON_MEMORY_OK &&
           anon_memory_commit(&second, 0, 4) == ANON_MEMORY_OK,
           "second anonymous process committed");
    for (unsigned int i = 0; i < 4; i++)
        for (unsigned int j = 0; j < 4; j++)
            expect(first.pages[i] != second.pages[j],
                   "anonymous process frames remain disjoint");

    pmm_u64 stolen = first.pages[2];
    expect(pmm_release(stolen, ANON_MEMORY_OWNER_BASE) == PMM_OK,
           "test releases one anonymous frame");
    expect(pmm_alloc(ANON_MEMORY_OWNER_BASE + 2) == stolen,
           "foreign anonymous owner takes frame");
    vmm_pt_word before[4];
    memcpy(before, first_entries, sizeof before);
    expect(anon_memory_release(&first, 0, 4) == ANON_MEMORY_E_CORRUPT,
           "foreign owner blocks whole release");
    expect(!memcmp(before, first_entries, sizeof before) &&
           first.committed_count == 4, "foreign-owner preflight prevents PTE mutation");
    expect(pmm_release(stolen, ANON_MEMORY_OWNER_BASE + 2) == PMM_OK,
           "foreign owner released");
    expect(pmm_alloc(ANON_MEMORY_OWNER_BASE) == stolen,
           "original anonymous owner repaired");
    expect(anon_memory_destroy(&first) == ANON_MEMORY_OK,
           "first process reclaims after ownership repair");
    expect(anon_memory_destroy(&second) == ANON_MEMORY_OK,
           "second process reclaims independently");
    expect(pmm_used_pages() == 0 && pmm_free_pages() == 12,
           "two-process reclaim restores PMM baseline");
}

static void test_mixed_release_and_holey_destroy(void)
{
    struct anon_memory memory = {0};
    struct flush_control control = {0};
    vmm_pt_word entries[ANON_MEMORY_PAGE_COUNT] = {0};
    expect(setup_pages(12) == PMM_OK, "holey-destroy pool initialized");
    expect(anon_memory_init(&memory, ANON_MEMORY_OWNER_BASE, entries, TEST_FLAGS,
                            flush, &control) == ANON_MEMORY_OK,
           "holey-destroy window initialized");
    expect(anon_memory_reserve(&memory, 0, 2) == ANON_MEMORY_OK &&
           anon_memory_commit(&memory, 0, 2) == ANON_MEMORY_OK,
           "first committed island created");
    expect(anon_memory_reserve(&memory, 4, 2) == ANON_MEMORY_OK,
           "reserved island created");
    expect(anon_memory_reserve(&memory, 8, 3) == ANON_MEMORY_OK &&
           anon_memory_commit(&memory, 8, 3) == ANON_MEMORY_OK,
           "second committed island created");
    expect(anon_memory_reserve(&memory, 12, 4) == ANON_MEMORY_OK &&
           anon_memory_commit(&memory, 12, 2) == ANON_MEMORY_OK,
           "mixed contiguous range created");
    expect(anon_memory_release(&memory, 12, 4) == ANON_MEMORY_OK,
           "mixed reserved and committed range released atomically");
    expect(memory.reserved_count == 7 && memory.committed_count == 5,
           "mixed release accounting exact");
    expect(anon_memory_destroy(&memory) == ANON_MEMORY_OK,
           "holey anonymous window destroyed by occupied islands");
    expect(pmm_used_pages() == 0 && pmm_free_pages() == 12,
           "holey destroy restores allocator baseline");
}

int main(void)
{
    test_reservation_and_collision();
    test_commit_zero_reclaim();
    test_oom_atomicity();
    test_transaction_failures();
    test_fail_closed_rollback();
    test_foreign_owner_and_two_processes();
    test_mixed_release_and_holey_destroy();
    printf("anonmemorytest: %d checks, %d failures\n", checks, failures);
    if (!failures)
        puts("anonymous pages reserve, commit, zero, roll back and reclaim exactly");
    return failures != 0;
}
