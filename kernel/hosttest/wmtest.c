/* wmtest.c - the compositor, asserted. fb.c + input.c + ui.c + wm.c, for real,
 * against fake hardware and a fake app.
 *
 * A compositor's bugs do not look like crashes. They look like a sliver of an
 * old window left on the wallpaper, a click landing on the window underneath,
 * a drag that stops the moment the pointer outruns the frame, or an app
 * quietly painting over its own title bar. Every one of those is invisible in
 * a screenshot taken a frame later, and several are invisible in a screenshot
 * ever, because they only show against some backgrounds.
 *
 * So the whole stack runs here with a cycle-free clock and a scriptable
 * pointer, and the assertions read the back buffer directly.
 *
 * Build and run:  ./build.sh && ./wmtest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "../ui.h"

#define ANIM_SETTLE 20  /* longest wall-clock animation is 18 PIT ticks */

/* ---- fb.c ---------------------------------------------------------------- */
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
unsigned int fb_get_px(int x, int y);
void fb_present(void);
void fb_clip_none(void);
void fb_clip(int x, int y, int w, int h);
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);

/* ---- input.c ------------------------------------------------------------- */
int input_next(void);

/* ---- wmglue.c ------------------------------------------------------------ */
int wm_bind_zl(void);
int wm_available(void);

/* wmglue boxes ints into zl Values to call across the C/zl seam. In the kernel
 * zl_num comes from runtime_kernel.c; here it is the one thing the harness has
 * to supply, the same way it supplies memory for fb.c. */
#include "../../runtime.h"
Value zl_num(double n)
{
    Value v;
    memset(&v, 0, sizeof v);
    v.type = V_NUM;
    v.num = n;
    return v;
}

/* ---- fake hardware ------------------------------------------------------- */
static int fake_x = 10, fake_y = 10, fake_btn = 0;
static unsigned fake_ticks = 1;

int idt_mouse_x(void)   { return fake_x; }
int idt_mouse_y(void)   { return fake_y; }
int idt_mouse_btn(void) { return fake_btn; }
/* the scroll wheel: read-and-clear, so a harness with no wheel must return
 * 0 rather than a stale notch (desktop/feel-and-control added this). */
int idt_mouse_wheel(void) { return 0; }
unsigned int idt_ticks(void) { return fake_ticks; }

/* wm.c times the body of a frame with the TSC (queue item 9). cpu.c is not
 * linked here - this harness is wm.c + fb.c + ui.c + input.c against fake
 * hardware - so the clock is faked too. A fixed rate and a counter that
 * advances by a plausible frame's worth per call keeps wm_frame_us() in range
 * without making any assertion depend on it. */
static unsigned long long fake_tsc = 0;
unsigned long long cpu_tsc(void) { fake_tsc += 20000000; return fake_tsc; }
unsigned int cpu_tsc_khz(void) { return 2000000; }
int idt_scan(void)      { return 0; }
int xhci_key(void)      { return 0; }
int ser_rx(void)        { return -1; }   /* no UART in the harness */

/* THE FRAME TIMER's clock. wm.c times itself with the TSC; the harness has no
 * cpu.c, and a frame measured here would be measuring this machine rather than
 * the guest anyway. Returning a monotonically rising count keeps the timing
 * code on the same path it takes in the kernel - a stub that returned 0 would
 * make wm_frame() take the "TSC unavailable" branch and stop exercising it. */
unsigned int cpu_tsc_lo(void)  { return (unsigned int)cpu_tsc(); }

static int fake_usb_ptr = 0, fake_ux = 0, fake_uy = 0, fake_ubtn = 0;

/* No USB pointer in the harness - which is a case worth being able to express,
 * because it is the PS/2 fallback the laptop's TrackPoint takes. */
int xhci_ptr_ready(void) { return fake_usb_ptr; }
int xhci_ptr_poll(void)  { return 0; }
int xhci_poll(int max)   { (void)max; return 0; }  /* the one ring drainer */
/* When this harness has a USB pointer at all, what it supplies is an
 * ABSOLUTE position - so it is a tablet, and it must say so. input.c no
 * longer infers "absolute" from "a USB pointer exists", because that is
 * exactly what sent every relative usb-mouse down the tablet branch. */
int xhci_ptr_abs(void)   { return 1; }
int xhci_ptr_take_dx(void) { return 0; }
int xhci_ptr_take_dy(void) { return 0; }
int xhci_ptr_x(void)     { return fake_ux; }
int xhci_ptr_y(void)     { return fake_uy; }
int xhci_ptr_btn(void)   { return fake_ubtn; }

void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void zl_putc_pub(char c) { (void)c; }        /* fb.c's boot line: not wanted here */

#define W 1280
#define H 800
/* ONE buffer now. C4 deleted the drag background and sprite, and the back
 * buffer moved down into the space they freed - see the high-RAM map at the
 * top of fb.c. 0x08000000..0x0A800000 is 40 MiB, bounded by the AP stacks. */
#define BACK_ADDR 0x08000000UL
#define BACK_SIZE (0x0A800000UL - BACK_ADDR)     /* 40 MiB */
/* The cached-blur arena, a SECOND fixed mapping. apps-in-windows' memory map
 * had no HI_BLUR - it arrived on overnight-compositor at 192 MiB, in the space
 * `back` vacated. Map only `back` and slot_capture writes to an unmapped page. */
#define BLUR_ADDR 0x0C000000UL
#define BLUR_SIZE (0x0D000000UL - BLUR_ADDR)   /* 16 MiB */

/* ---- the fake app --------------------------------------------------------
 * Fills its whole client area with a colour derived from its id, then
 * deliberately tries to draw a long way OUTSIDE it. The second half is the
 * point: the guarantee fb_clip buys is that an app which draws at -500,-500
 * simply produces nothing. */
#define APP_COLOUR(a) (0x00110000u * (unsigned)(a) + 0x00002200u)
static int draw_calls[8];
static int tick_returns;            /* what app_tick claims each frame */
static int last_event_app = -1, last_event_type, last_event_x, last_event_y;

static void t_draw(int app, int x, int y, int w, int h, int focused)
{
    (void)focused;
    if (app >= 0 && app < 8) draw_calls[app]++;
    fb_fill_px(x, y, w, h, APP_COLOUR(app));
    fb_fill_px(x - 500, y - 500, 400, 400, 0x00FF00FF);   /* must vanish */
    fb_fill_px(x, y - 40, w, 30, 0x00FF00FF);             /* into the title bar */
    fb_fill_px(x + w + 10, y, 200, h, 0x00FF00FF);        /* past the right edge */
}

static int t_event(int app, int win, int type, int code, int x, int y)
{
    (void)win; (void)code;
    last_event_app = app; last_event_type = type;
    last_event_x = x; last_event_y = y;
    return 1;
}

static int t_tick(int app, int win) { (void)app; (void)win; return tick_returns; }

#define WALL 0x00203040u
static void t_desk(int x, int y, int w, int h) { fb_fill_px(x, y, w, h, WALL); }

/* ---- assertions ----------------------------------------------------------- */
static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-56s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}

/* run one frame. The frame loop is gated on the tick so it cannot spin at
 * 100% CPU, so the clock has to move for anything to happen. */
static void frame(void) { fake_ticks++; wm_frame(); }

static void pointer(int x, int y, int btn)
{
    /* DRIVE THE TABLET, NOT THE PS/2 MOUSE, because this helper is absolute:
     * every caller says "the pointer is at (x,y)" and then asserts the app was
     * told exactly that. desktop/feel-and-control made the PS/2 path RELATIVE -
     * it accumulates scaled deltas into its own px_x/px_y - and a clamp at the
     * screen edge permanently offsets that accumulator from the raw position,
     * because the raw side keeps moving and the accumulated side cannot. The
     * "wander off" step below deliberately drives 900 px past the edge, so
     * every assertion after it was reading a pointer that had been silently
     * shifted. A tablet reports an absolute position and is not accelerated,
     * which is what this harness has always meant. inputtest covers the PS/2
     * relative path, and inputtest_feel covers the accel curve. */
    fake_usb_ptr = 1;
    fake_ux = x; fake_uy = y; fake_ubtn = btn;
    fake_x = x; fake_y = y; fake_btn = btn;
    frame();
}

/* is any pixel in this rectangle the "escaped" magenta? */
static int has_magenta(int x0, int y0, int x1, int y1)
{
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
            if (fb_get_px(x, y) == 0x00FF00FF) return 1;
    return 0;
}

static int all_wallpaper(int x0, int y0, int x1, int y1)
{
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
            if (fb_get_px(x, y) != WALL) return 0;
    return 1;
}


/* input.c's USB keyboard path takes RAW HID EVENTS now, not decoded chars
 * (claude/ecstatic-lewin-f617bb - there is no character for Up, so a decoded
 * arrow came back 0 and 0 means "nothing typed"). These harnesses drive the
 * pointer, so no USB keyboard is present. */
int xhci_key_event(void) { return 0; }
int xhci_kbd_mods(void)  { return 0; }

void input_set_accel(int on);

int main(void)
{
    /* THESE TESTS ASSERT THE IDENTITY: that the event carries the position
     * the ISR published. desktop/feel-and-control put an acceleration curve
     * between the two - gain rises to 300%% past a threshold - and this
     * harness moves the fake pointer in single large jumps, which is exactly
     * what the curve is built to amplify. Turning it off restores the
     * identity these assertions were written against; the curve itself is
     * tested in inputtest_feel/wmtest_feel, which drive it deliberately. */
    input_set_accel(0);

    struct { unsigned long a, n; } bufs[] = {
        { BACK_ADDR, BACK_SIZE },
        { BLUR_ADDR, BLUR_SIZE },
    };
    for (unsigned i = 0; i < sizeof bufs / sizeof bufs[0]; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p != (void *)bufs[i].a) { fprintf(stderr, "mmap failed\n"); return 1; }
        memset(p, 0, bufs[i].n);
    }
    void *vram = mmap(NULL, 16UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    fb_setup((unsigned long)vram, W * 4, W, H, 32);

    printf("wmtest - fb.c + input.c + ui.c + wm.c, against fake hardware\n\n");

    ui_theme_init(2);
    wm_init();
    wm_hooks(t_draw, t_event, t_tick, t_desk);
    const struct ui_theme *th = ui_theme();

    /* ---------------------------------------------------------------- table */
    int a = wm_open(1, "one",   100, 100, 400, 300);
    int b = wm_open(2, "two",   300, 200, 400, 300);
    int c = wm_open(3, "three", 500, 300, 400, 300);
    ok("three windows open into three slots", a == 0 && b == 1 && c == 2);
    ok("the last one opened has focus", wm_focused() == c);
    ok("z-order is open order, back to front",
       wm_zorder_at(0) == a && wm_zorder_at(1) == b && wm_zorder_at(2) == c);

    /* WM_MAX is a hard ceiling and a refusal, never a silent drop */
    int opened = 3;
    while (wm_open(9, "filler", 0, 0, 50, 50) >= 0) opened++;
    ok("WM_MAX is a ceiling, and full returns -1", opened == WM_MAX);
    for (int i = 3; i < WM_MAX; i++) wm_close(i);

    /* ------------------------------------------------------------- repaint */
    frame();
    ok("every open window was drawn once", draw_calls[1] && draw_calls[2] && draw_calls[3]);

    /* PAINT ORDER IS Z-ORDER: where two windows overlap, the FRONTMOST is the
     * one on screen. Getting this backwards is the classic compositor bug and
     * it looks like "the windows are see-through".
     *
     * The sample points are inside both CLIENT areas, not merely inside both
     * frames - a point in a title bar is drawn by chrome, not by the app, so
     * it says nothing about paint order. Client areas here:
     *   a  102..498 x 156..398      b  302..698 x 256..498
     *   c  502..898 x 356..598
     * so a and b share (400, 320), and b and c share (600, 400). All three
     * share nothing: a's client ends at x=498 and c's begins at x=502. */
    ok("back-to-front: b is painted over a", fb_get_px(400, 320) == APP_COLOUR(2));
    ok("back-to-front: c is painted over b", fb_get_px(600, 400) == APP_COLOUR(3));

    /* --------------------------------------------------------- the scissor */
    ok("an app cannot draw outside its client area",
       !has_magenta(0, 0, W, H));

    int cx, cy, cw, ch;
    wm_client(c, &cx, &cy, &cw, &ch);
    ok("...and its client area starts below the title bar",
       cy >= 300 + th->title_h - 1);
    ok("...so the title bar survived the app drawing over it",
       fb_get_px(500 + 200, 300 + th->title_h / 2) != APP_COLOUR(3));

    /* ---------------------------------------------------------- move/damage
     * The old position must be COMPLETELY gone - shadow included. Repainting
     * only the frame is what leaves a smear trail behind a dragged window,
     * and the shadow reaches further than the frame does. */
    wm_move(a, 100, 100);                       /* no-op: same place */
    wm_close(b);
    wm_close(c);
    frame();
    ok("one window left, on the wallpaper", wm_count() == 1);

    wm_move(a, 700, 400);
    frame();
    ok("after a move, the OLD position is pure wallpaper again",
       all_wallpaper(100 - 40, 100 - 40, 100 + 400 + 40, 100 + 300 + 40));
    ok("...and the window is at the new one", fb_get_px(750, 500) == APP_COLOUR(1));

    /* ------------------------------------------------------------- hit test */
    int d = wm_open(4, "four", 700, 400, 400, 300);   /* exactly over `a` */
    frame();
    ok("wm_at walks the z-order BACKWARDS - topmost wins", wm_at(750, 500) == d);
    ok("a point outside every window is -1", wm_at(5, 5) == -1);

    /* --------------------------------------------------------------- focus */
    wm_focus(a);
    ok("focus does NOT imply raise", wm_focused() == a && wm_zorder_at(1) == d);
    wm_raise(a);
    ok("raise moves to the END of the z-order", wm_zorder_at(1) == a);

    /* ------------------------------------------------------------- routing */
    /* press in the title bar and drag. The pointer deliberately leaves the
     * window entirely mid-drag: without a pointer grab the drag would stop
     * dead the moment it did, which is the whole reason grab exists. */
    int wx, wy, ww, wh;
    wm_geometry(a, &wx, &wy, &ww, &wh);
    pointer(wx + 50, wy + th->title_h / 2, 0);
    pointer(wx + 50, wy + th->title_h / 2, 1);        /* press */
    pointer(wx + 50 - 300, wy + th->title_h / 2 - 200, 1);   /* far outside */
    int nx, ny;
    wm_geometry(a, &nx, &ny, &ww, &wh);
    ok("a drag keeps tracking after the pointer leaves the window",
       nx == wx - 300 && ny == wy - 200);
    pointer(nx + 50 - 100, ny + th->title_h / 2, 0);  /* release */
    int rx, ry;
    wm_geometry(a, &rx, &ry, &ww, &wh);
    pointer(rx + 900, ry + 900, 0);                   /* wander off */
    int fx, fy;
    wm_geometry(a, &fx, &fy, &ww, &wh);
    ok("...and STOPS tracking after button-up", fx == rx && fy == ry);

    /* a press in the client area goes to the app, with coordinates */
    wm_geometry(a, &wx, &wy, &ww, &wh);
    last_event_app = -1;
    pointer(wx + 100, wy + th->title_h + 40, 1);
    ok("a click in the client area reaches the app", last_event_app == 1);
    ok("...carrying the pointer position", last_event_x == wx + 100);
    pointer(wx + 100, wy + th->title_h + 40, 0);

    /* the close box closes it, and focus lands on the new top */
    wm_geometry(a, &wx, &wy, &ww, &wh);
    int cs = UI_S3(th);
    pointer(wx + ww - cs - UI_S2(th) + cs / 2, wy + th->title_h / 2, 1);
    ok("the close box closes the window", !wm_is_open(a));
    ok("...and focus falls to the new top, not to nothing", wm_focused() == d);
    pointer(0, 0, 0);

    /* ---------------------------------------------------------------- modal
     * This branch of route_mouse had NO WAY TO BE REACHED until wm_set_modal
     * existed - nothing set WF_MODAL, so it was code that could never run.
     * That is this project's own named hazard: "the code exists" is not "the
     * code works", check for an actual caller. */
    int m = wm_open(5, "menu", 200, 200, 200, 300);
    wm_set_modal(m, 1);
    wm_raise(m);
    frame();
    ok("the menu opened", wm_is_open(m));

    /* a click INSIDE the modal reaches it and does not dismiss it */
    last_event_app = -1;
    pointer(250, 200 + th->title_h + 20, 1);
    ok("a modal takes a click inside itself", wm_is_open(m) && last_event_app == 5);
    pointer(250, 200 + th->title_h + 20, 0);

    /* a click OUTSIDE it dismisses it, and does NOT reach the window under it */
    last_event_app = -1;
    pointer(900, 700, 1);
    ok("a click outside a modal dismisses it", !wm_is_open(m));
    ok("...and does not fall through to the window underneath",
       last_event_app == -1);
    pointer(900, 700, 0);

    /* with the modal gone, a click in the same place reaches normally again */
    int under = wm_open(6, "under", 850, 650, 300, 200);
    frame();
    last_event_app = -1;
    pointer(900, 700 + th->title_h, 1);
    ok("with the modal gone, clicks route normally again", last_event_app == 6);
    pointer(900, 700 + th->title_h, 0);
    wm_close(under);

    /* ----------------------------------------------------------- app_tick
     * Settle FIRST. Everything above left damage pending - closing `under`
     * damages a region window `d` overlaps - and this asserts that nothing
     * repaints, so it has to start from a clean frame or it measures the
     * previous test instead of this one. */
    frame();
    draw_calls[4] = 0;
    tick_returns = 0;
    frame(); frame();
    ok("app_tick returning 0 does NOT repaint", draw_calls[4] == 0);
    tick_returns = 1;
    frame();
    ok("app_tick returning 1 DOES repaint", draw_calls[4] > 0);
    tick_returns = 0;

    /* ---------------------------------------------------------------- ui.c
     * The hit-test pass must fire the same widget the draw pass would, while
     * drawing nothing. If those two disagree, clicks land on the wrong
     * control - the single most likely thing to be got wrong here. */
    unsigned before = fb_get_px(20, 20);
    ui_begin(0, 0, 300, 400, UI_HITTEST, 0, 0, 0);
    ui_label("hello");
    int clicked_nothing = ui_button("OK");
    ok("a hit-test pass draws nothing at all", fb_get_px(20, 20) == before);
    ok("...and fires nothing without a click", !clicked_nothing);

    /* find where the button IS, by hit-testing with a click at a known point */
    int found = -1;
    for (int py = 0; py < 400 && found < 0; py += 4) {
        ui_begin(0, 0, 300, 400, UI_HITTEST, 40, py, 1);
        ui_label("hello");
        if (ui_button("OK")) found = py;
    }
    ok("hit-test finds the button by re-running the same layout", found > 0);

    ui_begin(0, 0, 300, 400, UI_HITTEST, 40, found, 1);
    ui_label("hello");
    ui_button("OK");
    ok("...and reports WHICH widget fired", ui_fired() == 0);

    int v = 50, on = 0;
    ui_begin(0, 0, 300, 400, UI_HITTEST, 150, found, 1);
    ui_label("hello");
    ui_button("OK");
    ui_toggle("wrap", &on);
    ui_slider(&v, 0, 100);
    ok("a toggle flips only when it is the thing hit", on == 0 || on == 1);

    /* ------------------------------------------------------------ animation
     * Four frames of growth, then settled. The properties that matter are not
     * "it moves" but: it ENDS, it ends at exactly the requested geometry, and
     * HIT TESTING NEVER SEES THE INTERMEDIATE SIZE - a click that misses
     * because the target was still growing is worse than no animation. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    int an = wm_open(7, "anim", 300, 300, 400, 300);
    int gx2, gy2, gw2, gh2;
    wm_geometry(an, &gx2, &gy2, &gw2, &gh2);
    ok("geometry is the SETTLED size from frame zero",
       gw2 == 400 && gh2 == 300);
    ok("...and so is hit testing, mid-animation", wm_at(310, 310) == an);

    /* the drawn window must actually be smaller on the first frame */
    frame();
    int grew = 0;
    for (int x = 300; x < 320; x++)
        if (fb_get_px(x, 450) == WALL) grew = 1;   /* left edge not yet reached */
    ok("the first frame is drawn SMALLER than the settled rect", grew);

    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    int settled = 1;
    for (int x = 302; x < 316; x++)
        if (fb_get_px(x, 450) == WALL) settled = 0;
    ok("...and after four frames it has settled at full size", settled);

    int before_calls = draw_calls[7];
    frame(); frame();
    ok("the animation ENDS - no repaint once settled",
       draw_calls[7] == before_calls);

    /* ------------------------------------------------- ui_list_row / ui_scroll
     * Both were held back until clipping was settled, and for a real reason:
     * a scrolled list draws rows PARTLY outside their viewport. */
    int off = 0, picked = -1;
    /* 200 rows into a 100px viewport, hit-testing a point inside it */
    ui_begin(0, 0, 400, 300, UI_HITTEST, 40, 60, 1);
    ui_scroll_begin(100, &off);
    for (int i = 0; i < 200; i++) if (ui_list_row("row", 0)) picked = i;
    ui_scroll_end(&off);
    ok("a 200-row list hit-tests to exactly one row", picked >= 0);
    ok("...and measures its full content height", ui_scroll_content() > 100);

    /* scrolled past the end, ui_scroll_end must clamp the app's variable back
     * - the app cannot know the content height until after the loop it just
     * ran, so this is the only place that information exists */
    off = 99999;
    ui_begin(0, 0, 400, 300, UI_HITTEST, 40, 60, 0);
    ui_scroll_begin(100, &off);
    for (int i = 0; i < 200; i++) ui_list_row("row", 0);
    ui_scroll_end(&off);
    ok("scrolling past the end is clamped, not left dangling",
       off == ui_scroll_content() - 100);

    off = -500;
    ui_begin(0, 0, 400, 300, UI_HITTEST, 40, 60, 0);
    ui_scroll_begin(100, &off);
    for (int i = 0; i < 200; i++) ui_list_row("row", 0);
    ui_scroll_end(&off);
    ok("...and so is scrolling above the start", off == 0);

    /* THE PROPERTY THAT MATTERS: rows outside the viewport are REJECTED, not
     * drawn and clipped. The scissor is a correctness guarantee, not a
     * substitute for not drawing - otherwise a 200-row list costs 200 rows of
     * drawing to show 4. Draw into a known-clear area and count the damage. */
    fb_clip_none();
    fb_fill_px(0, 0, 400, 400, WALL);
    off = 0;
    ui_begin(0, 0, 400, 300, UI_DRAW, -1, -1, 0);
    ui_scroll_begin(100, &off);
    for (int i = 0; i < 200; i++) ui_list_row("row", 0);
    ui_scroll_end(&off);
    int painted_below = 1;
    for (int y = 200; y < 400; y++)
        for (int x = 0; x < 380; x++)
            if (fb_get_px(x, y) != WALL) painted_below = 0;
    ok("rows past the viewport never reach the framebuffer", painted_below);

    /* ------------------------------------------------------------- the glue
     * wmglue.c's references to kernel.zl's app_* functions are WEAK, so this
     * binary has them as NULL. The property under test is that it says so
     * rather than calling through a null pointer - which is the entire reason
     * the whole thing can ship before kernel.zl grows those functions. */
    ok("wm_bind_zl declines cleanly when zl has no app_draw", wm_bind_zl() == 0);
    ok("wm_available is false without apps, framebuffer or not", !wm_available());

    /* ---------------------------------------------------------------- tabs
     * Several apps in one frame. The properties that matter: only the ACTIVE
     * tab draws, clicking a tab switches without moving the window, and the
     * tab strip lives inside the title bar - so if the drag were checked
     * first, tabs would be unclickable and the window would move instead. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    int tw = wm_open(1, "Editor", 200, 200, 600, 400);
    ok("a plain window has exactly one tab", wm_ntabs(tw) == 1);
    ok("adding a tab returns its index", wm_add_tab(tw, 2, "Build") == 1);
    wm_add_tab(tw, 3, "Docs");
    ok("...and they accumulate", wm_ntabs(tw) == 3 && wm_tab(tw) == 0);

    draw_calls[1] = draw_calls[2] = draw_calls[3] = 0;
    frame();
    ok("ONLY the active tab's app draws",
       draw_calls[1] > 0 && draw_calls[2] == 0 && draw_calls[3] == 0);

    /* click the second tab. Its rectangle is inside the title bar, and the
     * window must NOT move as a result. */
    int bx, by, bw, bh;
    wm_geometry(tw, &bx, &by, &bw, &bh);
    int strip_y = by + th->title_h / 2;
    int avail = bw - 2 * UI_S3(th) - UI_S6(th);
    int step = avail / 3;
    if (step > UI_S6(th) * 5) step = UI_S6(th) * 5;
    pointer(bx + UI_S2(th) + step + step / 4, strip_y, 1);
    ok("clicking a tab selects it", wm_tab(tw) == 1);
    int ax2, ay2;
    wm_geometry(tw, &ax2, &ay2, &bw, &bh);
    ok("...and does NOT drag the window", ax2 == bx && ay2 == by);
    pointer(bx + UI_S2(th) + step + step / 4, strip_y + 200, 0);

    draw_calls[1] = draw_calls[2] = draw_calls[3] = 0;
    frame();
    ok("...and now the SECOND tab's app is the one that draws",
       draw_calls[2] > 0 && draw_calls[1] == 0);

    /* keys go to the active tab, not to tab 0 */
    last_event_app = -1;
    pointer(bx + 100, by + th->title_h + 100, 1);
    ok("clicks in the client reach the ACTIVE tab's app", last_event_app == 2);
    pointer(bx + 100, by + th->title_h + 100, 0);

    ok("WM_TABS is a ceiling, and full refuses",
       wm_add_tab(tw, 4, "four") == 3 && wm_add_tab(tw, 5, "five") == -1);

    /* --------------------------------------------- elevation vs damage
     * A modal's shadow is 1.5x the size of an ordinary one, so closing one
     * must damage the LARGER rectangle. A fixed reach under-damages by 14 px
     * at scale 2 and leaves a ring of shadow on the wallpaper that nothing
     * ever cleans up - the same failure as the drag smear, but only for
     * modals, and only visible against some backgrounds. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    int md = wm_open(1, "modal", 400, 400, 400, 300);
    wm_set_modal(md, 1);
    frame();
    wm_close(md);
    frame();
    ok("closing a MODAL leaves no shadow ring behind",
       all_wallpaper(400 - 60, 400 - 60, 400 + 400 + 60, 400 + 300 + 60));

    /* and the same for losing focus, which SHRINKS the shadow: wm_focus
     * updates focus_win before damaging, so a reach derived from the new
     * state would miss the old, larger shadow.
     *
     * Park the pointer somewhere else first. wm_frame draws the cursor sprite
     * at the end of every frame, and it is not wallpaper - an earlier test
     * left the pointer inside this region and the check failed on the cursor
     * rather than on any shadow. Which is a fair thing for the assertion to
     * notice; it just is not what this one is asking about. */
    pointer(1500, 1000, 0);
    int f1 = wm_open(1, "one", 200, 200, 300, 200);
    int f2 = wm_open(2, "two", 900, 600, 300, 200);
    frame();
    wm_focus(f1);
    frame();
    wm_close(f1);
    wm_close(f2);
    frame();
    ok("a focus change leaves no shadow edge behind",
       all_wallpaper(200 - 60, 200 - 60, 200 + 300 + 60, 200 + 200 + 60));

    /* --------------------------------------------- the animation timeline
     * Three things have to be true and each has been got wrong by somebody:
     * it STARTS, it ENDS, and it does not move the target. The third is the
     * one that matters most - an animated window whose hit test follows the
     * animation is a UI where clicks land where the control WAS, and it is
     * invisible in any screenshot taken after the animation settles. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    pointer(1500, 1000, 0);
    int aw = wm_open(1, "anim", 300, 300, 400, 300);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();

    ok("a fresh window is not animating", wm_anim_running(aw) == 0);
    ok("...and is fully opaque", wm_anim_alpha(aw) == 255);

    ok("an animation starts", wm_anim(aw, ANIM_PULSE) == 1
                              && wm_anim_running(aw) == ANIM_PULSE);

    /* HIT TESTING IS UNAFFECTED, checked at the animation's most distorted
     * frame rather than at the end - checking after it settles proves nothing
     * at all, which is exactly how this class of bug survives. */
    frame(); frame();
    ok("...and hit testing still finds it mid-animation",
       wm_at(300 + 200, 300 + 150) == aw);
    ok("...and the alpha really is partial", wm_anim_alpha(aw) < 255
                                             && wm_anim_alpha(aw) > 0);

    /* IT ENDS. A timeline entry that never frees its slot exhausts ANIM_MAX
     * and every later animation is refused - which shows up as "the UI stopped
     * animating after a while", the worst kind of bug to chase. */
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    ok("an animation ends and frees its slot", wm_anim_running(aw) == 0);
    ok("...and leaves the window settled", wm_anim_alpha(aw) == 255);

    /* A SCALE KIND DOES NOT MOVE THE TARGET EITHER. ANIM_PRESS shrinks the
     * window to 96%, so a point 2% in from the edge is outside what is DRAWN
     * and must still hit. */
    ok("a scale animation starts", wm_anim(aw, ANIM_PRESS) == 1);
    frame();
    ok("...and an edge point still hits the settled rect",
       wm_at(300 + 4, 300 + 4) == aw);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    ok("...and it ends too", wm_anim_running(aw) == 0);

    /* ANIM_MAX IS A REFUSAL, not a silent drop - the same discipline as
     * wm_open's WM_MAX.
     *
     * The windows are opened and then LET SETTLE first, deliberately: wm_open
     * starts an ANIM_OPEN of its own now, so opening and animating in the same
     * loop measures "how many slots are left after the opens" rather than the
     * ceiling. That is exactly the confusion this assertion exists to avoid,
     * and it caught the change that introduced it. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    int wins2[WM_MAX], nw2 = 0;
    for (int i = 0; i < 9 && nw2 < WM_MAX; i++) {
        int w2 = wm_open(1, "x", 10 + i * 30, 10, 60, 40);
        if (w2 >= 0) wins2[nw2++] = w2;
    }
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    int still = 0;
    for (int i = 0; i < nw2; i++) if (wm_anim_running(wins2[i])) still++;
    ok("every open animation has finished before this", still == 0);

    int started = 0, refused = 0;
    for (int i = 0; i < nw2; i++) {
        if (wm_anim(wins2[i], ANIM_FADE)) started++; else refused++;
    }
    ok("the animation array is a ceiling, and full refuses",
       started > 0 && refused > 0 && started + refused == nw2);

    /* --------------------------------------------- the fade, COMPOSITED
     * wm_anim_alpha() has reported a fade since the timeline was written and
     * nothing drew it - the alpha was asserted and the pixels were not, which
     * is precisely how an effect ends up "done" and invisible.
     *
     * A fade is only real if the result sits BETWEEN the window and what is
     * behind it. Two windows, the top one fading in over the bottom one: at a
     * partial alpha the pixel must equal neither. Checking it is not the
     * window's colour would pass for a fade that drew nothing at all. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    pointer(1600, 1000, 0);
    int fdlo = wm_open(1, "under", 200, 200, 600, 400);
    for (int i = 0; i < 8; i++) frame();
    frame();
    unsigned int px_under = fb_get_px(400, 400);

    int fdhi = wm_open(2, "over", 200, 200, 600, 400);
    for (int i = 0; i < 8; i++) frame();
    frame();
    unsigned int px_over = fb_get_px(400, 400);
    ok("two stacked windows differ where they overlap", px_under != px_over);

    (void)fdlo;
    wm_anim(fdhi, ANIM_FADE);
    frame(); frame();                     /* a middle frame of the ramp */
    unsigned int px_mid = fb_get_px(400, 400);
    ok("a fading window is not fully drawn", px_mid != px_over);
    ok("...and not fully absent either",    px_mid != px_under);
    ok("...and the alpha is genuinely partial",
       wm_anim_alpha(fdhi) > 0 && wm_anim_alpha(fdhi) < 255);

    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    frame();
    ok("when it settles it is the window again", fb_get_px(400, 400) == px_over);

    /* --------------------------------------------- the resize grip
     * wm_resize() existed from the day wm.c was written and had NO CALLER -
     * the same shape as WF_MODAL before the start menu. These assert the three
     * things a grip has to get right, and the third is the one that is
     * invisible in a screenshot. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    pointer(1500, 1000, 0);
    int rw = wm_open(1, "resize", 300, 300, 400, 300);
    for (int i = 0; i < 8; i++) frame();
    int grx, gry, rww, rhh;

    /* 1. a press in the corner grabs the SIZE, not the app and not the move */
    pointer(300 + 400 - 3, 300 + 300 - 3, 1);
    pointer(300 + 500, 300 + 400, 1);            /* drag out */
    wm_geometry(rw, &grx, &gry, &rww, &rhh);
    ok("dragging the grip resizes the window", rww > 400 && rhh > 300);
    ok("...and does NOT move it", grx == 300 && gry == 300);
    pointer(300 + 500, 300 + 400, 0);

    /* 2. the minimum is a floor, not a suggestion. A window dragged to zero is
     *    a window that can never be grabbed again. */
    wm_geometry(rw, &grx, &gry, &rww, &rhh);
    pointer(grx + rww - 3, gry + rhh - 3, 1);
    pointer(grx + 2, gry + 2, 1);                /* drag right past the origin */
    pointer(grx + 2, gry + 2, 0);
    wm_geometry(rw, &grx, &gry, &rww, &rhh);
    ok("a window cannot be resized to nothing", rww >= 8 && rhh >= 8);

    /* 3. THE GRIP MUST NOT STEAL THE TITLE BAR. They are both chrome and the
     *    grip is checked second, but a window short enough that its title bar
     *    reaches the bottom edge would hand every title-bar press to the
     *    resize - i.e. the window could never be moved again. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    frame();
    int sw2 = wm_open(1, "short", 500, 500, 300, th->title_h + 4);
    for (int i = 0; i < 8; i++) frame();
    pointer(560, 500 + 3, 1);
    pointer(660, 600, 1);
    wm_geometry(sw2, &grx, &gry, &rww, &rhh);
    ok("the title bar still MOVES a very short window", grx != 500 || gry != 500);
    pointer(660, 600, 0);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
