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
 * function is the only version of this that cannot rot.
 *
 * THE HIT-TEST PASS IS THE ONLY ONE WITH click SET. ui.c's fire() is level
 * triggered on L.click, and ui_toggle flips its variable inside fire(), so a
 * control visited twice with the button down toggles twice and nothing appears
 * to happen. Draw passes therefore always pass click = 0 - they are for
 * looking, not deciding.
 */

#include "ui.h"

/* ---- what this file drives ------------------------------------------------
 * Six sinks, all of which already existed. Only wm_set_anim is new, because
 * ANIM_FRAMES was a compile-time constant with no runtime switch. */
void fb_set_subpixel(int on);
unsigned int fb_pxw(void);
unsigned int fb_pxh(void);
void input_set_speed(int pct);
void input_set_accel(int on);
void wm_set_anim(int on);

#define EV_MOUSE 4          /* wm.c's event type, same numbering as input.c */

/* ---- the accent palette ---------------------------------------------------
 * Named, not a colour picker: there is no colour-picker widget and building one
 * would be the mechanism this file is supposed to avoid. Every entry is a
 * saturated colour of roughly equal luminance against the dark surfaces, so
 * switching accent never changes how readable anything is - which is the
 * property a palette has to have and a free colour picker cannot promise. */
static const struct { const char *name; unsigned rgb; } ACCENTS[] = {
    { "Ice",     0x55D6FF },     /* the default - unchanged from ui_theme_init */
    { "Mint",    0x4FE0B0 },
    { "Amber",   0xFFB454 },
    { "Rose",    0xFF7A9C },
    { "Violet",  0xB08CFF },
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
    if (S.scale < 1) S.scale = 1;
    if (S.accent < 0) S.accent = 0;
    if (S.accent >= N_ACCENT) S.accent = N_ACCENT - 1;

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

static void settings_commit(void)
{
    settings_apply();
    wm_damage(0, 0, (int)fb_pxw(), (int)fb_pxh());
}

/* ---- the one widget sequence ----------------------------------------------
 * IT HAS TO FIT. ui.c's place() advances a cursor and never reports running
 * out of room: a widget past the bottom of the client area is still laid out,
 * still counted for widget identity, and simply drawn outside the scissor -
 * so it is invisible AND unclickable, with nothing anywhere saying so.
 *
 * The first draft used a full-width ui_list_row per accent and a ui_num beside
 * every slider. At ui scale 2 that came to roughly 995 px of content in a
 * 642 px client, so everything from "Interface scale" down was off the bottom
 * and could not be reached at all. It looked like a Settings window with two
 * controls.
 *
 * So the accents are a ROW of buttons rather than five stacked rows - 112 px
 * instead of 368 - and only the reading that is not obvious from the slider
 * position keeps its ui_num. See docs/desktop-feel.md; the silent overflow
 * itself is logged as a finding against ui.c.
 */
static void build_ui(void)
{
    ui_label("Accent");
    ui_row();
    for (int i = 0; i < N_ACCENT; i++)
        if (ui_button(ACCENTS[i].name)) S.accent = i;
    ui_endrow();

    ui_sep();
    ui_label("Interface scale");
    ui_slider(&S.scale, 1, 3);

    ui_sep();
    ui_label("Pointer speed");
    ui_slider(&S.speed, 25, 400);
    ui_num("percent", S.speed);
    ui_toggle("Pointer acceleration", &S.accel);

    ui_sep();
    ui_toggle("Subpixel text", &S.subpixel);
    ui_toggle("Window animations", &S.anim);
}

/* ---- the app hooks -------------------------------------------------------
 * Signatures match app_draw_fn / app_event_fn in ui.h exactly, so these can be
 * handed to wm_hooks directly or dispatched to from wmglue. */
void settings_draw(int app, int x, int y, int w, int h, int focused)
{
    (void)app; (void)focused;
    /* click = 0: a draw pass must never fire a widget. See the header. */
    ui_begin(x, y, w, h, UI_DRAW, -1, -1, 0);
    build_ui();
}

int settings_event(int app, int win, int type, int code, int x, int y)
{
    (void)app;
    if (type != EV_MOUSE) return 0;
    if (!(code & 1)) return 0;              /* left button down only */

    int cx, cy, cw, ch;
    wm_client(win, &cx, &cy, &cw, &ch);
    if (cw <= 0 || ch <= 0) return 0;

    /* Snapshot, run the SAME sequence with hit-testing on, and commit only if
     * something actually moved. Committing unconditionally would repaint the
     * entire screen on every click that landed on a label or on empty space,
     * which at 2560x1440 is a full frame thrown away per stray click. */
    struct settings before = S;
    ui_begin(cx, cy, cw, ch, UI_HITTEST, x, y, 1);
    build_ui();

    int changed = before.accent   != S.accent
               || before.scale    != S.scale
               || before.speed    != S.speed
               || before.accel    != S.accel
               || before.subpixel != S.subpixel
               || before.anim     != S.anim;
    if (changed) settings_commit();
    return ui_fired() >= 0;
}
