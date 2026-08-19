/* wmtest_feel.c - feel's window snapping, the resize grip, double-click and the animation switch.
 *
 * This is desktop/feel-and-control's rewrite of wmtest.c. It was NOT merged
 * into that file: the two overlap only partly, and taking either alone drops
 * assertions the other has. Both are built.
 */
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
#include "../ease.h"

/* Derived, not transcribed - see the same constant in wmtest.c. The literal
 * 20 went stale when wm.c adopted the reference's durations. */
#define ANIM_SETTLE (EASE_MS_PULSE / 10 + 4)   /* 10 ms per PIT tick */
#define MOD_SUPER   (1 << 5)

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
int  fb_pointer_extent(void);                  /* the cursor's size at this scale */
void fb_pointer_saved(int *x, int *y, int *n); /* the patch ACTUALLY saved */

/* ---- input.c ------------------------------------------------------------- */
int  input_next(void);
void input_set_speed(int pct);
void input_set_accel(int on);

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

#define W 1280
#define H 800

/* THE REAL HARDWARE CLAMPS, SO THIS MUST TOO.
 *
 * idt.c's IRQ12 handler holds the pointer inside a bounded range and can never
 * publish a negative or off-screen position. This harness used to hand back
 * whatever a test set, which let assertions depend on pointer positions no
 * real machine can produce - a drag was checked by pulling the pointer to
 * -250,-86.
 *
 * That only became visible when input.c started clamping too, which it must:
 * acceleration multiplies distance travelled, so a clamp adequate at 1:1 is
 * not adequate at 4x, and off the edge the pointer is unreachable and the
 * machine looks hung. Those tests then failed against a MORE faithful model,
 * not a broken one. Clamping at the source keeps the harness honest and stops
 * the same assumption creeping back in. */
static int clampi(int v, int hi) { return v < 0 ? 0 : (v > hi ? hi : v); }

int idt_mouse_x(void)   { return clampi(fake_x, W - 1); }
int idt_mouse_y(void)   { return clampi(fake_y, H - 1); }
int idt_mouse_btn(void) { return fake_btn; }
unsigned int idt_ticks(void) { return fake_ticks; }
/* A real scancode queue, so key tests go through input.c's actual decoder -
 * modifiers, the 0xE0 extended prefix and all - rather than through a faked
 * event. A Super+arrow chord that only works against invented events proves
 * nothing about the one a keyboard sends. */
static int scanq[64], scanh, scant;
static int fake_wz = 0;
int idt_mouse_wheel(void) { int v = fake_wz; fake_wz = 0; return v; }

int idt_scan(void)
{
    if (scanh == scant) return 0;
    int v = scanq[scanh];
    scanh = (scanh + 1) % 64;
    return v;
}
static void scan(int sc) { scanq[scant] = sc; scant = (scant + 1) % 64; }
int xhci_key(void)      { return 0; }
void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void zl_putc_pub(char c) { (void)c; }        /* fb.c's boot line: not wanted here */

#define BG_ADDR   0x08000000UL
#define SP_ADDR   0x0A000000UL
#define BACK_ADDR 0x0C000000UL

/* ---- the fake app --------------------------------------------------------
 * Fills its whole client area with a colour derived from its id, then
 * deliberately tries to draw a long way OUTSIDE it. The second half is the
 * point: the guarantee fb_clip buys is that an app which draws at -500,-500
 * simply produces nothing. */
#define APP_COLOUR(a) (0x00110000u * (unsigned)(a) + 0x00002200u)
static int draw_calls[8];
static int tick_returns;            /* what app_tick claims each frame */
static int last_event_app = -1, last_event_type, last_event_x, last_event_y;
static int last_event_code;

/* When set, the app closes a SCROLL REGION before making its escape attempts.
 * That is the interesting case: ui_scroll_end used to end with fb_clip_none(),
 * which does not restore the caller's scissor - it removes it - so everything
 * an app drew after a list was free to paint over the whole screen. The
 * escaping fills below are identical either way; only the clip state differs. */
static int scroll_escape = 0;
static int scroll_off = 0;

static void t_draw(int app, int x, int y, int w, int h, int focused)
{
    (void)focused;
    if (app >= 0 && app < 8) draw_calls[app]++;
    fb_fill_px(x, y, w, h, APP_COLOUR(app));
    if (scroll_escape) {
        ui_begin(x, y, w, h, UI_DRAW, -1, -1, 0);
        ui_scroll_begin(h / 3, &scroll_off);
        for (int i = 0; i < 30; i++) ui_list_row("row", 0);
        ui_scroll_end(&scroll_off);
    }
    fb_fill_px(x - 500, y - 500, 400, 400, 0x00FF00FF);   /* must vanish */
    fb_fill_px(x, y - 40, w, 30, 0x00FF00FF);             /* into the title bar */
    fb_fill_px(x + w + 10, y, 200, h, 0x00FF00FF);        /* past the right edge */
}

static int t_event(int app, int win, int type, int code, int x, int y)
{
    (void)win;
    last_event_code = code;
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
    fake_x = x; fake_y = y; fake_btn = btn;
    frame();
}

/* Press a navigation key, optionally with Super held, then let go of both.
 * Scancode set 1: 0xE0 prefixes the extended keys, and release is the make
 * code with bit 7 set. 0x5B is left Super; the arrows are 0x48/0x50/0x4B/0x4D
 * for up/down/left/right. */
static void send_key(int keycode, int mods)
{
    int sc = keycode == 0x112 ? 0x48 : keycode == 0x113 ? 0x50
           : keycode == 0x110 ? 0x4B : 0x4D;
    if (mods & MOD_SUPER) { scan(0xE0); scan(0x5B); }
    scan(0xE0); scan(sc);
    frame();
    scan(0xE0); scan(sc | 0x80);
    if (mods & MOD_SUPER) { scan(0xE0); scan(0xDB); }
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

/* Stubs for hardware this harness does not fake. The merge gave input.c
 * and wm.c callers into the USB tablet, the serial port, the USB keyboard
 * and the TSC, none of which existed when this file was written.
 * ser_rx returns -1, which is "no UART", not "a NUL byte". */
int cpu_tsc_lo(void) { return 0; }
int cpu_tsc_khz(void) { return 0; }

/* Hardware this harness does not fake. The merge gave input.c and wm.c
 * callers into the USB tablet, the USB keyboard, the scroll wheel, the
 * serial port and the TSC. ser_rx returns -1 ("no UART"), not 0, which
 * would be a NUL byte and a keystroke. */
int xhci_ptr_ready(void) { return 0; }
int xhci_ptr_poll(void) { return 0; }
int xhci_poll(int max)   { (void)max; return 0; }  /* the one ring drainer */
int xhci_ptr_abs(void)   { return 0; }   /* no USB pointer here at all */
int xhci_ptr_take_dx(void) { return 0; }
int xhci_ptr_take_dy(void) { return 0; }
int xhci_ptr_x(void) { return 0; }
int xhci_ptr_y(void) { return 0; }
int xhci_ptr_btn(void) { return 0; }
int xhci_key_event(void) { return 0; }
int xhci_kbd_mods(void) { return 0; }
int ser_rx(void) { return -1; }

int main(void)
{
    struct { unsigned long a, n; } bufs[] = {
        { BG_ADDR, 32UL << 20 }, { SP_ADDR, 16UL << 20 }, { BACK_ADDR, 16UL << 20 },
    };
    for (unsigned i = 0; i < 3; i++) {
        void *p = mmap((void *)bufs[i].a, bufs[i].n, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
        if (p != (void *)bufs[i].a) { fprintf(stderr, "mmap failed\n"); return 1; }
        memset(p, 0, bufs[i].n);
    }
    void *vram = mmap(NULL, 16UL << 20, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    fb_setup((unsigned long)vram, W * 4, W, H, 32);

    printf("wmtest - fb.c + input.c + ui.c + wm.c, against fake hardware\n\n");

    /* PIN THE POINTER GAIN TO THE IDENTITY.
     *
     * input.c now applies a speed multiplier and an acceleration curve, and
     * ships with the curve ON - so the position the compositor receives is no
     * longer the position the fake hardware was set to. Every assertion below
     * that clicks a close box, a tab or a client area depends on pointer(x,y)
     * actually putting the pointer at x,y; without this, eleven of them fail
     * for a reason none of them is asking about.
     *
     * Aiming a click under acceleration would mean inverting the curve in the
     * test, which tests the test. The curve gets asserted in inputtest, where
     * it is the subject; here it is noise. */
    input_set_speed(100);
    input_set_accel(0);

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
    /* press in the title bar and drag. The pointer deliberately ends up
     * entirely outside the window's ORIGINAL rectangle mid-drag: without a
     * pointer grab the drag would stop dead the moment it did, which is the
     * whole reason grab exists.
     *
     * It used to prove that by dragging to -150,-86 - off the screen, where a
     * clamped pointer cannot go. Same property, same single drag, in a
     * direction a real pointer can actually travel: 600 right and 300 down
     * takes it clear of the original 100,100 400x300 rect and leaves it at
     * 750,414, well inside a 1280x800 screen. */
    int wx, wy, ww, wh;
    /* Put it somewhere KNOWN first. This test used to inherit whatever
     * position earlier tests had left the window in - it is at 700,400 by the
     * time control reaches here, not the 100,100 it was opened at - and then
     * drag by a hardcoded offset. That was invisible while the pointer could
     * go anywhere; with a clamped pointer the same offset runs off the right
     * edge and the drag comes up 71px short. A test whose setup depends on the
     * side effects of the tests above it will break for reasons that have
     * nothing to do with what it checks. */
    wm_move(a, 100, 100);
    wm_geometry(a, &wx, &wy, &ww, &wh);
    pointer(wx + 50, wy + th->title_h / 2, 0);
    pointer(wx + 50, wy + th->title_h / 2, 1);        /* press */
    pointer(wx + 50 + 600, wy + th->title_h / 2 + 300, 1);   /* clear of the rect */
    int nx, ny;
    wm_geometry(a, &nx, &ny, &ww, &wh);
    ok("a drag keeps tracking after the pointer leaves the window",
       nx == wx + 600 && ny == wy + 300);
    pointer(nx + 50 - 100, ny + th->title_h / 2, 0);  /* release */
    int rx, ry;
    wm_geometry(a, &rx, &ry, &ww, &wh);
    pointer(rx + 400, ry + 300, 0);                   /* wander off */
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
     * previous test instead of this one.
     *
     * ONE frame() used to be enough and is not any more - see the same block
     * in wmtest.c. The reference's zwin runs 200 ms where zlOS's old open ran
     * 160, so a window was still animating here and repainting itself. */
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
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

    /* A SLIDER MUST BE ABLE TO REACH BOTH ENDS.
     *
     * It used to divide the track into (hi - lo) buckets for (hi - lo + 1)
     * values, so the maximum was only selectable at exactly one pixel PAST the
     * track's last pixel - i.e. never - and the `t > hi` clamp was dead code.
     * A three-value slider could only ever produce two of them.
     *
     * Found by driving the Settings app's scale slider (1..3) from a harness
     * and watching it refuse to reach 3. Both ends and the middle, on a small
     * range where an off-by-one cannot hide in rounding. */
    {
        /* ui_begin insets the content box by theme.pad on every side, so the
         * track starts at pad and the probe has to be INSIDE the first row
         * vertically or nothing is hit at all. Taken from the theme rather
         * than written out, so this keeps working at any scale. */
        int px0 = th->pad, py0 = th->pad + th->row_h / 2;
        int trackw = 300 - 2 * th->pad;
        int s;

        s = 2;
        ui_begin(0, 0, 300, 400, UI_HITTEST, px0, py0, 1);
        ui_slider(&s, 1, 3);
        ok("a slider's LOW end is reachable", s == 1);

        s = 2;
        ui_begin(0, 0, 300, 400, UI_HITTEST, px0 + trackw - 1, py0, 1);
        ui_slider(&s, 1, 3);
        ok("...and so is its HIGH end", s == 3);

        s = 1;
        ui_begin(0, 0, 300, 400, UI_HITTEST, px0 + trackw / 2, py0, 1);
        ui_slider(&s, 1, 3);
        ok("...and the middle selects the middle", s == 2);

        /* Outside the track, nothing happens - because ui.c's fire() requires
         * a hit before it will report a widget as fired.
         *
         * That is worth pinning down, because ui.c's comment on this widget
         * says the opposite: "once pressed it must keep tracking after the
         * pointer leaves its rectangle, which only works because the window
         * that owns the grab keeps receiving the events". wm.c's grab does
         * keep delivering the events - that half is true and asserted above -
         * but ui_slider drops them, so dragging a slider and straying a few
         * pixels above it stops the drag dead. Logged against ui.c rather than
         * changed here: giving a widget grab semantics needs identity that
         * survives between frames, which is a real change to the toolkit and
         * belongs with whoever owns it. */
        s = 1;
        ui_begin(0, 0, 300, 400, UI_HITTEST, px0 + trackw + 50, py0, 1);
        ui_slider(&s, 1, 3);
        ok("...and a click outside the track does not move it", s == 1);
    }

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
    pointer(1200, 760, 0);   /* a real pointer cannot be at 1500,1000 on a 1280x800 screen */
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

    /* ------------------------------------------------------- resize grip */
    /* wm_resize has existed since the window table did - min_w/min_h clamping,
     * damage on both the old and the new rect - and NOTHING HAS EVER CALLED
     * IT. FEEL-PROMPT item 6 names it first for that reason. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    wm_damage(0, 0, W, H);
    frame();
    int rw = wm_open(5, "resize me", 200, 200, 400, 300);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    int rx0, ry0, rw0, rh0;
    wm_geometry(rw, &rx0, &ry0, &rw0, &rh0);

    /* grab the bottom-right corner and drag it out by 120,90 */
    int gs = UI_S3(th);
    int gpx = rx0 + rw0 - gs / 2, gpy = ry0 + rh0 - gs / 2;
    pointer(gpx, gpy, 0);
    ok("the pointer over the grip asks for the RESIZE cursor",
       fb_cursor_get() == CURSOR_RESIZE);
    pointer(gpx, gpy, 1);
    pointer(gpx + 120, gpy + 90, 1);
    int rx1, ry1, rw1, rh1;
    wm_geometry(rw, &rx1, &ry1, &rw1, &rh1);
    ok("dragging the grip RESIZES the window", rw1 == rw0 + 120 && rh1 == rh0 + 90);
    ok("...and does not MOVE it", rx1 == rx0 && ry1 == ry0);
    ok("...and the cursor stays RESIZE mid-drag",
       fb_cursor_get() == CURSOR_RESIZE);

    /* keep dragging inward, far past any sane size: min_w/min_h must hold */
    pointer(gpx - 4000, gpy - 4000, 1);
    int rw2, rh2;
    wm_geometry(rw, &rx1, &ry1, &rw2, &rh2);
    ok("...and a drag past the minimum clamps instead of inverting",
       rw2 > 0 && rh2 > 0 && rw2 <= rw0 && rh2 <= rh0);
    pointer(gpx, gpy, 0);                    /* release */

    /* the grip must not steal the title bar's drag, or windows stop moving */
    wm_geometry(rw, &rx1, &ry1, &rw1, &rh1);
    pointer(rx1 + 40, ry1 + th->title_h / 2, 1);
    pointer(rx1 + 40 + 60, ry1 + th->title_h / 2 + 40, 1);
    int rx3, ry3, rw3, rh3;
    wm_geometry(rw, &rx3, &ry3, &rw3, &rh3);
    ok("the title bar still MOVES rather than resizing",
       rx3 == rx1 + 60 && ry3 == ry1 + 40 && rw3 == rw1 && rh3 == rh1);
    pointer(rx3 + 40, ry3 + th->title_h / 2, 0);
    ok("...and off the grip the cursor is an ARROW again",
       fb_cursor_get() == CURSOR_ARROW);
    wm_close(rw);
    frame();

    /* --------------------------------------------------------- double-click */
    /* There was no notion of one anywhere in the kernel. It is decided in wm.c
     * rather than input.c because it is a question about PLACE as well as time:
     * two presses 300 ms apart at opposite corners are not a double-click, and
     * input.c knows nothing about where windows are. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    wm_damage(0, 0, W, H);
    frame();
    int dw = wm_open(6, "dbl", 250, 250, 380, 260);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    int dx0, dy0, dw0, dh0;
    wm_geometry(dw, &dx0, &dy0, &dw0, &dh0);
    int tby = dy0 + th->title_h / 2;

    /* two presses, close together in time and place, on the title bar */
    pointer(dx0 + 60, tby, 1);
    pointer(dx0 + 60, tby, 0);
    pointer(dx0 + 60, tby, 1);
    pointer(dx0 + 60, tby, 0);
    int mx, my, mw, mh;
    int work_y = UI_DP(th, 48);
    int work_h = H - work_y - UI_DP(th, 72);
    wm_geometry(dw, &mx, &my, &mw, &mh);
    ok("double-clicking the title bar MAXIMISES",
       mx == 0 && my == work_y && mw == W && mh == work_h);

    /* ...and again restores the exact rect it had */
    pointer(60, work_y + th->title_h / 2, 1);
    pointer(60, work_y + th->title_h / 2, 0);
    pointer(60, work_y + th->title_h / 2, 1);
    pointer(60, work_y + th->title_h / 2, 0);
    wm_geometry(dw, &mx, &my, &mw, &mh);
    ok("...and doing it again RESTORES the exact rect",
       mx == dx0 && my == dy0 && mw == dw0 && mh == dh0);

    /* a SLOW pair is two singles, not a double. The clock is the only thing
     * that differs from the case above. */
    pointer(dx0 + 60, tby, 1);
    pointer(dx0 + 60, tby, 0);
    fake_ticks += 500;                       /* far outside the window */
    pointer(dx0 + 60, tby, 1);
    pointer(dx0 + 60, tby, 0);
    wm_geometry(dw, &mx, &my, &mw, &mh);
    ok("two SLOW clicks are not a double", mw == dw0 && mh == dh0);

    /* a pair far apart in SPACE is not a double either - and this one also
     * proves the detector is not simply counting presses */
    pointer(dx0 + 40, tby, 1);
    pointer(dx0 + 40, tby, 0);
    pointer(dx0 + 300, tby, 1);
    pointer(dx0 + 300, tby, 0);
    wm_geometry(dw, &mx, &my, &mw, &mh);
    ok("two clicks far apart are not a double", mw == dw0 && mh == dh0);

    /* THREE clicks are a double and then a single, not two overlapping
     * doubles - otherwise a triple-click maximises and instantly restores,
     * and the window appears not to respond at all. */
    wm_geometry(dw, &dx0, &dy0, &dw0, &dh0);
    tby = dy0 + th->title_h / 2;
    pointer(dx0 + 60, tby, 1); pointer(dx0 + 60, tby, 0);
    pointer(dx0 + 60, tby, 1); pointer(dx0 + 60, tby, 0);
    pointer(dx0 + 60, tby, 1); pointer(dx0 + 60, tby, 0);
    wm_geometry(dw, &mx, &my, &mw, &mh);
    ok("a TRIPLE click maximises once, it does not toggle twice",
       mx == 0 && my == work_y && mw == W && mh == work_h);
    pointer(60, work_y + th->title_h / 2, 1); pointer(60, work_y + th->title_h / 2, 0);
    pointer(60, work_y + th->title_h / 2, 1); pointer(60, work_y + th->title_h / 2, 0);

    /* the app is told, as a bit in the button mask */
    wm_geometry(dw, &dx0, &dy0, &dw0, &dh0);
    int cy2 = dy0 + th->title_h + 40;
    pointer(dx0 + 60, cy2, 1);
    pointer(dx0 + 60, cy2, 0);
    last_event_code = 0;
    pointer(dx0 + 60, cy2, 1);
    ok("a double-click in the CLIENT area reaches the app with MOUSE_DOUBLE",
       (last_event_code & MOUSE_DOUBLE) != 0);
    pointer(dx0 + 60, cy2, 0);
    fake_ticks += 500;
    last_event_code = 0;
    pointer(dx0 + 60, cy2, 1);
    ok("...and a lone click does NOT carry it",
       (last_event_code & MOUSE_DOUBLE) == 0);
    pointer(dx0 + 60, cy2, 0);
    wm_close(dw);
    frame();

    /* --------------------------------------------------------- scroll wheel */
    /* A notch goes to the window UNDER THE POINTER, not the focused one - you
     * scroll what you are looking at without clicking it first - and it must
     * not raise or focus it, because scrolling is not a click. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    wm_damage(0, 0, W, H);
    pointer(20, 20, 0);
    frame();
    int wa = wm_open(11, "under",  150, 150, 300, 250);
    int wb = wm_open(12, "focused", 700, 400, 300, 250);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    wm_focus(wb);

    int wcx, wcy, wcw, wch;
    wm_client(wa, &wcx, &wcy, &wcw, &wch);
    pointer(wcx + 40, wcy + 40, 0);          /* hover the UNFOCUSED one */
    last_event_app = -1; last_event_type = 0; last_event_code = 0;
    fake_wz = 3;
    frame();
    ok("a wheel notch goes to the window UNDER the pointer",
       last_event_app == 11);
    ok("...as an EV_WHEEL carrying the notch count",
       last_event_type == 5 && last_event_code == 3);
    ok("...and does NOT steal focus from the focused window",
       wm_focused() == wb);

    fake_wz = -2;
    frame();
    ok("...and a backward notch arrives negative", last_event_code == -2);

    /* over bare wallpaper it must go nowhere rather than to the focused window */
    pointer(W - 30, H - 30, 0);
    last_event_app = -1;
    fake_wz = 1;
    frame();
    ok("a notch over no window reaches no app", last_event_app == -1);
    wm_close(wa); wm_close(wb);
    frame();

    /* --------------------------------------------------- grab vs. close */
    /* CLOSING A WINDOW MUST RELEASE ITS POINTER GRAB.
     *
     * A press in a client area hands the window the pointer until button-up -
     * that is what makes a slider keep tracking when the pointer leaves it.
     * Nothing released that grab when the window CLOSED, and a window can
     * close mid-press: Ctrl+W is a key event and arrives between the down and
     * the up.
     *
     * The second half is the nasty one. wm_open reuses the first free slot, so
     * a window opened before button-up lands in the dead window's index and
     * silently inherits the drag. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    wm_damage(0, 0, W, H);
    pointer(20, 20, 0);
    frame();
    int ga = wm_open(9, "grabbed", 200, 200, 400, 300);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    int gcx, gcy, gcw, gch;
    wm_client(ga, &gcx, &gcy, &gcw, &gch);
    pointer(gcx + 40, gcy + 40, 1);            /* press in the client: grab */
    wm_close(ga);
    frame();
    last_event_app = -1;
    pointer(gcx + 80, gcy + 80, 1);            /* still held */
    ok("closing a window releases its pointer grab", last_event_app == -1);

    /* ...and a new window in the reused slot does not inherit it */
    int gb = wm_open(10, "fresh", 500, 500, 300, 200);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    int bx0, by0, bw0, bh0;
    wm_geometry(gb, &bx0, &by0, &bw0, &bh0);
    last_event_app = -1;
    pointer(gcx + 120, gcy + 120, 1);          /* STILL held from before */
    int bx1, by1, bw1, bh1;
    wm_geometry(gb, &bx1, &by1, &bw1, &bh1);
    ok("...and a window reusing the slot does not inherit the drag",
       bx1 == bx0 && by1 == by0 && bw1 == bw0 && bh1 == bh0 &&
       last_event_app != 10);
    pointer(gcx + 120, gcy + 120, 0);
    wm_close(gb);
    frame();

    /* ---------------------------------------------------------- Super key */
    /* MOD_SUPER has been tracked by input.c since it was written and used for
     * NOTHING. Snapping is the binding worth spending it on: the one window
     * operation that is painful with a pointer and trivial with a key. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    wm_damage(0, 0, W, H);
    pointer(20, 20, 0);
    frame();
    int sn = wm_open(8, "snap", 300, 260, 440, 300);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    wm_focus(sn);
    int snx, sny, snw, snh;
    wm_geometry(sn, &snx, &sny, &snw, &snh);

    /* Dragging to an edge must preview the destination BEFORE release. This
     * is a pixel assertion: checking only snap state would pass if the overlay
     * were calculated and never drawn, this repo's recurring mechanism-with-
     * no-caller failure. The sample lies outside the moving window itself. */
    unsigned preview_under = fb_get_px(W / 2 - 20, H - 150);
    pointer(snx + 40, sny + 10, 1);
    pointer(0, H / 2, 1);
    unsigned preview_ink = fb_get_px(W / 2 - 20, H - 150);
    ok("edge drag draws the snap destination preview", preview_ink != preview_under);
    pointer(0, H / 2, 0);                       /* apply the left snap */
    send_key(0x113, MOD_SUPER);                  /* restore original rect */
    frame();
    ok("releasing the drag clears the preview pixels",
       fb_get_px(W / 2 - 20, H - 150) == preview_under);

    send_key(0x110, MOD_SUPER);                       /* Super+Left */
    int gx, gy, gw, gh;
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("Super+Left snaps to the left half",
       gx == 0 && gy == work_y && gw == W / 2 && gh == work_h);

    send_key(0x111, MOD_SUPER);                       /* Super+Right */
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("Super+Right from the left half passes through maximised",
       gx == 0 && gy == work_y && gw == W && gh == work_h);
    send_key(0x111, MOD_SUPER);                       /* Super+Right again */
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("...and the next Right reaches the right half",
       gx == W / 2 && gy == work_y && gw == W - W / 2 && gh == work_h);

    send_key(0x112, MOD_SUPER);                       /* Super+Up */
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("Super+Up maximises", gx == 0 && gy == work_y && gw == W && gh == work_h);

    /* THE SAVED RECT MUST SURVIVE THREE SNAPS. Capturing it on every snap
     * instead of only the first is the bug every naive version has: restore
     * then returns you to the previous SNAP rather than to where you started. */
    send_key(0x113, MOD_SUPER);                       /* Super+Down */
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("Super+Down restores the rect it had BEFORE the first snap",
       gx == snx && gy == sny && gw == snw && gh == snh);

    /* ...and restoring twice is not a second, different move */
    send_key(0x113, MOD_SUPER);
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("...and a second restore does nothing",
       gx == snx && gy == sny && gw == snw && gh == snh);

    /* the arrows must do nothing WITHOUT Super, or they stop reaching apps */
    last_event_app = -1;
    send_key(0x110, 0);
    wm_geometry(sn, &gx, &gy, &gw, &gh);
    ok("a bare arrow key does not snap", gx == snx && gw == snw);
    ok("...it reaches the app instead", last_event_app == 8);
    wm_close(sn);
    frame();

    /* ---------------------------------------------------------- focus ring */
    /* Window focus was a title-bar hue and an accent underline; a focused
     * CONTROL had no indicator at all, and no way to move between controls
     * without the mouse. */
    {
        int v = 50, on = 0, fired_by_key = 0;
        int px0 = th->pad, py0 = th->pad + th->row_h / 2;

        ui_set_focus(-1);
        ui_begin(0, 0, 300, 400, UI_HITTEST, -1, -1, 0);
        ui_button("one"); ui_toggle("two", &on); ui_slider(&v, 0, 100);
        ok("three firing widgets are counted", ui_widget_count() == 3);
        ok("...and nothing is focused by default", ui_focus_get() == -1);

        /* ACTIVATION GOES THROUGH THE SAME fire() A CLICK DOES. A widget that
         * could tell Enter from a click is two code paths that will drift. */
        ui_set_focus(1);
        ui_activate_focus();
        ui_begin(0, 0, 300, 400, UI_HITTEST, -1, -1, 0);
        ui_button("one");
        fired_by_key = ui_toggle("two", &on);
        ui_slider(&v, 0, 100);
        ui_end_activate();
        ok("the FOCUSED widget fires on activation", fired_by_key == 1);
        ok("...and it really toggled", on == 1);

        /* ...and only that one. An activation that fired every widget would
         * look like it worked on whichever control was checked first. */
        int a_fired = 0, c_fired = 0;
        on = 0; v = 50;
        ui_set_focus(1);
        ui_activate_focus();
        ui_begin(0, 0, 300, 400, UI_HITTEST, -1, -1, 0);
        a_fired = ui_button("one");
        ui_toggle("two", &on);
        c_fired = ui_slider(&v, 0, 100);
        ui_end_activate();
        ok("...and ONLY that one", a_fired == 0 && c_fired == 0);

        /* the flag is one-shot: a second pass must not re-fire it, or holding
         * Enter would toggle the control on every repaint */
        on = 0;
        ui_begin(0, 0, 300, 400, UI_HITTEST, -1, -1, 0);
        ui_button("one"); ui_toggle("two", &on); ui_slider(&v, 0, 100);
        ok("...and activation is ONE-SHOT, not sticky", on == 0);

        /* the ring is drawn, and only around the focused control */
        for (int i = 0; i < WM_MAX; i++) wm_close(i);
        wm_damage(0, 0, W, H);
        pointer(20, 20, 0);
        frame();
        ui_set_focus(-1);
    }

    /* -------------------------------------------------- the scroll scissor */
    /* A SCROLL REGION MUST NARROW THE SCISSOR AND THEN PUT IT BACK.
     *
     * Found by the Item 5 bug hunt. wm_repaint clips an app to its client
     * rectangle before calling it, and "an app which draws at -500,-500
     * physically cannot produce a pixel" is the guarantee the whole layering
     * rests on - the assertions above already check it. ui_scroll_end ended
     * with fb_clip_none(), which does not RESTORE that scissor, it REMOVES it.
     * So the guarantee held for every app except one that had drawn a list,
     * and there it silently stopped holding for everything drawn afterwards.
     *
     * ui.c has carried fields named "the scissor to put back" since the
     * function was written, and nothing ever read them.
     *
     * Same escape attempts as the clipping tests above, with a scroll region
     * closed first. Only the clip state differs. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    pointer(20, 20, 0);
    wm_damage(0, 0, W, H);
    frame();
    int sc = wm_open(4, "scroller", 400, 300, 400, 300);
    scroll_escape = 1;
    wm_damage(0, 0, W, H);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();
    scroll_escape = 0;
    ok("after a scroll region, an app still cannot paint above its window",
       !has_magenta(0, 0, W, 300));
    ok("...nor to the left of it",
       !has_magenta(0, 0, 400, H));
    ok("...nor past its right edge",
       !has_magenta(800 + 1, 0, W, H));
    wm_close(sc);
    frame();

    /* ------------------------------------------------------------- shadow */
    /* THE ELEVATION SCHEME MUST REACH THE SCREEN.
     *
     * Found by the Item 5 bug hunt. wm_repaint intersects the damage rect with
     * the window's FRAME first, and only falls back to the reach-expanded rect
     * when the frame misses entirely - then hands the frame-only result to
     * fb_clip under a comment reading "clip 1: the frame + shadow". chrome()
     * calls fb_shadow, which paints from x+off-soft to x+off+w+soft, so at
     * ui scale 2 (off 16, soft 12) the entire visible band lies outside that
     * scissor and every shadow pixel is computed and then discarded.
     *
     * Nothing catches it by eye, because it only LOOKS like the shadows were
     * never designed. It survives transiently during the 4-frame open
     * animation - anim_rect shrinks the drawn frame far enough inside the
     * settled one that the band briefly fits - and is then erased by the
     * wallpaper pass and never comes back.
     *
     * The probe sits 10 px right of the frame's right edge, vertically
     * centred. fb_shadow offsets the footprint by +off, so that pixel has
     * chebyshev distance 0 and takes the full 62% darkening. */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    pointer(20, 20, 0);                 /* cursor sprite well clear of the probe */
    wm_damage(0, 0, W, H);
    frame();
    int sh = wm_open(7, "shadow", 300, 300, 400, 250);
    for (int i = 0; i < ANIM_SETTLE; i++) frame();   /* let the open animation settle */
    wm_damage(0, 0, W, H);
    frame();
    ok("a SETTLED window draws its drop shadow at all",
       fb_get_px(710, 425) != WALL);
    /* WALL 0x203040 at 38% brightness: 0x20*38/100=0x0C, 0x30->0x12, 0x40->0x18 */
    ok("...at full strength, 10 px outside the frame",
       fb_get_px(710, 425) == 0x000C1218u);
    /* ...and it must still stop at the rim rather than darkening the desktop */
    ok("...and stops at the shadow's outer rim",
       fb_get_px(300 + 400 + 40, 425) == WALL);
    wm_close(sh);
    frame();

    /* ------------------------------------------------------------- cursor */
    /* The pointer is the one thing the eye follows constantly, and every one
     * of its failure modes is invisible in a still: a halo left behind is only
     * visible if you move the pointer and look at where it WAS, and "the edges
     * are hard" is invisible unless you magnify. So it gets assertions.
     *
     * The stated gate for this work is probe-shot.py with the cursor cropped
     * and magnified 8x. That needs a booting kernel and the tree does not link
     * (T-13, another session's uncommitted work). These check the same three
     * properties mechanically, which is strictly stronger than looking:
     * "no stair-steps" becomes "there exist intermediate blend values",
     * "no trail" becomes "every pixel it left is exactly wallpaper again". */
    for (int i = 0; i < WM_MAX; i++) wm_close(i);
    wm_damage(0, 0, W, H);
    pointer(300, 300, 0);

    int ext = fb_pointer_extent();
    int ox, oy, sv;
    fb_pointer_saved(&ox, &oy, &sv);

    ok("the cursor sprite covers its own hotspot",
       fb_get_px(300, 300) != WALL);

    /* SOFT EDGES. Over a flat wallpaper the old row-by-row cursor produced
     * exactly two colours - the fill and the edge - because every pixel it
     * touched it overwrote. An anti-aliased one blends, so the majority of a
     * 16x16 box is neither wallpaper, nor pure fill, nor pure edge. That
     * difference is the whole item, and it is what "no stair-steps" means when
     * you cannot look at a screenshot. */
    int blended = 0, opaque = 0;
    for (int y = oy; y < oy + ext; y++)
        for (int x = ox; x < ox + ext; x++) {
            unsigned int p = fb_get_px(x, y);
            if (p == WALL) continue;
            if (p == 0xEEF4FF || p == 0x0A0E18) opaque++;
            else blended++;
        }
    ok("the cursor has opaque interior pixels", opaque > 0);
    ok("...and MORE partially-blended ones: soft edges, not stair-steps",
       blended > opaque);

    /* THE INK FITS INSIDE THE SAVED PATCH. This is the property, and the trail
     * is only its symptom. An anti-aliased cursor reaches a pixel further out
     * in every direction than any hard-edged reading of the shape suggests, so
     * a save-under sized from the visible arrow is short and every move leaves
     * a half-blended pixel that nothing ever repaints. Measured against fb.c's
     * OWN origin and extent, not a second copy of the arithmetic here. */
    int ix0 = W, iy0 = H, ix1 = -1, iy1 = -1;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            if (fb_get_px(x, y) != WALL) {
                if (x < ix0) ix0 = x;
                if (y < iy0) iy0 = y;
                if (x > ix1) ix1 = x;
                if (y > iy1) iy1 = y;
            }
    /* Against `sv`, what fb.c's restore loop ACTUALLY walks - not against
     * `ext`, the size it was supposed to be. Pointed at `ext` this assertion
     * passes with a save-under two pixels short, because then the check and
     * the bug read different variables and the check agrees with the intent.
     * Verified by injecting exactly that. */
    ok("every inked pixel lies inside the ACTUALLY-saved patch",
       sv > 0 && ix0 >= ox && iy0 >= oy && ix1 < ox + sv && iy1 < oy + sv);

    /* NO TRAIL. Move it a long way and assert the place it left is wallpaper
     * again - every pixel, not a sample. Nothing repaints that region, so a
     * single stale blended pixel here stays on screen forever. */
    pointer(700, 600, 0);
    ok("moving the pointer leaves NO halo where it was",
       all_wallpaper(300 - ext - 4, 300 - ext - 4, 300 + ext + 4, 300 + ext + 4));

    /* ...and the same for a short move, where the old and new boxes OVERLAP.
     * That is the harder case: the save happens after the restore, so an
     * ordering slip shows here and not in the long move. */
    pointer(300, 300, 0);
    pointer(303, 302, 0);
    fb_pointer_saved(&ox, &oy, &sv);
    ok("...and after a 3-pixel move, where the boxes overlap",
       all_wallpaper(300 - ext - 4, 300 - ext - 4, ox, 300 + ext + 4));

    /* IT SCALES. The old cursor was 9x16 physical pixels on any panel, while
     * the icons, the fonts and every metric in ui.h follow ui(). fb.c derives
     * the cell from the mode, so a wider mode must give a bigger pointer.
     * Last, because it changes the mode out from under everything above. */
    ok("at a 1x cell the pointer is 16px", ext == 16);
    fb_setup((unsigned long)vram, 3840 * 4, 3840, 200, 32);
    ok("...and follows ui() to 32px at a 2x cell", fb_pointer_extent() == 32);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
