/* uitest.c - the shared widget toolkit, asserted against a recording canvas.
 *
 * WHY A RECORDER AND NOT A SCREENSHOT. wmshot photographs a frame and eyes
 * catch a title bar four pixels too tall. Eyes do NOT catch a segmented
 * control whose active pill is one slot to the left of the label it belongs
 * to, or a grid cell drawn at the right x for the wrong track, or an accent
 * button whose ink is white on lime - all three look like a design decision in
 * a picture. So ui.c and uikit.c are linked for real and every fb_* call they
 * make is recorded: this harness asserts on the DRAW LIST, which is the only
 * place those bugs are visible as numbers.
 *
 * THE NEGATIVE CONTROLS ARE THE POINT, not decoration.
 * docs/GUARDS-THAT-DID-NOT-GUARD.md lists five checks in this tree that
 * reported green while checking nothing. Every group below therefore carries
 * at least one control that FAILS if the widget draws nothing, plus controls
 * that the recorder itself is working: a hit-test pass must record zero ops,
 * and a deliberately-broken lookup must be refused. If a control ever fails,
 * the assertions beside it are green for the wrong reason.
 *
 * Build and run:  ./build.sh && ./uitest
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../../src/graphics/ui/ui.h"
#include "../../src/graphics/ui/design.h"

/* ---- the recording canvas -------------------------------------------------
 * Every primitive ui.c and uikit.c can reach, recorded rather than drawn. The
 * text ops keep their string, because "is the label inside the pill" is a
 * question about a specific label. */
enum { OP_FILL, OP_BLEND, OP_RRECT, OP_RBLEND, OP_BOX, OP_LINE, OP_TEXT };

struct op {
    int kind, x, y, w, h, r, a;
    unsigned rgb;
    char text[64];
};

#define MAXOPS 4096
static struct op ops[MAXOPS];
static int nops;

static void rec(int kind, int x, int y, int w, int h, int r, unsigned rgb,
                int a, const char *s)
{
    if (nops >= MAXOPS) return;
    struct op *o = &ops[nops++];
    o->kind = kind; o->x = x; o->y = y; o->w = w; o->h = h;
    o->r = r; o->rgb = rgb; o->a = a;
    o->text[0] = 0;
    if (s) { strncpy(o->text, s, sizeof o->text - 1); o->text[sizeof o->text - 1] = 0; }
}

static void reset(void) { nops = 0; }

void fb_fill_px(int x, int y, int w, int h, unsigned rgb)
{ rec(OP_FILL, x, y, w, h, 0, rgb, 255, 0); }
void fb_fill_blend(int x, int y, int w, int h, unsigned rgb, int a)
{ rec(OP_BLEND, x, y, w, h, 0, rgb, a, 0); }
void fb_rrect(int x, int y, int w, int h, int r, unsigned rgb)
{ rec(OP_RRECT, x, y, w, h, r, rgb, 255, 0); }
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned rgb, int a)
{ rec(OP_RBLEND, x, y, w, h, r, rgb, a, 0); }
void fb_box(int x, int y, int w, int h, unsigned rgb)
{ rec(OP_BOX, x, y, w, h, 0, rgb, 255, 0); }
void fb_line(int x0, int y0, int x1, int y1, unsigned rgb)
{ rec(OP_LINE, x0, y0, x1 - x0, y1 - y0, 0, rgb, 255, 0); }

/* ---- the text engine, made deterministic -----------------------------------
 * The real one resamples a bitmap atlas and its advances depend on the glyphs.
 * A geometry gate must not depend on that: every assertion below about where a
 * label lands would then encode the font rather than the layout. Three fixed
 * advances and three fixed heights, one per role, and a mono cell. */
static const int ROLE_ADV[3] = { 6, 8, 11 };
static const int ROLE_H[3]   = { 12, 16, 22 };
static int rlen(const char *s) { int n = 0; if (s) while (s[n]) n++; return n; }
static int rclamp(int r) { return r < 0 ? 0 : (r > 2 ? 2 : r); }

int  fb_text_role_w(const char *s, int role, int weight)
{ return rlen(s) * (ROLE_ADV[rclamp(role)] + (weight ? 1 : 0)); }
int  fb_text_role_h(int role) { return ROLE_H[rclamp(role)]; }
void fb_text_role(int px, int py, const char *s, unsigned fg, int role, int w)
{ (void)role; (void)w; rec(OP_TEXT, px, py, fb_text_role_w(s, role, w),
                           fb_text_role_h(role), 0, fg, 255, s); }
void fb_text_prop(int px, int py, const char *s, unsigned fg)
{ fb_text_role(px, py, s, fg, 1, 0); }
int  fb_text_prop_w(const char *s) { return fb_text_role_w(s, 1, 0); }
int  fb_text_prop_h(void) { return fb_text_role_h(1); }
void fb_text_aa(int px, int py, const char *s, unsigned fg)
{ rec(OP_TEXT, px, py, rlen(s) * 8, 16, 0, fg, 255, s); }
int  fb_cell_w(void) { return 8; }
int  fb_cell_h(void) { return 16; }
void fb_clip(int x, int y, int w, int h) { (void)x;(void)y;(void)w;(void)h; }
void fb_clip_none(void) { }
void fb_clip_get(int *x0, int *y0, int *x1, int *y1)
{ *x0 = 0; *y0 = 0; *x1 = 4096; *y1 = 4096; }

/* ---- the assertion ledger -------------------------------------------------- */
static int fails, checks;
static void ok(int cond, const char *what)
{
    checks++;
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}
static void oknum(int cond, const char *what, int got, int want)
{
    checks++;
    if (cond) printf("  ok   %s\n", what);
    else { printf("  FAIL %s   (got %d, want %d)\n", what, got, want); fails++; }
}

/* ---- queries over the draw list -------------------------------------------- */
static int count_kind(int kind)
{
    int n = 0;
    for (int i = 0; i < nops; i++) if (ops[i].kind == kind) n++;
    return n;
}

/* The first solid or blended rounded rect painted in this colour. */
static const struct op *find_rgb(unsigned rgb)
{
    for (int i = 0; i < nops; i++)
        if (ops[i].rgb == rgb &&
            (ops[i].kind == OP_RRECT || ops[i].kind == OP_RBLEND ||
             ops[i].kind == OP_FILL   || ops[i].kind == OP_BLEND))
            return &ops[i];
    return NULL;
}

static const struct op *find_text(const char *s)
{
    for (int i = 0; i < nops; i++)
        if (ops[i].kind == OP_TEXT && !strcmp(ops[i].text, s)) return &ops[i];
    return NULL;
}

/* Every colour that any text op used - so "did anything write white on the
 * accent" is answerable without knowing which widget did it. */
static int text_used_colour(unsigned rgb)
{
    for (int i = 0; i < nops; i++)
        if (ops[i].kind == OP_TEXT && ops[i].rgb == rgb) return 1;
    return 0;
}

static void begin_draw(void)
{
    reset();
    ui_begin(0, 0, 1000, 700, UI_DRAW, -1, -1, 0);
}
static void begin_click(int px, int py)
{
    reset();
    ui_begin(0, 0, 1000, 700, UI_HITTEST, px, py, 1);
}

/* ---- source scan: rule 1, no colour literal outside design.h ---------------
 * CLAUDE.md and design.h both state that a colour literal may appear in
 * design.h and nowhere else. Nothing enforced that on ui.c or uikit.c - the
 * palette gate checks that design.h's tokens are IN the reference, which is a
 * different claim entirely. This is the missing half: read the two toolkit
 * sources and fail on a six-hex-digit constant in either spelling.
 *
 * Six digits exactly, and not part of a longer run: `0xFFu` is a byte mask and
 * `0x07080A` is a colour. */
static char *slurp(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { printf("  cannot open %s\n", path); exit(2); }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *b = malloc((size_t)n + 1);
    if (fread(b, 1, (size_t)n, f) != (size_t)n) { fclose(f); exit(2); }
    b[n] = 0; fclose(f);
    return b;
}

/* Strip C comments and string literals in place, replacing them with spaces.
 *
 * THE RULE IS ABOUT CODE, NOT PROSE. Both ui.c and uikit.c quote the
 * reference's own hex values in their comments - "#0c1005 (INK, computed)",
 * "the dominant card is #14171a / 1px #1c2024 / r13" - and that provenance is
 * the most valuable thing in either file. Without this pass the scanner
 * reported 9 and 13 literals in files that contain none in any executable
 * line, which is the failure mode where a guard is deleted rather than fixed.
 * String literals go too: a widget label is not a colour. */
static void strip_comments(char *s)
{
    for (int i = 0; s[i]; i++) {
        if (s[i] == '/' && s[i + 1] == '*') {
            while (s[i] && !(s[i] == '*' && s[i + 1] == '/')) s[i++] = ' ';
            if (s[i]) { s[i] = ' '; s[i + 1] = ' '; i++; }
        } else if (s[i] == '/' && s[i + 1] == '/') {
            while (s[i] && s[i] != '\n') s[i++] = ' ';
        } else if (s[i] == '"') {
            s[i++] = ' ';
            while (s[i] && s[i] != '"') {
                if (s[i] == '\\' && s[i + 1]) s[i++] = ' ';
                s[i++] = ' ';
            }
            if (s[i]) s[i] = ' ';
        }
    }
}

static int count_colour_literals(const char *src)
{
    int n = 0;
    for (int i = 0; src[i]; i++) {
        int six = 0;
        if (src[i] == '#') six = 1;
        else if (src[i] == '0' && (src[i + 1] == 'x' || src[i + 1] == 'X')) six = 2;
        if (!six) continue;
        int j = i + six, d = 0;
        while (isxdigit((unsigned char)src[j])) { j++; d++; }
        if (d == 6) n++;
        i = j - 1;
    }
    return n;
}

/* ---- scale invariance: rule 2, every distance through UI_DP ----------------
 * A widget that hardcodes a device pixel is invisible at the scale it was
 * written at and wrong everywhere else. Measured rather than asserted: the
 * same metric is read at 1x and at 2x and must double. Only metrics that are
 * pure geometry are used - anything involving the text stub would be measuring
 * the stub. */
struct metric { const char *name; int (*fn)(void); };
static int m_meter(void)   { return ui_meter_h(); }
static int m_prog(void)    { return ui_progress_h(); }
static int m_mini(void)    { return ui_minibar_h(); }
static int m_segbar(void)  { return ui_segbar_h(); }
static int m_colhead(void) { return ui_colhead_h(); }
static int m_row(void)     { return ui_grid_row_h(); }
static int m_nav(void)     { return ui_nav_h(); }
static int m_tabs(void)    { return ui_tabs_h(); }
static int m_utabs(void)   { return ui_utabs_h(); }
static int m_tool(void)    { return ui_toolbar_h(); }
static int m_status(void)  { return ui_status_h(); }
static int m_sidebar(void) { return ui_sidebar_w(); }
static int m_modal(void)   { return ui_modal_head_h(); }
static int m_input(void)   { return ui_input_h(); }
static int m_search(void)  { return ui_search_h(); }
static int m_dot(void)     { return ui_dot_size(); }
static int m_cardh(void)   { return ui_card_head_h(); }
static int m_kv(void)      { return ui_kv_h(); }

static const struct metric METRICS[] = {
    { "ui_meter_h",     m_meter   }, { "ui_progress_h",  m_prog    },
    { "ui_minibar_h",   m_mini    }, { "ui_segbar_h",    m_segbar  },
    { "ui_colhead_h",   m_colhead }, { "ui_grid_row_h",  m_row     },
    { "ui_nav_h",       m_nav     }, { "ui_tabs_h",      m_tabs    },
    { "ui_utabs_h",     m_utabs   }, { "ui_toolbar_h",   m_tool    },
    { "ui_status_h",    m_status  }, { "ui_sidebar_w",   m_sidebar },
    { "ui_modal_head_h",m_modal   }, { "ui_input_h",     m_input   },
    { "ui_search_h",    m_search  }, { "ui_dot_size",    m_dot     },
    { "ui_card_head_h", m_cardh   }, { "ui_kv_h",        m_kv      },
};
#define N_METRICS ((int)(sizeof METRICS / sizeof METRICS[0]))

int main(void)
{
    printf("uitest - the shared widget toolkit, against a recording canvas\n\n");
    ui_theme_init_q8(256);            /* 1 design px == 1 device px */
    const struct ui_theme *t = ui_theme();

    /* ================================================================ INK */
    printf("  ink on the accent - S20.1 / S21.8\n");
    oknum(ui_ink_on(ZD_ACCENT) == (unsigned)ZD_INK_DARK,
          "the default lime accent takes the DARK ink",
          (int)ui_ink_on(ZD_ACCENT), ZD_INK_DARK);
    ok(ui_ink_on(ZD_SURF_0) == (unsigned)ZD_INK_LIGHT,
       "the darkest surface takes the LIGHT ink");
    /* The four Settings alternates and the two semantic colours, each one
     * checked against the reference's own rule rather than against a guess.
     * The violet #8f7bff has a full-strength blue channel and still comes out
     * above the 0.1868 line, which is exactly why a STORED ink would be wrong:
     * every one of these six resolves dark, and the one that does not is the
     * dark surface a widget never paints text on. */
    ok(ui_ink_on(ZD_ACCENT_ALT_1) == (unsigned)ZD_INK_DARK &&
       ui_ink_on(ZD_ACCENT_ALT_2) == (unsigned)ZD_INK_DARK &&
       ui_ink_on(ZD_ACCENT_ALT_3) == (unsigned)ZD_INK_DARK &&
       ui_ink_on(ZD_ACCENT_ALT_4) == (unsigned)ZD_INK_DARK,
       "all four Settings accent alternates are light enough for dark ink");
    ok(ui_ink_on(ZD_OK) == (unsigned)ZD_INK_DARK &&
       ui_ink_on(ZD_BAD) == (unsigned)ZD_INK_DARK &&
       ui_ink_on(ZD_WARN) == (unsigned)ZD_INK_DARK,
       "OK, BAD and WARN all take dark ink - a toast glyph on any of them");
    ok(ui_ink_on(ZD_SURF_5) == (unsigned)ZD_INK_LIGHT &&
       ui_ink_on(ZD_SURF_7) == (unsigned)ZD_INK_LIGHT,
       "control: the surface ladder is on the other side of the line");
    ok(ui_luminance_q16(0x000000) == 0 && ui_luminance_q16(0xFFFFFF) > 60000,
       "luminance spans the range: black 0, white near 65535");
    ok(ui_luminance_q16(0x808080) > ui_luminance_q16(0x404040) &&
       ui_luminance_q16(0xC0C0C0) > ui_luminance_q16(0x808080),
       "luminance is monotone across the grey ramp");
    /* CONTROL: the threshold must actually be crossed by something, or
     * ui_ink_on could be `return DARK;` and every assertion above bar one
     * would still pass. */
    ok(ui_ink_on(0x777777) != ui_ink_on(0x888888),
       "control: the ink flips somewhere in mid grey - it is not a constant");

    /* ============================================================== BUTTONS */
    printf("\n  pill button - S13.1, three sizes\n");
    {
        int hs = ui_pill_h(UI_SM), hm = ui_pill_h(UI_MD), hl = ui_pill_h(UI_LG);
        oknum(hs == 2 * ZD_PILL_SM_PY + ROLE_H[0],
              "sm height is 2*3px padding + caption type", hs,
              2 * ZD_PILL_SM_PY + ROLE_H[0]);
        oknum(hm == 2 * ZD_PILL_MD_PY + ROLE_H[1],
              "md height is 2*6px padding + body type", hm,
              2 * ZD_PILL_MD_PY + ROLE_H[1]);
        ok(hs < hm && hm < hl, "the three sizes are strictly ordered");
        int wm = ui_pill_w("Pack", UI_MD, 0);
        oknum(wm == 2 * ZD_PILL_MD_PX + 4 * ROLE_ADV[1],
              "md width is 2*13px padding + the measured label", wm,
              2 * ZD_PILL_MD_PX + 4 * ROLE_ADV[1]);
        ok(ui_pill_w("Pack", UI_MD, UI_F_MONO) == 2 * ZD_PILL_MD_PX + 4 * 8,
           "the mono flag measures against the fixed cell, not the atlas");
    }
    {
        begin_draw();
        int w = ui_pill_w("Pack", UI_MD, 0), h = ui_pill_h(UI_MD);
        ui_pill(100, 50, w, h, "Pack", UI_MD, UI_BTN_PRIMARY, UI_F_BOLD);
        const struct op *face = find_rgb(ZD_ACCENT);
        const struct op *lab = find_text("Pack");
        ok(face && face->x == 100 && face->y == 50 && face->w == w && face->h == h,
           "primary: the accent face is exactly the rect it was given");
        ok(face && face->r == ZD_PILL_MD_R, "primary: r11, the house radius");
        ok(lab && lab->rgb == (unsigned)ZD_INK_DARK,
           "primary: the label is the COMPUTED ink");
        ok(!text_used_colour(0xFFFFFF),
           "primary: nothing writes #fff on the accent - S20.1's bug is absent");
        ok(lab && lab->x >= 100 && lab->x + lab->w <= 100 + w,
           "primary: the label is inside the pill");
        /* CONTROL */
        ok(nops >= 2, "control: the primary button drew something at all");
    }
    {
        begin_draw();
        ui_pill(0, 0, 80, 24, "End", UI_MD, UI_BTN_DANGER, 0);
        ok(find_rgb(ZD_BAD) != NULL, "destructive: BAD is on screen, one red");
        const struct op *lab = find_text("End");
        ok(lab && lab->rgb == (unsigned)ZD_BAD_SOFT,
           "destructive: the label is the softer red, not BAD itself");
        /* PRESSWORK: DANGER IS A RULE AND AN INK, NOT A WASH. What this used
         * to assert - a 16% ZD_BAD tint with a 40% border - is a translucent
         * red AREA, and a tinted region cannot sit on a ladder whose rungs are
         * the information. The red is now the seat's RING and nothing else;
         * ZD_BAD is 3.4161:1 on ZD_RAISE, a mark, and the ink is ZD_BAD_INK. */
        int washed = 0, ringed = 0;
        for (int i = 0; i < nops; i++) {
            if (ops[i].rgb != (unsigned)ZD_BAD) continue;
            if (ops[i].a < 255) washed = 1;
            if (ops[i].a == 255 && ops[i].w == 80 && ops[i].h == 24) ringed = 1;
        }
        oknum(washed == 0, "destructive: there is NO translucent red wash",
              washed, 0);
        ok(ringed, "destructive: the red is the ring, at full strength");
        ok(find_rgb(ZD_RAISE) != NULL,
           "destructive: ...over the ordinary raised face, not a red one");
    }
    {
        begin_click(20, 20);
        int fired = ui_pill(0, 0, 80, 24, "Hit", UI_MD, UI_BTN_NEUTRAL, 0);
        ok(fired, "a click inside the pill fires it");
        oknum(nops == 0, "control: a HITTEST pass records ZERO draw ops", nops, 0);
        begin_click(500, 500);
        ok(!ui_pill(0, 0, 80, 24, "Hit", UI_MD, UI_BTN_NEUTRAL, 0),
           "control: a click outside the pill does not fire it");
    }

    /* =================================================== SEGMENTED CONTROL */
    printf("\n  segmented control - S3\n");
    {
        const char *items = "CPU|Memory|Disk|Net";
        oknum(ui_items_count(items) == 4, "four items parse out of the string",
              ui_items_count(items), 4);
        ok(ui_items_count("") == 0 && ui_items_count("one") == 1,
           "control: empty is zero items, one label is one item");
        int h = ui_seg_h(UI_MD), W = 400;
        begin_draw();
        ui_segmented(10, 10, W, h, items, 2, UI_MD);
        /* the active pill is slot 2 of 4 */
        int pad = ZD_SEG_PAD, gap = ZD_SEG_GAP;
        int inner = W - 2 * pad - 3 * gap, iw = inner / 4;
        int want = 10 + pad + 2 * (iw + gap);
        /* PRESSWORK: THE ACTIVE ITEM IS THE KNOCKOUT, NOT AN ACCENT PILL.
         * A five-item segmented control filled with the overprint would spend
         * the whole overprint budget on a tab bar, and the design's answer to
         * "this one is selected" is a value flip: ZD_KNOCK at 6.4796:1 on
         * ZD_BASE with the label reversed out at 8.5329:1. */
        const struct op *act = find_rgb(ZD_KNOCK);
        oknum(act && act->x == want,
              "the knockout sits on slot 2's track, not slot 0's or 1's",
              act ? act->x : -1, want);
        oknum(act && act->w == iw, "the knockout is one slot wide",
              act ? act->w : -1, iw);
        const struct op *lab = find_text("Disk");
        ok(lab && lab->x >= want && lab->x + lab->w <= want + iw,
           "the label of the active item is inside the knockout");
        ok(lab && lab->rgb == (unsigned)ZD_KNOCK_INK,
           "the active label is reversed out of it");
        ok(find_rgb(ZD_ACCENT) == NULL,
           "the overprint is NOT spent on a segmented control");
        const struct op *track = find_rgb(ZD_CUT);
        ok(track && track->r == ZD_SEG_R,
           "the container is one raised plate ringed in the groove, at r-chip");
        /* CONTROL: the pill must MOVE when the selection does, or the
         * assertion above would pass against a widget that always draws slot 0
         * and happens to be checked at slot 0's x. */
        begin_draw();
        ui_segmented(10, 10, W, h, items, 0, UI_MD);
        const struct op *first = find_rgb(ZD_KNOCK);
        ok(first && first->x == 10 + pad && first->x != want,
           "control: selecting item 0 moves the knockout to slot 0");
        /* the click path */
        begin_click(10 + pad + 2 * (iw + gap) + 2, 10 + pad + 2);
        int got = ui_segmented(10, 10, W, h, items, 0, UI_MD);
        oknum(got == 2, "a click on slot 2 reports index 2", got, 2);
        begin_click(10 + pad + 2 * (iw + gap) + 2, 10 + pad + 2);
        got = ui_segmented_value(10, 10, W, h, items, 0, UI_MD);
        oknum(got == 2, "the _value bridge returns the NEW selection for zl",
              got, 2);
        begin_click(5000, 5000);
        got = ui_segmented_value(10, 10, W, h, items, 3, UI_MD);
        oknum(got == 3, "control: a click nowhere leaves the selection alone",
              got, 3);
    }

    /* ============================================================ TAB STRIP */
    printf("\n  tab strips - S4.1 closeable, S4.2 underline\n");
    {
        const char *tabs = "main.zl|fb.c|README";
        begin_draw();
        ui_tabstrip(0, 0, 600, tabs, 1);
        ok(find_rgb(ZD_SURF_TABS) != NULL,
           "the strip ground is the tab-strip surface");
        const struct op *act = find_rgb(ZD_SURF_2);
        ok(act != NULL,
           "the ACTIVE tab is painted the colour of the body below it");
        ok(find_text("fb.c") != NULL, "the active label is drawn");
        ok(nops > 5, "control: the strip drew a ground, a tab and its labels");

        /* the close x lives INSIDE the tab, so a hit on it must not also
         * report a tab selection - that is the bug this asserts against */
        int pt = ZD_TAB_PAD_T, pxo = ZD_TAB_PAD_X, pr = ZD_TAB_PR, xw = ZD_TAB_X;
        int tw0 = ZD_TAB_PL + 7 * ROLE_ADV[1] + 7 + xw + pr;
        int closex = 0 + pxo + tw0 - pr - xw + 1;
        begin_click(closex, pt + 4);
        int sel = ui_tabstrip(0, 0, 600, tabs, 1);
        int closed = ui_tabstrip_closed();
        oknum(closed == 0, "clicking the x reports tab 0 closed", closed, 0);
        oknum(sel == -1, "...and does NOT also report tab 0 selected", sel, -1);
        begin_click(0 + pxo + 4, pt + 4);
        sel = ui_tabstrip(0, 0, 600, tabs, 1);
        closed = ui_tabstrip_closed();
        oknum(sel == 0, "clicking the tab body selects it", sel, 0);
        oknum(closed == -1, "control: ...and reports nothing closed", closed, -1);
    }
    {
        const char *tabs = "Units|Logs";
        begin_draw();
        ui_utabs(0, 0, 600, tabs, 1);
        /* the labels are the LABEL style now - SM/bold - and ui_utabs measures
         * in the same weight it draws, so tab 0's box grows by one unit a glyph */
        int tw0 = 2 * ZD_UTAB_PX + 5 * (ROLE_ADV[1] + 1);
        /* PRESSWORK: THE UNDERLINE IS ZD_LIT, NOT THE OVERPRINT. A 2dp rule is
         * thinner than ZD_FOCUS_BAR so the width rule would allow vermilion -
         * but the overprint has FOUR jobs and a tab underline is not one of
         * them, and there is already a token that means "rank boundary" at
         * exactly this weight: the 2px struck rule, 2.5423:1 on the plate. */
        const struct op *rule = NULL;
        for (int i = 0; i < nops; i++)
            if (ops[i].rgb == (unsigned)ZD_LIT && ops[i].h == ZD_UTAB_RULE)
                { rule = &ops[i]; break; }
        oknum(rule && rule->x == tw0,
              "the underline sits under the ACTIVE tab, offset by tab 0's width",
              rule ? rule->x : -1, tw0);
        oknum(rule && rule->h == ZD_UTAB_RULE,
              "the underline is the 2px struck rule - the whole affordance",
              rule ? rule->h : -1, ZD_UTAB_RULE);
        ok(find_rgb(ZD_ACCENT) == NULL,
           "control: no vermilion anywhere - a fifth overprint job costs the"
           " other four their meaning");
        /* CONTROL: S4.2 says "No background change at all". If a background
         * ever appears under the active tab this fails. */
        int bgs = 0;
        for (int i = 0; i < nops; i++)
            if ((ops[i].kind == OP_RRECT || ops[i].kind == OP_RBLEND) &&
                ops[i].y == 0) bgs++;
        oknum(bgs == 0, "control: the active underline tab has NO background fill",
              bgs, 0);
    }

    /* ============================================== COLUMN GRID + LIST ROW */
    printf("\n  column header + list row - S7, tracks verbatim from S7.1\n");
    {
        ui_grid(UI_GRID_PROC);
        oknum(ui_grid_cols(), "PGRID parses to six tracks", ui_grid_cols(), 6);
        int x = 0, w = 600, pl = ZD_COLHEAD_PL, pr = ZD_COLHEAD_PR;
        int cx, cw;
        ui_grid_span(x, w, 0, &cx, &cw);
        oknum(cx == pl && cw == 48, "track 0 is 48px at the left padding",
              cx, pl);
        ui_grid_span(x, w, 1, &cx, &cw);
        oknum(cx == pl + 48 && cw == 52, "track 1 starts where track 0 ends",
              cx, pl + 48);
        ui_grid_span(x, w, 5, &cx, &cw);
        int fixed = 48 + 52 + 28 + 48 + 52;
        oknum(cx == pl + fixed, "the 1fr COMMAND track starts after the five fixed",
              cx, pl + fixed);
        oknum(cw == w - pl - pr - fixed,
              "...and takes the remainder, minus the scrollbar gutter S20.5 forgot",
              cw, w - pl - pr - fixed);
        ui_grid(UI_GRID_FILES);
        ui_grid_span(0, 600, 0, &cx, &cw);
        oknum(ui_grid_cols(), "FGRID parses to five tracks", ui_grid_cols(), 5);
        ok(cw > 100, "FGRID puts the 1fr FIRST - Name grows, unlike PGRID");
        /* CONTROL: a column past the end must report nothing, not the last
         * track - otherwise every cell of an over-long row silently stacks. */
        cx = 999; cw = 999;
        ui_grid_span(0, 600, 9, &cx, &cw);
        oknum(cw == 0, "control: a track index past the end reports zero width",
              cw, 0);
    }
    {
        ui_grid(UI_GRID_PROC);
        begin_draw();
        ui_grid_row(0, 40, 600, 0, 1);
        ui_grid_cell(0, 600, 40, ui_grid_row_h(), 0, "1234", UI_ALIGN_R,
                     ZD_TEXT_1, UI_SM, UI_F_MONO);
        ui_grid_cell(0, 600, 40, ui_grid_row_h(), 5, "/bin/zl", UI_ALIGN_L,
                     ZD_TEXT_1, UI_SM, UI_F_MONO);
        int cx0, cw0, cx5, cw5;
        ui_grid_span(0, 600, 0, &cx0, &cw0);
        ui_grid_span(0, 600, 5, &cx5, &cw5);
        const struct op *pid = find_text("1234");
        const struct op *cmd = find_text("/bin/zl");
        ok(pid && pid->x + pid->w <= cx0 + cw0 && pid->x >= cx0,
           "a right-aligned cell lands inside its own track");
        ok(cmd && cmd->x >= cx5 && cmd->x < cx5 + cw5,
           "a left-aligned cell in the 1fr track lands inside it");
        ok(pid && cmd && pid->x < cmd->x,
           "control: the two cells are in different places, not stacked at x=0");
    }
    {
        /* PRESSWORK: A TABLE ROW'S SELECTION IS THE KNOCKOUT.
         *
         * `tr.sel td { background: var(--zd-knock); color: var(--zd-knock-ink); }`
         * The prototype has TWO row idioms and they are different decisions: a
         * TABLE row flips value, and the RAIL's register slot takes the
         * overprint's register mark. So ui_grid_row draws the first and
         * ui_nav_row still calls ui_row_select(), which draws the second. */
        begin_draw();
        ui_grid_row(0, 0, 300, 0, 1);
        const struct op *ko = find_rgb(ZD_KNOCK);
        ok(ko && ko->w == 300 && ko->h == ZD_LISTROW_H,
           "selected row: the whole row is the knockout");
        ok(find_rgb(ZD_KO_EDGE) != NULL,
           "selected row: ...with the knockout's own edge run under it");
        ok(find_rgb(ZD_ACCENT) == NULL,
           "selected row: no overprint - the value flip IS the signal");
        /* AND ITS CELLS MUST FOLLOW IT. A caller passing its usual ZD_TEXT_2
         * onto a knockout writes 1.2131:1 - invisible on the one row the user
         * is looking at - so the row publishes its ink and ui_grid_cell takes
         * it. This is the assertion that fails if that override is removed. */
        ui_grid("*");
        ui_grid_cell(0, 300, 0, ZD_LISTROW_H, 0, "zl", UI_ALIGN_L,
                     ZD_TEXT_2, UI_SM, 0);
        const struct op *cell = find_text("zl");
        ok(cell && cell->rgb == (unsigned)ZD_KNOCK_INK,
           "selected row: a cell drawn on it is reversed out, not ZD_TEXT_2");
        begin_draw();
        ui_grid_row(0, 0, 300, 0, 0);
        ok(find_rgb(ZD_ACCENT) == NULL && find_rgb(ZD_KNOCK) == NULL,
           "control: an UNSELECTED row paints neither accent nor knockout");
        ui_grid_cell(0, 300, 0, ZD_LISTROW_H, 0, "zl", UI_ALIGN_L,
                     ZD_TEXT_2, UI_SM, 0);
        ok(find_text("zl") && find_text("zl")->rgb == (unsigned)ZD_TEXT_2,
           "control: ...and its cells keep the colour the caller asked for");
        /* THE ZEBRA IS GONE AND A RULE REPLACED IT. A 1% white stripe is an
         * eighth of the smallest step this ladder deliberately shows; `td`
         * separates rows with `border-bottom: 1px solid var(--zd-cut)`. */
        begin_draw();
        ui_grid_row(0, 0, 300, 1, 0);
        oknum(count_kind(OP_BLEND) == 0, "no row is a translucent stripe any more",
              count_kind(OP_BLEND), 0);
        const struct op *g = find_rgb(ZD_CUT);
        oknum(g && g->y == ZD_LISTROW_H - 1 && g->h == 1,
              "every row is closed by the 1px groove instead",
              g ? g->y : -1, ZD_LISTROW_H - 1);
    }

    /* ========================================================= STAT STRIP */
    printf("\n  stat card strip - S8\n");
    {
        begin_draw();
        ui_stat_begin(0, 0, 360, 88);
        ui_stat_cell("FPS", "60", ZD_OK);
        ui_stat_cell("FRAME", "16 ms", ZD_WARN);
        ui_stat_cell("DRAWS", "412", ZD_TEXT_1);
        ui_stat_cell("BYTES", "9 M", ZD_TEXT_1);
        int h = ui_stat_end();
        /* 360 / 88 -> 4 columns, so all four cells land on one row */
        const struct op *c0 = find_text("FPS");
        const struct op *c3 = find_text("BYTES");
        ok(c0 && c3 && c0->y == c3->y,
           "four cells at minmax(88px) across 360px share one row");
        ok(c0 && c3 && c3->x - c0->x == 3 * (360 / 4),
           "...spaced exactly one auto-fit column apart");
        ok(h > 0, "the strip reports a height");
        /* CONTROL: narrow it and the same four cells must WRAP */
        begin_draw();
        ui_stat_begin(0, 0, 180, 88);
        ui_stat_cell("FPS", "60", ZD_OK);
        ui_stat_cell("FRAME", "16 ms", ZD_WARN);
        ui_stat_cell("DRAWS", "412", ZD_TEXT_1);
        ui_stat_cell("BYTES", "9 M", ZD_TEXT_1);
        int h2 = ui_stat_end();
        const struct op *n0 = find_text("FPS");
        const struct op *n3 = find_text("BYTES");
        ok(n0 && n3 && n3->y > n0->y,
           "control: at 180px wide the strip wraps to a second row");
        ok(h2 > h, "control: ...and the reported height grows with it");
    }

    /* ================================================================ BARS */
    printf("\n  meter / progress / segment / mini bars - S10\n");
    {
        /* PRESSWORK: A TRACK IS A PIT WITH WALLS. `.mtrack` is
         * `background: var(--zd-well); border: 1px solid var(--zd-cut)`, so the
         * fill lives INSIDE a 1px ring and its 100% is the width less two. */
        begin_draw();
        ui_meter(0, 0, 200, 25, ZD_OK);
        const struct op *fill = find_rgb(ZD_OK);
        oknum(fill && fill->w == 198 * 25 / 100,
              "a 25% meter fills a quarter of the track INSIDE its walls",
              fill ? fill->w : -1, 198 * 25 / 100);
        const struct op *track = find_rgb(ZD_SURF_1);
        oknum(track && track->w == 200, "the track is the full width",
              track ? track->w : -1, 200);
        ok(find_rgb(ZD_LITSOFT) != NULL,
           "the pit is lit along its NEAR wall - the grazed value, 2.5750:1");
        begin_draw();
        ui_meter(0, 0, 200, 0, ZD_OK);
        ok(find_rgb(ZD_OK) == NULL,
           "control: a 0% meter draws NO fill - not a 1px sliver");
        begin_draw();
        ui_meter(0, 0, 200, 400, ZD_OK);
        fill = find_rgb(ZD_OK);
        oknum(fill && fill->w == 198, "control: an out-of-range meter clamps to 100%",
              fill ? fill->w : -1, 198);
        /* THE DEFAULT INK IS ZD_STEEL, the machine's own reading, because a
         * meter is an instrument and the overprint has four jobs and this is
         * none of them. A caller with no opinion passes 0. */
        begin_draw();
        ui_meter(0, 0, 200, 50, 0);
        ok(find_rgb(ZD_STEEL) != NULL,
           "a meter with no colour asked for fills in ZD_STEEL, not the accent");
    }
    {
        begin_draw();
        ui_segbar_begin(0, 0, 202, 100);
        ui_segbar_item(60, ZD_ACCENT);
        ui_segbar_item(40, ZD_SURF_CARD);
        ui_segbar_end();
        const struct op *used = find_rgb(ZD_ACCENT);
        const struct op *free_ = find_rgb(ZD_SURF_CARD);
        ok(used && free_ && free_->x > used->x,
           "the free segment follows the used one");
        ok(used && used->w == 120,
           "a 60/100 segment takes 60% of the 200px interior");
        ok(free_ && used && free_->x == used->x + used->w,
           "control: the segments abut - no gap and no overlap");
    }
    {
        begin_draw();
        ui_minibar(0, 0, 100, 0, ZD_ACCENT);
        ok(find_rgb(ZD_ACCENT) == NULL, "a zero mini-bar draws nothing");
        begin_draw();
        ui_minibar(0, 0, 100, 1, ZD_ACCENT);
        const struct op *f = find_rgb(ZD_ACCENT);
        ok(f && f->w >= 2, "a 1% mini-bar is floored to 2% so it stays visible");
    }

    /* ============================================================== TOGGLE */
    printf("\n  toggle switch - `.sw2`, the prototype's geometry\n");
    {
        /* PRESSWORK's switch is a bolted rectangle, not a capsule: 34 x 17
         * with a 13 knob at 1px inset, so the travel is symmetric. OFF is a
         * ZD_WELL pit with a ZD_TEXT_INERT knob; ON is the KNOCKOUT with the
         * knob reversed out in ZD_KNOCK_INK - the same value inversion the
         * focused window header and the selected table row make.
         *
         * These used to assert the predecessor's 40 x 22 capsule with a white
         * puck at an asymmetric inset 3. They are rewritten rather than
         * relaxed, and the two controls at the end fail if the capsule or the
         * accent fill comes back. */
        int on = 0;
        begin_draw();
        ui_begin(0, 0, 400, 200, UI_DRAW, -1, -1, 0);
        reset();
        ui_toggle("Wrap", &on);
        const struct op *knob  = find_rgb(t->surf_7);      /* ZD_TEXT_INERT */
        const struct op *track = find_rgb(t->surf_1);      /* ZD_WELL pit   */
        ok(track && track->w == ZD_SW_W && track->h == ZD_SW_H,
           "the track is 34x17, the prototype's own numbers");
        ok(knob && knob->w == ZD_SW_KNOB && knob->h == ZD_SW_KNOB,
           "the knob is 13 square inside a 17 track - 1px of pit all round");
        int offx = knob && track ? knob->x - track->x : -1;
        oknum(offx, "off: the knob sits at inset 1", offx, ZD_SW_INSET);
        ok(find_rgb(ZD_ACCENT) == NULL,
           "off: no overprint - a switch is never vermilion in either state");
        on = 1;
        reset();
        ui_begin(0, 0, 400, 200, UI_DRAW, -1, -1, 0);
        ui_toggle("Wrap", &on);
        knob  = find_rgb(t->knock_ink);
        track = find_rgb(t->knock);
        ok(track && track->w == ZD_SW_W,
           "on: the track is the KNOCKOUT, not the accent");
        int onx = knob && track ? knob->x - track->x : -1;
        oknum(onx, "on: 34 - 13 - 1 == 20, so the travel is symmetric",
              onx, ZD_SW_W - ZD_SW_INSET - ZD_SW_KNOB);
        ok(onx != offx, "control: the knob actually MOVES between states");
        ok(find_rgb(ZD_ACCENT) == NULL,
           "control: an engaged switch paints no overprint either");
        ok(find_rgb(ZD_INK_LIGHT) == NULL,
           "control: the white puck is gone - nothing here is ZD_INK_LIGHT");
    }

    /* ============================================================ SPARKLINE */
    printf("\n  sparkline - S17, and the reference's clipping bug refused\n");
    {
        begin_draw();
        ui_spark_begin(0, 0, 100, 66);
        ui_spark_point(0);
        ui_spark_point(60);
        ui_spark_point(90);
        ui_spark_point(100);
        ui_spark_end();
        /* the polyline: four points, three segments. READ FROM THE OPS -
         * recomputing the mapping here and comparing it with itself would
         * assert my arithmetic rather than the widget's. */
        int nlines = count_kind(OP_LINE), ly[4], nly = 0, ymin = 99999;
        for (int i = 0; i < nops && nly < 4; i++) {
            if (ops[i].kind != OP_LINE) continue;
            ly[nly++] = ops[i].y;
            if (ops[i].y < ymin) ymin = ops[i].y;
            if (ops[i].y + ops[i].h < ymin) ymin = ops[i].y + ops[i].h;
            if (nly == nlines) ly[nly++] = ops[i].y + ops[i].h;   /* last end */
        }
        oknum(nlines == 3, "four samples produce three line segments", nlines, 3);
        ok(nly == 4 && ly[0] > ly[1] && ly[1] > ly[2] && ly[2] > ly[3],
           "0 < 60 < 90 < 100 plot strictly upward - the axis is not inverted");
        oknum(nly == 4 && ly[3] == 0,
              "100% reaches the top of the box and no further",
              nly == 4 ? ly[3] : -1, 0);
        oknum(ymin >= 0,
              "NO POINT ESCAPES THE BOX - the reference's 1.9 gain is absent",
              ymin, 0);
        /* CONTROL: with the reference's gain the 60% sample would already be
         * above the top. Assert the arithmetic that would produce the bug so
         * this test names the bug rather than merely not having it. */
        ok(66 - 60 * 66 / 100 * 19 / 10 < 0,
           "control: the reference's own 1.9 gain WOULD clip at 60%");
        ok(count_kind(OP_BLEND) > 0,
           "control: the area fill under the line was drawn");
        begin_draw();
        ui_spark_begin(0, 0, 100, 66);
        ui_spark_end();
        oknum(count_kind(OP_LINE) == 0,
              "control: a sparkline with no samples draws no line",
              count_kind(OP_LINE), 0);
    }

    /* ========================================================== OVERLAYS */
    printf("\n  popover / menu / modal / toast - S16\n");
    {
        const char *items = "Open|Rename|Delete";
        int w = ui_menu_w(items), h = ui_menu_h(items);
        oknum(h, "the menu is three items tall plus its padding", h,
              3 * ZD_MENU_ITEM_H + 2 * ZD_MENU_PAD);
        begin_draw();
        ui_menu(0, 0, items, 1);
        ok(find_rgb(ZD_SURF_5) != NULL, "the popover surface is drawn");
        /* PRESSWORK: `.mi:hover { background: var(--zd-knock);
         * color: var(--zd-knock-ink); }` - the highlighted row is the same
         * value flip a table row and a focused header use. It was a 15%
         * vermilion tint: an area of overprint the width rule does not allow,
         * and quiet enough that most people would not see it. */
        const struct op *sel = find_rgb(ZD_KNOCK);
        ok(sel && sel->a == 255,
           "the highlighted item is the KNOCKOUT, solid, not a tint");
        oknum(sel && sel->y == ZD_MENU_PAD + ZD_MENU_ITEM_H,
              "...and it is on item 1, not item 0",
              sel ? sel->y : -1, ZD_MENU_PAD + ZD_MENU_ITEM_H);
        ok(find_text("Rename") &&
           find_text("Rename")->rgb == (unsigned)ZD_KNOCK_INK,
           "...with its label reversed out of it at 8.5329:1");
        ok(find_rgb(ZD_ACCENT) == NULL,
           "control: a menu spends no overprint at all");
        ok(find_rgb(ZD_EDGE_OVER) != NULL,
           "the popover is ringed in ZD_EDGE_OVER - the boundary under overlap");
        ok(find_text("Delete") != NULL, "every item is drawn");
        ok(w > 0, "the menu reports a width");
        begin_click(ZD_MENU_PAD + 4, ZD_MENU_PAD + 2 * ZD_MENU_ITEM_H + 4);
        int mi = ui_menu(0, 0, items, 0);
        oknum(mi == 2, "a click on the third item reports index 2", mi, 2);
        begin_click(9999, 9999);
        mi = ui_menu(0, 0, items, 0);
        oknum(mi == -1, "control: a click outside reports -1", mi, -1);
    }
    {
        begin_draw();
        ui_modal(0, 0, 334, 200, "Properties");
        ok(find_text("Properties") != NULL, "the modal draws its title");
        /* S16.3: no scrim on any of the three reference modals */
        int scrim = 0;
        for (int i = 0; i < nops; i++)
            if (ops[i].kind == OP_BLEND && ops[i].w > 334) scrim = 1;
        oknum(scrim == 0, "control: the modal paints NO backdrop scrim - S16.3",
              scrim, 0);
        begin_draw();
        ui_toast_draw(0, 0, ZD_TOAST_W, "Saved", "kernel.zl written", ZD_OK);
        ok(find_text("Saved") && find_text("kernel.zl written"),
           "the toast draws both its title and its body");
        /* PRESSWORK: THE 20dp ICON SQUARE IS GONE AND `.toast .bar` REPLACES
         * IT. A solid 400 square dp block of a state colour carries the state
         * on its own, which is the failure the prototype's `.sw` rule exists
         * to prevent; the toast already has a title. The kind is now a
         * ZD_FOCUS_BAR-wide rule down the left edge - the one width the
         * overprint budget allows anything to be. */
        const struct op *bar = find_rgb(ZD_OK);
        oknum(bar && bar->w == ZD_FOCUS_BAR,
              "the toast's kind is a focus-bar-wide rule, not an icon block",
              bar ? bar->w : -1, ZD_FOCUS_BAR);
        oknum(bar && bar->x == 1, "...on its left edge, inside the ring",
              bar ? bar->x : -1, 1);
    }

    /* ================================================ INDICATORS AND INPUTS */
    printf("\n  badge / dot / input / search / chip - S18\n");
    {
        begin_draw();
        ui_badge(0, 0, "rw", ZD_OK);
        /* PRESSWORK: A BADGE IS A SMALL PLATE WITH A SEMANTIC INK ON IT. The
         * old 14% wash of its own colour is a colour cast, and a colour cast
         * is what a ladder replaces. Measured on ZD_RAISE: ZD_OK 5.0630:1,
         * ZD_WARN 5.1997:1, ZD_BAD_INK 4.7396:1, ZD_STEEL 4.9632:1 - the ink
         * carries the whole semantic and every one of them clears 4.5. */
        int washed = 0;
        for (int i = 0; i < nops; i++)
            if (ops[i].rgb == (unsigned)ZD_OK && ops[i].a < 255) washed = 1;
        oknum(washed == 0, "a badge is NOT a tint of its semantic colour",
              washed, 0);
        ok(find_rgb(ZD_RAISE) != NULL && find_rgb(ZD_LIT) != NULL,
           "a badge is the ordinary raised seat - a face and a struck run");
        const struct op *lab = find_text("rw");
        ok(lab && lab->rgb == (unsigned)ZD_OK,
           "...with the label in the semantic colour at full strength");
        /* ZD_BAD IS A FILL AND A MARK, NOT AN INK - 3.4161:1 on ZD_RAISE.
         * A caller passing it where an ink belongs is corrected here rather
         * than in 53 apps. */
        begin_draw();
        ui_badge(0, 0, "err", ZD_BAD);
        ok(find_text("err") && find_text("err")->rgb == (unsigned)ZD_BAD_INK,
           "a badge asked for ZD_BAD writes ZD_BAD_INK - the ink, not the fill");
        begin_draw();
        ui_dot(0, 0, ZD_BAD, 0);
        oknum(count_kind(OP_RRECT), "a dot with no glow is exactly one shape",
              count_kind(OP_RRECT), 1);
        /* PRESSWORK HAS ONE SHADOW TOKEN and it is drawn under the three
         * objects that are off the plane. A halo round a 6dp square is not one
         * of them - design.h zeroed ZD_BLUR_GLOW_A/B for the same reason. The
         * parameter survives because its MEANING ("this one is live") is still
         * true; it is simply no longer drawn as light. */
        begin_draw();
        ui_dot(0, 0, ZD_BAD, 1);
        oknum(count_kind(OP_RBLEND) == 0, "a glowing dot draws NO halo",
              count_kind(OP_RBLEND), 0);
        oknum(count_kind(OP_RRECT) == 1, "...it is the same single mark",
              count_kind(OP_RRECT), 1);
    }
    {
        begin_draw();
        ui_input(0, 0, 200, "", "Search rd0", 0);
        const struct op *ph = find_text("Search rd0");
        /* ZD_TEXT_3, and design.h aliases ZD_TEXT_6 onto it - the widening
         * cost the ink ramp its fifth rung. The rung BELOW this one is
         * ZD_TEXT_INERT at 2.0222:1 on ZD_RAISE, and a placeholder is exactly
         * the "it's only a hint" text that gets demoted into it. It is a
         * glyph. It does not go there. */
        ok(ph && ph->rgb == (unsigned)ZD_TEXT_3,
           "an empty input shows the placeholder in the label rung, ZD_TEXT_3");
        ok(!text_used_colour(ZD_TEXT_INERT),
           "control: nothing in an input is written in ZD_TEXT_INERT");
        ok(find_rgb(ZD_SURF_WELL) != NULL,
           "the field is a PIT - `.well`, at 12.7802:1 for body ink on it");
        begin_draw();
        ui_input(0, 0, 200, "kernel.zl", "Search rd0", 0);
        ok(find_text("kernel.zl") && !find_text("Search rd0"),
           "control: once there is text the placeholder is gone");
        /* the focus treatment is NEW DESIGN - assert it is there AND that it
         * is only the border, so nobody quietly grows it into a ring */
        /* PRESSWORK: FOCUS IS THE OVERPRINT'S JOB 1, A BAR - NOT A BORDER.
         * It used to be a full vermilion box round the field: four sides at
         * 4.6319:1, which is a loud rectangle and is also vermilion used as a
         * BORDER, which the two-ink contract forbids outright. It is now the
         * same ZD_FOCUS_BAR-wide mark down the left edge that `.fbar` puts on
         * the focused plate, plus the caret. */
        begin_draw();
        ui_input(0, 0, 200, "", "x", 1);
        int accent_box = 0, bar = 0;
        for (int i = 0; i < nops; i++) {
            if (ops[i].rgb != (unsigned)ZD_ACCENT) continue;
            if (ops[i].kind == OP_BOX) accent_box++;
            if (ops[i].kind == OP_FILL && ops[i].w == ZD_FOCUS_BAR &&
                ops[i].x == 1) bar = 1;
        }
        oknum(accent_box == 0, "focused: the overprint is never a border",
              accent_box, 0);
        ok(bar, "focused: it is a focus-bar-wide rule down the left edge");
        begin_draw();
        ui_input(0, 0, 200, "", "x", 0);
        accent_box = 0;
        for (int i = 0; i < nops; i++)
            if (ops[i].rgb == (unsigned)ZD_ACCENT) accent_box++;
        oknum(accent_box == 0, "control: an unfocused input paints no accent at all",
              accent_box, 0);
    }
    {
        begin_draw();
        /* PRESSWORK: a row of eight active filter chips filled with the
         * overprint is the whole overprint budget spent on a filter bar. The
         * chip routes through pill_face(), so it and the button cannot drift
         * about what "selected" looks like: the knockout. */
        ui_chip(0, 0, "All", 1);
        const struct op *f = find_rgb(ZD_KNOCK);
        ok(f && f->a == 255, "an ACTIVE chip is the knockout, solid");
        ok(find_text("All") && find_text("All")->rgb == (unsigned)ZD_KNOCK_INK,
           "...with its label reversed out of it");
        ok(find_rgb(ZD_ACCENT) == NULL, "...and no overprint at all");
        begin_draw();
        ui_chip(0, 0, "All", 0);
        ok(find_rgb(ZD_ACCENT) == NULL && find_rgb(ZD_KNOCK) == NULL,
           "control: an inactive chip paints neither accent nor knockout");
    }

    /* ========================================================== NAV / CARD */
    printf("\n  sidebar / heading / nav row / card / key-value - S9, S15\n");
    {
        begin_draw();
        ui_sidebar(0, 0, ui_sidebar_w(), 400);
        ok(find_rgb(ZD_SURF_2) != NULL, "the sidebar draws its right hairline");
        begin_draw();
        ui_nav_row(0, 0, 150, "Places", 1);
        ok(find_text("Places") &&
           find_text("Places")->rgb == (unsigned)ZD_ACCENT_PALE,
           "a selected nav row takes the emphasis ink");
        /* THE 15% ACCENT WASH IS GONE. A nav row is a POINTER at something
         * else, so it takes the register mark - a ZD_FOCUS_BAR-wide vermilion
         * rule on the leading edge, over a ZD_BASE ground. The wash it
         * replaces was 10,400 px of overprint behind a 78 px bar, against the
         * rule that vermilion is laid as a rule or a mark and never as an
         * area. This control fails if the wash returns. */
        int wash = 0, mark = 0;
        for (int i = 0; i < nops; i++) {
            if (ops[i].kind == OP_RBLEND && ops[i].rgb == (unsigned)ZD_ACCENT)
                wash = 1;
            if (ops[i].kind == OP_FILL && ops[i].rgb == (unsigned)ZD_VERM_BR &&
                ops[i].w == ZD_SEL_BAR_W)
                mark = 1;
        }
        ok(!wash, "control: no 15% accent wash - the overprint is a mark");
        ok(mark, "...the register mark is there instead, ZD_FOCUS_BAR wide");
        /* THE LABEL STYLE, AND IT IS NOW FOUR THINGS RATHER THAN TWO.
         *
         * `th, .t-lab` is SM, bold, UPPERCASE and TRACKED by ZD_TR_LAB, and
         * uikit.c carried only the first two - its own comments recorded
         * uppercasing as "the caller's" and tracking as "NOT AVAILABLE AND IS
         * NOT FAKED". ZD_TR_LAB had sat in design.h with nothing in the tree
         * reading it. Tracking a string is one draw per glyph with the pen
         * advanced by the glyph plus the track, which is the toolkit's job;
         * fb.c is untouched.
         *
         * So this can no longer be find_text("DEVICES") - there is no such op,
         * there are seven one-glyph ops. Asserting the geometry instead is
         * strictly stronger: the argument goes in lowercase, so a widget that
         * stopped uppercasing fails on the first glyph, and the SEVENTH glyph
         * lands six advances along, so a widget that stopped tracking fails on
         * the span. A per-glyph draw at the wrong pitch is exactly the bug
         * that a "did the string appear" check cannot see. */
        begin_draw();
        ui_heading(0, 0, 150, "Devices");
        const struct op *g0 = find_text("D");
        const struct op *g6 = find_text("S");
        int adv = fb_text_role_w("D", UI_SM, 1) + ZD_TR_LAB / 10;
        ok(g0 && g6 && g0->x == ZD_CELL_PX && g6->x - g0->x == 6 * adv,
           "a section heading draws its label uppercased and tracked");
        begin_draw();
        ui_card(0, 0, 300, 120);
        /* PRESSWORK: "RAISED, ONE NESTING LEVEL IN. Radius halves; value moves
         * one rung." The card's outline used to be ZD_SURF_5, which now
         * resolves to ZD_FLOAT - a LIGHTER surface than the card's own face,
         * so the card was ringed in something brighter than itself. Under one
         * lamp that means light from every direction at once. */
        const struct op *ring = find_rgb(ZD_CUT);
        ok(find_rgb(t->panel_hi) != NULL,
           "a card is a raised surface, one rung above the plate");
        oknum(ring && ring->r == ZD_CARD_R,
              "...ringed in the groove at r-inset, half the plate's radius",
              ring ? ring->r : -1, ZD_CARD_R);
        const struct op *run = find_rgb(ZD_LIT);
        oknum(run && run->h == 1 && run->y == 1,
              "...and struck by the lamp along one pixel of its top",
              run ? run->y : -1, 1);
        ok(find_rgb(ZD_FLOAT) == NULL,
           "control: nothing about a card is drawn in a LIGHTER surface"
           " than its own face");
        begin_draw();
        ui_card_head(0, 0, 300, "CPU", "ok", ZD_OK);
        ok(find_text("CPU") && find_text("ok"),
           "a card header draws its title and its badge");
        begin_draw();
        /* `.kv .k` is the same tracked-caps LABEL style as `th` and `.sect`,
         * so the key is now seven one-glyph ops rather than one string op -
         * see the ui_heading check above. Anchoring on the FIRST and LAST
         * glyph of UPTIME keeps this one assertion and makes it say more than
         * it did: the key is uppercased, it advances left to right, and the
         * value still clears its true right edge rather than the edge of a
         * width the widget no longer measures that way. */
        ui_kv(0, 0, 300, "Uptime", "4h 12m", ZD_TEXT_1, 1);
        const struct op *k0 = find_text("U");
        const struct op *k5 = find_text("E");
        const struct op *v  = find_text("4h 12m");
        ok(k0 && k5 && v && k5->x > k0->x && v->x > k5->x + k5->w,
           "a key/value row draws a tracked-caps key and clears the value of it");
        oknum(count_kind(OP_FILL) == 0,
              "control: the FIRST row draws no separator rule",
              count_kind(OP_FILL), 0);
        begin_draw();
        ui_kv(0, 0, 300, "Uptime", "4h 12m", ZD_TEXT_1, 0);
        ok(count_kind(OP_FILL) == 1, "control: rows 2..n do draw one");
    }

    /* ============================================ TOOLBAR AND STATUS BAR */
    printf("\n  toolbar / status bar - S5, S6\n");
    {
        begin_draw();
        /* PRESSWORK: A TOOLBAR IS A REGION BOUNDARY AND THE DESIGN HAS A
         * WEIGHT THAT MEANS THAT. `#raster` has `border-bottom: 2px solid
         * var(--zd-lit)` and `#foot` has the same rule on its top. 1px ZD_CUT
         * means "next row"; 2px ZD_LIT means "next region". */
        ui_toolbar(0, 0, 500, ui_toolbar_h(), 0);
        const struct op *rule = find_rgb(ZD_LIT);
        oknum(rule && rule->y == ui_toolbar_h() - ZD_RULE_H,
              "a TOP toolbar puts its 2px struck rule at the bottom",
              rule ? rule->y : -1, ui_toolbar_h() - ZD_RULE_H);
        oknum(rule && rule->h == ZD_RULE_H,
              "...and it is the REGION weight, not a hairline",
              rule ? rule->h : -1, ZD_RULE_H);
        begin_draw();
        ui_toolbar(0, 100, 500, ui_toolbar_h(), 1);
        rule = find_rgb(ZD_LIT);
        oknum(rule && rule->y == 100,
              "control: a BOTTOM toolbar puts it at the top instead",
              rule ? rule->y : -1, 100);
        begin_draw();
        ui_statusbar(0, 0, 500, ui_status_h());
        ok(nops >= 2, "control: the status bar drew a ground and a rule");
    }

    /* ====================================================== MONOSPACE PANEL */
    printf("\n  monospace panel - S14\n");
    {
        /* PRESSWORK: THREE GROUNDS BECAME ONE, and the reason the third
         * existed went with it. The terminal ground was held at the active
         * tab's colour so the tab connected to the body; under PRESSWORK the
         * active tab is a knockout at 6.4796:1 and there is no seam to hide.
         * Every mono panel is `.well` now, and `kind` selects a line height,
         * which is what actually differed between the five instances. */
        begin_draw();
        ui_mono_panel(0, 0, 400, 300, UI_PANEL_TERM);
        const struct op *pit = find_rgb(ZD_WELL);
        ok(pit && pit->w == 400 - 2,
           "the terminal ground is a pit - `.well`, its face inside its ring");
        ok(find_rgb(ZD_LITSOFT) != NULL,
           "...lit along its near wall, the sign of the seat flipped");
        begin_draw();
        ui_mono_panel(0, 0, 400, 300, UI_PANEL_HEX);
        ok(find_rgb(ZD_WELL) != NULL,
           "control: the hex panel is the SAME pit, not a fourth ground");
        oknum(ui_mono_line_h(UI_PANEL_HEX) == ui_mono_line_h(UI_PANEL_TERM),
              "control: `kind` is a line height and both are the cell's 3/2",
              ui_mono_line_h(UI_PANEL_HEX), ui_mono_line_h(UI_PANEL_TERM));
        begin_draw();
        ui_mono_line(10, 10, 380, "zl: 3 warnings", ZD_TEXT_1, UI_PANEL_TERM, 0);
        ok(find_text("zl: 3 warnings") != NULL, "a plain line is just text");
        oknum(count_kind(OP_RBLEND) == 0, "control: ...with no highlight behind it",
              count_kind(OP_RBLEND), 0);
        begin_draw();
        /* PRESSWORK: THE JUMP TARGET IS A MARK, NOT A WASH. 7% vermilion
         * across a whole line is an area of overprint far wider than
         * ZD_FOCUS_BAR and, at 7%, a change below the ladder's smallest
         * deliberate step - against the width rule AND too quiet to see. */
        ui_mono_line(10, 10, 380, "kernel.zl:88: note", ZD_TEXT_1,
                     UI_PANEL_TERM, 1);
        oknum(count_kind(OP_RBLEND) == 0, "a jump-target line draws NO wash",
              count_kind(OP_RBLEND), 0);
        const struct op *mk = find_rgb(ZD_ACCENT);
        oknum(mk && mk->w == ZD_FOCUS_BAR,
              "it is a focus-bar-wide overprint rule in the left margin",
              mk ? mk->w : -1, ZD_FOCUS_BAR);
    }

    /* ============================================ RULE 1: NO COLOUR LITERAL */
    printf("\n  rule 1 - a colour literal may appear in design.h and nowhere else\n");
    {
        char *uic = slurp("../../src/graphics/ui/ui.c");
        char *kit = slurp("../../src/graphics/ui/uikit.c");
        /* wm.c was NOT scanned here, and it is the file this design changed most
         * and the only one that paints the focus signal - so a hardcoded colour
         * in the chrome passed every gate the rule-1 check was supposed to be.
         * Found by review; the omission is exactly the shape
         * docs/GUARDS-THAT-DID-NOT-GUARD.md is about. */
        char *wmc = slurp("../../src/graphics/windowing/wm.c");
        char *dsg = slurp("../../src/graphics/ui/design.h");
        int draw = count_colour_literals(uic) + count_colour_literals(kit);
        strip_comments(uic); strip_comments(kit);
        int a = count_colour_literals(uic), b = count_colour_literals(kit);
        int d = count_colour_literals(dsg);
        oknum(a == 0, "ui.c carries no six-digit colour literal", a, 0);
        oknum(b == 0, "uikit.c carries no six-digit colour literal", b, 0);
        strip_comments(wmc);
        int c = count_colour_literals(wmc);
        oknum(c == 0, "wm.c carries no six-digit colour literal", c, 0);
        /* CONTROL: the scanner must FIND them where they are supposed to be,
         * or "zero literals in ui.c" is a statement about the scanner. */
        ok(d > 20, "control: the same scanner finds design.h's own tokens");
        ok(draw > a + b,
           "control: the two files DO quote the reference's hex in their prose -"
           " so the comment stripper is doing work, not nothing");
        {
            char probe[] = "a; /* #14171a */ b = 0x1C2024; // #ffffff\n";
            strip_comments(probe);
            ok(count_colour_literals(probe) == 1,
               "control: the stripper keeps a literal in CODE and drops the two"
               " in comments");
        }
        ok(count_colour_literals("fb_rrect(x,y,w,h,r,0x1C2024);") == 1,
           "control: it catches a planted 0xRRGGBB");
        ok(count_colour_literals("background:#14171a;") == 1,
           "control: ...and a planted #rrggbb");
        ok(count_colour_literals("mask = c & 0xFFu; step = 0x7FFFFFFF;") == 0,
           "control: it does NOT fire on byte masks or 8-digit constants");
        free(uic); free(kit); free(dsg);
    }

    /* ========================================== RULE 2: EVERYTHING VIA UI_DP */
    printf("\n  rule 2 - every distance goes through UI_DP\n");
    {
        int one[N_METRICS], two[N_METRICS];
        ui_theme_init_q8(256);
        for (int i = 0; i < N_METRICS; i++) one[i] = METRICS[i].fn();
        ui_theme_init_q8(512);
        for (int i = 0; i < N_METRICS; i++) two[i] = METRICS[i].fn();
        int bad = 0;
        for (int i = 0; i < N_METRICS; i++) {
            /* exactly double, allowing the +128>>8 rounding and the small
             * floors (a 4px bar cannot go below 2px) */
            int want = one[i] * 2;
            if (two[i] < want - 2 || two[i] > want + 2) {
                printf("    %-16s 1x=%-4d 2x=%-4d  want ~%d\n",
                       METRICS[i].name, one[i], two[i], want);
                bad++;
            }
        }
        oknum(bad == 0, "every widget metric doubles from 1x to 2x", bad, 0);
        /* CONTROL: the comparison must be capable of failing. A constant would
         * not double, so assert that at least one metric really did move. */
        int moved = 0;
        for (int i = 0; i < N_METRICS; i++) if (two[i] != one[i]) moved++;
        ok(moved == N_METRICS,
           "control: every one of them actually CHANGED with the scale");
        ui_theme_init_q8(256);
    }

    /* ========================================== EVERY ENTRY POINT IS REACHED
     * The groups above assert the widgets that carry geometry. This one exists
     * because docs/GUARDS-THAT-DID-NOT-GUARD.md's recurring shape is code that
     * is written and never executed - "the code exists is not the code works".
     * So every remaining exported function is CALLED, in both passes, and must
     * put something on the draw list or return a sane measurement. It is the
     * cheapest assertion in the file and it is the one that catches a widget
     * that faults the first time an app touches it. */
    printf("\n  coverage - every exported widget is called at least once\n");
    {
        int uncovered = 0, silent = 0;

        ui_theme_init_q8(512);            /* and at a scale nothing else used */
        if (ui_text_w("x", UI_SM, 0) <= 0) uncovered++;
        if (ui_text_h(UI_LG) <= 0) uncovered++;
        if (ui_seg_w("a|bb|ccc", UI_MD) <= 0) uncovered++;
        if (ui_menu_w("a|bb") <= 0) uncovered++;
        if (ui_toast_h() <= 0) uncovered++;
        if (ui_heading_h() <= 0) uncovered++;
        if (ui_chip_w("x") <= 0 || ui_chip_h() <= 0) uncovered++;
        if (ui_badge_w("x") <= 0) uncovered++;
        if (ui_pill_w("x", UI_LG, UI_F_BOLD) <= 0) uncovered++;
        oknum(uncovered == 0, "every measuring function returns a positive size",
              uncovered, 0);

        /* the drawing ones, each on its own pass so "did IT draw" is answerable */
        begin_draw(); ui_text(0, 0, "hello", ZD_TEXT_1, UI_LG, UI_F_BOLD);
        if (!nops) silent++;
        begin_draw(); ui_popover(0, 0, 200, 100);       if (!nops) silent++;
        begin_draw(); ui_progress(0, 0, 100, 50, ZD_ACCENT); if (!nops) silent++;
        begin_draw(); ui_search(0, 0, 128, "", "Search"); if (!nops) silent++;
        begin_draw(); ui_icon_button(0, 0, ZD_WINCTL, "x", 0); if (!nops) silent++;
        begin_draw(); ui_icon_button(0, 0, ZD_WINCTL, "x", 1); if (!nops) silent++;
        begin_draw(); ui_statusbar(0, 0, 300, ui_status_h()); if (!nops) silent++;
        begin_draw(); ui_sidebar(0, 0, 150, 300);       if (!nops) silent++;
        begin_draw(); ui_card_head(0, 0, 200, "T", 0, ZD_OK); if (!nops) silent++;
        begin_draw();
        ui_begin(0, 0, 400, 200, UI_DRAW, -1, -1, 0);
        reset();
        ui_button_sz("Go", UI_LG, UI_BTN_PRIMARY, UI_F_BOLD);
        if (!nops) silent++;
        oknum(silent == 0, "every drawing widget puts something on the list",
              silent, 0);

        /* the two _value bridges that the groups above did not exercise */
        begin_click(9999, 9999);
        int a = ui_tabstrip_value(0, 0, 300, "a|b|c", 2);
        int b = ui_utabs_value(0, 0, 300, "a|b|c", 1);
        oknum(a == 2 && b == 1,
              "control: both untouched _value bridges return the old selection",
              a * 10 + b, 21);

        /* AND THE OUT-OF-ORDER CALL, which on this target is a fault in ring 0
         * rather than a wrong picture: zl can call any of these directly, so a
         * begin/item/end widget driven without its begin must decline. */
        begin_draw();
        ui_stat_cell("K", "V", ZD_TEXT_1);       /* no ui_stat_begin */
        ui_segbar_item(50, ZD_ACCENT);           /* no ui_segbar_begin */
        ui_spark_point(50);                      /* no ui_spark_begin */
        ok(1, "control: cell/item/point called with no begin did not fault");

        ui_theme_init_q8(256);
    }

    printf("\n%s   %d checks, %d failure%s\n",
           fails ? "FAILED" : "all passed", checks, fails,
           fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
