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

/* ---- the fixed addresses fb.c hardcodes -------------------------------- */
#define BG_ADDR   0x08000000UL   /* bg_buf   - 128 MiB */
#define SP_ADDR   0x0A000000UL   /* sp_buf   - 160 MiB */
#define BACK_ADDR 0x0C000000UL   /* back     - 192 MiB */
#define BG_SIZE   (16UL << 20)
#define SP_SIZE   (8UL  << 20)
#define BACK_SIZE (48UL << 20)   /* enough for 4K at 4 bytes per pixel */

/* ---- fb.c's public surface --------------------------------------------- */
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_shadow(int x, int y, int w, int h, int off, int soft);
void fb_shade(int x, int y, int w, int h, int num, int den);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
void fb_icon24(int px, int py, int n, unsigned int fg);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_present(void);
void fb_at(int row, int col, const char *s, unsigned char attr);
void fb_bg_snapshot(void);
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

static void hash_report(void)
{
    memset(vram_p, 0, (size_t)W * H * 4);
    scene();
    fb_present();                 /* no-op when back_on == 0; drawing already
                                     went straight to VRAM in that case */
    printf("  %-34s FNV %016llx\n", "scene hash (VRAM)",
           (unsigned long long)fnv1a(vram_p, (size_t)W * H * 4));
}

/* ---- driver ------------------------------------------------------------ */
static void run_at(int w, int h, unsigned long vram)
{
    W = w; H = h;
    vram_p = (unsigned char *)vram;
    fb_setup(vram, (unsigned)w * 4, (unsigned)w, (unsigned)h, 32);

    /* fb.c decides back_on internally from BACK_MAX; report what it chose by
     * timing a readback-heavy op is unreliable, so infer it the same way fb.c
     * does and state it. BACK_MAX is 1920*1200. */
    int back_on = (w * h) <= (1920 * 1200);

    printf("\n=== %dx%d  (%ld px)   back buffer: %s ===\n",
           w, h, (long)w * h, back_on ? "ON" : "OFF  <-- degraded");
    if (!back_on)
        printf("  subpixel text OFF, get_px reads VRAM, dragging disabled\n");
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
    bench("40 lines of AA text",      b_text_aa,  0);
    bench("40 rows console text",     b_console,  0);
    bench("200 diagonal lines",       b_lines,    0);
    printf("\n");
    bench("ONE WINDOW (full chrome)", b_window,   0);
    bench("WHOLE DESKTOP redraw",     b_desktop,  px);
    printf("\n");
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

    run_at(1920, 1200, (unsigned long)vram);   /* back buffer ON  */
    run_at(2560, 1440, (unsigned long)vram);   /* back buffer OFF */

    printf("\nnote: 'present' writes to ordinary RAM here. On the real machine\n");
    printf("it crosses PCIe into write-combining VRAM, so it is a FLOOR.\n");
    return 0;
}
