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
#include "ease.h"

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
void fb_icon24(int px, int py, int n, unsigned int fg);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
/* Titles are LABELS, not console text, so they take the proportional path.
 * That is the single change desktop-look.md item 4 asks for at this layer. */
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
void fb_present(void);
void fb_pointer_show(int x, int y);
void fb_pointer_hide(void);

/* gpucursor.c - the pointer as a display PLANE instead of a sprite.
 *
 * Gen9 composites a 64x64 ARGB cursor over the primary at scanout for free, so
 * a pointer move costs one register write instead of a save-under, a two-plane
 * composite and a restore inside the frame loop. gpu_cursor_move returns 0
 * whenever that path is not live - which is every build until someone calls
 * gpu_cursor_arm(1) on real hardware - so the software sprite below stays the
 * fallback and nothing here changes until the hardware path is proven. */
int gpu_cursor_move(int x, int y);
int gpu_cursor_is_live(void);
int  fb_cell_w(void);
int  fb_cell_h(void);

/* Optional real-panel pacing. Host harnesses and non-Intel backends leave
 * these weak symbols absent and use the TSC deadline below. */
int intel_supported(void) __attribute__((weak));
int intel_wait_vblank(void) __attribute__((weak));

/* ---- notify.c -------------------------------------------------------------
 * The notification surface, which SYSTEM-PROMPT.md §2 permits adding here and
 * which is the only thing this track has put in this file. Nothing above or
 * below it changed: no routing, no damage rule, no z-order.
 *
 * A toast is not a window. It is not in `wins`, not in `zorder`, and there is
 * no window id for it - which is not an implementation shortcut, it is the
 * feature. A notification that takes focus eats the next keystroke: you are
 * typing, something completes, and the character you were in the middle of
 * goes to something that is about to close itself. There is nothing here that
 * COULD take focus, and that is a stronger guarantee than remembering not to.
 */
int         notify_tick(unsigned now);
int         notify_active(void);
const char *notify_text(void);
int         notify_post(const char *text, unsigned ticks);
void        notify_rect(int sw, int sh, int reserve_bot, int scale,
                        int *x, int *y, int *w, int *h);

/* ---- snap.c ---------------------------------------------------------------
 * `wm_resize` has existed since this file was written and NOTHING HAS EVER
 * CALLED IT. These two triggers are its first callers.
 *
 * All the arithmetic lives in snap.c and is asserted on the host with no
 * compositor at all - the zones, the rectangles that tile an odd width
 * exactly, and the restore rectangle that is captured only on the transition
 * INTO a snapped state. What is here is only "when": a drop, and a key. */
#define SNAP_NONE 0
#define SK_LEFT   1
#define SK_RIGHT  2
#define SK_UP     3
#define SK_DOWN   4
int  snap_zone_for_point(int px, int py, int sw, int sh);
void snap_rect(int z, int sw, int sh, int reserve_top, int reserve_bot,
               int *x, int *y, int *w, int *h);
int  snap_apply(int win, int z, int cx, int cy, int cw, int ch,
                int sw, int sh, int rt, int rb, int *x, int *y, int *w, int *h);
/* the same arithmetic snap_apply uses, WITHOUT committing it to a window.
 * That is the whole point for the drag preview: the outline you see while
 * dragging and the rectangle you get on drop are computed by one function, so
 * the preview cannot promise a landing spot the snap then disagrees with. */
void snap_rect(int z, int sw, int sh, int reserve_top, int reserve_bot,
               int *x, int *y, int *w, int *h);
int  snap_release(int win, int *x, int *y, int *w, int *h);
int  snap_key_zone(int win, int dir);
int  snap_state(int win);
void snap_note_moved(int win);
void snap_note_closed(int win);
void snap_reset(void);
static int isect(int ax0, int ay0, int ax1, int ay1,
                 int bx0, int by0, int bx1, int by1,
                 int *x, int *y, int *w, int *h);

static int snap_preview_zone;
static int snap_preview_x, snap_preview_y, snap_preview_w, snap_preview_h;

static void snap_preview_damage(void)
{
    if (!snap_preview_zone) return;
    const struct ui_theme *t = ui_theme();
    int halo = UI_S1(t);
    wm_damage(snap_preview_x - halo, snap_preview_y - halo,
              snap_preview_w + 2 * halo, snap_preview_h + 2 * halo);
}

static void snap_preview_set(int zone)
{
    if (zone == snap_preview_zone) return;
    snap_preview_damage();
    snap_preview_zone = zone;
    if (zone) {
        const struct ui_theme *t = ui_theme();
        snap_rect(zone, (int)fb_pxw(), (int)fb_pxh(),
                  UI_DP(t, 32), UI_DP(t, 64),
                  &snap_preview_x, &snap_preview_y,
                  &snap_preview_w, &snap_preview_h);
    }
    snap_preview_damage();
}

static void snap_preview_draw(int rx0, int ry0, int rx1, int ry1)
{
    if (!snap_preview_zone) return;
    int x, y, w, h;
    if (!isect(snap_preview_x, snap_preview_y,
               snap_preview_x + snap_preview_w, snap_preview_y + snap_preview_h,
               rx0, ry0, rx1, ry1, &x, &y, &w, &h)) return;
    const struct ui_theme *t = ui_theme();
    fb_clip(x, y, w, h);
    fb_rrect_blend(snap_preview_x + UI_S1(t), snap_preview_y + UI_S1(t),
                   snap_preview_w - 2 * UI_S1(t), snap_preview_h - 2 * UI_S1(t),
                   t->radius, t->accent, 34);
    fb_rrect_blend(snap_preview_x, snap_preview_y,
                   snap_preview_w, snap_preview_h, t->radius,
                   t->accent, 82);
}

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
#define EV_WHEEL     5

#define KEY_SUPER   0x11A
#define MOD_SHIFT   (1 << 0)
#define MOD_ALT     (1 << 2)
#define MOD_SUPER   (1 << 5)

/* The key codes come from keycodes.h, which is the file that owns them, rather
 * than from a copy here.
 *
 * There used to be a copy - twice, in this one file, at what were lines 128-131
 * and 1443-1446 - and both happened to hold the right values. The cost was not a
 * wrong number, it was a MISSING one: whoever wrote the Alt+Tab test had no
 * KEY_TAB in scope because nobody had copied that line in, reached for '\t'
 * instead, and Alt+Tab has never fired. keycodes.h:14-16 states the rule the
 * copy could not enforce - codes live above 0x100 "where it cannot collide with
 * a character", and `code >= KEY_NONCHAR` is the test for "this key has no
 * character". A partial copy of a table cannot carry a rule. */
#include "keycodes.h"

unsigned int idt_ticks(void);
/* cpu.c. The TSC has been readable since cpu.c was written and nothing in the
 * compositor has ever timed a frame - desktop-TODO 0h says to do this BEFORE
 * any performance work, and the v10 run did the performance work first. */
unsigned int cpu_tsc_lo(void);
unsigned int cpu_tsc_khz(void);

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
    /* maximise/restore. The saved rect is only meaningful while maxed. */
    int  maxed;
    int  sav_x, sav_y, sav_w, sav_h;
    /* WHICH WORKSPACE. A window is on exactly one, and a workspace is not a
     * second z-order: the stack is global and unchanged, and `ws` is a filter
     * applied at the three places that ask "can this window be seen" -
     * the paint walk, the hit test, and the focus walks. Doing it that way
     * means switching workspaces cannot reorder anything, so coming back to a
     * workspace shows the stack exactly as it was left. */
    int  ws;
};

static struct win wins[WM_MAX];
static int zorder[WM_MAX];          /* window indices, BACK to FRONT */
static int nz;                      /* how many are in the z-order   */
static int focus_win = -1;
/* THE CURRENT WORKSPACE LIVES HERE and not in kernel.zl, for the same reason
 * the window table does: it is a property OF the window table. kernel.zl held
 * a `ws_cur` that the pips drew from and nothing else read, which is exactly
 * how an indicator ends up telling the truth about a variable and lying about
 * the machine. cur_ws()/set_ws() in kernel.zl now delegate here.
 *
 * HOW MANY there are is still policy and is still kernel.zl's (WS_N = 3). This
 * file only refuses a workspace below 1, because 0 would collide with the
 * value a zeroed window table already has.
 *
 * HOW MANY there are is policy, so it is CONFIGURED rather than hardcoded, and
 * it defaults to 1 - a compositor nobody has told about workspaces has exactly
 * one, and every window is on it. That default is what keeps the host tests
 * (which never call wm_set_ws_n) behaving exactly as they did before this
 * existed. kernel.zl sets it to WS_N right after wm_init. Without a ceiling
 * here, Super+9 would switch to an empty ninth workspace with no pip to get
 * back from. */
static int ws_cur = 1;
static int ws_n   = 1;
/* ZERO UNTIL wm_init() RUNS, and that matters more than it looks. wm_running()
 * is how the rest of the system asks "is the compositor the top of the system
 * right now" - draw_screen() uses it to choose between damaging the screen and
 * redrawing a text desktop, and help() uses it to choose which set of commands
 * to describe. Initialised to 1, it answered yes on a machine with no
 * framebuffer, where the compositor had never been near the screen: the text
 * shell printed the compositor's help and verify.sh caught it. */
static int running = 0;
static unsigned int last_tick, next_frame_tsc;
static int paced;

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
 * DURATION-BASED. The first version advanced through four tables once per
 * compositor call, so changing the frame cadence changed both the speed and
 * the shape. Each motion now has a wall-clock duration and a fixed-point
 * smoothstep. A late frame skips ahead instead of making the animation slow.
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
/* ---- the timeline ---------------------------------------------------------
 * What was here was ONE animation, hardcoded into the window struct as a frame
 * counter, and it could only ever be the open-scale. The prototype names seven
 * keyframes - zov, zpop, zpress, zpulse, zsweep, ztoast, zwin - which is not
 * seven times as much code, it is the same code with the kind as a parameter
 * and the steps in a table.
 *
 * A FIXED ARRAY, sampled once per frame, each entry marking its target damaged.
 * No allocation, no list, no callbacks. An animation that finishes frees its
 * slot; a slot that cannot be found is a refusal, not a silent drop.
 *
 * The interpolation is a bounded fixed-point smoothstep: no float, no
 * overshoot, no frame-count dependency and no allocation.
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

/* THE FURNITURE IDS. Negative, so they cannot collide with a window index, and
 * named, so a reader of a wm_anim_at() call can tell what is animating. wm.c
 * keeps -1 and -2 for the two things it draws itself; everything at or below
 * WM_FX_USER belongs to the policy layer, which is the only code that knows
 * where a dock tile is. ui.h publishes the same three. */
#define WM_FX_TOAST  (-1)      /* ztoast, the notification's entry     */
#define WM_FX_GHOST  (-2)      /* ANIM_CLOSE, the closing window       */
#define WM_FX_USER   (-16)     /* kernel.zl's, -16 and downward        */

/* Durations are wall-clock ticks, not frame counts. The old four-step tables
 * changed speed whenever the compositor cadence changed and visibly stair-
 * stepped on a 60 Hz panel.
 *
 * THE NUMBERS ARE NOW THE REFERENCE'S, not ours. ease.h states each one in
 * milliseconds exactly as docs/design/ds-reference.html declares it, and the
 * conversion to ticks happens here and only here - so a change to the PIT
 * rate can never silently change how the desktop feels. At the measured
 * 100 Hz, one tick is 10 ms.
 *
 * What moved: OPEN was 16 ticks (160 ms) against the reference's 200, and
 * PRESS was 8 (80 ms) against 250 - a press acknowledgement three times too
 * fast to see. CLOSE has no counterpart in the reference, which does not
 * animate closing at all; it keeps zwin's duration so the pair stays
 * symmetric, and that is a choice rather than a measurement. */
#define MS_TO_TICKS(ms) (((ms) + 5) / 10)

static const unsigned char anim_ticks[] = {
    /* NONE  */ 0,
    /* OPEN  */ MS_TO_TICKS(EASE_MS_WIN),     /* zwin   200 ms */
    /* CLOSE */ MS_TO_TICKS(EASE_MS_WIN),     /* no reference counterpart */
    /* PRESS */ MS_TO_TICKS(EASE_MS_PRESS),   /* zpress 250 ms */
    /* PULSE */ MS_TO_TICKS(EASE_MS_PULSE),   /* zpulse 1000 ms */
    /* FADE  */ MS_TO_TICKS(EASE_MS_OV),      /* zov/ztoast 160 ms */
};

/* Which curve each animation runs on. ds-reference.html lines 14-20: only the
 * window open gets the bespoke cubic-bezier; the pops and fades are ease-out
 * and the pulse is ease-in-out. Using one curve for all five - which is what
 * this file did - is what made every animation feel like the same animation. */
static const unsigned char anim_curve[] = {
    /* NONE  */ EASE_LINEAR,
    /* OPEN  */ EASE_WIN,
    /* CLOSE */ EASE_WIN,
    /* PRESS */ EASE_STD,
    /* PULSE */ EASE_IN_OUT,
    /* FADE  */ EASE_OUT,
};

/* THE ID IS NOT ALWAYS A WINDOW.
 *
 * Six of the reference's seven animations belong to things that are not in
 * `wins` at all - a dock tile, the dot under it, a toast, the wallpaper. The
 * timeline needed exactly one change to reach them: `win` is a KEY, and the
 * only place that assumed it indexes `wins` is the damage call. So an entry
 * carries its own damage rectangle, and an id outside 0..WM_MAX-1 is furniture
 * whose rectangle the caller supplies.
 *
 * dw == 0 means "this is a window, damage it the window way". That is not a
 * sentinel invented for the purpose: a zero-width damage rectangle is
 * meaningless, so no caller can want one. */
struct anim { int win; int kind; unsigned start; unsigned duration;
              int dx, dy, dw, dh; };
static struct anim anims[ANIM_MAX];

static void anim_damage(const struct anim *a)
{
    if (a->dw > 0 && a->dh > 0) wm_damage(a->dx, a->dy, a->dw, a->dh);
    else                        wm_damage_win(a->win);
}

static int anim_start(int id, int kind, int x, int y, int w, int h)
{
    if (kind <= ANIM_NONE || kind >= (int)(sizeof anim_ticks / sizeof anim_ticks[0]))
        return 0;
    /* One animation per id per kind. Re-triggering restarts it, which is
     * what a button pressed twice in quick succession should look like. */
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind && anims[i].win == id && anims[i].kind == kind) {
            anims[i].start = idt_ticks();
            anims[i].dx = x; anims[i].dy = y; anims[i].dw = w; anims[i].dh = h;
            anim_damage(&anims[i]);
            return 1;
        }
    for (int i = 0; i < ANIM_MAX; i++) {
        if (anims[i].kind) continue;
        anims[i].win = id;
        anims[i].kind = kind;
        anims[i].start = idt_ticks();
        anims[i].duration = anim_ticks[kind];
        anims[i].dx = x; anims[i].dy = y; anims[i].dw = w; anims[i].dh = h;
        anim_damage(&anims[i]);
        return 1;
    }
    wm_puts("  wm: no free animation slot, refusing\n");
    return 0;
}

/* Forget everything running on an id, without drawing a last frame.
 *
 * wm_open reuses the FIRST FREE SLOT, so a window index is recycled the moment
 * its predecessor closes - and a close animation still running on that index
 * would then be read as the new window's. That is the same recycled-index bug
 * wm_close's pointer-grab comment describes, one subsystem over. */
static void anim_cancel(int id)
{
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind && anims[i].win == id) anims[i].kind = ANIM_NONE;
}

static void anim_cancel_kind(int id, int kind)
{
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind == kind && anims[i].win == id) anims[i].kind = ANIM_NONE;
}

/* Start one. Returns 0 and says so if every slot is busy - the same refusal
 * discipline as wm_open's WM_MAX, and for the same reason: a silently dropped
 * animation is a UI that is intermittently unresponsive for no visible cause. */
int wm_anim(int win, int kind) { return anim_start(win, kind, 0, 0, 0, 0); }

/* ...and the furniture form: same timeline, same curves, same durations, but
 * the caller says what to repaint because wm.c does not know where a dock tile
 * is. `id` must be outside 0..WM_MAX-1 or it will collide with a window. */
int wm_anim_at(int id, int kind, int x, int y, int w, int h)
{
    if (id >= 0 && id < WM_MAX) return 0;      /* that is a window's id */
    if (w <= 0 || h <= 0) return 0;            /* nothing to repaint = nothing */
    return anim_start(id, kind, x, y, w, h);
}

/* Progress in thousandths, eased by whichever curve this animation runs on.
 *
 * This used to apply smoothstep to everything. Smoothstep is symmetric: it
 * eases IN as well as out, so a window opening spent its first quarter barely
 * moving. The reference's zwin is 74% of the way there at the same point -
 * hosttest/easetest.c prints both numbers side by side. That single difference
 * is most of why the two desktops feel unalike in motion. */
static int anim_progress(int win, int kind)
{
    for (int i = 0; i < ANIM_MAX; i++)
        if (anims[i].kind == kind && anims[i].win == win) {
            unsigned elapsed = idt_ticks() - anims[i].start;
            unsigned d = anims[i].duration ? anims[i].duration : 1u;
            if (elapsed >= d) return 1000;
            int p = (int)(elapsed * 1000u / d);
            return ease_apply(anim_curve[kind], p);
        }
    return -1;
}

/* IS THIS PARTICULAR KIND RUNNING? Not "what is running", which is a different
 * question and the wrong one for the repaint to ask.
 *
 * wm_anim_running() below returns the kind in the LOWEST-NUMBERED slot, so a
 * window carrying two animations at once answers with whichever of them
 * happened to be started into an earlier slot. The repaint asked it
 * `== ANIM_FADE` to decide whether to composite, which means a window that was
 * still opening when something faded it was drawn OPAQUE - the fade ran, the
 * alpha was correct, the timeline was correct, and no pixel was blended.
 *
 * It hid because slot order usually agreed with intent, and it surfaced the
 * moment wm_close began freeing its window's slots: an open animation that had
 * previously been REFUSED for want of a slot now succeeded, landed in slot 0,
 * and silently outvoted the fade in slot 2. wmtest's "a fading window is not
 * fully drawn" is the assertion that caught it. */
static int anim_is(int id, int kind) { return anim_progress(id, kind) >= 0; }

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
    int p = anim_progress(win, ANIM_FADE);
    if (p >= 0) return 48 + 207 * p / 1000;
    p = anim_progress(win, ANIM_PULSE);
    if (p >= 0) {
        int tri = p <= 500 ? p * 2 : (1000 - p) * 2;
        return 48 * tri / 1000;
    }
    return 255;
}

/* Sample every running animation and damage what moved.
 * Damaging the SETTLED rect - which is the largest - is what erases the
 * smaller frame drawn a moment ago.
 *
 * THIS USED TO ADVANCE BY ONE INDEX PER CALL, which made every duration in this
 * file a count of compositor passes rather than a length of time. The comment
 * on ANIM_FRAMES says "four frames at 100 Hz is 40 ms", and that was only true
 * if a pass happened to cost exactly 10 ms - so animation speed tracked host
 * load, scene complexity and resolution. Making the redraw faster made the
 * animations faster instead of smoother, which is the opposite of the point.
 *
 * idt_ticks() is 100 Hz, so one tick IS one intended frame and the conversion
 * is a subtraction. Two other subsystems in this file already reason about that
 * (see the notes at the drag threshold and the double-click window); the
 * timeline was the one that did not.
 *
 * anim_tick() is called every compositor pass whether or not anything is
 * animating, so anim_last stays current and an animation that starts after a
 * long idle does not jump straight to its end. The clamp is belt and braces for
 * the case where it does not - a stall long enough to skip a whole timeline
 * should end the animation, not wrap its index. */
static unsigned anim_last = 0;

/* 4 frames at 100 Hz = 40 ms. Restored from main: the branch's side of the
 * animation hunk won, but main's anim_tick came through unconflicted and
 * still reads this. */
#define ANIM_FRAMES 4

static void anim_tick(void)
{
    unsigned now  = idt_ticks();
    unsigned step = now - anim_last;          /* 100 Hz: one tick, one frame */
    anim_last = now;
    if (step == 0) return;                    /* no time passed: nothing moved */
    if (step > (unsigned)ANIM_FRAMES) step = (unsigned)ANIM_FRAMES;

    for (int i = 0; i < ANIM_MAX; i++) {
        if (!anims[i].kind) continue;
        anim_damage(&anims[i]);
        /* AN ANIMATION NEVER CHANGES WINDOW LIFETIME. It was tempting to have
         * ANIM_CLOSE call wm_close() when it finishes, so a closing window
         * shrinks away; that would make "the window closed" depend on a free
         * animation slot, and wm_anim() is allowed to refuse. A window that
         * sometimes does not close when every slot is busy is a far worse bug
         * than a window that closes without a flourish. The timeline draws;
         * the caller decides what exists. */
        if (idt_ticks() - anims[i].start >= anims[i].duration)
            anims[i].kind = ANIM_NONE;
    }
}

/* ...and a switch, because Settings exposes one. ANIM_FRAMES stays a constant
 * - this is not "how long" but "at all", and a zero-length animation is the
 * honest way to say off: anim_permille and anim_rect keep working unchanged and
 * every window is simply born settled. Making ANIM_FRAMES itself variable
 * would put a run-time value in the `steps` array bound. */
static int anim_on = 1;
void wm_set_anim(int on) { anim_on = on ? 1 : 0; }
int  wm_anim_enabled(void) { return anim_on; }

/* ---- what the policy layer reads ------------------------------------------
 * kernel.zl draws the dock, and the dock is where four of the reference's
 * seven animations live. It cannot call anim_progress (static, and rightly so)
 * so these are the two numbers it needs: how big to draw a thing, and how
 * opaque. Both are pure reads - nothing here starts, stops or damages. */
int wm_anim_progress(int id, int kind) { return anim_progress(id, kind); }

/* ---- the two INFINITE animations ------------------------------------------
 * zpulse (1s / 2.6s) and zsweep (7s) are `infinite` in the reference, and an
 * infinite entry in a fixed array of eight NEVER FREES ITS SLOT - two of them
 * and a quarter of the timeline is gone for the life of the boot, which shows
 * up later as "the UI stopped animating after a while".
 *
 * They do not need a slot. An animation with no beginning and no end is a pure
 * function of the clock, so it is computed on demand and stores nothing at
 * all. That is also why it cannot be refused, and why turning animations off
 * is the only thing that stops it.
 *
 * Returns an opacity 0..255. The floor is the reference's own .55, so this
 * never returns less than 140 - a pulse that reached zero would be a blink. */
int wm_pulse(int period_ms)
{
    if (!anim_on) return 255;
    unsigned d = (unsigned)MS_TO_TICKS(period_ms);
    if (!d) d = 1;
    int p = (int)((idt_ticks() % d) * 1000u / d);
    return 255 * ease_pulse(p) / 1000;
}

/* How big window `win` should be DRAWN this frame, in THOUSANDTHS.
 *
 * It was percent, and percent cannot express the reference's open scale: zwin
 * starts at scale(.965), which is 96.5% and rounds to either 96 or 97. At the
 * sizes windows actually are that is a 3-pixel difference in where the first
 * frame lands, and it is visible as a jump. Thousandths throughout, matching
 * ease.h, so no unit conversion happens at a call site.
 *
 * ONE MECHANISM. The open scale used to be a counter in the window struct and
 * the timeline was a second thing beside it that nothing triggered - so wm.c
 * carried two animation systems, one of which never ran. wm_open() starts an
 * ANIM_OPEN now and this reads it, which means the open scale and every other
 * kind share a code path and a bug in one is a bug you can actually see.
 */
static int anim_permille(int win)
{
    /* THE OPEN SCALE WAS 82%. The reference's is 96.5% - ds-reference.html
     * line 15, `scale(.965)`. 82% is a window that leaps at you from a sixth
     * of its size; 96.5% is a window that is essentially already there and
     * settles. Same duration, same curve, completely different gesture. */
    int p = anim_progress(win, ANIM_OPEN);
    if (p >= 0)
        return EASE_WIN_FROM_SCALE + (1000 - EASE_WIN_FROM_SCALE) * p / 1000;
    p = anim_progress(win, ANIM_CLOSE);
    if (p >= 0)
        return 1000 - (1000 - EASE_WIN_FROM_SCALE) * p / 1000;
    p = anim_progress(win, ANIM_PRESS);
    if (p >= 0) return ease_press_scale(p);
    return 1000;
}

/* The same number, for a caller that is not a window.
 *
 * zpress belongs to CONTROLS - a dock tile, a button - and not one of them is
 * in `wins`. Exporting the scale rather than a second press implementation is
 * what keeps kernel.zl's dock on the reference's curve and duration without
 * kernel.zl knowing what a cubic-bezier is. */
int wm_anim_scale(int id) { return anim_permille(id); }

/* ---- the closing window's GHOST -------------------------------------------
 * ANIM_CLOSE has existed since the timeline was written, anim_permille has
 * always known how to shrink for it, and NOTHING EVER STARTED ONE. The reason
 * is in anim_tick above: a closing window cannot be drawn by the repaint's
 * z-order walk, because by the time there is anything to draw it is no longer
 * in the z-order - and keeping it there until the animation finished would
 * make "the window closed" depend on a free animation slot, which that comment
 * refuses to allow and is right to refuse.
 *
 * So the window closes IMMEDIATELY, exactly as before, and what shrinks is a
 * GHOST: a rectangle and a colour. It is not in `wins`, not in the z-order,
 * not hit-testable, has no app and receives no events. Nothing can ask it a
 * question. If the timeline refuses the animation the ghost is simply never
 * armed and the window vanishes without a flourish, which is the right way for
 * decoration to fail.
 *
 * ONE ghost, not WM_MAX of them. Closing two windows inside 200 ms and seeing
 * only the second shrink is not a defect anybody can perceive, and an array
 * here would be more state with a lifetime - the thing this file is trying to
 * have less of. */
static struct { int live, x, y, w, h, reach; } ghost;

static void ghost_clear(void)
{
    if (!ghost.live) return;
    ghost.live = 0;
    wm_damage(ghost.x - ghost.reach, ghost.y - ghost.reach,
              ghost.w + 2 * ghost.reach, ghost.h + 2 * ghost.reach);
    anim_cancel(WM_FX_GHOST);
}

/* ---- zsweep ---------------------------------------------------------------
 * ds-reference.html:66 - a band 34% of the screen tall, filled with
 * `linear-gradient(180deg,transparent,rgba(184,232,56,.045),transparent)`,
 * running `zsweep 7s linear infinite`, which is
 * `translateY(-100%)` -> `translateY(100%)`.
 *
 * TWO THINGS ABOUT IT ARE NOT OBVIOUS AND BOTH ARE FAITHFUL TO THE SOURCE:
 *
 *  - A CSS translateY percentage is a percentage of the ELEMENT, not the
 *    parent. The band is 34% tall, so it travels from -34% to +34% of the
 *    screen - it never reaches the bottom third. That reads like a mistake in
 *    the reference and it may be one, but it is what the file says, and
 *    inventing a full-height sweep here would be inventing a different
 *    animation and calling it this one.
 *  - The colour is rgba(184,232,56), which is ZD_ACCENT. It is taken from
 *    ui_theme()->accent rather than written out, so it follows the accent the
 *    user picked in Settings instead of pinning one of the five.
 *
 * IT IS QUANTISED, and that is the one number here that is ours. The band is
 * a third of the screen and every row of it changes colour when it moves, so
 * an un-quantised sweep damages a third of the panel on EVERY frame - 1.25 M
 * pixels at 2560x1440, plus every window that overlaps it, sixty times a
 * second, for an effect whose peak contribution is 11/255 of one colour. At
 * 7 s of travel the band moves well under a pixel per tick, so snapping its
 * top to a step and repainting only when the step changes costs nothing
 * visible and turns a per-frame full-width repaint into an occasional one. */
#define SWEEP_H_PCT   34       /* `height:34%`                              */
#define SWEEP_A       11       /* .045 * 255 = 11.5, and 11 is the darker    */
#define SWEEP_BANDS   12       /* the gradient, in steps. At a peak alpha of
                                * 11 there are only 11 distinguishable ones. */
#define SWEEP_STEP     6       /* design px the band snaps to; see above     */

/* OFF UNTIL SOMEBODY ASKS, and that is not a hedge - it is what the reference
 * does. The band lives inside `<sc-if value="{{ crtOn }}">`, i.e. it is part of
 * an optional CRT overlay rather than part of the desktop. kernel.zl turns it
 * on when the desktop boots, which is where "is this desktop's wallpaper
 * alive" is a policy question and belongs.
 *
 * It also keeps every host gate that asserts on exact wallpaper pixels honest
 * by default: a tint that arrives without being asked for would turn
 * all_wallpaper() in wmtest and wmtest_feel from a check into a nuisance, and
 * a gate people learn to work around is worse than no gate. */
static int sweep_on;
static int sweep_last_top;
void wm_set_sweep(int on) { sweep_on = on ? 1 : 0; }
int  wm_sweep_enabled(void) { return sweep_on && anim_on; }
static int sweep_top(void);
/* Published because the quantised position IS the cost: the band only forces
 * a repaint on the ticks where this number changes, and "how often is that"
 * is the only performance question the sweep raises. A caller that wants to
 * know can count, rather than be told a figure in a comment. */
int wm_sweep_y(void) { return wm_sweep_enabled() ? sweep_top() : 0; }

static int sweep_band_h(void)
{
    return (int)fb_pxh() * SWEEP_H_PCT / 100;
}

/* Where the band's top edge is this instant, quantised. May be negative - the
 * band starts entirely above the screen, which is what translateY(-100%) is. */
static int sweep_top(void)
{
    int bh = sweep_band_h();
    unsigned d = (unsigned)MS_TO_TICKS(EASE_MS_SWEEP);
    if (!d) d = 1;
    /* zsweep is `linear`, so there is no curve to apply - and running it
     * through ease_apply(EASE_LINEAR) would say the same thing more slowly. */
    int p = (int)((idt_ticks() % d) * 1000u / d);
    int t = -bh + 2 * bh * p / 1000;
    int step = SWEEP_STEP * ui_theme()->scale;
    if (step < 1) step = 1;
    /* Round toward negative infinity, not toward zero: C division truncates,
     * which would make the quantised position stall for two steps as it
     * crosses y = 0 and jerk visibly at exactly the middle of the sweep. */
    return t >= 0 ? (t / step) * step : -(((-t) + step - 1) / step) * step;
}

static void anim_rect(int win, int *x, int *y, int *w, int *h)
{
    int p = anim_permille(win);
    struct win *W = &wins[win];
    *w = W->w * p / 1000;
    *h = W->h * p / 1000;
    /* grow from the CENTRE - a window that grows from its top-left corner
     * reads as sliding, which says something different */
    *x = W->x + (W->w - *w) / 2;
    *y = W->y + (W->h - *h) / 2;

    /* zwin is `scale(.965) translateY(10px)`: the window does not only grow,
     * it RISES the last 10 px into place. Dropping the translate leaves a
     * scale-only pop, which is the animation this file already had. The offset
     * is scaled with the UI so it is 10 design pixels, not 10 device ones. */
    int op = anim_progress(win, ANIM_OPEN);
    if (op >= 0) {
        int dy = EASE_WIN_FROM_DY * ui_metric(UI_METRIC_SCALE_Q8) / 256;
        *y += dy - dy * op / 1000;
    }
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

int wm_is_minimized(int win)
{
    return wm_is_open(win) && (wins[win].flags & WF_MINIMIZED);
}

/* ---- workspaces -----------------------------------------------------------
 * ONE predicate, used by everything that has to decide whether a window can be
 * seen or touched. It was tempting to write `ws != ws_cur` inline at each of
 * the five sites; the reason not to is that the five would then be five places
 * to forget, and a window that paints but cannot be clicked (or the reverse)
 * is a far worse bug than one that is simply hidden. */
static int on_ws(int win) { return wins[win].ws == ws_cur; }

/* VISIBLE = open, not minimised, and on the workspace you are looking at. */
static int win_visible(int win)
{
    return !(wins[win].flags & WF_MINIMIZED) && on_ws(win);
}

int wm_ws(void)            { return ws_cur; }
int wm_ws_count(void)      { return ws_n; }
int wm_win_ws(int win)     { return wm_is_open(win) ? wins[win].ws : 0; }

/* How many workspaces there are. Told, not assumed - see ws_n's declaration.
 * Refuses below 1, and refuses to shrink below where anything currently is,
 * because a window stranded on workspace 4 after the count drops to 3 is open,
 * focusable by nothing, and drawn nowhere. */
int wm_set_ws_n(int n)
{
    if (n < 1) return 0;
    for (int i = 0; i < WM_MAX; i++)
        if ((wins[i].flags & WF_OPEN) && wins[i].ws > n) return 0;
    if (ws_cur > n) return 0;
    ws_n = n;
    return 1;
}

static int top_visible(void)
{
    for (int i = nz - 1; i >= 0; i--)
        if (win_visible(zorder[i])) return zorder[i];
    return -1;
}

/* Declared here rather than only beside wm_close: moving a window off the
 * workspace you are on has to drop its pointer grab for exactly the reason
 * closing one does - a drag in progress would keep steering a window nobody
 * can see. */
static void wm_drop_grab(int win);

/* Move ONE window to another workspace. If it was the focused one it stops
 * being visible, so focus has to go somewhere that still is - otherwise every
 * subsequent keystroke goes to a window on a workspace nobody is looking at,
 * which looks exactly like a dead keyboard. */
int wm_set_win_ws(int win, int n)
{
    if (n < 1 || n > ws_n || !wm_is_open(win) || wins[win].ws == n) return 0;
    wm_damage_win(win);                      /* it vanishes from here... */
    wins[win].ws = n;
    wm_damage_win(win);                      /* ...or appears, if n == ws_cur */
    if (!win_visible(win)) {
        wm_drop_grab(win);
        if (focus_win == win) focus_win = top_visible();
    }
    return 1;
}

/* Switch. EVERY pixel is damaged, because every window on the old workspace
 * has to go and every window on the new one has to arrive - there is no
 * smaller correct rectangle when the whole set of windows changes. */
int wm_set_ws(int n)
{
    if (n < 1 || n > ws_n || n == ws_cur) return 0;
    ws_cur = n;
    focus_win = top_visible();
    if (fb_active()) wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
    return 1;
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
    for (int i = 0; i < WM_MAX; i++) { wins[i].flags = 0; wins[i].ws = 1; }
    nz = 0;
    nwd = 0;
    focus_win = -1;
    ws_cur = 1;
    /* THE TIMELINE IS STATE AND wm_init MEANS "none of it happened". A ghost
     * or a running press left over from a previous session would be drawn over
     * a window table that no longer contains what it is a picture of. */
    for (int i = 0; i < ANIM_MAX; i++) anims[i].kind = ANIM_NONE;
    ghost.live = 0;
    sweep_last_top = 0;
    snap_reset();
    snap_preview_zone = 0;
    last_tick = next_frame_tsc = 0;
    paced = 0;
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
        /* A NEW WINDOW LANDS ON THE WORKSPACE YOU ARE LOOKING AT. That is the
         * reference's rule too - ds.html's openApp() writes `winWs[id] = s.ws`
         * every time, and the per-app `ws:` field in its APPS table is only
         * the value each app STARTS with before it has ever been opened. A
         * window that opened onto a workspace you are not on would look
         * exactly like an app that failed to launch. */
        wins[i].ws = ws_cur;
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
        if (anim_on) wm_anim(i, ANIM_OPEN);   /* Settings can turn this off */
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

/* Declared here because wm_close has to drop the grab and the grab state is
 * declared with the routing, further down. */
static void wm_drop_grab(int win);

/* Close it, and shrink a ghost of it away. THE GESTURE form of wm_close.
 *
 * The split is deliberate. wm_close() is called by teardown loops, by policy
 * reshuffling windows, and by wm_init-adjacent code that wants the table empty
 * - none of which is a moment anybody is watching, and all of which would look
 * wrong animated. The ✕ box, Ctrl+W and dismissing a modal are the three
 * places a HUMAN closed something, and those are the three callers of this. */
void wm_close_fx(int win)
{
    if (!wm_is_open(win)) return;
    int visible = win_visible(win);
    int gx = wins[win].x, gy = wins[win].y;
    int gw = wins[win].w, gh = wins[win].h;
    int reach = shadow_reach(win);
    wm_close(win);
    if (!anim_on || !visible) return;
    ghost_clear();                       /* at most one; the newer wins */
    /* The rectangle is the SETTLED one plus its shadow reach: the ghost only
     * ever shrinks inside it, so this is the largest thing that has to be
     * erased on the frame the animation ends. */
    if (!wm_anim_at(WM_FX_GHOST, ANIM_CLOSE, gx - reach, gy - reach,
                    gw + 2 * reach, gh + 2 * reach)) return;
    ghost.live = 1;  ghost.reach = reach;
    ghost.x = gx;    ghost.y = gy;    ghost.w = gw;  ghost.h = gh;
}

void wm_close(int win)
{
    if (!wm_is_open(win)) return;
    wm_damage_win(win);
    /* A slot is about to become reusable, and wm_open takes the FIRST FREE ONE
     * - so an animation still running on this index would be inherited by
     * whatever opens next and read as its open-scale. */
    anim_cancel(win);
    wins[win].flags = 0;
    z_remove(win);
    /* A closed window must not leave its snap state behind for whatever opens
     * into the same slot next, or the new window un-snaps to a rectangle that
     * belonged to something else entirely. */
    snap_note_closed(win);
    /* focus the new top, so closing never leaves keys going nowhere */
    focus_win = top_visible();
    /* ...and the POINTER, for the same reason. A press hands the window the
     * pointer until button-up, and a window can close mid-press - Ctrl+W is a
     * key event and arrives between the down and the up. Left alone, the app
     * kept receiving mouse events for a window that no longer existed.
     *
     * The second half is worse: wm_open reuses the FIRST FREE SLOT, so a
     * window opened before button-up lands in the dead window's index and
     * silently inherits the drag - a brand new window that starts moving
     * because of a press the user aimed at something else. */
    wm_drop_grab(win);
}

void wm_raise(int win)
{
    if (!wm_is_open(win)) return;
    if (wins[win].flags & WF_MINIMIZED) {
        wins[win].flags &= ~WF_MINIMIZED;
        wm_damage_win(win);
    }
    /* RAISE MEANS "PUT IT WHERE I CAN SEE IT", and on another workspace that
     * has to include bringing it here. Every caller is someone asking for the
     * window - a dock tile, a taskbar chip, reg_open() finding the app already
     * running - and the alternative is a click that produces nothing at all,
     * because the window really did come to the front of a stack nobody is
     * looking at. It is also what the reference does: ds.html's openApp()
     * writes `winWs[id] = s.ws` whether the window was already open or not. */
    if (wins[win].ws != ws_cur) {
        wins[win].ws = ws_cur;
        wm_damage_win(win);
    }
    if (nz && zorder[nz - 1] == win) return;       /* already on top */
    z_append(win);
    wm_damage_win(win);
}

/* Focus does NOT imply raise. A menu can take the keys without reordering the
 * stack underneath it, and a click-to-focus that also raised would make that
 * impossible to express. */
void wm_focus(int win)
{
    if (wm_is_minimized(win)) {
        wins[win].flags &= ~WF_MINIMIZED;
        wm_damage_win(win);
    }
    /* ...and off another workspace, for the same reason it un-minimises. Focus
     * means "the keyboard goes here", and the keyboard cannot go to something
     * that is not on screen: the frame loop would keep routing every key to a
     * window nobody can see, which is indistinguishable from a dead keyboard.
     *
     * This is NOT the same thing as raising - the z-order is untouched, and
     * "focus does not imply raise" (wmtest asserts it) still holds. It is a
     * different axis: which workspace, not which depth. wm_raise does it too,
     * and both need it because the two are called separately - reg_open() in
     * apps_registry.zl calls focus THEN raise, and between those two lines the
     * invariant would otherwise be broken. */
    if (wm_is_open(win) && wins[win].ws != ws_cur) {
        wins[win].ws = ws_cur;
        wm_damage_win(win);
    }
    if (focus_win == win) return;
    int old = focus_win;
    focus_win = win;
    /* both title bars change: the old loses its hue and underline, the new
     * gains them. Two damages, not one. */
    if (wm_is_open(old)) wm_damage_win(old);
    if (wm_is_open(win)) wm_damage_win(win);
}

void wm_minimize(int win)
{
    if (!wm_is_open(win) || (wins[win].flags & (WF_MODAL | WF_NOCHROME))) return;
    wm_damage_win(win);
    wins[win].flags |= WF_MINIMIZED;
    if (focus_win == win) focus_win = top_visible();
    wm_drop_grab(win);
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
    int was = (wins[win].flags & WF_MODAL) != 0;
    wm_damage_win(win);
    if (on) wins[win].flags |= WF_MODAL;
    else    wins[win].flags &= ~WF_MODAL;
    wm_damage_win(win);

    /* zov / zpop - THE ONE PLACE, rather than at every popover's call site.
     *
     * ANIM_FADE had exactly one caller in the whole tree, kernel.zl's
     * open_menu(), so the start menu faded and every other overlay appeared
     * instantly. "Becoming modal" is what a popover, a dialog and a context
     * menu all have in common and it is the only thing they have in common,
     * so it is the right hook: a modal added later is animated by having been
     * written, not by somebody remembering.
     *
     * Only on the EDGE. wm_set_modal(win, 1) on a window that is already modal
     * is a no-op everywhere else in this function and has to be one here too,
     * or a caller that re-asserts modality every frame restarts the fade every
     * frame and the overlay never finishes appearing.
     *
     * ANIM_OPEN is cancelled first. wm_open started one a moment ago and a
     * modal is not a window that grows - the reference gives it zov, which is
     * a fade from scale(1.03), the opposite direction. Two scale animations on
     * one id would also both be read by anim_permille, and the first match
     * wins, so the fade would be drawn at the open animation's size. */
    if (on && !was) {
        anim_cancel_kind(win, ANIM_OPEN);
        if (anim_on) wm_anim(win, ANIM_FADE);
    }
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
        if (win_visible(zorder[i]) &&
            win_contains(zorder[i], x, y)) return zorder[i];
    return -1;
}

static int modal_win(void)
{
    for (int i = nz - 1; i >= 0; i--)
        if (win_visible(zorder[i]) &&
            (wins[zorder[i]].flags & WF_MODAL)) return zorder[i];
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

enum title_control { TITLE_CLOSE = 0, TITLE_MAXIMIZE = 1, TITLE_MINIMIZE = 2 };

static void title_control_rect(const struct win *W, int which,
                               int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    int cs = UI_DP(t, 26), gap = UI_DP(t, 6);
    *w = *h = cs;
    *x = W->x + W->w - UI_S2(t) - cs - which * (cs + gap);
    *y = W->y + (t->title_h - cs) / 2;
}

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
    if (focused)
        fb_rrect_blend(W->x, W->y, W->w, W->h, t->radius, t->accent, 44);
    fb_rrect(W->x + 1, W->y + 1, W->w - 2, W->h - 2, t->radius - 1, t->panel);

    if (W->flags & WF_NOCHROME) return;

    int tx = W->x + 2, tw = W->w - 4, th = t->title_h - 3;
    if (focused) {
        /* rounded at the top, to the SAME radius as the frame one pixel
         * outside it - see fb_rrect_grad_top */
        fb_rrect_grad_top(tx, W->y + 2, tw, th, t->radius - 2,
                          t->title, t->title_bot);
    } else {
        /* a GRADIENT, same as the focused bar and same as kernel.zl:794 - it
         * was two copies of one colour because the theme struct had one field
         * for it. The reference's own stops: #2a3550 -> #182238. */
        fb_rrect_grad_top(tx, W->y + 2, tw, th, t->radius - 2,
                          t->title_off, t->title_off_bot);
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
        int title_w = fb_text_prop_w(W->title);
        int title_x = W->x + (W->w - title_w) / 2;
        int safe_l = W->x + UI_S6(t);
        int safe_r = W->x + W->w - UI_DP(t, 112);
        if (title_x < safe_l) title_x = safe_l;
        if (title_x + title_w > safe_r) title_x = safe_r - title_w;
        fb_text_prop(title_x, W->y + (t->title_h - fb_text_prop_h()) / 2,
                     W->title, focused ? t->text : t->text_dim);
    }

    /* One window-control component, three actions. Shared geometry keeps the
     * painted buttons and their hit targets identical; atlas icons keep them
     * crisp at every UI scale. */
    for (int b = TITLE_CLOSE; b <= TITLE_MINIMIZE; b++) {
        int bx, by, bw, bh;
        title_control_rect(W, b, &bx, &by, &bw, &bh);
        int over = ptr_x >= bx && ptr_x < bx + bw && ptr_y >= by && ptr_y < by + bh;
        unsigned face = t->panel_hi, ink = t->text_dim;
        if (over) {
            face = (b == TITLE_CLOSE) ? t->danger : t->accent;
            ink = t->panel;
        }
        fb_rrect(bx, by, bw, bh, bw / 2, face);
        if (over && (last_btn & 1))
            fb_rrect_blend(bx, by, bw, bh, bw / 2, t->bg, 48);
        int glyph = b == TITLE_CLOSE ? 13 :
                    (b == TITLE_MINIMIZE ? 22 : (wins[win].maxed ? 24 : 23));
        fb_icon24(bx + (bw - UI_DP(t, 24)) / 2,
                  by + (bh - UI_DP(t, 24)) / 2, glyph, ink);
    }

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

/* Where the toast sits. The dock is desktop furniture drawn by hook_desk and
 * wm.c does not know how tall it is, so this asks for the same reserve the
 * policy layer uses - 72 * scale, matching kernel.zl's dock_y(). A toast that
 * lands under the dock is a toast you cannot read or click. */
static void toast_rect(int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    notify_rect((int)fb_pxw(), (int)fb_pxh(), 72 * t->scale, t->scale, x, y, w, h);
}

/* How far a toast still has to RISE, in device pixels.
 * ztoast is `from{opacity:0;transform:translateY(10px)}` - ds-reference.html
 * line 20 - so it comes up ten design pixels as it fades in, the same gesture
 * zwin makes and the same constant. */
static int toast_dy(void)
{
    const struct ui_theme *t = ui_theme();
    int dy = EASE_TOAST_FROM_DY * t->scale;
    int p = anim_progress(WM_FX_TOAST, ANIM_FADE);
    if (p < 0) return 0;
    return dy - dy * p / 1000;
}

/* Drawn LAST in each damage rectangle, so it is on top of every window without
 * being in the z-order at all. Same primitives and the same theme as chrome(),
 * because a toast that does not look like the rest of the desktop reads as a
 * bug in the desktop.
 *
 * ZTOAST. notify.c has never had an animation call in it and never will: it
 * owns the QUEUE, and where a toast is on screen is the compositor's business
 * - notify_rect is already computed here rather than there for exactly that
 * reason. The entry is a rise and a fade, and the fade is a real one, stashed
 * and blended back the same way a fading window is. A tint would have been
 * three lines shorter and would have looked like a differently-coloured toast
 * rather than a translucent one. */
static void toast_draw(int rx0, int ry0, int rx1, int ry1)
{
    if (!notify_active()) return;
    const char *msg = notify_text();
    if (!msg) return;

    int x, y, w, h, cx, cy, cw, ch;
    toast_rect(&x, &y, &w, &h);
    y += toast_dy();
    if (!isect(x, y, x + w, y + h, rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) {
        /* the shadow reaches outside the panel, exactly as a window's does */
        const struct ui_theme *ts = ui_theme();
        int reach = SHADOW_OFF(ts) + SHADOW_SOFT(ts);
        if (!isect(x - reach, y - reach, x + w + reach, y + h + reach,
                   rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) return;
    }

    /* The backdrop, taken BEFORE the toast goes on it - `toast * a +
     * behind * (1-a)` cannot be reconstructed after the toast is drawn. Same
     * stash/draw/blend-back as a fading window, and the same graceful failure:
     * no free slot means the toast is simply opaque. */
    int alpha = wm_anim_alpha(WM_FX_TOAST), stash = -1;
    if (alpha < 255) stash = fb_stash(cx, cy, cw, ch);

    fb_clip(cx, cy, cw, ch);

    const struct ui_theme *t = ui_theme();
    fb_shadow(x, y, w, h, SHADOW_OFF(t), SHADOW_SOFT(t));
    fb_rrect(x, y, w, h, t->radius, t->border);
    fb_rrect(x + 1, y + 1, w - 2, h - 2, t->radius - 1, t->panel_hi);
    /* one accent stripe down the left edge: the same "this is the one
     * saturated colour" rule the focused title bar follows */
    fb_fill_px(x + 1, y + 1, UI_S1(t) / 2, h - 2, t->accent);

    int th = fb_text_prop_h();
    fb_text_prop(x + UI_S3(t), y + (h - th) / 2, msg, t->text);

    if (stash >= 0) {
        fb_clip(cx, cy, cw, ch);
        fb_stash_blend(stash, cx, cy, 255 - alpha);
        fb_blur_free(stash);
    }
}

/* ---- the ghost, and the sweep, both drawn by wm_repaint ------------------- */
static void ghost_draw(int rx0, int ry0, int rx1, int ry1)
{
    if (!ghost.live) return;
    int p = anim_progress(WM_FX_GHOST, ANIM_CLOSE);
    if (p < 0) { ghost.live = 0; return; }    /* anim_tick already damaged it */

    /* wm_anim_scale, not a second copy of the shrink: ANIM_CLOSE's curve and
     * end point live in anim_permille and this reads them. */
    int s = wm_anim_scale(WM_FX_GHOST);
    int w = ghost.w * s / 1000, h = ghost.h * s / 1000;
    int x = ghost.x + (ghost.w - w) / 2, y = ghost.y + (ghost.h - h) / 2;
    int cx, cy, cw, ch;
    if (!isect(x, y, x + w, y + h, rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch))
        return;
    fb_clip(cx, cy, cw, ch);
    const struct ui_theme *t = ui_theme();
    /* It fades as it shrinks. A ghost that stayed opaque to the last frame
     * pops out of existence, which is the thing the animation exists to stop. */
    fb_rrect_blend(x, y, w, h, t->radius, t->panel, 255 - 255 * p / 1000);
}

static void sweep_draw(int rx0, int ry0, int rx1, int ry1)
{
    if (!wm_sweep_enabled()) return;
    int bh = sweep_band_h();
    if (bh < SWEEP_BANDS) return;            /* too small to have a gradient */
    int top = sweep_top();
    int sw = (int)fb_pxw();
    unsigned int acc = ui_theme()->accent;
    int step = bh / SWEEP_BANDS;
    if (step < 1) return;
    for (int i = 0; i < SWEEP_BANDS; i++) {
        /* transparent -> peak -> transparent, i.e. a triangle over the band */
        int t2 = i * 2 + 1;                              /* 1,3,..2N-1       */
        int tri = t2 <= SWEEP_BANDS ? t2 : 2 * SWEEP_BANDS - t2;
        int a = SWEEP_A * tri / SWEEP_BANDS;
        if (a <= 0) continue;
        int by = top + i * step;
        int cx, cy, cw, ch;
        if (!isect(0, by, sw, by + step, rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch))
            continue;
        fb_clip(cx, cy, cw, ch);
        fb_fill_blend(cx, cy, cw, ch, acc, a);
    }
}

/* ---- the snap preview -----------------------------------------------------
 * visual-speed-northstar.md §"Five-part working direction" item 3 lists "add
 * the missing snap preview" as outstanding: snap.c and the drop-time wiring
 * both existed, so dragging a window to an edge DID snap it - you just could
 * not see where it was going to land until you let go.
 *
 * The state lives here, above wm_repaint, because the repaint has to read it
 * and pgrab/RESERVE_TOP are declared further down the file. Only the cached
 * rectangle is read during paint; snap_preview_set() below does the geometry,
 * because RESERVE_TOP/RESERVE_BOT are not defined until line ~1280. */

void wm_repaint(void)
{
    if (!fb_active() || !nwd) return;
    for (int r = 0; r < nwd; r++) {
        int rx0 = wd[r].x0, ry0 = wd[r].y0, rx1 = wd[r].x1, ry1 = wd[r].y1;
        fb_clip(rx0, ry0, rx1 - rx0, ry1 - ry0);

        /* the wallpaper pass: furniture first, always at the bottom, never in
         * the z-order and never overlapped by anything but a window */
        if (hook_desk) hook_desk(rx0, ry0, rx1 - rx0, ry1 - ry0);
        /* zsweep sits ON the wallpaper and UNDER everything else, so it goes
         * between the furniture pass and the first window. */
        sweep_draw(rx0, ry0, rx1, ry1);
        fb_clip(rx0, ry0, rx1 - rx0, ry1 - ry0);   /* sweep_draw narrowed it */
        snap_preview_draw(rx0, ry0, rx1, ry1);

        for (int i = 0; i < nz; i++) {          /* BACK TO FRONT = paint order */
            int win = zorder[i];
            struct win *W = &wins[win];
            /* MINIMISED, or on another workspace. Both mean "paints nothing
             * this frame"; neither means "leaves the z-order", which is what
             * keeps a workspace switch from reshuffling anything. */
            if (!win_visible(win)) continue;
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
            int stash_x = cx, stash_y = cy, stash_w = cw, stash_h = ch;
            if (anim_is(win, ANIM_FADE)) {
                fade = wm_anim_alpha(win);
                if (fade < 255) stash = fb_stash(stash_x, stash_y, stash_w, stash_h);
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
             * ANIM_FADE IS drawn here, below - a real fade, the window
             * composited against what was BEHIND it at fractional opacity,
             * from the copy `stash` took of the rectangle before the window
             * was drawn on it. Blended at (sx, sy) - where it was TAKEN
             * FROM - never at (cx, cy), which by this point is whatever the
             * client isect above left behind. */
            if (stash >= 0) {
                /* cx/cy are reused by the narrower client intersection above.
                 * Restoring at that later origin shifted the saved backdrop
                 * into the app body. Keep the capture rectangle immutable. */
                fb_clip(stash_x, stash_y, stash_w, stash_h);
                fb_stash_blend(stash, stash_x, stash_y, 255 - fade);
                fb_blur_free(stash);
            }

            if (anim_is(win, ANIM_PULSE)) {
                int pa = wm_anim_alpha(win);
                if (pa > 0 && pa < 255) {
                    fb_clip(cx, cy, cw, ch);
                    fb_rrect_blend(fx, fy, fw, fh, ui_theme()->radius,
                                   ui_theme()->accent, pa);
                }
            }
        }

        /* The closing window's ghost goes where the window would have been in
         * the walk above - on top of everything behind it. It is drawn after
         * the loop rather than inside it because it is not IN the loop's list:
         * it left the z-order the instant it was closed. */
        ghost_draw(rx0, ry0, rx1, ry1);

        /* ...and the toast on top of all of them, still inside this damage
         * rectangle. Added, not woven in: the loop above is unchanged. */
        toast_draw(rx0, ry0, rx1, ry1);
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

static int pgrab = -1;          /* which window owns the pointer, or -1     */


/* wm_close calls this. Defined here, beside the state it clears, so the grab
 * and its lifetime stay in one place. */
static void wm_drop_grab(int win)
{
    if (pgrab == win) {
        pgrab = -1;
        snap_preview_set(SNAP_NONE);
    }
}
/* What the pointer grab is FOR. It used to be a bare 0/1 meaning "the app has
 * it" or "we are moving it"; resize is a third answer, and three states with
 * two values is how a bug gets in. */
#define GRAB_APP    0           /* the app owns the pointer until button-up  */
#define GRAB_MOVE   1           /* we are dragging the frame                 */
#define GRAB_RESIZE 2           /* we are dragging the bottom-right corner   */
static int grab_drag;
static int grab_dx, grab_dy;    /* pointer offset inside the frame          */
/* Where the window was BEFORE the drag started. A drag has already moved it
 * by the time it is dropped on an edge, so capturing the restore rectangle at
 * the drop stores the dragged position - the window comes back the right SIZE
 * in the wrong PLACE. This is the rectangle un-snapping should return to. */
static int grab_ox, grab_oy, grab_ow, grab_oh;

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
    /* a window with no chrome has no grip to grab - carried over from the
       corner-square in_grip() this replaced, which had the guard where the
       edge-band version did not */
    if (wins[win].flags & WF_NOCHROME) return 0;
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
    int avail = wins[win].w - UI_DP(t, 128);  /* three controls + both margins */
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
static int in_title_control(int win, int which, int x, int y)
{
    if (wins[win].flags & WF_NOCHROME) return 0;
    int bx, by, bw, bh;
    title_control_rect(&wins[win], which, &bx, &by, &bw, &bh);
    return x >= bx && x < bx + bw && y >= by && y < by + bh;
}

static int in_closebox(int win, int x, int y)
{ return in_title_control(win, TITLE_CLOSE, x, y); }

/* The desktop's furniture, in the only two numbers wm.c needs from it: the
 * header bar at the top and the dock at the bottom. kernel.zl's TOPBAR_H and
 * dock_y() are 48 and 72, times ui(). A "maximised" window that reaches under
 * the dock cannot reach its own status bar. */
#define RESERVE_TOP(t)  UI_DP((t), 48)
#define RESERVE_BOT(t)  UI_DP((t), 72)

/* Show, move or clear the drag preview. Called on every pointer motion during
 * a frame drag, so it does nothing at all when the zone has not changed - the
 * common case is dragging around the middle of the screen with no zone, and
 * that must not damage anything or every drag frame would repaint the desktop.
 *
 * Damage is issued for the rectangle being LEFT as well as the one being
 * entered, because the preview is not a window: nothing else in the compositor
 * knows those pixels changed, and a preview that only damages where it is
 * going leaves its previous outline painted on the wallpaper. That is the same
 * mistake snap_to_rect just below documents having made with wm_move. */

/* Snap `win` to `z` (or un-snap it if z is SNAP_NONE), applying whatever
 * geometry snap.c hands back. The two triggers below both end here, so there
 * is one place where a snap actually changes a window. */
static void snap_to_rect(int win, int z, int gx, int gy, int gw, int gh)
{
    const struct ui_theme *t = ui_theme();
    int nx, ny, nw, nh;

    if (z == SNAP_NONE) {
        if (!snap_release(win, &nx, &ny, &nw, &nh)) return;
    } else if (!snap_apply(win, z, gx, gy, gw, gh,
                           (int)fb_pxw(), (int)fb_pxh(),
                           RESERVE_TOP(t), RESERVE_BOT(t), &nx, &ny, &nw, &nh)) {
        return;
    }
    /* damage the OLD rectangle before moving, or the window leaves a copy of
     * itself behind on the wallpaper - wm_move and wm_resize each damage what
     * they touch, but neither knows about the other's half of this */
    wm_damage_win(win);
    wm_move(win, nx, ny);
    wm_resize(win, nw, nh);
    wm_damage_win(win);
}

/* the ordinary entry: the restore rectangle is where the window is NOW */
static void snap_to(int win, int z)
{
    int gx, gy, gw, gh;
    wm_geometry(win, &gx, &gy, &gw, &gh);
    snap_to_rect(win, z, gx, gy, gw, gh);
}

/* The keyboard half, public so it can be driven directly. Super+arrow arrives
 * as a modifier plus a key code, and synthesising that through the event queue
 * in a harness tests the queue rather than the snapping - so the trigger and
 * the action are separated here, and both ends are reachable. */
void wm_snap_key(int win, int dir)
{
    if (!wm_is_open(win)) return;
    snap_to(win, snap_key_zone(win, dir));
}

/* Double-click and Super+Arrow deliberately meet in the same snap state.
 * Keeping a second maximise/restore slot here made a drag snap impossible to
 * restore with the keyboard after the two branches were merged. */
static void wm_toggle_max(int win)
{
    wm_snap_key(win, snap_state(win) == SNAP_NONE ? SK_UP : SK_DOWN);
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
            over = (top >= 0 && in_resize_grip(top, x, y));
        }
        fb_cursor_set(over ? CURSOR_RESIZE : CURSOR_ARROW);
    }

    /* 1. POINTER GRAB */
    if (pgrab >= 0) {
        if (grab_drag == GRAB_MOVE) {
            wm_move(pgrab, x - grab_dx, y - grab_dy);
            snap_preview_set(snap_zone_for_point(x, y,
                                                 (int)fb_pxw(), (int)fb_pxh()));
        } else if (grab_drag == GRAB_RESIZE) {
            /* grab_dx/dy hold the offset from the pointer to the corner, so
             * the corner stays under the cursor instead of snapping to it. */
            wm_resize(pgrab, x + grab_dx - wins[pgrab].x,
                             y + grab_dy - wins[pgrab].y);
        } else if (hook_event) {
            hook_event(win_app(pgrab), pgrab, EV_MOUSE, btn, x, y);
        }
        if (up) {
            /* DROPPING A DRAGGED WINDOW AT AN EDGE SNAPS IT. This wiring is
             * desktop/exec-track's (via system-track) and it was the half of
             * snapping that got lost: snap.c and snap_to_rect both survived the
             * merge, but the only caller left was the keyboard path, so
             * Super+arrow snapped and dragging to an edge did nothing.
             *
             * grab_o* is the geometry the window had when the drag STARTED, so
             * un-snapping later restores the size it was rather than the size
             * it currently has, which is half the screen. */
            if (grab_drag == GRAB_MOVE) {
                int z = snap_zone_for_point(x, y, (int)fb_pxw(), (int)fb_pxh());
                snap_preview_set(SNAP_NONE);
                if (z != SNAP_NONE) {
                    snap_to_rect(pgrab, z, grab_ox, grab_oy, grab_ow, grab_oh);
                } else if (wins[pgrab].x != grab_ox || wins[pgrab].y != grab_oy ||
                           wins[pgrab].w != grab_ow || wins[pgrab].h != grab_oh) {
                    /* A click in the title bar takes the grab path too. Do not
                     * erase snap state unless the pointer actually moved or
                     * resized the window; otherwise the second click of a
                     * double-click cannot restore a maximised window. */
                    snap_note_moved(pgrab);
                }
            }
            pgrab = -1;
        }
        return;
    }

    int m = modal_win();
    int hit = wm_at(x, y);

    /* 2. MODAL */
    if (m >= 0 && hit != m) {
        if (down) wm_close_fx(m);       /* a click outside dismisses it */
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
        int dbl = is_double(hit, x, y);
        if (in_closebox(hit, x, y)) { wm_close_fx(hit); return; }
        if (in_title_control(hit, TITLE_MAXIMIZE, x, y)) { wm_toggle_max(hit); return; }
        if (in_title_control(hit, TITLE_MINIMIZE, x, y)) { wm_minimize(hit); return; }
        /* DOUBLE-CLICK THE TITLE BAR TO MAXIMISE, again to restore. Checked
         * before the drag, or the second press starts a drag instead - and a
         * maximise that also moves the window by however far the hand drifted
         * between the two clicks is worse than no maximise. */
        if (dbl && in_titlebar(hit, x, y)) { wm_toggle_max(hit); return; }
        /* a tab BEFORE the drag: the strip lives inside the title bar, so
         * checking the drag first would make tabs unclickable */
        int tb = in_tab(hit, x, y);
        if (tb >= 0) { wm_set_tab(hit, tb); return; }
        if (in_titlebar(hit, x, y)) {
            pgrab = hit; grab_drag = GRAB_MOVE;
            snap_preview_set(SNAP_NONE);
            grab_dx = x - wins[hit].x;
            grab_dy = y - wins[hit].y;
            wm_geometry(hit, &grab_ox, &grab_oy, &grab_ow, &grab_oh);
            return;
        }
        /* THE GRIP GOES AFTER THE TITLE BAR, and the comment that used to sit
         * here said the opposite. That was true of the grip it was written
         * for - a small square in the bottom-right corner, which cannot reach
         * the title bar. This grip is a band along the whole right and bottom
         * edge, so on a window barely taller than its own title bar the band
         * covers the title bar, and checking it first made such a window
         * impossible to drag. wmtest asserts exactly that case. */
        if (in_resize_grip(hit, x, y)) {
            pgrab = hit; grab_drag = GRAB_RESIZE;
            grab_dx = (wins[hit].x + wins[hit].w) - x;
            grab_dy = (wins[hit].y + wins[hit].h) - y;
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
    /* SKIP WHAT IS ON ANOTHER WORKSPACE. Minimised windows are still cycled
     * into - wm_focus restores them, and that has always been how you get one
     * back with the keyboard - but a window on another workspace must not be
     * reachable this way, or Alt+Tab silently teleports the keyboard to
     * something that is not on screen. The bounded loop is the point: at worst
     * it inspects every entry once and gives up, so a workspace with exactly
     * one window on it cannot spin here. */
    for (int step = 0; step < nz; step++) {
        int next = (cur <= 0) ? nz - 1 : cur - 1;
        cur = next;
        int win = zorder[next];
        if (!on_ws(win)) continue;
        wm_focus(win);
        wm_raise(win);
        return;
    }
}

static void route_key(int type, int code, int mods)
{
    /* KEY_TAB, not '\t'. Both keyboards deliver the KEY code here, never the
     * character: input.c:155 and :227 map the PS/2 scancodes straight to
     * KEY_TAB, and the USB HID path goes through key_of_char (input.c:633)
     * which does the same. input_code() returns last.code, the key. So
     * `code == '\t'` compared 0x103 against 9 and this branch had never once
     * been taken. */
    if (type == EV_KEY_DOWN && code == KEY_TAB && (mods & MOD_ALT)) {
        cycle_focus();
        return;
    }

    /* SUPER + 1/2/3 SWITCHES WORKSPACE, and SUPER+SHIFT+1/2/3 SENDS THE
     * FOCUSED WINDOW to one. This is here rather than as a title-bar menu
     * because the title bar has three controls and no menu at all, so "move
     * this window to another workspace" would have needed a whole popup
     * surface before it could be reached once. A key binding is the same
     * capability for four lines, in the same place the other two window
     * bindings already live.
     *
     * ws_n bounds both, so a number past the last workspace is refused rather
     * than switching to an empty one with no pip to come back from. */
    if (type == EV_KEY_DOWN && (mods & MOD_SUPER) && code >= '1' && code <= '9') {
        int n = code - '0';
        if (mods & MOD_SHIFT) { if (focus_win >= 0) wm_set_win_ws(focus_win, n); }
        else                  wm_set_ws(n);
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
        if (code == KEY_LEFT)  { wm_snap_key(focus_win, SK_LEFT);  return; }
        if (code == KEY_RIGHT) { wm_snap_key(focus_win, SK_RIGHT); return; }
        if (code == KEY_UP)    { wm_snap_key(focus_win, SK_UP);    return; }
        if (code == KEY_DOWN)  { wm_snap_key(focus_win, SK_DOWN);  return; }
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
        if (focus_win >= 0) wm_close_fx(focus_win);
        return;
    }
    int m = modal_win();
    int target = (m >= 0) ? m : focus_win;
    if (target < 0) return;
    if (hook_event) hook_event(win_app(target), target, type, code, 0, 0);
}

/* A wheel notch goes to the window UNDER THE POINTER, not to the focused one.
 * That is the behaviour every desktop has and the one people expect: you scroll
 * what you are looking at without clicking it first. It deliberately does not
 * raise or focus that window either - scrolling is not a click. */
static void route_wheel(int x, int y, int notches)
{
    int m = modal_win();
    int hit = wm_at(x, y);
    if (m >= 0 && hit != m) return;          /* a modal owns everything */
    if (hit < 0) return;
    if (hook_event) hook_event(win_app(hit), hit, EV_WHEEL, notches, x, y);
}

static void wm_route(int type)
{
    if (type == EV_WHEEL) { route_wheel(input_x(), input_y(), input_code()); return; }
    if (type == EV_MOUSE) route_mouse(input_x(), input_y(), input_code());
    else                  route_key(type, input_code(), input_mods());
}

/* ---- the frame loop -------------------------------------------------------
 * This is the top of the system. A calibrated TSC gives a 16.667 ms deadline;
 * the 100 Hz PIT remains only as the fallback clock. The caller executes HLT
 * after every attempt, so both the early-return and idle paths sleep for an
 * interrupt instead of burning a core.
 */
/* ---- what a frame actually costs -------------------------------------------
 * idt_ticks() is 100 Hz, which is 10 ms of resolution against a 16.67 ms
 * budget - useless. The TSC is a cycle counter and cpu.c has calibrated it
 * against the PIT since it was written.
 *
 * Microseconds, not milliseconds: a cheap frame is well under 1 ms and an
 * integer millisecond would report every one of them as "0". Only frames that
 * REPAINT are timed - a frame that finds no damage returns almost immediately
 * and averaging those in would report a desktop at rest as infinitely fast. */
static unsigned int frame_us, frame_peak_us;

/* ---- the number that describes SMOOTHNESS, which neither of the two above does
 *
 * An average hides stutter by construction and a peak is one sample: both are
 * compatible with a desktop that hitches once a second, and a person perceives
 * exactly that. What they perceive is the COUNT of frames that missed, so count
 * them.
 *
 * Two different misses, counted separately because they have different causes:
 *
 *   frame_late  a frame that was TIMED and came in over FRAME_BUDGET_US. This
 *               is the compositor's own fault - it drew too much.
 *   frame_lost  a 16.667 ms presentation deadline skipped before wm_frame()
 *               sampled it. This is cadence loss, separate from draw cost.
 *
 * THE BUDGET IS 16667 us, deliberately. The thing being missed is a panel
 * refresh, and the ThinkPad's panel
 * was measured at 59.998 Hz (kernel/HANDOFF.md, from PIPE_LINK_M1/N1). A frame
 * refresh, so a faster 10 ms software tick was never the correct budget.
 *
 * Neither is a rate. They are totals since the last reset, because a rate needs
 * a denominator and the honest denominator - painted frames - is not the same
 * as elapsed ticks on a desktop that idles. `peak` prints both alongside the
 * frame count so a probe can divide if it wants to. */
#define FRAME_BUDGET_US 16667u
static unsigned int frame_late, frame_lost, frame_painted;

int wm_frame_us(void)  { return (int)frame_us; }
int wm_peak_us(void)   { return (int)frame_peak_us; }
int wm_late(void)      { return (int)frame_late; }
int wm_lost(void)      { return (int)frame_lost; }
int wm_painted(void)   { return (int)frame_painted; }
int wm_budget_us(void) { return (int)FRAME_BUDGET_US; }
void wm_peak_reset(void) { frame_peak_us = 0; frame_late = 0; frame_lost = 0;
                           frame_painted = 0; }

void wm_frame(void)
{
    unsigned int clock_khz = cpu_tsc_khz();
    unsigned int now_tsc = cpu_tsc_lo();
    if (clock_khz) {
        unsigned int cyc_us = clock_khz / 1000u;
        if (!cyc_us) cyc_us = 1;
        unsigned int interval = cyc_us * FRAME_BUDGET_US;
        if (!paced) {
            paced = 1;
            next_frame_tsc = now_tsc;
        }
        if ((int)(now_tsc - next_frame_tsc) < 0) return;
        unsigned int behind = now_tsc - next_frame_tsc;
        unsigned int missed = interval ? behind / interval : 0;
        if (missed) frame_lost += missed;
        next_frame_tsc += (missed + 1u) * interval;
    } else {
        /* A calibrated TSC is expected on every graphical target. Keep the
         * old PIT gate as an honest fallback instead of busy-spinning. */
        unsigned int tick = idt_ticks();
        if (tick == last_tick) return;
        last_tick = tick;
    }
    unsigned int now = idt_ticks();
    last_tick = now;
    /* apps-in-windows timed the frame with the 64-bit cpu_tsc(); this tree
     * uses the 32-bit cpu_tsc_lo(). Both declarations survived the merge and
     * shadowed each other on the same name. One timer. */
    unsigned int t0 = cpu_tsc_lo();
    int did_paint = 0;

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
            if (!(wins[zorder[i]].flags & WF_MINIMIZED) &&
                hook_tick(win_app(zorder[i]), zorder[i])) wm_damage_win(zorder[i]);

    /* The toast appears and retires on a tick count of its own. This damages
     * ONLY its own rectangle and only when what is on screen actually changed
     * - notify_tick returns 1 for that and 0 otherwise, the same contract
     * hook_tick uses above. No existing damage rule is altered. */
    if (notify_tick(now)) {
        int tx, ty, tw, th;
        toast_rect(&tx, &ty, &tw, &th);
        const struct ui_theme *t = ui_theme();
        int reach = SHADOW_OFF(t) + SHADOW_SOFT(t);
        /* The rectangle has to cover the RISE as well as the panel: ztoast
         * starts ten design pixels low, so a toast damaged at its settled
         * height leaves its first frames' bottom edge on the wallpaper. */
        int rise = EASE_TOAST_FROM_DY * t->scale;
        wm_damage(tx - reach, ty - reach,
                  tw + 2 * reach, th + 2 * reach + rise);
        /* ztoast: .16s ease-out, opacity 0->1 with translateY(10px)->0.
         * notify_tick returns 1 for an arrival AND for a retirement; only an
         * arrival has something to animate, and notify_active() is what tells
         * them apart. Refusal is graceful - the toast is simply already
         * there, which is what it did before this existed. */
        if (anim_on && notify_active())
            wm_anim_at(WM_FX_TOAST, ANIM_FADE, tx - reach, ty - reach,
                       tw + 2 * reach, th + 2 * reach + rise);
    }

    /* zsweep, the one animation with no event behind it: it has been running
     * since boot and will run until shutdown, so what it needs from the frame
     * loop is not a trigger but an invalidation. Only when the quantised
     * position actually moves - see sweep_top() for why that matters. */
    if (wm_sweep_enabled() && fb_active()) {
        int top = sweep_top();
        if (top != sweep_last_top) {
            int bh = sweep_band_h();
            int y0 = top < sweep_last_top ? top : sweep_last_top;
            int y1 = (top > sweep_last_top ? top : sweep_last_top) + bh;
            wm_damage(0, y0, (int)fb_pxw(), y1 - y0);
            sweep_last_top = top;
        }
    }

    if (nwd) {
        /* Only the SPRITE has a save-under to go stale. A cursor on its own
         * plane is not in the back buffer at all, so a repaint cannot smear
         * it and hiding it would be a visible flicker for no reason. */
        if (!gpu_cursor_is_live())
            fb_pointer_hide();  /* the sprite's save-under is stale once the
                                   pixels under it are about to be redrawn */
        wm_repaint();
        did_paint = 1;
    }
    /* The plane first; the sprite only if it did not take. */
    if (!gpu_cursor_move(ptr_x, ptr_y))
        fb_pointer_show(ptr_x, ptr_y);
    /* Firmware or our modeset may already have an Intel pipe scanning out.
     * Wait only on that proven source; QEMU/BGA takes the deadline path and
     * never touches an absent MMIO block. */
    if (did_paint && intel_supported && intel_wait_vblank && intel_supported())
        intel_wait_vblank();
    fb_present();

    if (did_paint) {
        unsigned int khz = clock_khz;
        if (khz) {
            /* 32-bit throughout: a 64-bit divide would pull in libgcc's
             * __udivdi3 and this kernel links no libgcc at all. At 2.3 GHz a
             * 32-bit TSC wraps every ~1.8 s, so a wrapped delta is discarded
             * rather than reported as a colossal frame. */
            unsigned int dt = cpu_tsc_lo() - t0;
            if (dt < 0x40000000u) {
                frame_us = dt / (khz / 1000u ? khz / 1000u : 1u);
                if (frame_us > frame_peak_us) frame_peak_us = frame_us;
                /* counted only where the frame was actually TIMED - a wrapped
                 * TSC delta is discarded above, and charging a discarded
                 * measurement as a miss would invent stutter out of a 1.8 s
                 * counter wrap. */
                frame_painted++;
                if (frame_us > FRAME_BUDGET_US) frame_late++;
            }
        }
    }
}
