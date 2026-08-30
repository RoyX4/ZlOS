/* pmm.h - bounded physical-frame ownership for the current 1 GiB contract. */
#ifndef ZLOS_PMM_H
#define ZLOS_PMM_H

#include "arch/x86/memmap.h"

typedef unsigned long long pmm_u64;

#define PMM_PAGE_BYTES       4096ULL
#define PMM_DYNAMIC_FLOOR    ((pmm_u64)HI_PMM)
#define PMM_MANAGED_LIMIT    ((pmm_u64)HI_TOP)
#define PMM_LIMIT_UNLIMITED  (~0UL)

enum pmm_range_kind {
    PMM_RANGE_RESERVED = 0,
    PMM_RANGE_USABLE = 1
};

enum pmm_owner {
    PMM_OWNER_NONE = 0,
    PMM_OWNER_FREE = 1,
    PMM_OWNER_RESERVED = 2,
    PMM_OWNER_DYNAMIC_MIN = 16,
    PMM_OWNER_SELFTEST = 254
};

enum pmm_status {
    PMM_OK = 0,
    PMM_E_ARGUMENT = -1,
    PMM_E_STATE = -2,
    PMM_E_RANGE = -3,
    PMM_E_ALIGNMENT = -4,
    PMM_E_RESERVED = -5,
    PMM_E_DOUBLE_FREE = -6,
    PMM_E_OWNER = -7,
    PMM_E_CORRUPT = -8,
    PMM_E_LIMIT = -9
};

struct pmm_range {
    pmm_u64 base;
    pmm_u64 bytes;
    unsigned int kind;
};

struct pmm_owner_account {
    unsigned int owner;
    unsigned long used_pages;
    unsigned long high_water_pages;
    unsigned long limit_pages;
    unsigned long available_pages;
    unsigned long refusals;
};

int pmm_init_ranges(const struct pmm_range *ranges, unsigned int count);
int pmm_init_boot(void);
int pmm_boot_selftest(void);
void pmm_report(void);

pmm_u64 pmm_alloc(unsigned int owner);
int pmm_release(pmm_u64 address, unsigned int owner);
unsigned int pmm_page_owner(pmm_u64 address);
int pmm_set_owner_limit(unsigned int owner, unsigned long pages);
int pmm_owner_account(unsigned int owner, struct pmm_owner_account *account);

int pmm_ready(void);
unsigned long pmm_total_pages(void);
unsigned long pmm_free_pages(void);
unsigned long pmm_used_pages(void);
unsigned long pmm_high_water_pages(void);
unsigned long pmm_refusals(void);
unsigned long pmm_check(void);

#endif
