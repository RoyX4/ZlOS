/* paging.c - the kernel's own virtual memory, one window at a time.
 *
 * WHAT THIS DOES, AND WHAT IT REFUSES TO DO
 * =========================================
 * zlOS has run identity-mapped for its whole life. That is *why* every
 * multi-megabyte buffer in memmap.h is a hand-picked physical address: with
 * virtual == physical there is nowhere else to put anything, so the map is a
 * list of constants that people have collided five times.
 *
 * This file breaks that, in the smallest increment that is actually verifiable:
 * it maps ONE region - the heap - at a virtual address that is NOT its physical
 * address, and leaves absolutely everything else identity-mapped underneath.
 *
 * The heap is the right first region and the choice is not arbitrary:
 *
 *   - Nothing inside it has a fixed address, so no other file names one.
 *   - NO DEVICE IS EVER GIVEN A POINTER INTO IT. heap.c is not in
 *     check-dma.sh's DMA set, and docs/dma-sites.md enumerates every address
 *     that reaches hardware - none of them is a heap pointer. So the one
 *     failure mode that is silent and unrecoverable - a device handed a
 *     virtual address - cannot arise from this window.
 *   - If the window fails to materialise, falling back to the physical address
 *     is a complete, correct, already-tested system. There is no half state.
 *
 * WHERE THE WINDOW GOES
 * =====================
 * At a PML4 slot that is currently ABSENT, found by scanning 255 downward, with
 * our own PDPT and page directory hung off it. 512 GiB of address space per
 * slot; we use 64 MiB of it.
 *
 * IT TOOK TWO WRONG ANSWERS TO GET HERE and both were the same mistake -
 * reasoning about FIRMWARE's page tables from OUR bootloader's:
 *
 *   1. hardcoded PDPT slot 4 (virtual 4 GiB), because boot64.S fills PDPT[0..3]
 *      and leaves the rest zero;
 *   2. scanned PDPT slots 4..511 for a free one.
 *
 * Both refused on the EFI path, every boot, and the first version refused
 * SILENTLY - the boot line read "identity only - no window", which is exactly
 * what a build with no paging says, so it looked intentional. Making every
 * refusal name its reason is what produced the actual answer:
 *
 *     vmm: refused - every PDPT slot from 4 to 511 is already mapped
 *
 * OVMF identity-maps its whole address space with 1 GiB pages, so under PML4[0]
 * there is no free PDPT slot at all. One level up there is room to spare.
 *
 * HOW IT REFUSES
 * ==============
 * This is the file where "it looked right" is worth the least, so nothing here
 * is trusted:
 *
 *   1. The walk is VALIDATED at every step - CR3 sane, PML4[0] present and not
 *      a huge page, PDPT slot currently ABSENT. Any surprise aborts before a
 *      single byte is written.
 *   2. The entries are READ BACK after writing, through the identity mapping,
 *      and compared with what we meant to write. This is fault-free: it
 *      dereferences no new address.
 *   3. Only then is the new virtual address touched at all, and the probe
 *      writes through VIRTUAL and reads back at PHYSICAL. That is the check
 *      that catches the one dangerous outcome - a window that works but aliases
 *      the WRONG memory. A mapping that is merely absent faults instead, and
 *      idt.c installs fault_isr on all 32 exception vectors, so that halts with
 *      a message rather than triple-faulting into a reboot loop.
 *   4. On any failure the PDPT entry is put back to zero, CR3 is reloaded, and
 *      the caller is told to use the physical address. Loudly.
 *
 * WHAT IS STILL IDENTITY, DELIBERATELY
 * ====================================
 * Every region a DEVICE can reach - HI_XHCI, HI_NVME, HI_VGPU, virtio-net's
 * arena - stays identity-mapped, and there is no plan to change that. A driver
 * handed a physical address today and a physical address tomorrow is a driver
 * that does not have to change at all, and DMA is where this project's
 * recurring bug class lives. dma_addr() exists so that if that ever does
 * change, it changes in one place; it is not a promise that it will.
 *
 * The 32-bit build has paging OFF entirely (CR0.PG clear, flat segmentation),
 * so there is nothing to extend and vmm_map_window() says so and returns. Turning
 * paging on there means building tables for a machine that currently has none,
 * which is a much larger change than this one and buys nothing the 64-bit path
 * does not already demonstrate.
 */

#include "memmap.h"

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned char      u8;

#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__   uptr;
#else
typedef unsigned long      uptr;
#endif

void zl_putc_pub(char c);

static void vp(const char *s) { while (*s) zl_putc_pub(*s++); }

static void vpu(unsigned long long v)
{
    char b[24];
    int i = 0;
    if (!v) { zl_putc_pub('0'); return; }
    while (v) { b[i++] = (char)('0' + (int)(v % 10ULL)); v /= 10ULL; }
    while (i) zl_putc_pub(b[--i]);
}

/* ---- the window, which is the entire state of this file -------------------
 * win_bytes == 0 means "no window": every translation is the identity. Kept as
 * three plain integers rather than a struct so that vmm_phys() below is
 * obviously a range check and nothing else - it is called from dma_addr(), on
 * the path of every keystroke and every disk block, and it is the function that
 * must never be subtly wrong. */
static u64 win_virt  = 0;
static u64 win_phys  = 0;
static u64 win_bytes = 0;
static int vmm_live  = 0;

/* ---- the translation, and its inverse -------------------------------------
 * These two are the whole public contract and they are deliberately trivial.
 * hosttest/pagingtest.c drives them directly - the arithmetic is testable on a
 * Linux host even though the mapping is not. */
unsigned long long vmm_phys(unsigned long long virt)
{
    if (win_bytes && virt >= win_virt && virt - win_virt < win_bytes)
        return win_phys + (virt - win_virt);
    return virt;
}

unsigned long long vmm_virt(unsigned long long phys)
{
    if (win_bytes && phys >= win_phys && phys - win_phys < win_bytes)
        return win_virt + (phys - win_phys);
    return phys;
}

int vmm_active(void)                    { return vmm_live; }
unsigned long long vmm_window_virt(void)  { return win_virt; }
unsigned long long vmm_window_phys(void)  { return win_phys; }
unsigned long long vmm_window_bytes(void) { return win_bytes; }

/* Set the window WITHOUT touching any page table. This exists so the host test
 * can exercise vmm_phys/vmm_virt over the same code the kernel runs, and so a
 * failed mapping can put the state back with one call. Not exported to zl. */
void vmm_set_window(unsigned long long v, unsigned long long p, unsigned long long b)
{
    win_virt = v; win_phys = p; win_bytes = b;
}

#if defined(ZL_64)

/* ---- x86-64 paging constants ---------------------------------------------*/
#define PTE_P     (1ULL << 0)      /* present                                */
#define PTE_W     (1ULL << 1)      /* writable                               */
#define PTE_PS    (1ULL << 7)      /* page size: this entry IS the page      */
#define ADDR_MASK 0x000FFFFFFFFFF000ULL

/* THE SLOT IS SEARCHED FOR, NOT HARDCODED, and that is a correction rather than
 * a refinement. The first version took PDPT slot 4 - virtual 4..5 GiB - on the
 * reasoning that boot64.S fills 0..3 and leaves the rest zero. True for the
 * multiboot build; FALSE under OVMF, which maps well past 4 GiB itself. So on
 * the EFI path the "is this slot free" check correctly refused, every time, and
 * the window was never once installed on the only 64-bit path that has a boot
 * gate. Measured, not guessed: the boot log said
 *
 *     vmm: identity only - no window (the heap is at its physical address)
 *
 * The refusal was right. Hardcoding the slot was not. Scanning from the TOP
 * down finds a slot firmware has no reason to have touched, and the search
 * itself is cheap and bounded - 508 reads of a page we already have mapped. */
/* WE TAKE A PML4 SLOT, NOT A PDPT SLOT, AND THAT IS THE SECOND CORRECTION.
 *
 * Attempt 1 hardcoded PDPT slot 4 (virtual 4 GiB), reasoning from boot64.S,
 * which fills PDPT[0..3] and leaves the rest zero. Attempt 2 scanned PDPT
 * slots 4..511 for a free one. Both refused on the EFI path, and once the
 * refusals were made to explain themselves the boot log said why:
 *
 *     vmm: refused - every PDPT slot from 4 to 511 is already mapped
 *
 * OVMF identity-maps its whole address space with 1 GiB pages, so under
 * PML4[0] there is no free PDPT slot at all. Reasoning about firmware's page
 * tables from OUR bootloader's page tables was the mistake, twice.
 *
 * One level up there is room: PML4[1..255] each cover 512 GiB, and firmware has
 * no reason to have populated them. So we bring our own PDPT as well as our own
 * PD, hang it off a free PML4 slot, and get a window at (slot << 39).
 *
 * Staying below 256 keeps bit 47 clear, so the address is canonical without
 * sign extension - the upper half would need 0xFFFF prefixes and is where a
 * "higher half kernel" would live, which this is not. */
#define PML4_FIRST  1              /* 0 is everyone else's                    */
#define PML4_LAST   255            /* below 256: canonical without sign-extend */
#define SLOT_VIRT(s) ((u64)(s) << 39)
#define PD_ENTRIES  512            /* x 2 MiB = 1 GiB, the most one PD covers */
#define TWO_MIB     0x200000ULL

/* Our page directory. In .bss, which both 64-bit paths identity-map, so its
 * ADDRESS is its physical address - the assumption is stated because it is the
 * one thing here that is inherited rather than checked. On the multiboot path
 * the kernel is linked and loaded at 1 MiB; on the EFI path firmware chose the
 * address but efi.c records that UEFI's tables are identity, which is also why
 * every driver's fixed physical addresses still work after ExitBootServices. */
static u64 win_pd[PD_ENTRIES]   __attribute__((aligned(4096)));
static u64 win_pdpt[PD_ENTRIES] __attribute__((aligned(4096)));

static u64 rd_cr3(void) { u64 v; __asm__ volatile("mov %%cr3, %0" : "=r"(v)); return v; }
static void wr_cr3(u64 v) { __asm__ volatile("mov %0, %%cr3" :: "r"(v) : "memory"); }
static u64 rd_cr0(void) { u64 v; __asm__ volatile("mov %%cr0, %0" : "=r"(v)); return v; }
static void wr_cr0(u64 v) { __asm__ volatile("mov %0, %%cr0" :: "r"(v) : "memory"); }

#define CR0_WP (1ULL << 16)

/* ---- the probe ------------------------------------------------------------
 * Write through VIRTUAL, read back at PHYSICAL. This is the only check that
 * catches a window which works but points at the wrong memory, and that is the
 * outcome worth catching: a window that is merely absent faults on the first
 * touch and halts, which is loud. Two points, because a mapping wrong by one
 * 2 MiB page would still alias correctly at offset zero.
 *
 * Everything is saved and restored: heap.c has not initialised yet, but this
 * region is also probed by heap.c's own ram_backed(), and a probe that leaves
 * litter behind makes the second probe's result depend on the first. */
static int alias_ok(u64 virt, u64 phys, u64 bytes)
{
    volatile u32 *v_lo = (volatile u32 *)(uptr)virt;
    volatile u32 *p_lo = (volatile u32 *)(uptr)phys;
    volatile u32 *v_hi = (volatile u32 *)(uptr)(virt + bytes - 4);
    volatile u32 *p_hi = (volatile u32 *)(uptr)(phys + bytes - 4);
    u32 s_lo = *p_lo, s_hi = *p_hi;
    int ok;

    *v_lo = 0x5EED10ADu;
    *v_hi = 0xFEEDFACEu;
    ok = (*p_lo == 0x5EED10ADu) && (*p_hi == 0xFEEDFACEu);

    /* ...and the other direction, which catches a window that happens to alias
     * because BOTH addresses land on unbacked RAM that reads back what was
     * written by luck of a write buffer. */
    if (ok) {
        *p_lo = 0x0DDBA11Du;
        ok = (*v_lo == 0x0DDBA11Du);
    }

    *p_lo = s_lo; *p_hi = s_hi;
    return ok;
}

/* ---- install ---------------------------------------------------------------
 * Returns the virtual base, or 0 if the caller should use `phys` unchanged.
 */
unsigned long long vmm_map_window(unsigned long long phys, unsigned long long bytes)
{
    u64 cr3, *pml4, want, cr0, virt;
    u64 pages, i;
    int slot;

    if (vmm_live) return win_virt;              /* one window, once */

    /* EVERY REFUSAL BELOW SAYS WHY. The first version returned 0 silently from
     * the five validation checks, and the one that fired on the EFI path -
     * "that slot is taken" - was indistinguishable from "this build has no
     * paging". A refusal whose reason is invisible is how the window came to be
     * dead on the only path that could test it, with a boot line that read as
     * intentional. This project has shipped a silent fallback twice already. */

    /* ---- 1. can we even express this? ----------------------------------- */
    if (!bytes || bytes > (u64)PD_ENTRIES * TWO_MIB) {
        vp("  vmm: refused - the region is larger than one page directory\n");
        return 0;
    }
    if ((phys & (TWO_MIB - 1)) || (bytes & (TWO_MIB - 1))) {
        vp("  vmm: refused - base or size is not 2 MiB aligned\n");
        return 0;
    }

    /* ---- 2. walk, validating every step --------------------------------- */
    cr3 = rd_cr3();
    if (!(cr3 & ADDR_MASK)) { vp("  vmm: refused - CR3 is not a page table\n"); return 0; }
    pml4 = (u64 *)(uptr)(cr3 & ADDR_MASK);

    /* ---- 3. FIND a free PML4 slot ---------------------------------------
     * An occupied one is SKIPPED, never overwritten - taking one would unmap
     * 512 GiB of somebody else's address space, which is the one mistake here
     * that would be silent. */
    slot = -1;
    for (i = PML4_LAST; i >= PML4_FIRST; i--) {
        if (!(pml4[i] & PTE_P)) { slot = (int)i; break; }
    }
    if (slot < 0) {
        vp("  vmm: refused - PML4 slots 1..255 are all in use\n");
        return 0;
    }
    virt = SLOT_VIRT(slot);

    /* ---- 4. build our OWN PDPT and PD, in memory we own ------------------
     * Both are in .bss, which every 64-bit path identity-maps, so their
     * addresses are their physical addresses. That is the one assumption here
     * that is inherited rather than checked - and it is the same assumption
     * every fixed physical address in memmap.h already rests on. */
    for (i = 0; i < PD_ENTRIES; i++) { win_pd[i] = 0; win_pdpt[i] = 0; }
    pages = bytes / TWO_MIB;
    for (i = 0; i < pages; i++)
        win_pd[i] = (phys + i * TWO_MIB) | PTE_P | PTE_W | PTE_PS;

    win_pdpt[0] = ((u64)(uptr)win_pd) | PTE_P | PTE_W;
    want = ((u64)(uptr)win_pdpt) | PTE_P | PTE_W;

    /* ---- 4. install, with WP down --------------------------------------- *
     * The PDPT may be firmware's, and firmware commonly marks its own page
     * tables read-only. A supervisor write to a read-only page faults when
     * CR0.WP is set; clearing it for the duration is the standard, documented
     * way to do this and is exactly what Linux does to patch its own text.
     * Restored immediately, whatever happens next. */
    cr0 = rd_cr0();
    wr_cr0(cr0 & ~CR0_WP);
    pml4[slot] = want;
    wr_cr0(cr0);

    wr_cr3(cr3);                                /* flush the TLB */

    /* ---- 5. read back what we wrote, WITHOUT dereferencing the window --- */
    if (pml4[slot] != want || win_pd[0] != (phys | PTE_P | PTE_W | PTE_PS)) {
        cr0 = rd_cr0(); wr_cr0(cr0 & ~CR0_WP);
        pml4[slot] = 0;
        wr_cr0(cr0);
        wr_cr3(cr3);
        vp("  vmm: the page-directory entry did not read back - window refused\n");
        return 0;
    }

    /* ---- 6. only now, touch it ------------------------------------------ */
    if (!alias_ok(virt, phys, bytes)) {
        cr0 = rd_cr0(); wr_cr0(cr0 & ~CR0_WP);
        pml4[slot] = 0;
        wr_cr0(cr0);
        wr_cr3(cr3);
        vp("  vmm: the window did NOT alias its physical region - refused\n");
        return 0;
    }

    vmm_set_window(virt, phys, bytes);
    vmm_live = 1;
    return virt;
}

#else  /* !ZL_64 */

/* The 32-bit build runs with paging OFF - CR0.PG is never set, and boot.S/
 * raw_entry.S set up flat segmentation and nothing else. There are no page
 * tables to extend. Saying so and returning is not a stub: it is the correct
 * answer for this target, and the caller's fallback is the system that has
 * always run here. */
unsigned long long vmm_map_window(unsigned long long phys, unsigned long long bytes)
{
    (void)phys; (void)bytes;
    return 0;
}

#endif /* ZL_64 */

/* Called at boot after the heap knows what it wants. Prints one line with
 * ADDRESSES in it, the way fb.c and arena.c do, because "virtual memory is on"
 * is a claim and "64 MiB: virtual 4096 MiB -> physical 256 MiB" is a fact
 * somebody can check. */
void vmm_report(void)
{
    vp("  vmm: ");
    if (!vmm_live) {
#if defined(ZL_64)
        vp("identity only - no window (the heap is at its physical address)\n");
#else
        vp("paging is off on this build - identity, by construction\n");
#endif
        return;
    }
    /* GiB for the virtual side, MiB for the physical. A PML4 slot is 512 GiB
     * wide, so the virtual base in MiB is an eight-digit number nobody can
     * check at a glance - and the whole point of printing an address rather
     * than a claim is that somebody CAN check it. */
    vpu(win_bytes >> 20);
    vp(" MiB mapped: virtual ");
    vpu(win_virt >> 30);
    vp(" GiB -> physical ");
    vpu(win_phys >> 20);
    vp(" MiB  (everything else identity)\n");
}
