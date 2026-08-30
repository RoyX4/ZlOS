/* pmm.c - a bounded, owner-tagged physical page allocator.
 *
 * The current kernel still has a fixed physical map through 320 MiB. This
 * allocator does not reinterpret any of it. It admits only pages which the
 * sealed boot handover calls usable, at or above PMM_DYNAMIC_FLOOR, and below
 * the 1 GiB minimum-machine contract. That conservative ceiling keeps the
 * metadata fixed and makes every admitted physical address directly testable
 * on all current QEMU routes.
 */
#include "pmm.h"
#include "boot/boot_handover.h"

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

#define PMM_PAGE_COUNT ((unsigned int)(PMM_MANAGED_LIMIT / PMM_PAGE_BYTES))
#define PMM_FIRST_PAGE ((unsigned int)(PMM_DYNAMIC_FLOOR / PMM_PAGE_BYTES))
#define PMM_MAX_RANGES 4096U
#define PMM_OWNER_COUNT 256U

static u8 owners[PMM_PAGE_COUNT];
static unsigned int cursor;
static unsigned long total_count;
static unsigned long free_count;
static unsigned long used_count;
static unsigned long high_water;
static unsigned long refusal_count;
static unsigned long owner_used[PMM_OWNER_COUNT];
static unsigned long owner_high_water[PMM_OWNER_COUNT];
static unsigned long owner_limits[PMM_OWNER_COUNT];
static unsigned long owner_refusals[PMM_OWNER_COUNT];
static unsigned long owner_audit[PMM_OWNER_COUNT];
static int live;

_Static_assert(PMM_DYNAMIC_FLOOR % PMM_PAGE_BYTES == 0,
               "PMM floor must be page aligned");
_Static_assert(PMM_MANAGED_LIMIT % PMM_PAGE_BYTES == 0,
               "PMM limit must be page aligned");
_Static_assert(PMM_DYNAMIC_FLOOR < PMM_MANAGED_LIMIT,
               "PMM managed interval is inverted");
_Static_assert(PMM_PAGE_COUNT == 262144U,
               "PMM metadata no longer describes exactly the first GiB");

void zl_putc_pub(char c);

static void say(const char *s) { while (*s) zl_putc_pub(*s++); }

static void say_u(unsigned long value)
{
    char digits[24];
    int count = 0;
    if (!value) { zl_putc_pub('0'); return; }
    while (value) {
        digits[count++] = (char)('0' + value % 10UL);
        value /= 10UL;
    }
    while (count) zl_putc_pub(digits[--count]);
}

static int dynamic_owner(unsigned int owner)
{
    return owner >= PMM_OWNER_DYNAMIC_MIN && owner < PMM_OWNER_SELFTEST;
}

static int any_owner(unsigned int owner)
{
    return dynamic_owner(owner) || owner == PMM_OWNER_SELFTEST;
}

static void reset_state(void)
{
    for (unsigned int i = 0; i < PMM_PAGE_COUNT; i++)
        owners[i] = PMM_OWNER_RESERVED;
    cursor = PMM_FIRST_PAGE;
    total_count = 0;
    free_count = 0;
    used_count = 0;
    high_water = 0;
    refusal_count = 0;
    for (unsigned int owner = 0; owner < PMM_OWNER_COUNT; owner++) {
        owner_used[owner] = 0;
        owner_high_water[owner] = 0;
        owner_limits[owner] = PMM_LIMIT_UNLIMITED;
        owner_refusals[owner] = 0;
        owner_audit[owner] = 0;
    }
    live = 0;
}

static int range_shape(u64 base, u64 bytes)
{
    if (!bytes || base > ~0ULL - bytes) return PMM_E_RANGE;
    return PMM_OK;
}

static int boot_map_window(const struct zlos_boot_handover *boot)
{
    u64 pointer_max = (u64)(~(__UINTPTR_TYPE__)0);
    if (boot->memory_map_address > pointer_max ||
        boot->memory_map_bytes > pointer_max ||
        boot->memory_map_address > pointer_max - boot->memory_map_bytes)
        return PMM_E_RANGE;
    return PMM_OK;
}

static void apply_range(u64 base, u64 bytes, int usable)
{
    u64 end = base + bytes;
    /* Usable admission requires a complete page inside the descriptor.
     * Reservation is the inverse: any touched byte reserves the whole page.
     * Division plus a remainder check avoids `base + 4095` wrapping near the
     * top of the 64-bit physical-address domain. */
    u64 first = base / PMM_PAGE_BYTES;
    u64 after = end / PMM_PAGE_BYTES;
    if (usable && base % PMM_PAGE_BYTES) first++;
    if (!usable && end % PMM_PAGE_BYTES) after++;
    if (first < PMM_FIRST_PAGE) first = PMM_FIRST_PAGE;
    if (after > PMM_PAGE_COUNT) after = PMM_PAGE_COUNT;
    if (after <= first) return;

    for (u64 page = first; page < after; page++) {
        unsigned int index = (unsigned int)page;
        if (usable) {
            if (owners[index] == PMM_OWNER_RESERVED) {
                owners[index] = PMM_OWNER_FREE;
                total_count++;
                free_count++;
            }
        } else if (owners[index] == PMM_OWNER_FREE) {
            owners[index] = PMM_OWNER_RESERVED;
            total_count--;
            free_count--;
        }
    }
}

int pmm_init_ranges(const struct pmm_range *ranges, unsigned int count)
{
    reset_state();
    if (!ranges || !count || count > PMM_MAX_RANGES) return PMM_E_ARGUMENT;
    for (unsigned int i = 0; i < count; i++) {
        if (ranges[i].kind != PMM_RANGE_USABLE &&
            ranges[i].kind != PMM_RANGE_RESERVED)
            return PMM_E_ARGUMENT;
        if (range_shape(ranges[i].base, ranges[i].bytes) != PMM_OK)
            return PMM_E_RANGE;
    }
    /* Reserved descriptors win even if a malformed source overlaps them with
     * a usable descriptor. This is the only safe interpretation of overlap. */
    for (unsigned int pass = 0; pass < 2; pass++)
        for (unsigned int i = 0; i < count; i++)
            if ((pass == 0 && ranges[i].kind == PMM_RANGE_USABLE) ||
                (pass == 1 && ranges[i].kind == PMM_RANGE_RESERVED))
                apply_range(ranges[i].base, ranges[i].bytes, pass == 0);
    if (!total_count) return PMM_E_RANGE;
    live = 1;
    return PMM_OK;
}

static u32 load_u32(const u8 *p)
{
    return (u32)p[0] | ((u32)p[1] << 8) | ((u32)p[2] << 16) |
           ((u32)p[3] << 24);
}

static u64 load_u64(const u8 *p)
{
    return (u64)load_u32(p) | ((u64)load_u32(p + 4) << 32);
}

static int apply_uefi_map(const struct zlos_boot_handover *boot)
{
    if (boot->memory_map_entry_bytes < 40U ||
        boot->memory_map_bytes % boot->memory_map_entry_bytes != 0 ||
        boot->memory_map_bytes / boot->memory_map_entry_bytes > PMM_MAX_RANGES)
        return PMM_E_RANGE;
    const u8 *map = (const u8 *)(__UINTPTR_TYPE__)boot->memory_map_address;
    unsigned int count = (unsigned int)(boot->memory_map_bytes /
                                        boot->memory_map_entry_bytes);
    for (unsigned int i = 0; i < count; i++) {
        const u8 *d = map + (unsigned long)i * boot->memory_map_entry_bytes;
        u64 pages = load_u64(d + 24);
        if (!pages || pages > (~0ULL / PMM_PAGE_BYTES)) return PMM_E_RANGE;
        if (range_shape(load_u64(d + 8), pages * PMM_PAGE_BYTES) != PMM_OK)
            return PMM_E_RANGE;
    }
    for (unsigned int pass = 0; pass < 2; pass++) {
        for (unsigned int i = 0; i < count; i++) {
            const u8 *d = map + (unsigned long)i * boot->memory_map_entry_bytes;
            int usable = load_u32(d) == 7U; /* EfiConventionalMemory */
            if ((pass == 0 && usable) || (pass == 1 && !usable))
                apply_range(load_u64(d + 8), load_u64(d + 24) * PMM_PAGE_BYTES,
                            usable);
        }
    }
    return PMM_OK;
}

static int apply_multiboot_map(const struct zlos_boot_handover *boot)
{
    const u8 *map = (const u8 *)(__UINTPTR_TYPE__)boot->memory_map_address;
    u64 offset = 0;
    unsigned int count = 0;
    while (offset < boot->memory_map_bytes) {
        if (boot->memory_map_bytes - offset < 4ULL) return PMM_E_RANGE;
        u32 payload = load_u32(map + offset);
        u64 step = (u64)payload + 4ULL;
        if (payload < 20U || step > boot->memory_map_bytes - offset ||
            ++count > PMM_MAX_RANGES)
            return PMM_E_RANGE;
        if (range_shape(load_u64(map + offset + 4),
                        load_u64(map + offset + 12)) != PMM_OK)
            return PMM_E_RANGE;
        offset += step;
    }
    for (unsigned int pass = 0; pass < 2; pass++) {
        offset = 0;
        while (offset < boot->memory_map_bytes) {
            u32 payload = load_u32(map + offset);
            int usable = load_u32(map + offset + 20) == 1U;
            if ((pass == 0 && usable) || (pass == 1 && !usable))
                apply_range(load_u64(map + offset + 4),
                            load_u64(map + offset + 12), usable);
            offset += (u64)payload + 4ULL;
        }
    }
    return PMM_OK;
}

int pmm_init_boot(void)
{
    reset_state();
    const struct zlos_boot_handover *boot = zlos_boot_record();
    if (zlos_boot_validate(boot) != ZLOS_BOOT_OK ||
        !(boot->flags & ZLOS_BOOT_F_MEMORY_MAP))
        return PMM_E_STATE;
    if (!(boot->flags & ZLOS_BOOT_F_FIRMWARE_RETIRED)) return PMM_E_STATE;
    if (boot_map_window(boot) != PMM_OK) return PMM_E_RANGE;
    int status;
    if (boot->origin == ZLOS_BOOT_ORIGIN_NATIVE_UEFI)
        status = apply_uefi_map(boot);
    else if (boot->origin == ZLOS_BOOT_ORIGIN_MULTIBOOT1 &&
             (boot->flags & ZLOS_BOOT_F_VARIABLE_MAP_ENTRIES))
        status = apply_multiboot_map(boot);
    else
        return PMM_E_STATE;
    if (status != PMM_OK || !total_count)
        return status != PMM_OK ? status : PMM_E_RANGE;
    /* The sealed handover remains diagnostic evidence after initialization.
     * Never publish its backing pages even if firmware called them usable. */
    apply_range(boot->memory_map_address, boot->memory_map_bytes, 0);
    if (!total_count) return PMM_E_RANGE;
    live = 1;
    return PMM_OK;
}

static volatile u8 *page_pointer(u64 address)
{
#if defined(PMM_HOSTTEST)
    extern void *pmm_host_page_pointer(u64 address);
    return (volatile u8 *)pmm_host_page_pointer(address);
#else
    return (volatile u8 *)(__UINTPTR_TYPE__)address;
#endif
}

static int zero_page(u64 address)
{
    volatile u8 *page = page_pointer(address);
    if (!page) return 0;
    for (unsigned int i = 0; i < PMM_PAGE_BYTES; i++) page[i] = 0;
    return 1;
}

pmm_u64 pmm_alloc(unsigned int owner)
{
    if (!live || !any_owner(owner)) { refusal_count++; return 0; }
    if (owner_used[owner] >= owner_limits[owner]) {
        owner_refusals[owner]++;
        refusal_count++;
        return 0;
    }
    unsigned int start = cursor;
    for (unsigned int scanned = 0; scanned < PMM_PAGE_COUNT - PMM_FIRST_PAGE;
         scanned++) {
        unsigned int page = PMM_FIRST_PAGE +
            ((start - PMM_FIRST_PAGE + scanned) %
             (PMM_PAGE_COUNT - PMM_FIRST_PAGE));
        if (owners[page] != PMM_OWNER_FREE) continue;
        u64 address = (u64)page * PMM_PAGE_BYTES;
        owners[page] = (u8)owner;
        free_count--;
        used_count++;
        owner_used[owner]++;
        if (used_count > high_water) high_water = used_count;
        if (owner_used[owner] > owner_high_water[owner])
            owner_high_water[owner] = owner_used[owner];
        cursor = page + 1U;
        if (cursor >= PMM_PAGE_COUNT) cursor = PMM_FIRST_PAGE;
        if (!zero_page(address)) {
            owners[page] = PMM_OWNER_FREE;
            free_count++;
            used_count--;
            owner_used[owner]--;
            cursor = page;
            owner_refusals[owner]++;
            refusal_count++;
            return 0;
        }
        return address;
    }
    owner_refusals[owner]++;
    refusal_count++;
    return 0;
}

int pmm_release(pmm_u64 address, unsigned int owner)
{
    if (!live) return PMM_E_STATE;
    if (!any_owner(owner)) return PMM_E_OWNER;
    if (address % PMM_PAGE_BYTES) return PMM_E_ALIGNMENT;
    if (address < PMM_DYNAMIC_FLOOR || address >= PMM_MANAGED_LIMIT)
        return PMM_E_RANGE;
    unsigned int page = (unsigned int)(address / PMM_PAGE_BYTES);
    if (owners[page] == PMM_OWNER_RESERVED) return PMM_E_RESERVED;
    if (owners[page] == PMM_OWNER_FREE) return PMM_E_DOUBLE_FREE;
    if (owners[page] != owner) return PMM_E_OWNER;
    if (!used_count || !owner_used[owner]) return PMM_E_CORRUPT;
    owners[page] = PMM_OWNER_FREE;
    used_count--;
    owner_used[owner]--;
    free_count++;
    if (page < cursor) cursor = page;
    return PMM_OK;
}

unsigned int pmm_page_owner(pmm_u64 address)
{
    if (address % PMM_PAGE_BYTES || address >= PMM_MANAGED_LIMIT)
        return PMM_OWNER_NONE;
    return owners[(unsigned int)(address / PMM_PAGE_BYTES)];
}

int pmm_set_owner_limit(unsigned int owner, unsigned long pages)
{
    if (!live) return PMM_E_STATE;
    if (!any_owner(owner)) return PMM_E_OWNER;
    if (pages < owner_used[owner]) return PMM_E_LIMIT;
    owner_limits[owner] = pages;
    return PMM_OK;
}

int pmm_owner_account(unsigned int owner, struct pmm_owner_account *account)
{
    if (!live) return PMM_E_STATE;
    if (!account) return PMM_E_ARGUMENT;
    if (!any_owner(owner)) return PMM_E_OWNER;
    unsigned long available = free_count;
    if (owner_limits[owner] != PMM_LIMIT_UNLIMITED) {
        unsigned long owner_available = owner_limits[owner] - owner_used[owner];
        if (available > owner_available) available = owner_available;
    }
    account->owner = owner;
    account->used_pages = owner_used[owner];
    account->high_water_pages = owner_high_water[owner];
    account->limit_pages = owner_limits[owner];
    account->available_pages = available;
    account->refusals = owner_refusals[owner];
    return PMM_OK;
}

int pmm_boot_selftest(void)
{
    if (!live) return PMM_E_STATE;
    unsigned long baseline_free = free_count;
    unsigned long baseline_used = used_count;
    u64 first = pmm_alloc(PMM_OWNER_SELFTEST);
    if (!first) return PMM_E_RANGE;
    struct pmm_owner_account account;
    if (pmm_set_owner_limit(PMM_OWNER_SELFTEST, 1) != PMM_OK ||
        pmm_alloc(PMM_OWNER_SELFTEST) != 0 ||
        pmm_owner_account(PMM_OWNER_SELFTEST, &account) != PMM_OK ||
        account.used_pages != 1 || account.high_water_pages != 1 ||
        account.limit_pages != 1 || account.available_pages != 0 ||
        account.refusals != 1)
        return PMM_E_CORRUPT;
    volatile u8 *page = page_pointer(first);
    if (!page) return PMM_E_RANGE;
    for (unsigned int i = 0; i < PMM_PAGE_BYTES; i++)
        if (page[i] != 0) return PMM_E_CORRUPT;
    for (unsigned int i = 0; i < PMM_PAGE_BYTES; i++) page[i] = 0xA5U;
    if (pmm_release(first, PMM_OWNER_DYNAMIC_MIN) != PMM_E_OWNER ||
        pmm_page_owner(first) != PMM_OWNER_SELFTEST ||
        pmm_release(first, PMM_OWNER_SELFTEST) != PMM_OK ||
        pmm_release(first, PMM_OWNER_SELFTEST) != PMM_E_DOUBLE_FREE)
        return PMM_E_CORRUPT;
    u64 second = pmm_alloc(PMM_OWNER_SELFTEST);
    if (second != first) return PMM_E_CORRUPT;
    page = page_pointer(second);
    if (!page) return PMM_E_RANGE;
    for (unsigned int i = 0; i < PMM_PAGE_BYTES; i++)
        if (page[i] != 0) return PMM_E_CORRUPT;
    if (pmm_release(second, PMM_OWNER_SELFTEST) != PMM_OK ||
        pmm_set_owner_limit(PMM_OWNER_SELFTEST, PMM_LIMIT_UNLIMITED) != PMM_OK ||
        free_count != baseline_free || used_count != baseline_used ||
        pmm_owner_account(PMM_OWNER_SELFTEST, &account) != PMM_OK ||
        account.used_pages != 0 || account.high_water_pages != 1 ||
        account.limit_pages != PMM_LIMIT_UNLIMITED || account.refusals != 1)
        return PMM_E_CORRUPT;
    return PMM_OK;
}

void pmm_report(void)
{
    int status = pmm_init_boot();
    if (status != PMM_OK) {
        say("  pmm: unavailable status ");
        say_u((unsigned long)(-status));
        say("\n");
        return;
    }
    status = pmm_boot_selftest();
    say("  pmm: ");
    say_u(free_count);
    say("/");
    say_u(total_count);
    say(" pages free in [320, 1024) MiB\n");
    if (status == PMM_OK)
        say("  <- physical allocator reserved floor, owner quota/mismatch, double-free and zero/reuse passed; baseline restored\n");
    else {
        live = 0;
        say("  <- physical allocator selftest FAILED status ");
        say_u((unsigned long)(-status));
        say("\n");
    }
}

int pmm_ready(void) { return live; }
unsigned long pmm_total_pages(void) { return total_count; }
unsigned long pmm_free_pages(void) { return free_count; }
unsigned long pmm_used_pages(void) { return used_count; }
unsigned long pmm_high_water_pages(void) { return high_water; }
unsigned long pmm_refusals(void) { return refusal_count; }

unsigned long pmm_check(void)
{
    if (!live) return 1;
    unsigned long total = 0, free_pages = 0, used_pages = 0;
    for (unsigned int owner = 0; owner < PMM_OWNER_COUNT; owner++)
        owner_audit[owner] = 0;
    for (unsigned int page = PMM_FIRST_PAGE; page < PMM_PAGE_COUNT; page++) {
        u8 owner = owners[page];
        if (owner == PMM_OWNER_FREE) { total++; free_pages++; }
        else if (owner != PMM_OWNER_RESERVED) {
            if (!any_owner(owner)) return 2;
            total++;
            used_pages++;
            owner_audit[owner]++;
        }
    }
    unsigned long accounted = 0;
    for (unsigned int owner = 0; owner < PMM_OWNER_COUNT; owner++) {
        if (owner_audit[owner] != owner_used[owner] ||
            owner_high_water[owner] < owner_used[owner] ||
            owner_limits[owner] < owner_used[owner])
            return 4;
        accounted += owner_used[owner];
    }
    return total == total_count && free_pages == free_count &&
           used_pages == used_count && accounted == used_count &&
           free_count + used_count == total_count
        ? 0UL : 3UL;
}
