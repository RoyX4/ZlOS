#include <stdio.h>
#include <string.h>
#include "arch/x86/user_image64.h"

#define TEST_PAGES 24U
#define PRESENT 1ULL
#define WRITE 2ULL
#define USER 4ULL
#define NX (1ULL << 63)

static unsigned char backing[TEST_PAGES][PMM_PAGE_BYTES];
static unsigned int checks, failures;

void zl_putc_pub(char c) { putchar(c); }
void *pmm_host_page_pointer(pmm_u64 address)
{
    if (address < PMM_DYNAMIC_FLOOR) return NULL;
    pmm_u64 page = (address - PMM_DYNAMIC_FLOOR) / PMM_PAGE_BYTES;
    return page < TEST_PAGES ? backing[page] : NULL;
}

static void expect(int condition, const char *label)
{
    checks++;
    if (!condition) { failures++; printf("FAIL: %s\n", label); }
}

static void setup(unsigned int pages)
{
    struct pmm_range range = {
        PMM_DYNAMIC_FLOOR, (pmm_u64)pages * PMM_PAGE_BYTES, PMM_RANGE_USABLE
    };
    memset(backing, 0xa5, sizeof backing);
    expect(pmm_init_ranges(&range, 1) == PMM_OK, "initialize exact frame pool");
}

static pmm_u64 *page(const struct user_image64 *image,
                     enum process_memory_page role)
{
    return pmm_host_page_pointer(image->memory.pages[role]);
}

static void test_isolated_images(void)
{
    pmm_u64 root[USER_IMAGE64_ENTRIES] = {0};
    root[0] = 0x200003ULL; root[511] = 0x300003ULL;
    pmm_u64 root_before[USER_IMAGE64_ENTRIES];
    memcpy(root_before, root, sizeof root);
    const unsigned char code[] = {0xbb, 0x25, 0, 0, 0, 0x0f, 0x0b};
    struct user_image64 parent = {0}, child = {0};
    setup(TEST_PAGES);
    expect(user_image64_build(&parent, root, PROCESS_MEMORY_OWNER_BASE,
                              code, sizeof code) == USER_IMAGE64_OK,
           "parent image constructed");
    unsigned char parent_before[PROCESS_MEMORY_PAGE_COUNT][PMM_PAGE_BYTES];
    /* A parent's private anonymous mapping must never enter the child root. */
    page(&parent, PROCESS_MEMORY_PT)[USER_IMAGE64_ANON_FIRST] = 0x400007ULL;
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++)
        memcpy(parent_before[i], page(&parent, (enum process_memory_page)i),
               PMM_PAGE_BYTES);
    struct user_image64 parent_state = parent;
    expect(user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE + 1,
                              code, sizeof code) == USER_IMAGE64_OK,
           "child constructed from supervisor template");
    expect(!memcmp(&parent_state, &parent, sizeof parent),
           "child preparation leaves all parent image metadata intact");
    expect(!memcmp(root, root_before, sizeof root), "template stays immutable");
    for (unsigned int i = 0; i < PROCESS_MEMORY_PAGE_COUNT; i++) {
        expect(!memcmp(parent_before[i], page(&parent, (enum process_memory_page)i),
                       PMM_PAGE_BYTES), "every parent frame stays byte-identical");
        for (unsigned int j = 0; j < PROCESS_MEMORY_PAGE_COUNT; j++)
            expect(parent.memory.pages[i] != child.memory.pages[j],
                   "parent and child own disjoint frames");
    }
    pmm_u64 *pml4 = page(&child, PROCESS_MEMORY_PML4);
    pmm_u64 *pdpt = page(&child, PROCESS_MEMORY_PDPT);
    pmm_u64 *pd = page(&child, PROCESS_MEMORY_PD);
    pmm_u64 *pt = page(&child, PROCESS_MEMORY_PT);
    unsigned int slot = (unsigned int)(child.user_base >> 39);
    expect(slot == 1, "uses the first available canonical low-half slot");
    for (unsigned int i = 0; i < USER_IMAGE64_ENTRIES; i++)
        expect(pml4[i] == (i == slot ? child.memory.pages[PROCESS_MEMORY_PDPT] | 7ULL : root[i]),
               "only the child's own user root is present");
    expect(pdpt[0] == (child.memory.pages[PROCESS_MEMORY_PD] | 7ULL) &&
           pd[0] == (child.memory.pages[PROCESS_MEMORY_PT] | 7ULL),
           "intermediate tables reference the child's own frames");
    expect(pt[0] == (child.memory.pages[PROCESS_MEMORY_CODE] | PRESENT | USER),
           "code is readable/executable, never writable");
    expect(pt[2] == (child.memory.pages[PROCESS_MEMORY_USER_STACK] | 7ULL | NX),
           "user stack is writable and non-executable");
    expect(pt[1] == 0 && pt[3] == 0, "both lower stack guards are absent");
    expect(pt[4] == (child.memory.pages[PROCESS_MEMORY_KERNEL_STACK_LOW] | 3ULL | NX) &&
           pt[5] == (child.memory.pages[PROCESS_MEMORY_KERNEL_STACK_HIGH] | 3ULL | NX),
           "kernel stack is supervisor-only and non-executable");
    for (unsigned int i = USER_IMAGE64_ANON_FIRST; i < USER_IMAGE64_ENTRIES; i++)
        expect(pt[i] == 0, "no parent anonymous or trailing mapping inherited");
    unsigned char *image_code = (unsigned char *)page(&child, PROCESS_MEMORY_CODE);
    expect(!memcmp(image_code, code, sizeof code), "exact external bytes copied");
    for (unsigned int i = sizeof code; i < PMM_PAGE_BYTES; i++)
        expect(image_code[i] == 0xcc, "code tail traps instead of containing old bytes");
    expect(child.user_stack_top == child.user_base + 3ULL * PMM_PAGE_BYTES &&
           child.kernel_stack_top == child.user_base + 6ULL * PMM_PAGE_BYTES,
           "stack pointers match the guarded layout");
    expect(user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE + 1,
                              code, sizeof code) == USER_IMAGE64_E_STATE,
           "a live output cannot be overwritten");
    expect(user_image64_discard(&child) == USER_IMAGE64_OK &&
           pmm_used_pages() == PROCESS_MEMORY_PAGE_COUNT,
           "discarding child preserves all parent frame custody");
    expect(user_image64_discard(&parent) == USER_IMAGE64_OK &&
           pmm_used_pages() == 0 && pmm_check() == 0,
           "discard restores exact allocator baseline");
}

static void test_failure_boundaries(void)
{
    const unsigned char code[] = {0x0f, 0x0b};
    pmm_u64 root[USER_IMAGE64_ENTRIES] = {0};
    root[0] = 0x200003ULL;
    for (unsigned int available = 0; available < PROCESS_MEMORY_PAGE_COUNT; available++) {
        setup(PROCESS_MEMORY_PAGE_COUNT + available);
        struct user_image64 parent = {0}, child = {0}, empty = {0};
        expect(user_image64_build(&parent, root, PROCESS_MEMORY_OWNER_BASE,
                                  code, sizeof code) == USER_IMAGE64_OK,
               "failure fixture parent remains live");
        struct user_image64 before = parent;
        expect(user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE + 1,
                                  code, sizeof code) == USER_IMAGE64_E_NOMEM,
               "each child allocation boundary fails explicitly");
        expect(!memcmp(&child, &empty, sizeof child) &&
               !memcmp(&before, &parent, sizeof parent),
               "failed preparation preserves output and parent metadata");
        expect(pmm_used_pages() == PROCESS_MEMORY_PAGE_COUNT &&
               pmm_free_pages() == available && pmm_check() == 0,
               "failed preparation restores exact live-parent accounting");
        expect(user_image64_discard(&parent) == USER_IMAGE64_OK,
               "parent remains reclaimable after failed child preparation");
    }
    setup(TEST_PAGES);
    struct user_image64 child = {0}, empty = {0};
    root[1] = 0x400007ULL;
    expect(user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE,
                              code, sizeof code) == USER_IMAGE64_E_TEMPLATE,
           "parent-contaminated user root rejected before allocation");
    root[1] = 0;
    for (unsigned int i = 1; i < 255; i++) root[i] = 0x400003ULL;
    expect(user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE,
                              code, sizeof code) == USER_IMAGE64_E_TEMPLATE,
           "no free low-half root slot rejected before allocation");
    expect(user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE,
                              code, 0) == USER_IMAGE64_E_ARGUMENT &&
           user_image64_build(&child, root, PROCESS_MEMORY_OWNER_BASE,
                              code, PMM_PAGE_BYTES + 1) == USER_IMAGE64_E_ARGUMENT,
           "empty and oversized raw images rejected");
    expect(!memcmp(&child, &empty, sizeof child) && pmm_used_pages() == 0,
           "admission errors leave output and allocator untouched");
}

int main(void)
{
    test_isolated_images();
    test_failure_boundaries();
    printf("userimage64test: %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
