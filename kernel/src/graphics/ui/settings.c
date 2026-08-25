/* settings.c - the Settings app: controls that change the running desktop.
 *
 * FEEL-PROMPT finding 0.3: runtime customisation was ONE FUNCTION CALL AWAY AND
 * UNUSED. `ui_theme_set()` already existed, every colour and metric was already
 * data in a struct on a 4/8/12/16/24 spacing scale, `fb_set_subpixel()` existed
 * and nothing exposed it. So this is not a feature to build - it is a feature
 * to EXPOSE, and the whole file is wiring rather than mechanism.
 *
 * Same for the toolkit. ui_toggle, ui_slider, ui_label and ui_sep were built and
 * asserted and NOTHING USED THEM: grepping for ui_begin outside ui.c finds only
 * hosttest. This is what they were for.
 *
 * WHY C AND NOT kernel.zl. FEEL-PROMPT §2 offers the Settings app's app_draw
 * branch in kernel.zl, and that is the natural home - policy belongs in zl. It
 * is not reachable yet: zl exposes NO natives for ui_* at all (the same grep),
 * so a zl Settings app would need ~15 new builtins in runtime_kernel.c first,
 * and runtime_kernel.c is the display session's file and the source of T-13.
 * Written in C it works today and it is gateable today; moving it to zl later
 * is a translation, not a redesign, because the widget calls map one to one.
 *
 * THE WIDGET SEQUENCE IS EMITTED ONCE, by build_ui(), and both passes call it.
 * An immediate-mode toolkit identifies a widget by its ORDER, so a draw pass
 * and a hit-test pass that emit different sequences hit-test the wrong control
 * - and they drift the moment someone edits one and not the other. One
 * function is the only version of this that cannot rot. run_ui() now owns
 * ui_begin() as well, so the CURSOR ORIGIN cannot drift from the sequence
 * either: the two used to be set in three different places.
 *
 * THE HIT-TEST PASS IS THE ONLY ONE WITH click SET. ui.c's fire() is level
 * triggered on L.click, and ui_toggle flips its variable inside fire(), so a
 * control visited twice with the button down toggles twice and nothing appears
 * to happen. Draw passes therefore always pass click = 0 - they are for
 * looking, not deciding.
 */

#include "ui.h"
#include "design.h"

/* ---- what this file drives ------------------------------------------------
 * Six sinks, all of which already existed. Only wm_set_anim is new, because
 * ANIM_FRAMES was a compile-time constant with no runtime switch. */
void fb_set_subpixel(int on);
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);
/* the console cell. uikit.c draws every mono run from it, so the slider card's
 * value column is this tall and not ui_text_h() tall - see card_slider(). */
int  fb_cell_h(void);
void input_set_speed(int pct);
void input_set_accel(int on);
void wm_set_anim(int on);

#define EV_MOUSE     4      /* wm.c's event types, same numbering as input.c */
#define EV_KEY_DOWN  1
#define KEY_TAB      0x103
#define KEY_ENTER    0x104

/* ---- the accent palette ---------------------------------------------------
 * Named, not a colour picker: there is no colour-picker widget and building one
 * would be the mechanism this file is supposed to avoid. Every entry is a
 * saturated colour of roughly equal luminance against the dark surfaces, so
 * switching accent never changes how readable anything is - which is the
 * property a palette has to have and a free colour picker cannot promise. */
static const struct { const char *name; unsigned rgb; } ACCENTS[] = {
    /* THE FIVE ARE THE REFERENCE'S OWN, not a set invented here.
     * docs/design/ds-reference.html line 1212, verbatim:
     *
     *     const ACCENTS = ['#b8e838','#4ce0b3','#57b6ff','#8f7bff','#e86ec4'];
     *
     * It names none of them, so the names are ours; the values are not.
     *
     * ENTRY 0 MUST EQUAL ui_theme_init's accent, or the default is a colour
     * you cannot get back to: settings_apply() rebuilds the theme and then
     * writes ACCENTS[S.accent] over it, so a mismatch means opening Settings,
     * choosing the entry marked "the default", and silently repainting the
     * desktop in a different colour. hosttest/palette.c asserts the equality
     * rather than trusting this comment.
     *
     * This table previously led with "Ice" #60D2EB, the blue-slate accent.
     * That colour is retired along with the rest of the old palette. */
    { "Lime",    ZD_ACCENT },    /* == ui_theme_init's accent. Keep them equal. */
    { "Mint",    ZD_ACCENT_ALT_1 },
    { "Azure",   ZD_ACCENT_ALT_2 },
    { "Violet",  ZD_ACCENT_ALT_3 },
    { "Magenta", ZD_ACCENT_ALT_4 },
};
#define N_ACCENT ((int)(sizeof ACCENTS / sizeof ACCENTS[0]))

/* ---- the settings themselves ----------------------------------------------
 * The app owns its state, which is ui.c's whole design: "the widget owns no
 * state. The app already has the variable; a retained toolkit would have made
 * a second copy of it and then needed a way to keep the two in step."
 *
 * Defaults are what the system boots with, so that opening Settings and
 * changing nothing is a no-op rather than a silent reset to this file's idea
 * of the defaults. */
struct settings {
    int accent;        /* index into ACCENTS      */
    int scale;         /* ui() - 1 at 8px cells   */
    int speed;         /* pointer speed, percent  */
    int accel;         /* acceleration curve      */
    int subpixel;      /* subpixel text           */
    int anim;          /* window animations       */
};

/* The pointer-speed range, mirrored from input.c's own SPD_MIN/SPD_MAX. The
 * slider and the persistence clamp must agree, so it is named once here. */
#define SPD_MIN_PCT  25
#define SPD_MAX_PCT 400

/* The UI scale range, likewise named once. The load clamp used 1..4 while the
 * slider offered 1..3, so a block carrying 4 loaded fine and then drew a slider
 * pinned past its own maximum. A clamp that disagrees with the control it
 * feeds is a clamp that will drift again. */
#define SCALE_MIN 1
#define SCALE_MAX 3

static struct settings S = { 0, 2, 100, 1, 1, 1 };

int settings_accent(void)   { return S.accent; }
int settings_scale(void)    { return S.scale; }
int settings_speed(void)    { return S.speed; }
int settings_accel(void)    { return S.accel; }
int settings_subpixel(void) { return S.subpixel; }
int settings_anim(void)     { return S.anim; }

/* ---- applying ------------------------------------------------------------
 * ONE function pushes every setting to its sink, and it is the only place that
 * does. Splitting it per-control looks tidier and is wrong: ui_theme_init()
 * REBUILDS the whole theme from scratch, so it wipes the accent, and a scale
 * change would silently reset the colour back to Ice. Rebuild, then re-apply
 * the accent on top - in that order, always.
 */
void settings_apply(void)
{
    /* THE ONLY CLAMP. It lives here because this is the funnel every path goes
     * through - the widgets, the loader, and anything added later - and it is
     * the last thing before ui_theme_init actually uses the value.
     *
     * It used to clamp scale from BELOW ONLY, with a literal 1, while
     * settings_load clamped both ways with the named constants. Two clamps that
     * disagree is the same defect as the 1..4-versus-1..3 mismatch fixed
     * earlier, surviving in a second place - so settings_load's copy is gone
     * and this one is exhaustive. */
    if (S.scale  < SCALE_MIN)   S.scale  = SCALE_MIN;
    if (S.scale  > SCALE_MAX)   S.scale  = SCALE_MAX;
    if (S.speed  < SPD_MIN_PCT) S.speed  = SPD_MIN_PCT;
    if (S.speed  > SPD_MAX_PCT) S.speed  = SPD_MAX_PCT;
    if (S.accent < 0)           S.accent = 0;
    if (S.accent >= N_ACCENT)   S.accent = N_ACCENT - 1;
    S.accel    = S.accel    ? 1 : 0;
    S.subpixel = S.subpixel ? 1 : 0;
    S.anim     = S.anim     ? 1 : 0;

    ui_theme_init(S.scale);                 /* wipes the accent... */
    struct ui_theme t = *ui_theme();
    t.accent = ACCENTS[S.accent].rgb;       /* ...so put it back */
    ui_theme_set(&t);

    input_set_speed(S.speed);
    input_set_accel(S.accel);
    fb_set_subpixel(S.subpixel);
    wm_set_anim(S.anim);
}

/* Everything visible depends on the theme, so a change damages the WHOLE
 * screen rather than the Settings window. That is the point of the item: a
 * panel where the change shows up on next boot is a configuration file with
 * extra steps, and one that repaints only itself is a panel that lies about
 * what it just did to the wallpaper, the dock and every other window. */
void wm_damage(int x, int y, int w, int h);

int settings_save(void);

/* APPLYING AND PERSISTING ARE DIFFERENT RATES.
 *
 * Applying must happen on every change - that is the whole item, a setting
 * that takes effect on the next boot is a config file with extra steps.
 * Persisting must NOT: wm.c hands the app a pointer grab for the whole
 * duration of a press, so a slider being dragged delivers one event per mouse
 * motion, and saving on each one issued 376 synchronous block writes for a
 * single gesture (measured by the adversarial review).
 *
 * So a change marks the block dirty and the write happens once, on button-up.
 * "Never write on boot" still holds - settings_save has exactly one caller and
 * it is downstream of a mouse release. */
static int dirty;

static void settings_commit(void)
{
    settings_apply();
    wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
    dirty = 1;
}

/* Called when the button comes back up. One write per gesture, not per pixel. */
static void settings_flush(void)
{
    if (!dirty) return;
    dirty = 0;
    settings_save();
}

/* ============================================================================
 * THE LAYOUT - ds-reference.html lines 700-740
 *
 * The reference's Settings app is TWO PANES, not a list:
 *
 *   158px sidebar   #14171a, 1px #0b0d0f right border, padding 7px 6px,
 *                   holding the nav rows (`setNav`, 3854-3859)
 *   content pane    padding 14px 16px, opening with a 15px/700 title
 *                   (`setTitle`, 706) and a 14px bottom margin
 *   cards           #14171a, 1px #1c2024, radius 13, margin-bottom 14, in four
 *                   shapes: slider (707-711), accent swatches (713-721),
 *                   toggle rows (722-731), about key/value (732-736)
 *   a closing note  11px #74797f (737)
 *
 * WIDGET GEOMETRY IS NOT RE-DERIVED HERE. kernel/docs/reference/ui/widgets.md S11
 * and S12 are the specification and uikit.c is the implementation; every
 * number below is either a ZD_* token from design.h or one of the six pane and
 * card measurements the reference states inline, named once immediately below.
 *
 * IT HAS TO FIT. ui.c's place() advances a cursor and never reports running
 * out of room: a widget past the bottom of the client area is still laid out,
 * still counted for widget identity, and simply drawn outside the scissor - so
 * it is invisible AND unclickable, with nothing anywhere saying so. The first
 * draft of this file put roughly 995 px of content in a 642 px client and
 * everything from "Interface scale" down could not be reached at all.
 *
 * That is why the six controls are spread across FIVE PAGES rather than
 * stacked, and why settings_probe_fit() exists: layout() computes the slack at
 * the bottom of every page and hosttest/settingstest.c asserts it is never
 * negative, at every scale the app can be set to. A silent overflow is the one
 * failure mode this app has already shipped once.
 * ========================================================================= */

/* The reference's own inline measurements. Everything else is a ZD_* token. */
#define SET_PANE_PY     14   /* content pane padding, ds-reference.html 705 */
#define SET_PANE_PX     16
#define SET_TITLE_MB    14   /* title margin-bottom, 706                    */
#define SET_CARD_MB     14   /* every card's margin-bottom, 708/714/723/733 */
#define SET_ACC_PY      12   /* the accent card pads 12/13, sliders 11/13   */
#define SET_ACC_GAP      9   /* its column gap AND its swatch row gap, 714  */
#define SET_SLIDER_GAP   8   /* label row -> input, 708                     */

#define DP(n) UI_DP(ui_theme(), (n))

/* ---- the pages -------------------------------------------------------------
 * ONE TABLE, WALKED TWICE - once by layout() for geometry and once by
 * build_ui() to emit. That is the whole anti-drift argument: a card cannot be
 * given a height in one walk and different contents in the other, because
 * neither walk owns the list.
 *
 * The names are the reference's own (3854), minus the five panes that would
 * have nothing behind them - this kernel has no Sound, Kernel, Network, Games
 * or Shortcuts to configure. Every page here drives a real sink. */
enum { PG_APPEARANCE = 0, PG_WINDOWS, PG_DISPLAYS, PG_DEVICES, PG_ABOUT,
       N_PAGE };

#define CARD_ACCENT 0
#define CARD_TOGGLE 1
#define CARD_SLIDER 2
#define CARD_ABOUT  3
#define MAX_CARD    2

/* which sink a slider or toggle card drives */
#define CTL_SCALE   0
#define CTL_SPEED   1
#define CTL_ACCEL   2
#define CTL_SUBPX   3
#define CTL_ANIM    4
/* ...or which about table an about card shows */
#define ABOUT_DISPLAY 0
#define ABOUT_SYSTEM  1

struct card_def { unsigned char kind, rows, data; };
struct page_def {
    const char *name;
    unsigned char ncard;
    struct card_def card[MAX_CARD];
    const char *note;
};

static const struct page_def PAGES[N_PAGE] = {
    { "Appearance", 2, { { CARD_ACCENT, 0, 0          },
                         { CARD_TOGGLE, 1, CTL_SUBPX  } },
      "Applied on the click; written to disk when you let go." },
    { "Windows",    1, { { CARD_TOGGLE, 1, CTL_ANIM   },
                         { 0, 0, 0 } },
      "The animation is the window open and close transition." },
    { "Displays",   2, { { CARD_SLIDER, 1, CTL_SCALE  },
                         { CARD_ABOUT,  3, ABOUT_DISPLAY } },
      "Scale redraws the whole desktop, not just this window." },
    { "Devices",    2, { { CARD_SLIDER, 1, CTL_SPEED  },
                         { CARD_TOGGLE, 1, CTL_ACCEL  } },
      "Pointer speed and acceleration take effect immediately." },
    { "About",      1, { { CARD_ABOUT,  6, ABOUT_SYSTEM },
                         { 0, 0, 0 } },
      "Settings live in one 512-byte block at LBA 64." },
};

/* The about card's static rows. The row COUNT is in two places - here and in
 * PAGES above - so the two are asserted equal rather than trusted. */
static const struct { const char *k, *v; } ABOUT_SYS[] = {
    { "OS",             "zl 0.1"         },
    { "Window manager", "wm.c"           },
    { "Kernel",         "i386 - ring 0"  },
    { "Toolkit",        "ui.c + uikit.c" },
    { "Settings block", "LBA 64"         },
    { "Store",          "nvme, 512 B"    },
};
_Static_assert(sizeof ABOUT_SYS / sizeof ABOUT_SYS[0] == 6,
               "PAGES[PG_ABOUT] declares 6 about rows");

/* Which page the sidebar is on. NOT PERSISTED, deliberately: the on-disk
 * record is six fields at version 1 and adding a seventh would invalidate
 * every block already written. Where the user last was is not a setting. */
static int S_page;

int settings_page(void)  { return S_page; }
int settings_page_count(void) { return N_PAGE; }
const char *settings_page_name(int i)
{
    return (i >= 0 && i < N_PAGE) ? PAGES[i].name : "";
}

/* ---- the computed layout ---------------------------------------------------
 * Pure arithmetic over (client rect, page, theme). It fires nothing and draws
 * nothing, so it is safe to run before ui_begin - which it has to be, because
 * ui_begin needs the cursor origin this computes. */
static struct lay {
    int page;
    int sb_x, sb_y, sb_w, sb_h;            /* the sidebar panel        */
    int nav_x, nav_y, nav_w, nav_step;     /* the nav rows inside it   */
    int cx, cw;                            /* the card column          */
    int ix, iw;                            /* a card's interior        */
    int title_y;
    int cy[MAX_CARD], ch[MAX_CARD];
    int ncard;
    int chip_rows;
    int note_y;
    int cur_x, cur_y, cur_w;               /* where ui.c's cursor starts */
    int fits;                              /* client bottom - content bottom */
} LO;

/* Walk the accent chips. draw == 0 counts the rows they wrap onto and emits
 * NOTHING - one wrap rule, used by the measure and by the draw, because two
 * copies of a wrap rule is how a card ends up one row too short. */
static int chip_walk(int x0, int y0, int iw, int draw)
{
    int x = x0, y = y0, rows = 1;
    for (int i = 0; i < N_ACCENT; i++) {
        int w = ui_chip_w(ACCENTS[i].name);
        if (x > x0 && x + w > x0 + iw) {
            x = x0;
            y += ui_chip_h() + DP(SET_ACC_GAP);
            rows++;
        }
        if (draw && ui_chip(x, y, ACCENTS[i].name, i == S.accent)) S.accent = i;
        x += w + DP(SET_ACC_GAP);
    }
    return rows;
}

/* The slider card's label row is as tall as the taller of its two runs: the
 * name is proportional and the value is mono, and uikit.c's mono is ONE size
 * that does not follow UI_SM/MD/LG. Measuring the shorter of the two would
 * clip the value. */
static int label_row_h(void)
{
    int a = ui_text_h(UI_MD), b = fb_cell_h();
    return a > b ? a : b;
}

static int card_height(int kind, int rows)
{
    const struct ui_theme *t = ui_theme();
    switch (kind) {
    case CARD_SLIDER:
        return 2 * DP(ZD_CARD_PY) + label_row_h() + DP(SET_SLIDER_GAP)
             + t->row_h;
    case CARD_ACCENT:
        return 2 * DP(SET_ACC_PY) + ui_text_h(UI_MD) + DP(SET_ACC_GAP)
             + LO.chip_rows * ui_chip_h()
             + (LO.chip_rows - 1) * DP(SET_ACC_GAP)
             + DP(SET_ACC_GAP) + 2 * ui_text_h(UI_SM);
    case CARD_TOGGLE:
        /* ui.c's cursor puts theme.gap between consecutive widgets, so the
         * reference's borderTop separator between rows 2..n is a gap here.
         * Recorded in the report rather than faked with a hairline. */
        return 2 * DP(ZD_CARD_PY) + rows * t->row_h + (rows - 1) * t->gap;
    default:
        /* ui_kv already carries the reference's row padding and its borderTop,
         * so the about card is exactly its rows - no card padding on top. */
        return rows * ui_kv_h();
    }
}

static void layout(int x, int y, int w, int h, int page)
{
    const struct page_def *pg;
    int sbw, pw, cy, i;

    if (page < 0 || page >= N_PAGE) page = 0;
    pg = &PAGES[page];
    LO.page = page;

    sbw = ui_sidebar_w();
    if (sbw > w / 2) sbw = w / 2;       /* never let it eat the content pane */
    if (sbw < 0) sbw = 0;
    LO.sb_x = x; LO.sb_y = y; LO.sb_w = sbw; LO.sb_h = h;
    LO.nav_x    = x + DP(ZD_SIDEBAR_PX);
    LO.nav_y    = y + DP(ZD_SIDEBAR_PY);
    LO.nav_w    = sbw - 2 * DP(ZD_SIDEBAR_PX);
    LO.nav_step = ui_nav_h();
    if (LO.nav_w < 1) LO.nav_w = 1;

    pw = w - sbw;
    LO.cx = x + sbw + DP(SET_PANE_PX);
    LO.cw = pw - 2 * DP(SET_PANE_PX);
    if (LO.cw < DP(48)) LO.cw = DP(48);
    LO.ix = LO.cx + DP(ZD_CARD_PX);
    LO.iw = LO.cw - 2 * DP(ZD_CARD_PX);
    if (LO.iw < DP(24)) LO.iw = DP(24);

    LO.title_y  = y + DP(SET_PANE_PY);
    LO.chip_rows = chip_walk(0, 0, LO.iw, 0);

    cy = LO.title_y + ui_text_h(UI_LG) + DP(SET_TITLE_MB);
    LO.ncard = pg->ncard;
    for (i = 0; i < LO.ncard; i++) {
        LO.cy[i] = cy;
        LO.ch[i] = card_height(pg->card[i].kind, pg->card[i].rows);
        cy += LO.ch[i] + DP(SET_CARD_MB);
    }
    LO.note_y = cy;
    LO.fits   = (y + h) - (cy + ui_text_h(UI_SM) + DP(SET_PANE_PY));

    /* Where ui.c's layout cursor has to start: the y of the FIRST widget on
     * this page that takes its position from the cursor rather than from a
     * rectangle. ui_slider and ui_toggle are the only two - see build_ui. */
    LO.cur_x = LO.ix;
    LO.cur_w = LO.iw;
    LO.cur_y = y;
    for (i = 0; i < LO.ncard; i++) {
        if (pg->card[i].kind == CARD_SLIDER) {
            LO.cur_y = LO.cy[i] + DP(ZD_CARD_PY) + label_row_h()
                     + DP(SET_SLIDER_GAP);
            break;
        }
        if (pg->card[i].kind == CARD_TOGGLE) {
            LO.cur_y = LO.cy[i] + DP(ZD_CARD_PY);
            break;
        }
    }
}

/* Lay a page out into a client rectangle and report the slack at its bottom,
 * without emitting anything and without disturbing the pass in progress.
 * NEGATIVE MEANS CONTENT FELL OFF THE BOTTOM - which is invisible on screen
 * and unclickable, and which nothing in ui.c reports. The gate is
 * hosttest/settingstest.c. */
int settings_probe_fit(int page, int w, int h)
{
    struct lay save = LO;
    int slack;
    layout(0, 0, w, h, page);
    slack = LO.fits;
    LO = save;
    return slack;
}

/* ---- the cursor shadow -----------------------------------------------------
 * ui_slider and ui_toggle take NO RECTANGLE - they are ui.c's layout-cursor
 * widgets, and the cursor is the only thing that positions them. So the
 * content pane is laid out THROUGH the cursor rather than beside it, and
 * `flow` is this file's copy of where the cursor is.
 *
 * Every vertical step goes through flow_to() or through a widget whose height
 * is added straight afterwards, so the two cannot disagree. flow_to() can only
 * move FORWARD and can only move by more than one theme.gap - ui_space() has
 * no other shape - so a step it cannot take is counted rather than silently
 * dropped, and settingstest asserts the count stays zero. Every real gap in
 * the page table is at least a card's padding plus its margin (36 design px)
 * against a gap of 8, so the counter is a guard against a future edit, not
 * against today's layout. */
static int flow;
static int flow_fault;

int settings_flow_fault(void) { return flow_fault; }

static void flow_to(int target)
{
    int gap = ui_metric(UI_METRIC_GAP);
    int d = target - flow;
    if (d == 0) return;
    if (d < 0 || d <= gap) { flow_fault++; return; }
    ui_space(d - gap);              /* ui_space(n) advances n + gap */
    flow = target;
}

/* ---- the four cards --------------------------------------------------------
 * ds-reference.html 707-736. Each one draws its own background with ui_card
 * FIRST, because a card is a surface and its contents sit on it. */
struct sbuf { char b[40]; int n; };
static void sb_reset(struct sbuf *s) { s->n = 0; s->b[0] = 0; }
static void sb_c(struct sbuf *s, char c)
{
    if (s->n < (int)sizeof s->b - 1) { s->b[s->n++] = c; s->b[s->n] = 0; }
}
static void sb_s(struct sbuf *s, const char *p) { while (*p) sb_c(s, *p++); }
static void sb_u(struct sbuf *s, unsigned v)
{
    char t[12];
    int n = 0;
    if (!v) t[n++] = '0';
    while (v && n < 11) { t[n++] = (char)('0' + v % 10u); v /= 10u; }
    while (n) sb_c(s, t[--n]);
}

/* 708-711: a label row - name left, mono value right - above a full-width
 * range input. */
static void card_slider(int cy, int ctl)
{
    const struct ui_theme *t = ui_theme();
    int ly = cy + DP(ZD_CARD_PY);
    int lh = label_row_h();
    struct sbuf v;
    const char *name = ctl == CTL_SCALE ? "Interface scale" : "Pointer speed";
    int vw, my;

    sb_reset(&v);
    sb_u(&v, (unsigned)(ctl == CTL_SCALE ? S.scale : S.speed));
    sb_s(&v, ctl == CTL_SCALE ? "x" : " %");

    ui_text(LO.ix, ly + (lh - ui_text_h(UI_MD)) / 2, name,
            ui_color(UI_COLOR_TEXT), UI_MD, 0);
    vw = ui_text_w(v.b, UI_SM, UI_F_MONO);
    my = ly + (lh - fb_cell_h()) / 2;
    if (my < ly) my = ly;
    ui_text(LO.ix + LO.iw - vw, my, v.b, ui_color(UI_COLOR_TEXT_DIM),
            UI_SM, UI_F_MONO);

    flow_to(ly + lh + DP(SET_SLIDER_GAP));
    if (ctl == CTL_SCALE) ui_slider(&S.scale, SCALE_MIN, SCALE_MAX);
    else                  ui_slider(&S.speed, SPD_MIN_PCT, SPD_MAX_PCT);
    flow += t->row_h + t->gap;
}

/* 713-721: a 12px label, a row of accent swatches, and an 11px explanatory
 * line - the reference's own two sentences, verbatim.
 *
 * THE SWATCHES ARE CHIPS, NOT COLOUR SQUARES. ui.h has no colour-swatch
 * widget, and drawing five rounded rectangles in each accent here would be
 * hand-rolling one. So each accent is a ui_chip carrying its name, and the
 * SELECTED chip fills with the live accent - which is the colour it names,
 * because settings_apply has already pushed it into the theme. The four
 * unselected colours are therefore not previewed; that is the largest single
 * thing on this page the toolkit cannot currently say. */
static void card_accent(int cy)
{
    int ly = cy + DP(SET_ACC_PY);
    int sy = ly + ui_text_h(UI_MD) + DP(SET_ACC_GAP);
    int hy;

    ui_text(LO.ix, ly, "Accent colour", ui_color(UI_COLOR_TEXT), UI_MD, 0);
    chip_walk(LO.ix, sy, LO.iw, 1);

    hy = sy + LO.chip_rows * ui_chip_h()
       + (LO.chip_rows - 1) * DP(SET_ACC_GAP) + DP(SET_ACC_GAP);
    ui_text(LO.ix, hy, "Amber means warning and red means failure.",
            ui_color(UI_COLOR_TEXT_5), UI_SM, 0);
    ui_text(LO.ix, hy + ui_text_h(UI_SM),
            "Those two are wired to state, not to this setting.",
            ui_color(UI_COLOR_TEXT_5), UI_SM, 0);
}

/* 722-731: rows of label + track, one ui_toggle each. */
static void card_toggle(int cy, const struct card_def *cd)
{
    const struct ui_theme *t = ui_theme();
    flow_to(cy + DP(ZD_CARD_PY));
    for (int r = 0; r < cd->rows; r++) {
        switch (cd->data) {
        case CTL_SUBPX: ui_toggle("Subpixel text",       &S.subpixel); break;
        case CTL_ANIM:  ui_toggle("Window animations",   &S.anim);     break;
        default:        ui_toggle("Pointer acceleration", &S.accel);   break;
        }
        flow += t->row_h + t->gap;
    }
}

/* 732-736: key/value rows, borderTop on rows 2..n. */
static void card_about(int cy, int which)
{
    int kh = ui_kv_h(), y = cy;
    struct sbuf v;

    if (which == ABOUT_DISPLAY) {
        sb_reset(&v);
        sb_u(&v, fb_pxw()); sb_c(&v, 'x'); sb_u(&v, fb_pxh());
        ui_kv(LO.cx, y, LO.cw, "Framebuffer", v.b,
              ui_color(UI_COLOR_TEXT), 1);
        y += kh;
        sb_reset(&v); sb_u(&v, (unsigned)S.scale); sb_c(&v, 'x');
        ui_kv(LO.cx, y, LO.cw, "UI scale", v.b, ui_color(UI_COLOR_TEXT), 0);
        y += kh;
        sb_reset(&v);
        sb_u(&v, (unsigned)ui_metric(UI_METRIC_ROW_H)); sb_s(&v, " px");
        ui_kv(LO.cx, y, LO.cw, "Row height", v.b, ui_color(UI_COLOR_TEXT), 0);
        return;
    }
    for (int i = 0; i < (int)(sizeof ABOUT_SYS / sizeof ABOUT_SYS[0]); i++) {
        ui_kv(LO.cx, y, LO.cw, ABOUT_SYS[i].k, ABOUT_SYS[i].v,
              ui_color(UI_COLOR_TEXT), i == 0);
        y += kh;
    }
}

/* ---- the one widget sequence ---------------------------------------------- */
static void build_ui(void)
{
    const struct page_def *pg = &PAGES[LO.page];
    int i;

    /* The sidebar, 701-704. It is read from LO.page rather than from S_page,
     * so a nav row firing mid-pass cannot leave the rest of this sequence
     * emitting one page's widgets at another page's coordinates. */
    ui_sidebar(LO.sb_x, LO.sb_y, LO.sb_w, LO.sb_h);
    for (i = 0; i < N_PAGE; i++)
        if (ui_nav_row(LO.nav_x, LO.nav_y + i * LO.nav_step, LO.nav_w,
                       PAGES[i].name, i == LO.page))
            S_page = i;

    /* The content pane, 705-737. */
    ui_text(LO.cx, LO.title_y, pg->name, ui_color(UI_COLOR_TEXT_HI),
            UI_LG, UI_F_BOLD);

    flow = LO.cur_y;
    for (i = 0; i < LO.ncard; i++) {
        const struct card_def *cd = &pg->card[i];
        ui_card(LO.cx, LO.cy[i], LO.cw, LO.ch[i]);
        switch (cd->kind) {
        case CARD_SLIDER: card_slider(LO.cy[i], cd->data);      break;
        case CARD_ACCENT: card_accent(LO.cy[i]);                break;
        case CARD_TOGGLE: card_toggle(LO.cy[i], cd);            break;
        default:          card_about(LO.cy[i], cd->data);       break;
        }
    }

    ui_text(LO.cx, LO.note_y, pg->note, ui_color(UI_COLOR_TEXT_5), UI_SM, 0);
}

/* THE ONLY WAY THIS APP IS EVER RUN. It computes the layout, sets the cursor
 * origin from it and emits the sequence - three things that used to be spread
 * over settings_draw and two branches of settings_event, which is exactly how
 * a draw pass and a hit-test pass drift apart. */
static void run_ui(int x, int y, int w, int h,
                   int mode, int px, int py, int click)
{
    int pad = ui_metric(UI_METRIC_PAD);
    flow_fault = 0;
    layout(x, y, w, h, S_page);
    ui_begin(LO.cur_x - pad, LO.cur_y - pad, LO.cur_w + 2 * pad, h,
             mode, px, py, click);
    build_ui();
}

/* ---- persistence ----------------------------------------------------------
 * THE FIRST CODE IN THIS PROJECT THAT WRITES TO A DISK.
 *
 * The NVMe namespace is a dedicated scratch image (try.sh makes a 64 MiB
 * /tmp/zlos-nvme.img and attaches it as -device nvme); the boot disk is a
 * separate -drive. So a wrong LBA here cannot destroy the bootable image. That
 * is worth knowing and is NOT a reason to be casual: it is still the first
 * write path, and every guard below exists because the failure it prevents is
 * silent.
 *
 * THE BLOCK
 *
 *   0   4   magic     'z' 'l' 'S' '1'
 *   4   2   version
 *   6   2   count     how many u32 fields follow
 *   8   4   checksum  FNV-1a over the whole record with THIS FIELD ZEROED
 *   12  4n  the settings, one u32 each
 *
 * Rules, straight from FEEL-PROMPT item 4:
 *   - a fixed LBA, a magic number, a version field and a checksum
 *   - REFUSE a block whose magic or checksum is wrong, AND SAY SO
 *   - never write on boot; only when a setting changes
 *
 * And one the brief does not name, because it is the one that bites later:
 * A VALID CHECKSUM IS NOT A VALID VALUE. A block written by a future version,
 * or a bit flip that happens to land on a colliding checksum, can carry a UI
 * scale of two billion. Every field is clamped on the way in, by the same
 * apply path that clamps everything else.
 */
void zl_putc_pub(char c);
int  nvme_ready(void);
int  nvme_read_block(unsigned lba_lo, unsigned lba_hi);
int  nvme_write_block(unsigned lba_lo, unsigned lba_hi);
int  nvme_data_byte(int i);
void nvme_data_set(int i, int v);
unsigned nvme_blocksize(void);
unsigned nvme_blocks_lo(void);
unsigned nvme_blocks_hi(void);

/* zlfs is the ordinary persistence contract.  Weak keeps the standalone
 * settings harness and early builds usable; the raw scratch-sector format is
 * migration fallback only when no mounted zlfs exists. */
extern int fs_mounted(void) __attribute__((weak));
extern int fs_find(const char *name) __attribute__((weak));
extern int fs_create(const char *name, unsigned bytes) __attribute__((weak));
extern int fs_write(int idx, const void *src, unsigned bytes) __attribute__((weak));
extern int fs_read(int idx, void *dst, unsigned max) __attribute__((weak));
extern int fs_sync(void) __attribute__((weak));
#define SETTINGS_FILE "/system/settings"

static void s_puts(const char *s) { while (*s) zl_putc_pub(*s++); }

static void s_putu(unsigned v)
{
    char b[12];
    int n = 0;
    if (!v) b[n++] = '0';
    while (v && n < 11) { b[n++] = (char)('0' + v % 10u); v /= 10u; }
    while (n) zl_putc_pub(b[--n]);
}

/* LBA 2048 WAS THE WORST POSSIBLE CHOICE AND THE COMMENT DEFENDING IT WAS
 * EXACTLY BACKWARDS.
 *
 * It said "far enough from block 0 that anything that later wants a superblock
 * does not land on us". LBA 2048 is not a gap: it is the 1 MiB alignment
 * boundary that parted, gdisk, fdisk and every Linux and Windows installer
 * since about 2010 uses as the start of the first partition. It is the single
 * most-collided sector on a partitioned disk.
 *
 * On the machine this is being written on:
 *
 *   $ cat /sys/block/nvme0n1/nvme0n1p1/start
 *   2048
 *   $ lsblk -o NAME,START,PARTTYPENAME
 *   nvme0n1p1   2048   EFI System
 *
 * So the write would have landed on the EFI System Partition's boot sector of
 * a 477 GB system disk. Found by the adversarial review FEEL-PROMPT §6 asks for
 * on exactly this code, and it was right: the author anchored on the QEMU
 * scratch image and never asked which device the driver actually picks.
 *
 * THE LBA IS NOT THE SAFETY MECHANISM. set_device_ok's refusal is. This is
 * simply a block no partitioner claims, on a disk we have already established
 * is not partitioned at all. */
#define SET_LBA      64u
#define SET_MAGIC0   'z'
#define SET_MAGIC1   'l'
#define SET_MAGIC2   'S'
#define SET_MAGIC3   '1'
#define SET_VERSION  1u
#define SET_COUNT    6u                       /* the six fields of struct settings */
#define SET_BYTES    (12u + 4u * SET_COUNT)   /* 36 */

/* FNV-1a, 32-bit. Not a CRC: this guards against a torn or stale block, not
 * against an adversary, and it is twelve lines instead of a table. */
static unsigned set_hash(const unsigned char *p, unsigned n)
{
    unsigned h = 2166136261u;
    for (unsigned i = 0; i < n; i++) { h ^= p[i]; h *= 16777619u; }
    return h;
}

static void put32(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v & 0xFF);       p[1] = (unsigned char)((v >> 8) & 0xFF);
    p[2] = (unsigned char)((v >> 16) & 0xFF); p[3] = (unsigned char)((v >> 24) & 0xFF);
}

static unsigned get32(const unsigned char *p)
{
    return (unsigned)p[0] | ((unsigned)p[1] << 8)
         | ((unsigned)p[2] << 16) | ((unsigned)p[3] << 24);
}

/* Is the fixed LBA actually inside this namespace? A namespace smaller than
 * the block we picked would otherwise mean writing past the end of the device,
 * which NVMe reports as an error but which is worth not attempting. */
static int set_lba_ok(void)
{
    if (nvme_blocks_hi()) return 1;              /* > 4 G blocks: certainly fine */
    return nvme_blocks_lo() > SET_LBA;
}

/* IS THIS DISK OURS TO WRITE TO?
 *
 * This is the guard that actually matters, and its absence was a
 * data-destroying bug. The original code justified itself with "the NVMe
 * namespace is a dedicated scratch image (try.sh makes a 64 MiB
 * /tmp/zlos-nvme.img)". That is true of try.sh and of nothing else. It is a
 * property of one shell script, not of this code.
 *
 * nvme_find() takes the FIRST PCI device with class 01 / subclass 08 /
 * prog-if 02 and stops - no filter on model, serial or size. On the ThinkPad
 * test laptop that is the internal 477 GB system SSD, and install-esp.sh is a
 * documented, supported way to boot zlOS on that laptop from its own ESP. So
 * "open Settings, click a control" would have overwritten the EFI System
 * Partition's boot sector and the machine would not have booted again.
 *
 * Two refusals, both cheap, both checked before EVERY write rather than once:
 *
 *   PARTITIONED  bytes 510/511 of LBA 0 are 0x55 0xAA on any disk carrying an
 *                MBR or a GPT protective MBR, and LBA 1 of a GPT disk starts
 *                "EFI PART". A disk with a partition table belongs to someone
 *                else. The scratch image qemu-img creates is all zeros.
 *   TOO BIG      try.sh makes 64 MiB. Anything past a gigabyte is somebody's
 *                real disk, and refusing it costs nothing.
 *
 * Neither is clever and neither needs to be: the failure they prevent is
 * unrecoverable and the check is two reads.
 */
#define SET_MAX_BLOCKS  (2u * 1024u * 1024u)   /* 1 GiB at 512-byte blocks */

static int set_disk_is_ours(const char *what)
{
    /* NOTE: this reads into the shared transfer page, so it must run BEFORE
     * settings_save fills that page with the record. */
    if (!nvme_read_block(0, 0)) {
        s_puts("  settings: cannot read LBA 0, "); s_puts(what);
        s_puts(" skipped\n");
        return 0;
    }
    if ((nvme_data_byte(510) & 0xFF) == 0x55 && (nvme_data_byte(511) & 0xFF) == 0xAA) {
        s_puts("  settings: this disk has a PARTITION TABLE - refusing to write. ");
        s_puts(what); s_puts(" skipped\n");
        return 0;
    }
    if (!nvme_read_block(1, 0)) {
        s_puts("  settings: cannot read LBA 1, "); s_puts(what);
        s_puts(" skipped\n");
        return 0;
    }
    {
        static const char gpt[8] = { 'E','F','I',' ','P','A','R','T' };
        int match = 1;
        for (int i = 0; i < 8; i++)
            if ((nvme_data_byte(i) & 0xFF) != (int)(unsigned char)gpt[i]) { match = 0; break; }
        if (match) {
            s_puts("  settings: this disk is GPT - refusing to write. ");
            s_puts(what); s_puts(" skipped\n");
            return 0;
        }
    }
    if (nvme_blocks_hi() || nvme_blocks_lo() > SET_MAX_BLOCKS) {
        s_puts("  settings: namespace is too large to be the zlOS scratch disk"
               " - refusing to write. ");
        s_puts(what); s_puts(" skipped\n");
        return 0;
    }
    return 1;
}

static int set_device_ok(const char *what)
{
    if (!nvme_ready()) {
        s_puts("  settings: no NVMe, "); s_puts(what); s_puts(" skipped\n");
        return 0;
    }
    if (nvme_blocksize() < SET_BYTES) {
        s_puts("  settings: block size too small, "); s_puts(what); s_puts(" skipped\n");
        return 0;
    }
    /* ...and bounded ABOVE, which the first version missed. nvme.c transfers
     * one block into a single 4 KiB page and never programs PRP2, so an LBA
     * format larger than a page would read or write past that page - into
     * whatever follows NMEM_DATA. Refusing is the only safe answer here;
     * supporting it is a change to the driver, not to this file. */
    if (nvme_blocksize() > 4096u) {
        s_puts("  settings: block size larger than one page, ");
        s_puts(what); s_puts(" skipped\n");
        return 0;
    }
    if (!set_lba_ok()) {
        s_puts("  settings: LBA "); s_putu(SET_LBA);
        s_puts(" is past the end of the namespace, "); s_puts(what);
        s_puts(" skipped\n");
        return 0;
    }
    /* ...and last, because it costs two reads: is this disk ours at all? */
    return set_disk_is_ours(what);
}

/* Write the current settings. Called ONLY from settings_commit - never on
 * boot, never on load. */
int settings_save(void)
{
    unsigned char rec[SET_BYTES];
    rec[0] = SET_MAGIC0; rec[1] = SET_MAGIC1;
    rec[2] = SET_MAGIC2; rec[3] = SET_MAGIC3;
    rec[4] = (unsigned char)(SET_VERSION & 0xFF);
    rec[5] = (unsigned char)((SET_VERSION >> 8) & 0xFF);
    rec[6] = (unsigned char)(SET_COUNT & 0xFF);
    rec[7] = (unsigned char)((SET_COUNT >> 8) & 0xFF);
    put32(rec + 8, 0);                      /* checksum field zeroed while hashing */
    put32(rec + 12, (unsigned)S.accent);
    put32(rec + 16, (unsigned)S.scale);
    put32(rec + 20, (unsigned)S.speed);
    put32(rec + 24, (unsigned)S.accel);
    put32(rec + 28, (unsigned)S.subpixel);
    put32(rec + 32, (unsigned)S.anim);
    put32(rec + 8, set_hash(rec, SET_BYTES));

    if (fs_mounted && fs_find && fs_create && fs_write && fs_mounted()) {
        int idx = fs_find(SETTINGS_FILE);
        if (idx < 0) idx = fs_create(SETTINGS_FILE, SET_BYTES);
        if (idx < 0 || !fs_write(idx, rec, SET_BYTES)) {
            s_puts("  settings: zlfs write FAILED, not saved\n");
            return 0;
        }
        if (!fs_sync || !fs_sync()) {
            s_puts("  settings: zlfs flush FAILED, not durable\n");
            return 0;
        }
        return 1;
    }

    if (!set_device_ok("save")) return 0;

    /* ZERO THE WHOLE TRANSFER PAGE FIRST. nvme_data is shared with every other
     * user of the driver, so whatever the last read left there would otherwise
     * be written to the disk after our 36 bytes - leaking unrelated memory into
     * a block that is supposed to be ours, and making the block's tail differ
     * run to run for no reason. */
    unsigned bs = nvme_blocksize();
    if (bs > 4096u) bs = 4096u;
    for (unsigned i = 0; i < bs; i++) nvme_data_set((int)i, 0);
    for (unsigned i = 0; i < SET_BYTES; i++) nvme_data_set((int)i, rec[i]);

    if (!nvme_write_block(SET_LBA, 0)) {
        s_puts("  settings: NVMe write FAILED, not saved\n");
        return 0;
    }
    return 1;
}

/* Read them back. Returns 1 if a good block was found and applied, 0 if the
 * defaults are in force - and in every 0 case it has printed WHY.
 *
 * Never writes. A load that repaired the block would turn a read-only boot into
 * a write, which is exactly what "never write on boot" forbids. */
int settings_load(void)
{
    unsigned char rec[SET_BYTES];
    int from_zlfs = 0;
    if (fs_mounted && fs_find && fs_read && fs_mounted()) {
        int idx = fs_find(SETTINGS_FILE);
        if (idx >= 0) {
            if (fs_read(idx, rec, SET_BYTES) != SET_BYTES) {
                s_puts("  settings: short zlfs settings file, using defaults\n");
                return 0;
            }
            from_zlfs = 1;
        }
    }

    if (!from_zlfs) {
        if (!set_device_ok("load")) return 0;

        if (!nvme_read_block(SET_LBA, 0)) {
            s_puts("  settings: NVMe read failed, using defaults\n");
            return 0;
        }

        for (unsigned i = 0; i < SET_BYTES; i++)
            rec[i] = (unsigned char)nvme_data_byte((int)i);
    }

    if (rec[0] != SET_MAGIC0 || rec[1] != SET_MAGIC1 ||
        rec[2] != SET_MAGIC2 || rec[3] != SET_MAGIC3) {
        s_puts("  settings: no valid block at LBA "); s_putu(SET_LBA);
        s_puts(" (bad magic), using defaults\n");
        return 0;
    }

    unsigned ver = (unsigned)rec[4] | ((unsigned)rec[5] << 8);
    unsigned cnt = (unsigned)rec[6] | ((unsigned)rec[7] << 8);
    if (ver != SET_VERSION || cnt != SET_COUNT) {
        s_puts("  settings: block is version "); s_putu(ver);
        s_puts("/"); s_putu(cnt);
        s_puts(", this kernel wants "); s_putu(SET_VERSION);
        s_puts("/"); s_putu(SET_COUNT);
        s_puts(", using defaults\n");
        return 0;
    }

    unsigned stored = get32(rec + 8);
    put32(rec + 8, 0);
    if (set_hash(rec, SET_BYTES) != stored) {
        s_puts("  settings: CHECKSUM MISMATCH at LBA "); s_putu(SET_LBA);
        s_puts(", using defaults\n");
        return 0;
    }

    /* A good checksum proves the bytes are the ones that were written. It
     * proves nothing about whether they are sensible - so every field goes
     * through the same clamping settings_apply already does. */
    S.accent   = (int)get32(rec + 12);
    S.scale    = (int)get32(rec + 16);
    S.speed    = (int)get32(rec + 20);
    S.accel    = (int)get32(rec + 24);
    S.subpixel = (int)get32(rec + 28);
    S.anim     = (int)get32(rec + 32);

    /* No clamping here on purpose - settings_apply is the one clamp, and it
     * runs on the next line. A second copy is a second thing to keep in step,
     * and keeping it in step is exactly what failed last time. */
    settings_apply();
    return 1;
}

/* ---- the app hooks -------------------------------------------------------
 * Signatures match app_draw_fn / app_event_fn in ui.h exactly, so these can be
 * handed to wm_hooks directly or dispatched to from wmglue. */
void settings_draw(int app, int x, int y, int w, int h, int focused)
{
    (void)app; (void)focused;
    /* click = 0: a draw pass must never fire a widget. See the header. */
    run_ui(x, y, w, h, UI_DRAW, -1, -1, 0);
}

/* Which control the keyboard is on. The app owns it, like every other piece of
 * widget state - ui.c only remembers the number so it survives between the
 * hit-test pass and the draw pass. */
static int focus = -1;

int settings_event(int app, int win, int type, int code, int x, int y)
{
    (void)app;

    /* ---- keyboard ---------------------------------------------------------
     * Tab walks the controls, Enter presses the one with the ring. Without
     * this the ring would be an indicator of something the user cannot move,
     * which is worse than no indicator at all. */
    if (type == EV_KEY_DOWN && (code == KEY_TAB || code == KEY_ENTER)) {
        int cx, cy, cw, ch;
        wm_client(win, &cx, &cy, &cw, &ch);
        if (cw <= 0 || ch <= 0) return 0;

        int n = ui_widget_count();
        if (code == KEY_TAB) {
            /* n is whatever the LAST pass counted, so it already follows the
             * layout at the current scale rather than a number written here. */
            if (n > 0) focus = (focus + 1) % n;
            ui_set_focus(focus);
            wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
            return 1;
        }
        if (focus < 0) return 1;
        /* Enter: re-run the SAME sequence with the activation flag set, so the
         * focused widget fires through fire() exactly as a click would. */
        struct settings before = S;
        int page_before = S_page;
        ui_activate_focus();
        run_ui(cx, cy, cw, ch, UI_HITTEST, -1, -1, 0);
        ui_end_activate();
        if (before.accent   != S.accent   || before.scale != S.scale ||
            before.speed    != S.speed    || before.accel != S.accel ||
            before.subpixel != S.subpixel || before.anim  != S.anim) {
            settings_commit();
            settings_flush();       /* a key press is a whole gesture on its own */
        } else if (S_page != page_before) {
            /* A page change repaints and NOTHING ELSE. It touches no sink and
             * it is not persisted, so applying or dirtying here would write a
             * disk block because someone looked at another pane. */
            wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
        }
        return 1;
    }

    if (type != EV_MOUSE) return 0;
    if (!(code & 1)) {
        /* Button up: the gesture is over, so persist whatever it changed.
         * wm.c holds a pointer grab for the whole press and delivers an event
         * per motion, so this is the only place a write belongs. */
        settings_flush();
        return 0;
    }

    int cx, cy, cw, ch;
    wm_client(win, &cx, &cy, &cw, &ch);
    if (cw <= 0 || ch <= 0) return 0;

    /* Snapshot, run the SAME sequence with hit-testing on, and commit only if
     * something actually moved. Committing unconditionally would repaint the
     * entire screen on every click that landed on a label or on empty space,
     * which at 2560x1440 is a full frame thrown away per stray click. */
    struct settings before = S;
    int page_before = S_page;
    run_ui(cx, cy, cw, ch, UI_HITTEST, x, y, 1);

    int changed = before.accent   != S.accent
               || before.scale    != S.scale
               || before.speed    != S.speed
               || before.accel    != S.accel
               || before.subpixel != S.subpixel
               || before.anim     != S.anim;
    if (changed) settings_commit();
    /* A nav row moved the sidebar selection: repaint, but do NOT apply and do
     * NOT mark the block dirty. Which pane is open is not a setting. */
    else if (S_page != page_before) wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
    /* Clicking a control moves the keyboard focus onto it. Leaving the ring
     * behind on whatever was last Tabbed to makes the next Enter press a
     * different control than the one the user is looking at. */
    if (ui_fired() >= 0) { focus = ui_fired(); ui_set_focus(focus); }
    return ui_fired() >= 0;
}
