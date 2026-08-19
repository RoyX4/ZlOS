/* dma.h - the one place a kernel address becomes a device address.
 *
 * FOR EVERY ADDRESS A DEVICE IS ACTUALLY GIVEN, THIS RETURNS WHAT IT WAS GIVEN
 * - and that is now a CHECKED fact rather than a structural one.
 *
 * zlOS is identity-mapped almost everywhere: the 32-bit build has paging off
 * entirely, the 64-bit build identity-maps the first 4 GiB with 2 MiB pages
 * (boot64.S:16-19), and the EFI build runs on the page tables UEFI left behind,
 * which are also identity (efi.c:288). The ONE exception is paging.c's window
 * over the heap, and no device is ever handed a heap pointer - heap.c is
 * deliberately outside check-dma.sh's DMA set. So dma_addr() consults the
 * window, finds every device address outside it, and returns the identity.
 *
 * WHY WRITE IT THEN
 * -----------------
 * Because the moment any of that stops being true, every address handed to a
 * device is wrong, and it is wrong in the worst available way:
 *
 *   - nothing faults. The device is given a number and writes to it.
 *   - nothing logs. There is no MMU between the device and the memory.
 *   - the corruption lands wherever that number happens to point, which is
 *     some other subsystem's buffer, and it surfaces days later.
 *
 * HANDOFF.md already counts this bug class SIX times under "check this FIRST",
 * every time as a symptom that read like a protocol bug. This is the seventh
 * waiting to happen, and unlike the six it would arrive all at once, in every
 * driver, on the commit that switches paging on.
 *
 * So the seam went in FIRST, while it was an identity function and the change
 * was provably behaviour-preserving - the boot gates green before and green
 * after, the diff mechanical. paging.c's window landed second, on top of a seam
 * that was already everywhere. Mapping a region a device DOES reach is now a
 * change to one function instead of an archaeology exercise across four
 * drivers, and there is no plan to do it: see paging.c, "what is still
 * identity, deliberately".
 *
 * `docs/dma-sites.md` is the inventory this covers, with the argument for why
 * that inventory is closed rather than merely long. `check-dma.sh` is what stops
 * a new site being added that skips this.
 *
 * WHAT IT IS NOT FOR
 * ------------------
 * Three things look like DMA and are not. Do NOT wrap them:
 *
 *   1. MMIO register offsets. `mmio + GGTT_OFFSET` is an address in the
 *      DEVICE's BAR, not in our RAM. Nothing translates it.
 *   2. GGTT graphics addresses - `gpuring.c`'s REG_START, `intel.c`'s
 *      PLANE_SURF. Those are indices into the GPU's own page table, which is a
 *      second translation layer the CPU's page tables have no effect on.
 *      intel.c's ggtt_map() is where a real physical address enters THAT
 *      table, and it is the one site that must keep taking a physical address
 *      no matter what the CPU is doing.
 *   3. Anything only the CPU touches - i2c_hid.c's buffers, fb.c's back
 *      buffer, the task stacks. A pointer nobody hands to an engine needs no
 *      translation.
 */
#ifndef ZL_DMA_H
#define ZL_DMA_H

/* Same reasoning as arena.c:95 and heap.c: __UINTPTR_TYPE__ is a predefined
 * macro, available freestanding, and it is correct on the 32-bit kernel, the
 * 64-bit kernel and the LLP64 EFI target alike. A hand-rolled `unsigned long`
 * would be four bytes on one of those three - the exact defect CLAUDE.md opens
 * with, in the exact file whose job is address arithmetic. */
#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__   dma_uptr;
#else
typedef unsigned long long dma_uptr;
#endif

/* Returns ALWAYS 64-bit, whatever the build. Every caller writes this into a
 * device register or descriptor field that is 64 bits wide even on the 32-bit
 * build - xHCI's DCBAAP, NVMe's PRP1, virtio's queue bases are all 64-bit
 * fields - so narrowing here and widening at the call site is how the top half
 * ends up holding whatever was in the register before. */
/* paging.c. NOT identity any more - it is a range check against the one window
 * the kernel maps, and the identity is what it returns for everything outside
 * that window, which today is every address any device is ever given.
 *
 * So this still compiles to "return what you were given" for all 48 sites, and
 * it is now TRUE BY CHECK rather than true by construction. The difference
 * matters the day somebody maps a second window: nothing here needs editing. */
unsigned long long vmm_phys(unsigned long long virt);
unsigned long long vmm_virt(unsigned long long phys);

static inline unsigned long long dma_addr(dma_uptr virt)
{
    return vmm_phys((unsigned long long)virt);
}

/* THE INVERSE, and it is not decoration - two places in xhci.c need it.
 *
 * An independent audit of xhci.c found this and it is the subtler half of the
 * problem: a device does not only READ addresses we give it, it REPORTS them
 * back. An xHCI Command Completion Event carries the address of the Command TRB
 * that produced it, and a Transfer Event carries the address of the TRB that
 * completed. Both come back as DEVICE addresses, and both are then compared
 * against a KERNEL address:
 *
 *     cmd_wait():  if (p != trb_addr) continue;
 *     kbd_event(): idx = (param - ring) / TRB_BYTES;
 *
 * Neither hands anything to a device, so neither is a dma_addr() site, and a
 * grep for the outbound direction finds neither. They are still broken by
 * exactly the same commit: the day dma_addr() stops being identity, every
 * command times out because no completion ever matches, and every pointer
 * report fails its bounds check and is silently dropped.
 *
 * That is a HALF-CONVERTED driver, which is worse than an unconverted one,
 * because the outbound half looks finished. So the inverse goes in at the same
 * time as the forward direction, while both are identity and the change is
 * provably a no-op. */
static inline dma_uptr dma_kaddr(unsigned long long dev)
{
    return (dma_uptr)vmm_virt(dev);
}

#endif /* ZL_DMA_H */
