/* anon_memory.c - failure-atomic reservation, commitment and reclamation. */
#include "anon_memory.h"

static int valid_owner(unsigned int owner)
{
    return owner >= ANON_MEMORY_OWNER_BASE && owner < PMM_OWNER_SELFTEST;
}

static int valid_range(unsigned int first, unsigned int count)
{
    return count && first < ANON_MEMORY_PAGE_COUNT &&
           count <= ANON_MEMORY_PAGE_COUNT - first;
}

static void clear_memory(struct anon_memory *memory)
{
    for (unsigned int i = 0; i < ANON_MEMORY_PAGE_COUNT; i++) {
        memory->pages[i] = 0;
        memory->states[i] = ANON_MEMORY_FREE;
    }
    memory->entries = 0;
    memory->pte_flags = 0;
    memory->owner = 0;
    memory->reserved_count = 0;
    memory->committed_count = 0;
    memory->broken_count = 0;
    memory->fail_after_write = 0;
    memory->flush = 0;
    memory->flush_context = 0;
}

static int shape_check(const struct anon_memory *memory, int free_entries)
{
    unsigned int reserved = 0;
    unsigned int committed = 0;
    unsigned int broken = 0;

    if (!memory || !memory->entries || !memory->pte_flags || !memory->flush ||
        !valid_owner(memory->owner))
        return ANON_MEMORY_E_STATE;

    for (unsigned int i = 0; i < ANON_MEMORY_PAGE_COUNT; i++) {
        unsigned int state = memory->states[i];
        if (state == ANON_MEMORY_FREE) {
            if (memory->pages[i] || (free_entries && memory->entries[i]))
                return ANON_MEMORY_E_CORRUPT;
            continue;
        }
        reserved++;
        if (state == ANON_MEMORY_RESERVED) {
            if (memory->pages[i] || memory->entries[i])
                return ANON_MEMORY_E_CORRUPT;
            continue;
        }
        if (state != ANON_MEMORY_COMMITTED && state != ANON_MEMORY_BROKEN)
            return ANON_MEMORY_E_CORRUPT;
        if (!memory->pages[i] ||
            pmm_page_owner(memory->pages[i]) != memory->owner)
            return ANON_MEMORY_E_CORRUPT;
        for (unsigned int j = 0; j < i; j++)
            if (memory->pages[i] == memory->pages[j])
                return ANON_MEMORY_E_CORRUPT;
        if (state == ANON_MEMORY_COMMITTED) {
            committed++;
            vmm_pt_word expected = memory->pages[i] | memory->pte_flags;
            if ((memory->entries[i] & ~ANON_MEMORY_PTE_HARDWARE_BITS) !=
                (expected & ~ANON_MEMORY_PTE_HARDWARE_BITS))
                return ANON_MEMORY_E_CORRUPT;
        } else {
            broken++;
        }
    }
    if (reserved != memory->reserved_count ||
        committed != memory->committed_count ||
        broken != memory->broken_count ||
        committed + broken > reserved)
        return ANON_MEMORY_E_CORRUPT;
    return ANON_MEMORY_OK;
}

static int release_pages(pmm_u64 pages[ANON_MEMORY_PAGE_COUNT],
                         unsigned int count, unsigned int owner)
{
    while (count) {
        count--;
        if (pages[count] && pmm_release(pages[count], owner) != PMM_OK)
            return ANON_MEMORY_E_CORRUPT;
    }
    return ANON_MEMORY_OK;
}

int anon_memory_init(struct anon_memory *memory, unsigned int owner,
                     volatile vmm_pt_word *entries, vmm_pt_word pte_flags,
                     vmm_pt_flush_fn flush, void *flush_context)
{
    if (!memory || !entries || !pte_flags || !flush)
        return ANON_MEMORY_E_ARGUMENT;
    if (!(pte_flags & 1ULL) || (pte_flags & ANON_MEMORY_PTE_ADDRESS_MASK))
        return ANON_MEMORY_E_ARGUMENT;
    if (!valid_owner(owner)) return ANON_MEMORY_E_OWNER;
    if (memory->entries || memory->owner || memory->reserved_count ||
        memory->committed_count || memory->broken_count || memory->flush)
        return ANON_MEMORY_E_STATE;
    for (unsigned int i = 0; i < ANON_MEMORY_PAGE_COUNT; i++)
        if (memory->pages[i] || memory->states[i])
            return ANON_MEMORY_E_STATE;
        else if (entries[i])
            return ANON_MEMORY_E_COLLISION;

    memory->entries = entries;
    memory->pte_flags = pte_flags;
    memory->owner = owner;
    memory->flush = flush;
    memory->flush_context = flush_context;
    return shape_check(memory, 0);
}

int anon_memory_check(const struct anon_memory *memory)
{
    return shape_check(memory, 1);
}

int anon_memory_ready(const struct anon_memory *memory)
{
    return anon_memory_check(memory) == ANON_MEMORY_OK;
}

enum anon_memory_page_state anon_memory_state(const struct anon_memory *memory,
                                              unsigned int page)
{
    if (!memory || page >= ANON_MEMORY_PAGE_COUNT)
        return ANON_MEMORY_BROKEN;
    return (enum anon_memory_page_state)memory->states[page];
}

pmm_u64 anon_memory_page(const struct anon_memory *memory, unsigned int page)
{
    if (!memory || page >= ANON_MEMORY_PAGE_COUNT ||
        (memory->states[page] != ANON_MEMORY_COMMITTED &&
         memory->states[page] != ANON_MEMORY_BROKEN))
        return 0;
    return memory->pages[page];
}

void anon_memory_fail_after_write(struct anon_memory *memory,
                                  unsigned int writes)
{
    if (memory) memory->fail_after_write = writes;
}

int anon_memory_reserve(struct anon_memory *memory, unsigned int first,
                        unsigned int count)
{
    if (!memory) return ANON_MEMORY_E_ARGUMENT;
    if (!valid_range(first, count)) return ANON_MEMORY_E_LIMIT;
    if (shape_check(memory, 0) != ANON_MEMORY_OK)
        return ANON_MEMORY_E_CORRUPT;
    for (unsigned int i = first; i < first + count; i++) {
        if (memory->states[i] != ANON_MEMORY_FREE)
            return ANON_MEMORY_E_STATE;
        if (memory->entries[i]) return ANON_MEMORY_E_COLLISION;
    }
    for (unsigned int i = first; i < first + count; i++)
        memory->states[i] = ANON_MEMORY_RESERVED;
    memory->reserved_count += count;
    return ANON_MEMORY_OK;
}

int anon_memory_commit(struct anon_memory *memory, unsigned int first,
                       unsigned int count)
{
    pmm_u64 pages[ANON_MEMORY_PAGE_COUNT] = {0};
    struct vmm_pt_change changes[ANON_MEMORY_PAGE_COUNT];
    struct vmm_pt_transaction transaction;
    unsigned int allocated = 0;

    if (!memory) return ANON_MEMORY_E_ARGUMENT;
    if (!valid_range(first, count)) return ANON_MEMORY_E_LIMIT;
    if (shape_check(memory, 0) != ANON_MEMORY_OK)
        return ANON_MEMORY_E_CORRUPT;
    for (unsigned int i = first; i < first + count; i++) {
        if (memory->states[i] != ANON_MEMORY_RESERVED)
            return ANON_MEMORY_E_STATE;
        if (memory->entries[i]) return ANON_MEMORY_E_COLLISION;
    }

    for (; allocated < count; allocated++) {
        pages[allocated] = pmm_alloc(memory->owner);
        if (!pages[allocated]) {
            if (release_pages(pages, allocated, memory->owner) != ANON_MEMORY_OK)
                return ANON_MEMORY_E_CORRUPT;
            return ANON_MEMORY_E_NOMEM;
        }
    }

    if (vmm_pt_txn_begin(&transaction, changes, count, memory->flush,
                         memory->flush_context) != VMM_PT_TXN_OK) {
        if (release_pages(pages, allocated, memory->owner) != ANON_MEMORY_OK)
            return ANON_MEMORY_E_CORRUPT;
        return ANON_MEMORY_E_TRANSACTION;
    }
    for (unsigned int i = 0; i < count; i++) {
        if (vmm_pt_txn_reserve(&transaction, &memory->entries[first + i],
                               pages[i] | memory->pte_flags) != VMM_PT_TXN_OK) {
            if (release_pages(pages, allocated, memory->owner) != ANON_MEMORY_OK)
                return ANON_MEMORY_E_CORRUPT;
            return ANON_MEMORY_E_TRANSACTION;
        }
    }
    vmm_pt_txn_fail_after_write(&transaction, memory->fail_after_write);
    memory->fail_after_write = 0;
    int status = vmm_pt_txn_apply(&transaction);
    if (status == VMM_PT_TXN_ERR_ROLLBACK) {
        for (unsigned int i = 0; i < count; i++) {
            memory->pages[first + i] = pages[i];
            memory->states[first + i] = ANON_MEMORY_BROKEN;
        }
        memory->broken_count += count;
        return ANON_MEMORY_E_CORRUPT;
    }
    if (status != VMM_PT_TXN_OK) {
        if (release_pages(pages, allocated, memory->owner) != ANON_MEMORY_OK)
            return ANON_MEMORY_E_CORRUPT;
        return ANON_MEMORY_E_TRANSACTION;
    }
    if (vmm_pt_txn_commit(&transaction) != VMM_PT_TXN_OK) {
        for (unsigned int i = 0; i < count; i++) {
            memory->pages[first + i] = pages[i];
            memory->states[first + i] = ANON_MEMORY_BROKEN;
        }
        memory->broken_count += count;
        return ANON_MEMORY_E_CORRUPT;
    }

    for (unsigned int i = 0; i < count; i++) {
        memory->pages[first + i] = pages[i];
        memory->states[first + i] = ANON_MEMORY_COMMITTED;
    }
    memory->committed_count += count;
    return ANON_MEMORY_OK;
}

int anon_memory_release(struct anon_memory *memory, unsigned int first,
                        unsigned int count)
{
    struct vmm_pt_change changes[ANON_MEMORY_PAGE_COUNT];
    struct vmm_pt_transaction transaction;
    unsigned int mapped = 0;

    if (!memory) return ANON_MEMORY_E_ARGUMENT;
    if (!valid_range(first, count)) return ANON_MEMORY_E_LIMIT;
    if (shape_check(memory, 0) != ANON_MEMORY_OK)
        return ANON_MEMORY_E_CORRUPT;
    for (unsigned int i = first; i < first + count; i++) {
        if (memory->states[i] == ANON_MEMORY_FREE ||
            memory->states[i] == ANON_MEMORY_BROKEN)
            return ANON_MEMORY_E_STATE;
        if (memory->states[i] == ANON_MEMORY_COMMITTED) mapped++;
    }

    if (mapped) {
        if (vmm_pt_txn_begin(&transaction, changes, mapped, memory->flush,
                             memory->flush_context) != VMM_PT_TXN_OK)
            return ANON_MEMORY_E_TRANSACTION;
        for (unsigned int i = first; i < first + count; i++) {
            if (memory->states[i] != ANON_MEMORY_COMMITTED) continue;
            if (vmm_pt_txn_reserve(&transaction, &memory->entries[i], 0) !=
                VMM_PT_TXN_OK)
                return ANON_MEMORY_E_TRANSACTION;
        }
        vmm_pt_txn_fail_after_write(&transaction, memory->fail_after_write);
        memory->fail_after_write = 0;
        int status = vmm_pt_txn_apply(&transaction);
        if (status == VMM_PT_TXN_ERR_ROLLBACK) {
            for (unsigned int i = first; i < first + count; i++)
                if (memory->states[i] == ANON_MEMORY_COMMITTED) {
                    memory->states[i] = ANON_MEMORY_BROKEN;
                    memory->committed_count--;
                    memory->broken_count++;
                }
            return ANON_MEMORY_E_CORRUPT;
        }
        if (status != VMM_PT_TXN_OK)
            return ANON_MEMORY_E_TRANSACTION;
        if (vmm_pt_txn_commit(&transaction) != VMM_PT_TXN_OK)
            return ANON_MEMORY_E_CORRUPT;
    }

    for (unsigned int i = first; i < first + count; i++) {
        if (memory->states[i] == ANON_MEMORY_COMMITTED) {
            if (pmm_release(memory->pages[i], memory->owner) != PMM_OK)
                return ANON_MEMORY_E_CORRUPT;
            memory->pages[i] = 0;
            memory->committed_count--;
        }
        memory->states[i] = ANON_MEMORY_FREE;
        memory->reserved_count--;
    }
    return ANON_MEMORY_OK;
}

int anon_memory_destroy(struct anon_memory *memory)
{
    if (!memory) return ANON_MEMORY_E_ARGUMENT;
    if (anon_memory_check(memory) != ANON_MEMORY_OK || memory->broken_count)
        return ANON_MEMORY_E_CORRUPT;
    unsigned int page = 0;
    while (page < ANON_MEMORY_PAGE_COUNT) {
        while (page < ANON_MEMORY_PAGE_COUNT &&
               memory->states[page] == ANON_MEMORY_FREE)
            page++;
        if (page == ANON_MEMORY_PAGE_COUNT) break;
        unsigned int first = page;
        while (page < ANON_MEMORY_PAGE_COUNT &&
               memory->states[page] != ANON_MEMORY_FREE)
            page++;
        int status = anon_memory_release(memory, first, page - first);
        if (status != ANON_MEMORY_OK) return status;
    }
    clear_memory(memory);
    return ANON_MEMORY_OK;
}
