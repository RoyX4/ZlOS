/* anon_memory.h - bounded reserved/committed anonymous process pages. */
#ifndef ZLOS_ANON_MEMORY_H
#define ZLOS_ANON_MEMORY_H

#include "pmm.h"
#include "arch/x86/page_table_txn.h"

#define ANON_MEMORY_PAGE_COUNT 32U
#define ANON_MEMORY_OWNER_BASE 64U
#define ANON_MEMORY_PTE_ADDRESS_MASK 0x000ffffffffff000ULL
#define ANON_MEMORY_PTE_ACCESSED (1ULL << 5)
#define ANON_MEMORY_PTE_DIRTY (1ULL << 6)
#define ANON_MEMORY_PTE_HARDWARE_BITS \
    (ANON_MEMORY_PTE_ACCESSED | ANON_MEMORY_PTE_DIRTY)

enum anon_memory_page_state {
    ANON_MEMORY_FREE = 0,
    ANON_MEMORY_RESERVED,
    ANON_MEMORY_COMMITTED,
    ANON_MEMORY_BROKEN
};

enum anon_memory_status {
    ANON_MEMORY_OK = 0,
    ANON_MEMORY_E_ARGUMENT = -1,
    ANON_MEMORY_E_STATE = -2,
    ANON_MEMORY_E_LIMIT = -3,
    ANON_MEMORY_E_COLLISION = -4,
    ANON_MEMORY_E_NOMEM = -5,
    ANON_MEMORY_E_OWNER = -6,
    ANON_MEMORY_E_TRANSACTION = -7,
    ANON_MEMORY_E_CORRUPT = -8
};

struct anon_memory {
    pmm_u64 pages[ANON_MEMORY_PAGE_COUNT];
    unsigned char states[ANON_MEMORY_PAGE_COUNT];
    volatile vmm_pt_word *entries;
    vmm_pt_word pte_flags;
    unsigned int owner;
    unsigned int reserved_count;
    unsigned int committed_count;
    unsigned int broken_count;
    unsigned int fail_after_write;
    vmm_pt_flush_fn flush;
    void *flush_context;
};

int anon_memory_init(struct anon_memory *memory, unsigned int owner,
                     volatile vmm_pt_word *entries, vmm_pt_word pte_flags,
                     vmm_pt_flush_fn flush, void *flush_context);
int anon_memory_reserve(struct anon_memory *memory, unsigned int first,
                        unsigned int count);
int anon_memory_commit(struct anon_memory *memory, unsigned int first,
                       unsigned int count);
int anon_memory_release(struct anon_memory *memory, unsigned int first,
                        unsigned int count);
int anon_memory_destroy(struct anon_memory *memory);
int anon_memory_check(const struct anon_memory *memory);
int anon_memory_ready(const struct anon_memory *memory);
enum anon_memory_page_state anon_memory_state(const struct anon_memory *memory,
                                              unsigned int page);
pmm_u64 anon_memory_page(const struct anon_memory *memory, unsigned int page);
void anon_memory_fail_after_write(struct anon_memory *memory,
                                  unsigned int writes);

#endif
