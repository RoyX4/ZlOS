/* wm.c - the compositor: a window table, a z-order, focus, damage, routing.
 *
 * This replaces the shell's while-loop as the top of the system. Today
 * kernel.zl ends with `while running == 1 { prompt, read a key, run_command }`
 * and the shell IS the machine; after this the FRAME LOOP is, and the shell is
 * one window inside it.
 *
 * MECHANISM ONLY. What a window contains, what the dock launches, which apps
 * exist and what colour anything is are all policy and live above this file.
 * wm.c calls fb_clip and app_draw; it never draws a widget and never knows
 * what a button is.
 *
 * NO ALLOCATION ANYWHERE. Fixed arrays, WM_MAX windows, and "no free slot" is
 * a refusal that says so - never a silent drop. The zl kernel subset has no
 * lists at all (zl_list_n is a hard fault), which is why the window table is
 * in C rather than in kernel.zl: it is not a style choice, the language cannot
 * express it.
 *
 * Z-ORDER IS THE zorder ARRAY. Iteration order is paint order, back to front.
 * Raise is remove-and-append. Hit-test walks it backwards. There is no other
 * representation of "which window is on top" to get out of sync.
 */

#include "ui.h"

/* ---- fb.c ---------------------------------------------------------------- */
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);
int  fb_active(void);
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_gradient(int x, int y, int w, int h, unsigned int top, unsigned int bot);
void fb_rrect_grad_top(int x, int y, int w, int h, int r, unsigned int top, unsigned int bot);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_shadow(int x, int y, int w, int h, int off, int soft);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
/* the fade's two halves - see fb.c */
int  fb_stash(int x, int y, int w, int h);
void fb_stash_blend(int slot, int x, int y, int a);
void fb_blur_free(int slot);
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a);
void fb_fill_blend(int x, int y, int w, int h, unsigned int rgb, int a);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
/* Titles are LABELS, not console text, so they take the proportional path.
 * That is the single change desktop-look.md item 4 asks for at this layer. */
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
void fb_present(void);
void fb_pointer_show(int x, int y);
void fb_pointer_hide(void);
int  fb_cell_w(void);
int  fb_cell_h(void);

/* ---- input.c ------------------------------------------------------------- */
void input_poll(void);
int  input_next(void);
int  input_code(void);
int  input_mods(void);
int  input_x(void);
int  input_y(void);

#define EV_NONE      0
#define EV_KEY_DOWN  1
#define EV_KEY_UP    2
#define EV_CHAR      3
#define EV_MOUSE     4

#define KEY_SUPER   0x11A
#define MOD_ALT     (1 << 2)
#define MOD_SUPER   (1 << 5)

unsigned int idt_ticks(void);

/* The one character sink the whole kernel prints through. wm.c uses it for
 * refusals only - anything it declines to do says so, on the serial log, where
 * an unattended gate can read it. */
void zl_putc_pub(char c);
static void wm_puts(const char *s) { while (*s) zl_putc_pub(*s++); }

/* ---- the table ----------------------------------------------------------- */
struct win {
    int x, y, w, h;
    int app;
    int flags;
    int min_w, min_h;
    char title[32];
    /* TABS. Several apps sharing one frame, grouped by task - the idea worth
     * stealing from Essence. It is cheap here because a window already has
     * exactly one thing a tab needs to change: which app_draw gets called. */
    int  tab_app[WM_TABS];
    char tab_title[WM_TABS][16];
    int  ntab;                 /* 1 for an ordinary window */
    int  tab;                  /* which one is showing     */
};

static struct win wins[WM_MAX];
static int zorder[WM_MAX];          /* window indices, BACK to FRONT */
static int nz;                      /* how many are in the z-order   */
static int focus_win = -1;
/* ZERO UNTIL wm_init() RUNS, and that matters more than it looks. wm_running()
 * is how the rest of the system asks "is the compositor the top of the system
 * right now" - draw_screen() uses it to choose between damaging the screen and
 * redrawing a text desktop, and help() uses it to choose which set of commands
 * to describe. Initialised to 1, it answered yes on a machine with no
 * framebuffer, where the compositor had never been near the screen: the text
 * shell printed the compositor's help and verify.sh caught it. */
static int running = 0;

static app_draw_fn  hook_draw;
static app_event_fn hook_event;
static app_tick_fn  hook_tick;
static desk_draw_fn hook_desk;
/* A CLICK THAT HITS NO WINDOW WAS DROPPED, and the dock is not a window.
 * desk_draw has painted a dock, a start button and a tray since the compositor
 * booted, and every one of them was decoration: route_mouse found no window
 * under the pointer and returned. Desktop furniture needs a route of its own
 * for the same reason it needs a draw of its own - it is not in the z-order
 * and never will be. */
static desk_click_fn hook_desk_click;
static desk_key_fn   hook_desk_key;

/* ---- the damage list ------------------------------------------------------
 * NOT fb.c's. They are different questions and conflating them is a bug
 * waiting to happen:
 *
 *   wm damage   "these screen regions need REPAINTING" - a window moved, an
 *               app changed, a menu closed. Consumed by wm_repaint, which
 *               clips to each in turn.
 *   fb damage   "these pixels CHANGED and need blitting to the card".
 *               Accumulated by the drawing that repaint then does, and
 *               consumed by fb_present.
 *
 * One is intent, the other is consequence. Repainting a region always produces
 * fb damage, but not all fb damage came from a repaint - the pointer sprite,
 * for one - and fb's list is emptied by present, which happens after.
 */
#define WD_MAX 8
static struct { int x0, y0, x1, y1; } wd[WD_MAX];
static int nwd;

static int wd_touches(int i, int x0, int y0, int x1, int y1)
{
    return !(x0 > wd[i].x1 || x1 < wd[i].x0 || y0 > wd[i].y1 || y1 < wd[i].y0);
}

void wm_damage(int x, int y, int w, int h)
{
    int x0 = x, y0 = y, x1 = x + w, y1 = y + h;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int)fb_pxw()) x1 = (int)fb_pxw();
    if (y1 > (int)fb_pxh()) y1 = (int)fb_pxh();
    if (x0 >= x1 || y0 >= y1) return;

    for (int i = 0; i < nwd; ) {
        if (wd_touches(i, x0, y0, x1, y1)) {
            if (wd[i].x0 < x0) x0 = wd[i].x0;
            if (wd[i].y0 < y0) y0 = wd[i].y0;
            if (wd[i].x1 > x1) x1 = wd[i].x1;
            if (wd[i].y1 > y1) y1 = wd[i].y1;
            wd[i] = wd[--nwd];
            i = 0;                    /* the union grew; re-test everything */
            continue;
        }
        i++;
    }
    if (nwd >= WD_MAX) {              /* full: one rectangle, as before */
        for (int i = 0; i < nwd; i++) {
            if (wd[i].x0 < x0) x0 = wd[i].x0;
            if (wd[i].y0 < y0) y0 = wd[i].y0;
            if (wd[i].x1 > x1) x1 = wd[i].x1;
            if (wd[i].y1 > y1) y1 = wd[i].y1;
        }
        nwd = 0;
    }
    wd[nwd].x0 = x0; wd[nwd].y0 = y0;
    wd[nwd].x1 = x1; wd[nwd].y1 = y1;
    nwd++;
}

/* A window's damage is its FRAME PLUS ITS SHADOW. Erasing only the frame is
 * the bug that leaves a smear trail behind a dragged window: fb_shadow reaches
 * off + soft beyond the footprint, so anything that repaints w+16 when the
 * shadow drew to w+28 leaves 12 px of it on screen, every step of the drag. */
#define SHADOW_OFF(t)   (UI_S2(t))
#define SHADOW_SOFT(t)  (UI_S3(t) / 2)

/* ELEVATION MAKES THE SHADOW A VARIABLE, AND THAT IS A TRAP.
 *
 * chrome() draws three sizes - a modal at 1.5x, a focused window at 1x, an
 * unfocused one at about half - so "how far does this window's shadow reach"
 * is no longer one number. Damaging the wrong one under-damages, and
 * under-damage is precisely the smear the comment above is about.
 *
 * Two ways it bites, both real:
 *   a MODAL reaches 42 px at scale 2 while a fixed reach damages 28, so
 *   closing or moving one leaves 14 px of shadow behind;
 *   FOCUS CHANGE shrinks the shadow, and wm_focus damages AFTER updating
 *   focus_win - so the window that just lost focus would be damaged at its
 *   new, smaller size while its old, larger shadow is still on screen.
 *
 * So this returns the MAXIMUM reach for the window's flags, over both focus
 * states. Over-damaging costs a few pixels of repaint; under-damaging leaves
 * dirt on the screen that nothing will ever clean up. */
static int shadow_reach(int win)
{
    const struct ui_theme *t = ui_theme();
    int off = SHADOW_OFF(t), soft = SHADOW_SOFT(t);
    if (wins[win].flags & WF_MODAL) { off = off * 3 / 2; soft = soft * 3 / 2; }
    return off + soft;          /* the focused size; unfocused is smaller */
}

void wm_damage_win(int win)
{
    if (win < 0 || win >= WM_MAX || !(wins[win].flags & WF_OPEN)) return;
    int reach = shadow_reach(win);
    wm_damage(wins[win].x - reach, wins[win].y - reach,
              wins[win].w + 2 * reach, wins[win].h + 2 * reach);
}

/* ---- motion ---------------------------------------------------------------
 * Nothing in zlOS animated at all: windows appeared instantly, menus popped,
 * focus snapped. desktop-polish-and-speed.md calls that the single biggest
 * "feels modern" gap, and it was gated on damage tracking - a window that
 * appears over several frames is composited several times, which is only
 * affordable once that costs a rectangle instead of the screen.
 *
 * FOUR FRAMES. Not an easing curve, not a timeline system, not 60fps. A window
 * that grows into place over four frames already feels different from one that
 * teleports, and four frames at 100 Hz is 40 ms - under the ~100 ms where a
 * person starts calling it slow.
 *
 * It is a SCALE, not a fade, and that is not a compromise. A fade needs the
 * window composited against the background at a fraction of opacity, which
 * means an offscreen buffer this kernel has nowhere to put. A scale needs
 * nothing new: apps are already required to be size-agnostic by the app
 * contract, so drawing one at 82% is just drawing it, and the chrome is
 * parametric already.
 *
 * HIT TESTING IGNORES ALL OF IT. wm_at and the routing use the settled
 * geometry from the first frame, so a click during those 40 ms lands where the
 * window is ABOUT to be rather than where it momentarily looks. A pointer that
 * misses a target because the target was still growing is worse than no
 * animation.
 */
#define ANIM_FRAMES 4

/* ---- the timeline ---------------------------------------------------------
 * What was here was ONE animation, hardcoded into the window struct as a frame
 * counter, and it could only ever be the open-scale. The prototype names seven
 * keyframes - zov, zpop, zpress, zpulse, zsweep, ztoast, zwin - which is not
 * seven times as much code, it is the same code with the kind as a parameter
 * and the steps in a table.
 *
 * A FIXED ARRAY, ticked once per frame, each entry marking its target damaged.
 * No allocation, no list, no callbacks. An animation that finishes frees its
 * slot; a slot that cannot be found is a refusal, not a silent drop.
 *
 * STILL NO EASING CURVES, deliberately. Each kind is 4-8 integer steps in a
 * table. A table cannot produce a wrong in-between value, cannot overshoot
 * into a negative size, and cannot be got wrong by a cubic evaluated in
 * fixed point - and at 4-8 frames nobody can see the difference between a
 * table and a curve anyway.
 *
 * WHAT EACH KIND IS, and which are drawn rather than merely stored:
 *
 *   ANIM_OPEN    scale from 82% to 100%   - the window open (was `anim`)
 *   ANIM_CLOSE   scale from 100% to 82%   - its mirror
 *   ANIM_PRESS   scale 100 -> 96 -> 100   - zpress, a control acknowledging
 *   ANIM_PULSE   opacity 0 -> 40 -> 0     - zpulse, attention without motion
 *   ANIM_FADE    opacity 0 -> 100         - zov/zpop/ztoast, the opacity fades
 *
 * The opacity kinds are expressible only because fb_fill_blend exists; before
 * it, a fade needed an offscreen buffer per window and this kernel has nowhere
 * to put one. That is why v10 orders translucency before the timeline.
 */
#define ANIM_MAX 8

#define ANIM_NONE   0
#define ANIM_OPEN   1
#define ANIM_CLOSE  2
#define ANIM_PRESS  3
#define ANIM_PULSE  4
#define ANIM_FADE   5

/* Each row is read from the LAST element backwards as the counter runs down,
 * so index 0 is where the animation starts and the settled value is what the
 * window has when no animation is running at all. */
static const unsigned char anim_scale[][8] = {
    /* OPEN  */ { 82, 90, 95, 98, 100, 100, 100, 100 },
    /* CLOSE */ { 98, 95, 90, 82,  70,  70,  70,  70 },
    /* PRESS */ { 96, 97, 98, 99, 100, 100, 100, 100 },
};
static const unsigned char anim_len[] = {
    /* NONE */ 0, /* OPEN */ 4, /* CLOSE */ 4, /* PRESS */ 4,
    /* PULSE */ 6, /* FADE */ 5,
};
static const unsigned char anim_alpha[][8] = {
    /* PULSE */ {  0, 24, 40, 40, 24,  0, 0, 0 },
    /* FADE  */ { 60, 90, 120, 170, 220, 255, 255, 255 },
};

struct anim { int win; int kind; int frame; int len; };
static struct anim anims[ANIM_MAX];

/* Start one. Returns 0 and says so if every slot is busy - the same refusal
 * discipline as wm_open's WM_MAX, and for the same reason: a silently dropped
 * animation is a UI that is intermittently unresponsive for no visible cause. */
int wm_anim(int win, int kind)
{
    if (kind <= ANIM_NONE || kind >= (int)(sizeof anim_len / sizeof anim_len[0]))
        return 0;
    /* One animation per window per kind. Re-triggering restarts it, which is
     * what a button pressed twice in quick succession should look like. */
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind && anims[i].win == win && anims[i].kind == kind) {
            anims[i].frame = 0;
            return 1;
        }
    for (int i = 0; i < ANIM_MAX; i++) {
        if (anims[i].kind) continue;
        anims[i].win = win;
        anims[i].kind = kind;
        anims[i].frame = 0;
        anims[i].len = anim_len[kind];
        wm_damage_win(win);
        return 1;
    }
    wm_puts("  wm: no free animation slot, refusing\n");
    return 0;
}

/* Which frame of `kind` is window `win` on, or -1 for "not animating". */
static int anim_frame_of(int win, int kind)
{
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind == kind && anims[i].win == win) return anims[i].frame;
    return -1;
}

int wm_anim_running(int win)
{
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind && anims[i].win == win) return anims[i].kind;
    return 0;
}

/* The extra opacity a window is being drawn with, 0..255, or 255 for settled.
 * Read by the repaint; exposed so a test can assert on it without a screenshot. */
int wm_anim_alpha(int win)
{
    int f = anim_frame_of(win, ANIM_FADE);
    if (f >= 0) return anim_alpha[1][f];
    f = anim_frame_of(win, ANIM_PULSE);
    if (f >= 0) return anim_alpha[0][f];
    return 255;
}

/* Advance every running animation by one frame and damage what moved.
 * Damaging the SETTLED rect - which is the largest - is what erases the
 * smaller frame drawn a moment ago. */
static void anim_tick(void)
{
    for (int i = 0; i < ANIM_MAX; i++) {
        if (!anims[i].kind) continue;
        wm_damage_win(anims[i].win);
        /* AN ANIMATION NEVER CHANGES WINDOW LIFETIME. It was tempting to have
         * ANIM_CLOSE call wm_close() when it finishes, so a closing window
         * shrinks away; that would make "the window closed" depend on a free
         * animation slot, and wm_anim() is allowed to refuse. A window that
         * sometimes does not close when every slot is busy is a far worse bug
         * than a window that closes without a flourish. The timeline draws;
         * the caller decides what exists. */
        if (++anims[i].frame >= anims[i].len) anims[i].kind = ANIM_NONE;
    }
}

/* how big window `win` should be DRAWN this frame, as a percentage */
static int anim_pct(int win)
{
    /* ONE MECHANISM. The open scale used to be a counter in the window struct
     * and the timeline was a second thing beside it that nothing triggered -
     * so wm.c carried two animation systems, one of which never ran. wm_open()
     * starts an ANIM_OPEN now and this reads it, which means the open scale
     * and every other kind share a code path and a bug in one is a bug you can
     * actually see. */
    int f = anim_frame_of(win, ANIM_OPEN);
    if (f >= 0) return anim_scale[0][f];
    f = anim_frame_of(win, ANIM_CLOSE);
    if (f >= 0) return anim_scale[1][f];
    f = anim_frame_of(win, ANIM_PRESS);
    if (f >= 0) return anim_scale[2][f];
    return 100;
}

static void anim_rect(int win, int *x, int *y, int *w, int *h)
{
    int p = anim_pct(win);
    struct win *W = &wins[win];
    *w = W->w * p / 100;
    *h = W->h * p / 100;
    /* grow from the CENTRE - a window that grows from its top-left corner
     * reads as sliding, which says something different */
    *x = W->x + (W->w - *w) / 2;
    *y = W->y + (W->h - *h) / 2;
}

/* ---- z-order ------------------------------------------------------------- */
static int z_index_of(int win)
{
    for (int i = 0; i < nz; i++) if (zorder[i] == win) return i;
    return -1;
}

static void z_remove(int win)
{
    int i = z_index_of(win);
    if (i < 0) return;
    for (; i < nz - 1; i++) zorder[i] = zorder[i + 1];
    nz--;
}

static void z_append(int win)
{
    z_remove(win);
    if (nz < WM_MAX) zorder[nz++] = win;
}

int wm_zorder_at(int i) { return (i >= 0 && i < nz) ? zorder[i] : -1; }
int wm_count(void)      { return nz; }
int wm_focused(void)    { return focus_win; }
int wm_running(void)    { return running; }
void wm_stop(void)      { running = 0; }
int wm_is_open(int win)
{
    return win >= 0 && win < WM_MAX && (wins[win].flags & WF_OPEN);
}

void wm_geometry(int win, int *x, int *y, int *w, int *h)
{
    if (!wm_is_open(win)) { *x = *y = *w = *h = 0; return; }
    *x = wins[win].x; *y = wins[win].y; *w = wins[win].w; *h = wins[win].h;
}

/* The CLIENT area: inside the frame, below the title bar. This is the second,
 * narrower scissor in the repaint - the one that means an app physically
 * cannot draw over its own title bar no matter what it does. */
static void client_of(int fx, int fy, int fw, int fh, int flags,
                      int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    int b  = (flags & WF_NOCHROME) ? 0 : 2;
    int th = (flags & WF_NOCHROME) ? 0 : t->title_h;
    *x = fx + b;
    *y = fy + th;
    *w = fw - 2 * b;
    *h = fh - th - b;
    if (*w < 0) *w = 0;
    if (*h < 0) *h = 0;
}

/* The SETTLED client area - where the window will be, not where it may
 * momentarily be drawn mid-animation. Hit testing and app coordinates outside
 * the repaint both want this one. */
void wm_client(int win, int *x, int *y, int *w, int *h)
{
    if (!wm_is_open(win)) { *x = *y = *w = *h = 0; return; }
    client_of(wins[win].x, wins[win].y, wins[win].w, wins[win].h,
              wins[win].flags, x, y, w, h);
}

/* ---- lifecycle ------------------------------------------------------------
 * Every operation is DEFINED by what it damages. Get that wrong and the bug is
 * not a crash, it is a smear that only shows on some backgrounds. */
void wm_init(void)
{
    for (int i = 0; i < WM_MAX; i++) wins[i].flags = 0;
    nz = 0;
    nwd = 0;
    focus_win = -1;
    running = 1;
    if (fb_active()) wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
}

void wm_desk_click(desk_click_fn f) { hook_desk_click = f; }
void wm_desk_key(desk_key_fn f)     { hook_desk_key = f; }

void wm_hooks(app_draw_fn d, app_event_fn e, app_tick_fn t, desk_draw_fn desk)
{
    hook_draw = d; hook_event = e; hook_tick = t; hook_desk = desk;
}

static void title_copy(char *dst, const char *src)
{
    int i = 0;
    if (src) for (; src[i] && i < 31; i++) dst[i] = src[i];
    dst[i] = 0;
}

static void title_copy16(char *dst, const char *src)
{
    int i = 0;
    if (src) for (; src[i] && i < 15; i++) dst[i] = src[i];
    dst[i] = 0;
}

/* Add another app to this window's frame. Returns the tab index, or -1 when
 * the frame is full - a refusal that says so, like wm_open's. */
int wm_add_tab(int win, int app, const char *title)
{
    if (!wm_is_open(win)) return -1;
    if (wins[win].ntab >= WM_TABS) {
        wm_puts("  wm: window already has WM_TABS tabs, refusing to add\n");
        return -1;
    }
    int i = wins[win].ntab++;
    wins[win].tab_app[i] = app;
    title_copy16(wins[win].tab_title[i], title);
    wm_damage_win(win);
    return i;
}

int wm_tab(int win)   { return wm_is_open(win) ? wins[win].tab  : -1; }
int wm_ntabs(int win) { return wm_is_open(win) ? wins[win].ntab : 0; }

void wm_set_tab(int win, int tab)
{
    if (!wm_is_open(win) || tab < 0 || tab >= wins[win].ntab) return;
    if (wins[win].tab == tab) return;
    wins[win].tab = tab;
    wm_damage_win(win);
}

/* Which app this window is SHOWING. Everything downstream asks this rather
 * than reading .app, so a tabbed window and a plain one are the same thing to
 * the repaint and the routing - which is what keeps tabs from being a special
 * case threaded through the whole file. */
static int win_app(int win) { return wins[win].tab_app[wins[win].tab]; }

int wm_open(int app, const char *title, int x, int y, int w, int h)
{
    for (int i = 0; i < WM_MAX; i++) {
        if (wins[i].flags & WF_OPEN) continue;
        wins[i].x = x; wins[i].y = y; wins[i].w = w; wins[i].h = h;
        wins[i].app = app;
        wins[i].flags = WF_OPEN;
        wins[i].min_w = 8 * fb_cell_w();
        wins[i].min_h = 4 * fb_cell_h();
        wins[i].ntab = 1;
        wins[i].tab = 0;
        wins[i].tab_app[0] = app;
        title_copy16(wins[i].tab_title[0], title);
        title_copy(wins[i].title, title);
        z_append(i);
        focus_win = i;
        /* A refusal here degrades gracefully: every slot busy means the window
         * opens without a flourish, which is the right way for an animation to
         * fail. */
        wm_anim(i, ANIM_OPEN);
        wm_damage_win(i);
        return i;
    }
    /* WM_MAX is a hard ceiling, and a refusal has to SAY SO. A silent -1 is
     * how "the dock stopped launching things" becomes a twenty-minute hunt -
     * and it is the same bug class as the back buffer switching itself off
     * without a word (desktop-TODO 0a). */
    wm_puts("  wm: no free window slot (WM_MAX reached), refusing to open\n");
    return -1;
}

void wm_close(int win)
{
    if (!wm_is_open(win)) return;
    wm_damage_win(win);
    wins[win].flags = 0;
    z_remove(win);
    /* focus the new top, so closing never leaves keys going nowhere */
    focus_win = nz ? zorder[nz - 1] : -1;
}

void wm_raise(int win)
{
    if (!wm_is_open(win)) return;
    if (nz && zorder[nz - 1] == win) return;       /* already on top */
    z_append(win);
    wm_damage_win(win);
}

/* Focus does NOT imply raise. A menu can take the keys without reordering the
 * stack underneath it, and a click-to-focus that also raised would make that
 * impossible to express. */
void wm_focus(int win)
{
    if (focus_win == win) return;
    int old = focus_win;
    focus_win = win;
    /* both title bars change: the old loses its hue and underline, the new
     * gains them. Two damages, not one. */
    if (wm_is_open(old)) wm_damage_win(old);
    if (wm_is_open(win)) wm_damage_win(win);
}

/* WF_MODAL had no setter, so the modal branch in route_mouse was code that
 * could never execute - which is exactly the hazard HANDOFF.md names for
 * intel.c: "the code exists" is not "the code works", check for an actual
 * caller. C5 makes the start menu a modal window; this is what it will call. */
void wm_set_modal(int win, int on)
{
    if (!wm_is_open(win)) return;
    /* Damage BOTH ways round. Turning modal off shrinks the shadow, so
     * damaging only afterwards would leave the larger one's edge behind - the
     * same asymmetry as the focus change above. */
    wm_damage_win(win);
    if (on) wins[win].flags |= WF_MODAL;
    else    wins[win].flags &= ~WF_MODAL;
    wm_damage_win(win);
}

void wm_move(int win, int x, int y)
{
    if (!wm_is_open(win)) return;
    if (wins[win].x == x && wins[win].y == y) return;
    wm_damage_win(win);                 /* the OLD rect */
    wins[win].x = x;
    wins[win].y = y;
    wm_damage_win(win);                 /* UNION the new one */
}

void wm_resize(int win, int w, int h)
{
    if (!wm_is_open(win)) return;
    if (w < wins[win].min_w) w = wins[win].min_w;
    if (h < wins[win].min_h) h = wins[win].min_h;
    if (wins[win].w == w && wins[win].h == h) return;
    wm_damage_win(win);
    wins[win].w = w;
    wins[win].h = h;
    wm_damage_win(win);
}

/* ---- hit testing ---------------------------------------------------------- */
static int win_contains(int win, int x, int y)
{
    return x >= wins[win].x && x < wins[win].x + wins[win].w &&
           y >= wins[win].y && y < wins[win].y + wins[win].h;
}

int wm_at(int x, int y)
{
    for (int i = nz - 1; i >= 0; i--)          /* BACKWARDS: topmost first */
        if (win_contains(zorder[i], x, y)) return zorder[i];
    return -1;
}

static int modal_win(void)
{
    for (int i = nz - 1; i >= 0; i--)
        if (wins[zorder[i]].flags & WF_MODAL) return zorder[i];
    return -1;
}

/* Which app a window is showing - the ACTIVE tab's, not the one it was opened
 * with. Everything downstream asks this rather than reading .app, which is
 * what keeps a tabbed window and a plain one the same thing to the repaint,
 * the routing and now the taskbar. */
int wm_win_app(int win)
{
    if (!wm_is_open(win)) return -1;
    return win_app(win);
}


/* ---- the repaint ----------------------------------------------------------
 * The clip is set TWICE per window and that is the whole point:
 *   once to the frame, so chrome cannot bleed onto a neighbour
 *   once NARROWER to the client area, so an app PHYSICALLY CANNOT draw over
 *   its own title bar or outside its window, no matter what it does
 * An app that tries to draw at -500,-500 simply produces nothing. That
 * guarantee is what fb_clip was built for.
 */
static int isect(int ax0, int ay0, int ax1, int ay1,
                 int bx0, int by0, int bx1, int by1,
                 int *ox, int *oy, int *ow, int *oh)
{
    int x0 = ax0 > bx0 ? ax0 : bx0, y0 = ay0 > by0 ? ay0 : by0;
    int x1 = ax1 < bx1 ? ax1 : bx1, y1 = ay1 < by1 ? ay1 : by1;
    if (x0 >= x1 || y0 >= y1) return 0;
    *ox = x0; *oy = y0; *ow = x1 - x0; *oh = y1 - y0;
    return 1;
}

static void tab_rect(int win, int i, int *x, int *y, int *w, int *h);

/* Where the pointer is and what it is doing. chrome() reads these for hover
 * and press states, so they are declared here rather than down in the routing
 * section that writes them. */
static int last_btn;
static int ptr_x, ptr_y;

/* ELEVATION. Every window used to get the identical shadow, focused or not.
 * Real desktops encode a hierarchy - a menu above a window above the desktop -
 * and `off` and `soft` were ALREADY parameters of fb_shadow, so three levels
 * cost nothing but deciding. */
static void chrome(int win, int focused)
{
    const struct ui_theme *t = ui_theme();
    struct win Wa = wins[win];
    struct win *W = &Wa;
    anim_rect(win, &W->x, &W->y, &W->w, &W->h);
    int off  = SHADOW_OFF(t), soft = SHADOW_SOFT(t);
    if (W->flags & WF_MODAL) { off = off * 3 / 2; soft = soft * 3 / 2; }
    else if (!focused)       { off = off / 2;     soft = soft * 2 / 3; }

    fb_shadow(W->x, W->y, W->w, W->h, off, soft);
    fb_rrect(W->x, W->y, W->w, W->h, t->radius, t->border);
    fb_rrect(W->x + 1, W->y + 1, W->w - 2, W->h - 2, t->radius - 1, t->panel);

    if (W->flags & WF_NOCHROME) return;

    /* THE GRIP HAS TO BE VISIBLE or it is a secret. Three short diagonals in
     * the bottom-right corner - the convention every desktop uses, and cheap
     * enough to draw on every window every repaint. */
    {
        int gx = W->x + W->w - UI_S1(t) - 1, gy = W->y + W->h - UI_S1(t) - 1;
        int step = UI_S1(t) / 2 + 1;
        for (int i = 1; i <= 3; i++) {
            int d = i * step * 2;
            fb_line(gx - d, gy, gx, gy - d, t->border);
        }
    }

    int tx = W->x + 2, tw = W->w - 4, th = t->title_h - 3;
    if (focused) {
        /* rounded at the top, to the SAME radius as the frame one pixel
         * outside it - see fb_rrect_grad_top */
        fb_rrect_grad_top(tx, W->y + 2, tw, th, t->radius - 2,
                          t->title, t->title_bot);
        /* focus is title-bar hue PLUS the accent underline. Both already
         * existed; a third signal would be one too many. */
        fb_fill_px(tx, W->y + t->title_h - 2, tw, 2, t->accent);
    } else {
        fb_rrect_grad_top(tx, W->y + 2, tw, th, t->radius - 2,
                          t->title_off, t->title_off);
    }
    if (wins[win].ntab > 1) {
        /* a tab strip instead of a title. The active one is a raised surface
         * continuous with the client area below it - which is what makes it
         * read as "this tab is the window" rather than "here are some
         * buttons". The inactive ones stay flush with the bar. */
        for (int i = 0; i < wins[win].ntab; i++) {
            int tx, ty, tw, th;
            tab_rect(win, i, &tx, &ty, &tw, &th);
            /* tab_rect works from the settled rect; shift it onto the animated
             * one so a tabbed window still grows correctly */
            tx += W->x - wins[win].x;
            ty += W->y - wins[win].y;
            int on = (i == wins[win].tab);
            if (on) fb_rrect(tx, ty, tw, th + UI_S1(t), UI_S1(t) / 2, t->panel);
            fb_text_prop(tx + UI_S2(t), ty + (th - fb_text_prop_h()) / 2,
                       wins[win].tab_title[i],
                       on ? (focused ? t->text : t->text_dim) : t->text_dim);
        }
    } else {
        fb_text_prop(W->x + UI_S3(t), W->y + (t->title_h - fb_text_prop_h()) / 2,
                     W->title, focused ? t->text : t->text_dim);
    }

    /* THE CLOSE BOX. It used to be a hardcoded red square, always, in every
     * state. One button with three states is the difference between "drawn"
     * and "designed", and it costs one compare: quiet by default, accent on
     * hover, and danger red ONLY while it is actually being pressed - so the
     * one destructive control on a window is not shouting the whole time. */
    int cs = UI_S3(t);
    int bx = W->x + W->w - cs - UI_S2(t), by = W->y + (t->title_h - cs) / 2;
    int over = ptr_x >= bx && ptr_x < bx + cs && ptr_y >= by && ptr_y < by + cs;
    unsigned face = !over ? (focused ? t->text_dim : t->title_off)
                          : ((last_btn & 1) ? t->danger : t->accent);
    fb_rrect(bx, by, cs, cs, UI_S1(t) / 2, face);
}

void wm_repaint(void)
{
    if (!fb_active() || !nwd) return;
    for (int r = 0; r < nwd; r++) {
        int rx0 = wd[r].x0, ry0 = wd[r].y0, rx1 = wd[r].x1, ry1 = wd[r].y1;
        fb_clip(rx0, ry0, rx1 - rx0, ry1 - ry0);

        /* the wallpaper pass: furniture first, always at the bottom, never in
         * the z-order and never overlapped by anything but a window */
        if (hook_desk) hook_desk(rx0, ry0, rx1 - rx0, ry1 - ry0);

        for (int i = 0; i < nz; i++) {          /* BACK TO FRONT = paint order */
            int win = zorder[i];
            struct win *W = &wins[win];
            int cx, cy, cw, ch;
            if (!isect(W->x, W->y, W->x + W->w, W->y + W->h,
                       rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) {
                /* the shadow reaches outside the frame, so a window can still
                 * owe this rectangle something even when the frame misses it */
                int reach = shadow_reach(win);
                if (!isect(W->x - reach, W->y - reach,
                           W->x + W->w + reach, W->y + W->h + reach,
                           rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) continue;
            }
            /* A REAL FADE, and it needs the rectangle taken BEFORE anything
             * is drawn on it. window * a + behind * (1 - a) is not something
             * that can be reconstructed afterwards: once the window is drawn,
             * what was behind it is gone. So stash, draw, blend back.
             *
             * ANIM_PULSE is deliberately not routed through this - a tint is a
             * blend of one colour over what is there and needs no copy at all.
             * A refusal from fb_stash (every slot busy) degrades to drawing
             * the window opaque, which is the right way for an effect to fail. */
            int fade = 255, stash = -1;
            if (wm_anim_running(win) == ANIM_FADE) {
                fade = wm_anim_alpha(win);
                if (fade < 255) stash = fb_stash(cx, cy, cw, ch);
            }

            fb_clip(cx, cy, cw, ch);            /* clip 1: the frame + shadow */
            chrome(win, win == focus_win);

            int fx, fy, fw, fh, ax, ay, aw, ah;
            anim_rect(win, &fx, &fy, &fw, &fh);
            client_of(fx, fy, fw, fh, W->flags, &ax, &ay, &aw, &ah);
            if (hook_draw && isect(ax, ay, ax + aw, ay + ah,
                                   rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) {
                fb_clip(cx, cy, cw, ch);        /* clip 2: NARROWER - client   */
                hook_draw(win_app(win), ax, ay, aw, ah, win == focus_win);
            }

            /* ANIM_PULSE, composited. A tint laid over the finished window at
             * the pulse's alpha - correct with no offscreen buffer, because a
             * tint IS a blend of one colour over what is already there, which
             * is exactly what fb_fill_blend does.
             *
             * ANIM_FADE is a different animal and is NOT drawn here. A real
             * fade needs the window composited against what is BEHIND it at
             * fractional opacity, which needs a copy of the rectangle before
             * the window was drawn on it. wm_anim_alpha() reports it and
             * wmtest asserts it; the compositing waits for the scratch arena
             * in fb.c. Saying so is better than a tint pretending to be a
             * fade - they look different and only one of them is the effect
             * the prototype asks for. */
            if (stash >= 0) {
                fb_clip(cx, cy, cw, ch);
                fb_stash_blend(stash, cx, cy, 255 - fade);
                fb_blur_free(stash);
            }

            if (wm_anim_running(win) == ANIM_PULSE) {
                int pa = wm_anim_alpha(win);
                if (pa > 0 && pa < 255) {
                    fb_clip(cx, cy, cw, ch);
                    fb_rrect_blend(fx, fy, fw, fh, ui_theme()->radius,
                                   ui_theme()->accent, pa);
                }
            }
        }
    }
    fb_clip_none();
    nwd = 0;
}

/* ---- routing --------------------------------------------------------------
 * Three modes, checked in THIS order:
 *   1 POINTER GRAB  a drag or a slider owns ALL pointer events until
 *                   button-up, wherever the pointer goes. Without this a drag
 *                   breaks the instant the pointer outruns the window - which
 *                   today's code only survives because it is a bitmap stamp.
 *   2 MODAL         the start menu takes everything; a click outside dismisses
 *   3 NORMAL        pointer to the topmost window containing the point,
 *                   walking the z-order backwards; keys to the focus window.
 */
static int pgrab = -1;          /* which window owns the pointer, or -1     */
/* WHAT THE GRAB IS FOR. It was a flag - move, or hand the pointer to the app -
 * and resizing is a third answer, not a variant of either. */
#define GRAB_APP    0           /* the app has the pointer until button-up  */
#define GRAB_MOVE   1
#define GRAB_SIZE   2
static int grab_drag;
static int grab_dx, grab_dy;    /* pointer offset inside the frame          */

/* THE RESIZE GRIP. wm_resize() has existed since wm.c was written and NOTHING
 * HAS EVER CALLED IT - the same shape as WF_MODAL before the start menu, and
 * as intel.c's write paths. A window table with no way to resize a window is a
 * desktop where every window is the size somebody typed into wm_open.
 *
 * The grip is the bottom-right corner plus the right and bottom edges, which
 * is where every desktop puts it, and it is checked BEFORE the client-area
 * hand-off and AFTER the close box and tabs - an app that fills its window
 * would otherwise swallow the grab. It is deliberately NOT on the left or top
 * edges: those would need the origin to move as the size changes, which is a
 * second arithmetic to get wrong for a corner nobody reaches for. */
#define RESIZE_EDGE(t)  (UI_S2(t))          /* 8 * scale */

static int in_resize_grip(int win, int x, int y)
{
    const struct ui_theme *t = ui_theme();
    int e = RESIZE_EDGE(t);
    int rx = wins[win].x + wins[win].w, by = wins[win].y + wins[win].h;
    /* inside the window, within `e` of the right OR bottom edge */
    if (x < wins[win].x || y < wins[win].y || x >= rx || y >= by) return 0;
    return (x >= rx - e) || (y >= by - e);
}

/* Where tab `i` sits in the title bar. Drawing and hit-testing BOTH call this,
 * which is the only way to be sure a tab is clickable exactly where it looks -
 * two copies of this arithmetic is how a UI ends up with controls that respond
 * a few pixels off from where they are drawn. */
static void tab_rect(int win, int i, int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    int avail = wins[win].w - 2 * UI_S3(t) - UI_S6(t);   /* leave the close box */
    int tw = avail / wins[win].ntab;
    int max = UI_S6(t) * 5;
    if (tw > max) tw = max;
    *w = tw - UI_S1(t);
    *h = t->title_h - UI_S2(t);
    *x = wins[win].x + UI_S2(t) + i * tw;
    *y = wins[win].y + UI_S1(t);
}

static int in_tab(int win, int x, int y)
{
    if (wins[win].ntab < 2) return -1;
    for (int i = 0; i < wins[win].ntab; i++) {
        int tx, ty, tw, th;
        tab_rect(win, i, &tx, &ty, &tw, &th);
        if (x >= tx && x < tx + tw && y >= ty && y < ty + th) return i;
    }
    return -1;
}

static int in_titlebar(int win, int x, int y)
{
    const struct ui_theme *t = ui_theme();
    if (wins[win].flags & WF_NOCHROME) return 0;
    return y >= wins[win].y && y < wins[win].y + t->title_h &&
           x >= wins[win].x && x < wins[win].x + wins[win].w;
}

static int in_closebox(int win, int x, int y)
{
    const struct ui_theme *t = ui_theme();
    if (wins[win].flags & WF_NOCHROME) return 0;
    int cs = UI_S3(t);
    int bx = wins[win].x + wins[win].w - cs - UI_S2(t);
    int by = wins[win].y + (t->title_h - cs) / 2;
    return x >= bx && x < bx + cs && y >= by && y < by + cs;
}

static void route_mouse(int x, int y, int btn)
{
    int down = (btn & 1) && !(last_btn & 1);
    int up   = !(btn & 1) && (last_btn & 1);
    last_btn = btn;
    ptr_x = x; ptr_y = y;

    /* 1. POINTER GRAB */
    if (pgrab >= 0) {
        if (grab_drag == GRAB_MOVE) {
            wm_move(pgrab, x - grab_dx, y - grab_dy);
        } else if (grab_drag == GRAB_SIZE) {
            /* grab_dx/dy hold the pointer's offset from the corner it took
             * hold of, so the corner stays under the pointer instead of
             * jumping to it on the first motion event */
            wm_resize(pgrab, x - wins[pgrab].x + grab_dx,
                             y - wins[pgrab].y + grab_dy);
        } else if (hook_event) {
            hook_event(win_app(pgrab), pgrab, EV_MOUSE, btn, x, y);
        }
        if (up) pgrab = -1;
        return;
    }

    int m = modal_win();
    int hit = wm_at(x, y);

    /* 2. MODAL */
    if (m >= 0 && hit != m) {
        if (down) wm_close(m);          /* a click outside dismisses it */
        return;
    }

    /* 3. NORMAL */
    if (hit < 0) {
        /* THE FURNITURE - dock, start button, tray - gets every pointer event
         * over it, not just presses. A dock with no hover state reads as a
         * picture of a dock; knowing where the pointer is, is the whole of
         * making it feel like a control. The button mask is passed through so
         * policy can tell a hover from a press without a second callback. */
        if (hook_desk_click) hook_desk_click(x, y, btn);
        return;
    }
    if (down) {
        wm_focus(hit);
        wm_raise(hit);
        if (in_closebox(hit, x, y)) { wm_close(hit); return; }
        /* a tab BEFORE the drag: the strip lives inside the title bar, so
         * checking the drag first would make tabs unclickable */
        int tb = in_tab(hit, x, y);
        if (tb >= 0) { wm_set_tab(hit, tb); return; }
        if (in_titlebar(hit, x, y)) {
            pgrab = hit; grab_drag = GRAB_MOVE;
            grab_dx = x - wins[hit].x;
            grab_dy = y - wins[hit].y;
            return;
        }
        /* the resize grip, before the app gets the pointer - an app that fills
         * its window would otherwise swallow every grab at the edge */
        if (in_resize_grip(hit, x, y)) {
            pgrab = hit; grab_drag = GRAB_SIZE;
            grab_dx = wins[hit].x + wins[hit].w - x;
            grab_dy = wins[hit].y + wins[hit].h - y;
            return;
        }
        /* a press in the client area hands the pointer to the app until
         * button-up - that is what makes a slider work when the pointer
         * leaves the widget mid-drag */
        pgrab = hit; grab_drag = GRAB_APP;
    }
    if (hook_event) hook_event(win_app(hit), hit, EV_MOUSE, btn, x, y);
}

/* Alt+Tab walks the z-order BACKWARDS - the window below the top one is the
 * one you were looking at a moment ago, which is what "the last one" means to
 * a person. */
static void cycle_focus(void)
{
    if (nz < 2) return;
    int cur = z_index_of(focus_win);
    int next = (cur <= 0) ? nz - 1 : cur - 1;
    int win = zorder[next];
    wm_focus(win);
    wm_raise(win);
}

static void route_key(int type, int code, int mods)
{
    if (type == EV_KEY_DOWN && code == '\t' && (mods & MOD_ALT)) {
        cycle_focus();
        return;
    }
    /* Super, TAPPED, belongs to the desktop and not to the focused window -
     * routing it to whichever app has focus would mean every app had to know
     * about the start menu. MOD_SUPER has been tracked since input.c was
     * written and used for nothing at all. */
    if (type == EV_KEY_DOWN && code == KEY_SUPER) {
        if (hook_desk_key) hook_desk_key(code, mods);
        return;
    }
    /* Ctrl+W closes. Closing is the close box or Ctrl+W - NEVER "press any
     * key", which is the phrase this whole rewrite exists to delete. */
    if (type == EV_CHAR && code == 23) {        /* Ctrl+W */
        if (focus_win >= 0) wm_close(focus_win);
        return;
    }
    int m = modal_win();
    int target = (m >= 0) ? m : focus_win;
    if (target < 0) return;
    if (hook_event) hook_event(win_app(target), target, type, code, 0, 0);
}

static void wm_route(int type)
{
    if (type == EV_MOUSE) route_mouse(input_x(), input_y(), input_code());
    else                  route_key(type, input_code(), input_mods());
}

/* ---- the frame loop -------------------------------------------------------
 * This is the top of the system. It must NOT spin at 100% CPU, so it is gated
 * on the 100 Hz tick: at most one pass per tick, and the rest of the time it
 * returns immediately so the caller can idle.
 */
static unsigned int last_tick;

void wm_frame(void)
{
    unsigned int now = idt_ticks();
    if (now == last_tick) return;
    last_tick = now;

    input_poll();
    for (int guard = 0; guard < 64; guard++) {
        int t = input_next();
        if (!t) break;
        wm_route(t);
    }

    /* app_tick runs every frame, is cheap, and MUST NOT DRAW. Returning 1 is
     * how a clock or a snake says "my state changed" without owning the frame
     * - which is the whole reason those demos no longer need a while-loop. */
    /* advance every animation. Damaging the SETTLED rect (which is the
     * largest) is what erases the smaller frame drawn a moment ago. */
    anim_tick();

    if (hook_tick)
        for (int i = 0; i < nz; i++)
            if (hook_tick(win_app(zorder[i]), zorder[i])) wm_damage_win(zorder[i]);

    if (nwd) {
        fb_pointer_hide();      /* the sprite's save-under is stale once the
                                   pixels under it are about to be redrawn */
        wm_repaint();
    }
    fb_pointer_show(ptr_x, ptr_y);
    fb_present();
}
