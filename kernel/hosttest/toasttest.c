/* toasttest.c - the notification surface, INSIDE the compositor.
 *
 * systest.c asserts notify.c's own behaviour: the queue, the expiry, the tick
 * wrap. That leaves the part that only exists once wm.c is involved, and it is
 * the part the brief actually names as the gate:
 *
 *     "it appears, it expires on its own, it does not steal focus. That last
 *      one matters: a toast that takes focus eats the next keystroke."
 *
 * Focus is the interesting one. A toast here is NOT a window - it is not in
 * `wins`, not in `zorder`, and has no window id - so the claim is not "it is
 * careful with focus", it is "there is nothing here that could take it". That
 * is checkable: run the frames, then look at wm_focused() and at where a
 * keystroke went.
 *
 * The rest is pixels. A toast that is drawn behind the windows, or clipped
 * away by the damage rectangle it asked for, looks exactly like a toast that
 * was never posted.
 *
 * Same fake hardware as wmtest.c - fb.c against mmap'd memory at the addresses
 * it hardcodes, a scriptable pointer, and a clock that only moves when this
 * file says so.
 *
 * Build and run:  ./build.sh && ./toasttest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "../ui.h"

/* ---- fb.c ---------------------------------------------------------------- */
void fb_setup(unsigned long addr, unsigned int pitch, unsigned int width,
              unsigned int height, unsigned char bpp);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
unsigned int fb_get_px(int x, int y);
void fb_clip_none(void);

/* ---- snap.c -------------------------------------------------------------- */
#define SNAP_NONE 0
#define SNAP_LEFT 1
#define SNAP_RIGHT 2
#define SNAP_MAX 3
#define SNAP_TL 4
#define SK_DOWN 4
void wm_snap_key(int win, int dir);
void snap_reset(void);
int  snap_state(int win);
void snap_rect(int z, int sw, int sh, int rt, int rb, int *x, int *y, int *w, int *h);

/* ---- notify.c ------------------------------------------------------------ */
void        notify_reset(void);
int         notify_post(const char *text, unsigned ticks);
int         notify_active(void);
int         notify_queued(void);
const char *notify_text(void);
void        notify_rect(int sw, int sh, int rb, int scale,
                        int *x, int *y, int *w, int *h);

/* ---- wmglue.c ------------------------------------------------------------ */
#include "../../runtime.h"
Value zl_num(double n)
{
    Value v; memset(&v, 0, sizeof v); v.type = V_NUM; v.num = n; return v;
}

/* ---- fake hardware, identical to wmtest.c's ------------------------------ */
static int fake_x = 10, fake_y = 10, fake_btn = 0;
static unsigned fake_ticks = 1;

int idt_mouse_x(void)   { return fake_x; }
int idt_mouse_y(void)   { return fake_y; }
int idt_mouse_btn(void) { return fake_btn; }
unsigned int idt_ticks(void) { return fake_ticks; }
int idt_scan(void)      { return 0; }
int xhci_key(void)      { return 0; }
void idt_set_pointer_bounds(int w, int h) { (void)w; (void)h; }
void zl_putc_pub(char c) { (void)c; }

#define W 1280
#define H 800
#define BG_ADDR   0x08000000UL
#define SP_ADDR   0x0A000000UL
#define BACK_ADDR 0x0C000000UL
#define DOCK_H    (64 * 2)          /* kernel.zl's dock_y(), at scale 2 */

/* the app paints its whole client area one flat colour, so anything that is
 * NOT that colour inside a window is something else drawing on top */
#define APP_COLOUR 0x00112233u
#define WALL       0x00203040u

static void t_draw(int app, int x, int y, int w, int h, int focused)
{
    (void)app; (void)focused;
    fb_fill_px(x, y, w, h, APP_COLOUR);
}
/* Keys are not counted here on purpose. "Does not steal focus" is asserted
 * STRUCTURALLY below - wm_focused(), wm_count() and wm_zorder_at() - because a
 * toast that is not a window cannot be a focus target, and that is a stronger
 * statement than one keystroke happening to land in the right place. */
static int t_event(int app, int win, int type, int code, int x, int y)
{
    (void)app; (void)win; (void)type; (void)code; (void)x; (void)y;
    return 1;
}
static int t_tick(int app, int win) { (void)app; (void)win; return 0; }
static void t_desk(int x, int y, int w, int h) { fb_fill_px(x, y, w, h, WALL); }

static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-62s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}
static void frame(void) { fake_ticks++; wm_frame(); }

/* move the pointer and run one frame - the compositor only looks at the mouse
 * once per tick, so a drag is a sequence of these, not a sequence of writes */
/* THE TABLET, NOT THE PS/2 MOUSE. This helper is absolute - every caller says
 * "the pointer is at (x,y)" and then asserts on what the compositor did there.
 * desktop/feel-and-control made the PS/2 path RELATIVE, accumulating scaled
 * deltas into its own px_x/px_y, so a drag written as three absolute jumps
 * never arrives where it says. A tablet reports an absolute position and is
 * not accelerated. inputtest covers the relative path deliberately. */
static int tab_x, tab_y, tab_btn;
int xhci_ptr_ready(void) { return 1; }
int xhci_ptr_x(void)     { return tab_x; }
int xhci_ptr_y(void)     { return tab_y; }
int xhci_ptr_btn(void)   { return tab_btn; }

static void pointer(int x, int y, int btn)
{
    tab_x = x; tab_y = y; tab_btn = btn;
    fake_x = x; fake_y = y; fake_btn = btn;
    frame();
}

/* how many pixels in this rectangle are neither wallpaper nor the app colour?
 * That is the toast: the only other thing that paints. */
static int foreign_px(int x0, int y0, int x1, int y1)
{
    int n = 0;
    if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
    if (x1 > W) x1 = W; if (y1 > H) y1 = H;
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++) {
            unsigned p = fb_get_px(x, y);
            if (p != WALL && p != APP_COLOUR) n++;
        }
    return n;
}

/* wm.c times a frame with the TSC (the frame stopwatch). No CPU here. */
unsigned cpu_tsc_lo(void)  { return 0; }
unsigned cpu_tsc_khz(void) { return 0; }

/* Hardware this harness does not fake. ser_rx answers -1 ("no UART"),
 * never 0, which would be a NUL byte and therefore a keystroke. */
int xhci_ptr_poll(void) { return 0; }
int xhci_key_event(void) { return 0; }
int xhci_kbd_mods(void) { return 0; }
int idt_mouse_wheel(void) { return 0; }
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

    printf("toasttest - notify.c INSIDE the compositor\n\n");

    ui_theme_init(2);
    wm_init();
    wm_hooks(t_draw, t_event, t_tick, t_desk);
    notify_reset();

    /* a window that COVERS where the toast goes, so "it appears" means "it
     * appears on top of a window" and not "it appears on empty wallpaper" */
    int tx, ty, tw, th;
    notify_rect(W, H, DOCK_H, 2, &tx, &ty, &tw, &th);
    int win = wm_open(1, "cover", tx - 60, ty - 60, tw + 120, th + 120);
    for (int i = 0; i < 8; i++) frame();          /* let the open animation settle */

    ok("the covering window has focus to start with", wm_focused() == win);
    ok("...and nothing foreign is drawn where the toast will go",
       foreign_px(tx, ty, tx + tw, ty + th) == 0);

    /* ---- IT APPEARS ------------------------------------------------------ */
    printf("  -- it appears --\n");
    notify_post("disk mounted", 200);
    ok("posting alone paints nothing - the frame loop owns the screen",
       foreign_px(tx, ty, tx + tw, ty + th) == 0);

    frame();                                       /* notify_tick promotes it */
    frame();                                       /* ...and the damage paints */
    int painted = foreign_px(tx, ty, tx + tw, ty + th);
    ok("after a frame the toast is ON SCREEN", painted > 0);
    printf("      %d pixels of toast inside its rectangle\n", painted);
    ok("...and it is ON TOP of the window, not behind it",
       painted > (tw * th) / 4);
    ok("...notify.c agrees it is active", notify_active() == 1);

    /* ---- IT DOES NOT STEAL FOCUS ----------------------------------------- */
    printf("  -- it does not steal focus --\n");
    ok("focus is STILL the window, not the toast", wm_focused() == win);
    ok("...and the window count is unchanged - a toast is not a window",
       wm_count() == 1);
    ok("...it has no place in the z-order at all", wm_zorder_at(0) == win);

    /* ---- IT EXPIRES ON ITS OWN ------------------------------------------- */
    printf("  -- it expires on its own --\n");
    for (int i = 0; i < 210; i++) frame();
    ok("after its ticks run out notify.c has retired it", notify_active() == 0);
    frame();
    int left = foreign_px(tx, ty, tx + tw, ty + th);
    ok("...and the pixels are GONE, repainted by what was under it", left == 0);
    printf("      %d pixels left behind\n", left);

    /* A toast that retires without damaging its own rectangle leaves a ghost
     * on the wallpaper. That is the classic compositor bug and it is only
     * visible in the frame AFTER the one you would screenshot. */
    ok("...leaving no ghost - the window under it is intact",
       fb_get_px(tx + tw / 2, ty + th / 2) == APP_COLOUR);

    /* ---- and the queue drains through the compositor, one at a time ------ */
    printf("  -- one at a time, through the frame loop --\n");
    notify_reset();
    notify_post("first",  60);
    notify_post("second", 60);
    frame(); frame();
    ok("the first is up", notify_active() && strcmp(notify_text(), "first") == 0);
    ok("...and the second is waiting, not also drawn", notify_queued() == 2);
    for (int i = 0; i < 65; i++) frame();
    ok("the second takes over by itself",
       notify_active() && strcmp(notify_text(), "second") == 0);
    ok("...and focus never moved through any of it", wm_focused() == win);

    /* =================================================================
     * SNAPPING, THROUGH THE COMPOSITOR
     *
     * systest.c asserts snap.c's arithmetic. This asserts the part that only
     * exists once wm.c is involved: that a DROP at an edge actually resizes
     * the window, that dragging it away again clears the state so un-snapping
     * cannot teleport it, and that wm_resize - which had no caller at all
     * until now - is really being reached.
     * ================================================================= */
    printf("\n  -- snapping, through the compositor --\n");
    notify_reset();
    snap_reset();
    wm_close(win);
    for (int i = 0; i < 4; i++) frame();

    const int RT = 32 * 2, RB = 64 * 2;      /* TOPBAR_H and dock, at scale 2 */
    int s0 = wm_open(1, "snap", 300, 300, 420, 260);
    for (int i = 0; i < 8; i++) frame();
    int gx, gy, gw, gh;
    wm_geometry(s0, &gx, &gy, &gw, &gh);
    ok("a window opens at the size it asked for", gw == 420 && gh == 260);

    /* pick it up by the title bar, drag to the left edge, drop */
    const struct ui_theme *thm = ui_theme();
    pointer(gx + 100, gy + thm->title_h / 2, 1);     /* press on the title bar */
    pointer(60, 400, 1);                            /* drag left              */
    pointer(2, 400, 1);                             /* ...to the edge         */
    pointer(2, 400, 0);                             /* drop                   */
    frame();

    int ex, ey, ew, eh;
    snap_rect(SNAP_LEFT, W, H, RT, RB, &ex, &ey, &ew, &eh);
    wm_geometry(s0, &gx, &gy, &gw, &gh);
    ok("dropping it at the left edge SNAPS it", snap_state(s0) == SNAP_LEFT);
    ok("...to exactly the left half of the work area",
       gx == ex && gy == ey && gw == ew && gh == eh);
    printf("      %dx%d at %d,%d   (work area starts at y=%d, dock at y=%d)\n",
           gw, gh, gx, gy, RT, H - RB);
    ok("...which is below the header", gy >= RT);
    ok("...and stops above the dock", gy + gh == H - RB);

    /* drag it back into the middle: no longer snapped, and NOT restored */
    pointer(gx + 100, gy + thm->title_h / 2, 1);
    pointer(600, 400, 1);
    pointer(600, 400, 0);
    frame();
    ok("dragging it off the edge clears the snap", snap_state(s0) == SNAP_NONE);
    wm_geometry(s0, &gx, &gy, &gw, &gh);
    ok("...and leaves it the size it was, not teleported", gw == ew && gh == eh);

    /* a fresh window, snapped left then right, must restore to its ORIGINAL */
    snap_reset();
    wm_close(s0);
    for (int i = 0; i < 4; i++) frame();
    int s1 = wm_open(1, "restore", 250, 250, 500, 320);
    for (int i = 0; i < 8; i++) frame();

    pointer(250 + 60, 250 + thm->title_h / 2, 1);
    pointer(2, 400, 1);
    pointer(2, 400, 0);                             /* -> left half */
    frame();
    wm_geometry(s1, &gx, &gy, &gw, &gh);
    ok("snapped left", snap_state(s1) == SNAP_LEFT && gw == ew);

    pointer(gx + 60, gy + thm->title_h / 2, 1);
    pointer(W - 2, 400, 1);
    pointer(W - 2, 400, 0);                         /* -> right half */
    frame();
    ok("...then right", snap_state(s1) == SNAP_RIGHT);

    /* Super+Down, through the same entry point route_key uses */
    wm_snap_key(s1, SK_DOWN);
    frame();
    wm_geometry(s1, &gx, &gy, &gw, &gh);
    ok("un-snapping restores the ORIGINAL size, not the left half it passed through",
       gw == 500 && gh == 320);
    ok("...and its original position", gx == 250 && gy == 250);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
