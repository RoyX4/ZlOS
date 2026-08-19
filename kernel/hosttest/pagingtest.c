/* pagingtest.c - paging.c's translation arithmetic, on a Linux host.
 *
 * WHAT CAN AND CANNOT BE TESTED HERE, said first because the split is the whole
 * design of this file.
 *
 * CANNOT: the mapping itself. Installing a PDPT entry needs CR3, ring 0 and a
 * page directory at a known physical address. A Linux process has none of those.
 * The only proof that vmm_map_window() works is verify-efi.sh booting green with
 * it, plus the three refusal paths inside it that put the entry back.
 *
 * CAN, and this is the half that runs on EVERY DMA site: vmm_phys() and
 * vmm_virt(). Those two are called from dma_addr()/dma_kaddr(), which means
 * every keystroke, every mouse report, every disk block and every network frame
 * goes through them. They are twelve lines of range arithmetic, they are
 * compiled into all four targets, and a boundary error in them would hand a
 * device an address one page out - the exact silent corruption this whole stage
 * exists to prevent.
 *
 * So paging.c exposes vmm_set_window(), which sets the three numbers WITHOUT
 * touching a page table, and this file drives the arithmetic across every
 * boundary it has. Off-by-one at the top of the window is the bug this is
 * looking for: `virt - win_virt < win_bytes` is right and `virt <= win_virt +
 * win_bytes` is wrong, and both look fine.
 */
#include <stdio.h>

unsigned long long vmm_phys(unsigned long long virt);
unsigned long long vmm_virt(unsigned long long phys);
void vmm_set_window(unsigned long long v, unsigned long long p, unsigned long long b);
int  vmm_active(void);
unsigned long long vmm_window_virt(void);
unsigned long long vmm_window_phys(void);
unsigned long long vmm_window_bytes(void);
void vmm_report(void);

void zl_putc_pub(char c) { fputc(c, stdout); }

static int checks = 0, failures = 0;

static void eq(unsigned long long got, unsigned long long want, const char *what)
{
    checks++;
    if (got != want) {
        failures++;
        printf("  FAIL  %s: got 0x%llX, wanted 0x%llX\n", what, got, want);
    }
}

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) { failures++; printf("  FAIL  %s\n", what); }
}

#define VIRT  0x0000000100000000ULL      /* 4 GiB   - what paging.c picks   */
#define PHYS  0x0000000010000000ULL      /* 256 MiB - HI_HEAP               */
#define SIZE  0x0000000004000000ULL      /* 64 MiB                          */

int main(void)
{
    printf("pagingtest - paging.c's translation arithmetic, unmodified\n\n");

    /* ---- with NO window, everything is the identity ---------------------
     * This is the 32-bit build, and it is also what every failure path inside
     * vmm_map_window() leaves behind. It has to be exactly the identity, not
     * approximately: it is what all 48 DMA sites get today. */
    vmm_set_window(0, 0, 0);
    eq(vmm_phys(0), 0, "no window: 0");
    eq(vmm_phys(PHYS), PHYS, "no window: the heap's physical base");
    eq(vmm_phys(VIRT), VIRT, "no window: what would be the virtual base");
    eq(vmm_phys(0xFFFFFFFFFFFFFFFFULL), 0xFFFFFFFFFFFFFFFFULL, "no window: ~0");
    eq(vmm_virt(PHYS), PHYS, "no window, inverse");
    ok(vmm_active() == 0, "no window is not active");

    /* ---- with a window ---------------------------------------------------*/
    vmm_set_window(VIRT, PHYS, SIZE);
    eq(vmm_window_virt(), VIRT, "window virt readback");
    eq(vmm_window_phys(), PHYS, "window phys readback");
    eq(vmm_window_bytes(), SIZE, "window size readback");

    eq(vmm_phys(VIRT), PHYS, "the very first byte");
    eq(vmm_phys(VIRT + 1), PHYS + 1, "one byte in");
    eq(vmm_phys(VIRT + SIZE / 2), PHYS + SIZE / 2, "the middle");
    eq(vmm_phys(VIRT + SIZE - 1), PHYS + SIZE - 1, "THE LAST BYTE IN THE WINDOW");

    /* THE BOUNDARY, which is the entire point of this file. One past the end
     * must NOT translate - if it does, the range test was written as <= and
     * every allocation at the very top of the heap hands out an address one
     * byte outside the mapping. */
    eq(vmm_phys(VIRT + SIZE), VIRT + SIZE, "one past the end is NOT translated");
    eq(vmm_phys(VIRT - 1), VIRT - 1, "one before the start is NOT translated");

    /* Physical addresses must pass through untouched. This is what every DMA
     * site relies on: the device buffers are at 224-256 MiB, nowhere near the
     * window, and they must come back exactly as handed in. */
    eq(vmm_phys(0x0E000000ULL), 0x0E000000ULL, "the xHCI arena is untouched");
    eq(vmm_phys(0x0F000000ULL), 0x0F000000ULL, "the virtio-gpu arena is untouched");
    eq(vmm_phys(0x04000000ULL), 0x04000000ULL, "the virtio-net arena is untouched");
    eq(vmm_phys(0x0D000000ULL), 0x0D000000ULL, "the NVMe arena is untouched");

    /* THE HEAP'S PHYSICAL RANGE IS THE ONE THAT MUST NOT ROUND-TRIP THROUGH
     * vmm_phys. A physical address inside the window's PHYSICAL span is not a
     * virtual address, and translating it would be wrong. */
    eq(vmm_phys(PHYS), PHYS, "the heap's PHYSICAL base is not itself translated");

    /* ---- the inverse ----------------------------------------------------*/
    eq(vmm_virt(PHYS), VIRT, "inverse: first byte");
    eq(vmm_virt(PHYS + SIZE - 1), VIRT + SIZE - 1, "inverse: last byte");
    eq(vmm_virt(PHYS + SIZE), PHYS + SIZE, "inverse: one past the end untouched");
    eq(vmm_virt(PHYS - 1), PHYS - 1, "inverse: one before the start untouched");

    /* Round trip, across the whole window at 4 KiB steps plus both edges. A
     * single spot check would miss an error that only shows in one 2 MiB page. */
    {
        unsigned long long v;
        int bad = 0;
        for (v = VIRT; v < VIRT + SIZE; v += 4096)
            if (vmm_virt(vmm_phys(v)) != v) { bad = 1; break; }
        if (vmm_virt(vmm_phys(VIRT + SIZE - 1)) != VIRT + SIZE - 1) bad = 1;
        ok(!bad, "virt->phys->virt round trips across all 16384 pages");
    }

    /* ---- and a window somewhere else entirely ---------------------------
     * paging.c hardcodes 4 GiB today. The arithmetic must not. */
    vmm_set_window(0x0000800000000000ULL, 0x0000000020000000ULL, 0x00200000ULL);
    eq(vmm_phys(0x0000800000000000ULL), 0x0000000020000000ULL, "a different window");
    eq(vmm_phys(0x00008000001FFFFFULL), 0x00000000201FFFFFULL, "its last byte");
    eq(vmm_phys(0x0000800000200000ULL), 0x0000800000200000ULL, "one past it");

    vmm_set_window(0, 0, 0);
    printf("\n");
    vmm_report();

    printf("\n%d checks, %d failures\n", checks, failures);
    if (failures) { printf("FAIL  the translation is wrong\n"); return 1; }
    printf("ok    virt<->phys is exact at every boundary\n");
    return 0;
}
