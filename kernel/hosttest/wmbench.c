/* wmbench.c - time a compositor FRAME on the host, with a cycle counter.
 *
 * The stopwatch problem, stated plainly: wm.c times itself with the TSC, but
 * reading that number means booting QEMU, and this box has two sessions on
 * four cores. Measured drag cost went 19,399 us at host load 2.25 and 16,000
 * us at load 7.43 - which is not an A/B, it is two different machines. Every
 * re-run since has landed at load 5-7. CLAUDE.md already records a bisect lost
 * to exactly this.
 *
 * fbbench solved the same problem for fb.c by compiling the shipping source
 * and timing it here. This does it for a FRAME: wm.c + ui.c + fb.c + input.c +
 * term.c, the real ones, driven through wm_frame() with fake hardware, timed
 * with rdtsc and reported best-of-N. No QEMU, no boot, and a cycle count that
 * a busy host perturbs by cache pressure rather than by an order of magnitude.
 *
 * IT ALSO ATTRIBUTES. A frame is not one number - it is a wallpaper blit, some
 * chrome, and one app_draw per window that the damage touches. Timing the same
 * interaction with pieces disabled is the only way to say WHERE 19 ms went
 * rather than that it went.
 *
 *   ./wmbench            the standard set
 *   ./wmbench 2560 1440  at another mode
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>

#include "../ui.h"
#include "../../runtime.h"

void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
void fb_icon24(int px, int py, int n, unsigned int fg);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
void fb_present(void);
unsigned int fb_get_px(int x, int y);
int  fb_cell_h(void);
int  fb_wall_save(void);
void fb_wall_paint(int x, int y, int w, int h);
int  fb_wall_ok(void);

/* term.c, the real one - this is the file the drag turned out to be about */
void term_putc(char c);
void term_draw(int x, int y, int w, int h, unsigned int fg, unsigned int dim,
               unsigned int accent, int cursor_on);

/* ---- fake hardware -------------------------------------------------------- */
static int fake_x = 900, fake_y = 400, fake_btn = 0;
static unsigned fake_ticks = 1;
int idt_mouse_x(void)   { return fake_x; }
int idt_mouse_y(void)   { return fake_y; }
int idt_mouse_btn(void) { return fake_btn; }
/* the scroll wheel: read-and-clear, so a harness with no wheel must return
 * 0 rather than a stale notch (desktop/feel-and-control added this). */
int idt_mouse_wheel(void) { return 0; }
unsigned int idt_ticks(void) { return fake_ticks; }
int idt_scan(void)      { return 0; }
int xhci_key(void)      { return 0; }
int ser_rx(void)        { return -1; }
int xhci_ptr_ready(void) { return 0; }
int xhci_ptr_poll(void)  { return 0; }
int xhci_poll(int max)   { (void)max; return 0; }  /* the one ring drainer */
int xhci_ptr_abs(void)   { return 0; }   /* no USB pointer here at all */
int xhci_ptr_take_dx(void) { return 0; }
int xhci_ptr_take_dy(void) { return 0; }
int xhci_ptr_x(void)     { return 0; }
int xhci_ptr_y(void)     { return 0; }
int xhci_ptr_btn(void)   { return 0; }
static unsigned int fake_tsc;
unsigned int cpu_tsc_lo(void)  { return (fake_tsc += 1000); }
unsigned int cpu_tsc_khz(void) { return 2300000u; }
void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void zl_putc_pub(char c) { term_putc(c); }
Value zl_num(double n) { Value v; memset(&v, 0, sizeof v); v.type = V_NUM; v.num = n; return v; }

static int W = 1920, H = 1200;

/* ---- the clock ------------------------------------------------------------ */
static inline unsigned long long rdtsc(void)
{
    unsigned lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)hi << 32) | lo;
}
static double tsc_hz;
static void calibrate(void)
{
    struct timespec a, b;
    clock_gettime(CLOCK_MONOTONIC, &a);
    unsigned long long t0 = rdtsc();
    struct timespec s = { 0, 60000000 };            /* 60 ms */
    nanosleep(&s, 0);
    unsigned long long t1 = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &b);
    double secs = (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
    tsc_hz = (double)(t1 - t0) / secs;
}

/* ---- the apps. Deliberately the SHAPES kernel.zl uses, not ui_* ------------
 * wmshot's apps go through ui_* because it is a worked example of the app
 * contract. This one has to match what actually ships, because what actually
 * ships is what is slow: a terminal that redraws from a scrollback, and a
 * monitor that walks a 48-character string glyph by glyph and draws eight
 * antialiased sparkline segments. */
#define APP_SHELL   0
#define APP_MONITOR 1
#define APP_ABOUT   2

static int draw_term = 1, draw_mon = 1;

static void app_draw(int app, int x, int y, int w, int h, int focused)
{
    const struct ui_theme *t = ui_theme();
    (void)focused;
    if (app == APP_SHELL) {
        if (!draw_term) return;
        term_draw(x + UI_S2(t), y + UI_S1(t), w - UI_S4(t), h - UI_S3(t),
                  t->text, t->text_dim, t->accent, 1);
        return;
    }
    if (app == APP_MONITOR) {
        if (!draw_mon) return;
        /* the 48-character CPU brand string, glyph by glyph, as kernel.zl does */
        static const char *brand = "Intel(R) Core(TM) i7-10510U CPU @ 1.80GHz";
        fb_text_aa(x + UI_S3(t), y + UI_S4(t), brand, t->text);
        fb_fill_px(x + UI_S3(t), y + UI_S6(t) * 2, w - UI_S6(t), UI_S3(t), t->panel_hi);
        fb_fill_px(x + UI_S3(t), y + UI_S6(t) * 2, (w - UI_S6(t)) / 3, UI_S3(t), t->accent);
        /* eight antialiased sparkline segments - Wu, 4.3x Bresenham per pixel */
        int sy = y + h - UI_S6(t) * 2, sx = x + UI_S3(t), step = (w - UI_S6(t)) / 8;
        static const int pts[9] = { 40, 12, 30, 4, 26, 8, 34, 2, 20 };
        for (int i = 0; i < 8; i++)
            fb_line(sx + i * step, sy - pts[i], sx + (i + 1) * step, sy - pts[i + 1],
                    t->accent);
        return;
    }
    fb_text_aa(x + UI_S3(t), y + UI_S4(t), "zlOS 0.3", t->text);
}

static int app_event(int a, int w, int t, int c, int x, int y)
{ (void)a;(void)w;(void)t;(void)c;(void)x;(void)y; return 1; }
static int app_tick(int a, int w) { (void)a;(void)w; return 0; }

/* the wallpaper, exactly as kernel.zl does it: a cached bitmap blit */
static int desk_calls;
static void desk_draw(int x, int y, int w, int h)
{
    desk_calls++;
    if (fb_wall_ok()) { fb_wall_paint(x, y, w, h); return; }
    fb_gradient(0, 0, W, H, 0x141A2E, 0x0A0E18);
}

/* ---- the runs -------------------------------------------------------------
 * INTERLEAVED, not one scenario at a time, and that is not fussiness.
 *
 * The first version ran each scenario to completion in turn, and two runs of
 * it minutes apart reported 12,061 and 20,210 us/frame for the same code -
 * a 70% swing - because a second session shares this four-core box and its
 * load drifts. rdtsc counts at a CONSTANT rate regardless of core frequency,
 * so a contended or downclocked core inflates the cycle count for identical
 * work: a cycle counter is not immune to a busy machine, it is only immune to
 * QEMU.
 *
 * Running all four scenarios inside each repetition makes them share the same
 * span of time, so whatever the machine is doing it does to all of them. The
 * DIFFERENCES then mean something even when the absolutes drift, and the
 * differences are the whole point of an attribution. desktop-TODO 0c used the
 * same interleaved A/B to prove the damage list cost nothing.
 */
#define REPS 7
#define SCENARIOS 4

static unsigned long long best[SCENARIOS];

static unsigned long long one_drag(int win, int steps)
{
    wm_move(win, 1260, 100);
    for (int i = 0; i < 4; i++) { fake_ticks++; wm_frame(); }
    unsigned long long t0 = rdtsc();
    for (int i = 0; i < steps; i++) {
        wm_move(win, 1260 - i * 90, 100 + i * 40);
        fake_ticks++;
        wm_frame();
    }
    return rdtsc() - t0;
}

static void run_all(int win, int steps)
{
    static const struct { int term, mon; } scn[SCENARIOS] = {
        {1,1}, {0,1}, {1,0}, {0,0}
    };
    for (int i = 0; i < SCENARIOS; i++) best[i] = ~0ULL;
    for (int r = 0; r < REPS; r++)
        for (int i = 0; i < SCENARIOS; i++) {
            draw_term = scn[i].term;
            draw_mon  = scn[i].mon;
            unsigned long long dt = one_drag(win, steps);
            if (dt < best[i]) best[i] = dt;
        }
}

/* One attributed component. A delta at or below zero means the component is
 * cheaper than the run-to-run spread, which is a finding and not an error. */
static void attribute(const char *what, long long dcyc, unsigned long long all,
                      int frames)
{
    if (dcyc <= 0) {
        printf("    %-24s %s\n", what,
               "below the noise floor - drawing it measured no slower");
        return;
    }
    printf("    %-24s %7.2f us   (%2.0f%%)\n", what,
           (double)dcyc / tsc_hz * 1e6 / frames, 100.0 * dcyc / (double)all);
}

static void report(const char *what, unsigned long long cyc, int frames)
{
    double us = cyc / tsc_hz * 1e6 / frames;
    printf("  %-44s %8.0f cyc/frame  %7.2f us/frame\n",
           what, (double)cyc / frames, us);
}


/* input.c's USB keyboard path takes RAW HID EVENTS now, not decoded chars
 * (claude/ecstatic-lewin-f617bb - there is no character for Up, so a decoded
 * arrow came back 0 and 0 means "nothing typed"). These harnesses drive the
 * pointer, so no USB keyboard is present. */
int xhci_key_event(void) { return 0; }
int xhci_kbd_mods(void)  { return 0; }

/* term.c writes COM1 directly - term_say() tees to the scrollback AND the
 * serial log, because gates grep that log and the console's pixels are
 * muted under the compositor. This harness has no UART. */
void zl_serial_putc(char c) { (void)c; }

int main(int argc, char **argv)
{
    if (argc > 2) { W = atoi(argv[1]); H = atoi(argv[2]); }

    struct { unsigned long a, n; } bufs[] = {
        { 0x08000000UL, 48UL << 20 }, { 0x0C000000UL, 16UL << 20 },
    };
    for (unsigned i = 0; i < 2; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p != (void *)bufs[i].a) { fprintf(stderr, "mmap failed\n"); return 1; }
        memset(p, 0, bufs[i].n);
    }
    void *vram = mmap(NULL, 128UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    fb_setup((unsigned long)vram, (unsigned)W * 4, (unsigned)W, (unsigned)H, 32);
    calibrate();

    ui_theme_init(2);
    wm_init();
    wm_hooks(app_draw, app_event, app_tick, desk_draw);

    /* a realistic scrollback - this is what makes term_draw cost anything */
    for (int i = 0; i < 40; i++) {
        const char *l = "  [  OK  ] APIC: IRQs via I/O APIC at 0xFEC00000, 4 CPU(s)";
        while (*l) term_putc(*l++);
        term_putc('\n');
    }

    /* bake the wallpaper, as wm_boot_start does */
    fb_gradient(0, 0, W, H, 0x141A2E, 0x0A0E18);
    fb_wall_save();

    const struct ui_theme *t = ui_theme();
    int hb = t->title_h + UI_S1(t);
    wm_open(APP_SHELL,   "zl shell   ~",   UI_S6(t), hb + UI_S6(t), 1180, 880);
    int mon = wm_open(APP_MONITOR, "System Monitor", 1260, hb + UI_S6(t), 568, 428);
    wm_open(APP_ABOUT,   "About",          1260, hb + UI_S6(t) + 450, 568, 244);
    for (int i = 0; i < 10; i++) { fake_ticks++; wm_frame(); }

    printf("wmbench - a compositor FRAME, timed on this CPU, no QEMU\n");
    printf("TSC %.3f GHz   %dx%d   best of %d\n\n", tsc_hz / 1e9, W, H, REPS);

    const int STEPS = 12;
    printf("DRAGGING THE SYSTEM MONITOR ACROSS THE SHELL (%d steps)\n", STEPS);
    run_all(mon, STEPS);
    unsigned long long all = best[0], noterm = best[1],
                       nomon = best[2], bare = best[3];
    report("everything", all, STEPS);
    report("...without the shell's scrollback", noterm, STEPS);
    report("...without the monitor's contents", nomon, STEPS);
    report("...chrome and wallpaper only", bare, STEPS);

    /* SIGNED, and a difference below zero is REPORTED rather than hidden.
     *
     * These are unsigned cycle counts and the first version subtracted them
     * directly. When drawing LESS measured slower - which happens whenever a
     * component's cost is inside the noise - the subtraction underflowed and
     * the harness printed "667233241162193 us (8421476900969%)".
     *
     * A benchmark that prints an absurd number is worse than one that prints
     * nothing, because the absurd number is the one somebody quotes. A
     * negative delta is real information: it means this component costs less
     * than the run-to-run spread, and saying so is the answer. */
    printf("\n  attributed, per frame:\n");
    attribute("the shell's scrollback", (long long)all - (long long)noterm, all, STEPS);
    attribute("the monitor's contents", (long long)all - (long long)nomon, all, STEPS);
    attribute("chrome + wallpaper",     (long long)bare,                   all, STEPS);
    printf("\n  budget 16667 us/frame at 60 fps.\n");
    printf("  The PERCENTAGES are the trustworthy part: the four scenarios are\n");
    printf("  interleaved, so the machine treats them alike even as its load\n");
    printf("  drifts. Absolutes move with whatever else is running.\n");
    (void)desk_calls;
    return 0;
}
