#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pmm.h"
#include "boot/boot_handover.h"

#define TEST_PAGES 16U

static unsigned char backing[TEST_PAGES][PMM_PAGE_BYTES];
static int failures;
static int checks;

void zl_putc_pub(char c) { putchar(c); }

void *pmm_host_page_pointer(unsigned long long address)
{
    if (address < PMM_DYNAMIC_FLOOR) return NULL;
    unsigned long long page = (address - PMM_DYNAMIC_FLOOR) / PMM_PAGE_BYTES;
    if (page >= TEST_PAGES) return NULL;
    return backing[page];
}

static void expect(int condition, const char *message)
{
    checks++;
    if (!condition) {
        failures++;
        printf("FAIL: %s\n", message);
    }
}

static unsigned char *page_bytes(unsigned long long address)
{
    return backing[(address - PMM_DYNAMIC_FLOOR) / PMM_PAGE_BYTES];
}

struct efi_descriptor_test {
    uint32_t type;
    uint32_t pad;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t pages;
    uint64_t attributes;
};

static void store_u32(unsigned char *p, uint32_t value)
{
    for (unsigned int i = 0; i < 4; i++) p[i] = (unsigned char)(value >> (i * 8));
}

static void store_u64(unsigned char *p, uint64_t value)
{
    store_u32(p, (uint32_t)value);
    store_u32(p + 4, (uint32_t)(value >> 32));
}

static void test_ranges(void)
{
    struct pmm_range ranges[] = {
        {PMM_DYNAMIC_FLOOR - 4 * PMM_PAGE_BYTES, 8 * PMM_PAGE_BYTES,
         PMM_RANGE_USABLE},
        {PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES + 1, PMM_PAGE_BYTES,
         PMM_RANGE_RESERVED},
        {PMM_DYNAMIC_FLOOR + 4 * PMM_PAGE_BYTES, 8 * PMM_PAGE_BYTES,
         PMM_RANGE_USABLE},
    };
    expect(pmm_init_ranges(ranges, 3) == PMM_OK, "range init");
    expect(pmm_ready(), "allocator ready");
    expect(pmm_total_pages() == 10, "usable count excludes floor and hole");
    expect(pmm_free_pages() == 10 && pmm_used_pages() == 0, "initial accounting");
    expect(pmm_page_owner(PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES) ==
           PMM_OWNER_RESERVED &&
           pmm_page_owner(PMM_DYNAMIC_FLOOR + 3 * PMM_PAGE_BYTES) ==
           PMM_OWNER_RESERVED, "partial reserved overlap owns every touched page");
    expect(pmm_release(PMM_DYNAMIC_FLOOR, 20) == PMM_E_DOUBLE_FREE,
           "never-allocated usable page is a double free");
    expect(pmm_release(PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES, 20) ==
           PMM_E_RESERVED, "reserved page cannot be freed");
    expect(pmm_release(PMM_DYNAMIC_FLOOR + 1, 20) == PMM_E_ALIGNMENT,
           "unaligned free refused");

    unsigned long long allocated[10];
    for (unsigned int i = 0; i < 10; i++) {
        allocated[i] = pmm_alloc(20);
        expect(allocated[i] != 0, "allocation before exhaustion");
        expect(allocated[i] != PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES &&
               allocated[i] != PMM_DYNAMIC_FLOOR + 3 * PMM_PAGE_BYTES,
               "allocation skipped reserved hole");
        unsigned char *bytes = page_bytes(allocated[i]);
        int zero = 1;
        for (unsigned int j = 0; j < PMM_PAGE_BYTES; j++) zero &= bytes[j] == 0;
        expect(zero, "fresh allocation zeroed");
        memset(bytes, 0xA5, PMM_PAGE_BYTES);
    }
    expect(pmm_alloc(20) == 0, "deterministic exhaustion");
    expect(pmm_free_pages() == 0 && pmm_used_pages() == 10, "exhausted accounting");
    expect(pmm_release(allocated[4], 21) == PMM_E_OWNER,
           "foreign owner cannot release");
    expect(pmm_page_owner(allocated[4]) == 20, "foreign release did not mutate");
    expect(pmm_release(allocated[4], 20) == PMM_OK, "owned release");
    expect(pmm_release(allocated[4], 20) == PMM_E_DOUBLE_FREE, "double free refused");
    unsigned long long reused = pmm_alloc(21);
    expect(reused == allocated[4], "released page reused deterministically");
    int zero = 1;
    for (unsigned int j = 0; j < PMM_PAGE_BYTES; j++)
        zero &= page_bytes(reused)[j] == 0;
    expect(zero, "reused page zeroed");
    expect(pmm_release(reused, 21) == PMM_OK, "reused page released by new owner");
    for (unsigned int i = 0; i < 10; i++)
        if (i != 4) expect(pmm_release(allocated[i], 20) == PMM_OK,
                           "remaining page release");
    expect(pmm_free_pages() == 10 && pmm_used_pages() == 0, "baseline restored");
    expect(pmm_high_water_pages() == 10, "high water measured");
    expect(pmm_refusals() == 1, "exhaustion counted once");
    expect(pmm_check() == 0, "allocator invariant walk");
    expect(pmm_alloc(PMM_OWNER_FREE) == 0, "internal owner refused");
}

static void test_owner_accounting_and_limits(void)
{
    struct pmm_range range = {
        PMM_DYNAMIC_FLOOR, 8 * PMM_PAGE_BYTES, PMM_RANGE_USABLE
    };
    struct pmm_owner_account owner20;
    struct pmm_owner_account owner21;
    expect(pmm_init_ranges(&range, 1) == PMM_OK, "accounting map init");
    expect(pmm_owner_account(20, &owner20) == PMM_OK,
           "empty owner account query");
    expect(owner20.owner == 20 && owner20.used_pages == 0 &&
           owner20.high_water_pages == 0 &&
           owner20.limit_pages == PMM_LIMIT_UNLIMITED &&
           owner20.available_pages == 8 && owner20.refusals == 0,
           "empty owner account is exact");
    expect(pmm_owner_account(20, NULL) == PMM_E_ARGUMENT,
           "null account output refused");
    expect(pmm_owner_account(PMM_OWNER_FREE, &owner20) == PMM_E_OWNER &&
           pmm_set_owner_limit(PMM_OWNER_RESERVED, 1) == PMM_E_OWNER,
           "internal owner accounting refused");
    expect(pmm_set_owner_limit(20, 2) == PMM_OK,
           "owner quota configured");

    unsigned long long first = pmm_alloc(20);
    unsigned long long second = pmm_alloc(20);
    unsigned long long foreign = pmm_alloc(21);
    expect(first && second && foreign, "allocations within owner quotas");
    expect(pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.used_pages == 2 && owner20.high_water_pages == 2 &&
           owner20.limit_pages == 2 && owner20.available_pages == 0 &&
           owner20.refusals == 0, "bounded owner account after allocation");
    expect(pmm_owner_account(21, &owner21) == PMM_OK &&
           owner21.used_pages == 1 && owner21.high_water_pages == 1 &&
           owner21.limit_pages == PMM_LIMIT_UNLIMITED &&
           owner21.available_pages == 5 && owner21.refusals == 0,
           "independent owner account after allocation");
    expect(pmm_alloc(20) == 0, "owner quota refuses one-over allocation");
    expect(pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.used_pages == 2 && owner20.high_water_pages == 2 &&
           owner20.limit_pages == 2 && owner20.available_pages == 0 &&
           owner20.refusals == 1, "quota refusal leaves accounting unchanged");
    expect(pmm_owner_account(21, &owner21) == PMM_OK &&
           owner21.used_pages == 1 && owner21.refusals == 0,
           "quota refusal cannot charge another owner");
    expect(pmm_set_owner_limit(20, 1) == PMM_E_LIMIT &&
           pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.limit_pages == 2 && owner20.used_pages == 2,
           "limit cannot shrink below live ownership");

    expect(pmm_release(first, 20) == PMM_OK, "quota owner releases one page");
    expect(pmm_set_owner_limit(20, 1) == PMM_OK &&
           pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.used_pages == 1 && owner20.high_water_pages == 2 &&
           owner20.limit_pages == 1 && owner20.available_pages == 0,
           "limit can shrink to exact live ownership");
    expect(pmm_alloc(20) == 0 &&
           pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.refusals == 2 && owner20.used_pages == 1,
           "second quota refusal is counted exactly");
    expect(pmm_set_owner_limit(20, 3) == PMM_OK,
           "owner quota can grow");
    unsigned long long reused = pmm_alloc(20);
    expect(reused == first, "quota release reuses deterministic page");
    expect(pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.used_pages == 2 && owner20.high_water_pages == 2 &&
           owner20.limit_pages == 3 && owner20.available_pages == 1 &&
           owner20.refusals == 2, "grown quota account is exact");
    expect(pmm_refusals() == 2, "global refusals include owner quota denials");
    expect(pmm_check() == 0, "owner accounting invariant walk");

    expect(pmm_release(reused, 20) == PMM_OK &&
           pmm_release(second, 20) == PMM_OK &&
           pmm_release(foreign, 21) == PMM_OK,
           "all accounted pages released");
    expect(pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.used_pages == 0 && owner20.high_water_pages == 2 &&
           owner20.limit_pages == 3 && owner20.available_pages == 3 &&
           owner20.refusals == 2, "released owner retains bounded history");
    expect(pmm_check() == 0, "released accounting invariant walk");

    expect(pmm_init_ranges(&range, 1) == PMM_OK,
           "allocator reinitialization resets accounting");
    expect(pmm_owner_account(20, &owner20) == PMM_OK &&
           owner20.used_pages == 0 && owner20.high_water_pages == 0 &&
           owner20.limit_pages == PMM_LIMIT_UNLIMITED &&
           owner20.available_pages == 8 && owner20.refusals == 0,
           "reinitialization clears owner quota history");
}

static void test_boot_map(void)
{
    struct efi_descriptor_test map[] = {
        {7, 0, PMM_DYNAMIC_FLOOR, 0, 8, 0},
        {0, 0, PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES, 0, 2, 0},
        {7, 0, PMM_DYNAMIC_FLOOR + 8 * PMM_PAGE_BYTES, 0, 4, 0},
    };
    expect(zlos_boot_begin(ZLOS_BOOT_ORIGIN_NATIVE_UEFI, 64,
                           ZLOS_BOOT_GENERATION_CURRENT, 1, 3) == ZLOS_BOOT_OK,
           "boot begin");
    expect(zlos_boot_set_source(0x100000) == ZLOS_BOOT_OK, "boot source");
    expect(zlos_boot_set_memory_map((uint64_t)(uintptr_t)map, sizeof(map),
                                    sizeof(map[0]), 1, 0) == ZLOS_BOOT_OK,
           "boot map set");
    expect(zlos_boot_mark_firmware_retired() == ZLOS_BOOT_OK, "firmware retired");
    expect(zlos_boot_seal() == ZLOS_BOOT_OK, "boot sealed");
    expect(pmm_init_boot() == PMM_OK, "sealed UEFI map parsed");
    expect(pmm_total_pages() == 10, "UEFI conventional pages admitted");
    expect(pmm_page_owner(PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES) ==
           PMM_OWNER_RESERVED, "UEFI non-conventional hole reserved");
    expect(pmm_boot_selftest() == PMM_OK, "boot selftest");
    expect(pmm_free_pages() == 10 && pmm_used_pages() == 0,
           "boot selftest restored baseline");
    expect(pmm_check() == 0, "post-selftest invariants");

    unsigned char multiboot_map[48] = {0};
    store_u32(multiboot_map, 20);
    store_u64(multiboot_map + 4, PMM_DYNAMIC_FLOOR);
    store_u64(multiboot_map + 12, 6 * PMM_PAGE_BYTES);
    store_u32(multiboot_map + 20, 1);
    store_u32(multiboot_map + 24, 20);
    store_u64(multiboot_map + 28, PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES);
    store_u64(multiboot_map + 36, 2 * PMM_PAGE_BYTES);
    store_u32(multiboot_map + 44, 2);
    expect(zlos_boot_begin(ZLOS_BOOT_ORIGIN_MULTIBOOT1, 64,
                           ZLOS_BOOT_GENERATION_CURRENT, 1, 3) == ZLOS_BOOT_OK,
           "multiboot begin");
    expect(zlos_boot_set_source(0x200000) == ZLOS_BOOT_OK, "multiboot source");
    expect(zlos_boot_set_memory_map((uint64_t)(uintptr_t)multiboot_map,
                                    sizeof(multiboot_map), 0, 1, 1) == ZLOS_BOOT_OK,
           "variable multiboot map set");
    expect(zlos_boot_mark_firmware_retired() == ZLOS_BOOT_OK,
           "multiboot firmware retired");
    expect(zlos_boot_seal() == ZLOS_BOOT_OK, "multiboot sealed");
    expect(pmm_init_boot() == PMM_OK, "variable multiboot map parsed");
    expect(pmm_total_pages() == 4, "multiboot reserved overlap excluded");
    expect(pmm_check() == 0, "multiboot map invariants");
}

static void test_boundaries_and_rollback(void)
{
    struct pmm_range partial = {
        PMM_DYNAMIC_FLOOR + 1, 3 * PMM_PAGE_BYTES - 2, PMM_RANGE_USABLE
    };
    expect(pmm_init_ranges(&partial, 1) == PMM_OK, "partial usable range init");
    expect(pmm_total_pages() == 1, "only complete usable pages admitted");
    expect(pmm_page_owner(PMM_DYNAMIC_FLOOR) == PMM_OWNER_RESERVED,
           "partial leading page remains reserved");
    expect(pmm_page_owner(PMM_DYNAMIC_FLOOR + PMM_PAGE_BYTES) == PMM_OWNER_FREE,
           "complete interior page admitted");
    expect(pmm_page_owner(PMM_DYNAMIC_FLOOR + 2 * PMM_PAGE_BYTES) ==
           PMM_OWNER_RESERVED, "partial trailing page remains reserved");
    expect(pmm_page_owner(PMM_DYNAMIC_FLOOR + 1) == PMM_OWNER_NONE,
           "unaligned owner query refused");

    struct pmm_range edge = {
        PMM_MANAGED_LIMIT - PMM_PAGE_BYTES, 2 * PMM_PAGE_BYTES, PMM_RANGE_USABLE
    };
    expect(pmm_init_ranges(&edge, 1) == PMM_OK, "managed ceiling clamps range");
    expect(pmm_total_pages() == 1, "only page below managed ceiling admitted");
    expect(pmm_page_owner(PMM_MANAGED_LIMIT - PMM_PAGE_BYTES) == PMM_OWNER_FREE,
           "last managed page is admitted");
    expect(pmm_page_owner(PMM_MANAGED_LIMIT) == PMM_OWNER_NONE,
           "managed ceiling itself is outside the pool");

    struct pmm_range unbacked = {
        PMM_DYNAMIC_FLOOR + TEST_PAGES * PMM_PAGE_BYTES,
        PMM_PAGE_BYTES, PMM_RANGE_USABLE
    };
    expect(pmm_init_ranges(&unbacked, 1) == PMM_OK, "unbacked host seam init");
    expect(pmm_alloc(20) == 0, "failed zeroing refuses publication");
    expect(pmm_free_pages() == 1 && pmm_used_pages() == 0,
           "failed zeroing rolls accounting back");
    expect(pmm_page_owner(unbacked.base) == PMM_OWNER_FREE,
           "failed zeroing restores ownership");
    expect(pmm_refusals() == 1 && pmm_check() == 0,
           "failed zeroing is counted and invariant-safe");

    struct pmm_range reserved = {
        PMM_DYNAMIC_FLOOR, PMM_PAGE_BYTES, PMM_RANGE_RESERVED
    };
    expect(pmm_init_ranges(&reserved, 1) == PMM_E_RANGE,
           "reserved-only map has no allocator");
    expect(!pmm_ready() && pmm_check() == 1, "failed init remains unavailable");
    expect(pmm_init_ranges(&reserved, 0) == PMM_E_ARGUMENT, "zero range count refused");
    expect(pmm_init_ranges(&reserved, 4097) == PMM_E_ARGUMENT,
           "oversized range count refused before dereference");
}

static void seal_test_map(unsigned int origin, uint64_t address, uint64_t bytes,
                          unsigned int entry_bytes, int variable, int retired)
{
    expect(zlos_boot_begin(origin, 64, ZLOS_BOOT_GENERATION_CURRENT, 1, 3) ==
           ZLOS_BOOT_OK, "malformed-map boot begin");
    expect(zlos_boot_set_source(0x300000) == ZLOS_BOOT_OK,
           "malformed-map boot source");
    expect(zlos_boot_set_memory_map(address, bytes, entry_bytes, 1, variable) ==
           ZLOS_BOOT_OK, "malformed-map handover accepts representable shape");
    if (retired)
        expect(zlos_boot_mark_firmware_retired() == ZLOS_BOOT_OK,
               "malformed-map firmware retired");
    expect(zlos_boot_seal() == ZLOS_BOOT_OK, "malformed-map handover sealed");
}

static void test_bad_boot_maps(void)
{
    struct efi_descriptor_test descriptor = {
        7, 0, PMM_DYNAMIC_FLOOR, 0, 1, 0
    };
    expect(zlos_boot_begin(ZLOS_BOOT_ORIGIN_NATIVE_UEFI, 64,
                           ZLOS_BOOT_GENERATION_CURRENT, 1, 3) == ZLOS_BOOT_OK,
           "unretired-map boot begin");
    expect(zlos_boot_set_source(0x300000) == ZLOS_BOOT_OK,
           "unretired-map boot source");
    expect(zlos_boot_set_memory_map((uint64_t)(uintptr_t)&descriptor,
                                    sizeof(descriptor), sizeof(descriptor),
                                    1, 0) == ZLOS_BOOT_OK,
           "unretired-map handover set");
    expect(zlos_boot_seal() == ZLOS_BOOT_E_STATE,
           "handover ABI refuses sealing before firmware retirement");
    expect(pmm_init_boot() == PMM_E_STATE, "unretired firmware map refused");

    unsigned char short_descriptor[39] = {0};
    store_u32(short_descriptor, 7);
    store_u64(short_descriptor + 8, PMM_DYNAMIC_FLOOR);
    store_u64(short_descriptor + 24, 1);
    seal_test_map(ZLOS_BOOT_ORIGIN_NATIVE_UEFI,
                  (uint64_t)(uintptr_t)short_descriptor,
                  sizeof(short_descriptor), sizeof(short_descriptor), 0, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "short UEFI descriptor refused");

    unsigned char odd_descriptor[41] = {0};
    store_u32(odd_descriptor, 7);
    store_u64(odd_descriptor + 8, PMM_DYNAMIC_FLOOR);
    store_u64(odd_descriptor + 24, 1);
    seal_test_map(ZLOS_BOOT_ORIGIN_NATIVE_UEFI,
                  (uint64_t)(uintptr_t)odd_descriptor,
                  sizeof(odd_descriptor), 40, 0, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "non-integral UEFI map refused");

    descriptor.pages = 0;
    seal_test_map(ZLOS_BOOT_ORIGIN_NATIVE_UEFI,
                  (uint64_t)(uintptr_t)&descriptor, sizeof(descriptor),
                  sizeof(descriptor), 0, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "zero-page UEFI descriptor refused");

    descriptor.type = 0;
    descriptor.pages = 1;
    seal_test_map(ZLOS_BOOT_ORIGIN_NATIVE_UEFI,
                  (uint64_t)(uintptr_t)&descriptor, sizeof(descriptor),
                  sizeof(descriptor), 0, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "reserved-only UEFI map refused");

    unsigned char bad_multiboot[24] = {0};
    store_u32(bad_multiboot, 19);
    seal_test_map(ZLOS_BOOT_ORIGIN_MULTIBOOT1,
                  (uint64_t)(uintptr_t)bad_multiboot, sizeof(bad_multiboot),
                  0, 1, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "short Multiboot payload refused");
    store_u32(bad_multiboot, 20);
    seal_test_map(ZLOS_BOOT_ORIGIN_MULTIBOOT1,
                  (uint64_t)(uintptr_t)bad_multiboot, sizeof(bad_multiboot) - 1,
                  0, 1, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "truncated Multiboot entry refused");

    seal_test_map(ZLOS_BOOT_ORIGIN_NATIVE_UEFI, UINT64_MAX - 8,
                  sizeof(descriptor), sizeof(descriptor), 0, 1);
    expect(pmm_init_boot() == PMM_E_RANGE, "overflowing map pointer refused");
}

static void test_bad_inputs(void)
{
    struct pmm_range overflow = {~0ULL - 1024ULL, 4096, PMM_RANGE_USABLE};
    struct pmm_range high_outside = {~0ULL - 1023ULL, 1023, PMM_RANGE_USABLE};
    struct pmm_range unknown = {PMM_DYNAMIC_FLOOR, PMM_PAGE_BYTES, 99};
    expect(pmm_init_ranges(0, 1) == PMM_E_ARGUMENT, "null ranges refused");
    expect(pmm_init_ranges(&overflow, 1) == PMM_E_RANGE, "overflow refused");
    expect(pmm_init_ranges(&high_outside, 1) == PMM_E_RANGE,
           "high out-of-window range cannot wrap into the managed pool");
    expect(pmm_init_ranges(&unknown, 1) == PMM_E_ARGUMENT, "unknown kind refused");
    expect(!pmm_ready(), "failed init remains unavailable");
}

int main(void)
{
    memset(backing, 0xCC, sizeof(backing));
    test_ranges();
    test_owner_accounting_and_limits();
    test_boot_map();
    test_boundaries_and_rollback();
    test_bad_boot_maps();
    test_bad_inputs();
    printf("pmmtest: %d checks, %d failures\n", checks, failures);
    if (!failures)
        puts("typed physical allocator enforces owner quotas, rejects reserved/foreign/double-free and zeroes reuse");
    return failures ? 1 : 0;
}
