/* memmap.h - the fixed high-RAM map, in ONE place, checked by the compiler.
 *
 * There is no allocator in this kernel, so every multi-megabyte buffer lives at
 * a fixed physical address. They are NEIGHBOURS, and the only thing stopping
 * one from eating the next is arithmetic.
 *
 * WHY THIS IS A HEADER AND NOT A COMMENT
 * --------------------------------------
 * It used to be a comment. fb.c carried the list and told you, in writing, not
 * to trust it: "Every base below was read out of the file that owns it - do not
 * take this list on trust, re-grep it." That instruction is the admission. A
 * list you must re-verify by hand is a list that is wrong between the moment
 * someone adds a buffer and the moment someone else re-greps.
 *
 * It was wrong. i2c_hid.c put HID_BUF at 0x0C900000 and HID_DESC_BUF at
 * 0x0C900100 - 9 MiB into the 16 MiB arena that fb.c hands out for cached
 * blurs, and before that inside the span `back` occupied. Neither file knew.
 * Nothing failed to build, because nothing was checked. This is the DMA-arena
 * collision HANDOFF.md counts five times, in the shape that hides best: the
 * bases are each individually sensible and only collide when you subtract them.
 *
 * So the map is declared once, here, and every file that owns a region asserts
 * against it. Getting it wrong is now a build error rather than a corrupted
 * frame or a scribbled-on touchpad report.
 *
 * THE MAP
 * -------
 *   base        MiB   owner         what lives there              span
 *   0x08000000  128   fb.c          back, the back buffer         48 MiB
 *   0x0B000000  176   sched.c       task stacks + demo counters    8 MiB
 *   0x0B800000  184   i2c_hid.c     HID report + descriptor bufs   4 MiB
 *   0x0BC00000  188   gpuring.c     the GPU command ring           4 MiB
 *   0x0C000000  192   fb.c          the cached-blur arena         16 MiB
 *   0x0D000000  208   nvme.c        admin + I/O queues            16 MiB
 *   0x0E000000  224   xhci.c        the USB DMA arena             16 MiB
 *   0x0F000000  240   virtio_gpu.c  rings + the GPU framebuffer   16 MiB
 *   0x10000000  256   heap.c        the general allocator         64 MiB
 *   0x14000000  320   --- everything above here is unclaimed ---
 *   0x40000000 1024   --- HI_TOP: the smallest machine we promise ---
 *
 * A region's ceiling is the next region's base. "Does this fit" is that
 * subtraction, never a compile-time pixel count - a pixel count silently stops
 * being true when the panel gets bigger, which is how the ThinkPad's 2560x1440
 * lost the back buffer without printing a word.
 *
 * WHAT HI_TOP IS, AND WHAT IT IS NOT
 * ----------------------------------
 * It is NOT a hardware limit and never was. It is one promise, in one number:
 * *the smallest guest zlOS claims to boot on.* Everything a device DMAs must
 * sit below it, because below it is the only memory we have promised exists.
 * virtio_gpu.c records the failure when a buffer crosses that line - the device
 * cannot reach it and RESOURCE_ATTACH_BACKING returns ERR_UNSPEC, which reads
 * like a driver bug and is not one.
 *
 * It was 256 MiB, and that number bought nothing. The gates did not honour it
 * in either direction:
 *
 *   - verify.sh, verify-raw.sh and verify-iso.sh passed NO `-m` at all, so
 *     qemu-system-i386 gave them its default 128 MiB and HALF the map above was
 *     unbacked RAM on every one of them (measured; arena.c:38-52 has the
 *     query-memory-size-summary output, and it is why the program arena sits at
 *     8 MiB rather than above the map where it belongs)
 *   - verify-efi.sh and exercise.py already passed `-m 1G`, four times the
 *     ceiling the header was asserting against
 *
 * So the 256 MiB line was a promise nothing kept and nothing checked. It is now
 * 1 GiB and every gate passes `-m 1G`, which makes HI_TOP the first ceiling in
 * this file that is true where it is enforced. The trade is written down in
 * HANDOFF.md: zlOS no longer claims to boot on a 256 MB machine.
 *
 * Nothing may be placed at or above HI_TOP. Raising it again means raising
 * every gate's `-m` in the same commit, or it goes back to being decoration.
 *
 * ADDING A REGION: put its base here in ascending order, add it to the ordering
 * chain below, and have the owning file assert that its highest byte lands
 * under the next base. All three, or the check has a hole in it.
 */
#ifndef ZL_MEMMAP_H
#define ZL_MEMMAP_H

#define HI_BACK   0x08000000UL   /* fb.c         - the back buffer          */
/* THE AP STACKS, and this region is why BACK_LIMIT is 40 MiB and not 48.
 *
 * smp_trampoline.S:25 and smp_trampoline64.S:34 both hardcode
 * .equ STACK_BASE, 0x0A800000 - and they do so on ALL EIGHT branches, while
 * appearing in no memory map except desktop/apps-in-windows's. Without this
 * line `back` runs to HI_SCHED, spans 128..176 MiB, and swallows the stacks
 * every application processor is running on. apps's SMP band rendering makes
 * those cores live every frame, so it would not even be a rare corruption.
 *
 * The _Static_asserts in fb.c cannot catch it on their own: they compare the
 * constants this header declares, and STACK_BASE was not one of them. A
 * compile-time check that reads as coverage without being it is worse than
 * no check. Declaring the region here is what makes the assert honest. */
#define HI_APSTK  0x0A800000UL   /* smp_trampoline{,64}.S STACK_BASE        */
#define AP_STACK_SIZE 0x4000UL   /* 16 KiB per core                         */
#define AP_STACK_SPAN (17UL * AP_STACK_SIZE)  /* cpu_apic_ids[] holds 16    */
#define HI_SCHED  0x0B000000UL   /* sched.c      - stacks, counters         */
#define HI_HID    0x0B800000UL   /* i2c_hid.c    - HID over I2C buffers     */
#define HI_GPU    0x0BC00000UL   /* gpuring.c    - the GPU command ring      */
#define HI_BLUR   0x0C000000UL   /* fb.c         - the cached-blur arena    */
#define HI_NVME   0x0D000000UL   /* nvme.c       - admin + I/O queues       */
#define HI_XHCI   0x0E000000UL   /* xhci.c       - the USB DMA arena        */
#define HI_VGPU   0x0F000000UL   /* virtio_gpu.c - rings + GPU framebuffer  */
/* THE FIRST REGION THAT IS NOT HAND-PLACED. Everything above is one buffer at
 * one address chosen by a person; this is 64 MiB that heap.c hands out and
 * takes back, so nothing inside it has a fixed address and nothing inside it
 * appears in this file. That is the point of it.
 *
 * It is at 256 MiB - above every existing region rather than between two of
 * them - so adding it moved nothing. That was deliberate: a second job was
 * building the desktop app suite against this map at the time, and the one
 * thing that could not happen was an occupied region changing address. */
#define HI_HEAP   0x10000000UL   /* heap.c       - the general allocator     */
#define HI_TOP    0x40000000UL   /* 1 GiB - the smallest guest we promise   */

/* The same number the gates must pass to `-m`, in the units `-m` takes, so the
 * two cannot drift apart silently. check-ram.sh greps this line and every
 * qemu invocation in the tree and fails if they disagree. */
#define HI_TOP_MB (HI_TOP >> 20)
_Static_assert(HI_TOP_MB == 1024, "HI_TOP and the gates' -m no longer agree");

/* THE MAP MUST BE IN ORDER, AND THE COMPILER SHOULD SAY SO.
 *
 * Every ceiling is a subtraction of one base from the next, which is only
 * meaningful if the bases ascend. Reorder two of them by mistake and the
 * subtraction underflows to a colossal unsigned number, every "does it fit"
 * test passes, and a buffer lands on top of a neighbour.
 *
 * These cost nothing at run time and fail the build the moment the map stops
 * making sense. A comment claiming the order would not have. */
_Static_assert(HI_BACK  < HI_SCHED, "high-RAM map out of order: back >= sched");
_Static_assert(HI_SCHED < HI_HID,   "high-RAM map out of order: sched >= hid");
_Static_assert(HI_HID   < HI_GPU,   "high-RAM map out of order: hid >= gpu");
_Static_assert(HI_GPU   < HI_BLUR,  "high-RAM map out of order: gpu >= blur");
_Static_assert(HI_BLUR  < HI_NVME,  "high-RAM map out of order: blur >= nvme");
_Static_assert(HI_NVME  < HI_XHCI,  "high-RAM map out of order: nvme >= xhci");
_Static_assert(HI_XHCI  < HI_VGPU,  "high-RAM map out of order: xhci >= vgpu");
_Static_assert(HI_VGPU  < HI_HEAP,  "high-RAM map out of order: vgpu >= heap");
_Static_assert(HI_HEAP  < HI_TOP,   "high-RAM map out of order: heap >= HI_TOP");

#endif /* ZL_MEMMAP_H */
