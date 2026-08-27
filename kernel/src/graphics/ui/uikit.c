/* uikit.c - the shared widget catalogue the 53 apps are built out of.
 *
 * THIS FILE NOW DRESSES PRESSWORK. The design, its complete token ladder and
 * its geometry are docs/design/presswork-prototype.html in this worktree; the
 * tokens themselves are design.h and the semantic roles are ui.c's theme
 * struct. Where a comment below cites a CSS selector (.btn, .seg, tr.sel,
 * .mi, .toast) it names the prototype rule this widget is the port of, and
 * where the prototype and the older reference disagree THE PROTOTYPE WINS.
 *
 * THE FOUR THINGS PRESSWORK CHANGED HERE, and every function below is one of
 * them applied:
 *
 * 1. DEPTH IS A LIT/CUT EDGE PAIR, NOT A BORDER AND NOT A SHADOW. One raking
 *    light enters off-screen upper left and never moves. A raised control is
 *    therefore drawn as a 1px ZD_CUT ring (the groove the object sits in) with
 *    a 1px ZD_LIT run along the inside of its top edge (the side the light
 *    strikes) - `seat_raised()` below. A sunken one is the same recipe with
 *    the sign flipped: ZD_CUT along the inside top, ZD_LITSOFT along the
 *    inside bottom - `seat_sunken()`. Nothing in this file draws a 1px grey
 *    box any more, and nothing draws a drop shadow except the three objects
 *    that are genuinely off the plane (menu, modal, toast), which get
 *    `lift_shadow()`. Measured: ZD_LIT is 2.5423:1 on ZD_BASE and 2.0390:1 on
 *    ZD_RAISE; ZD_CUT is 1.4723:1 on ZD_BASE and 1.8357:1 on ZD_RAISE.
 *
 * 2. THE RADIUS IS A FOUR-VALUE SYSTEM AND IT ENCODES HOW MUCH THE OBJECT CAN
 *    MOVE. ZD_R_BOLT 0 is bolted to the screen (tab, segment item, meter,
 *    list row, menu item). ZD_R_PLATE 9 moves as a whole (a window, a modal).
 *    ZD_R_INSET 4 is one level inside a plate (card, menu body, toast).
 *    ZD_R_CHIP 2 is the smallest object (button, chip, badge, input). A chip
 *    and a window MUST NOT share a radius, which is exactly what the sixteen
 *    old ZD_R_* values let happen; they now alias onto these four.
 *
 * 3. THE OVERPRINT HAS FOUR JOBS AND A WIDTH RULE. ZD_VERM is an ink laid ON
 *    a surface, never the surface itself: the focus bar and register mark, the
 *    ONE primary action per view, the crop marks, the datum mark. It never
 *    carries text except in ZD_INK_ON (6.1400:1, computed). It never fills an
 *    area wider than ZD_FOCUS_BAR except that one primary button. So every
 *    "this one is selected" fill in this file that used to be an accent tint
 *    is now THE KNOCKOUT - ZD_KNOCK with ZD_KNOCK_INK reversed out of it,
 *    6.4796:1 on ZD_BASE and 8.5329:1 for the ink on the plate - and every
 *    instrument reading is ZD_STEEL, the machine's own ink, which is the
 *    other half of the old accent's job.
 *
 * 4. ZD_TEXT_INERT IS STRUCTURE AND NEVER A GLYPH. It is 2.0222:1 on ZD_RAISE
 *    and 1.7310:1 on ZD_FLOAT - it cannot carry a letter anywhere on this
 *    ladder. The label rung is ZD_TEXT_3 (5.3585:1 on ZD_RAISE, 4.5869:1 on
 *    ZD_FLOAT) and that is the dimmest ink this file will draw.
 *
 * LIGHT MODE IS DELIBERATELY OUT OF SCOPE. The prototype carries a second
 * ladder; its own verifier reports that on paper the struck run computes
 * 1.244:1 on the ground, so the widening this whole direction depends on does
 * not exist there. Every colour decision below is the dark ladder's, and
 * nothing here branches on a mode.
 *
 * PRESERVED, EXPLICITLY: subpixel LCD text, gamma-correct blending in linear
 * light, dithered gradients, anti-aliased rounded corners and the geometric
 * icons all still work exactly as they did. fb.c is not edited - this file
 * only changes which primitives it calls and with which tokens.
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
 * THE SPECIFICATION IS kernel/docs/reference/ui/widgets.md - 68 widget records
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
 * 3. FIVE STATES, EVERY CONTROL. The old reference had five hover rules in
 *    4338 lines and no :focus anywhere, and this file inherited that: pills
 *    drew one face and `pill_face`'s `on` argument was passed 0 from its only
 *    caller. PRESSWORK ships default / hover / focus / active / disabled on
 *    every control, resolved in ONE place - `ctl_state()` - so the five never
 *    drift between widgets. The states are expressed in the design's own
 *    physical grammar rather than in five invented colours:
 *
 *      default   the seat: ZD_RAISE face, ZD_CUT ring, ZD_LIT top run
 *      hover     the ring lifts to ZD_EDGE_OVER and the ink to ZD_TEXT_0
 *      focus     ui_ring(), the overprint - see the note on it below
 *      active    the seat inverts: ZD_WELL face, ZD_CUT top run. The light
 *                cannot strike a face that has been pushed below the plate,
 *                so the struck run is the thing that goes away.
 *      disabled  the face stays but the LIT RUN IS NOT DRAWN. An unlit plate
 *                is not raised, and a thing that is not raised cannot be
 *                pressed - the state is carried by the lamp, not by a wash.
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

/* THE PIXEL-SIZED FORM, and it is why this file no longer asks for a ROLE.
 * fb_text_role takes one of three roles and resolves them through fb.c's
 * `role_base[3] = {8,12,16}` scaled by the UI scale and then FLOORED AT 12 -
 * so on the 1920x1200 panel this design is drawn for, CAPTION and BODY are
 * both 12px and the whole scale is two steps, not three. Measured, not
 * assumed: fb_text_role_h() returns 12 / 12 / 16 at ui_theme_init_q8(256),
 * and fb_text_role_w() gives the SAME width for the same string at CAPTION
 * and at BODY.
 *
 * fb_text_rich is the same engine with the role resolution removed - it takes
 * a height in pixels and resamples the nearest baked atlas to reach it, which
 * is the path the browser's six heading sizes already take. So the three
 * sizes PRESSWORK actually specifies (ZD_T_SM/MD/LG, 11/13/21 design px) are
 * reachable without editing fb.c and without a fourth atlas. */
int  fb_text_rich_w(const char *s, int len, int size, int style);
void fb_text_rich(int px, int py, const char *s, int len, unsigned int fg,
                  int size, int style);
/* fb.c exports no header, so its style bits are mirrored here. Only the bold
 * bit is used: the oblique is synthesised and PRESSWORK has no italic, and the
 * mono path in this file goes through fb_text_aa rather than FBT_MONO. */
#define FBT_BOLD 1

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
 * THREE SIZES, AND UNTIL THIS CHANGE THERE WERE TWO. design.h states the scale
 * as SM 11 / MD 13 / LG 21 design px and calls it "a hard platform fact, not a
 * design preference" that there is no fourth. What it does not say - and what
 * the port had quietly lost - is that asking fb.c for a ROLE does not get you
 * three of them either. Measured at ui_theme_init_q8(256), which is the
 * 1920x1200 panel every render in docs/design/ is taken on:
 *
 *     fb_text_role_h(CAPTION) = 12   fb_text_role_w(S,CAPTION,BOLD) = 318
 *     fb_text_role_h(BODY)    = 12   fb_text_role_w(S,BODY,BOLD)    = 318
 *     fb_text_role_h(TITLE)   = 16   fb_text_role_w(S,TITLE,BOLD)   = 452
 *
 * CAPTION and BODY are the SAME SIZE, byte for byte, because fb.c derives a
 * role height from `role_base[] = {8,12,16}` times the UI scale and then
 * floors the result at 12 - and at scale 1.0 the caption's 8 is below the
 * floor. fb.c's own comment says so and calls the collapse deliberate: three
 * distinct sizes inside 16 pixels "is not a type scale, it is three illegible
 * sizes". That reasoning is about the CONSOLE CELL. It is not about this
 * design, whose SM is 11 and whose LG is 21 - a 1.91x range that the role
 * ladder renders as 1.33x.
 *
 * So the roles are not asked for any more. fb_text_rich takes a height in
 * PIXELS and resamples the nearest baked atlas to reach it, and UI_SM/MD/LG
 * resolve through design.h's own ZD_T_* tokens instead. After the change, same
 * conditions, same string:
 *
 *     ui_text_h(UI_SM) = 11   ui_text_w(S,UI_SM,UI_F_BOLD) = 283
 *     ui_text_h(UI_MD) = 13   ui_text_w(S,UI_MD,UI_F_BOLD) = 350
 *     ui_text_h(UI_LG) = 21   ui_text_w(S,UI_LG,UI_F_BOLD) = 542
 *
 * WHAT THIS COSTS, stated rather than hidden. 11px is BELOW the floor fb.c
 * put there, and it is reached by downsampling the 16px atlas, so SM is
 * marginally softer than the 12px it used to be drawn at. That is the right
 * trade for the same reason fb.c's own rich-text note gives for h1: a label
 * that is the WRONG SIZE reads as a bug, a label half a pixel soft reads as a
 * label. It was checked by eye at 2x before it was written, not argued from.
 *
 * ONE WEIGHT, AND THAT ONE WAS ALREADY RIGHT. The prototype declares
 * font-weight:700 ten times and never declares any other value, so the design
 * has exactly two states - inherited and 700 - and fb.c bakes exactly two
 * atlases per size, regular and a DRAWN bold. Nothing to change; recorded here
 * because "one weight" is easy to misread as "delete the bold".
 *
 * MONO IS ONE SIZE. fb.c's own note: "FBT_MONO routes to the fixed-cell AA
 * font, which is ONE size ... honoured for proportional text and ignored for
 * mono, consistently between the measure and the draw". Kept consistent here
 * for the same reason: a measure and a draw that disagree is a clipped label. */

/* ZD_T_* are stored x2 so a half-pixel size stays an integer. The scale is
 * applied to the DOUBLED value and halved afterwards, which is a rung more
 * accurate at the odd scales (q8 = 341, 427) than halving first would be. */
static const short type_x2[3] = { ZD_T_SM, ZD_T_MD, ZD_T_LG };

int ui_text_h(int size)
{
    int px = UI_DP(T, type_x2[clamp(size, UI_SM, UI_LG)]) / 2;
    /* fb.c refuses to draw below 12 through the role path and this one has no
     * floor at all, so the floor is here. 8px is where the 16px atlas stops
     * resampling into something with countable strokes; SM never reaches it
     * (11 at scale 1.0) and this only catches a scale nobody has shipped. */
    return px < 8 ? 8 : px;
}

int ui_text_w(const char *s, int size, int flags)
{
    if (!s) return 0;
    if (flags & UI_F_MONO) return ui_strlen(s) * fb_cell_w();
    return fb_text_rich_w(s, ui_strlen(s), ui_text_h(size),
                          (flags & UI_F_BOLD) ? FBT_BOLD : 0);
}

void ui_text(int x, int y, const char *s, unsigned rgb, int size, int flags)
{
    if (!s || ui_mode_get() != UI_DRAW) return;
    if (flags & UI_F_MONO) { fb_text_aa(x, y, s, rgb); return; }
    fb_text_rich(x, y, s, ui_strlen(s), rgb, ui_text_h(size),
                 (flags & UI_F_BOLD) ? FBT_BOLD : 0);
}

/* Vertically centre a line of this size inside a box of height h. */
static int text_cy(int y, int h, int size, int flags)
{
    int th = (flags & UI_F_MONO) ? fb_cell_h() : ui_text_h(size);
    return y + (h - th) / 2;
}

/* ---- THE LABEL STYLE, AND IT IS NOW DRAWN RATHER THAN DEFERRED -------------
 *
 *     th, .t-lab { font-size: var(--fs-sm); line-height: var(--lh-sm);
 *                  text-transform: uppercase; font-weight: 700;
 *                  letter-spacing: var(--tr-lab);
 *                  margin-right: calc(-1 * var(--tr-lab)); }
 *
 * PRESSWORK's fourth type style is not a fourth size - there are three baked
 * atlases and no rasteriser, so a fourth size does not exist to be asked for.
 * It is SM, uppercase, bold and TRACKED, and design.h has carried ZD_TR_LAB
 * (1.4 design px) since PRESSWORK landed with nothing in the tree reading it.
 * Two comments in this file - ui_colhead's and ui_heading's - recorded the
 * tracking as unavailable and the prototype's own note calls it "the one
 * unpriced thing in this file".
 *
 * It was never unavailable. What is unavailable is letter-spacing INSIDE
 * fb_text_role, which draws a whole string in one call; tracking a string is
 * one call per glyph with the pen advanced by the glyph's own width plus the
 * track, and that is this layer's job, not fb.c's. fb.c is not edited.
 *
 * THE MEASURE AND THE DRAW SHARE THE ADVANCE, which is the requirement
 * design.h states in the same breath as the token: a label measured without
 * the track and drawn with it is a label that clips at its right edge. Both
 * walk the same loop below, so they cannot disagree.
 *
 * THE TRAILING TRACK IS CANCELLED, and that is the prototype's rule rather
 * than a rounding choice - `margin-right: calc(-1 * var(--tr-lab))` on the
 * same selector. Letter-spacing in CSS is applied AFTER every glyph including
 * the last, which pushes a right-flushed label off its own edge by the track;
 * the negative margin takes it back. So the width here is the sum of the
 * glyphs plus (n-1) tracks, never n.
 *
 * UPPERCASING IS THE WIDGET'S, NOT THE CALLER'S. `text-transform` is in the
 * stylesheet in the prototype, so a caller passing "Name" gets "NAME" for the
 * same reason it gets bold: it asked for a column head, not for a string it
 * styled itself. The transform is ASCII-only, which is the whole of what the
 * three atlases carry.
 *
 * THEY ARE NO LONGER FILE-LOCAL, and that was the whole of what was left.
 * When this landed the pair sat static at the top of this file, so exactly
 * three widgets could draw the style - ui_colhead, ui_heading and ui_kv - and
 * NOTHING ELSE IN THE SYSTEM COULD. The shell is written in zl and every
 * tracked-caps run in it (the register rail's section heads, RASTER, ADVANCE,
 * IDLE WAKEUPS/H, BUDGET, MEMORY, WORKSPACE, the tray's NET/VOL/PWR) went
 * through the untracked `label` builtin, which is why the shipped desktop
 * reads tight against the prototype while a card's column heads do not. So
 * ui_text_tracked / ui_text_tracked_w are published in ui.h and registered as
 * zl builtins, and the three widgets below now go through the same pair the
 * shell does. There is one tracked-text implementation in the tree.
 *
 * THE UPPERCASING MOVED IN HERE WITH A FLAG rather than staying a loop the
 * caller writes. `text-transform: uppercase` sits on the SAME selector as
 * `letter-spacing` in the prototype, so a caller that gets one and not the
 * other has half a style; UI_F_CAPS makes them arrive together. It also means
 * no buffer - the transform is per glyph inside the loop, so a 200-character
 * column head is not truncated into a 48-byte scratch on its way to the pen.
 *
 * THE TRACK IS A PARAMETER because the design has TWO of them: ZD_TR_LAB 1.4
 * for the label style and ZD_TR_BIG 2.6 for the one large reading per view.
 * ZD_TR_BIG had no reader anywhere in the tree before this - `grep -rn
 * ZD_TR_BIG src/` returned only its definition - so the display style was
 * specified and never drawn. ui_caps() and ui_display() below bake in one
 * token each so the numbers stay in design.h.
 *
 * A track of 0 IS A LEGAL ARGUMENT and means untracked, which is what makes
 * this pair the single text engine the zl side calls for everything: one loop,
 * one advance, one place a measure and a draw can be made to agree. */
static char lab_upper(char c)
{
    return (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
}

/* A tracking token (design px x10) in device px. Floored at 1 whenever the
 * design asked for any track at all, so the style survives a scale where
 * DP(1.4) rounds to 0 - a label with no tracking is still a label, but one
 * whose measure and draw round differently is a clip. A token of 0 means the
 * caller asked for no tracking and gets none. */
static int track_dev(int track_x10)
{
    int tr;
    if (track_x10 <= 0) return 0;
    tr = UI_DP(T, track_x10) / 10;
    return tr < 1 ? 1 : tr;
}

/* ONE PEN ADVANCE, and the measure and the draw both come through here. This
 * is the requirement design.h states in the same breath as the token: a label
 * measured without the track and drawn with it clips at its right edge. They
 * cannot disagree, because there is only one loop - `draw` is what varies.
 *
 * THE TRAILING TRACK IS CANCELLED, and that is the prototype's own rule rather
 * than a rounding choice: `margin-right: calc(-1 * var(--tr-lab))` sits on the
 * same selector. CSS applies letter-spacing after EVERY glyph including the
 * last, which pushes a right-flushed label off its own edge by one track; the
 * negative margin takes it back. So the width is the glyphs plus (n-1) tracks,
 * never n. */
static int text_run(int x, int y, const char *s, unsigned rgb,
                    int size, int flags, int track_x10, int draw)
{
    char g[2] = { 0, 0 };
    int tr = track_dev(track_x10), gflags = flags & ~UI_F_CAPS;
    int w = 0, n = 0;
    if (!s) return 0;
    for (int i = 0; s[i]; i++) {
        int gw;
        g[0] = (flags & UI_F_CAPS) ? lab_upper(s[i]) : s[i];
        gw = ui_text_w(g, size, gflags);
        if (draw) fb_text_rich(x + w, y, g, 1, rgb, ui_text_h(size),
                               (gflags & UI_F_BOLD) ? FBT_BOLD : 0);
        w += gw + tr;
        n++;
    }
    return n ? w - tr : 0;
}

int ui_text_tracked_w(const char *s, int size, int flags, int track_x10)
{
    /* the mono path has a fixed cell and fb.c ignores a size for it, so a
     * track would be the only thing making a mono run stop lining up with the
     * column beside it. Refused rather than silently applied. */
    if (flags & UI_F_MONO) return ui_text_w(s, size, flags);
    return text_run(0, 0, s, 0, size, flags, track_x10, 0);
}

void ui_text_tracked(int x, int y, const char *s, unsigned rgb,
                     int size, int flags, int track_x10)
{
    /* UNGATED, deliberately. ui_text() checks ui_mode_get() because ui.c's
     * cursor runs the same widget code twice, once to hit-test and once to
     * draw, and a hit-test pass must record no ink. This pair is a PRIMITIVE
     * and the shell calls it from zl, outside any cursor pass - where L.mode
     * still holds whatever the last ui_begin() set, which after a settings
     * hit-test is UI_HITTEST. A gate here would make the whole desktop's text
     * vanish depending on what the user last clicked. The widgets below gate
     * themselves before calling in; see lab_text(). */
    if (flags & UI_F_MONO) { ui_text(x, y, s, rgb, size, flags); return; }
    (void)text_run(x, y, s, rgb, size, flags, track_x10, 1);
}

/* The two named styles, with design.h's tokens baked in so a call site never
 * spells a track. `.t-lab`/`th`/`.sect`/`.kv .k` are ui_caps; `.t-big` is
 * ui_display. */
int  ui_caps_w(const char *s, int size)
{ return ui_text_tracked_w(s, size, UI_F_BOLD | UI_F_CAPS, ZD_TR_LAB); }

void ui_caps(int x, int y, const char *s, unsigned rgb, int size)
{ ui_text_tracked(x, y, s, rgb, size, UI_F_BOLD | UI_F_CAPS, ZD_TR_LAB); }

int  ui_display_w(const char *s, int size)
{ return ui_text_tracked_w(s, size, UI_F_BOLD, ZD_TR_BIG); }

void ui_display(int x, int y, const char *s, unsigned rgb, int size)
{ ui_text_tracked(x, y, s, rgb, size, UI_F_BOLD, ZD_TR_BIG); }

/* the widget-side pair: the LABEL style at SM, gated for the hit-test pass */
static int lab_w(const char *s) { return ui_caps_w(s, UI_SM); }

static void lab_text(int x, int y, const char *s, unsigned rgb)
{
    if (ui_mode_get() != UI_DRAW) return;
    ui_caps(x, y, s, rgb, UI_SM);
}

/* ---- THE SEAT, and it replaces the neutral fill ----------------------------
 * What was here was rgba(255,255,255,.07) - white at an alpha rather than a
 * surface step, so the same pill took the colour of whatever it was drawn on.
 * That is a coherent idea and it is not PRESSWORK's. On a ladder whose whole
 * point is that the rungs are far enough apart to be seen, a 7% white wash
 * lands between two rungs and reads as neither; and it cannot express the one
 * thing the design actually says, which is that a raised object is LIT ALONG
 * ITS TOP because there is a lamp.
 *
 * So: the prototype's `.btn`, which is the shape everything raised in this
 * file now uses -
 *
 *     background: var(--zd-raise);
 *     border: 1px solid var(--zd-cut);
 *     box-shadow: inset 0 1px 0 0 var(--zd-lit);
 *
 * three declarations, drawn here as a ring, an object and a run. The ring is
 * OUTSIDE and the run is INSIDE it, which is why the run starts at y+1: a
 * CSS inset box-shadow sits inside the border box, and putting the run on top
 * of the groove instead would delete the groove along the whole top edge and
 * with it the object's boundary against a lighter ground.
 *
 * The run stops at the corner arcs (`inset` below). fb_rrect anti-aliases the
 * arcs; a 1px run carried across one would land on partially-covered pixels
 * and fringe, which is the same reason the window recipe in wm.c runs its top
 * only between x+r and x+w-r. */
/* THE DEPTH RECIPE NOW LIVES IN ui.c AND IS PUBLISHED IN ui.h. It used to be
 * five statics here, which meant ui.c's own widgets - the button, the switch,
 * the slider, the bar, the list row - could not reach it and stayed on the
 * predecessor's rounded-capsule idiom. Two copies of "which edge the light
 * strikes" is two chances for the lamp to move.
 *
 * These five forwarders exist so the ~40 call sites below did not have to
 * change names to gain a shared definition. They are the whole of the
 * indirection; the derivation, and the reason ZD_R_BOLT takes a different code
 * path from a rounded seat, are in ui.c beside the definitions. */
static void seat_face(int x, int y, int w, int h, int r, unsigned face, unsigned ring)
{ ui_seat_face(x, y, w, h, r, face, ring); }

static void run_top(int x, int y, int w, int r, unsigned edge)
{ ui_run_top(x, y, w, r, edge); }

static void run_bottom(int x, int y, int w, int h, int r, unsigned edge)
{ ui_run_bottom(x, y, w, h, r, edge); }

static void seat_raised(int x, int y, int w, int h, int r, unsigned face,
                        unsigned ring, int lit)
{ ui_seat_raised(x, y, w, h, r, face, ring, lit); }

static void seat_sunken(int x, int y, int w, int h, int r, unsigned face)
{ ui_seat_sunken(x, y, w, h, r, face); }

/* ---- THE ONLY SHADOW IN THE SYSTEM -----------------------------------------
 * ZD_LIFT, and it is drawn under exactly three things - the menu, the modal
 * sheet and the toast - because those three are genuinely off the plane. A
 * plate sitting still is separated by the ladder and the two runs alone, and
 * a shadow at rest is the single fastest way to make this design look like
 * every other one.
 *
 * There is no gaussian at this layer and there is no heap to build one in, so
 * the blur is a small fixed number of concentric blended rounded rectangles
 * with the alpha falling off as the SQUARE of the distance out. The square is
 * what makes it read as a blur rather than as a stack of borders: a linear
 * ramp puts equal ink in every band and the outermost band is the widest, so
 * linear reads as a halo. ZD_LIFT_A is the alpha at the object's own edge,
 * ZD_LIFT_BLUR is how far it reaches, ZD_LIFT_DY is how far the lamp pushes
 * it down.
 *
 * BANDS, NOT PIXELS, AND THE REASON IS THE MACHINE. One band per pixel of
 * blur would be ZD_LIFT_BLUR full-area blends per overlay per frame - 26 of
 * them at ui scale 2 - on a CPU that draws every pixel itself with no GPU
 * behind it. LIFT_BANDS is fixed, so the cost of a menu's shadow does not
 * change when the user changes the UI scale; only the spacing of the bands
 * does. Eight is where the banding stops being visible against a 55% alpha at
 * this radius, and it is the whole knob if it ever needs to move. */
#define LIFT_BANDS 8

static void lift_shadow(int x, int y, int w, int h, int r)
{
    int blur = DP(ZD_LIFT_BLUR), dy = DP(ZD_LIFT_DY);
    if (ui_mode_get() != UI_DRAW || blur < 1) return;
    for (int b = LIFT_BANDS; b >= 1; b--) {
        int i = blur * b / LIFT_BANDS;          /* how far out this band is */
        int a = ZD_LIFT_A * (LIFT_BANDS - b + 1) * (LIFT_BANDS - b + 1) /
                (LIFT_BANDS * LIFT_BANDS) / 2;
        if (i < 1 || a < 1) continue;
        fb_rrect_blend(x - i, y - i + dy, w + 2 * i, h + 2 * i, r + i,
                       (unsigned)ZD_LIFT, a);
    }
}

/* ---- THE FIVE STATES, RESOLVED ONCE ----------------------------------------
 * Every control in this file asks this and nothing else, so "what is a hover"
 * cannot drift between a pill and a chip. It is the same argument as ui.c's
 * fire() funnel one layer up.
 *
 * PRESS IS A CLICK FRAME, NOT A HELD BUTTON. ui.c's funnel gives this file a
 * pointer position and a one-frame `click`; there is no button-down state to
 * read, because ui.c deliberately does not expose one (a widget that could
 * tell a click from an Enter would drift from the keyboard path). So a press
 * renders on the frame the click lands. That is honest about what the layer
 * knows and it is enough to see, since the same frame also fires the action.
 *
 * UI_F_DISABLED IS DECLARED HERE UNDER A GUARD and it belongs in ui.h. Nothing
 * in the tree passes it yet - it is a new capability, not a port of an old one
 * - and ui.h is not this file's to edit in this change. The guard means the
 * moment ui.h declares it, whatever bit ui.h picks is the bit this file uses. */
#ifndef UI_F_DISABLED
#define UI_F_DISABLED (1 << 2)
#endif

#define UI_ST_OFF   0
#define UI_ST_HOVER 1
#define UI_ST_PRESS 2
#define UI_ST_ON    3
#define UI_ST_DIS   4

static int ctl_state(int x, int y, int w, int h, int on, int flags)
{
    if (flags & UI_F_DISABLED) return UI_ST_DIS;
    if (on) return UI_ST_ON;
    if (!ui_hit(x, y, w, h)) return UI_ST_OFF;
    return ui_click_get() ? UI_ST_PRESS : UI_ST_HOVER;
}

/* The ring a control draws around itself. ZD_CUT is the groove at rest; under
 * the pointer it lifts to ZD_EDGE_OVER, which is the design's own "this
 * object is in front" boundary at 4.9991:1 on ZD_BASE - the same token the
 * window uses when it lies over another, used here for the same reason. */
static unsigned state_ring(int st)
{
    return (st == UI_ST_HOVER) ? ui_color(UI_COLOR_EDGE_OVER)
                               : ui_color(UI_COLOR_CUT);
}

/* ============================================================================
 * BUTTONS - the prototype's `.btn`
 *
 * Three sizes, one shape, and the shape is now PRESSWORK's:
 *
 *     .btn { height: 22dp; padding: 0 10dp;
 *            background: var(--zd-raise); border: 1px solid var(--zd-cut);
 *            box-shadow: inset 0 1px 0 0 var(--zd-lit);
 *            border-radius: var(--zd-r-chip); color: var(--zd-text-2); }
 *     .btn:hover  { color: var(--zd-text-0); border-color: var(--zd-edge-over); }
 *     .btn:active { background: var(--zd-well);
 *                   box-shadow: inset 0 1px 0 0 var(--zd-cut); }
 *
 * THE RADIUS IS ZD_R_CHIP AND THAT IS THE POINT OF THE RADIUS SYSTEM. A button
 * is the smallest object on screen, so it takes the smallest radius, 2dp. The
 * window it sits in takes ZD_R_PLATE, 9dp. Under the old scale both of them
 * were an 11-to-13px pill and the two objects claimed the same amount of
 * mobility, which is the specific thing PRESSWORK's four-value radius system
 * exists to stop. design.h now points ZD_PILL_*_R at ZD_R_CHIP for all three
 * sizes; the sizes differ in padding only, which is what a size is.
 *
 * The three paddings stay the reference's - the collapse from twenty-six
 * near-identical pills to three is unchanged and still costs 1-4px against
 * the original in places. That was accepted before PRESSWORK and PRESSWORK
 * does not revisit it.
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

/* The face and the ink of one button kind in one state, together.
 *
 * `st` is one of the five UI_ST_*. The three kinds answer it differently and
 * the differences are the design, not decoration:
 *
 * NEUTRAL is the seat. Hover lifts the ring, press inverts the seat, disabled
 * puts the lamp out. Ink runs ZD_TEXT_2 (6.3046:1 on ZD_RAISE) at rest and
 * ZD_TEXT_0 (8.9894:1) under the pointer.
 *
 * PRIMARY IS THE OVERPRINT AND THERE IS EXACTLY ONE PER VIEW. `.btn.prim` is a
 * vermilion fill with a ZD_VERM_BR struck run and ZD_INK_ON text; it is the
 * ONLY place in this file where vermilion fills an area wider than
 * ZD_FOCUS_BAR, and that exemption is written into the prototype's own width
 * rule. The rule is not machine-enforced here - the prototype says the same of
 * itself, "enforced by hand, which is a real weakness of a prototype and would
 * be a lint in the kernel". It still would.
 *
 * THE OLD S13.2 BUG STAYS UNREPRODUCED. Three reference variants hard-coded
 * '#fff' on the accent where the computed ink is dark. PRIMARY routes through
 * ui_ink_on(), which is a luminance compare, not a stored answer. On ZD_VERM
 * it computes ZD_INK_DARK - and ZD_INK_DARK is ZD_INK_ON, at 6.1400:1, which
 * is the figure the prototype states for that pair. Verified rather than
 * assumed: ui_luminance_q16(0xE8734F) is 19701 against the 12242 threshold.
 *
 * DANGER IS A RULE AND AN INK, NOT A WASH. What was here was ZD_BAD at 16%
 * with a 40% border - a translucent red area, which is exactly the kind of
 * tinted region PRESSWORK removed everywhere else (a wash cannot sit on a
 * ladder whose rungs are the information). It is now the ordinary seat with a
 * ZD_BAD ring and ZD_BAD_INK text, 4.7396:1 on ZD_RAISE. The state is carried
 * by the boundary and the ink, which is how this design says everything else,
 * and it leaves the vermilion budget for the one action per view. */
static void pill_face(int x, int y, int w, int h, int r, int kind, int st,
                      unsigned *ink)
{
    int lit = (st != UI_ST_DIS);
    unsigned ring = state_ring(st);

    switch (kind) {
    case UI_BTN_PRIMARY:
        if (st == UI_ST_DIS) {
            seat_raised(x, y, w, h, r, ui_color(UI_COLOR_PANEL_HI), ring, 0);
            *ink = ui_color(UI_COLOR_TEXT_DIM);
            break;
        }
        {
            unsigned face = (st == UI_ST_HOVER || st == UI_ST_ON)
                                ? ui_color(UI_COLOR_ACCENT_BR)
                                : ui_color(UI_COLOR_ACCENT);
            seat_face(x, y, w, h, r, face, face);
            /* the struck run on the overprint is the overprint at full
             * strength - the same lamp, one rung up the ink rather than up
             * the surface ladder, because vermilion has no surface rung */
            run_top(x, y, w, r, st == UI_ST_PRESS
                                    ? ui_color(UI_COLOR_ACCENT)
                                    : ui_color(UI_COLOR_ACCENT_BR));
            *ink = ui_ink_on(face);
        }
        break;

    case UI_BTN_DANGER:
        seat_raised(x, y, w, h, r,
                    st == UI_ST_PRESS ? ui_color(UI_COLOR_SURF_WELL)
                                      : ui_color(UI_COLOR_PANEL_HI),
                    st == UI_ST_DIS ? ring : ui_color(UI_COLOR_DANGER),
                    lit && st != UI_ST_PRESS);
        if (st == UI_ST_PRESS) run_top(x, y, w, r, ui_color(UI_COLOR_CUT));
        *ink = (st == UI_ST_DIS) ? ui_color(UI_COLOR_TEXT_DIM)
                                 : (unsigned)ZD_BAD_INK;
        break;

    case UI_BTN_GHOST:
        /* no seat at all until the pointer arrives - the prototype's window
         * controls (`.cbtn`) are the same idea: bare glyphs behind a rule,
         * and the face only exists while it is being pointed at */
        if (st == UI_ST_HOVER || st == UI_ST_ON)
            seat_raised(x, y, w, h, r, ui_color(UI_COLOR_PANEL_HI), ring, 1);
        else if (st == UI_ST_PRESS)
            seat_sunken(x, y, w, h, r, ui_color(UI_COLOR_SURF_WELL));
        *ink = (st == UI_ST_DIS)   ? ui_color(UI_COLOR_TEXT_DIM)
             : (st == UI_ST_OFF)   ? ui_color(UI_COLOR_TEXT_DIM)
                                   : ui_color(UI_COLOR_TEXT_HI);
        break;

    default:
        if (st == UI_ST_PRESS) {
            seat_sunken(x, y, w, h, r, ui_color(UI_COLOR_SURF_WELL));
            *ink = ui_color(UI_COLOR_TEXT_HI);
        } else if (st == UI_ST_ON) {
            /* SELECTED IS THE KNOCKOUT, not a second idea and not the accent.
             * The same value flip the focused window header uses, on one
             * control: 6.4796:1 of surface against ZD_BASE with the label
             * reversed out at 8.5329:1. */
            seat_face(x, y, w, h, r, ui_color(UI_COLOR_KNOCK), ring);
            run_bottom(x, y, w, h, r, ui_color(UI_COLOR_KO_EDGE));
            *ink = ui_color(UI_COLOR_KNOCK_INK);
        } else {
            seat_raised(x, y, w, h, r, ui_color(UI_COLOR_PANEL_HI), ring, lit);
            *ink = (st == UI_ST_HOVER) ? ui_color(UI_COLOR_TEXT_HI)
                 : (st == UI_ST_DIS)   ? ui_color(UI_COLOR_TEXT_DIM)
                                       : ui_color(UI_COLOR_TEXT_2);
        }
        break;
    }
}

int ui_pill(int x, int y, int w, int h, const char *s,
            int size, int kind, int flags)
{
    int py, px, r;
    pill_metrics(size, &py, &px, &r);
    int st = ctl_state(x, y, w, h, 0, flags);
    /* A DISABLED CONTROL DOES NOT ENTER THE FUNNEL AT ALL. It must not fire on
     * a click and it must not be reachable by Tab - and ui.c's fire() is what
     * counts focus stops, so skipping the call is what removes it from the
     * tab order. The widget index therefore shifts when a control's disabled
     * state changes, which is correct: a control that cannot be operated is
     * not a stop. */
    int fired = (st == UI_ST_DIS) ? 0 : ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        unsigned ink = ui_color(UI_COLOR_TEXT);
        pill_face(x, y, w, h, r, kind, st, &ink);
        int tw = ui_text_w(s, size, flags);
        ui_text(x + (w - tw) / 2, text_cy(y, h, size, flags), s, ink, size,
                flags & ~UI_F_DISABLED);
        if (st != UI_ST_DIS) ui_ring(x, y, w, h);
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

/* A SQUARE ICON BUTTON - the prototype's `.cbtn` idiom, generalised.
 *
 * `.cbtn` has NO FACE AT REST: a bare ZD_TEXT_3 glyph, and the pointer brings
 * a ZD_RAISE face with it. That is the whole control. The old version drew a
 * permanent 8% white wash behind every icon in every toolbar, which on this
 * ladder is a fifth surface value nobody chose.
 *
 * ACTIVE IS THE KNOCKOUT, NOT THE ACCENT, and this is the width rule doing
 * real work. A 22dp square filled vermilion is 484 square dp of overprint;
 * the budget for it is a 3dp bar. So an icon button that is toggled on takes
 * ZD_KNOCK with its glyph reversed out in ZD_KNOCK_INK - the same value flip
 * the focused header and the selected row use - and the vermilion stays
 * available for the one primary action in the view.
 *
 * The radius is ZD_R_CHIP for both sizes. It was ZD_R_16 / ZD_R_14, which
 * under PRESSWORK's four-value scale both alias to ZD_R_PLATE - a window's
 * radius on a 22dp square, which is the exact collision the scale exists to
 * prevent. `px` is still the box; it no longer picks the corner. */
int ui_icon_button(int x, int y, int px, const char *glyph, int active)
{
    int d = DP(px), r = DP(ZD_R_CHIP);
    int st = ctl_state(x, y, d, d, active, 0);
    int fired = ui_fire(x, y, d, d);
    if (ui_mode_get() == UI_DRAW) {
        unsigned ink = (unsigned)ZD_WINCTL_INK;
        switch (st) {
        case UI_ST_ON:
            seat_face(x, y, d, d, r, ui_color(UI_COLOR_KNOCK),
                      ui_color(UI_COLOR_CUT));
            run_bottom(x, y, d, d, r, ui_color(UI_COLOR_KO_EDGE));
            ink = ui_color(UI_COLOR_KNOCK_INK);
            break;
        case UI_ST_PRESS:
            seat_sunken(x, y, d, d, r, ui_color(UI_COLOR_SURF_WELL));
            ink = ui_color(UI_COLOR_TEXT_HI);
            break;
        case UI_ST_HOVER:
            seat_raised(x, y, d, d, r, ui_color(UI_COLOR_PANEL_HI),
                        ui_color(UI_COLOR_EDGE_OVER), 1);
            ink = ui_color(UI_COLOR_TEXT_HI);
            break;
        default:
            break;                       /* no face at rest, on purpose */
        }
        int tw = ui_text_w(glyph, UI_SM, 0);
        ui_text(x + (d - tw) / 2, text_cy(y, d, UI_SM, 0), glyph, ink, UI_SM, 0);
        ui_ring(x, y, d, d);
    }
    return fired;
}

/* ============================================================================
 * SEGMENTED CONTROL - the prototype's `.seg`
 *
 *     .seg          { border: 1px solid var(--zd-cut); }
 *     .seg > span   { height: 20dp; padding: 0 9dp; background: var(--zd-raise);
 *                     border-right: 1px solid var(--zd-cut); }
 *     .seg > span.on{ background: var(--zd-knock); }
 *     .seg > span.on .t-lab { color: var(--zd-knock-ink); }
 *
 * THREE THINGS CHANGED AND THEY ARE THE SAME THING. The old control was a
 * sunken track with 2px of padding, a 2px gap and a rounded accent-filled
 * item floating inside it - a pill in a groove. PRESSWORK's is a row of
 * PLATES butted against each other inside one ring, divided by the groove
 * itself. So: design.h sets ZD_SEG_PAD and ZD_SEG_GAP to 0, the items are
 * ZD_R_BOLT because they cannot move, and the divider between them is a 1px
 * ZD_CUT rule rather than a gap of exposed track. The active item is the
 * knockout, 6.4796:1 against the plate, not a vermilion fill - a five-item
 * segmented control filled with the overprint would spend the entire
 * overprint budget on a tab bar.
 *
 * The container keeps ZD_SEG_R (ZD_R_CHIP): the control as a whole is a small
 * object and takes the small object's corner. Its items are square, so the
 * two outermost items are clipped by the container's arc, which is what a row
 * of plates inside a rounded frame looks like.
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
        /* UI_F_BOLD, because that is what ui_segmented() draws - the LABEL
         * style. Measuring regular and drawing bold is how a control ends up
         * one glyph short of its own label at the widest item. */
        w += 2 * px + ui_text_w(buf, size, UI_F_BOLD);
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
        /* the container is one raised plate; the items are painted over it,
         * so an item that fails to cover its cell shows the plate rather than
         * a hole. The struck run is drawn LAST, after the items, so it lies
         * on top of the row the way a lit edge lies on top of what it lights. */
        seat_face(x, y, w, h, DP(ZD_SEG_R), ui_color(UI_COLOR_PANEL_HI),
                  ui_color(UI_COLOR_CUT));
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
        int iy = y + pad;
        int len = item_at(items, i, buf, sizeof buf);
        if (len < 0) break;
        if (ui_fire(ix, iy, iw, ih)) hitidx = i;
        if (ui_mode_get() != UI_DRAW) continue;
        unsigned ink;
        int st = ctl_state(ix, iy, iw, ih, i == sel, 0);
        if (st == UI_ST_ON) {
            fb_fill_px(ix, iy, iw, ih, ui_color(UI_COLOR_KNOCK));
            /* the knockout's own edge run, on the BOTTOM. On a light plate
             * the direction with headroom is downward - ZD_KO_EDGE is
             * 2.5487:1 below it against 1.2131:1 for anything above - so the
             * same lamp puts the run on the other side. */
            fb_fill_px(ix, iy + ih - 1, iw, 1, ui_color(UI_COLOR_KO_EDGE));
            ink = ui_color(UI_COLOR_KNOCK_INK);
        } else if (st == UI_ST_HOVER || st == UI_ST_PRESS) {
            /* NEW DESIGN, one rung: ZD_FLOAT over ZD_RAISE is 1.1682:1, the
             * smallest step on the ladder, which is the right size for "the
             * pointer is here" and the wrong size for "this is selected". */
            fb_fill_px(ix, iy, iw, ih, ui_color(UI_COLOR_SURF_5));
            ink = ui_color(UI_COLOR_TEXT_HI);
        } else {
            ink = ui_color(UI_COLOR_TEXT_DIM);
        }
        /* the divider IS the groove - `border-right: 1px solid var(--zd-cut)`
         * on every item but the last */
        if (i + 1 < n && iw > 1)
            fb_fill_px(ix + iw - 1, iy, 1, ih, ui_color(UI_COLOR_CUT));
        int tw = imin(ui_text_w(buf, size, 0), iw);
        ui_text(ix + (iw - tw) / 2, text_cy(iy, ih, size, UI_F_BOLD), buf, ink,
                size, UI_F_BOLD);
        ui_ring(ix, iy, iw, ih);
    }
    /* the struck run last, over the items */
    if (ui_mode_get() == UI_DRAW)
        run_top(x, y, w, DP(ZD_SEG_R), ui_color(UI_COLOR_LIT));
    return hitidx;
}

int ui_segmented_value(int x, int y, int w, int h, const char *items,
                       int sel, int size)
{
    int got = ui_segmented(x, y, w, h, items, sel, size);
    return got >= 0 ? got : sel;
}

/* ============================================================================
 * TAB STRIPS - two idioms, and PRESSWORK moves them apart rather than together
 *
 * CLOSEABLE (ui_tabstrip) is a row of plates, the `.seg` shape at document
 * scale: ZD_R_BOLT, butted, divided by the 1px groove, and the active one is
 * the knockout.
 *
 * THE "TRUE CONNECTED TAB" IS GONE AND ITS REASON WENT WITH IT. The old
 * version painted the active tab the colour of the body below it so the seam
 * disappeared, which is a good trick and it depends on the active tab being
 * the same value as the body. Under PRESSWORK the active tab is a light plate
 * at 6.4796:1 against the body; there is no seam to hide and hiding it would
 * mean throwing away the selection signal. The tab now sits ON the strip and
 * the strip sits on the plate, which is what the ladder is for.
 *
 * UNDERLINE (ui_utabs) keeps its rule: no background change at all, the rule
 * under the active tab is the whole affordance. What changes is which rule.
 * See the note there - it is the one place in this file where a vermilion
 * mark would have been defensible and is still not taken.
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
        /* the strip ground is the plate, and the 2px ZD_LIT rule under it is
         * PRESSWORK's REGION break - the same weight the desk uses under the
         * raster strip and over the foot. 1px ZD_CUT means "next row"; 2px
         * ZD_LIT means "next region", and a tab strip is a region boundary. */
        fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL));
        int rh = imax(1, DP(ZD_RULE_H));
        fb_fill_px(x, y + h - rh, w, rh, ui_color(UI_COLOR_LIT));
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
        /* the ✕ first: it sits at the tab's trailing edge, so a hit on it must
         * not also count as a hit on the tab - its result wins.
         *
         * BOTH ui_fire() CALLS RUN, EVERY FRAME, AND THAT IS THE FIX. This was
         * `if (ui_fire(x)) ... else if (ui_fire(body)) ...`, so on the one
         * frame a click landed on the ✕ the body's ui_fire() was never
         * reached. ui.c's funnel counts a focus stop per ui_fire() call, so
         * that frame numbered every widget after this tab one lower than the
         * frame before it: the focus ring jumped to its neighbour, and if
         * focus sat past the strip it moved for real. A widget's identity may
         * not depend on where the pointer was - that is exactly the class the
         * disabled-pill note further up calls out, where the index shift IS
         * intended and is therefore stated. Here it was neither. The `else`
         * now lives on the RESULTS, which is where the precedence belonged. */
        int xx = cx + tw - pr - xw;
        int hit_close = ui_fire(xx, ty, xw, th);
        int hit_body  = ui_fire(cx, ty, tw - pr - xw, th);
        if (hit_close)     tab_closed_idx = i;
        else if (hit_body) hitidx = i;

        if (ui_mode_get() == UI_DRAW) {
            int st = ctl_state(cx, ty, tw, th, i == sel, 0);
            unsigned ink, xink;
            if (st == UI_ST_ON) {
                fb_rrect(cx, ty, tw, th, r, ui_color(UI_COLOR_KNOCK));
                ink  = ui_color(UI_COLOR_KNOCK_INK);
                xink = ui_color(UI_COLOR_KNOCK_INK2);   /* 4.6965:1 on it */
            } else {
                if (st == UI_ST_HOVER || st == UI_ST_PRESS)
                    fb_rrect(cx, ty, tw, th, r, ui_color(UI_COLOR_PANEL_HI));
                ink  = (st == UI_ST_OFF) ? ui_color(UI_COLOR_TEXT_DIM)
                                         : ui_color(UI_COLOR_TEXT_HI);
                xink = ui_color(UI_COLOR_TEXT_DIM);
            }
            /* the groove between tabs, and it is the only divider - a tab
             * strip with gaps would show the region rule through them */
            if (i + 1 < n && tw > 1)
                fb_fill_px(cx + tw - 1, ty, 1, th, ui_color(UI_COLOR_CUT));
            int room = tw - pl - pr - xw;
            if (room > 0)
                ui_text(cx + pl, text_cy(ty, th, UI_MD, 0), buf, ink, UI_MD, 0);
            /* the close glyph never takes ZD_TEXT_INERT. It used to take
             * ZD_SURF_7, which is that token - 2.0222:1 on ZD_RAISE, a mark
             * nobody can see and, worse, a GLYPH in the one colour this
             * ladder reserves for structure. */
            ui_text(xx, text_cy(ty, th, UI_SM, 0), "x", xink, UI_SM, 0);
            /* the third fire target in this file that drew no focus mark.
             * The ring lands on the tab BODY, because the body's ui_fire() is
             * the later of the two and focus_ring() marks `L.index - 1`; the
             * ✕ is a stop of its own and keeps the ring it cannot draw here.
             * Marking the body is the useful half - it is the tab you are on,
             * and the ✕ is one Tab further along from it. */
            ui_ring(cx, ty, tw, th);
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
        fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL));
        fb_fill_px(x, y + h - 1, w, 1, ui_color(UI_COLOR_CUT));
    }
    if (n <= 0) return -1;

    int cx = x;
    for (int i = 0; i < n; i++) {
        int len = item_at(items, i, buf, sizeof buf);
        if (len < 0) break;
        int tw = 2 * px + ui_text_w(buf, UI_MD, UI_F_BOLD);
        if (cx + tw > x + w) break;
        if (ui_fire(cx, y, tw, h)) hitidx = i;
        if (ui_mode_get() == UI_DRAW) {
            int st = ctl_state(cx, y, tw, h, i == sel, 0);
            /* no background change at all - the rule under the active tab is
             * the whole affordance, exactly as before. */
            ui_text(cx + px, text_cy(y, h, UI_MD, UI_F_BOLD), buf,
                    st == UI_ST_ON    ? ui_color(UI_COLOR_TEXT_HI)
                  : st == UI_ST_OFF   ? ui_color(UI_COLOR_TEXT_DIM)
                                      : ui_color(UI_COLOR_TEXT_2),
                    UI_MD, UI_F_BOLD);
            /* THE UNDERLINE IS ZD_LIT, NOT THE OVERPRINT, AND THAT IS A
             * DELIBERATE REFUSAL. A 2dp rule is thinner than ZD_FOCUS_BAR, so
             * the overprint's width rule would allow it - but the overprint
             * has FOUR jobs and this is not one of them: the focus bar and
             * register mark, the one primary action, the crop marks, the datum
             * mark. A fifth job costs the other four their meaning, and there
             * is already a token that says "this is a rank boundary" at
             * exactly this weight: the 2px ZD_LIT rule the desk uses between
             * regions, 2.5423:1 on the plate. The strip's own bottom edge
             * stays 1px ZD_CUT, so the two never read as the same rule. */
            if (st == UI_ST_ON)
                fb_fill_px(cx, y + h - rule, tw, rule, ui_color(UI_COLOR_LIT));
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
 * TOOLBAR AND STATUS BAR - the desk's own band grammar, one level in
 *
 * The prototype has two of these at desktop scale and they agree with each
 * other: `#raster` is a band at the TOP with `border-bottom: 2px solid
 * var(--zd-lit)`, and `#foot` is a band at the BOTTOM with `border-top: 2px
 * solid var(--zd-lit)`. Both stand on ZD_RAISE. So a toolbar is chrome one
 * rung proud of the plate it lies on, and the rule that separates it from the
 * content is the 2px STRUCK rule, not a hairline - because a toolbar is a
 * region boundary and the design has a weight that means exactly that.
 *
 * A STATUS BAND IS NOT A TOOLBAR AND THE PROTOTYPE IS EXPLICIT. `.sband` has
 * NO BACKGROUND AT ALL - it is part of the plate - and `border-top: 1px solid
 * var(--zd-cut)`. It is a row break inside one region, not a region break, so
 * it takes the 1px groove and the plate's own value. Drawing it as a raised
 * band, which is what this used to do, claimed a rank it does not have and
 * made every window look like it had two toolbars.
 * ========================================================================= */
int ui_toolbar_h(void) { return DP(ZD_TOOLBAR_H); }
int ui_status_h(void)  { return DP(ZD_STATUS_H); }

void ui_toolbar(int x, int y, int w, int h, int at_bottom)
{
    int rh = imax(1, DP(ZD_RULE_H));
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL_HI));
    /* the struck rule is always on the side facing the content, because the
     * lamp is above and the rule is the lit edge of the band nearest it */
    if (at_bottom) fb_fill_px(x, y, w, rh, ui_color(UI_COLOR_LIT));
    else           fb_fill_px(x, y + h - rh, w, rh, ui_color(UI_COLOR_LIT));
}

void ui_statusbar(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL));
    fb_fill_px(x, y, w, 1, ui_color(UI_COLOR_CUT));
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
    /* the prototype's `th`: background var(--zd-base), colour var(--zd-text-3),
     * uppercase 700 tracked, and `border-bottom: 2px solid var(--zd-lit)`. The
     * 2px struck rule again - the head is a different RANK from the rows under
     * it, and the rows separate from each other with the 1px groove. */
    if (ui_mode_get() == UI_DRAW) {
        int rh = imax(1, DP(ZD_RULE_H));
        fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL));
        fb_fill_px(x, y + h - rh, w, rh, ui_color(UI_COLOR_LIT));
    }
    for (int i = 0; i < G.n; i++) {
        int cx, cw;
        ui_grid_span(x, w, i, &cx, &cw);
        if (cw <= 0) continue;
        if (item_at(labels, i, buf, sizeof buf) < 0) break;
        if (ui_fire(cx, y, cw, h)) hitidx = i;
        if (ui_mode_get() != UI_DRAW) continue;
        int st = ctl_state(cx, y, cw, h, i == sortcol, 0);
        unsigned ink = (st == UI_ST_ON)  ? ui_color(UI_COLOR_TEXT_HI)
                     : (st == UI_ST_OFF) ? ui_color(UI_COLOR_TEXT_DIM)
                                         : ui_color(UI_COLOR_TEXT_2);
        /* THE LABEL STYLE, ALL FOUR PARTS OF IT. `th` is SM, uppercase, bold
         * and tracked by ZD_TR_LAB, and this used to carry two of the four -
         * the note here said tracking was "NOT AVAILABLE AND IS NOT FAKED"
         * and uppercasing "belongs to the caller". Both are now drawn, by
         * lab_text() above, which advances the pen a glyph at a time; see the
         * derivation there for why that is this layer's job and not fb.c's.
         *
         * The padding is the prototype's `td`/`th` 6dp on BOTH sides. It was
         * DP(8) left and DP(10) right - two bare numbers in a file whose rule
         * 2 says the numbers live in design.h, and asymmetric where the
         * prototype's padding is `0 6px`, one value both ends.
         *
         * WHAT THAT ASYMMETRY DID NOT DO IS MISALIGN A COLUMN, and the first
         * draft of this comment said it did. ui_grid_cell() spelled the same
         * DP(8)/DP(10), so head and cell already agreed; measured on the
         * scratch render at ui scale 2, the SIZE head's right edge sits at
         * x=902 and its column's first value at x=901, a 1px glyph
         * side-bearing that is unchanged after this edit (910 and 909). The
         * change is a token where two literals were, and the prototype's
         * margin where an invented one was. It is not a bug fix and is not
         * written up as one. */
        int tw = lab_w(buf);
        int cp = DP(ZD_CELL_PX);
        int tx = (G.fixed[i] && i != G.star) ? cx + cw - tw - cp : cx + cp;
        if (tx < cx) tx = cx;
        lab_text(tx, text_cy(y, h, UI_SM, UI_F_BOLD), buf, ink);
        if (i == sortcol) {
            /* the sort arrow: icon('sortU'|'sortD', 8) in the reference, a
             * 4px triangle of hairlines here - there is no icon atlas in the
             * toolkit layer and one glyph is not worth a dependency on it */
            int a = DP(4), ax = tx + tw + DP(4), ay = y + h / 2 - DP(2);
            for (int k = 0; k < a; k++) {
                /* the DIRECTION is the width ramp, not the y - a triangle that
                 * narrows downward points down and one that widens points up */
                int ww = sortdir >= 0 ? a - k : k + 1;
                /* NOT the overprint. A sort arrow is not one of vermilion's
                 * four jobs, and one per table across 53 apps is a lot of
                 * overprint spent on a direction. It is the sorted column's
                 * own ink, which is where the information already is. */
                fb_fill_px(ax + (a - ww) / 2, ay + k, ww, 1,
                           ui_color(UI_COLOR_TEXT_HI));
            }
        }
        /* A COLUMN HEAD IS A TAB STOP AND HAD NO FOCUS MARK. It calls
         * ui_fire(), which is what counts a focus stop in ui.c's funnel, so
         * a keyboard walking a table already landed on every sortable head -
         * and drew nothing when it got there. That is the whole of rule 3's
         * `focus` state missing on this control, and the fix is the one call
         * every other fire target in this file already makes.
         *
         * It goes HERE, at the foot of the item's own iteration: focus_ring()
         * tests `L.index - 1`, so it marks the widget whose ui_fire() was the
         * most recent one. Hoisting it out of the loop would draw the ring on
         * the last column no matter which one had focus. */
        ui_ring(cx, y, cw, h);
    }
    return hitidx;
}

int ui_grid_row_h(void) { return DP(ZD_LISTROW_H); }

/* SELECTION IS THE KNOCKOUT, AND THIS NO LONGER CALLS ui_row_select().
 *
 * That needs saying plainly, because ui_row_select() lives in ui.c precisely
 * so that "what does a selected row look like" has one home. The prototype
 * has TWO row idioms and they are not the same decision:
 *
 *   tr.sel td  { background: var(--zd-knock); color: var(--zd-knock-ink); }
 *   .slot.act  { background: var(--zd-base); }  + a vermilion register mark
 *
 * The first is a TABLE row - a row of cells inside a plate, where selection is
 * the value flip. The second is the RAIL's register slot, where selection is
 * the overprint's job 1, the register mark, and the ground moves one rung.
 * ui_row_select() draws the second (an accent bar plus a 15% accent wash), so
 * ui_nav_row still uses it and this does not. Two idioms, two call sites, and
 * the shared-home rule is kept where the decision really is shared.
 *
 * THE ZEBRA IS GONE. It was 1% white on alternating rows. On a ladder whose
 * smallest deliberate step is 1.1682:1 a 1% wash is an eighth of the smallest
 * thing anyone is meant to see, and the prototype's `td` separates rows with
 * `border-bottom: 1px solid var(--zd-cut)` instead - a rule, which is legible,
 * costs one row of pixels, and is the same groove everything else uses. The
 * `idx` parameter is kept because every caller passes it and removing it is a
 * 53-app change for nothing; it is now only the row's identity.
 *
 * THE ROW ALSO PUBLISHES ITS INK, and that is not a convenience. A caller
 * cannot know that its row came out as a knockout, so a caller that passes
 * its usual ZD_TEXT_2 writes 1.2131:1 on ZD_KNOCK - invisible text on the one
 * row the user is looking at. Every existing call site would have that bug on
 * the day the knockout landed. So the row records the answer and
 * ui_grid_cell() overrides the caller's colour on a selected row only. It is
 * one module-static, set by the row immediately before its own cells are
 * drawn, which is the only order any caller uses - the same contract as the
 * grid track table twenty lines up. */
static unsigned grid_row_ink = 0;

int ui_grid_row(int x, int y, int w, int idx, int selected)
{
    int h = ui_grid_row_h();
    int fired = ui_fire(x, y, w, h);
    (void)idx;
    grid_row_ink = selected ? ui_color(UI_COLOR_KNOCK_INK) : 0;
    if (ui_mode_get() == UI_DRAW) {
        int st = ctl_state(x, y, w, h, selected, 0);

        /* THE TEXTLESS KNOCKOUT SLAB IS NOT DRAWN HERE, AND THE PROOF IS IN
         * THE RETURN VALUE OF A LINKER WRAP.
         *
         * A previous pass read the 500 x 15 band of bare ZD_KNOCK in the
         * default wmshot frame as a grid row that a scroll viewport had cut in
         * half - the fill is a rectangle and survives the scissor, the label
         * sits 18px further down and does not - and gated the knockout here on
         * the row being wholly inside fb_clip_top()..fb_clip_bot(). The band
         * did not move, because ui_grid_row has no caller anywhere in the tree
         * outside uitest.c: the whole grid API is drawn by nothing yet.
         *
         * Wrapping fb_fill_px with -Wl,--wrap and printing a backtrace for
         * every call laying ZD_KNOCK wider than 200px names the drawer once
         * and for good:
         *
         *     FILL KNOCK x=1289 y=356 w=507 h=52 clip=[220,371)
         *         ui_list_row+0xdd
         *
         * ui_list_row, in ui.c. A 52px row whose scissor ends at 371 shows its
         * top 15px, and a row of this design carries 18px of leading above the
         * glyphs, so the visible sliver is by construction the blank part.
         *
         * The gate it needed therefore belongs in ui.c, where the viewport is
         * known (the S struct), and NOT here: uitest.c links ui.c and uikit.c
         * with no fb.c at all, so a reference to fb_clip_top from this file
         * does not fail a check, it fails the LINK - the 176-check suite
         * stopped building the moment that guard landed. This file learns
         * about clipping through the primitives it calls and in no other way.
         *
         * ui_grid_row can be cut in half by the same viewport once something
         * calls it inside one. When that caller exists the fix is ui.c's, made
         * once for both row idioms, not a second copy of the rule here. */

        if (st == UI_ST_ON) {
            fb_fill_px(x, y, w, h, ui_color(UI_COLOR_KNOCK));
            fb_fill_px(x, y + h - 1, w, 1, ui_color(UI_COLOR_KO_EDGE));
        } else {
            if (st == UI_ST_HOVER || st == UI_ST_PRESS)
                fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL_HI));
            fb_fill_px(x, y + h - 1, w, 1, ui_color(UI_COLOR_CUT));
        }
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
    /* THE SAME MARGIN THE HEAD STANDS ON, and now the same TOKEN. These were
     * DP(8) left and DP(10) right, the same two literals ui_colhead spelled -
     * so the head and the column already agreed, and what was wrong was that
     * they agreed by both repeating the number rather than by both reading
     * it. That is the failure mode design.h's one-home rule exists for: the
     * next edit to one of the two is the one that breaks the alignment, and
     * nothing would have caught it. ZD_CELL_PX is 6dp, the prototype's
     * `padding: 0 6px`, one value at both ends. */
    int cp = DP(ZD_CELL_PX);
    int tw = ui_text_w(s, size, flags);
    int tx = cx + cp;
    if (align == UI_ALIGN_R) tx = cx + cw - tw - cp;
    else if (align == UI_ALIGN_C) tx = cx + (cw - tw) / 2;
    if (tx < cx) tx = cx;
    if (grid_row_ink) rgb = grid_row_ink;
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
    /* the ground first, so the 1px gaps between cells show through as rules -
     * and the ground is ZD_CUT, which makes those gaps the same groove that
     * divides every other pair of things in the design */
    if (c == 0) fb_fill_px(SC.x, cy - gap, SC.w, SC.ch + gap,
                           ui_color(UI_COLOR_CUT));
    fb_fill_px(cx, cy, SC.cw - gap, SC.ch, ui_color(UI_COLOR_PANEL_HI));
    /* the struck run along each cell's top: these are plates in a row and the
     * lamp reaches all of them */
    fb_fill_px(cx, cy, SC.cw - gap, 1, ui_color(UI_COLOR_LIT));
    int px = DP(ZD_STAT_PX), py = DP(ZD_STAT_PY);
    /* the key is the LABEL style - SM, bold, ZD_TEXT_3 at 5.3585:1 on
     * ZD_RAISE. It was ZD_TEXT_6, which is a rung this ladder no longer has:
     * the widening cost the ink ramp its fifth step and design.h now aliases
     * ZD_TEXT_4/5/6 onto ZD_TEXT_3. Naming the rung that exists says what is
     * actually drawn. */
    ui_text(cx + px, cy + py, key, ui_color(UI_COLOR_TEXT_DIM), UI_SM,
            UI_F_BOLD);
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
 * SIDEBAR - the prototype's `#rail` and `.slot`, one level in
 *
 *     #rail          { background: var(--zd-raise);
 *                      border-right: 1px solid var(--zd-cut); }
 *     #rail::before  { left: 0; width: 1px; background: var(--zd-lit); }
 *     .slot          { height: var(--zd-row-h); color: var(--zd-text-3);
 *                      border-bottom: 1px solid var(--zd-cut); }
 *     .slot:hover:not(.act) { background: var(--zd-float); }
 *     .slot.act      { background: var(--zd-base); }
 *     .slot.act .nm  { color: var(--zd-text-0); font-weight: 700; }
 *     .slot .mk      { left: 0; width: var(--zd-focus-bar);
 *                      background: var(--zd-verm); }
 *
 * THE RAIL IS LIT ON ITS LEFT, NOT ON ITS TOP, and that is the lamp again
 * rather than a variation. The rail runs the full height of the screen: it has
 * no top for the light to strike, and the edge facing the source is its left
 * one. So the struck run goes there and the far edge is cut away.
 *
 * THE SELECTED SLOT IS THE REGISTER MARK, NOT THE KNOCKOUT, and this is the
 * one place in this file that still calls ui_row_select(). Table rows flip
 * value; register slots take the overprint's job 1 - a 3dp vermilion mark on
 * the rail's inner edge - and move one rung of ground. That is what
 * ui_row_select() already draws, so the shared decision keeps its single home
 * and the two idioms stay distinguishable, which is the entire point of
 * having two.
 * ========================================================================= */
int ui_sidebar_w(void) { return DP(ZD_SIDEBAR_W); }

void ui_sidebar(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    fb_fill_px(x, y, w, h, ui_color(UI_COLOR_PANEL_HI));
    fb_fill_px(x, y, 1, h, ui_color(UI_COLOR_LIT));          /* struck left */
    fb_fill_px(x + w - 1, y, 1, h, ui_color(UI_COLOR_CUT));  /* cut far side */
}

int ui_heading_h(void)
{
    return DP(ZD_HEADING_PT) + ui_text_h(UI_SM) + DP(ZD_HEADING_PB);
}

/* `#rail .sect` - the LABEL style, and now all four parts of it rather than
 * two. This said "tracking is not available at this layer (see ui_colhead)";
 * ui_colhead no longer says that either, and this draws through the same
 * lab_text() so the rail's section heads and a table's column heads are one
 * style with one implementation. The ink is ZD_TEXT_3, 5.3585:1 on the rail;
 * it was ZD_TEXT_6, a rung the widened ladder no longer has.
 *
 * The indent is ZD_CELL_PX, the same margin a cell stands on, not the DP(8)
 * literal that was here - a section head and the rows under it sitting on two
 * different left margins is the thing a reader sees before they read either. */
void ui_heading(int x, int y, int w, const char *s)
{
    (void)w;
    if (ui_mode_get() != UI_DRAW) return;
    lab_text(x + DP(ZD_CELL_PX), y + DP(ZD_HEADING_PT), s,
             ui_color(UI_COLOR_TEXT_DIM));
}

int ui_nav_h(void) { return DP(ZD_NAV_H); }

int ui_nav_row(int x, int y, int w, const char *s, int selected)
{
    int h = ui_nav_h();
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        int st = ctl_state(x, y, w, h, selected, 0);
        if (st == UI_ST_HOVER || st == UI_ST_PRESS)
            fb_fill_px(x, y, w, h, ui_color(UI_COLOR_SURF_5));   /* .slot:hover */
        ui_row_select(x, y, w, h, selected, 0);
        fb_fill_px(x, y + h - 1, w, 1, ui_color(UI_COLOR_CUT));  /* the groove */
        ui_text(x + DP(ZD_NAV_PX), text_cy(y, h, UI_MD, 0), s,
                selected      ? ui_color(UI_COLOR_TEXT_HI)
              : st == UI_ST_OFF ? ui_color(UI_COLOR_TEXT_DIM)
                                : ui_color(UI_COLOR_TEXT),
                UI_MD, selected ? UI_F_BOLD : 0);
        ui_ring(x, y, w, h);
    }
    return fired;
}

/* ============================================================================
 * BARS - the prototype's `.mtrack` / `.mfill`
 *
 *     .mtrack { height: 9dp; background: var(--zd-well);
 *               border: 1px solid var(--zd-cut); overflow: hidden; }
 *     .mfill  { background: var(--zd-steel); }
 *     .mfill.warn { background: var(--zd-warn); }
 *     .mfill.bad  { background: var(--zd-bad); }
 *
 * FOUR THINGS FOLLOW FROM THOSE FOUR LINES.
 *
 * 1  A TRACK IS A PIT. Sunken seat, ZD_WELL face, ZD_CUT ring, ZD_CUT along
 *    the inside top and ZD_LITSOFT along the inside bottom - 2.5750:1, the
 *    grazed value, because the light reaches the near wall of a groove across
 *    its floor rather than striking it. It was ZD_SURF_1/ZD_SURF_0 with no
 *    edge at all, which on the old ladder was invisible and on this one is a
 *    dark rectangle with no explanation of why it is dark.
 *
 * 2  THE RADIUS IS ZD_R_BOLT. A meter cannot move; nothing about it is a
 *    plate. design.h already says so for all four bars.
 *
 * 3  THE FILL IS ZD_STEEL AND NOT THE ACCENT. This is the two-ink contract:
 *    ZD_STEEL is THE MACHINE'S OWN READING and appears inside an instrument
 *    and nowhere else - meter fills, spark bars, plot lines. ZD_VERM is the
 *    overprint and has four jobs, none of which is "how full is the disk".
 *    The colour is still a parameter, because a caller passing ZD_WARN or
 *    ZD_BAD is `.mfill.warn` / `.mfill.bad` and the prototype has both; what
 *    changed is that ZD_STEEL is the answer when a caller has no opinion.
 *
 * 4  ZD_METER_H IS THE PROTOTYPE'S 9, NOT THE PREDECESSOR'S 5. This note used
 *    to record the disagreement instead of fixing it - design.h said 5 and
 *    `.mtrack` says 9 - and the disagreement mattered, because 1px of ring on
 *    each side of a 5dp track leaves 3dp of fill and the wall is then 40% of
 *    the instrument. design.h now carries 9; nothing in this file changed,
 *    which is the point of reading the height from a token.
 * ========================================================================= */
int ui_meter_h(void)    { return imax(2, DP(ZD_METER_H)); }
int ui_progress_h(void) { return imax(2, DP(ZD_PROG_H)); }
int ui_minibar_h(void)  { return imax(2, DP(ZD_MINIBAR_H)); }
int ui_segbar_h(void)   { return imax(4, DP(ZD_SEGBAR_H)); }

/* The one shape all four share: a pit, and a fill inside its walls. `inset` is
 * 1 when the track is tall enough to carry a ring and 0 when it is not - a
 * 2dp bar drawn with a 1px ring on each side has no interior left, so the
 * inline sparkbar keeps the pit and drops the walls. */
static void bar_track(int x, int y, int w, int h, int r, int pct,
                      unsigned rgb, int inset)
{
    pct = clamp(pct, 0, 100);
    if (inset && h >= 4) {
        seat_sunken(x, y, w, h, r, ui_color(UI_COLOR_SURF_WELL));
        x += 1; y += 1; w -= 2; h -= 2;
        r = r > 1 ? r - 1 : 0;
    } else {
        fb_rrect(x, y, w, h, r, ui_color(UI_COLOR_SURF_WELL));
    }
    if (w <= 0 || h <= 0) return;
    if (!rgb) rgb = ui_color(UI_COLOR_STEEL);   /* the machine's own reading */
    if (pct) fb_rrect(x, y, w * pct / 100, h, r, rgb);
}

void ui_meter(int x, int y, int w, int pct, unsigned rgb)
{
    if (ui_mode_get() != UI_DRAW) return;
    bar_track(x, y, w, ui_meter_h(), DP(ZD_METER_R), pct, rgb, 1);
}

void ui_progress(int x, int y, int w, int pct, unsigned rgb)
{
    if (ui_mode_get() != UI_DRAW) return;
    bar_track(x, y, w, ui_progress_h(), DP(ZD_PROG_R), pct, rgb, 1);
}

void ui_minibar(int x, int y, int w, int pct, unsigned rgb)
{
    int h = ui_minibar_h(), r = DP(ZD_MINIBAR_R);
    if (ui_mode_get() != UI_DRAW) return;
    pct = clamp(pct, 0, 100);
    /* the reference floors the fill at 2% so a non-zero value is never
     * invisible - a 0px bar and a 0-value bar mean different things */
    bar_track(x, y, w, h, r, imax(pct, pct ? 2 : 0), rgb, 0);
}

static struct { int x, y, w, total, used; } SB;

void ui_segbar_begin(int x, int y, int w, int total)
{
    SB.x = x; SB.y = y; SB.w = w; SB.total = total > 0 ? total : 1; SB.used = 0;
    if (ui_mode_get() != UI_DRAW) return;
    /* the same pit as every other bar. Its outline was ZD_SURF_5, which is now
     * ZD_FLOAT - a LIGHTER surface than the plate. A pale ring round a sunken
     * track says the track is raised, which is the opposite of true. */
    seat_sunken(x, y, w, ui_segbar_h(), DP(ZD_SEGBAR_R),
                ui_color(UI_COLOR_SURF_WELL));
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
 * MONOSPACE PANEL - the prototype's `.well`
 *
 * THREE GROUNDS BECAME ONE, and the reason the third existed is gone. The old
 * comment said the terminal ground was kept because it was the same colour as
 * an active tab, which is how the tab connected to the body. Under PRESSWORK
 * the active tab is a knockout at 6.4796:1 - there is nothing to connect to,
 * and holding a ground steady for a join that no longer exists is carrying a
 * constraint past the thing that imposed it.
 *
 * So every mono panel is `.well`: ZD_WELL face, ZD_CUT ring, ZD_CUT along the
 * inside top and ZD_LITSOFT along the inside bottom, at ZD_R_INSET because a
 * panel is one level inside a plate. Text on it measures 12.7802:1 for
 * ZD_TEXT_1 and 9.0436:1 for ZD_TEXT_3, so the ladder's whole ink ramp is
 * available in a pit, which is what a pit is for.
 *
 * `kind` still selects the line height, which is what actually differed
 * between the five instances. It no longer selects a ground.
 * ========================================================================= */
void ui_mono_panel(int x, int y, int w, int h, int kind)
{
    (void)kind;
    if (ui_mode_get() != UI_DRAW) return;
    seat_sunken(x, y, w, h, DP(ZD_R_INSET), ui_color(UI_COLOR_SURF_WELL));
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
    /* `w` was the width of the highlight wash. The wash is gone (see below)
     * and the mark is in the left margin, so the line no longer needs to know
     * how wide the panel is - kept in the signature because 53 apps call it
     * and the parameter is free. */
    (void)w;
    if (ui_mode_get() != UI_DRAW) return;
    if (highlight) {
        /* THE JUMP TARGET IS A MARK, NOT A WASH. It was a 7% vermilion tint
         * across the whole line - an area of overprint far wider than
         * ZD_FOCUS_BAR, and at 7% it computes to a change of well under the
         * ladder's smallest deliberate step, so it was simultaneously against
         * the width rule and too quiet to see. It is now the overprint doing
         * the job the overprint is for: a ZD_FOCUS_BAR-wide vermilion rule in
         * the panel's left margin, the same mark the focused plate and the
         * rail's active slot carry, at 4.6319:1 against the plate. */
        int bw = imax(1, DP(ZD_FOCUS_BAR)), pad = DP(4);
        fb_fill_px(x - pad - bw, y, bw, ui_mono_line_h(kind),
                   ui_color(UI_COLOR_ACCENT));
    }
    fb_text_aa(x, y, s, rgb);
}

/* ============================================================================
 * CARDS AND KEY/VALUE - the prototype's `.card` and `.kv`
 *
 *     .card { background: var(--zd-raise); border-radius: var(--zd-r-inset);
 *             border: 1px solid var(--zd-cut);
 *             box-shadow: inset 0 1px 0 0 var(--zd-lit); }
 *     .kv   { min-height: 19dp; border-bottom: 1px solid var(--zd-cut); }
 *     .kv .k{ SM, uppercase, 700, tracked, colour var(--zd-text-3) }
 *     .kv .v{ mono SM, colour var(--zd-text-0), right aligned }
 *
 * "RAISED, ONE NESTING LEVEL IN. Radius halves; value moves one rung." That
 * one line of the prototype is the whole card: ZD_R_INSET is half of
 * ZD_R_PLATE and ZD_RAISE is one rung above ZD_BASE. The card's outline was
 * ZD_SURF_5, which is now ZD_FLOAT - a lighter surface than the card itself,
 * so the card was ringed in something brighter than its own face. Under one
 * lamp that means light is coming from every direction at once, which is the
 * one thing this design does not allow. It is ZD_CUT, and the ZD_LIT run
 * along the inside top is where the light actually is.
 * ========================================================================= */
void ui_card(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    seat_raised(x, y, w, h, DP(ZD_CARD_R), ui_color(UI_COLOR_PANEL_HI),
                ui_color(UI_COLOR_CUT), 1);
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
    fb_fill_px(x, y + h - 1, w, 1, ui_color(UI_COLOR_CUT));
}

int ui_kv_h(void) { return DP(ZD_KV_H); }

void ui_kv(int x, int y, int w, const char *k, const char *v,
           unsigned v_rgb, int first)
{
    int h = ui_kv_h();
    if (ui_mode_get() != UI_DRAW) return;
    /* "cards that hold rows use borderTop on rows 2..n rather than a gap".
     * The prototype puts the rule on the bottom of every `.kv`; the difference
     * is one hairline at the end of the list, and drawing it on top of rows
     * 2..n is what stops it doubling with the card's own bottom edge. Kept as
     * it was; only the colour moves, from ZD_FLOAT to the groove. */
    if (!first) fb_fill_px(x, y, w, 1, ui_color(UI_COLOR_CUT));
    /* the key is the LABEL style and the value is the DATA style - mono, one
     * size forever, ZD_TEXT_0 unless the caller has a semantic to say.
     * 5.3585:1 and 8.9894:1 respectively on ZD_RAISE.
     *
     * `.kv .k` is `text-transform: uppercase; letter-spacing: var(--tr-lab);
     * font-weight: 700`, which is the same four-part style ui_colhead and
     * ui_heading draw, so it goes through the same helper. That is the whole
     * argument for having a helper: this is the FOURTH site, and a style with
     * four hand-rolled copies is a style that will have three variants by the
     * next change. The 13dp indent is left alone - it is not a number this
     * pass measured off the prototype, and moving it would be a guess. */
    lab_text(x + DP(13), text_cy(y, h, UI_SM, UI_F_BOLD), k,
             ui_color(UI_COLOR_TEXT_DIM));
    int vw = ui_text_w(v, UI_SM, UI_F_MONO);
    ui_text(x + w - DP(13) - vw, text_cy(y, h, UI_SM, UI_F_MONO), v,
            v_rgb ? v_rgb : ui_color(UI_COLOR_TEXT_HI), UI_SM, UI_F_MONO);
}

/* ============================================================================
 * OVERLAYS - the three objects that are genuinely off the plane
 *
 * A menu, a modal sheet and a toast are the ONLY things in PRESSWORK that get
 * a shadow, and they get one because they are the only things that are not
 * lying on the plate. Everything else is separated by the ladder and the two
 * 1px runs. They also share a ring: `border: 1px solid var(--zd-edge-over)`,
 * the design's "this object is in front of that one" boundary, 3.4322:1 on
 * ZD_FLOAT at worst and 4.9991:1 on ZD_BASE. That is the ONE place a light
 * boundary is correct, because the alternative - a darker ring - tops out at
 * 1.4723:1 on the plate and cannot clear the 3:1 an occlusion question needs.
 * ========================================================================= */
void ui_popover(int x, int y, int w, int h)
{
    if (ui_mode_get() != UI_DRAW) return;
    lift_shadow(x, y, w, h, DP(ZD_MENU_R));
    seat_face(x, y, w, h, DP(ZD_MENU_R), ui_color(UI_COLOR_SURF_5),
              ui_color(UI_COLOR_EDGE_OVER));
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
    /* `.mi` - the menu row. ZD_TEXT_1 at 6.4820:1 on ZD_FLOAT, a 1px ZD_CUT
     * rule under each row but the last, and the highlighted row is THE
     * KNOCKOUT: `.mi:hover { background: var(--zd-knock); color:
     * var(--zd-knock-ink); }`. It was a 15% vermilion tint, which is an area
     * of overprint the width rule does not allow and which computed to a
     * change most people would not see. */
    for (int i = 0; i < n; i++) {
        int iy = y + pad + i * ih;
        int iw = w - 2 * pad;
        if (item_at(items, i, buf, sizeof buf) < 0) break;
        if (ui_fire(x + pad, iy, iw, ih)) hitidx = i;
        if (ui_mode_get() != UI_DRAW) continue;
        int st = ctl_state(x + pad, iy, iw, ih, i == sel, 0);
        unsigned ink;
        if (st == UI_ST_ON || st == UI_ST_HOVER || st == UI_ST_PRESS) {
            fb_rrect(x + pad, iy, iw, ih, DP(ZD_MENU_ITEM_R),
                     ui_color(UI_COLOR_KNOCK));
            fb_fill_px(x + pad, iy + ih - 1, iw, 1, ui_color(UI_COLOR_KO_EDGE));
            ink = ui_color(UI_COLOR_KNOCK_INK);
        } else {
            if (i + 1 < n)
                fb_fill_px(x + pad, iy + ih - 1, iw, 1, ui_color(UI_COLOR_CUT));
            ink = ui_color(UI_COLOR_TEXT);
        }
        ui_text(x + pad + DP(ZD_MENU_ITEM_PX), text_cy(iy, ih, UI_MD, 0), buf,
                ink, UI_MD, 0);
        ui_ring(x + pad, iy, iw, ih);
    }
    return hitidx;
}

int ui_modal_head_h(void) { return DP(ZD_MODAL_HEAD_H); }

/* A MODAL IS A SHEET AND ITS HEADER IS A KNOCKOUT. The prototype's command
 * palette is the shape:
 *
 *     #palette .sheet { background: var(--zd-float);
 *                       border: 1px solid var(--zd-edge-over);
 *                       border-radius: var(--zd-r-plate);
 *                       box-shadow: 0 6dp 14dp var(--zd-lift); }
 *     #palette .ph    { height: var(--zd-title-h); background: var(--zd-knock); }
 *     #palette .ph span { color: var(--zd-knock-ink); }
 *
 * ZD_R_PLATE, not ZD_R_INSET: a modal moves as a whole, exactly like a window,
 * so it takes the window's corner. And the header is the knockout for the same
 * reason a focused window's is - a modal has the keyboard by definition, and
 * PRESSWORK says that with a value flip and nothing else. 8.5329:1 for the
 * title on the plate, and ZD_KO_EDGE along the bottom of it at 2.5487:1,
 * because on a light plate the direction with headroom is downward. */
void ui_modal(int x, int y, int w, int h, const char *title)
{
    int r = DP(ZD_MODAL_R);
    if (ui_mode_get() != UI_DRAW) return;
    lift_shadow(x, y, w, h, r);
    seat_face(x, y, w, h, r, ui_color(UI_COLOR_SURF_5),
              ui_color(UI_COLOR_EDGE_OVER));
    if (title && title[0]) {
        int hh = ui_modal_head_h();
        int tw = ui_text_w(title, UI_MD, UI_F_BOLD);
        /* the header plate is clipped by the sheet's own top arc, so it is
         * drawn as a rounded rect of the same radius and then squared off
         * along its bottom - the same two calls the window header uses */
        fb_rrect(x + 1, y + 1, w - 2, hh, r - 1, ui_color(UI_COLOR_KNOCK));
        fb_fill_px(x + 1, y + hh / 2, w - 2, hh - hh / 2,
                   ui_color(UI_COLOR_KNOCK));
        fb_fill_px(x + 1, y + hh, w - 2, 1, ui_color(UI_COLOR_KO_EDGE));
        ui_text(x + (w - tw) / 2, text_cy(y, hh, UI_MD, UI_F_BOLD), title,
                ui_color(UI_COLOR_KNOCK_INK), UI_MD, UI_F_BOLD);
    }
    /* NO SCRIM, still. The prototype dims the desk behind the command palette
     * and the overview and behind nothing else, and neither of those is this
     * widget. Its shade term is ZD_CUT at 78%, which is a shade rather than a
     * fog because ZD_CUT is L* 1.17; whoever draws it owns that call. */
}

int ui_toast_h(void)
{
    return 2 * DP(ZD_TOAST_PY) + ui_text_h(UI_MD) + DP(3) + ui_text_h(UI_SM);
}

/* `.toast` - the third object off the plane.
 *
 *     .toast { background: var(--zd-float);
 *              border: 1px solid var(--zd-edge-over);
 *              border-radius: var(--zd-r-inset);
 *              padding-left: 14dp;
 *              box-shadow: 0 6dp 14dp var(--zd-lift); }
 *     .toast .bar { left: 0; top: 0; bottom: 0;
 *                   width: var(--zd-focus-bar); background: var(--zd-verm); }
 *
 * THE 20dp ICON SQUARE IS GONE AND THE BAR REPLACES IT. A solid 20dp square of
 * ZD_BAD or ZD_WARN is 400 square dp of a state colour carrying the state on
 * its own, which is the thing the prototype's `.sw` rule exists to prevent
 * ("four characters of text next to the mark, which is why the colour axis
 * never carries a state alone"). The toast already has a title saying what
 * happened. So the kind becomes a ZD_FOCUS_BAR-wide rule down the left edge -
 * the same mark the focused plate and the rail's active slot carry, at the
 * one width the overprint budget allows - and the reclaimed column goes to
 * the text.
 *
 * The bar takes `kind_rgb` rather than always ZD_VERM. The prototype's toast
 * is always vermilion because its toast is always neutral; a kernel toast can
 * be a failure, and ZD_BAD / ZD_WARN / ZD_OK on a 3dp rule is within the same
 * width rule that lets the vermilion one exist. */
void ui_toast_draw(int x, int y, int w, const char *title, const char *body,
                   unsigned kind_rgb)
{
    int h = ui_toast_h(), py = DP(ZD_TOAST_PY), px = DP(ZD_TOAST_PX);
    int bw = imax(1, DP(ZD_FOCUS_BAR)), r = DP(ZD_TOAST_R);
    if (ui_mode_get() != UI_DRAW) return;
    lift_shadow(x, y, w, h, r);
    seat_face(x, y, w, h, r, ui_color(UI_COLOR_SURF_5),
              ui_color(UI_COLOR_EDGE_OVER));
    fb_fill_px(x + 1, y + r, bw, h - 2 * r,
               kind_rgb ? kind_rgb : ui_color(UI_COLOR_ACCENT));
    int tx = x + px + bw;
    ui_text(tx, y + py, title, ui_color(UI_COLOR_TEXT_HI), UI_MD, UI_F_BOLD);
    ui_text(tx, y + py + ui_text_h(UI_MD) + DP(3), body,
            ui_color(UI_COLOR_TEXT_2), UI_SM, 0);
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
 * 52.63% RENDERS AS A FLAT LINE PINNED TO THE TOP. docs/reference/ui/widgets.md S20.18
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
    /* quarters - three lines, not four edges. The rule is ZD_CUT, the groove,
     * because a ruled chart is a ruled sheet and every rule in this design is
     * the same groove. It was ZD_SURF_5, which now resolves to ZD_FLOAT: a
     * LIGHTER value than the plate, so the graticule would have been brighter
     * than the surface it is ruled onto. */
    for (int i = 1; i < ZD_SPARK_GRID; i++)
        fb_fill_px(x, y + SP.h * i / ZD_SPARK_GRID, w, 1,
                   ui_color(UI_COLOR_CUT));
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

/* THE PLOT IS STEEL AND THIS IS THE TWO-INK CONTRACT'S SHARPEST EDGE.
 *
 * The line and the area under it were ui_color(UI_COLOR_ACCENT) - vermilion -
 * and the area fill was 16% of it across the whole chart. On a 200 x 66 dp
 * spark that is 13,200 square dp of overprint, against a budget of a 3dp bar.
 * ZD_STEEL is the token that exists for exactly this: "the machine's own
 * reading... raster columns, meter fills, spark bars, PLOT LINES, ruler
 * regions. Never a control, never focus, never a border." It reads 6.1881:1
 * on ZD_BASE and 8.3765:1 in a well, so the plot is louder than it was and
 * costs the overprint nothing.
 *
 * ZD_STEEL_BR is the line and ZD_STEEL is the area, one rung apart, so the
 * trace stays readable where it crosses its own fill. */
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
                          ui_color(UI_COLOR_STEEL), pct255(ZD_SPARK_AREA_A));
        }
    }
    for (int i = 0; i + 1 < SP.n; i++)
        fb_line(spark_x(i), spark_y(i), spark_x(i + 1), spark_y(i + 1),
                ui_color(UI_COLOR_STEEL_BR));
    SP.n = 0;
}

/* ============================================================================
 * INDICATORS AND INPUTS - S18
 * ========================================================================= */
int ui_dot_size(void) { return imax(2, DP(ZD_DOT)); }

/* `.dot` - a 7dp SQUARE mark, and the prototype gives it no border-radius at
 * all. design.h takes it to ZD_R_CHIP, which is 2dp of corner on a 6dp mark;
 * that is the token layer's call and it is used as written, and it still reads
 * as a printed mark rather than as an LED.
 *
 * THE GLOW IS DELETED. It was two blended rings approximating a 7px
 * box-shadow. PRESSWORK has one shadow token and it is drawn under three
 * objects that are off the plane; a status dot is not one of them, and a halo
 * round a 6dp square is the single most recognisable "this is a dark-mode UI
 * kit" tell there is. design.h already zeroed ZD_BLUR_GLOW_A/B for the same
 * reason. The parameter stays, because every call site passes it and its
 * meaning - "this one is live" - is still true; it is simply not drawn as
 * light any more. Callers that need to say "live" have the state word beside
 * the mark, which is the prototype's `.sw` rule. */
void ui_dot(int x, int y, unsigned rgb, int glow)
{
    int d = ui_dot_size();
    (void)glow;
    if (ui_mode_get() != UI_DRAW) return;
    fb_rrect(x, y, d, d, DP(ZD_DOT_R), rgb);
}

int ui_badge_h(void) { return 2 * DP(ZD_BADGE_PY) + ui_text_h(UI_SM); }

int ui_badge_w(const char *s)
{
    return 2 * DP(ZD_BADGE_PX) + ui_text_w(s, UI_SM, UI_F_MONO);
}

/* A BADGE IS A SMALL PLATE WITH A SEMANTIC INK ON IT, not a tinted region.
 *
 * It was a 14% wash of its own colour with the text in that colour on top. The
 * old reasoning still holds - a solid BAD badge beside a solid BAD status dot
 * reads as two errors - and PRESSWORK reaches the same conclusion by a route
 * that survives the ladder: the badge is the ordinary raised seat, ZD_RAISE
 * with the ZD_CUT ring and the ZD_LIT run, and the semantic lives entirely in
 * the ink. Measured on ZD_RAISE: ZD_OK 5.0630:1, ZD_WARN 5.1997:1, ZD_BAD_INK
 * 4.7396:1, ZD_STEEL 4.9632:1 - every one clears 4.5. A 14% wash of ZD_BAD on
 * ZD_RAISE clears nothing; it is a colour cast, and a colour cast is what this
 * design replaced with a ladder.
 *
 * ZD_BAD ITSELF IS NOT AN INK. It is 3.4161:1 on ZD_RAISE - a fill and a mark,
 * which is what design.h says it is. Failure TEXT is ZD_BAD_INK, and a caller
 * passing the fill colour where an ink belongs gets corrected here rather than
 * in 53 apps. */
void ui_badge(int x, int y, const char *s, unsigned rgb)
{
    int w = ui_badge_w(s), h = ui_badge_h();
    if (ui_mode_get() != UI_DRAW) return;
    seat_raised(x, y, w, h, DP(ZD_BADGE_R), ui_color(UI_COLOR_PANEL_HI),
                ui_color(UI_COLOR_CUT), 1);
    if (rgb == ui_color(UI_COLOR_DANGER)) rgb = (unsigned)ZD_BAD_INK;
    ui_text(x + DP(ZD_BADGE_PX), text_cy(y, h, UI_SM, UI_F_MONO), s, rgb,
            UI_SM, UI_F_MONO);
}

int ui_input_h(void)  { return DP(ZD_INPUT_H); }
int ui_search_h(void) { return DP(ZD_SEARCH_H); }

/* AN INPUT IS A PIT, AND ITS FOCUS IS THE OVERPRINT'S JOB 1.
 *
 * The prototype's `#palette .pin` is `background: var(--zd-well)` with a
 * ZD_CUT rule under it, and the general sunken shape is `.well`. So the field
 * is seat_sunken: ZD_WELL face, ZD_CUT ring, ZD_CUT along the inside top,
 * ZD_LITSOFT along the inside bottom. Text in it reads 12.7802:1, the best in
 * the system, which is the right way round for the one surface people type on.
 *
 * FOCUS IS A ZD_FOCUS_BAR-WIDE VERMILION RULE DOWN THE LEFT EDGE. It was a
 * full vermilion box round the field - four sides at 4.6319:1, which is a
 * loud rectangle and is also a BORDER in the overprint, and the two-ink
 * contract says vermilion is never a border. The bar is the same mark the
 * focused plate carries down its own left edge (`.fbar`), at the same width,
 * for the same reason: it is where the keyboard is. Everything else about the
 * field is unchanged when it gains focus, which is what makes the bar legible.
 *
 * THE CARET IS DRAWN. `.caret` is a 7 x 13dp vermilion block, overprint on a
 * pit; without it a focused empty field and an unfocused one differ only at
 * the left margin. It does not blink - there is no per-frame clock at this
 * layer and a caret that blinks in one app and not another is worse than one
 * that does not blink at all.
 *
 * THE PLACEHOLDER IS ZD_TEXT_3 AND NOT ZD_TEXT_INERT. It was ZD_TEXT_6, which
 * design.h now aliases onto ZD_TEXT_3, so the pixels are already right - but
 * naming the rung that exists matters, because the neighbouring rung is
 * ZD_TEXT_INERT at 1.73:1 and a placeholder is exactly the kind of "it's only
 * a hint" text that gets demoted into it. It is a glyph. It does not go there. */
static int input_body(int x, int y, int w, int h, int r, const char *text,
                      const char *placeholder, int focused)
{
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() != UI_DRAW) return fired;
    int has = text && text[0];
    seat_sunken(x, y, w, h, r, ui_color(UI_COLOR_SURF_WELL));
    const char *s = has ? text : placeholder;
    unsigned ink = has ? ui_color(UI_COLOR_TEXT)
                       : ui_color(UI_COLOR_TEXT_DIM);
    int bw = imax(1, DP(ZD_FOCUS_BAR)), cw = imax(1, DP(2));
    int tx = x + DP(ZD_INPUT_PX);
    /* THE CONTENT MOVES OVER BY THE BAR, and that is the prototype's own
     * behaviour rather than a compromise: `.win.focus .wbody` gains exactly
     * `var(--zd-focus-bar)` of left padding when the plate takes focus. The
     * mark is drawn IN the field, not around it, so something has to give it
     * room, and the alternative - letting the bar sit on top of the first
     * glyph - is the thing that would look like a bug. */
    if (focused) tx += bw;
    /* an empty focused field gets the caret where the first glyph would go and
     * the placeholder steps aside for it; a field with text gets it after the
     * text. Either way the caret never lands on a glyph. */
    int cx = focused ? (has ? tx + ui_text_w(text, UI_MD, 0) + DP(1) : tx) : 0;
    if (focused && !has) tx += cw + DP(2);
    if (s) ui_text(tx, text_cy(y, h, UI_MD, 0), s, ink, UI_MD, 0);
    if (focused) {
        fb_fill_px(x + 1, y + 1, bw, h - 2, ui_color(UI_COLOR_ACCENT));
        if (cx + cw < x + w - 1)
            fb_fill_px(cx, text_cy(y, h, UI_MD, 0), cw, ui_text_h(UI_MD),
                       ui_color(UI_COLOR_ACCENT));
    }
    /* THERE ARE TWO NOTIONS OF FOCUS HERE AND ONLY ONE OF THEM WAS DRAWN.
     *
     * `focused` is the APP's: this is the field my program is putting
     * keystrokes into, and the in-field bar and caret above say so. ui_ring()
     * is the TOOLKIT's: ui.c's funnel counts a focus stop per ui_fire() call
     * and this widget makes one, so a keyboard walking a form has always
     * stopped here - and drew nothing when it did, because the only mark this
     * control had was the one the app controls. A tab stop that renders
     * identically whether or not it is the tab stop is the `focus` state
     * missing, which is the third of the five this file's header promises.
     *
     * The two marks do not collide: ui_ring() draws its bar OUTSIDE the left
     * edge, in the gutter ui.c's layout cursor leaves, where every other
     * control in the toolkit puts it; the app's bar is drawn INSIDE the well.
     * When both are true - the common case, an app focusing the field the
     * keyboard is on - they stack into one continuous mark, which is the
     * right picture for a field that is focused twice over. */
    ui_ring(x, y, w, h);
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

/* THE CHIP IS THE BUTTON AT ITS SMALLEST, so it routes through pill_face()
 * rather than repeating it. That is not tidiness: a chip and a button that
 * disagree about what "selected" looks like is exactly the drift this file's
 * third rule is about, and pill_face's UI_ST_ON branch had no caller until
 * this one. Its selected state is the knockout, 6.4796:1, not a vermilion
 * fill - a row of eight active filter chips filled with the overprint is the
 * whole overprint budget spent on a filter bar.
 *
 * ZD_CHIP_R is ZD_R_CHIP, which is the radius named after this widget: 2dp,
 * the smallest object on screen. Under the old sixteen-value scale it was an
 * 8px pill and shared a corner with a window. */
int ui_chip(int x, int y, const char *s, int active)
{
    int w = ui_chip_w(s), h = ui_chip_h(), r = DP(ZD_CHIP_R);
    int st = ctl_state(x, y, w, h, active, 0);
    int fired = ui_fire(x, y, w, h);
    if (ui_mode_get() == UI_DRAW) {
        unsigned ink = ui_color(UI_COLOR_TEXT_2);
        pill_face(x, y, w, h, r, UI_BTN_NEUTRAL, st, &ink);
        ui_text(x + DP(ZD_CHIP_PX), text_cy(y, h, UI_SM, 0), s, ink, UI_SM, 0);
        ui_ring(x, y, w, h);
    }
    return fired;
}
