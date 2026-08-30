/* process_memory.h - failure-atomic physical frames for one process image. */
#ifndef ZLOS_PROCESS_MEMORY_H
#define ZLOS_PROCESS_MEMORY_H

#include "pmm.h"

#define PROCESS_MEMORY_PAGE_COUNT 8U
#define PROCESS_MEMORY_OWNER_BASE 32U

enum process_memory_page {
    PROCESS_MEMORY_PML4 = 0,
    PROCESS_MEMORY_PDPT,
    PROCESS_MEMORY_PD,
    PROCESS_MEMORY_PT,
    PROCESS_MEMORY_CODE,
    PROCESS_MEMORY_USER_STACK,
    PROCESS_MEMORY_KERNEL_STACK_LOW,
    PROCESS_MEMORY_KERNEL_STACK_HIGH
};

enum process_memory_status {
    PROCESS_MEMORY_OK = 0,
    PROCESS_MEMORY_E_ARGUMENT = -1,
    PROCESS_MEMORY_E_STATE = -2,
    PROCESS_MEMORY_E_OWNER = -3,
    PROCESS_MEMORY_E_NOMEM = -4,
    PROCESS_MEMORY_E_CORRUPT = -5
};

struct process_memory {
    pmm_u64 pages[PROCESS_MEMORY_PAGE_COUNT];
    unsigned int owner;
    unsigned int acquired;
};

int process_memory_acquire(struct process_memory *memory, unsigned int owner);
int process_memory_release(struct process_memory *memory);
int process_memory_ready(const struct process_memory *memory);
int process_memory_check(const struct process_memory *memory);
pmm_u64 process_memory_page(const struct process_memory *memory,
                            enum process_memory_page page);

#endif
