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

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
