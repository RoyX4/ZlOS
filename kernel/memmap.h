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
 *   0x02000000   32   kernel.zl     snake, fs, shell line + history  1 MiB
 *                       (check-memmap.sh owns the detail; the SPAN is here
 *                        so this file's asserts can see it)
 *   0x03000000   48   png.c         decoded pictures + scratch     4 MiB
 *   0x08000000  128   fb.c          back, the back buffer         48 MiB
 *   0x0B000000  176   sched.c       task stacks + demo counters    8 MiB
 *   0x0B800000  184   i2c_hid.c     HID report + descriptor bufs   8 MiB
 *   0x0C000000  192   fb.c          the cached-blur arena         16 MiB
 *   0x0D000000  208   nvme.c        admin + I/O queues            16 MiB
 *   0x0E000000  224   xhci.c        the USB DMA arena             16 MiB
 *   0x0F000000  240   virtio_gpu.c  rings + the GPU framebuffer   16 MiB
 *   0x10000000  256   --- top of a -m 256 guest, nothing above ---
 *
 * A region's ceiling is the next region's base. "Does this fit" is that
 * subtraction, never a compile-time pixel count - a pixel count silently stops
 * being true when the panel gets bigger, which is how the ThinkPad's 2560x1440
 * lost the back buffer without printing a word.
 *
 * 256 MiB IS A HARD CEILING, NOT A ROUND NUMBER. HANDOFF.md records `-m 256` as
 * the minimum zlOS boots with, and virtio_gpu.c explains what happens when a
 * buffer crosses it: the device cannot reach the memory and
 * RESOURCE_ATTACH_BACKING fails with ERR_UNSPEC, which reads like a driver bug
 * and is not one. Nothing may be placed at or above HI_TOP.
 *
 * ADDING A REGION: put its base here in ascending order, add it to the ordering
 * chain below, and have the owning file assert that its highest byte lands
 * under the next base. All three, or the check has a hole in it.
 */
#ifndef ZL_MEMMAP_H
#define ZL_MEMMAP_H

/* THE FIRST REGION BELOW 128 MiB, and the first thing in this map that is not
 * a driver's DMA arena. A decoded picture is 4 bytes a pixel and a page has
 * several, so this is megabytes - and megabytes cannot be BSS here.
 *
 * MEASURED, on this branch, with everything in: __kernel_end is 0x005DAAC0 =
 * 5.854 MiB against link.ld's ASSERT that it stays under 0x00600000, which
 * leaves the WHOLE KERNEL 152,896 bytes - 149 KiB - of room to grow. A static
 * 2 MiB arena in png.c could not link, and the error it produced would read
 * "the kernel image has grown into the raw-boot stack at 6 MiB", naming the
 * stack rather than the picture that took the space.
 *
 * 149 KiB IS THE NUMBER TO CHECK BEFORE ADDING ANY ARRAY ANYWHERE IN THIS
 * KERNEL, not just here. Total BSS is already 3,339,328 bytes; the browser's
 * share of it (a 256 KiB document, ~700 KiB of runs, ~650 KiB of html nodes)
 * is most of the recent growth. Re-measure with:
 *     ./build.sh && nm kernel.elf | grep __kernel_end
 *
 * IT WAS AT 32 MiB AND THAT WAS WRONG, which is worth keeping rather than
 * quietly correcting, because the mistake is the exact one this file exists
 * to prevent and it was made by someone who had this file open.
 *
 * 32 MiB looked free: it is 2 MiB aligned, it is the first such address above
 * arena.c's program arena (8..24 MiB), and every assert in this file passed.
 * **It is not free.** kernel.zl keeps a SECOND fixed-address block there -
 * SNAKE_X/SNAKE_Y at 0x02000000, FS_META and FS_DATA at 0x02010000, and the
 * shell's LINE_BUF and HIST_BUF at 0x02030000 - and a 4 MiB picture arena
 * based at 0x02000000 lands on all six. The bundled 32x32 home-page image
 * already overwrote both Snake arrays; a 132x132 PNG from any server would
 * have written through the RAM filesystem's metadata and data.
 *
 * THE ASSERTS DID NOT CATCH IT BECAUSE THEY ONLY COMPARE THIS FILE'S OWN
 * REGIONS TO EACH OTHER. That is the same hole, one map over, that this
 * file's header describes i2c_hid.c falling into: "the bases are each
 * individually sensible and only collide when you subtract them." Two maps
 * that do not know about each other are two maps that will collide.
 *
 * So: the region moved to 48 MiB, which is above the whole zl block and 80 MiB
 * clear of HI_BACK - and, more importantly, the zl block is now DECLARED here
 * so the compiler checks it too. Found by an adversarial review from a
 * different model family, after check-memmap.sh had printed the colliding
 * addresses in this very session and nobody joined them up.
 *
 * THE REGION IS CARVED IN TWO, and the split is here rather than in png.c
 * because this file is where a neighbour would come looking:
 *   HI_IMG            .. +2 MiB   png.c's pixel arena, 524288 ARGB pixels
 *   HI_IMG_SCRATCH    .. +2 MiB   browser.c's base64 buffer for data: URIs
 * The second half is nearly all spare on purpose: a data: URI is bounded by
 * the 256 KiB document that carries it, so 2 MiB can never be short. */
/* kernel.zl's OWN fixed addresses, declared here so this file's asserts can
 * see them. They are not owned by memmap.h - check-memmap.sh derives their
 * exact extents from kernel.zl and is still the authority on the details -
 * but their SPAN belongs in the map, or the map is lying by omission.
 * 0x02000000 (SNAKE_X) to 0x02032000 (end of HIST_BUF), rounded up. */
#define ZL_LOW_BASE  0x02000000UL
#define ZL_LOW_END   0x02100000UL   /* 33 MiB - covers the block with room  */

#define HI_IMG    0x03000000UL   /* png.c        - decoded picture arena    */
#define HI_IMG_SCRATCH (HI_IMG + 0x200000UL)
#define HI_IMG_END     (HI_IMG + 0x400000UL)

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
#define HI_BLUR   0x0C000000UL   /* fb.c         - the cached-blur arena    */
#define HI_NVME   0x0D000000UL   /* nvme.c       - admin + I/O queues       */
#define HI_XHCI   0x0E000000UL   /* xhci.c       - the USB DMA arena        */
#define HI_VGPU   0x0F000000UL   /* virtio_gpu.c - rings + GPU framebuffer  */
#define HI_TOP    0x10000000UL   /* 256 MiB - the guest ends here           */

/* THE MAP MUST BE IN ORDER, AND THE COMPILER SHOULD SAY SO.
 *
 * Every ceiling is a subtraction of one base from the next, which is only
 * meaningful if the bases ascend. Reorder two of them by mistake and the
 * subtraction underflows to a colossal unsigned number, every "does it fit"
 * test passes, and a buffer lands on top of a neighbour.
 *
 * These cost nothing at run time and fail the build the moment the map stops
 * making sense. A comment claiming the order would not have. */
_Static_assert(HI_IMG   < HI_BACK,  "high-RAM map out of order: img >= back");
_Static_assert(HI_IMG_END <= HI_BACK,
               "the picture arena has grown into fb.c's back buffer");
/* THE ONE THAT WAS MISSING. Without it the picture arena sat on Snake, the
 * filesystem and the shell's history for the length of one review cycle. */
_Static_assert(HI_IMG >= ZL_LOW_END,
               "the picture arena overlaps kernel.zl's fixed block at 32 MiB "
               "(SNAKE_X/FS_DATA/HIST_BUF - see check-memmap.sh)");
_Static_assert(ZL_LOW_BASE < ZL_LOW_END, "zl low block is inverted");
_Static_assert(HI_BACK  < HI_SCHED, "high-RAM map out of order: back >= sched");
_Static_assert(HI_SCHED < HI_HID,   "high-RAM map out of order: sched >= hid");
_Static_assert(HI_HID   < HI_BLUR,  "high-RAM map out of order: hid >= blur");
_Static_assert(HI_BLUR  < HI_NVME,  "high-RAM map out of order: blur >= nvme");
_Static_assert(HI_NVME  < HI_XHCI,  "high-RAM map out of order: nvme >= xhci");
_Static_assert(HI_XHCI  < HI_VGPU,  "high-RAM map out of order: xhci >= vgpu");
_Static_assert(HI_VGPU  < HI_TOP,   "high-RAM map out of order: vgpu >= 256 MiB");

#endif /* ZL_MEMMAP_H */
