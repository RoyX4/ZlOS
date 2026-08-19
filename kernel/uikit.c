/* uikit.c - the shared widget catalogue the 53 apps are built out of.
 *
 * WHY THIS IS NOT IN ui.c. ui.c is the LAYOUT CURSOR: one flowing algorithm,
 * a handful of widgets that use it, and the funnel that decides whether
 * anything fired. This file is a CATALOGUE - fifty-odd shapes, each of which
 * knows its own geometry and draws where it is told. Mixing the two makes one
 * 2000-line file where the interesting 200 lines are the cursor.
 *
 * The two files are not peers. Everything here calls DOWN into ui.c's funnel
 * (ui_fire / ui_hit / ui_ring) and into fb.c; nothing in ui.c calls up into
 * this file except ui_row_select, which lives in ui.c precisely because both
 * need it and a shared decision must have one home. If a widget here ever
 * needs more of ui.c's cursor than the published seven functions, it belongs
 * in ui.c instead of behind a wider window into it.
 *
 * THE SPECIFICATION IS kernel/docs/reference-widgets.md - 68 widget records
 * measured out of docs/design/ds-reference.html, with the line numbers. Every
 * section reference below (S3, S13.1, S20.2 ...) is into that document. Do not
 * re-derive a number from the HTML; it has already been measured, and the
 * places where the reference contradicts itself have already been indexed.
 *
 * THREE RULES THIS FILE KEEPS, AND THEY ARE THE WHOLE POINT OF IT
 *
 * 1. NO COLOUR LITERAL. Every colour is a ZD_* token from design.h or a
 *    ui_color() role. hosttest/uitest.c scans this file's text for #rrggbb and
 *    0xRRGGBB and fails the build if one appears.
 *
 * 2. NO DEVICE PIXEL LITERAL. Every distance goes through UI_DP(), so the
 *    toolkit is correct at every UI scale rather than at the one somebody
 *    looked at. The numbers going in are the reference's own design pixels,
 *    and they live in design.h, not here.
 *
 * 3. NO HOVER INVENTED BY DEFAULT. reference-widgets.md S0 and S21.1: the
 *    reference has FIVE hover rules in 4338 lines and no :focus anywhere. Its
 *    interaction language is SELECTION. Where this file adds a pointer-state
 *    or keyboard-focus visual it is marked NEW DESIGN in a comment and kept to
 *    one surface step, because zlOS does have a pointer and a keyboard focus
 *    ring and losing those would be a regression, not fidelity.
 */

#include "ui.h"
#include "design.h"

/* ---- fb.c, the layer below ------------------------------------------------ */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_fill_blend(int x, int y, int w, int h, unsigned int rgb, int a);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_line(int x0, int y0, int x1, int y1, unsigned int rgb);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
void fb_text_role(int px, int py, const char *s, unsigned int fg,
                  int role, int weight);
int  fb_text_role_w(const char *s, int role, int weight);
int  fb_text_role_h(int role);
void fb_text_aa(int px, int py, const char *s, unsigned int fg);
int  fb_cell_w(void);
int  fb_cell_h(void);

/* UI_SM/MD/LG are fb.c's TEXT_CAPTION/BODY/TITLE and UI_F_BOLD is its
 * TEXT_BOLD. Static-asserted rather than commented, because the two files are
 * compiled separately and a renumber in fb.c would otherwise shift every
 * widget's type size silently. */
_Static_assert(UI_SM == 0 && UI_MD == 1 && UI_LG == 2,
               "UI_SM/MD/LG must equal fb.c's TEXT_CAPTION/BODY/TITLE");
_Static_assert(UI_F_BOLD == 2, "UI_F_BOLD is used as fb.c's weight != 0");

#define T (ui_theme())
#define DP(n) UI_DP(T, (n))

static int imax(int a, int b) { return a > b ? a : b; }
static int imin(int a, int b) { return a < b ? a : b; }
static int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static int pct255(int p) { return clamp(p, 0, 100) * 255 / 100; }

static int ui_strlen(const char *s)
{
    int n = 0;
    if (!s) return 0;
    while (s[n]) n++;
    return n;
}

/* ---- the '|'-separated item list -------------------------------------------
 * One string is the only container that crosses the zl boundary, so every
 * multi-item widget takes "CPU|Memory|Disk". Items are copied into a small
 * fixed buffer for drawing - the toolkit allocates nothing, and a label longer
 * than the buffer is truncated rather than overrunning it. */
#define UI_ITEM_MAX 48

int ui_items_count(const char *items)
{
    int n;
    if (!items || !items[0]) return 0;
    n = 1;
    for (int i = 0; items[i]; i++) if (items[i] == '|') n++;
    return n;
}

/* Copy item `idx` into `out`. Returns its length, or -1 when idx is past the
 * end - so a caller can walk without counting first. */
static int item_at(const char *items, int idx, char *out, int cap)
{
    int cur = 0, n = 0;
    out[0] = 0;
    if (!items || idx < 0) return -1;
    if (!items[0]) return -1;
    for (int i = 0;; i++) {
        char c = items[i];
        if (c == '|' || c == 0) {
            if (cur == idx) { out[n] = 0; return n; }
            cur++;
            n = 0;
            if (c == 0) return -1;
            continue;
        }
        if (cur == idx && n < cap - 1) out[n++] = c;
    }
}

/* ---- text ------------------------------------------------------------------
 * Three sizes, not eight. reference-widgets.md S1.1 counts eight font sizes
 * between 9 and 12.5 px; fb.c generates three atlases (16/24/32 at ui 2) and
 * resamples between them. Collapsing 9/9.5/10/10.5 onto SM, 11/11.5/12/12.5
 * onto MD and the display numerals onto LG is a real loss of fidelity and is
 * recorded as one - a port differs from the reference by up to 1.5 px of type.
 * The alternative is eight generated atlases in a kernel with no font server.
 *
 * MONO IS ONE SIZE. fb.c's own note: "FBT_MONO routes to the fixed-cell AA
 * font, which is ONE size ... honoured for proportional text and ignored for
 * mono, consistently between the measure and the draw". Kept consistent here
 * for the same reason: a measure and a draw that disagree is a clipped label. */
int ui_text_h(int size)
{
    return fb_text_role_h(clamp(size, UI_SM, UI_LG));
}

int ui_text_w(const char *s, int size, int flags)
{
    if (!s) return 0;
    if (flags & UI_F_MONO) return ui_strlen(s) * fb_cell_w();
    return fb_text_role_w(s, clamp(size, UI_SM, UI_LG),
                          (flags & UI_F_BOLD) ? 1 : 0);
}

void ui_text(int x, int y, const char *s, unsigned rgb, int size, int flags)
{
    if (!s || ui_mode_get() != UI_DRAW) return;
    if (flags & UI_F_MONO) { fb_text_aa(x, y, s, rgb); return; }
    fb_text_role(x, y, s, rgb, clamp(size, UI_SM, UI_LG),
                 (flags & UI_F_BOLD) ? 1 : 0);
}

/* Vertically centre a line of this size inside a box of height h. */
static int text_cy(int y, int h, int size, int flags)
{
    int th = (flags & UI_F_MONO) ? fb_cell_h() : ui_text_h(size);
    return y + (h - th) / 2;
}

/* ---- the neutral fill ------------------------------------------------------
 * rgba(255,255,255,.07), 14 uses, the most common button face in the file.
 * It is white at an alpha rather than a surface step, so it takes the colour
 * of whatever it is drawn on - which is why the same pill reads correctly on
 * the window body, a toolbar and a modal without three variants. */
static void neutral_fill(int x, int y, int w, int h, int r, int alpha_pct)
{
    fb_rrect_blend(x, y, w, h, r, (unsigned)ZD_INK_LIGHT, pct255(alpha_pct));
}

/* ============================================================================
 * BUTTONS - reference-widgets.md S13
 *
 * S13.1 catalogues TWENTY-SIX near-identical pills and reports that no two
 * agree on all of padding, radius and size. Its own recommended collapse is
 * three sizes plus two flags, and that is what this is:
 *
 *     sm   3px  9px  / r9  / 10px
 *     md   5-6px 12-13px / r11 / 11px
 *     lg   7px 13-17px / r12 / 11.5px
 *
 * RECORDED, AS THAT SECTION ASKS: the reference does NOT collapse cleanly.
 * Where it writes a range above, one value was picked (design.h says which),
 * so a port built from this differs from the reference by 1-4 px in places -
 * `swBtnStyle` is 7px 17px and comes out 7px 15px here, `imgZoom` is 5px 9px
 * and comes out 3px 9px. That is the cost of having a button instead of
 * twenty-six buttons, and it was accepted deliberately.
 * ========================================================================= */
static void pill_metrics(int size, int *py, int *px, int *r)
{
    switch (clamp(size, UI_SM, UI_LG)) {
    case UI_SM: *py = DP(ZD_PILL_SM_PY); *px = DP(ZD_PILL_SM_PX);
                *r  = DP(ZD_PILL_SM_R);  break;
    case UI_LG: *py = DP(ZD_PILL_LG_PY); *px = DP(ZD_PILL_LG_PX);
                *r  = DP(ZD_PILL_LG_R);  break;
    default:    *py = DP(ZD_PILL_MD_PY); *px = DP(ZD_PILL_MD_PX);
                *r  = DP(ZD_PILL_MD_R);  break;
    }
}

int ui_pill_h(int size)
{
    int py, px, r;
    pill_metrics(size, &py, &px, &r);
    return 2 * py + ui_text_h(size);
}

int ui_pill_w(const char *s, int size, int flags)
{
    int py, px, r;
    pill_metrics(size, &py, &px, &r);
    return 2 * px + ui_text_w(s, size, flags);
}

/* The face and the ink of one button kind, together.
 *
 * S13.2's BUG IS NOT REPRODUCED. Three reference variants (spinBtnStyle 3707,
 * renameOkStyle 4302, the window hamburger 3121) hard-code '#fff' on the
 * accent where the computed INK is the dark #0c1005. On the default lime that
 * is white-on-light-green - a contrast failure the reference's own INK
 * computation exists to prevent. PRIMARY here routes through ui_ink_on(), so
 * it is right for any accent the Settings pane can select, including the two
 * light alternates where a stored dark ink would fail the other way. */
static void pill_face(int x, int y, int w, int h, int r, int kind, int on,
                      unsigned *ink)
{
    switch (kind) {
    case UI_BTN_PRIMARY:
        fb_rrect(x, y, w, h, r, ui_color(UI_COLOR_ACCENT));
        *ink = ui_ink_on(ui_color(UI_COLOR_ACCENT));
        break;
    case UI_BTN_DANGER:
        /* S13.3: killStyle is BAD at 16% with a 40% border and #ffb3a4 text.
         * S20.7 records a SECOND destructive red, rgba(212,105,90,...), used
         * in four places and never as a hex - a phantom colour with no token.
         * One red, and it is BAD. */
        fb_rrect_blend(x, y, w, h, r, (unsigned)ZD_BAD, pct255(16));
        fb_rrect_blend(x, y, w, 1, 0, (unsigned)ZD_BAD, pct255(40));
        fb_rrect_blend(x, y + h - 1, w, 1, 0, (unsigned)ZD_BAD, pct255(40));
        *ink = (unsigned)ZD_BAD_SOFT;
        break;
    case UI_BTN_GHOST:
        if (on) neutral_fill(x, y, w, h, r, ZD_FILL_SOFT_A);
        *ink = on ? ui_color(UI_COLOR_TEXT_HI) : ui_color(UI_COLOR_TEXT_DIM);
        break;
    default:
        if (on) {
            fb_rrect(x, y, w, h, r, ui_color(UI_COLOR_ACCENT));
            *ink = ui_ink_on(ui_color(UI_COLOR_ACCENT));
        } else {
            neutral_fill(x, y, w, h, r, ZD_FILL_A);
            *ink = ui_color(UI_COLOR_TEXT);
        }
        break;
    }
}

int ui_pill(int x, int y, int w, int h, const char *s,
            int size, int kind, int flags)
{
    int py, px, r;
    pill_metrics(size, &py, &px, &r);
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        unsigned ink = ui_color(UI_COLOR_TEXT);
        pill_face(x, y, w, h, r, kind, 0, &ink);
        int tw = ui_text_w(s, size, flags);
        ui_text(x + (w - tw) / 2, text_cy(y, h, size, flags), s, ink, size, flags);
        ui_ring(x, y, w, h);
    }
    return fired;
}

/* The cursor-flowing form, for an app laying widgets out rather than placing
 * them. ui_button() stays exactly as it was - it is the MD/NEUTRAL case and
 * fifty call sites already use it - and this is the same pill through ui.c's
 * layout cursor, so a form can mix a primary and a neutral button in a row. */
int ui_button_sz(const char *s, int size, int kind, int flags)
{
    int x, y, w = ui_pill_w(s, size, flags), h = ui_pill_h(size);
    ui_place(w, h, &x, &y);
    return ui_pill(x, y, w, h, s, size, kind, flags);
}

/* S13.4 / S2.3: 26x26 r16 in the title bar, 22x22 r14 in the find bar, both
 * rgba(255,255,255,.08) with a #b9bec4 glyph. One function, `px` is the box. */
int ui_icon_button(int x, int y, int px, const char *glyph, int active)
{
    int d = DP(px), r = DP(px >= ZD_WINCTL ? ZD_R_16 : ZD_R_14);
    int fired = ui_fire(x, y, d, d);
    if (ui_mode_get() == UI_DRAW) {
        unsigned ink;
        if (active) {
            fb_rrect(x, y, d, d, r, ui_color(UI_COLOR_ACCENT));
            ink = ui_ink_on(ui_color(UI_COLOR_ACCENT));   /* NOT #fff - S20.1 */
        } else {
            neutral_fill(x, y, d, d, r, ZD_FILL_ICON_A);
            ink = (unsigned)ZD_WINCTL_INK;
        }
        int tw = ui_text_w(glyph, UI_SM, 0);
        ui_text(x + (d - tw) / 2, text_cy(y, d, UI_SM, 0), glyph, ink, UI_SM, 0);
        ui_ring(x, y, d, d);
    }
    return fired;
}

/* ============================================================================
 * SEGMENTED CONTROL - S3.  System Monitor, Kernel Log, Renderer, Font Atlas.
 *
 * Container #090a0c + 1px #1c2024 + r11 + 2px padding + 2px gap; items r9,
 * active ACC/INK, inactive transparent/#9ba0a6. Four instances, four item
 * paddings and three font sizes (S20.10) - hence the size parameter. Font
 * Atlas alone uses #07080a for the container; ignored, one instance.
 * ========================================================================= */
static void seg_item_metrics(int size, int *py, int *px)
{
    switch (clamp(size, UI_SM, UI_LG)) {
    case UI_SM: *py = DP(3); *px = DP(10); break;
    case UI_LG: *py = DP(5); *px = DP(15); break;
    default:    *py = DP(ZD_SEG_ITEM_PY); *px = DP(ZD_SEG_ITEM_PX); break;
    }
}

int ui_seg_h(int size)
{
    int py, px;
    seg_item_metrics(size, &py, &px);
    return 2 * DP(ZD_SEG_PAD) + 2 * py + ui_text_h(size);
}

int ui_seg_w(const char *items, int size)
{
    char buf[UI_ITEM_MAX];
    int py, px, n = ui_items_count(items);
    int w = 2 * DP(ZD_SEG_PAD);
    seg_item_metrics(size, &py, &px);
    for (int i = 0; i < n; i++) {
        item_at(items, i, buf, sizeof buf);
        w += 2 * px + ui_text_w(buf, size, 0);
        if (i) w += DP(ZD_SEG_GAP);
    }
    return w;
}

int ui_segmented(int x, int y, int w, int h, const char *items, int sel, int size)
{
    char buf[UI_ITEM_MAX];
    int n = ui_items_count(items), hitidx = -1;
    int pad = DP(ZD_SEG_PAD), gap = DP(ZD_SEG_GAP);
    int py, px;
    seg_item_metrics(size, &py, &px);
    if (n <= 0) return -1;

    if (ui_mode_get() == UI_DRAW) {
        fb_rrect(x, y, w, h, DP(ZD_SEG_R), (unsigned)ZD_SURF_1);
        fb_box(x, y, w, h, (unsigned)ZD_SURF_5);
    }

    /* THE ITEMS DIVIDE THE TRACK EQUALLY. The reference sizes each item to its
     * own label, which makes the control jump when a label changes - and this
     * toolkit has to serve apps whose labels are counts. Equal division is a
     * deliberate divergence, and it is also what stops the last item falling
     * off the end when a caller passes a width narrower than ui_seg_w(). */
    int inner = w - 2 * pad - (n - 1) * gap;
    int iw = inner > 0 ? inner / n : 0;
    int ih = h - 2 * pad;
    for (int i = 0; i < n; i++) {
        int ix = x + pad + i * (iw + gap);
        int len = item_at(items, i, buf, sizeof buf);
        if (len < 0) break;
        if (ui_fire(ix, y + pad, iw, ih)) hitidx = i;
        if (ui_mode_get() != UI_DRAW) continue;
        unsigned ink;
        if (i == sel) {
            fb_rrect(ix, y + pad, iw, ih, DP(ZD_SEG_ITEM_R),
                     ui_color(UI_COLOR_ACCENT));
            ink = ui_ink_on(ui_color(UI_COLOR_ACCENT));
        } else {
            ink = ui_color(UI_COLOR_TEXT_DIM);
        }
        int tw = imin(ui_text_w(buf, size, 0), iw);
        ui_text(ix + (iw - tw) / 2, text_cy(y + pad, ih, size, 0), buf, ink,
                size, 0);
        ui_ring(ix, y + pad, iw, ih);
    }
    return hitidx;
}

int ui_segmented_value(int x, int y, int w, int h, const char *items,
                       int sel, int size)
{
    int got = ui_segmented(x, y, w, h, items, sel, size);
    return got >= 0 ? got : sel;
}

/* ============================================================================
 * TAB STRIPS - S4.  Three unrelated idioms exist in the reference; two are
 * ported (closeable and underline) and the third, the Image Viewer's loose
 * pill row, is just ui_pill in a row and needs no widget of its own.
 * ========================================================================= */
static int tab_closed_idx = -1;

int ui_tabs_h(void) { return DP(ZD_TAB_H); }

int ui_tabstrip(int x, int y, int w, const char *items, int sel)
{
    char buf[UI_ITEM_MAX];
    int n = ui_items_count(items), hitidx = -1;
    int h = ui_tabs_h(), pt = DP(ZD_TAB_PAD_T), pxo = DP(ZD_TAB_PAD_X);
    int gap = DP(ZD_TAB_GAP), pl = DP(ZD_TAB_PL), pr = DP(ZD_TAB_PR);
    int xw = DP(ZD_TAB_X), maxw = DP(ZD_TAB_MAXW), r = DP(ZD_TAB_R);

    tab_closed_idx = -1;
    if (ui_mode_get() == UI_DRAW) {
        fb_fill_px(x, y, w, h, (unsigned)ZD_SURF_TABS);
        fb_fill_px(x, y + h - 1, w, 1, (unsigned)ZD_SURF_2);
    }
    if (n <= 0) return -1;

    int cx = x + pxo, th = h - pt;
    for (int i = 0; i < n; i++) {
        int len = item_at(items, i, buf, sizeof buf);
        if (len < 0) break;
        int tw = imin(pl + ui_text_w(buf, UI_MD, 0) + DP(7) + xw + pr, maxw);
        if (cx + tw > x + w - pxo) tw = x + w - pxo - cx;
        if (tw <= 0) break;
        int ty = y + pt;
        /* the ✕ first: it sits INSIDE the tab, so a hit on it must not also
         * count as a hit on the tab - test the smaller rect first and return */
        int xx = cx + tw - pr - xw;
        if (ui_fire(xx, ty, xw, th)) tab_closed_idx = i;
        else if (ui_fire(cx, ty, tw - pr - xw, th)) hitidx = i;

        if (ui_mode_get() == UI_DRAW) {
            /* "a true connected tab": the active one is painted the colour of
             * the body BELOW it, so the seam disappears. S4.1. */
            if (i == sel) fb_rrect(cx, ty, tw, th + r, r, (unsigned)ZD_SURF_2);
            else          neutral_fill(cx, ty, tw, th + r, r, 4);
            unsigned ink = (i == sel) ? ui_color(UI_COLOR_TEXT)
                                      : (unsigned)ZD_TEXT_5;
            int room = tw - pl - pr - xw;
            if (room > 0)
                ui_text(cx + pl, text_cy(ty, th, UI_MD, 0), buf, ink, UI_MD, 0);
            ui_text(xx, text_cy(ty, th, UI_SM, 0), "x",
                    (i == sel) ? ui_color(UI_COLOR_TEXT_DIM)
                               : (unsigned)ZD_SURF_7, UI_SM, 0);
        }
        cx += tw + gap;
    }
    return hitidx;
}

int ui_tabstrip_closed(void) { return tab_closed_idx; }

int ui_tabstrip_value(int x, int y, int w, const char *items, int sel)
{
    int got = ui_tabstrip(x, y, w, items, sel);
    return got >= 0 ? got : sel;
}

int ui_utabs_h(void) { return DP(ZD_UTAB_H); }

int ui_utabs(int x, int y, int w, const char *items, int sel)
{
    char buf[UI_ITEM_MAX];
    int n = ui_items_count(items), hitidx = -1;
    int h = ui_utabs_h(), px = DP(ZD_UTAB_PX), rule = DP(ZD_UTAB_RULE);

    if (ui_mode_get() == UI_DRAW) {
        fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL_HI));
        fb_fill_px(x, y + h - 1, w, 1, (unsigned)ZD_SURF_2);
    }
    if (n <= 0) return -1;

    int cx = x;
    for (int i = 0; i < n; i++) {
        int len = item_at(items, i, buf, sizeof buf);
        if (len < 0) break;
        int tw = 2 * px + ui_text_w(buf, UI_MD, 0);
        if (cx + tw > x + w) break;
        if (ui_fire(cx, y, tw, h)) hitidx = i;
        if (ui_mode_get() == UI_DRAW) {
            /* S4.2: no background change at all - the 2px inset underline is
             * the whole affordance. */
            ui_text(cx + px, text_cy(y, h, UI_MD, 0), buf,
                    i == sel ? ui_color(UI_COLOR_TEXT_HI) : (unsigned)ZD_TEXT_5,
                    UI_MD, 0);
            if (i == sel)
                fb_fill_px(cx, y + h - rule, tw, rule, ui_color(UI_COLOR_ACCENT));
            ui_ring(cx, y, tw, h);
        }
        cx += tw;
    }
    return hitidx;
}

int ui_utabs_value(int x, int y, int w, const char *items, int sel)
{
    int got = ui_utabs(x, y, w, items, sel);
    return got >= 0 ? got : sel;
}

/* ============================================================================
 * TOOLBAR AND STATUS BAR - S5, S6
 *
 * S5 records fourteen toolbars at five heights and S6 five status bars at
 * three, with no content difference to justify the spread (S20.16). One height
 * each, and the rule the reference IS consistent about is kept: a top toolbar
 * sits on the window ground with a bottom hairline, a bottom toolbar sits on
 * the raised surface with a top hairline.
 * ========================================================================= */
int ui_toolbar_h(void) { return DP(ZD_TOOLBAR_H); }
int ui_status_h(void)  { return DP(ZD_STATUS_H); }

void ui_toolbar(int x, int y, int w, int h, int at_bottom)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h, at_bottom ? ui_color(UI_COLOR_PANEL_HI)
                                     : ui_color(UI_COLOR_PANEL));
    if (at_bottom) fb_fill_px(x, y, w, 1, (unsigned)ZD_SURF_2);
    else           fb_fill_px(x, y + h - 1, w, 1, (unsigned)ZD_SURF_2);
}

void ui_statusbar(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL_HI));
    fb_fill_px(x, y, w, 1, (unsigned)ZD_SURF_2);
}

/* ============================================================================
 * COLUMN HEADER + LIST ROW - S7
 *
 * The reference declares its grid track string TWICE per table - once in the
 * header and once in the row - so the two stay aligned (S7.1). Here they are
 * declared once, in the module state below, and both the header and the row
 * read it. That is a deliberate improvement rather than a port: S20.5 records
 * pcolGrid missing the 15px scrollbar gutter its two siblings have, so System
 * Monitor's headers sit 15px off its rows in the reference. One track table
 * makes that class of bug impossible.
 * ========================================================================= */
#define UI_GRID_MAX 8
static struct {
    int n;
    int fixed[UI_GRID_MAX];   /* design px, or 0 for the '*' track */
    int star;                 /* which track is the 1fr, or -1     */
} G = { 0, { 0 }, -1 };

void ui_grid(const char *tracks)
{
    G.n = 0;
    G.star = -1;
    if (!tracks) return;
    for (int i = 0; tracks[i] && G.n < UI_GRID_MAX;) {
        if (tracks[i] == '*') {
            G.fixed[G.n] = 0;
            G.star = G.n;
            G.n++;
            i++;
        } else if (tracks[i] >= '0' && tracks[i] <= '9') {
            int v = 0;
            while (tracks[i] >= '0' && tracks[i] <= '9')
                v = v * 10 + (tracks[i++] - '0');
            G.fixed[G.n++] = v;
        } else {
            i++;                    /* commas and anything else: skip */
        }
    }
}

int ui_grid_cols(void) { return G.n; }

void ui_grid_span(int x, int w, int col, int *cx, int *cw)
{
    int pl = DP(ZD_COLHEAD_PL), pr = DP(ZD_COLHEAD_PR);
    int avail = w - pl - pr, used = 0;
    if (cx) *cx = x;
    if (cw) *cw = 0;
    if (col < 0 || col >= G.n) return;
    for (int i = 0; i < G.n; i++) used += DP(G.fixed[i]);
    int star_w = avail - used;
    if (star_w < 0) star_w = 0;

    int run = x + pl;
    for (int i = 0; i < G.n; i++) {
        int tw = (i == G.star) ? star_w : DP(G.fixed[i]);
        /* minmax(0,Npx): a fixed track may shrink, never grow. When the box is
         * too narrow for the fixed tracks the star collapses first and then
         * the fixed ones are truncated at the right edge - the alternative,
         * letting them overflow, puts a column outside the scissor. */
        if (run + tw > x + w - pr) tw = imax(0, x + w - pr - run);
        if (i == col) {
            if (cx) *cx = run;
            if (cw) *cw = tw;
            return;
        }
        run += tw;
    }
}

int ui_colhead_h(void) { return DP(ZD_COLHEAD_H); }

int ui_colhead(int x, int y, int w, const char *labels, int sortcol, int sortdir)
{
    char buf[UI_ITEM_MAX];
    int h = ui_colhead_h(), hitidx = -1;
    if (ui_mode_get() == UI_DRAW) {
        fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL));
        fb_fill_px(x, y + h - 1, w, 1, (unsigned)ZD_SURF_2);
    }
    for (int i = 0; i < G.n; i++) {
        int cx, cw;
        ui_grid_span(x, w, i, &cx, &cw);
        if (cw <= 0) continue;
        if (item_at(labels, i, buf, sizeof buf) < 0) break;
        if (ui_fire(cx, y, cw, h)) hitidx = i;
        if (ui_mode_get() != UI_DRAW) continue;
        unsigned ink = (i == sortcol) ? ui_color(UI_COLOR_TEXT_HI)
                                      : ui_color(UI_COLOR_TEXT_DIM);
        int tw = ui_text_w(buf, UI_SM, 0);
        int tx = (G.fixed[i] && i != G.star) ? cx + cw - tw - DP(10)
                                             : cx + DP(8);
        if (tx < cx) tx = cx;
        ui_text(tx, text_cy(y, h, UI_SM, 0), buf, ink, UI_SM, 0);
        if (i == sortcol) {
            /* the sort arrow: icon('sortU'|'sortD', 8) in the reference, a
             * 4px triangle of hairlines here - there is no icon atlas in the
             * toolkit layer and one glyph is not worth a dependency on it */
            int a = DP(4), ax = tx + tw + DP(4), ay = y + h / 2 - DP(2);
            for (int k = 0; k < a; k++) {
                /* the DIRECTION is the width ramp, not the y - a triangle that
                 * narrows downward points down and one that widens points up */
                int ww = sortdir >= 0 ? a - k : k + 1;
                fb_fill_px(ax + (a - ww) / 2, ay + k, ww, 1,
                           (unsigned)ZD_ACCENT_BR);
            }
        }
    }
    return hitidx;
}

int ui_grid_row_h(void) { return DP(ZD_LISTROW_H); }

int ui_grid_row(int x, int y, int w, int idx, int selected)
{
    int h = ui_grid_row_h();
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        ui_row_select(x, y, w, h, selected, idx & 1);
        ui_ring(x, y, w, h);
    }
    return fired;
}

void ui_grid_cell(int x, int w, int y, int h, int col, const char *s,
                  int align, unsigned rgb, int size, int flags)
{
    int cx, cw;
    if (ui_mode_get() != UI_DRAW) return;
    ui_grid_span(x, w, col, &cx, &cw);
    if (cw <= 0) return;
    int tw = ui_text_w(s, size, flags);
    int tx = cx + DP(8);
    if (align == UI_ALIGN_R) tx = cx + cw - tw - DP(10);
    else if (align == UI_ALIGN_C) tx = cx + (cw - tw) / 2;
    if (tx < cx) tx = cx;
    ui_text(tx, text_cy(y, h, size, flags), s, rgb, size, flags);
}

/* ============================================================================
 * STAT CARD STRIP - S8.  repeat(auto-fit, minmax(Npx,1fr)).
 *
 * The 1px gap over the hairline surface IS the hairline - the cells carry no
 * border of their own. S20.4 records four instances with three minmax values
 * and three paddings and nothing in the content to explain it; 88px and
 * 7px 9px are the modes and are the default here.
 * ========================================================================= */
static struct {
    int x, y, w, cols, cw, ch, i, minw;
} SC;

void ui_stat_begin(int x, int y, int w, int minw)
{
    SC.x = x; SC.y = y; SC.w = w; SC.i = 0;
    SC.minw = DP(minw > 0 ? minw : ZD_STAT_MIN);
    SC.cols = SC.minw > 0 ? w / SC.minw : 1;
    if (SC.cols < 1) SC.cols = 1;
    SC.cw = w / SC.cols;
    SC.ch = 2 * DP(ZD_STAT_PY) + ui_text_h(UI_SM) + DP(2) + ui_text_h(UI_MD);
}

void ui_stat_cell(const char *key, const char *val, unsigned val_rgb)
{
    /* CALLED OUT OF ORDER IS A DIVIDE BY ZERO, and zl can call this directly.
     * SC is static, so a program that calls ui_stat_cell without ever calling
     * ui_stat_begin arrives here with cols == 0 - which on this target is not
     * a wrong picture, it is a fault in ring 0. Every begin/item/end widget in
     * this file is guarded the same way, because "the app will call them in
     * order" is exactly the assumption an app written by somebody else breaks. */
    if (SC.cols < 1) return;
    int gap = DP(ZD_STAT_GAP);
    if (gap < 1) gap = 1;
    int r = SC.i / SC.cols, c = SC.i % SC.cols;
    int cx = SC.x + c * SC.cw, cy = SC.y + r * (SC.ch + gap);
    SC.i++;
    if (ui_mode_get() != UI_DRAW) return;
    /* the ground first, so the 1px gaps between cells show through as rules */
    if (c == 0) fb_fill_px(SC.x, cy - gap, SC.w, SC.ch + gap, (unsigned)ZD_SURF_2);
    fb_fill_px(cx, cy, SC.cw - gap, SC.ch, ui_color(UI_COLOR_PANEL_HI));
    int px = DP(ZD_STAT_PX), py = DP(ZD_STAT_PY);
    ui_text(cx + px, cy + py, key, (unsigned)ZD_TEXT_6, UI_SM, 0);
    ui_text(cx + px, cy + py + ui_text_h(UI_SM) + DP(2), val, val_rgb,
            UI_MD, UI_F_MONO);
}

int ui_stat_end(void)
{
    int gap = DP(ZD_STAT_GAP) < 1 ? 1 : DP(ZD_STAT_GAP);
    if (SC.cols < 1) return 0;
    int rows = SC.i ? (SC.i + SC.cols - 1) / SC.cols : 0;
    return rows * (SC.ch + gap);
}

/* ============================================================================
 * SIDEBAR - S9
 *
 * S20.6: six section headings, five treatments. The dominant shape is
 * 10px / 400 / .08em / uppercase / #5c6167 and that is what this draws; the
 * Files pair (10.5 / 700 / mixed case / #74797f) is the outlier and is not
 * reproduced. Letter-spacing is not available in fb.c's text path, so the
 * tracking is lost - recorded rather than faked with inserted spaces, which
 * would break every width measurement.
 * ========================================================================= */
int ui_sidebar_w(void) { return DP(ZD_SIDEBAR_W); }

void ui_sidebar(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL_HI));
    fb_fill_px(x + w - 1, y, 1, h, (unsigned)ZD_SURF_2);
}

int ui_heading_h(void)
{
    return DP(ZD_HEADING_PT) + ui_text_h(UI_SM) + DP(ZD_HEADING_PB);
}

void ui_heading(int x, int y, int w, const char *s)
{
    (void)w;
    if (ui_mode_get() != UI_DRAW) return;
    ui_text(x + DP(8), y + DP(ZD_HEADING_PT), s, (unsigned)ZD_TEXT_6, UI_SM, 0);
}

int ui_nav_h(void) { return DP(ZD_NAV_H); }

int ui_nav_row(int x, int y, int w, const char *s, int selected)
{
    int h = ui_nav_h();
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        ui_row_select(x, y, w, h, selected, 0);
        ui_text(x + DP(ZD_NAV_PX), text_cy(y, h, UI_MD, 0), s,
                selected ? (unsigned)ZD_ACCENT_PALE : (unsigned)ZD_TEXT_3,
                UI_MD, 0);
        ui_ring(x, y, w, h);
    }
    return fired;
}

/* ============================================================================
 * BARS - S10.  Four bar widgets that share no radius and no track colour
 * (S20.11), reproduced as four because each one's proportions are load-bearing
 * where it is used: the meter is a hairline inside a card, the segment bar is
 * a container of parts, the mini-bar is an inline sparkbar in a list row.
 * ========================================================================= */
int ui_meter_h(void)    { return imax(2, DP(ZD_METER_H)); }
int ui_progress_h(void) { return imax(2, DP(ZD_PROG_H)); }
int ui_minibar_h(void)  { return imax(2, DP(ZD_MINIBAR_H)); }
int ui_segbar_h(void)   { return imax(4, DP(ZD_SEGBAR_H)); }

void ui_meter(int x, int y, int w, int pct, unsigned rgb)
{
    int h = ui_meter_h(), r = DP(ZD_METER_R);
    if (ui_mode_get() != UI_DRAW) return;
    pct = clamp(pct, 0, 100);
    fb_rrect(x, y, w, h, r, (unsigned)ZD_SURF_1);
    if (pct) fb_rrect(x, y, w * pct / 100, h, r, rgb);
}

void ui_progress(int x, int y, int w, int pct, unsigned rgb)
{
    int h = ui_progress_h(), r = DP(ZD_PROG_R);
    if (ui_mode_get() != UI_DRAW) return;
    pct = clamp(pct, 0, 100);
    fb_rrect(x, y, w, h, r, (unsigned)ZD_SURF_WELL);
    if (pct) fb_rrect(x, y, w * pct / 100, h, r, rgb);
}

void ui_minibar(int x, int y, int w, int pct, unsigned rgb)
{
    int h = ui_minibar_h(), r = DP(ZD_MINIBAR_R);
    if (ui_mode_get() != UI_DRAW) return;
    pct = clamp(pct, 0, 100);
    fb_rrect(x, y, w, h, r, (unsigned)ZD_SURF_0);
    /* the reference floors the fill at 2% so a non-zero value is never
     * invisible - a 0px bar and a 0-value bar mean different things */
    int fw = w * imax(pct, pct ? 2 : 0) / 100;
    if (fw) fb_rrect(x, y, fw, h, r, rgb);
}

static struct { int x, y, w, total, used; } SB;

void ui_segbar_begin(int x, int y, int w, int total)
{
    SB.x = x; SB.y = y; SB.w = w; SB.total = total > 0 ? total : 1; SB.used = 0;
    if (ui_mode_get() != UI_DRAW) return;
    int h = ui_segbar_h();
    fb_rrect(x, y, w, h, DP(ZD_SEGBAR_R), (unsigned)ZD_SURF_0);
    fb_box(x, y, w, h, (unsigned)ZD_SURF_5);
}

void ui_segbar_item(int amount, unsigned rgb)
{
    if (SB.total < 1) return;              /* see ui_stat_cell - same hazard */
    int pad = imax(1, DP(1)), h = ui_segbar_h();
    int inner = SB.w - 2 * pad;
    if (amount < 0) amount = 0;
    int sx = SB.x + pad + (int)((long)SB.used * inner / SB.total);
    int ex = SB.x + pad + (int)((long)(SB.used + amount) * inner / SB.total);
    SB.used += amount;
    if (ui_mode_get() != UI_DRAW) return;
    if (ex - sx > 0)
        fb_rrect(sx, SB.y + pad, ex - sx, h - 2 * pad, DP(ZD_SEGBAR_SEG_R), rgb);
}

void ui_segbar_end(void) { }

/* ============================================================================
 * MONOSPACE PANEL - S14.  Five instances, no two sharing a background, size
 * and line-height. Three grounds are kept because they are load-bearing: the
 * terminal/editor ground is the same colour as an active tab (that is how the
 * tab connects to the body), and the hex/inset ground is one step darker.
 * ========================================================================= */
void ui_mono_panel(int x, int y, int w, int h, int kind)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h,
               kind == UI_PANEL_HEX ? (unsigned)ZD_SURF_1 : (unsigned)ZD_SURF_2);
}

int ui_mono_line_h(int kind)
{
    /* line-height 1.5 / 1.55 / 1.5 in the reference; 3/2 of the cell here,
     * which is the same number without a fractional multiply. */
    (void)kind;
    return fb_cell_h() * 3 / 2;
}

void ui_mono_line(int x, int y, int w, const char *s, unsigned rgb, int kind,
                  int highlight)
{
    if (ui_mode_get() != UI_DRAW) return;
    if (highlight) {
        /* S14.1's jump-target line: a 7% accent wash that bleeds 4px past the
         * panel padding via a negative margin. The bleed is dropped - a
         * negative margin has no meaning against a scissor - and the wash is
         * kept, because it is the only thing marking a clickable diagnostic. */
        int pad = DP(4);
        fb_rrect_blend(x - pad, y, w + 2 * pad, ui_mono_line_h(kind), DP(ZD_R_7),
                       ui_color(UI_COLOR_ACCENT), pct255(7));
    }
    fb_text_aa(x, y, s, rgb);
}

/* ============================================================================
 * CARDS AND KEY/VALUE - S15.  The dominant card is #14171a / 1px #1c2024 /
 * r13, eight uses; System Info's #1a1d21 / r12 and the ping panel's third
 * shape (S20.14) are not reproduced.
 * ========================================================================= */
void ui_card(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    int r = DP(ZD_CARD_R);
    fb_rrect(x, y, w, h, r, ui_color(UI_COLOR_PANEL_HI));
    /* the inner rrect stays one pixel tighter so the hairline follows the
     * outer curve instead of cutting across it - ui.c's own rule */
    fb_box(x, y, w, h, (unsigned)ZD_SURF_5);
}

int ui_card_head_h(void) { return DP(ZD_CARD_HEAD_H); }

void ui_card_head(int x, int y, int w, const char *title,
                  const char *badge, unsigned badge_rgb)
{
    int h = ui_card_head_h();
    if (ui_mode_get() != UI_DRAW) return;
    ui_text(x + DP(12), text_cy(y, h, UI_MD, UI_F_BOLD), title,
            ui_color(UI_COLOR_TEXT_HI), UI_MD, UI_F_BOLD);
    if (badge && badge[0]) {
        int bw = ui_badge_w(badge);
        ui_badge(x + w - DP(12) - bw, y + (h - ui_badge_h()) / 2, badge,
                 badge_rgb);
    }
    fb_fill_px(x, y + h - 1, w, 1, (unsigned)ZD_SURF_5);
}

int ui_kv_h(void) { return DP(ZD_KV_H); }

void ui_kv(int x, int y, int w, const char *k, const char *v,
           unsigned v_rgb, int first)
{
    int h = ui_kv_h();
    if (ui_mode_get() != UI_DRAW) return;
    /* "cards that hold rows use borderTop on rows 2..n rather than a gap" */
    if (!first) fb_fill_px(x, y, w, 1, (unsigned)ZD_SURF_5);
    ui_text(x + DP(13), text_cy(y, h, UI_SM, 0), k, ui_color(UI_COLOR_TEXT_DIM),
            UI_SM, 0);
    int vw = ui_text_w(v, UI_SM, UI_F_MONO);
    ui_text(x + w - DP(13) - vw, text_cy(y, h, UI_SM, UI_F_MONO), v, v_rgb,
            UI_SM, UI_F_MONO);
}

/* ============================================================================
 * OVERLAYS - S16
 * ========================================================================= */
void ui_popover(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_rrect(x, y, w, h, DP(ZD_MENU_R), (unsigned)ZD_SURF_5);
    fb_box(x, y, w, h, (unsigned)ZD_SURF_6);
}

int ui_menu_w(const char *items)
{
    char buf[UI_ITEM_MAX];
    int n = ui_items_count(items), w = 0;
    for (int i = 0; i < n; i++) {
        item_at(items, i, buf, sizeof buf);
        w = imax(w, ui_text_w(buf, UI_MD, 0));
    }
    w += 2 * DP(ZD_MENU_ITEM_PX) + 2 * DP(ZD_MENU_PAD) + DP(ZD_MENU_GAP);
    return imax(w, DP(ZD_MENU_W) / 2);
}

int ui_menu_h(const char *items)
{
    return ui_items_count(items) * DP(ZD_MENU_ITEM_H) + 2 * DP(ZD_MENU_PAD);
}

int ui_menu(int x, int y, const char *items, int sel)
{
    char buf[UI_ITEM_MAX];
    int n = ui_items_count(items), hitidx = -1;
    int w = ui_menu_w(items), h = ui_menu_h(items);
    int pad = DP(ZD_MENU_PAD), ih = DP(ZD_MENU_ITEM_H);
    ui_popover(x, y, w, h);
    for (int i = 0; i < n; i++) {
        int iy = y + pad + i * ih;
        if (item_at(items, i, buf, sizeof buf) < 0) break;
        if (ui_fire(x + pad, iy, w - 2 * pad, ih)) hitidx = i;
        if (ui_mode_get() != UI_DRAW) continue;
        if (i == sel)
            fb_rrect_blend(x + pad, iy, w - 2 * pad, ih, DP(ZD_MENU_ITEM_R),
                           ui_color(UI_COLOR_ACCENT), pct255(ZD_SEL_TINT_A));
        ui_text(x + pad + DP(ZD_MENU_ITEM_PX), text_cy(iy, ih, UI_MD, 0), buf,
                ui_color(UI_COLOR_TEXT), UI_MD, 0);
        ui_ring(x + pad, iy, w - 2 * pad, ih);
    }
    return hitidx;
}

int ui_modal_head_h(void) { return DP(ZD_MODAL_HEAD_H); }

void ui_modal(int x, int y, int w, int h, const char *title)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_rrect(x, y, w, h, DP(ZD_MODAL_R), (unsigned)ZD_SURF_5);
    fb_box(x, y, w, h, (unsigned)ZD_SURF_6);
    if (title && title[0]) {
        int hh = ui_modal_head_h();
        int tw = ui_text_w(title, UI_MD, UI_F_BOLD);
        ui_text(x + (w - tw) / 2, text_cy(y, hh, UI_MD, UI_F_BOLD), title,
                ui_color(UI_COLOR_TEXT_HI), UI_MD, UI_F_BOLD);
        fb_fill_px(x, y + hh - 1, w, 1, (unsigned)ZD_SURF_6);
    }
    /* NO SCRIM. S16.3: "There is no backdrop scrim on any of the three
     * modals." Only the command palette and the overview dim what is behind
     * them, and neither is this widget. */
}

int ui_toast_h(void)
{
    return 2 * DP(ZD_TOAST_PY) + ui_text_h(UI_MD) + DP(3) + ui_text_h(UI_SM);
}

void ui_toast_draw(int x, int y, int w, const char *title, const char *body,
                   unsigned kind_rgb)
{
    int h = ui_toast_h(), py = DP(ZD_TOAST_PY), px = DP(ZD_TOAST_PX);
    int ic = DP(ZD_TOAST_ICON), gap = DP(ZD_TOAST_GAP);
    if (ui_mode_get() != UI_DRAW) return;
    fb_rrect(x, y, w, h, DP(ZD_TOAST_R), (unsigned)ZD_SURF_CARD);
    fb_box(x, y, w, h, (unsigned)ZD_SURF_6);
    fb_rrect(x + px, y + py, ic, ic, DP(ZD_TOAST_ICON_R), kind_rgb);
    int tx = x + px + ic + gap;
    ui_text(tx, y + py, title, ui_color(UI_COLOR_TEXT_HI), UI_MD, UI_F_BOLD);
    ui_text(tx, y + py + ui_text_h(UI_MD) + DP(3), body,
            ui_color(UI_COLOR_TEXT_DIM), UI_SM, 0);
}

/* ============================================================================
 * CHART - S17, and the one place this toolkit knowingly draws something the
 * reference does not.
 *
 * ds-reference.html 3249 maps a CPU percentage to a y coordinate with
 *
 *     y = 66 - v / 100 * 66 * 1.9
 *
 * inside a viewBox 66 units tall. At v = 52.63 that reaches 0; above it the
 * point is outside the box and the SVG root clips, so EVERY CPU READING OVER
 * 52.63% RENDERS AS A FLAT LINE PINNED TO THE TOP. reference-widgets.md S20.18
 * calls it a bug in the reference rather than a style and says a port should
 * drop the 1.9 or clamp.
 *
 * This drops it AND clamps. The gain is 1, so 100% reaches the top of the box
 * and nothing above it exists; the clamp then catches a caller passing 120.
 * hosttest/uitest.c asserts that a 90% sample lands lower than a 60% one,
 * which is the assertion that fails if anyone ever "restores fidelity" here.
 * ========================================================================= */
static struct {
    int x, y, w, h, n;
    short v[ZD_SPARK_MAX];
} SP;

void ui_spark_begin(int x, int y, int w, int h)
{
    SP.x = x; SP.y = y; SP.w = w; SP.h = h > 0 ? h : DP(ZD_SPARK_H); SP.n = 0;
    if (ui_mode_get() != UI_DRAW) return;
    /* quarters, stroke #1c2024 - three lines, not four edges */
    for (int i = 1; i < ZD_SPARK_GRID; i++)
        fb_fill_px(x, y + SP.h * i / ZD_SPARK_GRID, w, 1, (unsigned)ZD_SURF_5);
}

void ui_spark_point(int pct)
{
    if (SP.n >= ZD_SPARK_MAX) return;
    SP.v[SP.n++] = (short)clamp(pct, 0, 100);
}

static int spark_y(int i)
{
    return SP.y + SP.h - SP.v[i] * SP.h / 100;
}

static int spark_x(int i)
{
    return SP.n > 1 ? SP.x + i * (SP.w - 1) / (SP.n - 1) : SP.x;
}

void ui_spark_end(void)
{
    if (ui_mode_get() != UI_DRAW || SP.n <= 0) return;
    /* the area fill first, as a column per sample - there is no polygon
     * rasteriser at this layer and a column fill is exact for a function of x */
    for (int i = 0; i < SP.n; i++) {
        int x0 = spark_x(i);
        int x1 = (i + 1 < SP.n) ? spark_x(i + 1) : x0 + 1;
        int y0 = spark_y(i);
        int y1 = (i + 1 < SP.n) ? spark_y(i + 1) : y0;
        for (int px = x0; px < x1; px++) {
            int yy = (x1 > x0) ? y0 + (y1 - y0) * (px - x0) / (x1 - x0) : y0;
            fb_fill_blend(px, yy, 1, SP.y + SP.h - yy,
                          ui_color(UI_COLOR_ACCENT), pct255(ZD_SPARK_AREA_A));
        }
    }
    for (int i = 0; i + 1 < SP.n; i++)
        fb_line(spark_x(i), spark_y(i), spark_x(i + 1), spark_y(i + 1),
                ui_color(UI_COLOR_ACCENT));
    SP.n = 0;
}

/* ============================================================================
 * INDICATORS AND INPUTS - S18
 * ========================================================================= */
int ui_dot_size(void) { return imax(2, DP(ZD_DOT)); }

void ui_dot(int x, int y, unsigned rgb, int glow)
{
    int d = ui_dot_size();
    if (ui_mode_get() != UI_DRAW) return;
    if (glow) {
        /* box-shadow 0 0 7px <col>, approximated as two soft rings - there is
         * no gaussian at this layer and a hard halo reads worse than none */
        int g = DP(3);
        fb_rrect_blend(x - g, y - g, d + 2 * g, d + 2 * g, (d + 2 * g) / 2,
                       rgb, pct255(12));
        fb_rrect_blend(x - g / 2, y - g / 2, d + g, d + g, (d + g) / 2,
                       rgb, pct255(22));
    }
    fb_rrect(x, y, d, d, d / 2, rgb);
}

int ui_badge_h(void) { return 2 * DP(ZD_BADGE_PY) + ui_text_h(UI_SM); }

int ui_badge_w(const char *s)
{
    return 2 * DP(ZD_BADGE_PX) + ui_text_w(s, UI_SM, UI_F_MONO);
}

void ui_badge(int x, int y, const char *s, unsigned rgb)
{
    int w = ui_badge_w(s), h = ui_badge_h();
    if (ui_mode_get() != UI_DRAW) return;
    /* the badge is ALWAYS a tint of its own semantic colour, never a fill:
     * a solid BAD badge beside a solid BAD status dot reads as two errors */
    fb_rrect_blend(x, y, w, h, DP(ZD_BADGE_R), rgb, pct255(ZD_BADGE_TINT_A));
    ui_text(x + DP(ZD_BADGE_PX), text_cy(y, h, UI_SM, UI_F_MONO), s, rgb,
            UI_SM, UI_F_MONO);
}

int ui_input_h(void)  { return DP(ZD_INPUT_H); }
int ui_search_h(void) { return DP(ZD_SEARCH_H); }

/* The focus treatment is NEW DESIGN and is marked as such. S18.3 and S21.2:
 * "There is no focus ring anywhere. No input declares :focus, no box-shadow on
 * focus, no border change." zlOS is keyboard-navigable and an input that gives
 * no sign of holding the caret is unusable without a mouse - so the border
 * lifts to the accent, one property, no ring and no glow. */
static int input_body(int x, int y, int w, int h, int r, const char *text,
                      const char *placeholder, int focused)
{
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() != UI_DRAW) return fired;
    fb_rrect(x, y, w, h, r, (unsigned)ZD_SURF_1);
    fb_box(x, y, w, h, focused ? ui_color(UI_COLOR_ACCENT)
                               : (unsigned)ZD_SURF_5);
    const char *s = (text && text[0]) ? text : placeholder;
    unsigned ink = (text && text[0]) ? ui_color(UI_COLOR_TEXT)
                                     : (unsigned)ZD_TEXT_6;
    if (s) ui_text(x + DP(ZD_INPUT_PX), text_cy(y, h, UI_MD, 0), s, ink,
                   UI_MD, 0);
    return fired;
}

int ui_input(int x, int y, int w, const char *text, const char *placeholder,
             int focused)
{
    return input_body(x, y, w, ui_input_h(), DP(ZD_INPUT_R), text, placeholder,
                      focused);
}

int ui_search(int x, int y, int w, const char *text, const char *placeholder)
{
    return input_body(x, y, w, ui_search_h(), DP(ZD_SEARCH_R), text,
                      placeholder, 0);
}

int ui_chip_h(void) { return 2 * DP(ZD_CHIP_PY) + ui_text_h(UI_SM); }

int ui_chip_w(const char *s)
{
    return 2 * DP(ZD_CHIP_PX) + ui_text_w(s, UI_SM, 0);
}

int ui_chip(int x, int y, const char *s, int active)
{
    int w = ui_chip_w(s), h = ui_chip_h(), r = DP(ZD_CHIP_R);
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        unsigned ink;
        if (active) {
            fb_rrect(x, y, w, h, r, ui_color(UI_COLOR_ACCENT));
            ink = ui_ink_on(ui_color(UI_COLOR_ACCENT));
        } else {
            neutral_fill(x, y, w, h, r, ZD_FILL_SOFT_A);
            ink = ui_color(UI_COLOR_TEXT_DIM);
        }
        ui_text(x + DP(ZD_CHIP_PX), text_cy(y, h, UI_SM, 0), s, ink, UI_SM, 0);
        ui_ring(x, y, w, h);
    }
    return fired;
}
