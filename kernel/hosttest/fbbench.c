/* fbbench.c - time the REAL fb.c on the real target CPU, from Linux.
 *
 * Same idea as gpu-dev.sh does for intel.c: the kernel's renderer is just C
 * against a block of memory, so it runs here at native speed with a cycle
 * counter and a diff, instead of a reboot and a stopwatch.
 *
 * This compiles the EXACT fb.c that ships in the kernel. The only thing the
 * harness provides is memory:
 *
 *   fb.c parks three buffers at fixed PHYSICAL addresses, because the kernel
 *   has no allocator (fb.c:92, 793, 794). We mmap those same addresses with
 *   MAP_FIXED so the shipping source compiles unmodified - no #ifdef, no
 *   patched copy that could drift from what actually boots.
 *
 * Reported in CYCLES PER PIXEL, which is clock-independent. Wall time is shown
 * too but this CPU turbos between 1.8 and 4.9 GHz, so treat ms as indicative
 * and cycles as the real number.
 *
 * Build and run:  ./build.sh && ./fbbench
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>

/* ---- the fixed addresses fb.c hardcodes --------------------------------
 * These MUST match the high-RAM map at the top of fb.c, including the sizes:
 * fb.c now decides whether a mode fits by subtracting one base from the next,
 * so mapping a different amount here would make the harness disagree with the
 * kernel about what degrades. Re-read that comment block if this ever fails. */
#define BG_ADDR   0x08000000UL   /* bg_buf - 128 MiB, ceiling = sp_buf   */
#define SP_ADDR   0x0A000000UL   /* sp_buf - 160 MiB, ceiling = sched.c  */
#define BACK_ADDR 0x0C000000UL   /* back   - 192 MiB, ceiling = nvme.c   */
#define BG_SIZE   (SP_ADDR    - BG_ADDR)      /* 32 MiB */
#define SP_SIZE   (0x0B000000UL - SP_ADDR)    /* 16 MiB */
#define BACK_SIZE (0x0D000000UL - BACK_ADDR)  /* 16 MiB */

/* what fb.c will decide, by the same arithmetic */
static int fits_back(int w, int h) { return (unsigned long)w * h * 4 <= BACK_SIZE; }

/* ---- fb.c's public surface --------------------------------------------- */
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_shadow(int x, int y, int w, int h, int off, int soft);
void fb_shade(int x, int y, int w, int h, int num, int den);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
void fb_icon24(int px, int py, int n, unsigned int fg);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_present(void);
void fb_at(int row, int col, const char *s, unsigned char attr);
void fb_bg_snapshot(void);
void fb_clip(int x, int y, int w, int h);
void fb_damage(int x, int y, int w, int h);
int  fb_damage_count(void);
unsigned int fb_damage_area(void);
void fb_clip_none(void);
void fb_bg_restore(int x, int y, int w, int h);
void fb_grab(int x, int y, int w, int h);
void fb_stamp(int x, int y);
unsigned int fb_get_px(int x, int y);
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);
int  fb_cell_w(void);
void fb_set_subpixel(int on);
int  fb_get_subpixel(void);

/* The one symbol fb.c calls OUT to. fb_setup() tells the mouse ISR how big the
 * screen is (idt.c is built -mgeneral-regs-only, so it cannot ask). There is no
 * mouse here, so it is a stub - but it has to exist or the harness will not
 * link, and "fbbench does not build" is indistinguishable from "fb.c is
 * broken" at exactly the moment you want to tell those apart. */
void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }

/* fb_setup() reports the mode it took and whether the back buffer survived it.
 * That goes through the kernel's one character sink; here it is stdout, so the
 * harness prints exactly what the boot log would. */
void zl_putc_pub(char c) { fputc(c, stdout); }

/* ---- timing ------------------------------------------------------------ */
static inline uint64_t rdtsc(void)
{
    unsigned lo, hi;
    __asm__ volatile("lfence; rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static double tsc_hz = 0.0;

static void calibrate(void)
{
    struct timespec a, b;
    uint64_t t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &a);
    t0 = rdtsc();
    struct timespec nap = { 0, 200000000L };      /* 200 ms */
    nanosleep(&nap, NULL);
    t1 = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &b);
    double secs = (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
    tsc_hz = (double)(t1 - t0) / secs;
}

/* Run fn REPS times, keep the FASTEST - the minimum is the cleanest estimate
 * of the real cost, since every source of noise (interrupts, migration,
 * another process) can only make a run slower, never faster. */
#define REPS 7

typedef void (*benchfn)(void);

static void bench(const char *name, benchfn fn, long pixels)
{
    uint64_t best = ~0ULL;
    for (int i = 0; i < REPS; i++) {
        uint64_t t0 = rdtsc();
        fn();
        uint64_t t1 = rdtsc();
        if (t1 - t0 < best) best = t1 - t0;
    }
    double ms  = (double)best / tsc_hz * 1000.0;
    double cpp = pixels ? (double)best / (double)pixels : 0.0;
    if (pixels)
        printf("  %-34s %9.3f ms   %8.2f cyc/px   (%ld px)\n",
               name, ms, cpp, pixels);
    else
        printf("  %-34s %9.3f ms\n", name, ms);
}

/* ---- the pixel-identity check -------------------------------------------
 * Several changes in this renderer are supposed to be INVISIBLE: a clip
 * rectangle set to the full screen, a damage list that merges to the same
 * region, a SIMD blend that computes the same value. "Looks the same in a
 * screenshot" cannot tell a one-pixel clamp error from a correct change, and a
 * one-pixel clamp error is exactly the failure mode those changes have.
 *
 * So render a fixed scene and FNV-1a the result. The number either matches the
 * recorded one or it does not, and if it does not, the change moved a pixel.
 * VRAM is hashed rather than the back buffer because it is what the screen
 * actually shows, and it is the one buffer the back-buffered path and the
 * straight-to-VRAM path both end up agreeing on.
 */
static uint64_t fnv1a(const void *p, size_t n)
{
    const unsigned char *b = (const unsigned char *)p;
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; i++) { h ^= b[i]; h *= 1099511628211ULL; }
    return h;
}

/* ---- the scenarios ----------------------------------------------------- */
static int W, H;
static unsigned char *vram_p;

static void b_fill(void)     { fb_fill_px(0, 0, W, H, 0x1B2340); }
static void b_gradient(void) { fb_gradient(0, 0, W, H, 0x141A2E, 0x2A3350); }
/* fb_present() early-returns when nothing is dirty, so it must be measured
 * together with something that dirties the whole screen. Subtract the "fill
 * whole screen" row above to get the blit on its own. */
static void b_present(void)  { fb_fill_px(0, 0, W, H, 0x1B2340); fb_present(); }
static void b_snapshot(void) { fb_bg_snapshot(); }

static void b_shadow(void)   { fb_shadow(200, 200, 600, 460, 16, 12); }
static void b_rrect(void)    { fb_rrect(200, 200, 600, 460, 10, 0x1E2A44); }
static void b_icons(void)
{
    for (int i = 0; i < 10; i++) fb_icon24(100 + i * 60, 100, i, 0xC8D4EC);
}
static void b_text_aa(void)
{
    for (int i = 0; i < 40; i++)
        fb_text_aa(100, 100 + i * 20,
                   "the quick brown fox jumps over the lazy dog 0123456789", 0xE4EDFF);
}
/* The proportional path draws the full CELL per glyph - 30px wide at title
 * size - even when the advance is 8. Coverage is zero outside the ink so it is
 * correct, but it is up to 3.7x the pixels touched for a narrow glyph, and a
 * label-heavy UI draws a lot of narrow glyphs. Worth a number rather than a
 * shrug. */
static void b_text_prop(void)
{
    for (int i = 0; i < 40; i++)
        fb_text_prop(100, 100 + i * 20,
                     "the quick brown fox jumps over the lazy dog 0123456789", 0xE4EDFF);
}

static void b_console(void)
{
    for (int r = 0; r < 40; r++)
        fb_at(r, 0, "the quick brown fox jumps over the lazy dog 0123456789", 0x07);
}
static void b_lines(void)
{
    for (int i = 0; i < 200; i++) fb_line(100, 100 + i, 900, 300 + i, 0x55FFFF);
}

/* one window: shadow + two rrects + title gradient + a label. This is what
 * draw_window() in kernel.zl actually does. */
static void b_window(void)
{
    fb_shadow(200, 200, 600, 460, 16, 12);
    fb_rrect(200, 200, 600, 460, 10, 0x141A2A);
    fb_rrect(201, 201, 598, 458, 8, 0x1E2A44);
    fb_gradient(204, 203, 592, 51, 0x305CA8, 0x16285C);
    fb_text_aa(224, 214, "System Monitor", 0xEAF3FF);
}

/* the whole desktop, once: wallpaper + 3 windows + a dock strip */
static void b_desktop(void)
{
    fb_gradient(0, 0, W, H, 0x141A2E, 0x2A3350);
    for (int i = 0; i < 3; i++) {
        int x = 120 + i * 90, y = 120 + i * 70;
        fb_shadow(x, y, 600, 460, 16, 12);
        fb_rrect(x, y, 600, 460, 10, 0x141A2A);
        fb_rrect(x + 1, y + 1, 598, 458, 8, 0x1E2A44);
        fb_gradient(x + 4, y + 3, 592, 51, 0x305CA8, 0x16285C);
        fb_text_aa(x + 24, y + 14, "System Monitor", 0xEAF3FF);
    }
    fb_gradient(0, H - 128, W, 128, 0x282E42, 0x121420);
    for (int i = 0; i < 10; i++) fb_icon24(236 + i * 112, H - 108, i, 0xC8D4EC);
}

/* The scene the hash is taken over. Every primitive that has a correctness
 * risk appears at least once, and nothing in it is random or timing-dependent.
 * Keep it STABLE: changing the scene changes the number, and then the number
 * proves nothing about the change you were actually testing. */
static void scene(void)
{
    fb_gradient(0, 0, W, H, 0x141A2E, 0x2A3350);
    for (int i = 0; i < 3; i++) {
        int x = 120 + i * 90, y = 120 + i * 70;
        fb_shadow(x, y, 600, 460, 16, 12);
        fb_rrect(x, y, 600, 460, 10, 0x141A2A);
        fb_rrect(x + 1, y + 1, 598, 458, 8, 0x1E2A44);
        fb_gradient(x + 4, y + 3, 592, 51, 0x305CA8, 0x16285C);
        fb_text_aa(x + 24, y + 14, "System Monitor", 0xEAF3FF);
    }
    fb_gradient(0, H - 128, W, 128, 0x282E42, 0x121420);
    for (int i = 0; i < 10; i++) fb_icon24(236 + i * 112, H - 108, i, 0xC8D4EC);
    for (int i = 0; i < 8; i++) fb_line(300, 900 + i * 5, 900, 700 + i * 11, 0x55FFFF);
    fb_shade(1400, 200, 300, 200, 3, 5);
    fb_box(1400, 200, 300, 200, 0x55FF55);
    for (int r = 0; r < 6; r++)
        fb_at(r, 0, "the quick brown fox 0123456789", 0x07);
}

/* Does the DRAG machinery survive this mode?
 *
 * Dragging goes through a second pair of fixed buffers, bg_buf and sp_buf,
 * each of which had its own compile-time PIXEL ceiling - and bg_buf's was
 * 1920x1200, so at 2560x1440 bg_ok went to 0 and every drag became a silent
 * no-op. That is a different failure from the back buffer's and it needs its
 * own check, because "the desktop draws" does not test it at all.
 *
 * bg_ok and sp_ok are static, so this asks functionally instead: snapshot,
 * scribble, restore, and see whether the scribble went away. A refused
 * snapshot makes fb_bg_restore a no-op and the scribble stays. Same for the
 * sprite: grab a patch, stamp it somewhere else, and look for it there. */
static int drag_check(void)
{
    const unsigned MARK = 0x00FF00FF, WALL = 0x00203040;
    int ok = 1;

    fb_fill_px(0, 0, W, H, WALL);
    fb_bg_snapshot();
    fb_fill_px(100, 100, 200, 200, MARK);
    fb_bg_restore(100, 100, 200, 200);
    if (fb_get_px(150, 150) != WALL) {
        printf("  %-34s FAIL - bg_restore did not undo the scribble\n",
               "drag: background snapshot");
        ok = 0;
    } else {
        printf("  %-34s ok\n", "drag: background snapshot");
    }

    /* a window-sized sprite: the System Monitor at ui()==2 is 568x428 */
    fb_fill_px(400, 400, 568, 428, MARK);
    fb_grab(400, 400, 568, 428);
    fb_bg_restore(400, 400, 568, 428);
    fb_stamp(900, 500);
    if (fb_get_px(900 + 284, 500 + 214) != MARK) {
        printf("  %-34s FAIL - the sprite did not land\n", "drag: window sprite");
        ok = 0;
    } else {
        printf("  %-34s ok\n", "drag: window sprite");
    }
    return ok;
}

/* Does the scissor actually scissor?
 *
 * The full-screen identity hash proves the clip did not BREAK anything. It
 * cannot prove the clip WORKS - a fb_clip() that did nothing at all would pass
 * it perfectly. So drive every primitive at a rectangle that is deliberately
 * far bigger than the scissor and check both halves of the claim: the pixel
 * just inside changed, the pixel just outside did not.
 *
 * Corners matter more than edges here. An off-by-one in a clamp usually shows
 * at exactly one boundary, so each of the four is tested on its own rather
 * than sampling the middle and calling it clipped. */
static int clip_check(void)
{
    const unsigned WALL = 0x00112233, INK = 0x00FF7700;
    const int cx = 300, cy = 200, cw = 400, ch = 300;
    int bad = 0;

    fb_clip_none();
    fb_fill_px(0, 0, W, H, WALL);

    fb_clip(cx, cy, cw, ch);
    fb_fill_px(0, 0, W, H, INK);            /* the whole screen, scissored */
    fb_clip_none();

    /* inside, at each corner and the middle */
    if (fb_get_px(cx, cy) != INK)                     { printf("  clip: top-left inside not drawn\n");     bad++; }
    if (fb_get_px(cx + cw - 1, cy) != INK)            { printf("  clip: top-right inside not drawn\n");    bad++; }
    if (fb_get_px(cx, cy + ch - 1) != INK)            { printf("  clip: bottom-left inside not drawn\n");  bad++; }
    if (fb_get_px(cx + cw - 1, cy + ch - 1) != INK)   { printf("  clip: bottom-right inside not drawn\n"); bad++; }
    if (fb_get_px(cx + cw / 2, cy + ch / 2) != INK)   { printf("  clip: centre not drawn\n");              bad++; }
    /* one pixel outside, on all four sides */
    if (fb_get_px(cx - 1, cy) != WALL)                { printf("  clip: LEAKED left\n");   bad++; }
    if (fb_get_px(cx + cw, cy) != WALL)               { printf("  clip: LEAKED right\n");  bad++; }
    if (fb_get_px(cx, cy - 1) != WALL)                { printf("  clip: LEAKED above\n");  bad++; }
    if (fb_get_px(cx, cy + ch) != WALL)               { printf("  clip: LEAKED below\n");  bad++; }

    /* every primitive, not just the fill: a rounded rect, a shadow, a
     * gradient, text and an icon, all drawn far outside the scissor. Nothing
     * of any of them may land beyond it. */
    fb_clip_none();
    fb_fill_px(0, 0, W, H, WALL);
    fb_clip(cx, cy, cw, ch);
    fb_gradient(0, 0, W, H, 0x808080, 0x404040);
    fb_rrect(cx - 200, cy - 150, cw + 400, ch + 300, 20, 0x00AA00);
    fb_shadow(cx - 100, cy - 100, cw + 200, ch + 200, 16, 12);
    fb_text_aa(cx - 180, cy - 40, "clipped text should not appear", 0xFFFFFF);
    fb_icon24(cx - 60, cy + ch + 10, 0, 0xFFFFFF);
    fb_line(0, 0, W - 1, H - 1, 0x00FFFF);
    fb_clip_none();

    int leaks = 0;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++) {
            int in = (x >= cx && x < cx + cw && y >= cy && y < cy + ch);
            if (!in && fb_get_px(x, y) != WALL) leaks++;
        }
    if (leaks) { printf("  clip: %d pixels ESCAPED the scissor\n", leaks); bad++; }

    printf("  %-34s %s\n", "clip: scissor holds",
           bad ? "FAIL" : "ok  (every primitive, all four edges)");
    return !bad;
}

/* Is the damage list actually smaller than the box it replaced?
 *
 * The single dirty box grew to enclose everything touched, so a clock in one
 * corner and a monitor in the other unioned to the whole screen. The claim is
 * that eight rectangles keep them apart. That is a NUMBER, not an opinion, and
 * DECISIONS.md #25 is about exactly this: an argument is not a measurement.
 *
 * The old behaviour is recoverable for comparison without keeping the old code
 * around - it is the bounding box of the new list, because that is precisely
 * what unioning everything into one would have produced. */
static void damage_check(void)
{
    if (!fits_back(W, H)) {
        /* No back buffer means no blit to shrink: drawing already went
         * straight to VRAM, so the damage list is legitimately empty. Saying
         * "0 rects" here without saying why reads as a failure. */
        printf("  %-34s n/a - no back buffer, drawing goes straight to VRAM\n",
               "damage list");
        return;
    }
    struct { const char *what; int n; int corners[4][4]; } scenes[] = {
        { "two corners (clock + monitor)", 2,
          { { 20, 20, 200, 60 }, { W - 320, H - 90, 300, 70 } } },
        { "four corners", 4,
          { { 0, 0, 200, 60 }, { W - 200, 0, 200, 60 },
            { 0, H - 60, 200, 60 }, { W - 200, H - 60, 200, 60 } } },
    };

    for (unsigned s = 0; s < sizeof scenes / sizeof scenes[0]; s++) {
        fb_present();                       /* start from an empty list */
        for (int i = 0; i < scenes[s].n; i++) {
            const int *r = scenes[s].corners[i];
            fb_fill_px(r[0], r[1], r[2], r[3], 0x00304050);
            /* text too, so the per-pixel accumulator is in play, not just the
             * rect primitives - that is the half most likely to over-merge */
            fb_text_aa(r[0] + 4, r[1] + 4, "12:04:55", 0x00E4EDFF);
        }

        int n = fb_damage_count();
        unsigned area = fb_damage_area();

        /* what the single box would have been: the bounding box of the lot */
        long bx0 = W, by0 = H, bx1 = 0, by1 = 0;
        for (int i = 0; i < scenes[s].n; i++) {
            const int *r = scenes[s].corners[i];
            if (r[0] < bx0) bx0 = r[0];
            if (r[1] < by0) by0 = r[1];
            if (r[0] + r[2] > bx1) bx1 = r[0] + r[2];
            if (r[1] + r[3] > by1) by1 = r[1] + r[3];
        }
        long boxed = (bx1 - bx0) * (by1 - by0);

        printf("  %-34s %d rects, %u px   (one box: %ld px, %.1fx more)\n",
               scenes[s].what, n, area, boxed,
               area ? (double)boxed / (double)area : 0.0);
        if (n < 2)
            printf("       ^ MERGED into one - the list is not separating them\n");

        /* Does present actually blit EVERY rectangle? The scene hash cannot
         * answer that: its first call is a full-screen gradient, so it always
         * presents as one rect and would pass with a present() that only ever
         * blitted dmg[0]. Zero VRAM, present, and look. */
        memset(vram_p, 0, (size_t)W * H * 4);
        fb_present();
        int missing = 0, bled = 0;
        for (int i = 0; i < scenes[s].n; i++) {
            const int *r = scenes[s].corners[i];
            const unsigned char *p = vram_p + ((size_t)(r[1] + r[3] / 2) * W
                                             + (r[0] + r[2] - 2)) * 4;
            if (!(p[0] | p[1] | p[2])) missing++;
        }
        /* the middle of the screen was never drawn, so it must still be zero */
        const unsigned char *mid = vram_p + ((size_t)(H / 2) * W + W / 2) * 4;
        if (mid[0] | mid[1] | mid[2]) bled = 1;
        printf("  %-34s %s\n", "  ...and present blits them all",
               missing ? "FAIL - a damaged rectangle never reached VRAM"
                       : bled ? "FAIL - present wrote outside the damage"
                              : "ok");
    }
    fb_present();
}

static void hash_report(void)
{
    memset(vram_p, 0, (size_t)W * H * 4);
    scene();
    /* how many rectangles this scene actually presented as. If it is 1 the
     * hash proves nothing about the multi-rect blit path, so say the number. */
    int n = fb_damage_count();
    unsigned area = fb_damage_area();
    fb_present();
    printf("  %-34s FNV %016llx   (%d rects, %u px presented)\n",
           "scene hash (VRAM)",
           (unsigned long long)fnv1a(vram_p, (size_t)W * H * 4), n, area);
}

/* ---- driver ------------------------------------------------------------ */
static void run_at(int w, int h, unsigned long vram)
{
    W = w; H = h;
    vram_p = (unsigned char *)vram;
    printf("\n=== %dx%d  (%ld px)   back buffer: %s ===\n",
           w, h, (long)w * h, fits_back(w, h) ? "ON" : "OFF  <-- degraded");
    /* fb_setup prints its own verdict - the same line the boot log gets */
    fb_setup(vram, (unsigned)w * 4, (unsigned)w, (unsigned)h, 32);
    printf("  subpixel flag: %d   cell: %dpx\n", fb_get_subpixel(), fb_cell_w());

    long px = (long)w * h;
    bench("fill whole screen",        b_fill,     px);
    bench("gradient whole screen",    b_gradient, px);
    bench("fill + present (blit)",     b_present,  px);
    bench("bg_snapshot",              b_snapshot, px);
    printf("\n");
    bench("shadow 600x460 soft=12",   b_shadow,   (600 + 24L) * (460 + 24));
    bench("rrect 600x460 r=10",       b_rrect,    600L * 460);
    bench("10 dock icons",            b_icons,    10L * 48 * 48);
    bench("40 lines of AA text (mono)",b_text_aa,  0);
    bench("40 lines of PROPORTIONAL",  b_text_prop, 0);
    bench("40 rows console text",     b_console,  0);
    bench("200 diagonal lines",       b_lines,    0);
    printf("\n");
    bench("ONE WINDOW (full chrome)", b_window,   0);
    bench("WHOLE DESKTOP redraw",     b_desktop,  px);
    printf("\n");
    drag_check();
    clip_check();
    damage_check();
    hash_report();
}

int main(void)
{
    /* map the three fixed scratch buffers fb.c expects at physical addresses */
    struct { unsigned long a; unsigned long n; const char *what; } bufs[] = {
        { BG_ADDR,   BG_SIZE,   "bg_buf"   },
        { SP_ADDR,   SP_SIZE,   "sp_buf"   },
        { BACK_ADDR, BACK_SIZE, "back"     },
    };
    for (unsigned i = 0; i < 3; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p == MAP_FAILED || p != (void *)bufs[i].a) {
            fprintf(stderr, "cannot map %s at 0x%lx - fb.c hardcodes it\n",
                    bufs[i].what, bufs[i].a);
            return 1;
        }
        memset(p, 0, bufs[i].n);
    }

    /* a stand-in for video memory. Ordinary RAM, so the present blit here is
     * FASTER than on real hardware, where it crosses PCIe into a
     * write-combining region. Treat present as a floor, not a measurement. */
    void *vram = mmap(NULL, 64UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (vram == MAP_FAILED) { perror("vram"); return 1; }
    memset(vram, 0, 64UL << 20);

    calibrate();
    printf("fbbench - the shipping fb.c, timed on this CPU\n");
    printf("TSC %.3f GHz   (min of %d runs; cycles are the real number)\n",
           tsc_hz / 1e9, REPS);

    run_at(1920, 1200, (unsigned long)vram);   /* back buffer ON               */
    run_at(2560, 1440, (unsigned long)vram);   /* the ThinkPad panel - ON now, */
                                               /* OFF before desktop-TODO 0a   */
    run_at(3840, 2160, (unsigned long)vram);   /* 4K: still OFF, and it says so */

    printf("\nnote: 'present' writes to ordinary RAM here. On the real machine\n");
    printf("it crosses PCIe into write-combining VRAM, so it is a FLOOR.\n");
    return 0;
}
