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
#include "telemetry.h"
/* design.h, FOR SHAPE ONLY. Every colour on a window frame comes from the
 * theme struct - t->knock, t->cut, t->edge_over - because the one rule that
 * makes the palette re-pointable is that a colour literal exists in design.h
 * and nowhere else, and a ZD_* colour token read here would be a second copy
 * of the decision ui.c already made. What this include is for is the window's
 * SHAPE: ZD_WINCTL, ZD_R_BOLT, ZD_LIFT_DY and ZD_LIFT_BLUR are the prototype's
 * own measurements of the frame, and spelling them as bare numbers here is how
 * a design ends up living in two files. Nothing below reads a colour from it. */
#include "design.h"

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
/* The same atlas at a size the caller picks. fb_icon24 floors at ICON_W and so
 * physically cannot draw the window controls at the size the authority uses. */
void fb_icon_dp(int px, int py, int n, int dp, unsigned int fg);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
/* Titles are LABELS, not console text, so they take the proportional path.
 * That is the single change docs/desktop/desktop-look.md item 4 asks for at this layer. */
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
/* The three-argument form, because PRESSWORK's title bar is TWO type styles
 * rather than one: the name is the label style (MD, bold) and everything
 * beside it is a mono readout. fb_text_prop is TEXT_BODY/regular and cannot
 * say the second half of that.
 *
 * THE CHROME NO LONGER DRAWS THROUGH THESE. UI_SM/UI_MD/UI_LG used to BE
 * fb.c's roles - uikit.c static-asserted it - and they are not any more: a
 * role resolves through fb.c's `role_base` ladder, which is floored at 12px,
 * so CAPTION and BODY both come out 12 and the design's 11/13/21 cannot be
 * expressed. uikit.c now resolves UI_SM/MD/LG through design.h's ZD_T_*
 * instead, and the two call sites below that draw type go through ui_text*
 * with it. If they had kept calling fb_text_role, a window title would be
 * 12px while every widget inside that window was 13 - one design, two type
 * scales, differing by a pixel, which is exactly the kind of drift nobody
 * sees in a screenshot and everybody feels. The declarations stay because
 * fb_text_prop below is still the plain proportional path. */
void fb_text_role(int px, int py, const char *s, unsigned int fg,
                  int role, int weight);
int  fb_text_role_w(const char *s, int role, int weight);
int  fb_text_role_h(int role);
void fb_present(void);
void fb_pointer_show(int x, int y);
void fb_pointer_hide(void);
int  fb_surface_begin(unsigned int *pixels, int width, int height,
                      int origin_x, int origin_y);
int  fb_surface_begin_alpha(unsigned int *pixels, unsigned char *alpha,
                            int width, int height, int origin_x, int origin_y);
void fb_surface_end(void);
int  fb_surface_blit(const unsigned int *pixels, int width, int height,
                     int origin_x, int origin_y);
int  fb_surface_blit_alpha(const unsigned int *pixels,
                           const unsigned char *alpha,
                           int width, int height, int origin_x, int origin_y);

/* heap.c is present in every shipping kernel build. The weak boundary keeps
 * focused host harnesses linkable and makes absence a direct-render fallback
 * rather than an unresolved symbol. */
void *heap_alloc(unsigned long bytes) __attribute__((weak));
void  heap_free(void *p) __attribute__((weak));

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
int  fb_damage_count(void);
unsigned int fb_damage_area(void);
unsigned int fb_bits_per_pixel(void);

/* Optional real-panel pacing. Host harnesses and non-Intel backends leave
 * these weak symbols absent and use the TSC deadline below. */
int intel_supported(void) __attribute__((weak));
int intel_wait_vblank(void) __attribute__((weak));

/* zllog.c's frame seam is deliberately optional.  Host renderers link wm.c
 * without the flight recorder, and a missing weak symbol must cost them no
 * timer reads.  The implementation is RAM-only: persistence is scheduled
 * elsewhere, never from this latency-sensitive path.
 *
 * One record carries the whole attribution so the reader never has to join
 * half a frame after a torn write.  flags: bit 0 late, bit 1 periodic sample,
 * bit 2 an Intel vblank wait actually ran. */
void zllog_frame(unsigned input_us, unsigned tick_us,
                 unsigned compositor_us, unsigned vblank_us,
                 unsigned present_us, unsigned total_us,
                 unsigned flags, unsigned damage_count,
                 unsigned damage_area) __attribute__((weak));
void zllog_frame_observe(unsigned input_us, unsigned tick_us,
                         unsigned compositor_us, unsigned vblank_us,
                         unsigned present_us, unsigned total_us,
                         unsigned flags, unsigned damage_count,
                         unsigned damage_area, unsigned input_to_present_us,
                         unsigned input_sequence, unsigned missed_deadlines,
                         unsigned queue_depth, unsigned present_bytes,
                         unsigned desk_us, unsigned chrome_us,
                         unsigned app_us, unsigned effects_us,
                         unsigned repaint_rects, unsigned repaint_pixels,
                         unsigned window_visits, unsigned app_calls)
                         __attribute__((weak));
#define FRAMELOG_LATE    (1u << 0)
#define FRAMELOG_SAMPLE  (1u << 1)
#define FRAMELOG_VBLANK  (1u << 2)
#define FRAMELOG_CURSOR_ONLY (1u << 4)
#define FRAMELOG_SAMPLE_N 60u

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
const char *notify_body(void);
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
void snap_set_side_reserves(int left, int right);
void snap_rect_lr(int z, int sw, int sh, int reserve_top, int reserve_bot,
                  int reserve_left, int reserve_right,
                  int *x, int *y, int *w, int *h);
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

/* TWO QUESTIONS, NOT ONE, and collapsing them bolted down half the desktop.
 *
 * Maximised is a QUESTION rather than a stored flag - the snap system owns the
 * rect and the truth, so anything that needs to know asks it. That much was
 * right. But the first version of this asked `snap_state(win) != SNAP_NONE`,
 * and snap.c has SEVEN non-zero states: LEFT, RIGHT, MAX, TL, TR, BL and BR.
 * So dragging a window to the left edge made win_maxed() true, and the radius
 * reader below gave it ZD_R_BOLT - square corners and full-bleed runs on all
 * four edges - for a plate sitting on visible ground down its whole right side.
 *
 * The authority has no such state: .win.max is applied by maxWin() alone, and
 * a half-snapped window in the prototype keeps its radius.
 *
 * win_snapped answers "is the snap system holding this rect" - which is what
 * the restore path and the control glyph want. win_maxed answers "does this
 * plate touch every edge" - which is what the radius wants. They are different
 * questions and only one of them is about corners. */
#define SNAP_MAX  3                    /* snap.c SNAP_MAX */
static int win_snapped(int win) { return snap_state(win) != SNAP_NONE; }
static int win_maxed(int win)   { return snap_state(win) == SNAP_MAX; }
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
        /* THE PREVIEW MUST SHOW WHERE THE WINDOW WILL ACTUALLY LAND, and it
         * did not: this said UI_DP(t, 32), UI_DP(t, 64) - a THIRD pair of
         * hand-typed reserves, different from the 48/72 the commit path used
         * and different again from the shell's real 30/46. So the ghost was
         * drawn in one rectangle and the window dropped into another. Both
         * paths take the same macros now, which is the only way they can stay
         * equal; RESERVE_* are defined further down, hence the extern-style
         * forward use here being fine - they are macros over the theme. */
        snap_rect_lr(zone, (int)fb_pxw(), (int)fb_pxh(),
                     t->strip_h, t->foot_h, t->rail_w, 0,
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
int  input_queued(void);
unsigned int input_event_tsc(void);
unsigned int input_event_seq(void);

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

/* THE DESKTOP'S FURNITURE, TAKEN FROM THE THEME RATHER THAN RETYPED.
 *
 * These were UI_DP((t), 48) and UI_DP((t), 72), and the comment justifying them
 * cited kernel.zl's TOPBAR_H and dock_y(). Both of those symbols are gone: the
 * shell no longer has a top bar or a dock. It has a 30dp raster strip on the
 * top edge, a 46dp foot on the bottom, and - the one that actually broke - a
 * 170dp REGISTER RAIL down the LEFT, which nothing here reserved at all. A
 * maximised window was drawn from x = 0 straight over the launcher.
 *
 * Numbers that describe the shell belong to the theme, which is where the
 * shell reads them from too; two copies of 48 in two files is how the first
 * pair went stale without anything noticing. ZD_RAIL_W / ZD_STRIP_H /
 * ZD_FOOT_H are the tokens, theme.rail_w / .strip_h / .foot_h are already
 * scaled by the same q8 as everything else here. */
#define RESERVE_TOP(t)   ((t)->strip_h)
#define RESERVE_BOT(t)   ((t)->foot_h)
#define RESERVE_LEFT(t)  ((t)->rail_w)

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
/* support.c. Lifecycle receipts belong on the unattended evidence channel,
 * not in the user's Terminal scrollback. */
void ser_puts(const char *s) __attribute__((weak));

static void wm_ser_putu(unsigned int value)
{
    char digits[11];
    int n = 0;
    if (!value) { ser_puts("0"); return; }
    while (value && n < (int)sizeof(digits)) {
        digits[n++] = (char)('0' + value % 10U);
        value /= 10U;
    }
    char out[12];
    int i = 0;
    while (n) out[i++] = digits[--n];
    out[i] = 0;
    ser_puts(out);
}

static void wm_lifecycle(const char *event, int win, int app,
                         unsigned int generation, int live)
{
    /* Host compositor harnesses intentionally do not link support.c. The
     * production kernel does; a missing evidence sink must never change window
     * behavior or make a mechanism-only host test grow a fake serial device. */
    if (!ser_puts) return;
    ser_puts("wm:lifecycle v=1 event="); ser_puts(event);
    ser_puts(" slot="); wm_ser_putu((unsigned int)win);
    ser_puts(" app="); wm_ser_putu((unsigned int)app);
    ser_puts(" generation="); wm_ser_putu(generation);
    ser_puts(" live="); wm_ser_putu((unsigned int)live);
    ser_puts("\n");
}

/* ---- the table ----------------------------------------------------------- */
struct win {
    int x, y, w, h;
    int app;
    /* A slot is reusable; an observation is not. Generation distinguishes a
     * new occupant from the window that previously used the same integer. */
    unsigned int generation;
    int ready_app;              /* app whose first client draw was receipted */
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
    /* WHAT THE TITLE BAR AND THE FOOT BAND READ OUT, and none of it is a
     * decoration. PRESSWORK's header is "01 TERMINAL  zlsh" and its foot is
     * "01  tty1 - 80x24   APP US 995 us            ws 01": a register number,
     * a name, a mono qualifier, a per-window cost and a workspace. wm.c can
     * derive exactly two of those - the cost, because it makes the app_draw
     * call, and the workspace, because it owns it. The other three are POLICY
     * and belong to whoever opened the window, so they arrive through
     * wm_set_label / wm_set_status rather than being guessed at here.
     *
     * A window that never gets told stays honest: reg 0 prints no register
     * cell, an empty sub prints no qualifier, an empty status prints no
     * readout, and the band still carries the two figures wm.c really did
     * measure. Nothing in here is ever faked to fill a slot. */
    int  reg;                  /* register slot 1..99; 0 = never told */
    char sub[16];              /* the mono qualifier after the title  */
    char status[24];           /* the foot band's left readout        */
    /* app_us is written every time this window's app actually draws; band_us
     * is the value the band is SHOWING. They are two fields because they have
     * two different rates: app_us follows the app, and the band is a printed
     * readout that must not re-set itself sixty times a second. See
     * band_us_latch() for the cadence and why the shell cache needs it. */
    unsigned app_us, band_us, band_us_tick;
    /* MAXIMISE IS THE SNAP SYSTEM'S, and these fields were the evidence that
     * it used to be something else. `maxed` was READ three times and WRITTEN
     * nowhere in the tree; sav_x/sav_y/sav_w/sav_h had no references at all
     * outside this declaration. wm_toggle_max is one line - it calls
     * wm_snap_key(win, SK_UP or SK_DOWN) - and snap owns the saved rect, which
     * is why restore works and wmtest_feel's "doing it again RESTORES the exact
     * rect" passes.
     *
     * The three reads were not harmless. Each guarded a real difference a
     * maximised window is supposed to show - a bolt radius instead of a plate
     * radius, a different glyph on the control, and a cache-key bit - and each
     * tested a field that is always zero, so none of them ever fired. The
     * branches were right and had no input. win_maxed() is that input, derived
     * from the state that actually changes. */
    /* WHICH WORKSPACE. A window is on exactly one, and a workspace is not a
     * second z-order: the stack is global and unchanged, and `ws` is a filter
     * applied at the three places that ask "can this window be seen" -
     * the paint walk, the hit test, and the focus walks. Doing it that way
     * means switching workspaces cannot reorder anything, so coming back to a
     * workspace shows the stack exactly as it was left. */
    int  ws;
    /* Retained CLIENT pixels. Shell/chrome gets its own surface after this
     * seam is proved; combining them would make focus/hover/shadow lifetime
     * impossible to audit. */
    unsigned int *client_px;
    unsigned long client_bytes;
    int client_w, client_h;
    unsigned client_generation;
    int client_valid;
    /* A valid retained client can be refreshed in place. Coordinates are
     * client-local and x1/y1 are exclusive. This is what keeps one changed
     * terminal prompt row from rebuilding every glyph in the window. */
    int client_dirty;
    int client_dirty_x0, client_dirty_y0;
    int client_dirty_x1, client_dirty_y1;
    unsigned int *shell_px;
    unsigned char *shell_alpha;
    unsigned long shell_bytes;
    int shell_w, shell_h;
    unsigned shell_generation;
    unsigned shell_key;
    int shell_valid;
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
static unsigned int frame_log_seq;

/* The allocator owns 64 MiB; client surfaces may consume at most 48 MiB so
 * files, browser state and other kernel objects retain a hard 16 MiB floor. */
#define CLIENT_SURFACE_BUDGET (48UL * 1024UL * 1024UL)
static unsigned long retained_surface_used;
static unsigned client_surface_generation = 1;
static unsigned client_surface_refusals;
static unsigned retained_shell_builds;
static void window_surfaces_prepare(int win);
static void client_of(int fx, int fy, int fw, int fh, int flags,
                      int *x, int *y, int *w, int *h);
int wm_anim_running(int win);
static int dispatching_win = -1;
static int dispatch_damage_explicit;

static app_draw_fn  hook_draw;
static app_event_fn hook_event;
static app_tick_fn  hook_tick;
static desk_draw_fn hook_desk;
static overlay_draw_fn hook_overlay;      /* above the windows AND the toast */
static win_menu_fn hook_win_menu;         /* a right-press over a window     */
extern int userwin_is_app(int app) __attribute__((weak));
extern void userwin_draw_app(int app, int x, int y, int w, int h, int focused)
    __attribute__((weak));
extern int userwin_event_app(int app, int win, int type, int code, int x, int y)
    __attribute__((weak));

static int app_drawable(int app)
{
    return hook_draw || (userwin_is_app && userwin_is_app(app));
}

/* PER-WINDOW APP COST, and it is measured HERE because here is the only place
 * an app draws. The prototype's own note on the status band prices this item
 * honestly - "wm.c measures app_us in AGGREGATE only, so per-window
 * attribution is new work" - and this is that work, all of it: two reads of
 * the same 32-bit TSC the frame timer already uses, wrapped round the one call
 * that enters app code. The three call sites in wm_repaint pass their window
 * so the number lands in the right slot; before this they passed only the app,
 * which is not the same thing once a tabbed frame exists.
 *
 * NOT GATED ON paint_trace. paint_begin/paint_end next door are a profiling
 * instrument that is off by default; this is a value the chrome PRINTS, so a
 * window whose band read "0 us" unless tracing was on would be a band that
 * lies in the normal case. Two rdtsc against an app redraw - which is the most
 * expensive thing in the frame and the reason the retained client exists - is
 * not a cost worth an if.
 *
 * A zero or uncalibrated clock leaves app_us alone rather than storing a
 * garbage duration, and the same wrap guard frame_delta_us uses rejects a
 * sample that straddles a 32-bit wrap. */
static void app_draw_dispatch(int win, int app, int x, int y, int w, int h,
                              int focused)
{
    unsigned int khz = cpu_tsc_khz();
    unsigned int t0 = khz ? cpu_tsc_lo() : 0u;

    if (userwin_is_app && userwin_draw_app && userwin_is_app(app))
        userwin_draw_app(app, x, y, w, h, focused);
    else if (hook_draw)
        hook_draw(app, x, y, w, h, focused);

    if (!khz || win < 0 || win >= WM_MAX) return;
    unsigned int cyc_us = khz / 1000u;
    if (!cyc_us) cyc_us = 1;
    unsigned int delta = cpu_tsc_lo() - t0;
    if (delta >= 0x40000000u) return;        /* a wrapped/stale sample */
    wins[win].app_us = delta / cyc_us;
}
/* A CLICK THAT HITS NO WINDOW WAS DROPPED, and the dock is not a window.
 * desk_draw has painted a dock, a start button and a tray since the compositor
 * booted, and every one of them was decoration: route_mouse found no window
 * under the pointer and returned. Desktop furniture needs a route of its own
 * for the same reason it needs a draw of its own - it is not in the z-order
 * and never will be. */
static desk_click_fn hook_desk_click;
static desk_key_fn   hook_desk_key;
static can_close_fn  hook_can_close;
static overlay_click_fn hook_overlay_click;

static void client_surface_free(int win)
{
    if (win < 0 || win >= WM_MAX) return;
    if (wins[win].client_px && heap_free) heap_free(wins[win].client_px);
    if (wins[win].client_bytes <= retained_surface_used)
        retained_surface_used -= wins[win].client_bytes;
    else
        retained_surface_used = 0; /* metadata damage must not wrap the budget */
    wins[win].client_px = 0;
    wins[win].client_bytes = 0;
    wins[win].client_w = wins[win].client_h = 0;
    wins[win].client_valid = 0;
    wins[win].client_dirty = 0;
}

static void shell_surface_free(int win)
{
    if (win < 0 || win >= WM_MAX) return;
    if (wins[win].shell_px && heap_free) heap_free(wins[win].shell_px);
    if (wins[win].shell_bytes <= retained_surface_used)
        retained_surface_used -= wins[win].shell_bytes;
    else
        retained_surface_used = 0;
    wins[win].shell_px = 0;
    wins[win].shell_alpha = 0;
    wins[win].shell_bytes = 0;
    wins[win].shell_w = wins[win].shell_h = 0;
    wins[win].shell_valid = 0;
}

static void window_surfaces_free(int win)
{
    client_surface_free(win);
    shell_surface_free(win);
}

static int client_surface_ensure(int win, int width, int height)
{
    if (width <= 0 || height <= 0 || !heap_alloc || !heap_free) return 0;
    unsigned long long bytes64 = (unsigned long long)(unsigned)width *
                                 (unsigned long long)(unsigned)height * 4ULL;
    if (!bytes64 || bytes64 > CLIENT_SURFACE_BUDGET) {
        client_surface_refusals++;
        return 0;
    }
    unsigned long bytes = (unsigned long)bytes64;
    if (wins[win].client_px &&
        (wins[win].client_w != width || wins[win].client_h != height ||
         wins[win].client_generation != client_surface_generation))
        client_surface_free(win);
    if (wins[win].client_px) return 1;
    if (bytes > CLIENT_SURFACE_BUDGET - retained_surface_used) {
        client_surface_refusals++;
        return 0;
    }
    unsigned int *pixels = (unsigned int *)heap_alloc(bytes);
    if (!pixels) {
        client_surface_refusals++;
        return 0;
    }
    /* Never expose old heap contents when an app accidentally leaves a pixel
     * unpainted on its first full redraw. */
    for (unsigned long i = 0; i < bytes / 4UL; i++) pixels[i] = 0;
    wins[win].client_px = pixels;
    wins[win].client_bytes = bytes;
    wins[win].client_w = width;
    wins[win].client_h = height;
    wins[win].client_generation = client_surface_generation;
    wins[win].client_valid = 0;
    wins[win].client_dirty = 0;
    retained_surface_used += bytes;
    return 1;
}

/* A bounded, disjoint region used during one compose pass.  Subtracting an
 * opaque rectangle can produce at most four pieces.  If fragmentation would
 * exceed the bound, the caller paints the original damage rectangle: slower,
 * but never missing a changed pixel. */
#define WM_VIS_REGION_MAX 16
struct wm_region { int x0, y0, x1, y1; };
static unsigned int region_fallbacks;
static unsigned long long region_occluded_pixels;
static int win_visible(int win);

static unsigned long long region_area(const struct wm_region *r)
{
    return (unsigned long long)(unsigned)(r->x1 - r->x0) *
           (unsigned long long)(unsigned)(r->y1 - r->y0);
}

static int region_push(struct wm_region *out, int *n,
                       int x0, int y0, int x1, int y1)
{
    if (x0 >= x1 || y0 >= y1) return 1;
    if (*n >= WM_VIS_REGION_MAX) return 0;
    out[*n].x0 = x0; out[*n].y0 = y0;
    out[*n].x1 = x1; out[*n].y1 = y1;
    (*n)++;
    return 1;
}

static int region_subtract(struct wm_region *list, int *count,
                           int ox0, int oy0, int ox1, int oy1)
{
    struct wm_region next[WM_VIS_REGION_MAX];
    int nn = 0;
    for (int i = 0; i < *count; i++) {
        struct wm_region r = list[i];
        int ix0 = r.x0 > ox0 ? r.x0 : ox0;
        int iy0 = r.y0 > oy0 ? r.y0 : oy0;
        int ix1 = r.x1 < ox1 ? r.x1 : ox1;
        int iy1 = r.y1 < oy1 ? r.y1 : oy1;
        if (ix0 >= ix1 || iy0 >= iy1) {
            if (!region_push(next, &nn, r.x0, r.y0, r.x1, r.y1)) return 0;
            continue;
        }
        /* Four non-overlapping strips around the intersection. */
        if (!region_push(next, &nn, r.x0, r.y0, r.x1, iy0) ||
            !region_push(next, &nn, r.x0, iy1, r.x1, r.y1) ||
            !region_push(next, &nn, r.x0, iy0, ix0, iy1) ||
            !region_push(next, &nn, ix1, iy0, r.x1, iy1)) return 0;
    }
    for (int i = 0; i < nn; i++) list[i] = next[i];
    *count = nn;
    return 1;
}

static int window_opaque_rect(int win, int *x0, int *y0, int *x1, int *y1)
{
    if (!win_visible(win) || wm_anim_running(win)) return 0;
    int r = ui_theme()->radius;
    if (r < 1) r = 1;
    *x0 = wins[win].x + r; *y0 = wins[win].y + r;
    *x1 = wins[win].x + wins[win].w - r;
    *y1 = wins[win].y + wins[win].h - r;
    return *x0 < *x1 && *y0 < *y1;
}

static int visible_damage_regions(int zpos, int x0, int y0, int x1, int y1,
                                  struct wm_region *out)
{
    out[0].x0 = x0; out[0].y0 = y0; out[0].x1 = x1; out[0].y1 = y1;
    int n = 1;
    unsigned long long before = region_area(&out[0]);
    for (int j = zpos + 1; j < nz; j++) {
        int ox0, oy0, ox1, oy1;
        if (!window_opaque_rect(zorder[j], &ox0, &oy0, &ox1, &oy1)) continue;
        if (!region_subtract(out, &n, ox0, oy0, ox1, oy1)) {
            region_fallbacks++;
            out[0].x0 = x0; out[0].y0 = y0; out[0].x1 = x1; out[0].y1 = y1;
            return 1;
        }
        if (!n) break;
    }
    unsigned long long after = 0;
    for (int i = 0; i < n; i++) after += region_area(&out[i]);
    if (after < before) region_occluded_pixels += before - after;
    return n;
}

unsigned int wm_region_fallbacks(void) { return region_fallbacks; }
unsigned long long wm_region_occluded_pixels(void) { return region_occluded_pixels; }

int wm_region_fragmentation_probe(void)
{
    struct wm_region r[WM_VIS_REGION_MAX] = {{ 0, 0, 1000, 1000 }};
    int n = 1;
    for (int i = 0; i < 8; i++)
        if (!region_subtract(r, &n, 495, 100 + i * 100,
                            505, 110 + i * 100)) return 1;
    return 0;
}

static int shell_surface_ensure(int win, int width, int height)
{
    if (width <= 0 || height <= 0 || !heap_alloc || !heap_free) return 0;
    unsigned long long bytes64 = (unsigned long long)(unsigned)width *
                                 (unsigned long long)(unsigned)height * 5ULL;
    if (!bytes64 || bytes64 > CLIENT_SURFACE_BUDGET) {
        client_surface_refusals++;
        return 0;
    }
    unsigned long bytes = (unsigned long)bytes64;
    if (wins[win].shell_px &&
        (wins[win].shell_w != width || wins[win].shell_h != height ||
         wins[win].shell_generation != client_surface_generation))
        shell_surface_free(win);
    if (wins[win].shell_px) return 1;
    if (bytes > CLIENT_SURFACE_BUDGET - retained_surface_used) {
        client_surface_refusals++;
        return 0;
    }
    unsigned int *pixels = (unsigned int *)heap_alloc(bytes);
    if (!pixels) {
        client_surface_refusals++;
        return 0;
    }
    unsigned long pixels_bytes = (unsigned long)(unsigned)width *
                                 (unsigned long)(unsigned)height * 4UL;
    unsigned char *alpha = (unsigned char *)pixels + pixels_bytes;
    for (unsigned long i = 0; i < pixels_bytes / 4UL; i++) pixels[i] = 0;
    for (unsigned long i = 0; i < bytes - pixels_bytes; i++) alpha[i] = 0;
    wins[win].shell_px = pixels;
    wins[win].shell_alpha = alpha;
    wins[win].shell_bytes = bytes;
    wins[win].shell_w = width;
    wins[win].shell_h = height;
    wins[win].shell_generation = client_surface_generation;
    wins[win].shell_valid = 0;
    retained_surface_used += bytes;
    return 1;
}

static void wm_invalidate_shell(int win)
{
    if (win < 0 || win >= WM_MAX || !(wins[win].flags & WF_OPEN)) return;
    wins[win].shell_valid = 0;
    wm_damage_win(win);
}

void wm_invalidate_client(int win)
{
    if (win < 0 || win >= WM_MAX || !(wins[win].flags & WF_OPEN)) return;
    wins[win].client_valid = 0;
    wins[win].client_dirty = 0;
    wm_damage_win(win);
}

/* Refresh a client-local rectangle in an otherwise valid retained surface.
 * The full invalidation path remains the correctness fallback for a first
 * draw, an animation, a missing surface, or bad geometry. */
void wm_invalidate_client_rect(int win, int x, int y, int w, int h)
{
    if (win < 0 || win >= WM_MAX || !(wins[win].flags & WF_OPEN)) return;
    struct win *W = &wins[win];
    if (w <= 0 || h <= 0) return;
    if (!W->client_valid || !W->client_px || wm_anim_running(win)) {
        if (win == dispatching_win) dispatch_damage_explicit = 1;
        wm_invalidate_client(win);
        return;
    }
    long long lx0 = x, ly0 = y;
    long long lx1 = lx0 + (long long)w, ly1 = ly0 + (long long)h;
    if (lx0 < 0) lx0 = 0;
    if (ly0 < 0) ly0 = 0;
    if (lx1 > W->client_w) lx1 = W->client_w;
    if (ly1 > W->client_h) ly1 = W->client_h;
    int x0 = (int)lx0, y0 = (int)ly0, x1 = (int)lx1, y1 = (int)ly1;
    if (x0 >= x1 || y0 >= y1) return;
    if (win == dispatching_win) dispatch_damage_explicit = 1;
    if (!W->client_dirty) {
        W->client_dirty_x0 = x0; W->client_dirty_y0 = y0;
        W->client_dirty_x1 = x1; W->client_dirty_y1 = y1;
        W->client_dirty = 1;
    } else {
        if (x0 < W->client_dirty_x0) W->client_dirty_x0 = x0;
        if (y0 < W->client_dirty_y0) W->client_dirty_y0 = y0;
        if (x1 > W->client_dirty_x1) W->client_dirty_x1 = x1;
        if (y1 > W->client_dirty_y1) W->client_dirty_y1 = y1;
    }
    int ax, ay, aw, ah;
    client_of(W->x, W->y, W->w, W->h, W->flags, &ax, &ay, &aw, &ah);
    (void)aw; (void)ah;
    wm_damage(ax + x0, ay + y0, x1 - x0, y1 - y0);
}

unsigned long wm_client_surface_bytes(void) { return retained_surface_used; }
unsigned int wm_client_surface_refusals(void) { return client_surface_refusals; }
unsigned int wm_retained_shell_builds(void) { return retained_shell_builds; }

void wm_surface_mode_changed(void)
{
    client_surface_generation++;
    if (!client_surface_generation) client_surface_generation = 1;
    for (int i = 0; i < WM_MAX; i++) {
        if (wins[i].client_px || wins[i].shell_px) window_surfaces_free(i);
        if (wins[i].flags & WF_OPEN) wins[i].client_valid = 0;
    }
    for (int i = 0; i < WM_MAX; i++)
        if (wins[i].flags & WF_OPEN) window_surfaces_prepare(i);
}

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

/* RAM-only attribution for one compositor pass. Enabled only when a recorder
 * hook is linked, so standalone renderers pay no extra TSC reads. The final
 * compositor bucket minus these four owners remains loop/intersection/cursor
 * overhead and is intentionally recoverable by the host extractor. */
static int paint_trace;
static unsigned int paint_desk_cycles, paint_chrome_cycles;
static unsigned int paint_app_cycles, paint_effect_cycles;
static unsigned int paint_repaint_rects, paint_repaint_pixels;
static unsigned int paint_window_visits, paint_app_calls;

static unsigned int paint_begin(void)
{
    return paint_trace ? cpu_tsc_lo() : 0u;
}

static void paint_end(unsigned int *bucket, unsigned int began)
{
    if (paint_trace) *bucket += cpu_tsc_lo() - began;
}

static void paint_reset(int enabled)
{
    paint_trace = enabled;
    paint_desk_cycles = paint_chrome_cycles = 0;
    paint_app_cycles = paint_effect_cycles = 0;
    paint_repaint_rects = enabled ? (unsigned int)nwd : 0u;
    paint_repaint_pixels = paint_window_visits = paint_app_calls = 0u;
    if (!enabled) return;
    unsigned long long pixels = 0;
    for (int i = 0; i < nwd; i++)
        pixels += (unsigned)(wd[i].x1 - wd[i].x0) *
                  (unsigned)(wd[i].y1 - wd[i].y0);
    paint_repaint_pixels = pixels > 0xffffffffULL ? 0xffffffffu : (unsigned)pixels;
}

static int wd_mergeable(int i, int x0, int y0, int x1, int y1)
{
    int touches = !(x0 > wd[i].x1 || x1 < wd[i].x0 ||
                    y0 > wd[i].y1 || y1 < wd[i].y0);
    if (!touches) return 0;
    int overlaps = x0 < wd[i].x1 && x1 > wd[i].x0 &&
                   y0 < wd[i].y1 && y1 > wd[i].y0;
    if (overlaps) return 1;
    int ux0 = x0 < wd[i].x0 ? x0 : wd[i].x0;
    int uy0 = y0 < wd[i].y0 ? y0 : wd[i].y0;
    int ux1 = x1 > wd[i].x1 ? x1 : wd[i].x1;
    int uy1 = y1 > wd[i].y1 ? y1 : wd[i].y1;
    unsigned long long sum =
        (unsigned long long)(unsigned)(x1 - x0) * (unsigned)(y1 - y0) +
        (unsigned long long)(unsigned)(wd[i].x1 - wd[i].x0) *
        (unsigned)(wd[i].y1 - wd[i].y0);
    unsigned long long box =
        (unsigned long long)(unsigned)(ux1 - ux0) * (unsigned)(uy1 - uy0);
    return box <= sum || (box - sum) * 100ULL <= sum * 25ULL;
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
        if (wd_mergeable(i, x0, y0, x1, y1)) {
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
/* PRESSWORK has exactly ONE shadow and it is ZD_LIFT: 5dp of offset under 13dp
 * of blur, black at 55%. It replaces the 8dp/6dp pair that used to be here,
 * which was a shadow every window wore at rest - see chrome_shadow, where the
 * "at rest" half of that is now gone entirely. The alpha is fb.c's own and is
 * not a parameter of fb_shadow, so ZD_LIFT_A is not reachable from this file;
 * fb.c is the vocabulary and is deliberately not edited. */
#define SHADOW_OFF(t)   (UI_DP((t), ZD_LIFT_DY))
#define SHADOW_SOFT(t)  (UI_DP((t), ZD_LIFT_BLUR))
/* THE OFF-PLANE PAIR IS A DIFFERENT PAIR, NOT A MULTIPLE OF THIS ONE. The
 * palette sheet, the menu and the toast all carry 6dp/14dp in the authority
 * (proto:897, :942, :965); the dragged plate carries 5dp/13dp (proto:643).
 * The two sites below used `off * 3 / 2` on the plate pair, which is 7/19 -
 * a figure that is in no document. See design.h's ZD_OFFPLANE_* block. */
#define OFFPLANE_OFF(t)  (UI_DP((t), ZD_OFFPLANE_DY))
#define OFFPLANE_SOFT(t) (UI_DP((t), ZD_OFFPLANE_BLUR))

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
 * dirt on the screen that nothing will ever clean up.
 *
 * PRESSWORK MADE THAT MAXIMUM MATTER MORE, NOT LESS. An ordinary window now
 * casts nothing at rest and ZD_LIFT only while it is in hand, so the reach
 * changes on grab and on release - two more moments where a reach computed
 * from the CURRENT state would under-damage. This still returns the largest
 * reach the window can ever have, which is the lifted one; the resting window
 * simply over-damages by a band it never used, which is the safe direction. */
static int shadow_reach(int win)
{
    const struct ui_theme *t = ui_theme();
    int off = SHADOW_OFF(t), soft = SHADOW_SOFT(t);
    if (wins[win].flags & WF_MODAL) { off = OFFPLANE_OFF(t); soft = OFFPLANE_SOFT(t); }
    return off + soft;          /* the LIFTED size; at rest there is none */
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
 * focus snapped. docs/desktop/desktop-polish-and-speed.md calls that the single biggest
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
/* THE FOCUS CHANGE, WHICH WAS A CUT AND WAS NOT SUPPOSED TO BE.
 *
 * design.h on ZD_MS_RISE: "the load-bearing half of the focus signal. A CUT
 * (0ms) was offered and REFUSED: the knockout is a full value flip and cutting
 * it makes the whole screen twitch."
 *
 * wm_focus invalidated and damaged and nothing else, so the flip happened in
 * one frame - the refused option, shipped. The prototype transitions six
 * properties on this change; this animates the one that carries it, the
 * header band, over RISE. */
#define ANIM_FOCUS  6

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

/* THE DURATIONS COME FROM design.h NOW, AND THERE ARE THREE OF THEM.
 *
 * These read ease.h's EASE_MS_*, whose own comment says they are "exactly as
 * the reference states them" and cites ds-reference.html - the SUPERSEDED
 * predecessor. Its scheme had ten durations: 200, 100, 250, 1000, 2600, 160,
 * 160, 7000.
 *
 * design.h replaced that with three - RISE 90, TRAVEL 160, SETTLE 240 - and
 * defined ZD_MS_WIN / ZD_MS_PRESS / ZD_MS_OV / ZD_MS_PULSE as names mapped onto
 * them, with the comment "They keep their names so wm.c's timeline keeps
 * compiling; there are three values here, not ten, and that is the point."
 *
 * wm.c never took them up. The mapping was written, the old header stayed
 * wired, and every animation in the compositor has been running on the
 * predecessor's numbers since - a press acknowledging in 250 ms where the
 * design says 90, a window opening in 200 where it says 160. Intent written
 * down and never connected is this repo's most repeated failure and this is
 * another instance of it. */
static const unsigned char anim_ticks[] = {
    /* NONE  */ 0,
    /* OPEN  */ MS_TO_TICKS(ZD_MS_WIN),       /* TRAVEL 160 */
    /* CLOSE */ MS_TO_TICKS(ZD_MS_WIN),       /* its mirror */
    /* PRESS */ MS_TO_TICKS(ZD_MS_PRESS),     /* RISE    90 */
    /* PULSE */ MS_TO_TICKS(ZD_MS_PULSE),     /* SETTLE 240 */
    /* FADE  */ MS_TO_TICKS(ZD_MS_OV),        /* TRAVEL 160 */
    /* FOCUS */ MS_TO_TICKS(ZD_MS_RISE),      /* RISE    90 */
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
    /* FOCUS */ EASE_STD,
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
    /* FOCUS NEVER TAKES THE LAST SLOTS.
     *
     * The table is eight slots shared by every window and every piece of
     * furniture, and a focus change starts TWO animations - the window gaining
     * it and the one losing it. Adding that made focus the most frequent
     * animation in the system, and it promptly starved a fade: wmtest's
     * "the alpha really is partial" failed because ANIM_FADE was refused a slot
     * and never ran, so the window drew opaque. The file's own comment two
     * hundred lines up describes this hazard from the other direction, when an
     * open animation that had "previously been REFUSED for want of a slot" got
     * one and outvoted a fade.
     *
     * A focus transition is the cheapest thing here to lose - it degrades to
     * the cut it replaced, which is what shipped until today - and a fade or an
     * open degrading is a visible fault. So focus keeps two slots free for
     * them. */
    int free_slots = 0;
    for (int i = 0; i < ANIM_MAX; i++) if (!anims[i].kind) free_slots++;
    if (kind == ANIM_FOCUS && free_slots <= 2) return 0;

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
/* Two colours, p thousandths of the way from a to b. Integer, per channel, no
 * float in the drawing path - the same rule fb_mix follows. */
static unsigned blend_rgb(unsigned a, unsigned b, int p)
{
    if (p < 0) p = 0;
    if (p > 1000) p = 1000;
    int q = 1000 - p;
    unsigned r = ((((a >> 16) & 0xFF) * q) + (((b >> 16) & 0xFF) * p)) / 1000;
    unsigned g = ((((a >> 8)  & 0xFF) * q) + (((b >> 8)  & 0xFF) * p)) / 1000;
    unsigned c = ((( a        & 0xFF) * q) + (( b        & 0xFF) * p)) / 1000;
    return (r << 16) | (g << 8) | c;
}

static int anim_progress(int win, int kind)
{
    /* MOTION OFF means every RUNNING transition is already finished. It does
     * NOT mean every query answers "finished".
     *
     * The first version returned 1000 unconditionally, and -1 is how this
     * function says "no such animation" - so with motion off every caller was
     * told every animation was complete, including ones that had never started.
     * chrome_header gates on `focused || fp >= 0`, so the focus fill ran for
     * UNFOCUSED windows and erased the struck top run on all of them.
     *
     * The shortcut still has to answer the existence question; it only skips
     * the easing. */
    if (!ui_motion_get()) {
        for (int i = 0; i < ANIM_MAX; i++)
            if (anims[i].kind == kind && anims[i].win == win) return 1000;
        return -1;
    }
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
    /* THE FOOT BAND PRINTS THE WORKSPACE, so the cached shell is now wrong -
     * and damage alone would not fix it, because damage repaints from the
     * cache. shell_state_key() deliberately does not carry ws (see the note
     * there), so the invalidation belongs here, at the write. */
    wins[win].shell_valid = 0;
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

/* HOW TALL THIS WINDOW'S FOOT BAND IS, and it is a function of the window
 * rather than of the theme for exactly the reason chrome_header() clamps its
 * own band: wm_open() stores the caller's h verbatim and min_h is a RESIZE
 * floor applied afterwards, so nothing guarantees a chrome window is tall
 * enough for its own furniture.
 *
 * ONE ANSWER, TWO READERS. client_of() subtracts this and chrome_band() draws
 * it. If they could ever disagree the app would paint into the band or the
 * band would paint over the app, and which of the two you got would depend on
 * the window's height - the kind of bug that reproduces on one screen size.
 * So it is a function, not two copies of the same arithmetic.
 *
 * ZD_STATUS_H plus one row for the 1px ZD_CUT rule along its top, which is
 * .sband's `border-top` and is 1.4723:1 on the plate - a groove, not a line
 * you read. THE BAND IS REFUSED RATHER THAN SQUEEZED when the window cannot
 * also afford ZD_STATUS_H of client above it: a foot band with nothing over it
 * is a window that has become a caption, and the prototype has no such state.
 * The threshold is a clean step because the client rect is a retained surface
 * and a band that faded in over four pixels would reallocate it four times. */
static int band_h_of(int fh, int flags)
{
    const struct ui_theme *t = ui_theme();
    if (flags & WF_NOCHROME) return 0;
    /* BORDER-BOX, like .hdr. `.sband { height: var(--zd-band-h); border-top:
     * 1px }` with --zd-band-h 20dp means 20dp TOTAL with the rule inside it;
     * this added a row for the rule on top of the full height, so the band was
     * 21dp. Identical to the header's fault two commits ago, in the element at
     * the other end of the plate - both from reading a CSS height as content
     * when the sheet sets box-sizing: border-box globally. */
    int bh = t->band_h;
    if (fh - t->title_h - 2 - bh < UI_DP(t, ZD_STATUS_H)) return 0;
    return bh;
}

/* The CLIENT area: inside the frame, below the title bar and ABOVE THE FOOT
 * BAND. This is the second, narrower scissor in the repaint - the one that
 * means an app physically cannot draw over its own title bar no matter what it
 * does, and now the same for its status band. */
static void client_of(int fx, int fy, int fw, int fh, int flags,
                      int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    int b  = (flags & WF_NOCHROME) ? 0 : 2;
    int th = (flags & WF_NOCHROME) ? 0 : t->title_h;
    int bh = band_h_of(fh, flags);
    /* THE LEFT INSET CLEARS THE VERMILION FOCUS BAR - the plate's 1px ring
     * plus the bar's own 3dp - AND IT DOES SO IN EVERY FOCUS STATE, not only
     * in the focused one. The prototype moves .wbody's padding when a window
     * takes focus. It cannot work that way here: the client area is a RETAINED
     * SURFACE and its rectangle is what an app lays itself out against, so a
     * focus-dependent width would resize every app's surface and re-flow its
     * layout each time the pointer picked a different window - a whole class
     * of repaint churn bought for three pixels of gutter.
     *
     * Constant is the right trade. An unfocused plate shows a little more of
     * its own ground down the left, the client rect is stable, and - the part
     * that is not cosmetic - the app can never paint over the focus bar. The
     * shell layer is composited BEFORE the client, so without this the bar
     * would be drawn and then two thirds of it immediately overwritten. */
    /* THE CLIENT MUST MOVE WITH THE CHROME, and two commits ago it did not.
     *
     * Making .hdr border-box put its foot rule at row fy + th, and floating the
     * foot band up by .wbody's 6dp bottom padding freed rows at the other end -
     * and client_of was left computing the old rectangle for both. The shell is
     * composited BEFORE the client, so the app won: its first row painted over
     * the header's knockout edge, and its last rows painted over the foot
     * band's rule and into the band itself, five rows at scale 1 and eleven at
     * scale 2.
     *
     * That is the cost of a geometry with two readers: moving one is a change,
     * moving one of two is a bug, and the chrome and the client are the two
     * readers most likely to be edited apart.
     *
     * The header owns rows fy+1 .. fy+th inclusive, so the client starts after
     * it. The band, when there is one, floats ZD_BODY_PY above the ring, so the
     * client stops that much earlier. */
    int bl = (flags & WF_NOCHROME) ? 0 : 1 + t->focus_bar;
    int bpy = bh ? UI_DP(t, ZD_BODY_PY) : 0;
    *x = fx + bl;
    *y = fy + th + 1;
    *w = fw - bl - b;
    *h = fh - th - 1 - b - bh - bpy;
    if (*w < 0) *w = 0;
    if (*h < 0) *h = 0;
}

/* Allocation is lifecycle work, never frame work. Open/resize/restore and a
 * mode-generation change prepare settled-size surfaces. A refusal keeps the
 * direct renderer as fallback without retrying heap work on every frame. */
static void window_surfaces_prepare(int win)
{
    if (win < 0 || win >= WM_MAX || !(wins[win].flags & WF_OPEN)) return;
    int ax, ay, aw, ah;
    client_of(wins[win].x, wins[win].y, wins[win].w, wins[win].h,
              wins[win].flags, &ax, &ay, &aw, &ah);
    (void)ax; (void)ay;
    (void)client_surface_ensure(win, aw, ah);
    int reach = shadow_reach(win);
    (void)shell_surface_ensure(win, wins[win].w + 2 * reach,
                              wins[win].h + 2 * reach);
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
    for (int i = 0; i < WM_MAX; i++) window_surfaces_free(i);
    client_surface_generation++;
    if (!client_surface_generation) client_surface_generation = 1;
    client_surface_refusals = 0;
    retained_shell_builds = 0;
    region_fallbacks = 0;
    region_occluded_pixels = 0;
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
    frame_log_seq = 0;
    running = 1;
    if (fb_active()) wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
}

void wm_desk_click(desk_click_fn f) { hook_desk_click = f; }
void wm_desk_key(desk_key_fn f)     { hook_desk_key = f; }
void wm_can_close(can_close_fn f)   { hook_can_close = f; }
void wm_overlay_click(overlay_click_fn f) { hook_overlay_click = f; }

void wm_hooks(app_draw_fn d, app_event_fn e, app_tick_fn t, desk_draw_fn desk)
{
    hook_draw = d; hook_event = e; hook_tick = t; hook_desk = desk;
}

/* Registered separately from wm_hooks() rather than added as a fifth parameter,
 * for the reason snap_rect_lr exists: wm_hooks has callers that mean "the four
 * layers this desktop had", and widening the signature would edit every one of
 * them to pass 0 and prove nothing. */
void wm_overlay(overlay_draw_fn f) { hook_overlay = f; }
void wm_win_menu(win_menu_fn f) { hook_win_menu = f; }

/* ---- A WINDOW, BOX-FILTERED INTO A TILE -------------------------------------
 * The prototype's activities grid does not draw placeholders. Each tile is the
 * window's own content at scale - it clones the .wbody and scales the clone -
 * so the overview answers "which window is that" by showing it rather than by
 * naming it. A tile with an empty body would be a picture of a preview.
 *
 * zlOS had no way to do this. fb_surface_blit is 1:1, and every box filter in
 * this tree - the icon atlas, the fonts - runs OFFLINE in a generator and ships
 * as a table. This is the first one that runs at draw time.
 *
 * IT READS THE RETAINED CLIENT SURFACE, which is what makes it cheap enough to
 * be worth doing: `client_px` is already the window's last painted content, so
 * a thumbnail costs no app code, no re-entry into zl, and nothing that can
 * recurse. A window whose surface is not valid is skipped rather than faked -
 * an app that has never painted has nothing to show, and inventing something
 * would be the same lie as a placeholder.
 *
 * Integer box filter: destination pixel (x, y) is the average of the source
 * block it covers. No floats - this is the drawing path. The empty-block guard
 * (sx1 <= sx0) matters when the tile is LARGER than the source in an axis,
 * which happens to a very small window in a wide grid.
 *
 * fb_fill_px at 1x1 rather than a direct framebuffer write, because it is the
 * one that honours the scissor - and this is drawn from the overlay layer,
 * where the scissor is the damage rectangle. */
int wm_thumb(int win, int dx, int dy, int dw, int dh)
{
    if (win < 0 || win >= WM_MAX) return 0;
    struct win *W = &wins[win];
    if (!(W->flags & WF_OPEN) || !W->client_px || !W->client_valid) return 0;
    if (dw <= 0 || dh <= 0 || W->client_w <= 0 || W->client_h <= 0) return 0;

    for (int y = 0; y < dh; y++) {
        int sy0 = (int)((long long)y * W->client_h / dh);
        int sy1 = (int)((long long)(y + 1) * W->client_h / dh);
        if (sy1 <= sy0) sy1 = sy0 + 1;
        if (sy1 > W->client_h) sy1 = W->client_h;
        for (int x = 0; x < dw; x++) {
            int sx0 = (int)((long long)x * W->client_w / dw);
            int sx1 = (int)((long long)(x + 1) * W->client_w / dw);
            if (sx1 <= sx0) sx1 = sx0 + 1;
            if (sx1 > W->client_w) sx1 = W->client_w;
            unsigned long r = 0, g = 0, b = 0, n = 0;
            for (int sy = sy0; sy < sy1; sy++) {
                const unsigned int *row = W->client_px + (unsigned long)sy * W->client_w;
                for (int sx = sx0; sx < sx1; sx++) {
                    unsigned int px = row[sx];
                    r += (px >> 16) & 0xFFu;
                    g += (px >> 8) & 0xFFu;
                    b += px & 0xFFu;
                    n++;
                }
            }
            if (!n) continue;
            fb_fill_px(dx + x, dy + y, 1, 1,
                       (unsigned int)(((r / n) << 16) | ((g / n) << 8) | (b / n)));
        }
    }
    return 1;
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

static void str_copy_n(char *dst, const char *src, int cap)
{
    int i = 0;
    if (src) for (; src[i] && i < cap - 1; i++) dst[i] = src[i];
    dst[i] = 0;
}

/* THE TWO THINGS THE HEADER AND THE BAND CANNOT DERIVE, handed over rather
 * than invented. `reg` is the window's slot in the shell's REGISTER rail and
 * `sub` is the mono qualifier that follows the title - "01 TERMINAL  zlsh".
 * Both are policy: wm.c has an app id and a title and neither of them is a
 * register number.
 *
 * reg <= 0 or > 99 clears the cell, which is also the state a window that was
 * never told is in. There is no default and no fallback to the app id: an app
 * id printed in a register slot would be a plausible-looking wrong number, and
 * this whole readout exists to be trusted.
 *
 * Both invalidate the retained shell directly rather than being folded into
 * shell_state_key(). They change at most once in a window's life, and a key
 * bit spent on something that never changes is a bit the states that DO change
 * every frame cannot have. */
void wm_set_label(int win, int reg, const char *sub)
{
    if (!wm_is_open(win)) return;
    int r = (reg > 0 && reg <= 99) ? reg : 0;
    char buf[16];
    str_copy_n(buf, sub, (int)sizeof buf);
    int same = (wins[win].reg == r);
    for (int i = 0; same && i < (int)sizeof buf; i++)
        if (wins[win].sub[i] != buf[i]) same = 0;
    if (same) return;
    wins[win].reg = r;
    str_copy_n(wins[win].sub, buf, (int)sizeof wins[win].sub);
    wm_invalidate_shell(win);
}

/* The foot band's left readout - "tty1 - 80x24", "rd0 30 entries", whatever
 * this app's one line of state is. Empty is a legitimate value and prints
 * nothing; the band still carries the app cost and the workspace, which wm.c
 * measured and owns. */
void wm_set_status(int win, const char *status)
{
    if (!wm_is_open(win)) return;
    char buf[24];
    str_copy_n(buf, status, (int)sizeof buf);
    int same = 1;
    for (int i = 0; same && i < (int)sizeof buf; i++)
        if (wins[win].status[i] != buf[i]) same = 0;
    if (same) return;
    str_copy_n(wins[win].status, buf, (int)sizeof wins[win].status);
    wm_invalidate_shell(win);
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
    wm_invalidate_shell(win);
    return i;
}

int wm_tab(int win)   { return wm_is_open(win) ? wins[win].tab  : -1; }
int wm_ntabs(int win) { return wm_is_open(win) ? wins[win].ntab : 0; }

void wm_set_tab(int win, int tab)
{
    if (!wm_is_open(win) || tab < 0 || tab >= wins[win].ntab) return;
    if (wins[win].tab == tab) return;
    wins[win].tab = tab;
    wins[win].shell_valid = 0;
    wm_invalidate_client(win);
}

/* Which app this window is SHOWING. Everything downstream asks this rather
 * than reading .app, so a tabbed window and a plain one are the same thing to
 * the repaint and the routing - which is what keeps tabs from being a special
 * case threaded through the whole file. */
static int win_app(int win) { return wins[win].tab_app[wins[win].tab]; }

int wm_open(int app, const char *title, int x, int y, int w, int h)
{
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_KERNEL, 0u,
        ZLLOG_OP_WINDOW_OPEN, (unsigned)app);
    for (int i = 0; i < WM_MAX; i++) {
        if (wins[i].flags & WF_OPEN) continue;
        wins[i].x = x; wins[i].y = y; wins[i].w = w; wins[i].h = h;
        wins[i].app = app;
        wins[i].generation++;
        if (!wins[i].generation) wins[i].generation = 1;
        wins[i].ready_app = -1;
        wins[i].flags = WF_OPEN;
        wins[i].client_px = 0;
        wins[i].client_bytes = 0;
        wins[i].client_w = wins[i].client_h = 0;
        wins[i].client_generation = client_surface_generation;
        wins[i].client_valid = 0;
        wins[i].client_dirty = 0;
        wins[i].shell_px = 0;
        wins[i].shell_alpha = 0;
        wins[i].shell_bytes = 0;
        wins[i].shell_w = wins[i].shell_h = 0;
        wins[i].shell_generation = client_surface_generation;
        wins[i].shell_key = 0;
        wins[i].shell_valid = 0;
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
        /* A REUSED SLOT MUST NOT INHERIT THE PREVIOUS OCCUPANT'S READOUT. The
         * register, the qualifier and the status line are all told to a window
         * from outside, so a slot that is not cleared here would open the next
         * window wearing the last one's register number - the exact failure
         * `generation` exists further up to make impossible for observations. */
        wins[i].reg = 0;
        wins[i].sub[0] = 0;
        wins[i].status[0] = 0;
        wins[i].app_us = 0;
        wins[i].band_us = 0;
        wins[i].band_us_tick = 0;
        wins[i].tab_app[0] = app;
        title_copy16(wins[i].tab_title[0], title);
        title_copy(wins[i].title, title);
        z_append(i);
        focus_win = i;
        window_surfaces_prepare(i);
        wm_lifecycle("open", i, app, wins[i].generation, nz);
        /* A refusal here degrades gracefully: every slot busy means the window
         * opens without a flourish, which is the right way for an animation to
         * fail. */
        if (anim_on) wm_anim(i, ANIM_OPEN);   /* Settings can turn this off */
        wm_damage_win(i);
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_OPEN, i, 0u, (unsigned)app);
        zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_WINDOW, (unsigned)i,
                      ZLLOG_LIFE_START, (unsigned)app,
                      (unsigned)w | ((unsigned)h << 16));
        zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_APP, (unsigned)app,
                      ZLLOG_LIFE_START, (unsigned)i, 0u);
        return i;
    }
    /* WM_MAX is a hard ceiling, and a refusal has to SAY SO. A silent -1 is
     * how "the dock stopped launching things" becomes a twenty-minute hunt -
     * and it is the same bug class as the back buffer switching itself off
     * without a word (desktop-TODO 0a). */
    wm_puts("  wm: no free window slot (WM_MAX reached), refusing to open\n");
    zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                         ZLLOG_OP_WINDOW_OPEN, -1, 28u, (unsigned)app);
    zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_APP, (unsigned)app,
                  ZLLOG_LIFE_REFUSED, 0u, 28u);
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
    unsigned operation_id = zlt_operation_begin(
        ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_KERNEL, 0u,
        ZLLOG_OP_WINDOW_CLOSE, (unsigned)win);
    if (!wm_is_open(win)) {
        zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                             ZLLOG_OP_WINDOW_CLOSE, -1, 9u, (unsigned)win);
        return;
    }
    int app = win_app(win);
    unsigned int generation = wins[win].generation;
    wm_damage_win(win);
    /* A slot is about to become reusable, and wm_open takes the FIRST FREE ONE
     * - so an animation still running on this index would be inherited by
     * whatever opens next and read as its open-scale. */
    anim_cancel(win);
    window_surfaces_free(win);
    wins[win].flags = 0;
    z_remove(win);
    wm_lifecycle("close", win, app, generation, nz);
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
    zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_WINDOW, (unsigned)win,
                  ZLLOG_LIFE_EXIT, (unsigned)app, 0u);
    zlt_lifecycle(ZLLOG_SUB_DISPLAY, ZLLOG_OBJ_APP, (unsigned)app,
                  ZLLOG_LIFE_EXIT, (unsigned)win, 0u);
    zlt_operation_result(ZLLOG_SUB_DISPLAY, operation_id,
                         ZLLOG_OP_WINDOW_CLOSE, 0, 0u, (unsigned)app);
}

void wm_raise(int win)
{
    if (!wm_is_open(win)) return;
    if (wins[win].flags & WF_MINIMIZED) {
        wins[win].flags &= ~WF_MINIMIZED;
        window_surfaces_prepare(win);
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
        window_surfaces_prepare(win);
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
    /* app_draw receives `focused`, and Terminal uses it for its caret.  Until
     * that state becomes a tiny overlay, both clients must be invalidated for
     * correctness. Move/raise still retain content, which is the hot path. */
    if (wm_is_open(old)) { wm_invalidate_shell(old); wm_invalidate_client(old); }
    if (wm_is_open(win)) { wm_invalidate_shell(win); wm_invalidate_client(win); }
    /* BOTH ENDS OF THE FLIP ANIMATE. The one gaining focus rises into the
     * knockout and the one losing it falls back out, so the pair reads as one
     * movement rather than as two independent twitches. The comment above has
     * always said "both title bars change"; now both of them do it over time.
     * The stale half of that comment - "the old loses its HUE and underline" -
     * describes the predecessor's coloured header, not this knockout. */
    if (wm_is_open(old)) wm_anim(old, ANIM_FOCUS);
    if (wm_is_open(win)) wm_anim(win, ANIM_FOCUS);
}

void wm_minimize(int win)
{
    if (!wm_is_open(win) || (wins[win].flags & (WF_MODAL | WF_NOCHROME))) return;
    wm_damage_win(win);
    window_surfaces_free(win);
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
    shell_surface_free(win);
    window_surfaces_prepare(win);
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

/* THE RAIL MUST NEVER BE COVERED, and nothing enforced it.
 *
 * The prototype states this as its rule 1 and clamps in layout():
 *
 *     x = Math.max(0, Math.min(x, Math.max(0, fw - 120)));
 *     y = Math.max(0, Math.min(y, Math.max(0, fh - 60)));
 *
 * where fw/fh are the FIELD - the desk inside the rail, below the strip and
 * above the foot. wm_move assigned x and y verbatim, wm_open stored them
 * verbatim, and route_mouse's GRAB_MOVE passed a raw pointer delta straight
 * through. So a window could be dragged fully over the rail, over the raster
 * strip, over the foot, or to negative coordinates with NO ROUTE BACK - the
 * title bar you would grab to drag it home is the first part to go under.
 *
 * The only reserve anything respected was inside snap_to_rect, so a SNAPPED
 * window sat beside the rail correctly while a dragged one covered it: two
 * behaviours for one edge, and only the unused one written down.
 *
 * 120 and 60 are the prototype's own - enough of the window must stay on the
 * field to grab it by - in design px there and here. */
/* THE INVARIANT IS "A WINDOW ON THE FIELD STAYS ON IT", not "every window is
 * forced onto the field". The difference is not pedantry, it is three failing
 * assertions: wmtest_feel runs ui_theme_init(2), so on its 1280x800 screen the
 * rail alone is 340 px, and the window it opens at x=200 legitimately starts
 * underneath it. A clamp that pulls any window into the field teleports that
 * one on its first drag and breaks the exact-move check - punishing the test
 * for a state the real desktop never reaches, since wm_open there goes through
 * desk_x() which already clears the rail.
 *
 * So the lower bound is min(field edge, where the window already is): a window
 * inside cannot leave, and one that began outside is free to stay where it is
 * or move further in. That is the rule the prototype's layout() is expressing
 * and the one that keeps the rail reachable. */
static void clamp_to_field(int *x, int *y, int cur_x, int cur_y)
{
    const struct ui_theme *t = ui_theme();
    int fx = RESERVE_LEFT(t);
    int fy = RESERVE_TOP(t);
    int fw = (int)fb_pxw() - fx;
    int fh = (int)fb_pxh() - fy - RESERVE_BOT(t);
    int maxx = fx + fw - UI_DP(t, 120);
    int maxy = fy + fh - UI_DP(t, 60);
    if (maxx < fx) maxx = fx;
    if (maxy < fy) maxy = fy;
    if (fx > cur_x) fx = cur_x;
    if (fy > cur_y) fy = cur_y;
    if (maxx < cur_x) maxx = cur_x;
    if (maxy < cur_y) maxy = cur_y;
    if (*x < fx) *x = fx;
    if (*y < fy) *y = fy;
    if (*x > maxx) *x = maxx;
    if (*y > maxy) *y = maxy;
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
    window_surfaces_free(win);
    wins[win].w = w;
    wins[win].h = h;
    window_surfaces_prepare(win);
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

/* ---- PRESSWORK, and what it changed about a window frame -------------------
 *
 * ONE RAKING LIGHT, ENTERING OFF-SCREEN FROM THE UPPER LEFT, AND IT NEVER
 * MOVES. That single premise is the whole depth grammar and it is why almost
 * nothing below draws a shadow: a plate is separated from its ground by a 1px
 * STRUCK run along the top (the side the light hits), a 1px GRAZED run down
 * the left (the side it skims), and a 1px CUT groove along the bottom and
 * right (the side it cannot reach). Depth is lighting, not blur.
 *
 * THE SURFACE LADDER WAS WIDENED SO THAT A KNOCKOUT HAS A RUNG TO LIVE ON.
 * The parent design ran four surface steps all below perceptual threshold and
 * spent its entire separation budget on those 1px runs; here the runs CONFIRM
 * a plate that the ladder already states. The room that bought is spent in one
 * place - the focused window's header inverts to a solid light plate with the
 * title reversed out of it, 6.4796:1 on the plate ground.
 *
 * THREE SIGNALS, THREE QUESTIONS, AND THEY ARE NOT ALLOWED TO BLUR TOGETHER:
 *   the RING   says which plate is ON TOP        (theme.border / theme.edge_over)
 *   the HEADER says which plate has the KEYBOARD (the knockout)
 *   the SHADOW says which plate is OFF THE PLANE (ZD_LIFT, in hand only)
 * The predecessor conflated the first two - a focus wash on the header AND an
 * accent tint on the ring - and spent its focus budget on 1px of hairline.
 *
 * Every contrast figure quoted in this section was computed from design.h's
 * hex values with the WCAG relative-luminance formula during this change; the
 * one place a figure differs from the prototype's own is called out where it
 * appears. Preserved and untouched, because none of this is a rendering
 * change: subpixel LCD text, gamma-correct blending in linear light, dithered
 * gradients, anti-aliased rounded corners and the geometric icon atlases. This
 * file calls fb.c's primitives; it does not change them.
 *
 * LIGHT MODE IS DELIBERATELY OUT OF SCOPE. The prototype carries a second
 * ladder whose own verifier reports it unfinished - on paper the struck run
 * computes 1.244:1 on the ground, so the headroom this design depends on does
 * not exist there. Where the geometry below would mirror in light, it does not
 * get a branch: zlOS ships the dark ladder only.
 */

/* How wide the three-control cluster is, together. The title's safe margin and
 * the tab strip's available width both need it, and two copies of `3 * 22` is
 * how a title ends up running under a close box at one UI scale only. */
/* THE CLUSTER'S INSIDE FACE, PUBLISHED ONCE. Three places need it: the rect of
 * each control, and the two hard stops that keep the title run and the module
 * code clear of it. When the cluster moved left by .hdr's 6dp right padding,
 * only the first of the three moved - so the intended 8dp gutter between the
 * title and the controls silently became 2dp, and the comment above each stop
 * went on describing the old arithmetic. Three copies of one edge is three
 * chances to update two of them. */
static int title_controls_w(const struct ui_theme *t);
static int title_cluster_x(const struct win *W, const struct ui_theme *t)
{ return W->x + W->w - 1 - UI_DP(t, ZD_HDR_PR) - title_controls_w(t); }

static int title_controls_w(const struct ui_theme *t)
{
    return 3 * UI_DP(t, ZD_WINCTL);
}

/* ---- THE MODULE READOUT ----------------------------------------------------
 *
 * "M0101 6x4" in the right of every title bar, and it is not decoration: it is
 * WHERE THIS PLATE IS SITTING ON THE RULED MODULE GRID - column, row, and how
 * many modules of each it covers. PRESSWORK rules a 12x8 grid onto the desk
 * and then floats windows over it, so the grid EXPLAINS rather than tiles; the
 * readout is what turns that explanation into something you can read off a
 * window instead of by squinting at the rules behind it.
 *
 * WHO KNOWS WHAT. wm.c knows the geometry - it owns every window rectangle -
 * and design.h knows the grid's shape (ZD_GRID_COLS/ROWS/MARGIN/GUTTER). The
 * one thing neither knows is where the FIELD begins, because that is the
 * shell's furniture: the register rail down the left, the raster strip across
 * the top, the printer's slug along the foot. So the field is the one number
 * that comes in from outside, through wm_set_field().
 *
 * AND WHEN NOBODY HAS SET IT, THE FALLBACK IS DERIVED, NOT INVENTED. The theme
 * already carries rail_w / strip_h / foot_h - they are the same three metrics
 * the shell lays its own bands out from - so the field is computed from them
 * and the readout is a true statement about a grid ruled at those reserves. It
 * differs from kernel.zl's own field by that shell's 1dp hairlines above and
 * below the strip, which is under a pixel of a 120px module row and cannot
 * change which module a window lands on except exactly on a boundary. It is
 * still a derivation rather than a measurement, which is why the setter
 * exists; the shell should call it. */
static int mod_fx, mod_fy, mod_fw, mod_fh;
static int mod_field_set;

void wm_set_field(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) { mod_field_set = 0; return; }
    if (mod_field_set && mod_fx == x && mod_fy == y &&
        mod_fw == w && mod_fh == h) return;
    mod_fx = x; mod_fy = y; mod_fw = w; mod_fh = h;
    mod_field_set = 1;
    /* every window's code may have changed, and a retained shell that is not
     * told keeps printing the old module */
    for (int i = 0; i < WM_MAX; i++)
        if (wins[i].flags & WF_OPEN) wm_invalidate_shell(i);
}

static void module_field(int *x, int *y, int *w, int *h)
{
    if (mod_field_set) { *x = mod_fx; *y = mod_fy; *w = mod_fw; *h = mod_fh; return; }
    const struct ui_theme *t = ui_theme();
    *x = t->rail_w;
    *y = t->strip_h;
    *w = (int)fb_pxw() - t->rail_w;
    *h = (int)fb_pxh() - t->strip_h - t->foot_h;
}

/* Two digits, always - "M0101", never "M11". The code is a coordinate pair
 * read at a glance and a variable-width one stops being scannable down a
 * column of title bars. */
static int mod_put2(char *b, int n)
{
    if (n < 0) n = 0;
    if (n > 99) n = 99;
    b[0] = (char)('0' + n / 10);
    b[1] = (char)('0' + n % 10);
    return 2;
}

/* THE SPAN'S RULE IS "HALF A MODULE COUNTS", and it is the prototype's own:
 * a plate's far edge claims a module once it has crossed that module's
 * midpoint. The near edge is the opposite - it belongs to a module from one
 * gutter before that module starts - so a window nudged into the gutter still
 * reads as being on the module it is next to rather than on the one behind it.
 * Both are asymmetric on purpose; a symmetric rule makes every window that is
 * a few pixels out read one module too wide.
 *
 * THE REMAINDER IS SPREAD, one pixel each onto the first `rem` columns, which
 * is what kernel.zl's draw_grid() does and what its legend prints as "133/132".
 * Walking the columns rather than dividing is how this stays in step with the
 * rules that are actually drawn: a divide would put a window one module out at
 * the right-hand end of every resolution whose width does not divide by 12.
 * Integer throughout - `w/2` is the midpoint, and a half-pixel does not decide
 * a module. */
static void module_code(int win, char *buf, int cap)
{
    buf[0] = 0;
    if (cap < 12) return;
    int fx, fy, fw, fh;
    module_field(&fx, &fy, &fw, &fh);
    const struct ui_theme *t = ui_theme();
    int mar = UI_DP(t, ZD_GRID_MARGIN), gut = UI_DP(t, ZD_GRID_GUTTER);
    int inw = fw - 2 * mar, inh = fh - 2 * mar;
    if (inw <= 0 || inh <= 0) return;
    int cw  = (inw - gut * (ZD_GRID_COLS - 1)) / ZD_GRID_COLS;
    int cwr = (inw - gut * (ZD_GRID_COLS - 1)) - cw * ZD_GRID_COLS;
    int rh  = (inh - gut * (ZD_GRID_ROWS - 1)) / ZD_GRID_ROWS;
    int rhr = (inh - gut * (ZD_GRID_ROWS - 1)) - rh * ZD_GRID_ROWS;
    if (cw < 1 || rh < 1) return;

    /* the plate, in FIELD space - the grid is ruled inside the field, not on
     * the screen, and a window's rectangle is in screen coordinates */
    int wx = wins[win].x - fx, wy = wins[win].y - fy;
    int wx1 = wx + wins[win].w, wy1 = wy + wins[win].h;

    int c0 = 0, c1 = 0, r0 = 0, r1 = 0;
    int cx = mar;
    for (int i = 0; i < ZD_GRID_COLS; i++) {
        int w = cw + (i < cwr ? 1 : 0);
        if (wx  >= cx - gut)    c0 = i;
        if (wx1 >= cx + w / 2)  c1 = i;
        cx += w + gut;
    }
    int ry = mar;
    for (int i = 0; i < ZD_GRID_ROWS; i++) {
        int h = rh + (i < rhr ? 1 : 0);
        if (wy  >= ry - gut)    r0 = i;
        if (wy1 >= ry + h / 2)  r1 = i;
        ry += h + gut;
    }
    int cspan = c1 - c0 + 1, rspan = r1 - r0 + 1;
    if (cspan < 1) cspan = 1;
    if (rspan < 1) rspan = 1;
    if (cspan > 99) cspan = 99;
    if (rspan > 99) rspan = 99;

    int n = 0;
    buf[n++] = 'M';
    n += mod_put2(buf + n, c0 + 1);
    n += mod_put2(buf + n, r0 + 1);
    buf[n++] = ' ';
    if (cspan >= 10) buf[n++] = (char)('0' + cspan / 10);
    buf[n++] = (char)('0' + cspan % 10);
    buf[n++] = 'x';
    if (rspan >= 10) buf[n++] = (char)('0' + rspan / 10);
    buf[n++] = (char)('0' + rspan % 10);
    buf[n] = 0;
}

/* THE CONTROL CLUSTER, AND IT HAS NO FACE AT REST. Three bare glyphs flush to
 * the right end of the header, each ZD_WINCTL wide and the full height of the
 * header band, each with a 1px groove down its left - the prototype's .ctl /
 * .cbtn rules, which are a printed sheet's corner apparatus rather than three
 * floating pills. They are BUTTED, with no gap, on purpose: a gap makes each
 * one an object, and butted they read as one component with three cells, which
 * is what they are. The old rects were 26dp squares with 6dp gaps, vertically
 * centred in a 36dp bar and painted as filled capsules at rest.
 *
 * The band stops one pixel above the header's foot groove, so a control can
 * never paint over the rule that separates the header from the plate, and one
 * pixel below the top run for the same reason. */
static void title_control_rect(const struct win *W, int which,
                               int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    int cw = UI_DP(t, ZD_WINCTL);
    *w = cw;
    /* THE FULL CONTENT BOX. With .hdr border-box its content is title_h - 1
     * (28dp total, 1px border-bottom), and `.ctl { align-self: stretch }` with
     * .cbtn stretching inside it makes each cell that whole height. This was
     * title_h - 2, left over from when the header was laid out from the ring
     * row, so the cells were a row short of the band they sit in and the hover
     * plate stopped one pixel above the groove. */
    *h = t->title_h - 1;
    if (*h < 1) *h = 1;
    /* which == TITLE_CLOSE is the rightmost cell, and the cluster grows
     * leftward from the header's CONTENT-box right edge - which is the plate's
     * padding-box right minus .hdr's 6dp right padding, not the inside face of
     * the ring. Growing from the ring itself was the old behaviour and it left
     * the close box touching the frame while the title kept an 11dp margin on
     * the other side. Both margins are now the same document's numbers. */
    /* TITLE_MINIMIZE == 2 is the LEFTMOST cell, so it sits at the cluster's
     * inside face and the others step right from it. Byte-identical to the
     * arithmetic this replaces; it just goes through the one owner now. */
    *x = title_cluster_x(W, t) + (TITLE_MINIMIZE - which) * cw;
    *y = W->y + 1;
}

/* IS THIS PLATE ON TOP OF ANOTHER ONE? The occluder draws the light boundary
 * and the occluded never does, so the question is only ever asked from above:
 * is anything VISIBLE below me in the z-order and under my footprint.
 *
 * A darker boundary cannot answer it. ZD_CUT on the plate is 1.4723:1, well
 * under the 3:1 floor a "which of these two is in front" edge has to clear, so
 * under overlap the ring goes LIGHTER instead - theme.edge_over, 4.9991:1 on
 * ZD_BASE and 3.4322:1 in its worst case on ZD_FLOAT.
 *
 * Walks the z-order from the back as far as this window's own slot. WM_MAX is
 * small and this is asked once per shell rebuild, not once per pixel - it is
 * part of shell_state_key below, so a neighbour moving out from under this
 * window invalidates the cached shell rather than leaving a stale ring. */
static int win_over_below(int win)
{
    if (win < 0 || win >= WM_MAX || !wm_is_open(win) || !win_visible(win))
        return 0;
    for (int i = 0; i < nz; i++) {
        int o = zorder[i];
        if (o == win) return 0;              /* reached myself: nothing below */
        if (!wm_is_open(o) || !win_visible(o)) continue;
        int ox, oy, ow, oh;
        if (isect(wins[win].x, wins[win].y,
                  wins[win].x + wins[win].w, wins[win].y + wins[win].h,
                  wins[o].x, wins[o].y,
                  wins[o].x + wins[o].w, wins[o].y + wins[o].h,
                  &ox, &oy, &ow, &oh))
            return 1;
    }
    return 0;
}

/* Is this plate IN HAND right now - being dragged or resized. Declared here
 * and defined down beside the pointer grab, because the grab is routing state
 * and this is a read of it rather than a second copy of it. */
static int win_lifted(int win);

/* NEVER A DROP SHADOW AT REST.
 *
 * This used to encode an elevation ladder - a modal at 1.5x, a focused window
 * at 1x, an unfocused one at about half - and every window on screen wore one
 * of them permanently. PRESSWORK does not have that ladder to retune: depth
 * comes from the lamp, and a plate lying on the desk casts nothing. So the
 * three sizes collapse to two states, and the common one draws no shadow at
 * all.
 *
 * ZD_LIFT survives for the case where an object is GENUINELY off the plane:
 *   - a MODAL, which is one of the three off-plane objects design.h names
 *     (the menu, the modal and the toast), and
 *   - a plate IN HAND, being dragged or resized this instant. The prototype
 *     spells that one .win.drag, and it is the only shadow an ordinary window
 *     ever casts.
 * Opening, closing, focused, unfocused and settled all draw nothing.
 *
 * shadow_reach() above still returns the LIFTED reach for every window, so
 * grabbing and dropping a plate cannot under-damage the band the shadow
 * occupies on either side of the transition. */
static void chrome_shadow(int win, int focused)
{
    const struct ui_theme *t = ui_theme();
    struct win Wa = wins[win];
    struct win *W = &Wa;
    int modal = (W->flags & WF_MODAL) != 0;
    (void)focused;                 /* focus is the knockout, not an elevation */
    if (!modal && !win_lifted(win)) return;
    anim_rect(win, &W->x, &W->y, &W->w, &W->h);
    int off  = SHADOW_OFF(t), soft = SHADOW_SOFT(t);
    /* A MODAL IS OFF-PLANE; A DRAGGED PLATE IS NOT. Both cast, and the
     * authority gives them different pairs - not the same pair times a
     * number. `off * 3 / 2` read as a deliberate elevation and was arithmetic
     * on a figure that belongs to the other object. */
    if (modal) { off = OFFPLANE_OFF(t); soft = OFFPLANE_SOFT(t); }

    fb_shadow(W->x, W->y, W->w, W->h, off, soft);
}

/* THE SEAT. Four calls, and they are the prototype's own fb_seat() written out
 * - it was authored as an implementation note for this function.
 *
 *   1  the RING     the plate's boundary, 1px on all four sides. theme.border
 *                   normally; theme.edge_over when this plate is on top of
 *                   another one. FOCUS DOES NOT TOUCH IT.
 *   2  the PLATE    one pixel inside, radius one pixel tighter so the groove
 *                   follows the curve instead of cutting across it.
 *   3  the TOP RUN  1px struck, along the ring's own top row, inset by the
 *                   radius at both ends so it stops where the arc starts.
 *   4  the LEFT RUN 1px grazed, down the ring's own left column.
 *
 * The two runs sit ON the ring rather than inside it, which is what the CSS
 * does too (.trun/.lrun are absolutely positioned at the border box's edge).
 *
 * ON A FOCUSED PLATE THE TOP RUN IS DRAWN theme.knock AND MERGES. It is not
 * skipped and there is no second code path: same call, same site, one ternary.
 * The surface under it is the knockout at L* 72.19 and the lamp cannot lighten
 * that further, so honestly drawn the struck run is the same value as the
 * thing it lies on. (theme.lit on the knockout would be 2.5487:1 the WRONG
 * WAY - a dark line where the light is supposed to be hitting.)
 *
 * The left run then starts BELOW the header instead of at the corner arc,
 * because on a focused plate the header is a different material and the run
 * belongs to the plate. Its value does not change; only its y0 does. */
static void chrome_seat(const struct win *W, int r, int focused, int over)
{
    const struct ui_theme *t = ui_theme();
    int ri = r > 0 ? r - 1 : 0;
    int chrome = !(W->flags & WF_NOCHROME);

    fb_rrect(W->x, W->y, W->w, W->h, r, over ? t->edge_over : t->border);
    fb_rrect(W->x + 1, W->y + 1, W->w - 2, W->h - 2, ri, t->panel);

    /* THE RUNS LIE ON THE PANEL, ONE PIXEL INSIDE THE RING - not on it.
     *
     * They were painted at row W->y and column W->x, straight over the ring
     * drawn two lines above, so the plate lost its boundary along its whole top
     * edge and its whole left edge. The comment that justified it claimed
     * ".trun/.lrun are absolutely positioned at the border box's edge". They are
     * not: `.win` carries `border: 1px solid var(--zd-cut)` (proto:601) and an
     * absolutely positioned box is laid out against its containing block's
     * PADDING edge (CSS 2.1 10.1), so `.trun{top:0}` and `.lrun{left:0}`
     * (proto:607,610) sit one pixel inside the border, on the panel.
     *
     * That is also the only reading that makes sense of what they are FOR. A
     * struck highlight is light catching the top of a raised surface; painting
     * it over the surface's own edge deletes the edge and leaves the highlight
     * floating with nothing to be the top of. */
    int run_w = W->w - 2 * r;
    if (run_w > 0)
        fb_fill_px(W->x + r, W->y + 1, run_w, 1, focused ? t->knock : t->lit);

    int ly = W->y + 1 + ((focused && chrome) ? t->title_h : r);
    int lh = (W->y + W->h - 1 - r) - ly;
    if (lh > 0) fb_fill_px(W->x + 1, ly, 1, lh, t->litsoft);
}

/* THE KNOCKOUT, and it is the entire focus signal.
 *
 * The focused window's header fills solid theme.knock and the title reverses
 * out of it in theme.knock_ink. There is no ring tint, no glow and no wash -
 * the accent-blend that used to sit over the whole frame here is deleted
 * rather than reduced, because it was answering the ring's question with the
 * header's answer. Computed here from design.h: the knockout is 6.4796:1
 * against the plate, 46.61% of the 13.9030:1 the plate has upward. The
 * comparison figure for what it replaces - 1.3999:1 for the predecessor's
 * focus wash, on an identical pixel count - is the PROTOTYPE'S number and is
 * quoted as such: that token is not in this ladder and I did not compute it.
 *
 * ONE FILL AND ONE RULE, AND THE RULE IS ON THE SIDE THE HEADROOM IS ON. From
 * the plate there is 1.5105:1 of room downward and 13.9030:1 upward, so an
 * unfocused header separates from its plate with the groove BELOW it. From the
 * knockout the room runs the other way, so the focused header's foot takes
 * theme.ko_edge, which is theme.lit's own value used as a groove: 2.5487:1
 * down off the knockout against 2.5423:1 up off the plate, the same loudness
 * within a quarter of a percent, mirrored.
 *
 * The header is 28dp tall, down from 36, and that is what makes a solid light
 * plate on every focused window affordable rather than overwhelming. The fill
 * is rounded at the top to the plate's inner radius - a square band inside a
 * radius-9 plate reads as two shapes that do not fit each other, and at this
 * value it would also be a light square poking out of a dark corner. */
static void chrome_header(const struct win *W, int win, int r, int focused)
{
    const struct ui_theme *t = ui_theme();
    int ix = W->x + 1, iw = W->w - 2;

    /* THE HEADER IS CLAMPED TO THE WINDOW, NOT TO THE THEME. It used to take
     * t->title_h unconditionally, which is only the same number while the
     * window is at least that tall - and nothing guarantees that. wm_open()
     * stores the caller's h verbatim and only sets min_h afterwards as a
     * RESIZE floor, so a caller may open a chrome window shorter than the
     * title bar. At h < title_h the band and its foot rule both landed below
     * W->y + W->h.
     *
     * That is a write outside the frame, not a cosmetic overrun: fb_fill_px
     * and fb_rrect_grad_top do not clip on their own, and neither surrounding
     * path saves us. The retained-shell path enters through
     * fb_surface_begin_alpha(), which sets the scissor to the whole shell
     * surface, and the direct path clips to frame-plus-shadow - so the stray
     * rows land in the shadow band or on the neighbour beneath it.
     *
     * hh is the header height this window can actually afford. Everything
     * below is derived from it rather than from the theme. */
    /* BORDER-BOX. The prototype sets `* { box-sizing: border-box }` (proto:325)
     * and .hdr is `height: var(--zd-title-h)` with `border-bottom: 1px`
     * (proto:653,657) - so title_h is the TOTAL: 27 rows of ground plus the
     * groove. As a flex child of .win it starts at the padding edge, row y+1.
     *
     * This treated row y as belonging to the header, so the fill was one row
     * short and the groove one row high. The comment rationalised it as "row y
     * is the merged top run above", which is the same mistake the two runs make
     * one function over: the runs are inside the padding box too. */
    int hh = t->title_h;
    if (hh > W->h - 2) hh = W->h - 2;
    int foot = W->y + hh;
    if (iw <= 0 || hh < 2) return;
    /* THE BAND IS INTERPOLATED WHILE ANIM_FOCUS RUNS.
     *
     * A focus change used to be one frame: the header went from the plate's own
     * ground to the full knockout in a single step. design.h calls that the
     * refused option in as many words - "A CUT (0ms) was offered and REFUSED:
     * the knockout is a full value flip and cutting it makes the whole screen
     * twitch."
     *
     * Both directions are handled here, which is why the unfocused arm draws at
     * all now: a window LOSING focus interpolates from knock back to panel, and
     * without this it would snap out while the other rose in. */
    /* THE INDEX IS PASSED, BECAUSE W IS NOT IN wins[].
     *
     * This read `anim_progress((int)(W - wins), ANIM_FOCUS)` under a comment
     * asserting "W points into wins[], so its index is the offset". It does
     * not: chrome_shell takes a local copy first - `struct win Wa = wins[win];
     * struct win *W = &Wa;` - and passes &Wa, a stack object. The difference
     * between a stack address and a static array is not an index, it was never
     * in [0, WM_MAX), anim_progress never matched, and the focus animation this
     * commit's own message called its headline feature HAS NEVER RUN. A comment
     * asserting a fact about memory layout, wrong, in the file that keeps
     * catching exactly that. */
    int fp = anim_progress(win, ANIM_FOCUS);
    if (focused || fp >= 0) {
        int ri = r > 0 ? r - 1 : 0;
        unsigned band = t->knock;
        if (fp >= 0) {
            int mix = focused ? fp : 1000 - fp;
            band = blend_rgb(t->panel, t->knock, mix);
        } else if (!focused) {
            band = t->panel;
        }
        /* rows y+1 .. y+hh-1, with the groove at y+hh: border-box, so the
         * ground is hh-1 rows and the 1px border completes the declared hh. */
        fb_rrect_grad_top(ix, W->y + 1, iw, hh - 1, ri, band, band);
    }
    fb_fill_px(ix, foot, iw, 1, focused ? t->ko_edge : t->border);
}

/* THE VERMILION FOCUS BAR. Overprint job 1: 3dp down the focused plate's left
 * edge, starting at the FOOT of the knockout and running to the top of the
 * bottom arc.
 *
 * IT DOES NOT CROSS THE HEADER, and that is measured rather than stylistic.
 * The overprint on the knockout computes 1.3989:1 here - the prototype reports
 * 1.4014:1 for the same pair - so a vermilion bar laid across the knockout
 * would be 3dp of an accent carrying no information at all. On the plate it is
 * 4.6319:1 and it reads. That constraint is the whole reason the bar starts
 * where it does, and it is the same one that makes the command palette's
 * selected row take theme.ko_edge for its mark instead of the overprint.
 *
 * It stops at the arc rather than at the frame: the bottom-left corner is
 * anti-aliased and a square bar run into it would eat the curve. The prototype
 * gets this free from overflow:hidden; here it is one subtraction. */
static void chrome_focus_bar(const struct win *W, int r, int focused)
{
    const struct ui_theme *t = ui_theme();
    if (!focused || (W->flags & WF_NOCHROME)) return;
    int bw = t->focus_bar, by = W->y + t->title_h;
    int bh = W->h - t->title_h - 1 - r;
    if (bw > W->w - 2) bw = W->w - 2;
    if (bw > 0 && bh > 0) fb_fill_px(W->x + 1, by, bw, bh, t->accent);
}

/* ---- THE TITLE LINE --------------------------------------------------------
 *
 * "01 TERMINAL  zlsh" flush LEFT, and a module code out to the right.
 *
 * IT WAS CENTRED, AND CENTRING IT WAS THE ONE DECISION THAT MADE THE HEADER
 * DECORATIVE. A centred title has no fixed edge, so it cannot be scanned down
 * a stack of windows, it cannot be followed by a second field, and its
 * position encodes the length of the string rather than anything about the
 * window. PRESSWORK's header is a printed document's running head: a fixed
 * left margin, fields in a fixed order, and figures out at the right. Four
 * cells, and each one answers a different question:
 *
 *   REG    which register slot this window is         mono, 2 digits, dim
 *   TITLE  what it is                                 MD bold, UPPERCASED
 *   SUB    which instance of it                       mono, dim
 *   CRD    where it is on the module grid             mono, dim, out right
 *
 * TWO OF THE FOUR TYPE PROPERTIES ARE NOT AVAILABLE AND ARE NOT FAKED. The
 * label style is SM/uppercase/bold/tracked; there are three baked atlases and
 * no rasteriser, so there is no tracking, and uikit.c already priced that same
 * item the same way rather than inserting spaces (which would break every
 * width measurement). The name is drawn at MD rather than SM because at MD it
 * is the header's one piece of running text and SM bold beside a mono figure
 * of the same nominal size reads as a second figure. Uppercasing IS done, here
 * rather than at the caller, because wm.c owns the copy and a shell that
 * shouted its own strings would be shouting them in the taskbar too.
 *
 * WHERE THE CRD SITS IS MEASURED OFF THE PROTOTYPE'S PIXELS, NOT OFF ITS
 * PROSE. The brief for this change says the code is right-aligned. It is not:
 * `.crd` and `.ctl` BOTH carry `margin-left:auto`, so flexbox splits the free
 * space equally between them and the code lands at the midpoint of the gap
 * between the subtitle and the control cluster. Measured in the rendered
 * reference at 1920x1200: window 02's subtitle ends at x=1157, its controls
 * begin at x=1829, and its code's ink runs 1467..1525 - centre 1496 against a
 * midpoint of 1493. Three pixels. That is a centred field, and it is what is
 * built here.
 *
 * THE TITLE IS TRUNCATED RATHER THAN ELLIPSISED when the run will not fit.
 * The prototype gets `text-overflow: ellipsis` free from the browser; here an
 * ellipsis would have to be measured back out of the string, and a hard cut
 * at a known x is both cheaper and unambiguous about where the field ends.
 * Nothing may cross into the control cluster: the retained-shell path sets the
 * scissor to the whole shell surface, so an overlong title is not clipped by
 * anything and would be drawn straight over the close box.
 *
 * ONE FUNCTION, MEASURED OR DRAWN, AND `draw` IS THE ONLY DIFFERENCE. It
 * returns the x just past the run, which is where the module code's field
 * begins - and the code is NOT drawn from inside the retained shell (see
 * chrome_module below), so something has to be able to ask for that x without
 * painting. Two functions would be two copies of the same six advances, and
 * the first time one of them gained a field the code would start landing in
 * the wrong place on windows with a subtitle only. */
static int chrome_title_run(const struct win *W, int focused, int hh, int draw)
{
    const struct ui_theme *t = ui_theme();
    /* the prototype's `.hdr` is 11dp of padding inside a 1px ring, which is
     * 12dp from the plate's outer edge - one step of the spacing scale, and
     * the same left margin the module grid's own columns use */
    /* ZD_HDR_PL, .hdr's own padding-left. This was UI_S3, a generic 12dp
     * spacing step that happened to sit next to the right answer without being
     * it - the prototype says 11dp (proto:655) and the header is one pixel per
     * ui-unit further in than the document it is copying. An uncited constant
     * that is nearly right is the hardest kind to notice, because nothing looks
     * broken; it just is not the same drawing. */
    int x = W->x + 1 + UI_DP(t, ZD_HDR_PL);
    int gut = UI_DP(t, ZD_GAP);
    /* the cluster's inside face, less one gutter, is the hard stop for
     * everything on this line */
    int stop = title_cluster_x(W, t) - gut;

    unsigned ink_dim = focused ? t->knock_ink2 : t->text_dim;
    /* ZD_TITLE_INK. The name is ZD_TEXT_2 at rest - 7.8606:1 on the plate -
     * and everything secondary beside it is ZD_TEXT_3 at 6.6809:1, which is
     * the whole reason the name reads as the name. This drew the title at
     * text_dim, i.e. at the same value as its own qualifiers. */
    unsigned ink_ttl = focused ? t->knock_ink : t->text_2;

    int cy_mono = W->y + (hh - fb_cell_h()) / 2;
    int cy_ttl  = W->y + (hh - ui_text_h(UI_MD)) / 2;
    int cw = fb_cell_w();

    if (W->reg > 0) {
        char r[3];
        r[0] = (char)('0' + W->reg / 10);
        r[1] = (char)('0' + W->reg % 10);
        r[2] = 0;
        if (draw && x + 2 * cw <= stop) fb_text_aa(x, cy_mono, r, ink_dim);
        x += 2 * cw + gut;
    }

    /* UPPERCASED INTO A LOCAL COPY. wins[].title is what the taskbar, the
     * overview and the lifecycle log all read, and none of them shouts. */
    char up[32];
    int n = 0;
    for (; W->title[n] && n < (int)sizeof up - 1; n++) {
        char c = W->title[n];
        up[n] = (c >= 'a' && c <= 'z') ? (char)(c - 32) : c;
    }
    up[n] = 0;
    /* THE TITLE IS TRACKED, and `.hdr .ttl` is the third selector in the
     * prototype that carries `letter-spacing: var(--tr-lab)` - the other two
     * being `th` and `.kv .k`, which uikit.c already tracks. It is the ONE
     * tracked run at MD rather than SM, which is why ui_caps takes a size.
     * The uppercasing above stays where it is rather than moving to
     * UI_F_CAPS: this copy is also what the hard cut below measures, and a
     * caller that truncates a string has to hold the transformed one anyway.
     *
     * The cut, and it is the reason the tracked pair had to be published
     * rather than left file-local in uikit.c: characters are dropped until
     * the run fits, so the MEASURE runs once per dropped character and the
     * draw runs once - if those two used different advances a long title
     * would be cut to fit one width and then drawn at another. They are the
     * same loop with the ink switched off; see ui.h. */
    while (n > 0 && x + ui_caps_w(up, UI_MD) > stop) up[--n] = 0;
    if (n > 0) {
        if (draw) ui_caps(x, cy_ttl, up, ink_ttl, UI_MD);
        x += ui_caps_w(up, UI_MD) + gut;
    }

    if (W->sub[0]) {
        int sw = 0;
        while (W->sub[sw]) sw++;
        if (x + sw * cw <= stop) {
            if (draw) fb_text_aa(x, cy_mono, W->sub, ink_dim);
            x += sw * cw + gut;
        }
    }
    return x;
}

/* THE MODULE CODE, AND IT IS THE ONE PIECE OF CHROME THAT CANNOT LIVE IN THE
 * RETAINED SHELL SURFACE.
 *
 * Everything else a window frame draws is a function of the window's SIZE and
 * its state; the cached shell layer is therefore position-independent and a
 * move is a blit at a new offset with no chrome primitive rerun at all. wmtest
 * asserts exactly that - "a move that keeps the overlap causes zero
 * shell/shadow rebuilds" - and the assertion is protecting the most expensive
 * call in a window redraw, fb_shadow, which is inside that same surface.
 *
 * The module code is the first thing in the frame that is a function of WHERE
 * THE WINDOW IS. Folding it into shell_state_key() makes it correct and makes
 * every frame of every drag rebuild the whole shell plus its shadow - a
 * position readout paid for with the compositor's headline optimisation.
 *
 * So it is drawn in the direct pass instead, immediately after the shell is
 * composited, under the same scissor. It costs one mono run per repaint of a
 * window whose header is in the damage, it follows the window with no cache to
 * invalidate, and the two paths through wm_repaint - cached and direct - stay
 * identical because neither of them draws it inside chrome_shell.
 *
 * IT IS CENTRED IN WHAT THE TITLE RUN LEFT OVER, which for a TABBED window is
 * the space after the last tab rather than after a title: a tab strip replaces
 * the title, and the code is a property of the window rather than of the tab,
 * so it stays. */
static void chrome_module(int win, int focused)
{
    const struct ui_theme *t = ui_theme();
    struct win Wa = wins[win];
    struct win *W = &Wa;
    if (W->flags & WF_NOCHROME) return;
    anim_rect(win, &W->x, &W->y, &W->w, &W->h);
    int hh = t->title_h;
    if (hh > W->h) hh = W->h;
    if (hh < 2) return;

    int gut = UI_DP(t, ZD_GAP);
    int x;
    if (W->ntab > 1) {
        int tx, ty, tw, th;
        tab_rect(win, W->ntab - 1, &tx, &ty, &tw, &th);
        x = tx + (W->x - wins[win].x) + tw + gut;
    } else {
        x = chrome_title_run(W, focused, hh, 0);
    }
    int stop = title_cluster_x(W, t) - gut;

    char code[16];
    module_code(win, code, (int)sizeof code);
    if (!code[0]) return;
    int kn = 0;
    while (code[kn]) kn++;
    int kw = kn * fb_cell_w();
    int room = stop - x;
    if (kw > room) return;                 /* no honest place to put it */
    fb_text_aa(x + (room - kw) / 2, W->y + (hh - fb_cell_h()) / 2, code,
               focused ? t->knock_ink2 : t->text_dim);
}

/* ---- THE FOOT BAND ---------------------------------------------------------
 *
 * "01  tty1 - 80x24   APP US 995 us                            ws 01", the
 * band across the bottom of every window. Real zlOS windows had none at all.
 *
 * IT IS CHROME, NOT CONTENT, AND THAT IS WHY IT IS DRAWN HERE. The prototype
 * builds it inside each app's renderer, which is fine in a document where
 * every window is written by the same hand; here it would mean 53 apps each
 * remembering to draw a band, drawing it slightly differently, and being able
 * to draw anything they liked in it. Drawn as chrome it is uniform by
 * construction, and client_of() takes the space away from the app so an app
 * physically cannot paint over it - the same guarantee the title bar already
 * had, extended to the other end of the window.
 *
 * FOUR CELLS, AND ONLY TWO OF THEM ARE FIGURES wm.c MEASURED:
 *   reg      the register slot   from wm_set_label, dim
 *   status   the app's own line  from wm_set_status, dim
 *   APP US   what the last app_draw for this window actually cost, in
 *            microseconds off the same TSC the frame timer uses. The label is
 *            dim and the VALUE is theme.text_hi, because a measured figure is
 *            the one thing on this band that changes.
 *   ws NN    the workspace, flush right. wm.c owns workspaces outright.
 *
 * THE BAND IS THE BODY'S COLUMN, RULE AND ALL. In the prototype `.sband` is a
 * child of `.wbody`, so its border-top and its first character both begin at
 * the BODY's content edge rather than at the plate's, and end at it on the
 * right. Measured in the reference at 1920x1200: window 01's plate ring is at
 * x=182 and both its band rule and its first glyph's ink start at x=192; at
 * the other end window 05's plate ring is at x=1905 and its "ws 01" ink ends
 * at x=1893. Ten pixels in on the left, twelve on the right for a five-glyph
 * mono run whose last advance box is wider than its ink - one 9dp body inset
 * on each side, plus the vermilion bar on the left when there is one.
 *
 * So the band's own left edge is client_of()'s left inset plus ZD_PAD, which
 * puts its first character in the app's own first column, and the bar stays
 * unbroken down the left because the rule starts to the right of it.
 *
 * THE TEXT STOPS SHORT OF THE RESIZE GRIP; THE RULE DOES NOT. The grip is
 * UI_S3 square in the bottom-right corner and the band is taller than that, so
 * the two occupy the same rows - the workspace figure would be drawn through
 * three diagonal rules. The prototype has the same overlap and resolves it the
 * same way: the border runs the full width and the grip is drawn on top of it,
 * because a 1px groove crossed by a grip still reads as a groove and a digit
 * crossed by one does not.
 *
 * A 1px ZD_CUT rule along the top, `.sband`'s `border-top`, at 1.4723:1 on the
 * plate - a groove you feel rather than a line you read, which is the same
 * weight everything else in this system separates two bands with. */
static void chrome_band(const struct win *W, int focused, int bh)
{
    const struct ui_theme *t = ui_theme();
    (void)focused;                 /* the band is on the PLATE in both states */
    if (bh <= 1) return;
    /* .sband IS THE LAST CHILD OF .wbody, NOT THE LAST ROW OF THE PLATE.
     * .wbody pads 6dp at the foot (proto:711), so 6dp of ground shows BELOW the
     * band and the band never touches the ring. This put its last row at
     * W->y + W->h - 2, flush against the frame, which reads as a band welded to
     * the plate rather than one sitting inside it.
     *
     * The inset is .wbody's 9dp for the same reason, not ZD_PAD's 10. The
     * comment that used to sit here derived the right number off the rendered
     * reference - "one 9dp body inset on each side" - and then reached for the
     * generic spacing step anyway. A derivation nobody applied is not a
     * measurement, it is a note. */
    int ry = W->y + W->h - 1 - UI_DP(t, ZD_BODY_PY) - bh;
    int pad = UI_DP(t, ZD_BODY_PX);
    int ix = W->x + 1 + t->focus_bar + pad;
    int iw = W->w - 2 - t->focus_bar - 2 * pad;
    if (iw <= 0) return;
    fb_fill_px(ix, ry, iw, 1, t->cut);

    int by = ry + 1, bhh = bh - 1;
    int cw = fb_cell_w();
    int cy = by + (bhh - fb_cell_h()) / 2;
    int gap = UI_DP(t, ZD_STATUS_GAP);
    int x = ix;
    int stop = ix + iw;
    int grip = W->x + W->w - 1 - UI_S3(t);
    if (stop > grip) stop = grip;
    if (stop <= x) return;

    /* the workspace first, because it is the one field with a fixed right
     * edge and everything to its left has to stop before it */
    char ws[8];
    ws[0] = 'w'; ws[1] = 's'; ws[2] = ' ';
    ws[3] = (char)('0' + (W->ws / 10) % 10);
    ws[4] = (char)('0' + W->ws % 10);
    ws[5] = 0;
    int wsw = 5 * cw;
    if (stop - wsw >= x) {
        fb_text_aa(stop - wsw, cy, ws, t->text_dim);
        stop -= wsw + gap;
    }

    if (W->reg > 0) {
        char r[3];
        r[0] = (char)('0' + W->reg / 10);
        r[1] = (char)('0' + W->reg % 10);
        r[2] = 0;
        if (x + 2 * cw > stop) return;
        fb_text_aa(x, cy, r, t->text_dim);
        x += 2 * cw + gap;
    }

    if (W->status[0]) {
        int sw = 0;
        while (W->status[sw]) sw++;
        if (x + sw * cw > stop) return;
        fb_text_aa(x, cy, W->status, t->text_dim);
        x += sw * cw + gap;
    }

    /* APP US, and the label IS the label style now: SM, bold, uppercase and
     * TRACKED. The prototype emits this exact run as
     * `<span class="t-lab">app us</span>` (line 1614) - lower case in the
     * markup, uppercased and spaced by the stylesheet - so it was never
     * meant to be the untracked caption this drew. The note that used to sit
     * here deferred the tracking "for the reason chrome_title gives", and
     * chrome_title tracks now, so there is no reason left. */
    int lw = ui_caps_w("APP US", UI_SM);
    char v[16];
    unsigned u = W->band_us;
    if (u > 999999u) u = 999999u;
    int n = 0;
    char rev[8];
    if (!u) rev[n++] = '0';
    while (u && n < 6) { rev[n++] = (char)('0' + (int)(u % 10u)); u /= 10u; }
    int k = 0;
    while (n > 0) v[k++] = rev[--n];
    v[k++] = ' '; v[k++] = 'u'; v[k++] = 's'; v[k] = 0;
    int vw = k * cw;
    if (x + lw + UI_S1(t) + vw > stop) return;
    ui_caps(x, by + (bhh - ui_text_h(UI_SM)) / 2, "APP US", t->text_dim, UI_SM);
    fb_text_aa(x + lw + UI_S1(t), cy, v, t->text_hi);
}

static void chrome_shell(int win, int focused)
{
    const struct ui_theme *t = ui_theme();
    struct win Wa = wins[win];
    struct win *W = &Wa;
    anim_rect(win, &W->x, &W->y, &W->w, &W->h);
    /* ZD_R_BOLT. A maximised window is BOLTED DOWN: it has no ground to sit on
     * at any edge, so it has no corner to round and its runs go full width and
     * full height. Radius in PRESSWORK encodes how much an object can move. */
    int r = win_maxed(win) ? UI_DP(t, ZD_R_BOLT) : t->radius;

    /* the occlusion edge, switchable: off falls back to the plain ring, which
     * is what the reference's `.win.over` rule does when it is not applied */
    chrome_seat(W, r, focused, ui_over_get() && win_over_below(win));

    if (W->flags & WF_NOCHROME) return;

    chrome_header(W, win, r, focused);
    chrome_focus_bar(W, r, focused);
    chrome_band(W, focused, band_h_of(W->h, W->flags));

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
            /* ZD_TAB_R, WHICH IS ZD_R_BOLT, AND IT MATTERS ON THE KNOCKOUT.
             * This was ZD_R_CHIP. A rounded tab drawn on a knocked-out header
             * leaves the header's own light colour showing in the notch under
             * each bottom corner - four bright specks where the tab meets the
             * client - because the thing behind the curve is now ZD_KNOCK
             * rather than a surface one rung away. design.h already said BOLT
             * for a tab, and it says so because radius encodes how much an
             * object can move: a tab is bolted to its window. Square, it
             * merges into the plate below with no seam at all. */
            if (on) fb_rrect(tx, ty, tw, th + UI_S1(t), UI_DP(t, ZD_TAB_R),
                             t->panel);
            /* INK FOLLOWS THE GROUND UNDER IT, NOT THE FOCUS STATE. The active
             * tab is a theme.panel plate drawn ON the header, so its label is
             * the plate's ink either way. The inactive ones are lying directly
             * on the header, and on a knocked-out header theme.text_dim would
             * be 1.0311:1 - not dim, invisible - so they take knock_ink2 at
             * 4.6965:1 instead. This is the one place the knockout costs a
             * branch rather than a ternary. */
            unsigned tab_ink;
            if (on)             tab_ink = focused ? t->text : t->text_dim;
            else if (focused)   tab_ink = t->knock_ink2;
            else                tab_ink = t->text_dim;
            fb_text_prop(tx + UI_S2(t), ty + (th - fb_text_prop_h()) / 2,
                       wins[win].tab_title[i], tab_ink);
        }
    } else {
        /* THE TITLE LINE, and it is clamped to the header this window can
         * actually afford - the same hh chrome_header() derives, for the same
         * reason. A window shorter than its own title bar would otherwise
         * centre its text below its own foot. */
        int hh = t->title_h;
        if (hh > W->h) hh = W->h;
        if (hh >= 2) (void)chrome_title_run(W, focused, hh, 1);
    }

    /* One window-control component, three actions. Shared geometry keeps the
     * painted buttons and their hit targets identical; atlas icons keep them
     * crisp at every UI scale.
     *
     * NO FACE AT REST. Three capsules the colour of theme.panel_hi used to sit
     * in every title bar whether or not anyone was pointing at them, which put
     * three permanent objects in the one band that has to stay quiet enough
     * for a knockout to be the loudest thing on it. Now the header's own
     * ground shows through and the only mark is a 1px rule down each cell's
     * left - the same groove the rest of the system separates cells with.
     *
     * The glyph atlas is 24dp and the cell is ZD_WINCTL, 22dp, so the icon box
     * overhangs by 1dp on each side. Measured rather than assumed: at scale 2
     * the close glyph's actual ink runs x 10..37 of its 48px box, so it lands
     * 8px inside a 44px cell and nothing crosses a rule. */
    for (int b = TITLE_CLOSE; b <= TITLE_MINIMIZE; b++) {
        int bx, by, bw, bh;
        title_control_rect(W, b, &bx, &by, &bw, &bh);
        int over = ptr_x >= bx && ptr_x < bx + bw && ptr_y >= by && ptr_y < by + bh;
        /* At rest the ink and the rule both come off the ground beneath them.
         * On the knockout theme.text_dim would be 1.0311:1 and theme.border
         * 9.5398:1 - one invisible, the other a black gash across a light
         * plate - so both switch to the knockout's own secondary at 4.6965:1,
         * which is what the prototype's .win.focus .hdr .cbtn rule does. */
        unsigned ink  = focused ? t->knock_ink2 : t->text_dim;
        unsigned rule = focused ? t->knock_ink2 : t->border;
        if (over) {
            /* HOVER FILLS, AND THE CLOSE BOX CANNOT BE RED ON THE KNOCKOUT.
             * On the plate, close takes the overprint with theme.ink_on over
             * it at 6.1400:1 - design.h's ZD_CLOSE_HOVER_BG / _INK - and the
             * other two take theme.panel_hi with theme.text_hi. On a focused
             * header neither works: the overprint on the knockout is 1.3989:1
             * and theme.panel_hi on it is 5.1970:1 the wrong way round, a hole
             * rather than a lift. So the focused header inverts a second time -
             * fill theme.knock_ink, ink theme.knock, 8.5329:1 - and the close
             * box is told apart by position and glyph, which is how a printed
             * control cluster does it anyway. */
            unsigned face;
            if (focused)                 { face = t->knock_ink; ink = t->knock; }
            else if (b == TITLE_CLOSE)   { face = t->accent;    ink = t->ink_on; }
            else                         { face = t->panel_hi;  ink = t->text_hi; }
            fb_fill_px(bx, by, bw, bh, face);
        }
        fb_fill_px(bx, by, 1, bh, rule);
        /* pressed SEATS the cell rather than tinting it toward the desk: the
         * groove is what a pressed thing in this system is made of */
        if (over && (last_btn & 1))
            fb_fill_blend(bx, by, bw, bh, t->cut, 48);
        int glyph = b == TITLE_CLOSE ? 13 :
                    /* win_snapped, not win_maxed: the button RESTORES from
                     * any snap state - wm_toggle_max sends SK_DOWN whenever
                     * snap_state is not SNAP_NONE - so the restore glyph has
                     * to appear for a half-snapped window too. Only the corner
                     * radius cares specifically about SNAP_MAX. */
                    (b == TITLE_MINIMIZE ? 22 : (win_snapped(win) ? 24 : 23));
        /* ZD_WINCTL_GLYPH, not 24. The authority's .cbtn svg is 11x11 in a
         * 22dp cell - the glyph is half the cell's width. At 24dp the box was
         * two dp WIDER than the cell it was being centred in, so the centring
         * term went negative and the glyph overhung on both sides at every
         * scale. The comment that used to defend this measured the INK's extent
         * and concluded it did not cross the cell - which is true and is not
         * the same claim as the glyph being the right size. */
        int gd = UI_DP(t, ZD_WINCTL_GLYPH);
        fb_icon_dp(bx + (bw - gd) / 2, by + (bh - gd) / 2, glyph, ZD_WINCTL_GLYPH, ink);
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
        /* theme.surf_7 is ZD_TEXT_INERT, 2.5213:1 on the plate - STRUCTURE
         * ONLY, and the grip is structure rather than a glyph, which is why it
         * is allowed the rung that must never carry text. It used to take
         * theme.title_off when unfocused; under PRESSWORK title_off IS the
         * plate, so that grip would have been drawn base-on-base and vanished
         * entirely. A remapped token turning a mark invisible is exactly the
         * failure mode a role-based palette is supposed to make findable. */
        unsigned ink = focused ? t->text_dim : t->surf_7;
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

static unsigned int shell_state_key(int win, int focused)
{
    unsigned int key = (unsigned int)(focused ? 1 : 0);
    key |= (unsigned int)(wins[win].flags & (WF_MODAL | WF_NOCHROME)) << 1;
    key ^= (unsigned int)(wins[win].tab & 0x0f) << 8;
    key ^= (unsigned int)(wins[win].ntab & 0x0f) << 12;
    /* Both questions go in the key: the glyph follows win_snapped and the
     * radius follows win_maxed, so a cache keyed on only one of them serves a
     * stale plate whenever the other changes alone - which is exactly what a
     * drag-to-edge does. */
    key ^= (unsigned int)(win_snapped(win) ? 1 : 0) << 16;
    key ^= (unsigned int)(win_maxed(win) ? 1 : 0) << 17;
    int over_any = 0;
    for (int b = TITLE_CLOSE; b <= TITLE_MINIMIZE; b++) {
        int x, y, w, h;
        title_control_rect(&wins[win], b, &x, &y, &w, &h);
        if (ptr_x >= x && ptr_x < x + w && ptr_y >= y && ptr_y < y + h) {
            key ^= 1u << (20 + b);
            over_any = 1;
        }
    }
    if (over_any && (last_btn & 1)) key ^= 1u << 24;
    /* TWO STATES THAT LIVE OUTSIDE THIS WINDOW, and both change what the shell
     * draws, so both have to be in the key or the retained layer goes stale.
     *
     *   over    the ring becomes theme.edge_over, and it depends on where the
     *           NEIGHBOURS are. Move the window underneath out and this
     *           window's own pixels change without this window moving at all.
     *   lifted  a plate in hand casts ZD_LIFT and a resting one casts nothing,
     *           and chrome_shadow is inside this same cached surface.
     *
     * Missing either is the quiet kind of bug: the screen looks nearly right
     * and only disagrees with itself after a particular sequence of moves. */
    /* THE SWITCH IS PART OF THE STATE, not just the predicate. chrome_seat is
     * handed `ui_over_get() && win_over_below(win)`, and this keyed on the
     * predicate alone - so turning the occlusion edge off changed the argument
     * and not the key, the retained shell surface was reused unchanged, and the
     * new Settings switch appeared to do nothing at all.
     *
     * Key on what the drawing function is actually given. A cache key that
     * omits an input is a cache that serves the wrong picture, and it looks
     * exactly like a dead control - which is what this was reported as. */
    key ^= (unsigned int)((ui_over_get() && win_over_below(win)) ? 1 : 0) << 25;
    /* ...and the other two switches, for the same reason: both change how every
     * plate is composited and neither was in the key. */
    key ^= (unsigned int)(ui_motion_get() ? 1 : 0) << 26;
    key ^= (unsigned int)(ui_track_get()  ? 1 : 0) << 27;
    key ^= (unsigned int)(win_lifted(win) ? 1 : 0) << 26;
    /* NOTHING THE FOOT BAND OR THE TITLE LINE PRINTS IS IN THIS KEY, and that
     * is a decision rather than an omission. This key is an ENUMERATION of
     * states - one bit per state, no two states sharing a value - and a
     * band_us of 995 or a status string does not fit in a bit. Hashing them in
     * would turn the key into a digest and give the cache a collision mode it
     * has never had, for values that change on their own schedule anyway. So
     * each of them invalidates the shell where it is written instead:
     *
     *   reg / sub / status   wm_set_label(), wm_set_status()
     *   band_us              band_us_latch(), on its 320 ms cadence
     *   ws                   wm_set_win_ws(), which already damages twice
     *
     * THE MODULE CODE IS NOT HERE EITHER, and for a stronger reason: it is the
     * one thing the frame draws that depends on the window's POSITION, and
     * this surface is position-independent by design. Any mechanism that made
     * it correct here - key bit or invalidation - would rebuild the shell and
     * its shadow on every frame of every drag. It is drawn in the direct pass
     * instead; see chrome_module(). */
    return key;
}

/* THE BAND'S FIGURE IS A PRINTED READOUT, NOT A LIVE COUNTER, and the
 * difference is the whole reason app_us and band_us are two fields.
 *
 * app_us is written by app_draw_dispatch() every time the app draws, which for
 * a terminal with a blinking cursor is every frame. Feeding that straight into
 * the band would re-key the retained shell surface sixty times a second and
 * hand back exactly the chrome rebuild the retained layer exists to avoid -
 * for a three-digit number nobody can read at that rate anyway.
 *
 * So the band latches: it takes the current measurement at most once every
 * WM_BAND_US_TICKS of the 100 Hz PIT, which is 320 ms. On an idle desktop this
 * loop does nothing at all - app_us only moves when an app actually redraws -
 * and the damage is the band's own strip rather than the whole window. */
#define WM_BAND_US_TICKS 32
static void band_us_latch(void)
{
    unsigned int now = idt_ticks();
    for (int i = 0; i < WM_MAX; i++) {
        if (!(wins[i].flags & WF_OPEN)) continue;
        if (wins[i].band_us == wins[i].app_us) continue;
        if (wins[i].band_us_tick &&
            now - wins[i].band_us_tick < WM_BAND_US_TICKS) continue;
        wins[i].band_us_tick = now ? now : 1u;
        wins[i].band_us = wins[i].app_us;
        int bh = band_h_of(wins[i].h, wins[i].flags);
        if (!bh || !win_visible(i)) continue;
        wins[i].shell_valid = 0;
        wm_damage(wins[i].x, wins[i].y + wins[i].h - 1 - bh, wins[i].w, bh);
    }
}

/* Shell AND shadow are one retained straight-alpha layer. Rounded corners and
 * the soft shadow stay translucent, so moving a window only composites the
 * cached layer over its new backdrop; it never reruns chrome primitives. */
static int shell_compose(int win, int focused,
                         int dx0, int dy0, int dx1, int dy1)
{
    struct win *W = &wins[win];
    int reach = shadow_reach(win);
    int sx = W->x - reach, sy = W->y - reach;
    int sw = W->w + 2 * reach, sh = W->h + 2 * reach;
    if (wm_anim_running(win) || !W->shell_px || W->shell_w != sw ||
        W->shell_h != sh ||
        W->shell_generation != client_surface_generation) return 0;
    unsigned int key = shell_state_key(win, focused);
    if (!W->shell_valid || W->shell_key != key) {
        unsigned long npx = (unsigned long)(unsigned)sw * (unsigned long)(unsigned)sh;
        for (unsigned long i = 0; i < npx; i++) {
            W->shell_px[i] = 0;
            W->shell_alpha[i] = 0;
        }
        if (!fb_surface_begin_alpha(W->shell_px, W->shell_alpha,
                                    sw, sh, sx, sy)) return 0;
        chrome_shadow(win, focused);
        chrome_shell(win, focused);
        fb_surface_end();
        retained_shell_builds++;
        W->shell_key = key;
        W->shell_valid = 1;
    }
    int cx, cy, cw, ch;
    if (!isect(sx, sy, sx + sw, sy + sh,
               dx0, dy0, dx1, dy1, &cx, &cy, &cw, &ch)) return 1;
    fb_clip(cx, cy, cw, ch);
    return fb_surface_blit_alpha(W->shell_px, W->shell_alpha,
                                 sw, sh, sx, sy);
}


/* Where the toast sits. The foot is desktop furniture drawn by hook_desk and
 * wm.c does not know how tall it is, so this asks for the same reserve every
 * other edge of the field asks for. A toast that lands under the foot is a
 * toast you cannot read or click.
 *
 * IT WAS `72 * t->scale`, AND IT CITED A FUNCTION THAT NO LONGER EXISTS. The
 * comment said "matching kernel.zl's dock_y()"; kernel.zl calls that "the OLD
 * dock_y()" in the one place it still names it, because the dock became the
 * 46 dp ZD_FOOT_H band. So the toast was being held 26 dp clear of a piece of
 * furniture with different dimensions - a gap that is not a margin, sized by a
 * number whose justification had been deleted out from under it.
 *
 * This was the FIFTH hardcoded copy of a reserve in this desktop. The rail, the
 * snap and the chrome were the first three, toasttest.c the fourth. Each one
 * looked right on its own and only disagreed with the others. RESERVE_BOT(t) is
 * the single place that answers this now. */
static void toast_rect(int *x, int *y, int *w, int *h)
{
    const struct ui_theme *t = ui_theme();
    notify_rect((int)fb_pxw(), (int)fb_pxh(), RESERVE_BOT(t), t->scale, x, y, w, h);
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
        int reach = OFFPLANE_OFF(ts) + OFFPLANE_SOFT(ts);
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
    /* THE TOAST IS THE THIRD OFF-PLANE OBJECT (proto:965), so it wears the
     * off-plane pair like the other two - not the dragged plate's. */
    fb_shadow(x, y, w, h, OFFPLANE_OFF(t), OFFPLANE_SOFT(t));
    fb_rrect(x, y, w, h, t->radius, t->border);
    fb_rrect(x + 1, y + 1, w - 2, h - 2, t->radius - 1, t->panel_hi);
    /* THE BAR IS --zd-focus-bar WIDE, WHICH IS THE FOCUS BAR'S OWN WIDTH.
     * `.toast .bar { width: var(--zd-focus-bar) }` - the same 3dp the focused
     * plate uses, because it is the same signal. This drew UI_S1(t)/2, which
     * is 2dp: a third narrower than the thing it is quoting. */
    fb_fill_px(x + 1, y + 1, t->focus_bar, h - 2, t->accent);

    /* TITLE AND BODY, WHICH IS WHAT A TOAST IS.
     *
     * The prototype's toast is two lines - the title says what happened, the
     * body says the measurement or the reason - and all sixteen it can raise
     * use both. This drew one centred line, so a toast could say "knockout on"
     * or it could say what that means, never both, and the second half is the
     * half worth reading.
     *
     * Left padding is 14dp because the bar occupies the first 3 and the text
     * must clear it - `.toast { padding-left: calc(14px * var(--ui)) }`. With
     * no body the title still centres, exactly as before, so every existing
     * caller looks unchanged. */
    int th = fb_text_prop_h();
    const char *body = notify_body();
    int tx = x + UI_DP(t, 14);
    if (!body) {
        fb_text_prop(tx, y + (h - th) / 2, msg, t->text);
    } else {
        int pad = UI_DP(t, 6);
        fb_text_prop(tx, y + pad, msg, t->text);
        fb_text_prop(tx, y + pad + th + UI_DP(t, 2), body, t->text_2);
    }

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
        unsigned int phase_started = paint_begin();
        if (hook_desk) hook_desk(rx0, ry0, rx1 - rx0, ry1 - ry0);
        /* zsweep sits ON the wallpaper and UNDER everything else, so it goes
         * between the furniture pass and the first window. */
        sweep_draw(rx0, ry0, rx1, ry1);
        fb_clip(rx0, ry0, rx1 - rx0, ry1 - ry0);   /* sweep_draw narrowed it */
        snap_preview_draw(rx0, ry0, rx1, ry1);
        paint_end(&paint_desk_cycles, phase_started);

        for (int i = 0; i < nz; i++) {          /* BACK TO FRONT = paint order */
            int win = zorder[i];
            struct win *W = &wins[win];
            /* MINIMISED, or on another workspace. Both mean "paints nothing
             * this frame"; neither means "leaves the z-order", which is what
             * keeps a workspace switch from reshuffling anything. */
            if (!win_visible(win)) continue;
            struct wm_region vis[WM_VIS_REGION_MAX];
            int nvis = visible_damage_regions(i, rx0, ry0, rx1, ry1, vis);
            for (int vi = 0; vi < nvis; vi++) {
            int rx0 = vis[vi].x0, ry0 = vis[vi].y0;
            int rx1 = vis[vi].x1, ry1 = vis[vi].y1;
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
            if (paint_trace) paint_window_visits++;
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
            phase_started = paint_begin();
            if (anim_is(win, ANIM_FADE)) {
                fade = wm_anim_alpha(win);
                if (fade < 255) stash = fb_stash(stash_x, stash_y, stash_w, stash_h);
            }
            paint_end(&paint_effect_cycles, phase_started);

            fb_clip(cx, cy, cw, ch);            /* clip 1: the frame + shadow */
            phase_started = paint_begin();
            if (!shell_compose(win, win == focus_win, rx0, ry0, rx1, ry1)) {
                fb_clip(cx, cy, cw, ch);
                chrome_shadow(win, win == focus_win);
                chrome_shell(win, win == focus_win);
            }
            /* THE MODULE CODE, ON TOP OF WHICHEVER PATH DREW THE FRAME. It is
             * the only chrome that depends on where the window IS, so it lives
             * outside the position-independent shell cache - see
             * chrome_module(). Drawn here rather than inside chrome_shell so
             * the cached and direct paths produce the same pixels, and under
             * the frame-plus-shadow scissor already set above. */
            fb_clip(cx, cy, cw, ch);
            chrome_module(win, win == focus_win);
            paint_end(&paint_chrome_cycles, phase_started);

            int fx, fy, fw, fh, ax, ay, aw, ah;
            anim_rect(win, &fx, &fy, &fw, &fh);
            client_of(fx, fy, fw, fh, W->flags, &ax, &ay, &aw, &ah);
            if (app_drawable(win_app(win)) && isect(ax, ay, ax + aw, ay + ah,
                                   rx0, ry0, rx1, ry1, &cx, &cy, &cw, &ch)) {
                int retained = !wm_anim_running(win) && W->client_px &&
                               W->client_w == aw && W->client_h == ah &&
                               W->client_generation == client_surface_generation;
                if (retained && !W->client_valid) {
                    /* Render the WHOLE client offscreen once. Rendering only
                     * this damage intersection would cache untouched heap
                     * bytes and expose them when the window later moves. */
                    phase_started = paint_begin();
                    if (fb_surface_begin(W->client_px, aw, ah, ax, ay)) {
                        app_draw_dispatch(win, win_app(win), ax, ay, aw, ah,
                                          win == focus_win);
                        fb_surface_end();
                        W->client_valid = 1;
                        W->client_dirty = 0;
                        if (paint_trace) paint_app_calls++;
                    } else {
                        retained = 0;
                    }
                    paint_end(&paint_app_cycles, phase_started);
                }
                if (retained && W->client_valid && W->client_dirty) {
                    int dx0 = W->client_dirty_x0;
                    int dy0 = W->client_dirty_y0;
                    int dx1 = W->client_dirty_x1;
                    int dy1 = W->client_dirty_y1;
                    phase_started = paint_begin();
                    if (fb_surface_begin(W->client_px, aw, ah, ax, ay)) {
                        fb_clip(ax + dx0, ay + dy0, dx1 - dx0, dy1 - dy0);
                        app_draw_dispatch(win, win_app(win), ax, ay, aw, ah,
                                          win == focus_win);
                        fb_surface_end();
                        W->client_dirty = 0;
                        if (paint_trace) paint_app_calls++;
                    } else {
                        W->client_valid = 0;
                        W->client_dirty = 0;
                        retained = 0;
                    }
                    paint_end(&paint_app_cycles, phase_started);
                }
                fb_clip(cx, cy, cw, ch);        /* clip 2: NARROWER - client */
                if (retained && W->client_valid) {
                    if (!fb_surface_blit(W->client_px, aw, ah, ax, ay))
                        retained = 0;
                }
                if (!retained) {
                    /* Allocation, target binding, or compositor-back-buffer
                     * refusal: today's direct path remains the correctness
                     * fallback and never leaves a blank/stale client. */
                    phase_started = paint_begin();
                    app_draw_dispatch(win, win_app(win), ax, ay, aw, ah,
                                      win == focus_win);
                    paint_end(&paint_app_cycles, phase_started);
                    if (paint_trace) paint_app_calls++;
                }
                int ready_app = win_app(win);
                if (W->ready_app != ready_app) {
                    W->ready_app = ready_app;
                    wm_lifecycle("ready", win, ready_app, W->generation, nz);
                }
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
            phase_started = paint_begin();
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
            paint_end(&paint_effect_cycles, phase_started);
            }
        }

        /* The closing window's ghost goes where the window would have been in
         * the walk above - on top of everything behind it. It is drawn after
         * the loop rather than inside it because it is not IN the loop's list:
         * it left the z-order the instant it was closed. */
        phase_started = paint_begin();
        ghost_draw(rx0, ry0, rx1, ry1);

        /* ...and the toast on top of all of them, still inside this damage
         * rectangle. Added, not woven in: the loop above is unchanged. */
        toast_draw(rx0, ry0, rx1, ry1);

        /* ...and the overlay above even that. Last in, last drawn: a modal that
         * a toast can cover is not modal. Same damage rectangle, so a menu
         * repaint costs the menu and not the screen.
         *
         * RESTORE THE SCISSOR FIRST - the window loop above narrows it to each
         * window's frame and then to its client, and leaves it there. ghost_draw
         * and toast_draw survive that because each sets its own clip; an
         * overlay that does not would be clipped to whichever window happened
         * to be painted last. That is not a hypothetical: the command palette
         * drew only across the Terminal, and the System Monitor beside it
         * appeared to be ON TOP of a modal. Same fix, and the same reason, as
         * the `sweep_draw narrowed it` restore at the top of this loop. */
        fb_clip(rx0, ry0, rx1 - rx0, ry1 - ry0);
        if (hook_overlay) hook_overlay(rx0, ry0, rx1, ry1);
        paint_end(&paint_effect_cycles, phase_started);
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

/* Declared up beside chrome_shadow, defined here where the grab it reads
 * actually lives. A plate is OFF THE PLANE while it is in your hand, and that
 * - together with WF_MODAL - is the only state in which anything on the desk
 * casts a shadow under PRESSWORK. Resize counts as well as move: a plate being
 * pulled by its corner is no more at rest than one being carried. */
static int win_lifted(int win)
{
    return pgrab == win && (grab_drag == GRAB_MOVE || grab_drag == GRAB_RESIZE);
}
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
    /* the cluster is derived, not a constant: the controls are ZD_WINCTL wide
     * and butted now, so the 128dp that used to stand for "three 26dp squares
     * with 6dp gaps plus both margins" no longer describes anything */
    int avail = wins[win].w - title_controls_w(t) - UI_S4(t);
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
    /* Rows y+1 .. y+title_h inclusive, matching what chrome_header paints and
     * what client_of now leaves alone. `< y + title_h` stopped one row short of
     * the header's own foot rule, which is a row the header owns and drags
     * from - a one-pixel dead stripe along the bottom of every title bar. */
    return y >= wins[win].y && y <= wins[win].y + t->title_h &&
           x >= wins[win].x && x < wins[win].x + wins[win].w;
}

static int dispatch_app_event(int win, int type, int code, int x, int y)
{
    if (!wm_is_open(win)) return 0;
    int app = win_app(win);
    int changed;
    dispatching_win = win;
    dispatch_damage_explicit = 0;
    if (userwin_is_app && userwin_event_app && userwin_is_app(app))
        changed = userwin_event_app(app, win, type, code, x, y);
    else if (hook_event)
        changed = hook_event(app, win, type, code, x, y);
    else
        changed = 0;
    dispatching_win = -1;
    if (changed && !dispatch_damage_explicit) wm_invalidate_client(win);
    dispatch_damage_explicit = 0;
    return changed;
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
    } else if (snap_set_side_reserves(RESERVE_LEFT(t), 0),
               !snap_apply(win, z, gx, gy, gw, gh,
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

/* THE SAME TOGGLE, REACHABLE FROM zl. The window menu's "maximise" row needs
 * it, and SK_UP / SK_DOWN / SNAP_NONE are all private to this file - binding
 * the raw wm_snap_key would mean hand-copying three constants into the runtime,
 * which is the class of mistake this tree keeps finding. One wrapper instead,
 * so the menu row and a double-click cannot land in different states. */
void wm_max_toggle(int win) { wm_toggle_max(win); }

/* IS THIS PLATE OVER ANOTHER ONE? The chrome has asked this since the occluder
 * was written; the window menu's row 5 stated the answer as a literal because
 * nothing published it. Same shape as the row below it, which stated the focus
 * state until wm_focus() was read instead. */
int wm_over_below(int win) { return wm_is_open(win) ? win_over_below(win) : 0; }

static void route_mouse(int x, int y, int btn)
{
    int down = (btn & 1) && !(last_btn & 1);
    int up   = !(btn & 1) && (last_btn & 1);
    /* Bit 1 is the right button - ui.h states the PS/2 mask where it explains
     * why the double-click bit had to go at bit 8. Read BEFORE last_btn is
     * overwritten, like down and up, so this is an edge and not a level. */
    int rdown = (btn & 2) && !(last_btn & 2);
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
            /* CLAMPED HERE, NOT IN wm_move. The first attempt put it inside
             * wm_move and broke six wmtest_feel assertions at once - maximise,
             * restore, Super+Down and the snap round-trip all place windows
             * PROGRAMMATICALLY through the same function, and a restore that
             * cannot return a window to the exact rect it came from is not a
             * restore. The prototype clamps in layout(), which runs on the
             * user's interaction; this is that path and nothing else. */
            int mx = x - grab_dx, my = y - grab_dy;
            clamp_to_field(&mx, &my, wins[pgrab].x, wins[pgrab].y);
            wm_move(pgrab, mx, my);
            snap_preview_set(snap_zone_for_point(x, y,
                                                 (int)fb_pxw(), (int)fb_pxh()));
        } else if (grab_drag == GRAB_RESIZE) {
            /* grab_dx/dy hold the offset from the pointer to the corner, so
             * the corner stays under the cursor instead of snapping to it. */
            wm_resize(pgrab, x + grab_dx - wins[pgrab].x,
                             y + grab_dy - wins[pgrab].y);
        } else if (hook_event) {
            dispatch_app_event(pgrab, EV_MOUSE, btn, x, y);
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

    /* 0. AN OVERLAY OUTRANKS EVERYTHING BELOW IT.
     *
     * After the pointer-grab block above, so a drag already in flight still
     * finishes; before modal_win(), so an overlay covers the start menu too.
     * Every event goes here, not only presses - a release that leaked through
     * would end up starting or ending a drag on a window the user cannot even
     * see, which is exactly what used to happen when the window menu was open
     * on top of the window it belongs to.
     *
     * The keyboard has followed this rule since desk_key was wired; the pointer
     * simply never did, and no hit test for any overlay surface existed in the
     * tree to make it possible.
     *
     * `down` rather than btn is passed deliberately: btn is a LEVEL, so a row
     * would re-run on every frame the button stayed held. down is the press
     * edge, computed at the top of this function beside up and rdown for
     * exactly this reason. last_btn is already stored up there. */
    if (hook_overlay_click && hook_overlay_click(x, y, down)) return;

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
    /* THE WINDOW'S MENU, before the left button's three destinations. It does
     * not focus or raise: the prototype's first item IS "bring to front", so
     * doing it on the way in would make that row a no-op describing something
     * that already happened. */
    if (rdown && hook_win_menu) { hook_win_menu(hit, x, y); return; }
    if (down) {
        wm_focus(hit);
        wm_raise(hit);
        int dbl = is_double(hit, x, y);
        /* THE APP IS ASKED FIRST. There was no close hook anywhere in this
         * OS - wm.c had no app_close callback and kernel.zl defined only
         * app_tick - so this line used to destroy an editor holding unsaved
         * text with no toast, no confirm and no trace. The next open re-read
         * the file from disk and the edits were simply gone. The pane
         * advertised the loss in its own footer one frame before it happened. */
        if (in_closebox(hit, x, y)) {
            if (hook_can_close && !hook_can_close(hit)) return;
            wm_close_fx(hit); return;
        }
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
    dispatch_app_event(hit, EV_MOUSE, btn, x, y);
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
     * KEY_TAB, and the USB HID path maps the HID usage directly to the same
     * key code. input_code() returns last.code, the key. So
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
    /* AN OVERLAY IS MODAL TO THE KEYBOARD, and the desk says so by consuming.
     *
     * Only Super, Escape and F1 were offered to the desk, which is why the
     * palette could be opened and then not driven: its arrows, its Enter and
     * every character typed into it went to whichever window had focus behind
     * it. ov_active() exists in kernel.zl for exactly this question and had
     * ZERO READERS.
     *
     * The desk is offered every key now and answers 1 only when it took one.
     *
     * WHICH KEYS THOSE ARE HAS GROWN, and the comment here said three when it
     * had become six. With no overlay up the desk claims Super, Escape and F1
     * as before, AND Ctrl+G, Ctrl+K and Ctrl+L, which the field menu advertises
     * as global shortcuts and the prototype binds globally too. That is the
     * intended behaviour - a desktop shortcut that only works when no window
     * has focus is not a desktop shortcut - but it does mean a focused window
     * no longer sees those three control codes, and saying "the terminal still
     * gets its characters" without that qualification was no longer true.
     * Everything else still falls through untouched. */
    /* ONE LOGICAL PRESS, ONE OFFER. This read `EV_KEY_DOWN || EV_CHAR`, which
     * offered a PS/2 press to the desk TWICE and made every desk toggle a net
     * no-op on real hardware.
     *
     * The reason is one line in input.c:412 - `if (!key && ch) key = ch;`. A
     * letter has no sc_special() keycode, so its EV_KEY_DOWN is BACKFILLED with
     * the character. Ctrl+G therefore pushes EV_KEY_DOWN code 7 AND EV_CHAR
     * code 7 (input.c:436-437), the same number down both, and desk_key ran
     * `desk_grid_on = 1 - desk_grid_on` on each - back to where it started,
     * with two toasts, the second announcing the state it had just left.
     *
     * NO PROBE IN THIS TREE COULD SEE IT. Every probe-*.py drives COM1, and the
     * serial path pushes EV_CHAR alone and says so at input.c:851 ("EV_CHAR
     * ONLY, DELIBERATELY"). The bug existed exclusively on the wire no test
     * harness uses, which is the whole reason it survived being "verified".
     *
     * The split below is by CODE SPACE, not by event type, because that is
     * what actually distinguishes the two: keycodes are >= 0x100 (KEY_ESC,
     * KEY_F1, KEY_SUPER, the arrows) and reach us only as EV_KEY_DOWN, since
     * sc_extended() sets ch = 0. Characters are < 0x100 and are authoritative
     * on EV_CHAR down both wires. Neither key is now offered twice, and the
     * serial ESC (27, a character) still arrives - which is why the separate
     * EV_CHAR-27 arm that used to sit below is gone rather than kept. */
    if (type == EV_KEY_DOWN && code >= 0x100) {
        if (hook_desk_key && hook_desk_key(code, mods)) return;
    }
    if (type == EV_CHAR && code < 0x100) {
        if (hook_desk_key && hook_desk_key(code, mods)) return;
    }
    if (type == EV_KEY_DOWN && (code == KEY_SUPER || code == KEY_ESC || code == KEY_F1)) {
        /* Super is the desktop's whether or not anything wanted it. Escape and
         * F1 fall through to the focused window when no overlay took them - the
         * editor's Escape still saves and closes. */
        if (code == KEY_SUPER) return;
    }
    /* Ctrl+W closes. Closing is the close box or Ctrl+W - NEVER "press any
     * key", which is the phrase this whole rewrite exists to delete. */
    if (type == EV_CHAR && code == 23) {        /* Ctrl+W */
        if (focus_win >= 0) {
            if (hook_can_close && !hook_can_close(focus_win)) return;
            wm_close_fx(focus_win);
        }
        return;
    }
    int m = modal_win();
    int target = (m >= 0) ? m : focus_win;
    if (target < 0) return;
    dispatch_app_event(target, type, code, 0, 0);
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
    dispatch_app_event(hit, EV_WHEEL, notches, x, y);
}

static void wm_route(int type)
{
    if (type == EV_WHEEL) { route_wheel(input_x(), input_y(), input_code()); return; }
    if (type == EV_MOUSE) route_mouse(input_x(), input_y(), input_code());
    else                  route_key(type, input_code(), input_mods());
}

static unsigned int pending_input_tsc, pending_input_seq;

/* Route input whenever an interrupt wakes the main loop, even if the next
 * visual commit deadline has not arrived.  Then drain once more immediately
 * before the commit so a just-arrived event is never left behind an older
 * frame.  pending_input_* keeps the oldest causal edge until pixels present. */
static void wm_input_drain(void)
{
    input_poll();
    for (int guard = 0; guard < 64; guard++) {
        int t = input_next();
        if (!t) break;
        if (!pending_input_seq) {
            pending_input_tsc = input_event_tsc();
            pending_input_seq = input_event_seq();
        }
        wm_route(t);
    }
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
/* An input may be consumed in a frame which needs no repaint (focus was
 * already correct, a button was released over inert chrome, etc.). Keep the
 * oldest unpresented input until the next painted frame instead of losing the
 * correlation at the end of that CPU iteration. */
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
#define WM_SAMPLE_N 256
static unsigned int sample_frame[WM_SAMPLE_N], sample_input[WM_SAMPLE_N];
static unsigned int sample_head, sample_n;

static unsigned int frame_delta_us(unsigned int start, unsigned int end,
                                   unsigned int cycles_per_us)
{
    unsigned int delta = end - start;
    /* Same guard as the total-frame counter below.  It rejects a duration
     * large enough to be a stale/wrapped sample instead of persisting a fake
     * multi-second phase. */
    if (!cycles_per_us || delta >= 0x40000000u) return 0;
    return delta / cycles_per_us;
}

int wm_frame_us(void)  { return (int)frame_us; }

/* The per-window figure. app_us has been measured per window since the two
 * rdtsc in app_draw_dispatch went in, and until now NOTHING COULD READ IT:
 * the value was written into wins[win].app_us every frame and no accessor
 * existed in any .c, .h or .zl in the tree. So the settings pane printed
 * wm_frame_us() under the label "THIS PANE, LAST DRAW" - the whole
 * compositor's frame, every window's work, presented as one pane's cost -
 * while the comment above that row asserted it was the per-window number.
 * A measurement that exists but cannot be read is indistinguishable from one
 * that was never taken, and it stays that way until someone reads the slot. */
int wm_win_us(int win)
{
    if (!wm_is_open(win)) return 0;
    return (int)wins[win].app_us;
}
int wm_peak_us(void)   { return (int)frame_peak_us; }
int wm_late(void)      { return (int)frame_late; }
int wm_lost(void)      { return (int)frame_lost; }
int wm_painted(void)   { return (int)frame_painted; }
int wm_budget_us(void) { return (int)FRAME_BUDGET_US; }
int wm_sample_count(void) { return (int)sample_n; }
static unsigned int sample_slot(int i)
{
    if (i < 0 || (unsigned)i >= sample_n) return WM_SAMPLE_N;
    return (sample_head + WM_SAMPLE_N - sample_n + (unsigned)i) % WM_SAMPLE_N;
}
int wm_sample_frame(int i) { unsigned s = sample_slot(i); return s < WM_SAMPLE_N ? (int)sample_frame[s] : -1; }
int wm_sample_input(int i) { unsigned s = sample_slot(i); return s < WM_SAMPLE_N ? (int)sample_input[s] : -1; }
void wm_peak_reset(void) { frame_peak_us = 0; frame_late = 0; frame_lost = 0;
                           frame_painted = 0; sample_head = sample_n = 0; }

void wm_frame(void)
{
    /* This executes before the pacing early-return.  HID->route latency is
     * therefore interrupt/main-loop latency, not one 60 Hz frame interval. */
    wm_input_drain();
    unsigned int clock_khz = cpu_tsc_khz();
    unsigned int now_tsc = cpu_tsc_lo();
    unsigned int missed_deadlines = 0;
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
        missed_deadlines = missed;
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
    unsigned int frame_started_tick = now;
    last_tick = now;
    /* apps-in-windows timed the frame with the 64-bit cpu_tsc(); this tree
     * uses the 32-bit cpu_tsc_lo(). Both declarations survived the merge and
     * shadowed each other on the same name. One timer. */
    unsigned int t0 = cpu_tsc_lo();
    int trace_frame = zllog_frame != 0 || zllog_frame_observe != 0;
    unsigned int t_input = t0, t_tick = t0, t_compositor = t0, t_vblank = t0;
    int did_paint = 0;
    /* Close the race between the early drain and the atomic visual commit. */
    wm_input_drain();
    if (trace_frame) t_input = cpu_tsc_lo();

    /* app_tick runs every frame, is cheap, and MUST NOT DRAW. Returning 1 is
     * how a clock or a snake says "my state changed" without owning the frame
     * - which is the whole reason those demos no longer need a while-loop. */
    /* advance every animation. Damaging the SETTLED rect (which is the
     * largest) is what erases the smaller frame drawn a moment ago. */
    anim_tick();

    /* the foot band's app_us figure, moved on its own slow cadence rather than
     * on the app's. See band_us_latch. */
    band_us_latch();

    /* THE DESK TICKS EVEN WITH NOTHING OPEN. The loop below is the only
     * per-frame hook the policy layer has, and it is gated twice over - on a
     * window existing at all, and on that window not being minimised. Anything
     * the policy layer clocked from inside it therefore STOPPED whenever the
     * desktop was empty: the raster ring took no samples while the compositor
     * kept painting frames and kept counting them, so the plot and the ADVANCE
     * figure beside it drifted apart by exactly the frames composited while the
     * desk was clear - and the wall clock stopped asking for its own block back.
     *
     * A desk-wide tick is passed id -1 and win -1, which no app can be, so the
     * policy layer branches on it at the top of app_tick and every window arm
     * below stays untouched. Its return is ignored: it draws nothing and owns
     * no client rect to invalidate. */
    if (hook_tick) hook_tick(-1, -1);

    if (hook_tick)
        for (int i = 0; i < nz; i++)
            if (!(wins[zorder[i]].flags & WF_MINIMIZED) &&
                hook_tick(win_app(zorder[i]), zorder[i]))
                wm_invalidate_client(zorder[i]);

    /* The toast appears and retires on a tick count of its own. This damages
     * ONLY its own rectangle and only when what is on screen actually changed
     * - notify_tick returns 1 for that and 0 otherwise, the same contract
     * hook_tick uses above. No existing damage rule is altered. */
    if (notify_tick(now)) {
        int tx, ty, tw, th;
        toast_rect(&tx, &ty, &tw, &th);
        const struct ui_theme *t = ui_theme();
        int reach = OFFPLANE_OFF(t) + OFFPLANE_SOFT(t);
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
    if (trace_frame) t_tick = cpu_tsc_lo();

    if (nwd) {
        paint_reset(trace_frame && zllog_frame_observe != 0);
        /* Only the SPRITE has a save-under to go stale. A cursor on its own
         * plane is not in the back buffer at all, so a repaint cannot smear
         * it and hiding it would be a visible flicker for no reason. */
        if (!gpu_cursor_is_live())
            fb_pointer_hide();  /* the sprite's save-under is stale once the
                                   pixels under it are about to be redrawn */
        wm_repaint();
        paint_trace = 0;
        did_paint = 1;
    } else paint_reset(0);
    /* The plane first; the sprite only if it did not take. */
    if (!gpu_cursor_move(ptr_x, ptr_y))
        fb_pointer_show(ptr_x, ptr_y);
    /* fb damage is broader than WM repaint: the software cursor restores and
     * draws two patches without adding WM damage.  Measuring only did_paint
     * hid the path the user reported as the laggiest. */
    unsigned int damage_count = (unsigned int)fb_damage_count();
    unsigned int damage_area = fb_damage_area();
    int did_present = damage_count != 0u;
    if (trace_frame) t_compositor = cpu_tsc_lo();
    /* Firmware or our modeset may already have an Intel pipe scanning out.
     * Wait only on that proven source; QEMU/BGA takes the deadline path and
     * never touches an absent MMIO block. */
    int waited_vblank = 0;
    if (did_paint && intel_supported && intel_wait_vblank && intel_supported()) {
        waited_vblank = intel_wait_vblank() != 0;
    }
    if (trace_frame) t_vblank = cpu_tsc_lo();
    fb_present();

    if (did_present) {
        unsigned int khz = clock_khz;
        if (khz) {
            /* The phase clock is intentionally 32-bit to avoid libgcc in the
             * freestanding kernel. A physical freeze can outlive that TSC
             * window, though, and silently dropping the worst frame defeated
             * the recorder. Preserve it with the 10 ms PIT clock and saturate
             * only if even that duration exceeds the record field. */
            unsigned int t_present = cpu_tsc_lo();
            unsigned int dt = t_present - t0;
            unsigned int cycles_per_us = khz / 1000u ? khz / 1000u : 1u;
            if (dt < 0x40000000u) frame_us = dt / cycles_per_us;
            else {
                unsigned int ticks = idt_ticks() - frame_started_tick;
                frame_us = ticks > 429496u ? 0xffffffffu : ticks * 10000u;
            }
            if (frame_us > frame_peak_us) frame_peak_us = frame_us;
            frame_painted++;
            int late = frame_us > FRAME_BUDGET_US;
            if (late) frame_late++;
            unsigned int input_latency = pending_input_tsc
                ? frame_delta_us(pending_input_tsc, t_present, cycles_per_us)
                : 0u;
            sample_frame[sample_head] = frame_us;
            sample_input[sample_head] = input_latency;
            sample_head = (sample_head + 1u) % WM_SAMPLE_N;
            if (sample_n < WM_SAMPLE_N) sample_n++;

            unsigned int seq = frame_log_seq++;
            int sample = (seq % FRAMELOG_SAMPLE_N) == 0;
            if (trace_frame && zllog_frame_observe) {
                unsigned int flags = (late ? FRAMELOG_LATE : 0u) |
                                     (waited_vblank ? FRAMELOG_VBLANK : 0u) |
                                     (!did_paint ? FRAMELOG_CURSOR_ONLY : 0u);
                unsigned int bpp = fb_bits_per_pixel() / 8u;
                unsigned int present_bytes = damage_area * (bpp ? bpp : 4u);
                zllog_frame_observe(
                    frame_delta_us(t0, t_input, cycles_per_us),
                    frame_delta_us(t_input, t_tick, cycles_per_us),
                    frame_delta_us(t_tick, t_compositor, cycles_per_us),
                    frame_delta_us(t_compositor, t_vblank, cycles_per_us),
                    frame_delta_us(t_vblank, t_present, cycles_per_us),
                    frame_us, flags, damage_count, damage_area,
                    input_latency, pending_input_seq, missed_deadlines,
                    (unsigned)input_queued(), present_bytes,
                    paint_desk_cycles / cycles_per_us,
                    paint_chrome_cycles / cycles_per_us,
                    paint_app_cycles / cycles_per_us,
                    paint_effect_cycles / cycles_per_us,
                    paint_repaint_rects, paint_repaint_pixels,
                    paint_window_visits, paint_app_calls);
            } else if (trace_frame && (late || sample)) {
                unsigned int flags = (late ? FRAMELOG_LATE : 0u) |
                                     (sample ? FRAMELOG_SAMPLE : 0u) |
                                     (waited_vblank ? FRAMELOG_VBLANK : 0u) |
                                     (!did_paint ? FRAMELOG_CURSOR_ONLY : 0u);
                zllog_frame(frame_delta_us(t0, t_input, cycles_per_us),
                            frame_delta_us(t_input, t_tick, cycles_per_us),
                            frame_delta_us(t_tick, t_compositor, cycles_per_us),
                            frame_delta_us(t_compositor, t_vblank, cycles_per_us),
                            frame_delta_us(t_vblank, t_present, cycles_per_us),
                            frame_us, flags, damage_count, damage_area);
            }
        }
        /* This is the first visible presentation after the retained input.
         * Clear only after its correlation has been handed to the recorder. */
        pending_input_tsc = 0;
        pending_input_seq = 0;
    }
}
