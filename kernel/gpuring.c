/* gpuring.c - the ring zlOS submits through. The driver half of the GPU work.
 *
 * WHAT THIS IS
 * ------------
 * `gpu.c` builds commands. This is what puts them in front of the hardware:
 * a ring buffer in memory the GPU can reach, the four registers that arm it,
 * and a bounded wait for the engine to catch up.
 *
 * HONEST STATUS, AND DO NOT LET THIS PARAGRAPH GO STALE. Two different claims
 * here, and they must not be blurred:
 *
 *   THE MODEL IS PROVEN. On 2026-08-19 hosttest/gpu_ring.c performed exactly
 *   this sequence on real 8086:9B41 silicon with i915 unbound - forcewake, GGTT
 *   map, RING_CTL=0 / HEAD=0 / TAIL=0 / START / CTL=VALID, commands, MI_FLUSH_DW,
 *   advance TAIL, poll HEAD - and filled 16384 of 16384 pixels, verified by
 *   reading the destination back. A sole owner CAN drive the Gen9.5 legacy ring;
 *   no execlists are required. docs/gpu-driver.md has the register dump.
 *
 *   THIS FILE HAS STILL NOT RUN. The code below is the same sequence, but it
 *   has never executed inside zlOS, which needs a USB boot rather than a two
 *   minute detach. So `gpu_ring_arm()` stays and defaults to off - the same
 *   discipline `intel.c` uses with `lt_armed`. "The model works" is not "this
 *   implementation of it works".
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
int  intel_ggtt_map_range(gr_u32 gfx_page, gr_u32 phys_addr, int pages);

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

/* WHICH ENGINE. The four engines are identical in ring shape - TAIL/HEAD/START/
 * CTL at +0x30/34/38/3C from the engine's own base - which is why supporting a
 * second one costs a parameter rather than a rewrite. Read off this machine,
 * read-only, all four idle because i915 drives them through execlists:
 *
 *   RCS0 render    base 0x02000
 *   VCS0 video     base 0x12000
 *   VECS0 enhance  base 0x1A000
 *   BCS0 blitter   base 0x22000
 *
 * BCS is what has actually run: 16384/16384 pixels on this silicon. RCS is the
 * one the MEASUREMENTS want - blending is 48x there and a plain fill is a tie on
 * the blitter - and its ring is this same sequence at a different base.
 *
 * RCS's FORCEWAKE domain is now CONFIRMED on this part - see the note by
 * FORCEWAKE_RENDER_GEN9 below. What RCS still needs beyond a ring is pipeline
 * state, surface state and a binding table; the ring is necessary, not
 * sufficient. */
#define GPU_ENGINE_BCS 0
#define GPU_ENGINE_RCS 1

int gpu_ring_init(void);
int gpu_ring_init_engine(int engine);

/* FORCEWAKE. On Gen9 the GT power wells sleep, and a register read of a
 * sleeping well returns 0 while a WRITE IS DISCARDED - silently. Every ring
 * register below is inside that well, so it must be held first or the whole
 * sequence is a no-op that looks like broken hardware. Measured acking on this
 * part by hosttest/gpu_ring.c --survey. */
#define FORCEWAKE_BLITTER_GEN9     0x0A188u
#define FORCEWAKE_ACK_BLITTER_GEN9 0x130044u
#define FORCEWAKE_RENDER_GEN9      0x0A278u
#define FORCEWAKE_ACK_RENDER_GEN9  0x00D84u
#define FW_KERNEL_BIT 1u

/* ALL THREE DOMAINS CONFIRMED ON THIS PART, 2026-08-19, and none of them looked
 * up - found by reading the request block and then watching which ack answered:
 *
 *   RENDER   req 0x0A278  ack 0x00D84    0 -> 1 -> 0
 *   MEDIA    req 0x0A270  ack 0x00D88    0 -> 1 -> 0
 *   BLITTER  req 0x0A188  ack 0x130044   0 -> 1 -> 0
 *
 * The three request registers sit together and share a resting value of
 * 0x00010000 (mask latched, value clear), which is what identified them.
 *
 * THE RENDER WELL IS SLOWER, and this is the part that matters. The blitter acks
 * on the first read; render took **312 polls**. A single read after a settle loop
 * reported the ack as 0 and made the register look wrong - the same experiment
 * with a poll instead of a read confirmed it immediately. So forcewake_get polls
 * for the ack and never assumes a fixed delay is enough, exactly as this repo's
 * boot gates learned to wait for output rather than a clock. */
static gr_u32 engine_fw_req(int engine)
{
    return engine == GPU_ENGINE_RCS ? FORCEWAKE_RENDER_GEN9 : FORCEWAKE_BLITTER_GEN9;
}
static gr_u32 engine_fw_ack(int engine)
{
    return engine == GPU_ENGINE_RCS ? FORCEWAKE_ACK_RENDER_GEN9 : FORCEWAKE_ACK_BLITTER_GEN9;
}

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
 * The SEQUENCE below is proven on silicon; THIS CODE has not run. See the
 * header for why those are different claims. */

static int   ring_engine = GPU_ENGINE_BCS;

static gr_u32 engine_base(void)
{
    return ring_engine == GPU_ENGINE_RCS ? RCS_BASE : BCS_BASE;
}

/* Which engine the ring is pointed at. Exposed for two reasons, and the second
 * is the one that made it necessary.
 *
 * It is a useful diagnostic: on hardware with no serial port, "which engine did
 * it actually select" is a question the screen has to be able to answer.
 *
 * And a test cannot otherwise tell WHY an init refused. In a harness with no GPU
 * every path returns 0 - the RCS guard, the intel_present check, the arm gate -
 * so asserting the return value proves nothing about which one fired. Deleting
 * the RCS guard entirely left a 121-check suite green. Reading the engine back
 * distinguishes them, because the guard returns BEFORE the assignment. */
int gpu_ring_engine(void) { return ring_engine; }

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

/* Release the well. Holding forcewake permanently keeps the GT awake and burns
 * power on a laptop for no reason, so every early-return path that took it has
 * to give it back - the leak is otherwise silent.
 *
 * IT WAS CALLED AND NEVER DEFINED. gcc accepted the implicit declaration and
 * built kernel.elf and kernel64.elf clean; clang under buildefi.sh rejected it,
 * because C99 dropped implicit declarations. That asymmetry is the whole reason
 * this repo builds all four targets rather than the one being worked on. */
static void forcewake_put(void)
{
    mmio_w(engine_fw_req(ring_engine), (FW_KERNEL_BIT << 16) | 0u);
}

static int forcewake_get(void)
{
    gr_u32 req = engine_fw_req(ring_engine), ack = engine_fw_ack(ring_engine);
    mmio_w(req, (FW_KERNEL_BIT << 16) | FW_KERNEL_BIT);
    /* Poll. The render well needed 312 iterations on this part where the blitter
     * answers on the first read; a fixed settle makes a correct register look
     * wrong. The ceiling is generous rather than tuned. */
    for (int i = 0; i < 2000000; i++)
        if (mmio_r(ack) & FW_KERNEL_BIT) return 1;
    return 0;
}

/* Bring the ring up. Returns 1 if the hardware took it.
 *
 * ORDER MATTERS. Disable first, then point START at our page, then enable,
 * and only then move TAIL. Writing TAIL into a ring that still points at
 * whatever the firmware or i915 left behind tells the engine to execute that. */
int gpu_ring_init_engine(int engine)
{
    ring_live = 0;
    ring_tail = 0;
    if (engine != GPU_ENGINE_BCS && engine != GPU_ENGINE_RCS) return 0;
    /* RCS is no longer refused: its forcewake domain is confirmed on this part
     * (0x0A278 / ack 0x00D84, watched going 0 -> 1 -> 0). What is still true is
     * that a RENDER submission needs pipeline state, surface state and a binding
     * table that BCS does not - so a ring on RCS is necessary and not sufficient,
     * and gpu_fill_try deliberately still runs on the blitter. */
    ring_engine = engine;
    if (!intel_present() || !intel_supported()) return 0;
    if (!ring_armed) return 0;               /* refuse rather than half-arm */

    /* FORCEWAKE FIRST, matching the order the silicon run used. Nothing here is
     * known to depend on it - the GGTT window is not in the GT well - but the
     * sequence that has actually filled pixels is the one to copy, and any
     * difference from it should be deliberate rather than left over. */
    if (!forcewake_get()) return 0;

    /* the ring's physical page, into the graphics address the engine will use */
    if (!intel_ggtt_map(GPU_RING_GFX >> 12, (gr_u32)HI_GPU)) { forcewake_put(); return 0; }
    (void)mmio_r(0x800000u);                 /* posting read: flush the PTE */

    mmio_w(engine_base() + REG_CTL,   0);
    mmio_w(engine_base() + REG_HEAD,  0);
    mmio_w(engine_base() + REG_TAIL,  0);
    mmio_w(engine_base() + REG_START, GPU_RING_GFX);
    (void)mmio_r(engine_base() + REG_START);
    mmio_w(engine_base() + REG_CTL,   RING_VALID);   /* one page, enabled */

    /* Release the well on THIS path too. It is the one failure that happens
     * after forcewake_get succeeds and is easy to miss, because it looks like
     * a simple "the ring refused" rather than a resource path - which is
     * precisely how a silent leak gets written. Found by auditing every return
     * between the get and the end of the function, not by reading it once. */
    if (!(mmio_r(engine_base() + REG_CTL) & RING_VALID)) { forcewake_put(); return 0; }
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
    gr_u32 head = mmio_r(engine_base() + REG_HEAD) & (GPU_RING_BYTES - 1u);

    /* n dwords + the 5-dword flush + up to 8 bytes of qword pad */
    if (gpu_ring_space(head, ring_tail, GPU_RING_BYTES) < n * 4u + 5u * 4u + 8u) return 0;

    gr_u32 t = gpu_ring_write(ring, GPU_RING_BYTES, ring_tail, dw, n);

    /* MI_FLUSH_DW, because the sequence PROVEN on silicon has one and this
     * function did not.
     *
     * hosttest/gpu_ring.c filled 16384/16384 pixels on 8086:9B41 with a flush
     * after the blit; this driver emitted the blit alone. Whether the flush is
     * load-bearing was never isolated - the successful run changed two things
     * at once, the flush and a GGTT mapping fix - so the honest position is
     * that one sequence is known to work and the other is a guess that differs
     * from it. Matching the proven one costs five dwords.
     *
     * There is no GEM here to do domain management, so if the blitter's writes
     * sit in a cache nothing else will push them out. Gen8+ MI_FLUSH_DW is five
     * dwords: opcode 0x26 in the MI client, length (5-2), no post-sync write. */
    /* UNCONDITIONAL. The space check above already reserved these five dwords,
     * so a second test here can only ever do one thing: silently drop the flush
     * and submit a sequence that differs from the proven one with nothing to
     * say so. That is the shape this tree keeps getting caught by - a guard that
     * reads as caution and acts as a silent skip. If the reservation is ever
     * wrong, the right outcome is refusing the submission at the top, not
     * emitting most of it. */
    const gr_u32 flush[5] = { (0x26u << 23) | 3u, 0, 0, 0, 0 };
    t = gpu_ring_write(ring, GPU_RING_BYTES, t, flush, 5);

    t = gpu_ring_pad(ring, GPU_RING_BYTES, t);
    ring_tail = t;

    mmio_w(engine_base() + REG_TAIL, ring_tail);

    for (int spin = 0; spin < 2000000; spin++) {
        gr_u32 h = mmio_r(engine_base() + REG_HEAD) & (GPU_RING_BYTES - 1u);
        if (h == ring_tail) return 1;
    }
    return 0;                                 /* engine never caught up */
}

/* ---- the compositor's fill path ------------------------------------------
 *
 * fb_fill_px calls gpu_fill_try first and falls back to its own SSE path when
 * this returns 0 - which is every build until the ring is armed on hardware.
 *
 * THE THRESHOLD IS NOT A GUESS, AND ITS STATED REASON WAS WRONG. Measured on
 * this exact part against fb.c's real fill32 (docs/gpu-blitter.md), with ONE
 * submission per rectangle, which is what a call from fb_fill_px is:
 *
 *     64x64        CPU wins 8.39x   - submission cost dwarfs the fill
 *     1024x768     CPU wins 1.62x
 *     1920x1200    CPU wins 1.74x
 *     3840x2160    blitter wins 1.14x
 *
 * So the blitter only pays off above roughly four megapixels. The number
 * stands; the REASON originally given for it - "submission cost dominates
 * below this" - does not, and the correction matters because it changes what
 * would move the threshold.
 *
 * Those figures were taken through i915's ioctl path, which costs 0.652 ms per
 * submit+wait (docs/gpu-blitter.md). zlOS pays nothing like that: it owns the
 * ring, so a submission is a register write and a poll, and the --ring run on
 * real silicon reported submit-to-complete as 0.00 ms - below the timer's
 * resolution. So if submission cost were the binding constraint, zlOS's
 * threshold would be far LOWER than the harness data suggests.
 *
 * It is not the binding constraint. Look at the raw rates: at 1920x1200 the
 * blitter does 3643 Mpix/s against fb.c's 3897, and at 3840x2160 it does 3590
 * against 3037. The blitter has no fill-rate advantage at all until the surface
 * stops fitting in cache, and that crossover - not submission overhead - is
 * what sits near four megapixels. Removing submission cost entirely would move
 * the threshold hardly at all.
 *
 * Consequence for anyone tempted to tune this: making submission cheaper is not
 * the lever. The lever is surface size, and on a 2560x1440 panel there is no
 * size that helps.
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

    /* USE THE RANGE FUNCTION, not an open-coded loop.
     *
     * eaa5492 gave intel_ggtt_map_range overflow guards this loop does not
     * have: `gfx_page + i` and `phys_addr + i * 4096` are u32 sums, and a
     * wrapped pair is a perfectly valid-looking mapping of the wrong page to
     * the wrong frame. The back buffer is up to 40 MiB - 10240 pages - so this
     * is the one caller in the driver with enough pages to care.
     *
     * Mapping a PREFIX rather than the whole surface is the failure this
     * protects against in the other direction: a fill correct at the top of the
     * screen and landing somewhere else below it. */
    gr_u32 pages = (bytes + 4095u) / 4096u;
    if (!intel_ggtt_map_range(GPU_FB_GFX >> 12, phys, (int)pages)) return 0;

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

/* ---- the self-test: what a USB boot is FOR ---------------------------------
 *
 * There is no serial port on the ThinkPad, so the screen is the only
 * diagnostic (docs/thinkpad-first-boot.md). A boot that arms the ring and says
 * nothing is a boot wasted. This runs the whole sequence and leaves every
 * number a human needs behind it, for kernel.zl to print.
 *
 * It is a COMMAND, not a boot step, deliberately. If this hangs the GPU, a
 * desktop that already came up is worth far more than one that never did - and
 * the person running it chose the moment.
 *
 * The sequence is the one proven on 8086:9B41 (docs/gpu-driver.md). What is NOT
 * proven is this implementation of it, which is the entire point of running it.
 */
#define GPU_ST_W     64u
#define GPU_ST_H     64u
#define GPU_ST_PITCH (GPU_ST_W * 4u)
#define GPU_ST_BYTES (GPU_ST_PITCH * GPU_ST_H)
#define GPU_ST_PHYS  ((gr_u64)HI_GPU + 4096u + 16384u)   /* after ring + cursor */
#define GPU_ST_GFX   0x04002000u                          /* ring gfx + 2 pages */
#define GPU_ST_COLOR 0x60D2EBu
#define GPU_ST_POISON 0xDEADBEEFu

_Static_assert(GPU_ST_PHYS + GPU_ST_BYTES <= (gr_u64)HI_BLUR,
               "the self-test surface runs past HI_GPU into the blur arena");

static gr_u32 st_filled, st_poison, st_ctl, st_head, st_tail;

gr_u32 gpu_st_filled(void) { return st_filled; }
gr_u32 gpu_st_want(void)   { return GPU_ST_W * GPU_ST_H; }
gr_u32 gpu_st_poison(void) { return st_poison; }
gr_u32 gpu_st_ctl(void)    { return st_ctl; }
gr_u32 gpu_st_head(void)   { return st_head; }
gr_u32 gpu_st_tail(void)   { return st_tail; }

/* Returns 0 on success, or the number of the step that failed - so a screen
 * with one integer on it still says WHERE it stopped. */
int gpu_selftest(void)
{
    st_filled = st_poison = st_ctl = st_head = st_tail = 0;

    if (!intel_present() || !intel_supported()) return 1;

    gpu_ring_arm(1);                      /* the caller asked for this */
    if (!gpu_ring_init()) { gpu_ring_arm(0); return 2; }
    st_ctl = mmio_r(engine_base() + REG_CTL);

    if (!intel_ggtt_map_range(GPU_ST_GFX >> 12, (gr_u32)GPU_ST_PHYS,
                              (int)(GPU_ST_BYTES / 4096u))) { gpu_ring_arm(0); return 3; }

    /* Poison first, so "filled" can never be confused with "never ran" - the
     * same discipline every other check in this driver uses. */
    gr_u32 *dst = (gr_u32 *)(gr_uptr)GPU_ST_PHYS;
    for (gr_u32 i = 0; i < GPU_ST_BYTES / 4u; i++) dst[i] = GPU_ST_POISON;

    gr_u32 dw[16];
    struct gpu_batch b;
    gpu_batch_init(&b, dw, 16);
    if (!gpu_fill_rect(&b, GPU_ST_GFX, GPU_ST_PITCH, 0, 0,
                       (int)GPU_ST_W, (int)GPU_ST_H, GPU_ST_COLOR)) {
        gpu_ring_arm(0); return 4;
    }
    /* No MI_BATCH_BUFFER_END: these go into the RING, and gpu_ring_submit
     * appends the flush itself. */
    int ok = gpu_ring_submit(dw, b.at);
    st_head = mmio_r(engine_base() + REG_HEAD);
    st_tail = mmio_r(engine_base() + REG_TAIL);

    for (gr_u32 i = 0; i < GPU_ST_BYTES / 4u; i++) {
        if ((dst[i] & 0xFFFFFFu) == GPU_ST_COLOR) st_filled++;
        else if (dst[i] == GPU_ST_POISON) st_poison++;
    }

    gpu_ring_arm(0);                      /* leave it disarmed either way */
    if (!ok) return 5;                    /* engine never caught up */
    if (st_filled != GPU_ST_W * GPU_ST_H) return 6;   /* ran, drew nothing/partial */
    return 0;
}

/* The original entry point, unchanged in behaviour: the blitter. Kept so no
 * existing caller has to learn about engines to keep working. */
int gpu_ring_init(void) { return gpu_ring_init_engine(GPU_ENGINE_BCS); }
