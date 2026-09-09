#include "user_image64.h"

#define PRESENT 1ULL
#define WRITE 2ULL
#define USER 4ULL
#define NX (1ULL << 63)

static void *page_pointer(pmm_u64 physical)
{
#if defined(PMM_HOSTTEST)
    extern void *pmm_host_page_pointer(pmm_u64 address);
    return pmm_host_page_pointer(physical);
#else
    return (void *)(__UINTPTR_TYPE__)physical;
#endif
}

static int empty_image(const struct user_image64 *image)
{
    return image && !image->memory.acquired &&
           process_memory_check(&image->memory) == PROCESS_MEMORY_OK &&
           !image->cr3 && !image->user_base && !image->user_stack_top &&
           !image->kernel_stack_top;
}

int user_image64_discard(struct user_image64 *image)
{
    if (!image) return USER_IMAGE64_E_ARGUMENT;
    if (process_memory_release(&image->memory) != PROCESS_MEMORY_OK)
        return USER_IMAGE64_E_CORRUPT;
    *image = (struct user_image64){0};
    return USER_IMAGE64_OK;
}

int user_image64_build(struct user_image64 *output,
                       const pmm_u64 kernel_root[USER_IMAGE64_ENTRIES],
                       unsigned int owner, const unsigned char *code,
                       unsigned int bytes)
{
    if (!output || !kernel_root || !code || !bytes || bytes > PMM_PAGE_BYTES ||
        owner < PROCESS_MEMORY_OWNER_BASE || owner >= PMM_OWNER_SELFTEST)
        return USER_IMAGE64_E_ARGUMENT;
    if (!empty_image(output)) return USER_IMAGE64_E_STATE;
    int slot = -1;
    for (unsigned int i = 0; i < USER_IMAGE64_ENTRIES; i++) {
        if ((kernel_root[i] & (PRESENT | USER)) == (PRESENT | USER))
            return USER_IMAGE64_E_TEMPLATE;
        if (i > 0 && i < 255 && !(kernel_root[i] & PRESENT) && slot < 0)
            slot = (int)i;
    }
    if (slot < 0) return USER_IMAGE64_E_TEMPLATE;

    struct user_image64 next = {0};
    int status = process_memory_acquire(&next.memory, owner);
    if (status != PROCESS_MEMORY_OK) {
        if (status == PROCESS_MEMORY_E_NOMEM) return USER_IMAGE64_E_NOMEM;
        *output = next; /* Retain any corrupt allocation's custody. */
        return USER_IMAGE64_E_CORRUPT;
    }
    void *pages[PROCESS_MEMORY_PAGE_COUNT];
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++) {
        pages[i] = page_pointer(next.memory.pages[i]);
        if (!pages[i]) {
            if (user_image64_discard(&next) != USER_IMAGE64_OK) *output = next;
            return USER_IMAGE64_E_CORRUPT;
        }
    }
    pmm_u64 *pml4 = pages[PROCESS_MEMORY_PML4];
    pmm_u64 *pdpt = pages[PROCESS_MEMORY_PDPT];
    pmm_u64 *pd = pages[PROCESS_MEMORY_PD];
    pmm_u64 *pt = pages[PROCESS_MEMORY_PT];
    for (unsigned int i = 0; i < USER_IMAGE64_ENTRIES; i++) {
        pml4[i] = kernel_root[i];
        pdpt[i] = pd[i] = pt[i] = 0;
    }
    pml4[slot] = next.memory.pages[PROCESS_MEMORY_PDPT] | PRESENT | WRITE | USER;
    pdpt[0] = next.memory.pages[PROCESS_MEMORY_PD] | PRESENT | WRITE | USER;
    pd[0] = next.memory.pages[PROCESS_MEMORY_PT] | PRESENT | WRITE | USER;
    pt[0] = next.memory.pages[PROCESS_MEMORY_CODE] | PRESENT | USER;
    pt[2] = next.memory.pages[PROCESS_MEMORY_USER_STACK] | PRESENT | WRITE | USER | NX;
    /* PTE 1 and PTE 3 are absent lower guards. All anonymous PTEs stay absent. */
    pt[4] = next.memory.pages[PROCESS_MEMORY_KERNEL_STACK_LOW] | PRESENT | WRITE | NX;
    pt[5] = next.memory.pages[PROCESS_MEMORY_KERNEL_STACK_HIGH] | PRESENT | WRITE | NX;
    unsigned char *image_code = pages[PROCESS_MEMORY_CODE];
    unsigned char *stack = pages[PROCESS_MEMORY_USER_STACK];
    unsigned char *low = pages[PROCESS_MEMORY_KERNEL_STACK_LOW];
    unsigned char *high = pages[PROCESS_MEMORY_KERNEL_STACK_HIGH];
    for (unsigned int i = 0; i < PMM_PAGE_BYTES; i++) {
        image_code[i] = i < bytes ? code[i] : 0xcc;
        stack[i] = 0;
        low[i] = high[i] = USER_IMAGE64_STACK_FILL;
    }
    next.cr3 = next.memory.pages[PROCESS_MEMORY_PML4];
    next.user_base = (pmm_u64)(unsigned int)slot << 39;
    next.user_stack_top = next.user_base + 3ULL * PMM_PAGE_BYTES;
    next.kernel_stack_top = next.user_base + 6ULL * PMM_PAGE_BYTES;
    *output = next;
    return USER_IMAGE64_OK;
}
