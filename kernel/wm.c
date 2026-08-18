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
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_shadow(int x, int y, int w, int h, int off, int soft);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
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
    int anim;                  /* frames left in the open animation, 0 = none */
    char title[32];
    /* TABS. Several apps sharing one frame, grouped by task - the idea worth
     * stealing from Essence. It is cheap here because a window already has
     * exactly one thing a tab needs to change: which app_draw gets called. */
    int  tab_app[WM_TABS];
    char tab_title[WM_TABS][16];
    int  ntab;                 /* 1 for an ordinary window */
    int  tab;                  /* which one is showing     */
    /* maximise/restore. The saved rect is only meaningful while maxed. */
    int  maxed;
    int  sav_x, sav_y, sav_w, sav_h;
};

static struct win wins[WM_MAX];
static int zorder[WM_MAX];          /* window indices, BACK to FRONT */
static int nz;                      /* how many are in the z-order   */
static int focus_win = -1;
static int running = 1;

static app_draw_fn  hook_draw;
static app_event_fn hook_event;
static app_tick_fn  hook_tick;
static desk_draw_fn hook_desk;

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

/* ...and a switch, because Settings exposes one. ANIM_FRAMES stays a constant
 * - this is not "how long" but "at all", and a zero-length animation is the
 * honest way to say off: anim_pct and anim_rect keep working unchanged and
 * every window is simply born settled. Making ANIM_FRAMES itself variable
 * would put a run-time value in the `steps` array bound. */
static int anim_on = 1;
void wm_set_anim(int on) { anim_on = on ? 1 : 0; }
int  wm_anim(void)       { return anim_on; }

/* how big window `win` should be DRAWN this frame, as a percentage */
static int anim_pct(int win)
{
    int a = wins[win].anim;
    if (a <= 0) return 100;
    /* 82, 90, 95, 98 -> 100. Decelerating, by subtracting a shrinking
     * fraction rather than by evaluating a curve. */
    static const unsigned char steps[ANIM_FRAMES] = { 82, 90, 95, 98 };
    return steps[ANIM_FRAMES - a];
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
        wins[i].anim = anim_on ? ANIM_FRAMES : 0;
        wins[i].ntab = 1;
        wins[i].tab = 0;
        wins[i].tab_app[0] = app;
        title_copy16(wins[i].tab_title[0], title);
        title_copy(wins[i].title, title);
        z_append(i);
        focus_win = i;
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

    int tx = W->x + 2, tw = W->w - 4, th = t->title_h - 3;
    if (focused) {
        fb_gradient(tx, W->y + 2, tw, th, t->title, t->title_bot);
        /* focus is title-bar hue PLUS the accent underline. Both already
         * existed; a third signal would be one too many. */
        fb_fill_px(tx, W->y + t->title_h - 2, tw, 2, t->accent);
    } else {
        fb_fill_px(tx, W->y + 2, tw, th, t->title_off);
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

    /* THE RESIZE GRIP, drawn. A corner you cannot see is a corner nobody finds,
     * and the pointer shape only helps once you are already on it.
     *
     * Three short diagonal rules stepping in from the corner - the universal
     * mark for it, and cheap: three fills, no new primitive. Dim by default so
     * it does not compete with the close box, which is the only other thing on
     * a window frame that does something. */
    {
        int gs = UI_S3(t);
        int gx = W->x + W->w - gs, gy = W->y + W->h - gs;
        int step = gs / 4;
        unsigned ink = focused ? t->text_dim : t->title_off;
        /* Three rules PARALLEL TO THE CORNER'S DIAGONAL, stepping inward. The
         * first attempt drew them all at the same offset with different
         * lengths, which merges into a single L-bracket - it renders, and it
         * reads as a border artefact rather than as a grip. Only looking at it
         * showed that. */
        if (step > 0)
            for (int i = 1; i <= 3; i++) {
                int d = i * step;
                fb_line(gx + gs - d, gy + gs - 1, gx + gs - 1, gy + gs - d, ink);
            }
    }
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
            /* THE FRAME PLUS ITS SHADOW REACH, always - not the frame with the
             * reach as a fallback.
             *
             * This used to intersect the FRAME first and only expand by the
             * reach when the frame missed the damage rect entirely. Whenever
             * the frame did hit - which is every settled window in every
             * ordinary repaint - the scissor handed to fb_clip was the frame
             * alone, while the comment below claimed it was frame + shadow.
             * chrome() then called fb_shadow, which paints from x+off-soft to
             * x+off+w+soft, so at ui scale 2 the whole visible band lay
             * outside the scissor: every shadow pixel computed and discarded.
             *
             * The result was a total loss of the elevation scheme - modal,
             * focused and unfocused all rendered identically shadowless - and
             * it hid because it just looked like shadows had never been
             * designed. It survived transiently during the open animation,
             * where anim_rect shrinks the drawn frame far enough inside the
             * settled one that the band fits, and was then erased by the
             * wallpaper pass. It also made fb_shadow, the single most
             * expensive call in a window redraw at 4.3 ms of 5.1 ms, into the
             * most expensive wasted work in the compositor.
             *
             * Expanding by reach >= 0 gives a strict superset of the frame, so
             * the two tests collapse into this one. */
            int reach = shadow_reach(win);
            if (!isect(W->x - reach, W->y - reach,
                       W->x + W->w + reach, W->y + W->h + reach,
                       rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) continue;
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
/* ---- double-click ---------------------------------------------------------
 * There was no notion of one anywhere in the kernel.
 *
 * It is decided HERE rather than in input.c because it is a question about
 * PLACE as well as time - two presses 300 ms apart at opposite corners of the
 * screen are not a double-click - and input.c deliberately knows nothing about
 * where windows are. idt_ticks() is 100 Hz, which is ample: the window is 40
 * ticks, and no human double-clicks faster than 10 ms.
 *
 * The slop follows ui() because a "few pixels" on a 2560-wide panel at 2x is
 * not the same distance as on an 800-wide one, and a fixed number makes the
 * gesture harder on exactly the screens where the pointer moves furthest.
 */
#define DBL_TICKS  40           /* 400 ms at 100 Hz */
static int dbl_slop(void) { return UI_S2(ui_theme()); }

static unsigned dbl_when;
static int dbl_x, dbl_y, dbl_win = -1;

static int is_double(int win, int x, int y)
{
    unsigned now = idt_ticks();
    int dx = x - dbl_x, dy = y - dbl_y, s = dbl_slop();
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    /* unsigned subtraction, so a tick counter that wraps cannot report a
     * gigantic elapsed time and silently disable the gesture forever */
    int soon = (now - dbl_when) < (unsigned)DBL_TICKS;
    int near = dx <= s && dy <= s;
    int same = (win == dbl_win);

    dbl_when = now; dbl_x = x; dbl_y = y; dbl_win = win;
    if (soon && near && same) {
        /* consume it: three clicks in a row are a double and then a single,
         * not two overlapping doubles */
        dbl_win = -1;
        return 1;
    }
    return 0;
}

/* ---- snapping -------------------------------------------------------------
 * Maximise, the two halves, and back again. wm_move and wm_resize each damage
 * the old rect and the new one, so none of this needs damage of its own.
 *
 * ONE saved rect serves all of them. `maxed` means "this window is snapped
 * somewhere and sav_* holds where it came from", not specifically "maximised" -
 * so snapping left and then right does NOT overwrite the original geometry with
 * the left half, and one restore always gets you back to where you started.
 * That is the bug every naive version of this has: the saved rect is captured
 * on every snap instead of only on the first. */
#define SNAP_NONE   0
#define SNAP_MAX    1
#define SNAP_LEFT   2
#define SNAP_RIGHT  3

static void wm_snap(int win, int how)
{
    if (!wm_is_open(win)) return;

    if (how == SNAP_NONE) {
        if (!wins[win].maxed) return;          /* nothing to go back to */
        wins[win].maxed = 0;
        wm_move(win, wins[win].sav_x, wins[win].sav_y);
        wm_resize(win, wins[win].sav_w, wins[win].sav_h);
        return;
    }

    if (!wins[win].maxed) {                    /* remember, ONCE */
        wins[win].sav_x = wins[win].x; wins[win].sav_y = wins[win].y;
        wins[win].sav_w = wins[win].w; wins[win].sav_h = wins[win].h;
    }
    wins[win].maxed = how;

    int sw = (int)fb_pxw(), sh = (int)fb_pxh();
    /* Move BEFORE resize when going left, resize before move when going right?
     * No - wm_resize clamps to min_w/min_h and wm_move does not care, so the
     * order cannot produce a rect neither call asked for. Move first, always,
     * for one less thing to reason about. */
    if (how == SNAP_MAX)   { wm_move(win, 0, 0);       wm_resize(win, sw, sh); }
    if (how == SNAP_LEFT)  { wm_move(win, 0, 0);       wm_resize(win, sw / 2, sh); }
    if (how == SNAP_RIGHT) { wm_move(win, sw / 2, 0);  wm_resize(win, sw - sw / 2, sh); }
}

/* the double-click gesture: maximise, or put it back */
static void wm_toggle_max(int win)
{
    wm_snap(win, wins[win].maxed ? SNAP_NONE : SNAP_MAX);
}

static int pgrab = -1;          /* which window owns the pointer, or -1     */
/* What the pointer grab is FOR. It used to be a bare 0/1 meaning "the app has
 * it" or "we are moving it"; resize is a third answer, and three states with
 * two values is how a bug gets in. */
#define GRAB_APP    0           /* the app owns the pointer until button-up  */
#define GRAB_MOVE   1           /* we are dragging the frame                 */
#define GRAB_RESIZE 2           /* we are dragging the bottom-right corner   */
static int grab_drag;
static int grab_dx, grab_dy;    /* pointer offset inside the frame          */

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

/* THE RESIZE GRIP.
 *
 * wm_resize has existed since the window table did - with min_w/min_h clamping
 * and correct damage on both the old and the new rect - and NOTHING HAS EVER
 * CALLED IT. That is this project's own named hazard, "the code exists is not
 * the code works", sitting in the compositor.
 *
 * A corner, not an edge: an edge grip has to decide which edge from a few
 * pixels of hit area, and every one of those decisions is another place for an
 * off-by-one against the frame rect. The bottom-right corner is one rectangle,
 * the same size as the close box, and it grows the window in the direction the
 * pointer is already moving.
 *
 * It sits INSIDE the frame rather than straddling the border, so it cannot
 * overlap the shadow - which is drawn outside the frame and is not part of the
 * window for hit-testing purposes. */
static int in_grip(int win, int x, int y)
{
    const struct ui_theme *t = ui_theme();
    if (wins[win].flags & WF_NOCHROME) return 0;
    int gs = UI_S3(t);
    int gx = wins[win].x + wins[win].w - gs;
    int gy = wins[win].y + wins[win].h - gs;
    return x >= gx && x < gx + gs && y >= gy && y < gy + gs;
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

    /* THE POINTER SHAPE IS THE ONLY AFFORDANCE A GRIP HAS. A resize corner you
     * cannot see and that does not announce itself is a feature nobody finds.
     * Held during a resize drag too, so the shape does not flicker back to an
     * arrow the moment the pointer outruns the corner. */
    {
        int over = (pgrab >= 0 && grab_drag == GRAB_RESIZE);
        if (!over) {
            int top = wm_at(x, y);
            over = (top >= 0 && in_grip(top, x, y));
        }
        fb_cursor_set(over ? CURSOR_RESIZE : CURSOR_ARROW);
    }

    /* 1. POINTER GRAB */
    if (pgrab >= 0) {
        if (grab_drag == GRAB_MOVE) {
            wm_move(pgrab, x - grab_dx, y - grab_dy);
        } else if (grab_drag == GRAB_RESIZE) {
            /* grab_dx/dy hold the offset from the pointer to the corner, so
             * the corner stays under the cursor instead of snapping to it. */
            wm_resize(pgrab, x + grab_dx - wins[pgrab].x,
                             y + grab_dy - wins[pgrab].y);
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
    if (hit < 0) return;
    if (down) {
        wm_focus(hit);
        wm_raise(hit);
        int dbl = is_double(hit, x, y);
        if (in_closebox(hit, x, y)) { wm_close(hit); return; }
        /* DOUBLE-CLICK THE TITLE BAR TO MAXIMISE, again to restore. Checked
         * before the drag, or the second press starts a drag instead - and a
         * maximise that also moves the window by however far the hand drifted
         * between the two clicks is worse than no maximise. */
        if (dbl && in_titlebar(hit, x, y)) { wm_toggle_max(hit); return; }
        /* the grip BEFORE the title bar: they cannot overlap on any sane
         * window, but a window shorter than its own title bar is exactly the
         * degenerate case where checking the bigger region first would swallow
         * the smaller one */
        if (in_grip(hit, x, y)) {
            pgrab = hit; grab_drag = GRAB_RESIZE;
            grab_dx = (wins[hit].x + wins[hit].w) - x;
            grab_dy = (wins[hit].y + wins[hit].h) - y;
            return;
        }
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
        /* a press in the client area hands the pointer to the app until
         * button-up - that is what makes a slider work when the pointer
         * leaves the widget mid-drag */
        pgrab = hit; grab_drag = GRAB_APP;
        /* Hand the app the double-click too, as a bit in the button mask. An
         * app that does not care masks for button 1 and never sees it. */
        if (dbl) btn |= MOUSE_DOUBLE;
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

#define KEY_LEFT   0x110
#define KEY_RIGHT  0x111
#define KEY_UP     0x112
#define KEY_DOWN   0x113

static void route_key(int type, int code, int mods)
{
    if (type == EV_KEY_DOWN && code == '\t' && (mods & MOD_ALT)) {
        cycle_focus();
        return;
    }

    /* SUPER + ARROWS SNAP THE FOCUSED WINDOW. MOD_SUPER has been tracked by
     * input.c since it was written and used for NOTHING - FEEL-PROMPT item 6.5.
     *
     * Snapping is the binding worth spending it on: it is the one window
     * operation that is genuinely painful with a pointer and trivial with a
     * key, and it needs no launcher, no menu and no new policy in kernel.zl -
     * only wm_move and wm_resize, which the grip and the double-click already
     * gave callers. */
    if (type == EV_KEY_DOWN && (mods & MOD_SUPER) && focus_win >= 0) {
        if (code == KEY_LEFT)  { wm_snap(focus_win, SNAP_LEFT);  return; }
        if (code == KEY_RIGHT) { wm_snap(focus_win, SNAP_RIGHT); return; }
        if (code == KEY_UP)    { wm_snap(focus_win, SNAP_MAX);   return; }
        if (code == KEY_DOWN)  { wm_snap(focus_win, SNAP_NONE);  return; }
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
    /* advance any open animation. Damaging the SETTLED rect (which is the
     * largest) is what erases the smaller frame drawn a moment ago. */
    for (int i = 0; i < nz; i++) {
        int win = zorder[i];
        if (wins[win].anim > 0) { wins[win].anim--; wm_damage_win(win); }
    }

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
