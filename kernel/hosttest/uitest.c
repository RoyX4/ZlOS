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

#include "../ui.h"
#include "../design.h"

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
        ok(find_rgb(ZD_BAD) != NULL, "destructive: the fill is BAD, one red");
        const struct op *lab = find_text("End");
        ok(lab && lab->rgb == (unsigned)ZD_BAD_SOFT,
           "destructive: the label is the softer red, not BAD itself");
        int tinted = 0;
        for (int i = 0; i < nops; i++)
            if (ops[i].kind == OP_RBLEND && ops[i].rgb == (unsigned)ZD_BAD &&
                ops[i].a < 255) tinted = 1;
        ok(tinted, "destructive: the fill is a TINT, not a solid red block");
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
        const struct op *act = find_rgb(ZD_ACCENT);
        oknum(act && act->x == want,
              "the active pill sits on slot 2's track, not slot 0's or 1's",
              act ? act->x : -1, want);
        oknum(act && act->w == iw, "the active pill is one slot wide",
              act ? act->w : -1, iw);
        const struct op *lab = find_text("Disk");
        ok(lab && lab->x >= want && lab->x + lab->w <= want + iw,
           "the label of the active item is inside the active pill");
        ok(lab && lab->rgb == (unsigned)ZD_INK_DARK,
           "the active label is the computed ink");
        const struct op *track = find_rgb(ZD_SURF_1);
        ok(track && track->r == ZD_SEG_R, "the container is the sunken well at r11");
        /* CONTROL: the pill must MOVE when the selection does, or the
         * assertion above would pass against a widget that always draws slot 0
         * and happens to be checked at slot 0's x. */
        begin_draw();
        ui_segmented(10, 10, W, h, items, 0, UI_MD);
        const struct op *first = find_rgb(ZD_ACCENT);
        ok(first && first->x == 10 + pad && first->x != want,
           "control: selecting item 0 moves the pill to slot 0");
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
        int tw0 = 2 * ZD_UTAB_PX + 5 * ROLE_ADV[1];
        const struct op *rule = find_rgb(ZD_ACCENT);
        oknum(rule && rule->x == tw0,
              "the underline sits under the ACTIVE tab, offset by tab 0's width",
              rule ? rule->x : -1, tw0);
        oknum(rule && rule->h == ZD_UTAB_RULE,
              "the underline is 2px - S4.2's whole affordance",
              rule ? rule->h : -1, ZD_UTAB_RULE);
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
        /* THE SELECTION TREATMENT - one of the reference's three, S20.2 */
        begin_draw();
        ui_grid_row(0, 0, 300, 0, 1);
        int tint = 0, bar = 0, solid = 0;
        for (int i = 0; i < nops; i++) {
            if (ops[i].rgb != (unsigned)ZD_ACCENT) continue;
            if (ops[i].kind == OP_RBLEND && ops[i].a < 255) tint = 1;
            if (ops[i].kind == OP_FILL && ops[i].w <= 4) bar = 1;
            if (ops[i].kind == OP_RRECT && ops[i].a == 255 && ops[i].w > 100)
                solid = 1;
        }
        ok(tint, "selected row: the accent TINT is drawn");
        ok(bar, "selected row: the 2px inset left bar is drawn");
        oknum(solid == 0, "selected row: it is NOT the solid-accent treatment",
              solid, 0);
        begin_draw();
        ui_grid_row(0, 0, 300, 0, 0);
        ok(find_rgb(ZD_ACCENT) == NULL,
           "control: an UNSELECTED row paints no accent at all");
        begin_draw();
        ui_grid_row(0, 0, 300, 1, 0);   /* odd index -> zebra */
        ok(count_kind(OP_BLEND) >= 1, "an odd row gets the zebra stripe");
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
        begin_draw();
        ui_meter(0, 0, 200, 25, ZD_OK);
        const struct op *fill = find_rgb(ZD_OK);
        oknum(fill && fill->w == 50, "a 25% meter fills a quarter of the track",
              fill ? fill->w : -1, 50);
        const struct op *track = find_rgb(ZD_SURF_1);
        oknum(track && track->w == 200, "the track is the full width",
              track ? track->w : -1, 200);
        begin_draw();
        ui_meter(0, 0, 200, 0, ZD_OK);
        ok(find_rgb(ZD_OK) == NULL,
           "control: a 0% meter draws NO fill - not a 1px sliver");
        begin_draw();
        ui_meter(0, 0, 200, 400, ZD_OK);
        fill = find_rgb(ZD_OK);
        oknum(fill && fill->w == 200, "control: an out-of-range meter clamps to 100%",
              fill ? fill->w : -1, 200);
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
    printf("\n  toggle switch - S11, reference geometry\n");
    {
        int on = 0;
        begin_draw();
        ui_begin(0, 0, 400, 200, UI_DRAW, -1, -1, 0);
        reset();
        ui_toggle("Wrap", &on);
        const struct op *knob = find_rgb(ZD_INK_LIGHT);
        const struct op *track = find_rgb(t->border);
        ok(track && track->w == ZD_SW_W && track->h == ZD_SW_H,
           "the track is 40x22, the reference's own numbers");
        ok(knob && knob->w == ZD_SW_KNOB,
           "the knob is 16px inside a 22px track - 3px top and bottom");
        int offx = knob ? knob->x - track->x : -1;
        oknum(offx, "off: the knob sits at inset 3", offx, ZD_SW_INSET);
        on = 1;
        reset();
        ui_begin(0, 0, 400, 200, UI_DRAW, -1, -1, 0);
        ui_toggle("Wrap", &on);
        knob = find_rgb(ZD_INK_LIGHT);
        track = find_rgb(t->accent);
        int onx = (knob && track) ? knob->x - track->x : -1;
        oknum(onx, "on: 3 + 16 + 21 == 40, so the knob lands flush right",
              onx, ZD_SW_W - ZD_SW_INSET - ZD_SW_KNOB);
        ok(onx != offx, "control: the knob actually MOVES between states");
        ok(track && track->rgb == t->accent,
           "on: the track is the accent, off: it is the border surface");
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
        const struct op *sel = find_rgb(ZD_ACCENT);
        ok(sel && sel->a < 255,
           "the highlighted item is a TINT, the same treatment as a list row");
        oknum(sel && sel->y == ZD_MENU_PAD + ZD_MENU_ITEM_H,
              "...and it is on item 1, not item 0",
              sel ? sel->y : -1, ZD_MENU_PAD + ZD_MENU_ITEM_H);
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
        const struct op *icon = find_rgb(ZD_OK);
        ok(icon && icon->w == ZD_TOAST_ICON,
           "the toast icon is a 22px square in the kind colour");
    }

    /* ================================================ INDICATORS AND INPUTS */
    printf("\n  badge / dot / input / search / chip - S18\n");
    {
        begin_draw();
        ui_badge(0, 0, "rw", ZD_OK);
        const struct op *bg = find_rgb(ZD_OK);
        ok(bg && bg->a < 255, "a badge is a TINT of its own semantic colour");
        const struct op *lab = find_text("rw");
        ok(lab && lab->rgb == (unsigned)ZD_OK,
           "...with the label in that colour at full strength");
        begin_draw();
        ui_dot(0, 0, ZD_BAD, 0);
        oknum(count_kind(OP_RRECT), "a dot with no glow is exactly one shape",
              count_kind(OP_RRECT), 1);
        begin_draw();
        ui_dot(0, 0, ZD_BAD, 1);
        ok(count_kind(OP_RBLEND) == 2,
           "a glowing dot adds two soft rings and nothing else");
    }
    {
        begin_draw();
        ui_input(0, 0, 200, "", "Search rd0", 0);
        const struct op *ph = find_text("Search rd0");
        ok(ph && ph->rgb == (unsigned)ZD_TEXT_6,
           "an empty input shows the placeholder in the quaternary ink");
        begin_draw();
        ui_input(0, 0, 200, "kernel.zl", "Search rd0", 0);
        ok(find_text("kernel.zl") && !find_text("Search rd0"),
           "control: once there is text the placeholder is gone");
        /* the focus treatment is NEW DESIGN - assert it is there AND that it
         * is only the border, so nobody quietly grows it into a ring */
        begin_draw();
        ui_input(0, 0, 200, "", "x", 1);
        int accent_box = 0;
        for (int i = 0; i < nops; i++)
            if (ops[i].kind == OP_BOX && ops[i].rgb == (unsigned)ZD_ACCENT)
                accent_box++;
        oknum(accent_box, "focused: exactly one accent hairline, no ring, no glow",
              accent_box, 1);
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
        ui_chip(0, 0, "All", 1);
        const struct op *f = find_rgb(ZD_ACCENT);
        ok(f && f->a == 255, "an ACTIVE chip is a solid accent fill");
        ok(find_text("All") && find_text("All")->rgb == (unsigned)ZD_INK_DARK,
           "...with the computed ink on it");
        begin_draw();
        ui_chip(0, 0, "All", 0);
        ok(find_rgb(ZD_ACCENT) == NULL,
           "control: an inactive chip paints no accent");
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
           "a selected nav row takes the pale lime ink");
        int tint = 0;
        for (int i = 0; i < nops; i++)
            if (ops[i].kind == OP_RBLEND && ops[i].rgb == (unsigned)ZD_ACCENT)
                tint = 1;
        ok(tint, "control: ...and the same tint treatment as the list row");
        begin_draw();
        ui_heading(0, 0, 150, "DEVICES");
        ok(find_text("DEVICES") != NULL, "a section heading draws its label");
        begin_draw();
        ui_card(0, 0, 300, 120);
        ok(find_rgb(t->panel_hi) != NULL && count_kind(OP_BOX) == 1,
           "a card is a raised surface with exactly one hairline border");
        begin_draw();
        ui_card_head(0, 0, 300, "CPU", "ok", ZD_OK);
        ok(find_text("CPU") && find_text("ok"),
           "a card header draws its title and its badge");
        begin_draw();
        ui_kv(0, 0, 300, "Uptime", "4h 12m", ZD_TEXT_1, 1);
        const struct op *k = find_text("Uptime");
        const struct op *v = find_text("4h 12m");
        ok(k && v && v->x > k->x + k->w,
           "a key/value row right-aligns the value clear of the key");
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
        ui_toolbar(0, 0, 500, ui_toolbar_h(), 0);
        const struct op *rule = find_rgb(ZD_SURF_2);
        oknum(rule && rule->y == ui_toolbar_h() - 1,
              "a TOP toolbar puts its hairline at the bottom",
              rule ? rule->y : -1, ui_toolbar_h() - 1);
        begin_draw();
        ui_toolbar(0, 100, 500, ui_toolbar_h(), 1);
        rule = find_rgb(ZD_SURF_2);
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
        begin_draw();
        ui_mono_panel(0, 0, 400, 300, UI_PANEL_TERM);
        ok(find_rgb(ZD_SURF_2) != NULL,
           "the terminal ground is the same surface an active tab uses");
        begin_draw();
        ui_mono_panel(0, 0, 400, 300, UI_PANEL_HEX);
        ok(find_rgb(ZD_SURF_1) != NULL, "the hex ground is one step darker");
        begin_draw();
        ui_mono_line(10, 10, 380, "zl: 3 warnings", ZD_TEXT_1, UI_PANEL_TERM, 0);
        ok(find_text("zl: 3 warnings") != NULL, "a plain line is just text");
        oknum(count_kind(OP_RBLEND) == 0, "control: ...with no highlight behind it",
              count_kind(OP_RBLEND), 0);
        begin_draw();
        ui_mono_line(10, 10, 380, "kernel.zl:88: note", ZD_TEXT_1,
                     UI_PANEL_TERM, 1);
        ok(count_kind(OP_RBLEND) == 1,
           "a jump-target line gets exactly one accent wash");
    }

    /* ============================================ RULE 1: NO COLOUR LITERAL */
    printf("\n  rule 1 - a colour literal may appear in design.h and nowhere else\n");
    {
        char *uic = slurp("../ui.c");
        char *kit = slurp("../uikit.c");
        char *dsg = slurp("../design.h");
        int draw = count_colour_literals(uic) + count_colour_literals(kit);
        strip_comments(uic); strip_comments(kit);
        int a = count_colour_literals(uic), b = count_colour_literals(kit);
        int d = count_colour_literals(dsg);
        oknum(a == 0, "ui.c carries no six-digit colour literal", a, 0);
        oknum(b == 0, "uikit.c carries no six-digit colour literal", b, 0);
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

    printf("\n%s   %d checks, %d failure%s\n",
           fails ? "FAILED" : "all passed", checks, fails,
           fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
