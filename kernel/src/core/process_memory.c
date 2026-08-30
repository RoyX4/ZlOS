/* process_memory.c - owned page-table, code and guarded-stack frames. */
#include "process_memory.h"

static int process_owner(unsigned int owner)
{
    return owner >= PROCESS_MEMORY_OWNER_BASE && owner < PMM_OWNER_SELFTEST;
}

static void clear_memory(struct process_memory *memory)
{
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++)
        memory->pages[i] = 0;
    memory->owner = 0;
    memory->acquired = 0;
}

int process_memory_check(const struct process_memory *memory)
{
    if (!memory) return PROCESS_MEMORY_E_ARGUMENT;
    if (!memory->acquired) {
        if (memory->owner) return PROCESS_MEMORY_E_CORRUPT;
        for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++)
            if (memory->pages[i]) return PROCESS_MEMORY_E_CORRUPT;
        return PROCESS_MEMORY_OK;
    }
    if (memory->acquired != PROCESS_MEMORY_PAGE_COUNT ||
        !process_owner(memory->owner))
        return PROCESS_MEMORY_E_CORRUPT;
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++) {
        if (!memory->pages[i] ||
            pmm_page_owner(memory->pages[i]) != memory->owner)
            return PROCESS_MEMORY_E_CORRUPT;
        for (unsigned int j = 0; j < i; j++)
            if (memory->pages[i] == memory->pages[j])
                return PROCESS_MEMORY_E_CORRUPT;
    }
    return PROCESS_MEMORY_OK;
}

int process_memory_ready(const struct process_memory *memory)
{
    return process_memory_check(memory) == PROCESS_MEMORY_OK &&
           memory && memory->acquired == PROCESS_MEMORY_PAGE_COUNT;
}

pmm_u64 process_memory_page(const struct process_memory *memory,
                            enum process_memory_page page)
{
    if (!process_memory_ready(memory) ||
        (unsigned int)page >= PROCESS_MEMORY_PAGE_COUNT)
        return 0;
    return memory->pages[(unsigned int)page];
}

int process_memory_acquire(struct process_memory *memory, unsigned int owner)
{
    if (!memory) return PROCESS_MEMORY_E_ARGUMENT;
    if (memory->owner || memory->acquired) return PROCESS_MEMORY_E_STATE;
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++)
        if (memory->pages[i]) return PROCESS_MEMORY_E_STATE;
    if (!process_owner(owner)) return PROCESS_MEMORY_E_OWNER;

    memory->owner = owner;
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++) {
        pmm_u64 page = pmm_alloc(owner);
        if (!page) {
            while (memory->acquired) {
                unsigned int index = --memory->acquired;
                if (pmm_release(memory->pages[index], owner) != PMM_OK)
                    return PROCESS_MEMORY_E_CORRUPT;
                memory->pages[index] = 0;
            }
            clear_memory(memory);
            return PROCESS_MEMORY_E_NOMEM;
        }
        memory->pages[i] = page;
        memory->acquired++;
    }
    return process_memory_check(memory);
}

int process_memory_release(struct process_memory *memory)
{
    if (!memory) return PROCESS_MEMORY_E_ARGUMENT;
    if (!memory->acquired) return PROCESS_MEMORY_E_STATE;
    if (process_memory_check(memory) != PROCESS_MEMORY_OK)
        return PROCESS_MEMORY_E_CORRUPT;

    while (memory->acquired) {
        unsigned int index = --memory->acquired;
        int status = pmm_release(memory->pages[index], memory->owner);
        if (status != PMM_OK) {
            memory->acquired++;
            return PROCESS_MEMORY_E_CORRUPT;
        }
        memory->pages[index] = 0;
    }
    clear_memory(memory);
    return PROCESS_MEMORY_OK;
}
