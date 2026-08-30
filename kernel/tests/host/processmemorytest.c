#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "process_memory.h"

#define TEST_PAGES 32U

static unsigned char backing[TEST_PAGES][PMM_PAGE_BYTES];
static int checks;
static int failures;

void zl_putc_pub(char c) { putchar(c); }

void *pmm_host_page_pointer(unsigned long long address)
{
    if (address < PMM_DYNAMIC_FLOOR) return NULL;
    unsigned long long page = (address - PMM_DYNAMIC_FLOOR) / PMM_PAGE_BYTES;
    return page < TEST_PAGES ? backing[page] : NULL;
}

static void expect(int condition, const char *message)
{
    checks++;
    if (!condition) {
        failures++;
        printf("FAIL: %s\n", message);
    }
}

static int setup_pages(unsigned int pages)
{
    struct pmm_range range = {
        PMM_DYNAMIC_FLOOR, (pmm_u64)pages * PMM_PAGE_BYTES, PMM_RANGE_USABLE
    };
    memset(backing, 0xa5, sizeof backing);
    return pmm_init_ranges(&range, 1);
}

static void test_single_lifecycle(void)
{
    struct process_memory memory = {0};
    expect(process_memory_check(&memory) == PROCESS_MEMORY_OK,
           "zero state is valid and empty");
    expect(!process_memory_ready(&memory), "zero state is not ready");
    expect(process_memory_release(&memory) == PROCESS_MEMORY_E_STATE,
           "empty release refused");
    expect(process_memory_page(&memory, PROCESS_MEMORY_PML4) == 0,
           "empty page lookup refused");
    expect(setup_pages(PROCESS_MEMORY_PAGE_COUNT) == PMM_OK,
           "single-process pool initialized");
    expect(process_memory_acquire(&memory, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_OK, "eight process pages acquired");
    expect(process_memory_ready(&memory), "complete process memory is ready");
    expect(pmm_used_pages() == PROCESS_MEMORY_PAGE_COUNT && pmm_free_pages() == 0,
           "single process owns the complete pool");
    expect(process_memory_acquire(&memory, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_E_STATE, "live process cannot be reacquired");

    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++) {
        pmm_u64 page = process_memory_page(
            &memory, (enum process_memory_page)i);
        expect(page != 0, "role has a physical page");
        expect(pmm_page_owner(page) == PROCESS_MEMORY_OWNER_BASE,
               "role page carries process owner");
        for (unsigned int j = 0; j < i; j++)
            expect(page != memory.pages[j], "role pages are distinct");
    }
    expect(process_memory_page(&memory,
           (enum process_memory_page)PROCESS_MEMORY_PAGE_COUNT) == 0,
           "out-of-range role refused");

    pmm_u64 original[PROCESS_MEMORY_PAGE_COUNT];
    memcpy(original, memory.pages, sizeof original);
    expect(process_memory_release(&memory) == PROCESS_MEMORY_OK,
           "complete process release");
    expect(!process_memory_ready(&memory) &&
           process_memory_check(&memory) == PROCESS_MEMORY_OK,
           "released process returns to valid empty state");
    expect(pmm_used_pages() == 0 &&
           pmm_free_pages() == PROCESS_MEMORY_PAGE_COUNT,
           "release restores allocator baseline");
    expect(process_memory_release(&memory) == PROCESS_MEMORY_E_STATE,
           "double process release refused");

    expect(process_memory_acquire(&memory, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_OK, "replacement process acquired");
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++)
        expect(memory.pages[i] == original[i],
               "replacement deterministically reuses released role page");
    expect(process_memory_release(&memory) == PROCESS_MEMORY_OK,
           "replacement process released");
}

static void test_failure_atomic_acquire(void)
{
    struct process_memory memory = {0};
    struct pmm_range reserved = {
        PMM_DYNAMIC_FLOOR, PMM_PAGE_BYTES, PMM_RANGE_RESERVED
    };
    expect(pmm_init_ranges(&reserved, 1) == PMM_E_RANGE,
           "zero-page pool remains unavailable");
    expect(process_memory_acquire(&memory, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_E_NOMEM, "first-page allocation failure reported");
    expect(process_memory_check(&memory) == PROCESS_MEMORY_OK,
           "first-page failure leaves empty state");

    for (unsigned int pages = 1; pages < PROCESS_MEMORY_PAGE_COUNT; pages++) {
        memset(&memory, 0, sizeof memory);
        expect(setup_pages(pages) == PMM_OK, "short pool initialized");
        expect(process_memory_acquire(&memory, PROCESS_MEMORY_OWNER_BASE) ==
               PROCESS_MEMORY_E_NOMEM, "nth-page allocation failure reported");
        expect(process_memory_check(&memory) == PROCESS_MEMORY_OK &&
               !process_memory_ready(&memory),
               "nth-page failure restores empty process state");
        expect(pmm_used_pages() == 0 && pmm_free_pages() == pages,
               "nth-page failure restores exact allocator baseline");
        expect(pmm_check() == 0, "nth-page rollback preserves PMM invariants");
    }
}

static void test_two_processes_and_foreign_owner(void)
{
    struct process_memory first = {0};
    struct process_memory second = {0};
    expect(setup_pages(2 * PROCESS_MEMORY_PAGE_COUNT) == PMM_OK,
           "two-process pool initialized");
    expect(process_memory_acquire(&first, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_OK, "first process acquired");
    expect(process_memory_acquire(&second, PROCESS_MEMORY_OWNER_BASE + 1) ==
           PROCESS_MEMORY_OK, "second process acquired");
    expect(pmm_used_pages() == 2 * PROCESS_MEMORY_PAGE_COUNT,
           "two process allocations are accounted");
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++)
        for (unsigned int j = 0; j < PROCESS_MEMORY_PAGE_COUNT; j++)
            expect(first.pages[i] != second.pages[j],
                   "processes never share an owned role page");

    pmm_u64 stolen = first.pages[3];
    expect(pmm_release(stolen, PROCESS_MEMORY_OWNER_BASE) == PMM_OK,
           "test releases one role page");
    expect(pmm_alloc(PROCESS_MEMORY_OWNER_BASE + 2) == stolen,
           "foreign owner takes released role page");
    unsigned long before = pmm_used_pages();
    expect(process_memory_release(&first) == PROCESS_MEMORY_E_CORRUPT,
           "foreign-owned role blocks whole release");
    expect(pmm_used_pages() == before && first.acquired == PROCESS_MEMORY_PAGE_COUNT,
           "foreign-owner preflight prevents partial release");
    expect(pmm_release(stolen, PROCESS_MEMORY_OWNER_BASE + 2) == PMM_OK,
           "foreign test owner released");
    expect(pmm_alloc(PROCESS_MEMORY_OWNER_BASE) == stolen,
           "original process owner restored");
    expect(process_memory_release(&first) == PROCESS_MEMORY_OK,
           "first process releases after ownership repair");
    expect(pmm_used_pages() == PROCESS_MEMORY_PAGE_COUNT &&
           process_memory_ready(&second),
           "first teardown leaves second process intact");
    expect(process_memory_release(&second) == PROCESS_MEMORY_OK,
           "second process released");
    expect(pmm_used_pages() == 0 &&
           pmm_free_pages() == 2 * PROCESS_MEMORY_PAGE_COUNT,
           "two-process teardown restores baseline");
}

static void test_invalid_states(void)
{
    struct process_memory memory = {0};
    expect(setup_pages(PROCESS_MEMORY_PAGE_COUNT) == PMM_OK,
           "invalid-state pool initialized");
    expect(process_memory_acquire(NULL, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_E_ARGUMENT, "null acquire refused");
    expect(process_memory_acquire(&memory, PMM_OWNER_FREE) ==
           PROCESS_MEMORY_E_OWNER, "internal PMM owner refused");
    memory.pages[0] = PMM_DYNAMIC_FLOOR;
    expect(process_memory_check(&memory) == PROCESS_MEMORY_E_CORRUPT,
           "hidden page in empty state detected");
    expect(process_memory_acquire(&memory, PROCESS_MEMORY_OWNER_BASE) ==
           PROCESS_MEMORY_E_STATE, "dirty empty state cannot acquire");
    memory.pages[0] = 0;
    expect(process_memory_page(NULL, PROCESS_MEMORY_PML4) == 0,
           "null page lookup refused");
    expect(process_memory_release(NULL) == PROCESS_MEMORY_E_ARGUMENT,
           "null release refused");
}

int main(void)
{
    test_single_lifecycle();
    test_failure_atomic_acquire();
    test_two_processes_and_foreign_owner();
    test_invalid_states();
    printf("processmemorytest: %d checks, %d failures\n", checks, failures);
    if (!failures)
        puts("process frames acquire atomically, remain disjoint, and reclaim exactly");
    return failures != 0;
}
