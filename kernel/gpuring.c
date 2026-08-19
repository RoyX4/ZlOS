/* gpuring.c - the ring zlOS submits through. The driver half of the GPU work.
 *
 * WHAT THIS IS
 * ------------
 * `gpu.c` builds commands. This is what puts them in front of the hardware:
 * a ring buffer in memory the GPU can reach, the four registers that arm it,
 * and a bounded wait for the engine to catch up.
 *
 * HONEST STATUS, AND DO NOT LET THIS PARAGRAPH GO STALE. The command streams
 * this submits are verified pixel-for-pixel on real 8086:9B41 silicon, through
 * i915's render node (`hosttest/gpu_blt.c`). **The submission path below has
 * never executed on hardware.** Every MMIO write in `ring_arm()` and
 * `gpu_ring_kick()` is written from the register documentation and the register
 * dumps in `docs/gpu-blitter.md`, and nothing has run them. That is the whole
 * reason `gpu_ring_arm()` exists and defaults to off - the same discipline
 * `intel.c` uses with `lt_armed`, and for the same reason: this file can hang a
 * GPU, and "the code exists" is not "the code works".
 *
 * What IS tested, on any machine and in milliseconds, is every piece of
 * bookkeeping: ring space, the wrap, qword alignment of the tail, and refusing
 * a batch that cannot fit. `hosttest/gputest.c` covers it and its checks have
 * been watched failing. Those are the parts that are wrong quietly; the MMIO is
 * wrong loudly.
 *
 * WHY BCS AND NOT RCS. The blitter is the smaller target and the stepping
 * stone: same GGTT, same forcewake, same four ring registers, at a different
 * base. `docs/gpu-next.md` has the measurement saying the *performance* case is
 * on the render engine instead - this exists to make that reachable, not
 * because a blitter fill beats fb.c. It does not.
 *
 * NO DISPLAY REGISTER IS TOUCHED HERE, and no panel power. intel.c's hazard
 * list is about hardware this file never addresses.
 */

#include "memmap.h"
#include "gpu.h"

typedef unsigned int       gr_u32;
typedef unsigned long long gr_u64;

#if defined(ZL_64) || defined(__x86_64__)
typedef unsigned long long gr_uptr;
#else
typedef unsigned int       gr_uptr;
#endif

/* from intel.c - the BAR0 mapping and the GGTT it already knows how to program */
int  intel_present(void);
int  intel_supported(void);
gr_u32 intel_mmio(void);
gr_u32 intel_ggtt_size(void);
int  intel_ggtt_map(gr_u32 gfx_page, gr_u32 phys_addr);

/* ---- where the ring lives ------------------------------------------------
 *
 * PHYSICALLY at HI_GPU, declared in memmap.h like every other fixed buffer in
 * this kernel, because an address that no map declares is the defect that put
 * intel.c's EDID buffer inside fb.c's blur arena.
 *
 * IN GRAPHICS ADDRESS SPACE at GPU_RING_GFX. That is a separate space: the
 * engine resolves it through the GGTT, so the ring's physical address and its
 * graphics address have nothing to do with each other and BOTH have to be
 * right. intel.c's own modeset path maps the scanout framebuffer at graphics
 * 1 MiB and it spans the whole framebuffer - 33 MiB at 3840x2160 - so the ring
 * sits at 64 MiB, clear of it with room to spare.
 */
#define GPU_RING_BYTES 4096u                 /* one page, and RING_CTL's length
                                              * field counts pages minus one   */
#define GPU_RING_GFX   0x04000000u           /* 64 MiB into the graphics space */
#define GPU_FB_GFX     0x08000000u           /* 128 MiB: the back buffer, mapped
                                              * for gpu_fill_try. Clear of the
                                              * ring above and of intel.c's
                                              * scanout at graphics 1 MiB. */

_Static_assert(GPU_RING_BYTES == 4096u, "the RING_CTL length field below assumes one page");
_Static_assert((gr_u64)HI_GPU + GPU_RING_BYTES <= (gr_u64)HI_BLUR,
               "the GPU ring runs into fb.c's blur arena");

/* ---- the engine's registers ---------------------------------------------
 * Bases read off this machine's own i915_engine_info rather than a datasheet;
 * see docs/gpu-blitter.md. RCS is here too because this file is the stepping
 * stone to it and the offsets are identical. */
#define BCS_BASE  0x22000u
#define RCS_BASE  0x02000u
#define REG_TAIL  0x30u
#define REG_HEAD  0x34u
#define REG_START 0x38u
#define REG_CTL   0x3Cu
#define RING_VALID 1u

/* FORCEWAKE. On Gen9 the GT power wells sleep, and a register read of a
 * sleeping well returns 0 while a WRITE IS DISCARDED - silently. Every ring
 * register below is inside that well, so it must be held first or the whole
 * sequence is a no-op that looks like broken hardware. Measured acking on this
 * part by hosttest/gpu_ring.c --survey. */
#define FORCEWAKE_BLITTER_GEN9     0x0A188u
#define FORCEWAKE_ACK_BLITTER_GEN9 0x130044u
#define FW_KERNEL_BIT 1u

/* ---- the pure bookkeeping ------------------------------------------------
 * Everything in this section is arithmetic on integers, has no side effects,
 * and is what hosttest/gputest.c hammers. It is also where a ring goes wrong
 * quietly: an off-by-one in the space calculation overwrites commands the
 * engine has not read yet, and the symptom is the GPU executing garbage.
 */

/* Bytes we may write without catching up to HEAD.
 *
 * The ring is full when TAIL would reach HEAD, so one qword is always left
 * unused - otherwise a full ring and an empty ring both read HEAD == TAIL and
 * nothing can tell them apart. That reserved qword is not an off-by-one; it is
 * what makes the two states distinguishable. */
gr_u32 gpu_ring_space(gr_u32 head, gr_u32 tail, gr_u32 size)
{
    if (size == 0) return 0;
    head %= size;
    tail %= size;
    /* Bytes the engine has not consumed yet. TAIL >= HEAD is the un-wrapped
     * case; otherwise the used span crosses the end of the buffer. */
    gr_u32 used = (tail >= head) ? (tail - head) : (size - head + tail);
    gr_u32 freebytes = size - used;
    /* Written as a comparison, NOT as `freebytes - 8`, because these are
     * unsigned: a ring with fewer than 8 bytes free would otherwise wrap to
     * about four billion and report unlimited space. */
    return freebytes > 8u ? freebytes - 8u : 0u;
}

/* Copy `n` dwords into the ring at `tail`, wrapping, and return the new tail.
 * The caller has already checked the space; this only has to get the wrap
 * right. `ring` is the CPU view; `size` is in bytes. */
gr_u32 gpu_ring_write(gr_u32 *ring, gr_u32 size, gr_u32 tail,
                      const gr_u32 *dw, gr_u32 n)
{
    gr_u32 words = size / 4u;
    gr_u32 at = tail / 4u;
    for (gr_u32 i = 0; i < n; i++) {
        ring[at] = dw[i];
        at = (at + 1u) % words;
    }
    return at * 4u;
}

/* The tail handed to the hardware must be qword aligned. Pad with MI_NOOP,
 * which is 0 - and NOT with anything else: the engine parses whatever is
 * between HEAD and TAIL, so a pad has to be a real instruction. */
gr_u32 gpu_ring_pad(gr_u32 *ring, gr_u32 size, gr_u32 tail)
{
    gr_u32 words = size / 4u;
    while (tail & 7u) {
        ring[(tail / 4u) % words] = 0u;      /* MI_NOOP */
        tail = (tail + 4u) % size;
    }
    return tail;
}

/* ---- the hardware half ---------------------------------------------------
 * NONE OF THIS HAS EVER RUN. See the header. */

static int   ring_armed = 0;      /* nothing writes a ring register until this
                                   * is set, deliberately, by a caller that
                                   * knows the display is not in use */
static int   ring_live  = 0;      /* init succeeded and the ring is enabled   */
static gr_u32 ring_tail = 0;

void gpu_ring_arm(int on) { ring_armed = on ? 1 : 0; }
int  gpu_ring_is_live(void) { return ring_live; }

static gr_u32 mmio_r(gr_u32 off)
{
    return *(volatile gr_u32 *)((gr_uptr)intel_mmio() + (gr_uptr)off);
}
static void mmio_w(gr_u32 off, gr_u32 val)
{
    if (!ring_armed) return;                 /* the gate, checked at the ONE
                                              * place every write goes through */
    *(volatile gr_u32 *)((gr_uptr)intel_mmio() + (gr_uptr)off) = val;
}

static int forcewake_get(void)
{
    mmio_w(FORCEWAKE_BLITTER_GEN9, (FW_KERNEL_BIT << 16) | FW_KERNEL_BIT);
    for (int i = 0; i < 100000; i++)
        if (mmio_r(FORCEWAKE_ACK_BLITTER_GEN9) & FW_KERNEL_BIT) return 1;
    return 0;
}

/* Bring the ring up. Returns 1 if the hardware took it.
 *
 * ORDER MATTERS. Disable first, then point START at our page, then enable,
 * and only then move TAIL. Writing TAIL into a ring that still points at
 * whatever the firmware or i915 left behind tells the engine to execute that. */
int gpu_ring_init(void)
{
    ring_live = 0;
    ring_tail = 0;
    if (!intel_present() || !intel_supported()) return 0;
    if (!ring_armed) return 0;               /* refuse rather than half-arm */

    /* the ring's physical page, into the graphics address the engine will use */
    if (!intel_ggtt_map(GPU_RING_GFX >> 12, (gr_u32)HI_GPU)) return 0;
    (void)mmio_r(0x800000u);                 /* posting read: flush the PTE */

    if (!forcewake_get()) return 0;

    mmio_w(BCS_BASE + REG_CTL,   0);
    mmio_w(BCS_BASE + REG_HEAD,  0);
    mmio_w(BCS_BASE + REG_TAIL,  0);
    mmio_w(BCS_BASE + REG_START, GPU_RING_GFX);
    (void)mmio_r(BCS_BASE + REG_START);
    mmio_w(BCS_BASE + REG_CTL,   RING_VALID);   /* one page, enabled */

    if (!(mmio_r(BCS_BASE + REG_CTL) & RING_VALID)) return 0;
    ring_live = 1;
    return 1;
}

/* Submit `n` dwords and wait for the engine to consume them.
 *
 * The wait is bounded by a spin count, not a clock: a wedged engine must not
 * hang the compositor, and this kernel has no timer it can block on here. */
int gpu_ring_submit(const gr_u32 *dw, gr_u32 n)
{
    if (!ring_live || !ring_armed) return 0;
    if (n == 0) return 0;

    gr_u32 *ring = (gr_u32 *)(gr_uptr)HI_GPU;
    gr_u32 head = mmio_r(BCS_BASE + REG_HEAD) & (GPU_RING_BYTES - 1u);

    /* +8 for the qword pad the tail may need */
    if (gpu_ring_space(head, ring_tail, GPU_RING_BYTES) < n * 4u + 8u) return 0;

    gr_u32 t = gpu_ring_write(ring, GPU_RING_BYTES, ring_tail, dw, n);
    t = gpu_ring_pad(ring, GPU_RING_BYTES, t);
    ring_tail = t;

    mmio_w(BCS_BASE + REG_TAIL, ring_tail);

    for (int spin = 0; spin < 2000000; spin++) {
        gr_u32 h = mmio_r(BCS_BASE + REG_HEAD) & (GPU_RING_BYTES - 1u);
        if (h == ring_tail) return 1;
    }
    return 0;                                 /* engine never caught up */
}

/* ---- the compositor's fill path ------------------------------------------
 *
 * fb_fill_px calls gpu_fill_try first and falls back to its own SSE path when
 * this returns 0 - which is every build until the ring is armed on hardware.
 *
 * THE THRESHOLD IS NOT A GUESS AND IT IS DELIBERATELY HUGE. Measured on this
 * exact part against fb.c's real fill32 (docs/gpu-blitter.md), with ONE
 * submission per rectangle, which is what a call from fb_fill_px is:
 *
 *     64x64        CPU wins 8.39x   - submission cost dwarfs the fill
 *     1024x768     CPU wins 1.62x
 *     1920x1200    CPU wins 1.74x
 *     3840x2160    blitter wins 1.14x
 *
 * So the blitter only pays off for a rectangle bigger than roughly four
 * megapixels, submitted on its own. Anything smaller is slower, and a naive
 * "send every fill to the GPU" wiring would make the compositor worse - which
 * is exactly why this exists as a threshold rather than an unconditional call.
 *
 * Being honest about the consequence: at 2560x1440 the whole screen is 3.7
 * Mpix, so on THIS panel this path will essentially never fire. It is wired
 * because the milestone asked for it and because the shape is right for a 4K
 * external panel; it is not wired because it is expected to help here. The
 * number that settles it is frame time with the flag on, not fill rate.
 */
#define GPU_FILL_MIN_PX 4000000u

static gr_u32 fb_gfx    = 0;      /* the back buffer's graphics address */
static gr_u32 fb_pitch  = 0;      /* bytes per row */
static int    fb_mapped = 0;

/* Map the back buffer where the engine can reach it. Called once, after
 * gpu_ring_init. `phys` is HI_BACK; `bytes` is what fb.c actually uses. */
int gpu_fb_attach(gr_u32 phys, gr_u32 bytes, gr_u32 pitch)
{
    fb_mapped = 0;
    if (!ring_live || !pitch || !bytes) return 0;
    if (pitch > 0xFFFFu) return 0;              /* BR13's field, see gpu.c */

    /* Every page needs an entry. Mapping a prefix is a fill that is correct at
     * the top of the screen and lands somewhere else below it. */
    gr_u32 pages = (bytes + 4095u) / 4096u;
    for (gr_u32 i = 0; i < pages; i++)
        if (!intel_ggtt_map((GPU_FB_GFX >> 12) + i, phys + i * 4096u)) return 0;

    fb_gfx   = GPU_FB_GFX;
    fb_pitch = pitch;
    fb_mapped = 1;
    return 1;
}

/* Returns 1 if the GPU did the fill, 0 if the caller should do it itself. */
int gpu_fill_try(int x, int y, int w, int h, gr_u32 rgb)
{
    if (!ring_live || !fb_mapped) return 0;
    if (w <= 0 || h <= 0) return 0;
    if ((gr_u32)w * (gr_u32)h < GPU_FILL_MIN_PX) return 0;

    gr_u32 dw[16];
    struct gpu_batch b;
    gpu_batch_init(&b, dw, 16);
    if (!gpu_fill_rect(&b, fb_gfx, fb_pitch, x, y, x + w, y + h, rgb)) return 0;
    /* No MI_BATCH_BUFFER_END: these dwords go straight into the RING, which the
     * engine executes up to TAIL. An END here would stop the ring, not a batch. */
    return gpu_ring_submit(dw, b.at);
}
