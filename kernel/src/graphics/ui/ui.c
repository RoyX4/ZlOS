/* ui.c - an immediate-mode toolkit. No allocation, no widget tree.
 *
 * IMMEDIATE MODE WAS FORCED, NOT CHOSEN. A retained tree (Qt, GTK,
 * SerenityOS's LibGUI) allocates an object per widget and holds parent/child
 * pointers - a tree needs a heap, and a tree of children IS a list. zlOS has
 * neither: the kernel subset's zl_list_n is a hard fault. So ui_button("OK")
 * RETURNS whether it was clicked, nothing is allocated, and widget state stays
 * in the app where it already lives.
 *
 * THE CONSEQUENCE, AND IT IS THE THING MOST LIKELY TO BE GOT WRONG:
 * hit testing re-runs app_draw with drawing switched OFF - the same trick
 * intel_modeset_dry() uses for its 35-step sequence. So a widget must RETURN
 * whether it fired and must NEVER take an action as an argument, because C
 * evaluates arguments eagerly: ui_button("Delete", delete_everything()) would
 * delete everything on every hit-test pass, twice per click. That is the same
 * language behaviour that forced MS_STEP to be a macro rather than a flag.
 *
 * LAYOUT IS A FLOWING CURSOR and that is the entire algorithm. A widget asks
 * for a size, is placed at the cursor, the cursor advances by size + gap, and
 * rows wrap at the content width. No tree walk, no constraint solver, no
 * second pass.
 */

#include "ui.h"
#include "design.h"

/* ui.c is also linked by standalone host probes without the compositor. Weak
 * settled fallbacks keep that boundary real; wm.c's strong definitions replace
 * them in the kernel and compositor harnesses. */
__attribute__((weak)) int wm_anim_at(int id, int kind, int x, int y, int w, int h)
{ (void)id; (void)kind; (void)x; (void)y; (void)w; (void)h; return 0; }
__attribute__((weak)) int wm_anim_progress(int id, int kind)
{ (void)id; (void)kind; return -1; }

void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_fill_blend(int x, int y, int w, int h, unsigned int rgb, int a);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_rrect_blend(int x, int y, int w, int h, int r, unsigned int rgb, int a);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
int  fb_cell_w(void);
int  fb_cell_h(void);

/* ---- the theme ------------------------------------------------------------
 * One struct, every colour and every metric. Before this they were spread
 * across kernel.zl and fb.c as literals picked by eye per window. */
static struct ui_theme theme;

const struct ui_theme *ui_theme(void) { return &theme; }
void ui_theme_set(const struct ui_theme *t) { theme = *t; }

static int dp(int n, int q8) { return (n * q8 + 128) >> 8; }

/* ---- the two runtime switches the settings pane owns -----------------------
 * THEY ARE STATE ABOVE THE THEME, NOT INSIDE IT, and that is the whole reason
 * they exist as statics here instead of as fields the caller pokes.
 * ui_theme_init_q8() rebuilds the entire struct from design.h on every scale
 * change, so a switch stored in the struct is silently reverted the next time
 * anybody drags the UI-scale slider - the setting would appear to work and
 * then quietly undo itself, which is this repo's most expensive bug shape.
 * Held here, the rebuild reads them back and the two survive a rescale.
 *
 * knock_off  1 == the knockout is switched OFF and the focused header falls
 *            back to ZD_FOCUS_WASH. See design.h's note on that token for what
 *            the fallback is and what it measures.
 * fbar_dp    the focus bar's width in DESIGN px. Stored in dp rather than in
 *            pixels because a rescale has to re-derive it; storing the pixel
 *            value would pin a 3dp bar at its 1x width on a 2x screen.
 */
static int knock_off;                 /* 0 == the knockout, which is default */
static int fbar_dp = ZD_FOCUS_BAR;

void ui_theme_init_q8(int scale_q8)
{
    if (scale_q8 < 192) scale_q8 = 192;
    if (scale_q8 > 768) scale_q8 = 768;
    /* ---- ONE PALETTE, and design.h is the one -------------------------------
     * Every value below names a token in kernel/src/graphics/ui/design.h, and
     * design.h's values come from docs/design/presswork-prototype.html - the
     * running, pixel-verified prototype of PRESSWORK. No call site anywhere
     * names a colour: kernel.zl carries semantic role numbers and calls
     * ui_color(). Re-pointing the whole desktop is editing design.h and
     * nothing else, and that is the property worth protecting here.
     *
     * WHAT PRESSWORK IS, because every mapping below only makes sense against
     * it. A warm-graphite machine lit by ONE RAKING LIGHT entering from
     * off-screen upper left, carrying the apparatus of a printed technical
     * document. The lamp never moves. Consequently depth is not a shadow: a
     * plate is separated from its ground by a 1px LIT run along its top (the
     * side the light strikes), a 1px LITSOFT run down its left (the side the
     * light grazes), and a 1px CUT groove along its bottom. There is exactly
     * one shadow token in the system and it is drawn only on a plate being
     * dragged and under the three objects genuinely off the plane.
     *
     * WHY THE LADDER WAS WIDENED, which is the one structural decision. The
     * parent design ran its four surface steps at 1.073 / 1.126 / 1.089 /
     * 1.077 : 1 - every one below perceptual threshold - and spent its whole
     * separation budget on those 1px runs. PRESSWORK keeps the ground
     * (ZD_BASE, unchanged) and widens the steps until the smallest is
     * 1.1682:1, specifically so that a KNOCKOUT has a rung to live on. The
     * widening is asymmetric because the arithmetic is: below ZD_BASE there is
     * only 1.5105:1 of room in total, above it there is 13.9030:1.
     *
     * WHAT THE KNOCKOUT IS AND WHAT IT COSTS. The focused window's header
     * inverts to a solid light plate (theme.knock) with its title reversed out
     * (theme.knock_ink). That IS the focus signal - there is no ring, no glow
     * and no coloured bar. It is 6.4796:1 on ZD_BASE, 46.6% of the available
     * upward ceiling; on the un-widened ladder the same move would have landed
     * at 11.9231:1, 92% louder, and that is the failure the widening avoids.
     * Its price is paid in three places and none of them is hidden:
     *   1  Subtlety is gone. The 1px runs now CONFIRM a plate rather than
     *      CONSTITUTE it.
     *   2  The ink ramp lost a rung - four, not five. theme.text_5 and
     *      theme.text_6 therefore land on the SAME colour, and the collapse
     *      ui.h warns about is real here. It is the bill, not a slip: at
     *      ZD_FLOAT the 4.5:1 floor sits at L* 72.4 and the fifth rung no
     *      longer cleared it. Anything dimmer is theme.surf_7 / ZD_TEXT_INERT,
     *      which is STRUCTURE ONLY and must never carry a glyph.
     *   3  There is no room left below the ground, ever.
     *
     * LIGHT MODE IS DELIBERATELY OUT OF SCOPE. The prototype carries a second
     * ladder and its own verifier reports it unfinished - on paper the struck
     * run computes 1.244:1 on the ground, so the headroom the widening depends
     * on does not exist there. zlOS ships the dark ladder only.
     *
     * PRESERVED, EXPLICITLY, because none of the above touches them: subpixel
     * LCD text, gamma-correct blending in linear light, dithered gradients,
     * anti-aliased rounded corners, and geometric icons all stay exactly as
     * they are. This is a token change, not a rendering change; fb.c is not
     * edited and its primitives are the vocabulary PRESSWORK is written in.
     *
     * Mapping notes, where a role is not a straight one-to-one:
     *
     *  - border is ZD_CUT, not a light grey. The prototype draws every plate
     *    as `1px solid var(--zd-cut)` - the boundary is the groove. The LIGHT
     *    boundary, theme.edge_over, exists only under overlap: a darker edge
     *    tops out at 1.4723:1 on ZD_BASE, so it cannot clear the 3:1 floor the
     *    "which plate is on top" question needs, and the occluder has to draw
     *    a lighter one. 3.4322:1 at worst.
     *  - title, title_bot, title_off and title_off_bot are ALL ZD_BASE. That
     *    is not four copies of a missing decision: at rest the header is the
     *    plate, and focus is the knockout. Until wm.c inverts the header, a
     *    focused window is under-signalled rather than mis-signalled, which is
     *    the safe direction - painting theme.knock here instead would put
     *    light-on-light text on the focused window.
     *  - accent is the OVERPRINT (ZD_VERM), not an all-purpose highlight.
     *    PRESSWORK carries two inks and splits the old accent's work between
     *    them: vermilion is an ink laid ON a surface for the ONE thing to act
     *    on (focus bar, one primary button per view, crop marks, the datum
     *    mark) and theme.steel is the machine's own reading, used inside
     *    instruments - meters, spark bars, plot lines - and nowhere else.
     *    Never a control, never focus, never a border.
     *  - bar_hi is ZD_LIT because the shell's bands are separated by a 2px
     *    struck rule, not by a fill: the strip has one under it and the foot
     *    has one over it.
     *  - ok / warn / danger are wired to STATE and never to the accent. ZD_BAD
     *    is 4.2591:1, so it is a fill and a mark; failure TEXT is ZD_BAD_INK.
     */
    theme.bg        = ZD_VOID;        /* the desk behind everything          */
    theme.panel     = ZD_BASE;        /* THE PLATE - the window interior     */
    theme.panel_hi  = ZD_RAISE;       /* raised on a plate, and the rail     */
    theme.text      = ZD_TEXT_1;      /* the desktop's root ink              */
    theme.text_dim  = ZD_TEXT_3;      /* labels, captions - the last rung    */
    theme.accent    = ZD_VERM;        /* THE OVERPRINT. four jobs, no more.  */
    theme.border    = ZD_CUT;         /* the groove. the plate's own ring.   */
    theme.danger    = ZD_BAD;         /* failure fills and the close hover   */
    theme.title     = ZD_BASE;        /* the header at rest IS the plate     */
    theme.title_bot = ZD_BASE;        /* flat. the runs carry the depth.     */
    theme.title_off = ZD_BASE;        /* identical, on purpose - see above   */
    theme.title_off_bot = ZD_BASE;
    /* THE DESK IS ONE CALL, not nine layers: a single raking light entering
     * off-screen upper left and falling to ZD_VOID at the lower right. These
     * two are that glow flattened to a gradient's ends, so kernel.zl can draw
     * it with no colour of its own. The ruled module grid on top of it is
     * theme.grid. */
    theme.wallpaper_top = ZD_WALL_0;
    theme.wallpaper_bot = ZD_WALL_100;
    theme.bar_top   = ZD_RAISE;       /* the rail and the bands, flat        */
    theme.bar_bot   = ZD_RAISE;
    theme.bar_hi    = ZD_LIT;         /* the 2px struck rule between bands   */
    theme.chrome    = ZD_RAISE;
    theme.chrome_line = ZD_CUT;       /* every hairline is the groove        */
    theme.text_hi   = ZD_TEXT_0;      /* emphasis, above body                */
    theme.ok        = ZD_OK;
    /* the nine that let an app say what it means - see ui.h. text_5 and
     * text_6 share a value; that is the widening's bill, explained above. */
    theme.text_2    = ZD_TEXT_2;
    theme.text_5    = ZD_TEXT_3;
    theme.text_6    = ZD_TEXT_3;
    theme.warn      = ZD_WARN;
    theme.surf_1    = ZD_WELL;
    theme.surf_5    = ZD_FLOAT;       /* menus, toasts - off the plane       */
    theme.surf_7    = ZD_TEXT_INERT;  /* STRUCTURE ONLY. never a glyph.      */
    theme.surf_well = ZD_WELL;
    theme.accent_br = ZD_VERM_BR;

    /* ---- PRESSWORK's own roles, indices 30..41 ----------------------------
     * The grammar the twenty-nine above cannot express: a boundary is a colour,
     * and the focused header is a value inversion. Assigned in struct order,
     * because that order is the ABI ui_color() indexes. */
    theme.cut        = ZD_CUT;        /* 30 */
    theme.lit        = ZD_LIT;        /* 31  2.5423:1 on ZD_BASE            */
    theme.litsoft    = ZD_LITSOFT;    /* 32  the grazed left run            */
    theme.edge_over  = ZD_EDGE_OVER;  /* 33  under overlap only             */
    /* THE FOUR KNOCKOUT ROLES MOVE TOGETHER OR NOT AT ALL. Switching the fill
     * back to the wash without switching the two inks with it would leave
     * ZD_KNOCK_INK - a near-black chosen to reverse OUT of a light plate - on
     * a surface one rung above the ground, at 1.29:1. The fallback's inks are
     * the ordinary ramp precisely because its surface is an ordinary surface
     * again, and ko_edge goes back to the groove because on the wash the
     * headroom is upward again and the foot is a cut, not a run. This mirrors
     * the prototype's `body.nokock` block rule for rule. */
    if (knock_off) {
        theme.knock      = ZD_FOCUS_WASH; /* 34  1.3681:1 on ZD_BASE        */
        theme.knock_ink  = ZD_TEXT_0;     /* 35  the title, not reversed    */
        theme.knock_ink2 = ZD_TEXT_3;     /* 36  the label rung             */
        theme.ko_edge    = ZD_CUT;        /* 37  the groove is back below   */
    } else {
        theme.knock      = ZD_KNOCK;      /* 34  6.4796:1 on ZD_BASE        */
        theme.knock_ink  = ZD_KNOCK_INK;  /* 35  8.5329:1 on the knockout   */
        theme.knock_ink2 = ZD_KNOCK_INK2; /* 36  4.6965:1 on the knockout   */
        theme.ko_edge    = ZD_KO_EDGE;    /* 37  2.5487:1, 0.25% off lit    */
    }
    theme.grid       = ZD_GRID;       /* 38  1.6470:1 on ZD_VOID            */
    theme.steel      = ZD_STEEL;      /* 39  instruments only               */
    theme.steel_br   = ZD_STEEL_BR;   /* 40 */
    theme.ink_on     = ZD_INK_ON;     /* 41  6.1400:1 on ZD_VERM            */
    theme.bad_ink    = ZD_BAD_INK;    /* 42  failure TEXT, 5.9093:1         */

    /* ---- metrics ------------------------------------------------------------
     * Read off the prototype's stylesheet, not picked by eye and not snapped:
     *
     *   pad 10   gap 8   row 26   radius 9 (ZD_R_PLATE)   title 28
     *
     * THE RADIUS IS THE ONE THAT CHANGES HOW IT READS, and it went the other
     * way from the previous design. At 16 a window is a soft rounded card; at
     * 9 it is a plate with a machined corner, and a plate is what PRESSWORK
     * says every window is. Radius here encodes HOW MUCH THE OBJECT CAN MOVE,
     * and nesting halves it: 9 for a window, 4 for something inset in one, 2
     * for a chip, 0 for anything bolted down (a maximised window, the rail,
     * the strip, the foot). The inner rrect stays exactly one pixel tighter so
     * the groove still follows the outer curve instead of cutting across it.
     *
     * THE HEADER IS 28, DOWN FROM 36, and that is what makes the knockout
     * affordable: it is a solid light plate, and a solid light plate 36px tall
     * on every focused window is a different design. 28 with a tracked
     * uppercase title is a caption bar on a technical drawing.
     *
     * These are NOT on the 4/8/12/16/24 step scale, and that is deliberate
     * rather than an oversight. UI_S1..UI_S6 remain that scale and remain what
     * widget-internal spacing uses; the five values here are the prototype's
     * own measurements and snapping them would move every plate off the module
     * grid the desk is ruled with. Everything still goes through dp()/UI_DP(),
     * so the whole system scales as one.
     */
    theme.scale_q8 = scale_q8;
    theme.scale   = (scale_q8 + 128) >> 8;
    if (theme.scale < 1) theme.scale = 1;
    theme.pad     = dp(ZD_PAD,      scale_q8);
    theme.gap     = dp(ZD_GAP,      scale_q8);
    theme.row_h   = dp(ZD_ROW_H,    scale_q8);
    theme.radius  = dp(ZD_R_PLATE,  scale_q8);
    theme.title_h = dp(ZD_TITLE_H,  scale_q8);
    /* the shell's fixed furniture. Ruled once; it does not reflow. */
    theme.focus_bar = dp(fbar_dp, scale_q8);
    theme.rail_w    = dp(ZD_RAIL_W,    scale_q8);
    theme.strip_h   = dp(ZD_STRIP_H,   scale_q8);
    theme.foot_h    = dp(ZD_FOOT_H,    scale_q8);
    theme.band_h    = dp(ZD_BAND_H,    scale_q8);
}

void ui_theme_init(int scale) { ui_theme_init_q8(scale * 256); }

unsigned ui_color(int role)
{
    const unsigned *first = &theme.bg;
    if ((unsigned)role >= UI_COLOR_COUNT) return theme.danger;
    return first[role];
}

int ui_metric(int role)
{
    switch (role) {
    case UI_METRIC_PAD: return theme.pad;
    case UI_METRIC_GAP: return theme.gap;
    case UI_METRIC_ROW_H: return theme.row_h;
    case UI_METRIC_RADIUS: return theme.radius;
    case UI_METRIC_TITLE_H: return theme.title_h;
    case UI_METRIC_SCALE_Q8: return theme.scale_q8;
    case UI_METRIC_FOCUS_BAR: return theme.focus_bar;
    case UI_METRIC_RAIL_W: return theme.rail_w;
    case UI_METRIC_STRIP_H: return theme.strip_h;
    case UI_METRIC_FOOT_H: return theme.foot_h;
    case UI_METRIC_BAND_H: return theme.band_h;
    default: return 0;
    }
}


/* ============================================================================
 * THE CONTRAST ENGINE, IN INTEGERS, BECAUSE THE SETTINGS PANE ARGUES RATHER
 * THAN ASSERTS.
 *
 * The prototype's settings pane recomputes every ratio it prints from the live
 * custom properties - that is what makes it a control surface and not a page
 * of prose, and it is the property design.h's header tells you to re-derive
 * rather than trust. Reproducing it here needs WCAG relative luminance, and
 * WCAG relative luminance needs the sRGB transfer function, which is a power
 * of 2.4. THERE IS NO FLOATING POINT IN THE DRAWING PATH and there is not
 * going to be one, so the curve is a table.
 *
 * SRGB_LIN[c] is the linear-light value of channel byte c, in units of 1e-7,
 * so SRGB_LIN[255] is exactly 10,000,000. Generated from
 *
 *     c/255 <= 0.03928 ? (c/255)/12.92 : ((c/255 + 0.055)/1.055)^2.4
 *
 * rounded to nearest. 256 entries x 4 bytes = 1 KiB of .rodata, which buys
 * every ratio in the system with no heap, no float and no per-call cost.
 *
 * ACCURACY IS MEASURED, NOT ASSUMED. Against IEEE double over all 900 ordered
 * pairs of the 30 tokens in design.h the worst absolute error is 6.2e-5, i.e.
 * the fourth decimal is right or off by one in its last place. Every figure
 * this file's callers print at four decimals - 6.4796 knock on base, 2.5487
 * ko_edge on knock, 2.5423 lit on base, 4.6319 verm on base, 1.4723 cut on
 * base, 8.5329 and 4.6965 the two inks on the knockout - reproduces design.h
 * exactly. The rounding, not truncation, in both divisions below is what buys
 * that last decimal; truncating cost up to 1e-4 and lost three of them.
 *
 * WHY x10^4 OUT. zl numbers are doubles but zl has no formatting, so a ratio
 * crosses the seam as an integer and the zl side prints it with a fixed-point
 * helper. Four decimals is what the prototype prints at its widest (r4), so
 * that is the fixed point, and a caller wanting fewer just divides.
 */
static const unsigned SRGB_LIN[256] = {
           0,     3035,     6071,     9106,    12141,    15176,    18212,    21247,
       24282,    27317,    30353,    33465,    36765,    40247,    43914,    47770,
       51815,    56054,    60488,    65121,    69954,    74990,    80232,    85681,
       91341,    97212,   103298,   109601,   116122,   122865,   129830,   137021,
      144438,   152085,   159963,   168074,   176420,   185002,   193824,   202886,
      212190,   221739,   231534,   241576,   251869,   262412,   273209,   284260,
      295568,   307134,   318960,   331048,   343398,   356013,   368895,   382044,
      395462,   409152,   423114,   437350,   451862,   466651,   481718,   497066,
      512695,   528606,   544803,   561285,   578054,   595112,   612461,   630100,
      648033,   666259,   684782,   703601,   722719,   742136,   761854,   781874,
      802198,   822827,   843762,   865005,   886556,   908417,   930590,   953075,
      975873,   998987,  1022417,  1046165,  1070231,  1094617,  1119324,  1144354,
     1169707,  1195384,  1221388,  1247718,  1274377,  1301365,  1328683,  1356333,
     1384316,  1412633,  1441285,  1470273,  1499598,  1529262,  1559265,  1589608,
     1620294,  1651322,  1682694,  1714411,  1746474,  1778884,  1811642,  1844750,
     1878208,  1912017,  1946178,  1980693,  2015563,  2050787,  2086369,  2122308,
     2158605,  2195262,  2232280,  2269659,  2307400,  2345506,  2383976,  2422811,
     2462013,  2501583,  2541521,  2581829,  2622507,  2663556,  2704978,  2746773,
     2788943,  2831487,  2874408,  2917706,  2961383,  3005438,  3049873,  3094689,
     3139887,  3185468,  3231432,  3277781,  3324515,  3371636,  3419144,  3467041,
     3515326,  3564001,  3613068,  3662526,  3712377,  3762621,  3813260,  3864294,
     3915725,  3967552,  4019778,  4072402,  4125426,  4178851,  4232677,  4286905,
     4341536,  4396572,  4452012,  4507858,  4564110,  4620770,  4677838,  4735315,
     4793202,  4851499,  4910208,  4969330,  5028865,  5088813,  5149177,  5209956,
     5271151,  5332764,  5394795,  5457245,  5520114,  5583404,  5647115,  5711248,
     5775804,  5840784,  5906188,  5972018,  6038273,  6104956,  6172066,  6239604,
     6307571,  6375969,  6444797,  6514056,  6583748,  6653873,  6724432,  6795425,
     6866853,  6938718,  7011019,  7083758,  7156935,  7230551,  7304607,  7379104,
     7454042,  7529422,  7605245,  7681511,  7758222,  7835378,  7912979,  7991027,
     8069523,  8148466,  8227858,  8307699,  8387990,  8468732,  8549926,  8631572,
     8713671,  8796224,  8879231,  8962694,  9046612,  9130987,  9215819,  9301109,
     9386857,  9473065,  9559734,  9646862,  9734453,  9822506,  9911021, 10000000,
};

/* relative luminance x 1e-7. 0 .. 10,000,000. */
static unsigned ui_lum(unsigned rgb)
{
    unsigned long long y = 2126ULL * SRGB_LIN[(rgb >> 16) & 255]
                         + 7152ULL * SRGB_LIN[(rgb >>  8) & 255]
                         +  722ULL * SRGB_LIN[ rgb        & 255];
    return (unsigned)((y + 5000) / 10000);
}

/* CIE L* x100, without libm. The forward L*->Y function is monotone, so a
 * fourteen-step integer binary search gives the nearest hundredth over the
 * complete 0..100 range. Y uses ui_lum's x1e7 scale; the cubic branch stays
 * within u64 even at white (11600^3 * 1e7 < 2^64). */
static unsigned long long lum_from_lstar_q2(unsigned lq2)
{
    if (lq2 <= 800u)
        return ((unsigned long long)lq2 * 10000000ULL + 45165ULL) / 90330ULL;
    unsigned long long p = (unsigned long long)lq2 + 1600ULL;
    unsigned long long p3 = p * p * p;
    return (p3 * 10000000ULL + 780448000000ULL) / 1560896000000ULL;
}

unsigned ui_lstar_q2(unsigned rgb)
{
    unsigned long long y = ui_lum(rgb);
    unsigned lo = 0, hi = 10000;
    while (lo < hi) {
        unsigned mid = lo + (hi - lo + 1) / 2;
        if (lum_from_lstar_q2(mid) <= y) lo = mid;
        else hi = mid - 1;
    }
    if (lo < 10000) {
        unsigned long long yl = lum_from_lstar_q2(lo);
        unsigned long long yh = lum_from_lstar_q2(lo + 1);
        if (yh - y < y - yl) return lo + 1;
    }
    return lo;
}

/* WCAG contrast x 10^4. Order-independent: the ratio is defined lighter over
 * darker, so ui_ratio(a,b) == ui_ratio(b,a) and no caller has to know which of
 * its two colours is on top. 0.05 in these units is 500,000. */
unsigned ui_ratio_q4(unsigned a, unsigned b)
{
    unsigned long long ya = ui_lum(a) + 500000u;
    unsigned long long yb = ui_lum(b) + 500000u;
    if (ya < yb) { unsigned long long t = ya; ya = yb; yb = t; }
    return (unsigned)((ya * 10000ULL + yb / 2) / yb);
}

/* THE TWO CEILINGS, AND THEY ARE WHAT DECIDES WHICH SIDE AN EDGE GOES ON.
 * A surface has a fixed amount of room below it and a fixed amount above it,
 * set by the surface alone: the loudest DARKER term against it is the ratio to
 * pure black, and the loudest LIGHTER term is the ratio to pure white. That is
 * the arithmetic behind "the ground decides which direction has the headroom",
 * which is the rule the lit/cut grammar and the knockout's foot edge both
 * follow. The two endpoints are ZD_AXIS_BLACK / ZD_AXIS_WHITE in design.h -
 * NOT because they are design colours, they are not and nothing paints with
 * them, but because "a colour literal may appear in design.h and nowhere else"
 * is enforced by a scanner that counts six hex digits and does not take
 * intent as an argument. It caught them written here. See design.h. */
unsigned ui_ceil_dn_q4(unsigned rgb) { return ui_ratio_q4(rgb, ZD_AXIS_BLACK); }
unsigned ui_ceil_up_q4(unsigned rgb) { return ui_ratio_q4(ZD_AXIS_WHITE, rgb); }

/* ---- the comparison ladder, published as data -------------------------------
 * design.h's ZD_REF_* block, reachable by index so that kernel.zl can compute
 * against the parent designs without a colour literal of its own. The rule
 * "a colour literal may appear in design.h and nowhere else" has no exception
 * for a comparison, and a zl file that spelled #F2EDE4 would be the third
 * simultaneous palette this system has shipped.
 *
 * NOTHING PAINTS WITH THESE. They are arguments to ui_ratio_q4() and to
 * nothing else; ui_theme_init_q8 assigns none of them to a role. An
 * out-of-range index returns the ground rather than a colour, so the worst a
 * caller can do by miscounting is print 1.0000:1 - a figure obviously wrong
 * rather than plausibly wrong, which is the direction to fail in. */
unsigned ui_ref_color(int which)
{
    switch (which) {
    case UI_REF_LIT_RAKING:  return ZD_REF_LIT_RAKING;
    case UI_REF_BASE_RAKING: return ZD_REF_BASE_RAKING;
    case UI_REF_KNOCK_PLATE: return ZD_REF_KNOCK_PLATE;
    case UI_REF_WASH_RAKING: return ZD_REF_WASH_RAKING;
    default: return ZD_BASE;
    }
}

/* ...and the five figures the pane needs that are not colours. Published the
 * same way and for the same reason: kernel.zl duplicating 600, 420 or 28 would
 * be a second copy of a design.h number that drifts the first time one of them
 * moves, and this pane's whole claim is that its arithmetic is the design's
 * own. The one duplication kernel.zl already carries (RAIL_W_D and friends)
 * exists because ui_metric answers 0 before the theme is built and the shell
 * draws on paths where it is not; the settings pane has no such path - it
 * cannot be open before the compositor is up - so it takes no copy. */
int ui_ref_num(int which)
{
    switch (which) {
    case UI_REFN_PLATE_W:  return ZD_REF_PLATE_W;
    case UI_REFN_PLATE_H:  return ZD_REF_PLATE_H;
    case UI_REFN_TITLE_H:  return ZD_TITLE_H;      /* dp, not theme.title_h */
    case UI_REFN_RAKING_PX: return ZD_REF_RAKING_PX;
    case UI_REFN_GRAPHITE_PX: return ZD_REF_GRAPHITE_PX;
    case UI_REFN_WASH_Q4:  return ZD_REF_WASH_Q4;
    case UI_REFN_RAKING_Q4: return ZD_REF_RAKING_Q4;
    default: return 0;
    }
}

/* ---- the two switches ------------------------------------------------------
 * Both rebuild the theme rather than poking one field, because the knockout is
 * FOUR roles that have to move together (see ui_theme_init_q8) and because a
 * rebuild is the only path that cannot leave the struct half-updated. It costs
 * one pass over a struct, on a gesture, so the cheap version would be an
 * optimisation of something nobody is waiting for.
 *
 * NEITHER REPAINTS. A theme change invalidates every cached window surface in
 * the system and this layer does not know about surfaces - wm.c does. The
 * caller damages; kernel.zl's settings pane calls wm_damage_win_all() straight
 * after. Doing it here would make ui.c depend on the compositor and break the
 * layering ui.h states.
 */
/* THREE MORE SWITCHES THE PROTOTYPE HAS AND THIS PANE DID NOT.
 *
 * Each gates behaviour that already exists rather than adding any - which is
 * the only kind of control worth drawing. A switch wired to nothing is the
 * dead-control fault with a nicer surface.
 *
 *   over    the occlusion edge. `.win.over` in the reference; the chrome
 *           already computes it, and off falls back to the plain ring.
 *   motion  the animation timings. Off means every transition completes on the
 *           frame it starts, which is what a machine with no motion does.
 *   track   letter-spacing on the tracked faces. `body.notrack .t-big
 *           { letter-spacing: 0 }` is the reference's own switch for it.
 *
 * Like the knockout above, NONE of these repaints: the caller damages. */
static int over_off;
static int motion_off;
static int track_off;

int ui_over_get(void)     { return !over_off; }
int ui_over_set(int on)   { over_off   = on ? 0 : 1; return !over_off; }
int ui_motion_get(void)   { return !motion_off; }
int ui_motion_set(int on) { motion_off = on ? 0 : 1; return !motion_off; }
int ui_track_get(void)    { return !track_off; }
int ui_track_set(int on)  { track_off  = on ? 0 : 1; return !track_off; }

/* PER WINDOW TIMING, which wm.c could not see.
 *
 * proto:717-719 and proto:724 attach this to the status band -
 * `body.nous .sband .us { display: none }` - and the band is drawn in wm.c,
 * which had no way to read the mode at all: ui.c exported the knockout, the
 * occlusion edge, motion and tracking, and not this one. So two of the three
 * rungs were silent on the surface the authority attaches them to.
 *
 * 0 measured, 1 repaint, 2 off - the same three the Settings segment sets. */
static int us_mode;
int ui_us_get(void)       { return us_mode; }
int ui_us_set(int m)      { us_mode = (m < 0 || m > 2) ? 0 : m; return us_mode; }

int ui_knockout_get(void) { return !knock_off; }
int ui_knockout_set(int on)
{
    knock_off = on ? 0 : 1;
    ui_theme_init_q8(theme.scale_q8);
    return !knock_off;
}

/* dp, not pixels - see the note on fbar_dp. Clamped to the prototype's own
 * slider range: 1 is the thinnest run the framebuffer can draw and 6 is where
 * the bar stops reading as a register mark and starts reading as a panel. */
int ui_focus_bar_dp(void) { return fbar_dp; }
int ui_focus_bar_min_dp(void) { return UI_FBAR_MIN; }
int ui_focus_bar_max_dp(void) { return UI_FBAR_MAX; }
int ui_focus_bar_set(int n)
{
    if (n < UI_FBAR_MIN) n = UI_FBAR_MIN;
    if (n > UI_FBAR_MAX) n = UI_FBAR_MAX;
    fbar_dp = n;
    ui_theme_init_q8(theme.scale_q8);
    return fbar_dp;
}

/* ---- the layout cursor ---------------------------------------------------- */
static struct {
    int x, y, w, h;          /* the content box, inside the padding */
    int cx, cy;              /* where the next widget goes          */
    int row_h;               /* tallest thing in the current row    */
    int mode;                /* UI_DRAW or UI_HITTEST               */
    int px, py, click;       /* the pointer, and whether it is down */
    int fired;               /* which widget index fired, or -1     */
    int index;               /* widget counter, for identity        */
    int in_row;              /* 1 while ui_row() is open            */
} L;

/* ---- keyboard focus -------------------------------------------------------
 * Window focus was a title-bar hue and an accent underline; a focused CONTROL
 * had no indicator at all, and no way to move between controls without the
 * mouse.
 *
 * THE FOCUS INDEX LIVES HERE, NOT IN L. L is reset by every ui_begin, and
 * focus has to survive between passes or it would be forgotten between the
 * hit-test and the draw - and between frames, which is the entire point of it.
 * Which widget is focused is still the APP's to choose (ui_set_focus), the way
 * every other piece of widget state is; this only remembers the number.
 *
 * -1 is "nothing focused", and it is the default: a desktop that boots with a
 * ring already on some arbitrary control looks broken. */
static int focus_idx = -1;
static int focus_activate;      /* one-shot: the focused widget fires */
static int focus_count;         /* how many fired-capable widgets last pass */

void ui_set_focus(int idx)   { focus_idx = idx; }
int  ui_focus_get(void)      { return focus_idx; }
int  ui_widget_count(void)   { return focus_count; }
void ui_activate_focus(void) { focus_activate = 1; }

void ui_begin(int x, int y, int w, int h, int mode, int px, int py, int click)
{
    L.x = x + theme.pad;
    L.y = y + theme.pad;
    L.w = w - 2 * theme.pad;
    L.h = h - 2 * theme.pad;
    L.cx = L.x;
    L.cy = L.y;
    L.row_h = 0;
    L.mode = mode;
    L.px = px; L.py = py; L.click = click;
    L.fired = -1;
    L.index = 0;
    L.in_row = 0;
    focus_count = 0;      /* recounted every pass, so it follows the layout */
}

/* Consume the one-shot activation. Called by the app AFTER it has re-run its
 * widget sequence, so the flag covers exactly one pass and an Enter cannot
 * fire the same control again on the next repaint. */
void ui_end_activate(void) { focus_activate = 0; }

int ui_fired(void) { return L.fired; }

void ui_row(void)    { L.in_row = 1; }
void ui_endrow(void) { L.in_row = 0; L.cx = L.x; L.cy += L.row_h + theme.gap; L.row_h = 0; }

/* Place a widget of this size and return where it landed. The ONE function
 * that knows about the cursor; every widget below is a call to this plus
 * some drawing. */
static void place(int w, int h, int *ox, int *oy)
{
    if (w > L.w) w = L.w;
    if (L.in_row && L.cx + w > L.x + L.w) {     /* wrap */
        L.cx = L.x;
        L.cy += L.row_h + theme.gap;
        L.row_h = 0;
    }
    *ox = L.cx;
    *oy = L.cy;
    if (h > L.row_h) L.row_h = h;
    if (L.in_row) {
        L.cx += w + theme.gap;
    } else {
        L.cx = L.x;
        L.cy += h + theme.gap;
        L.row_h = 0;
    }
}

static int hit(int x, int y, int w, int h)
{
    return L.px >= x && L.px < x + w && L.py >= y && L.py < y + h;
}


/* Every widget that can fire funnels through this, so "did it fire" is decided
 * in exactly one place and cannot drift between widgets. In UI_HITTEST mode
 * nothing is drawn and this is the only thing that happens.
 *
 * The keyboard path goes through the SAME funnel, deliberately. A widget must
 * not be able to tell a click from an Enter - the moment it can, the two paths
 * drift and one of them grows a bug the other does not have. */
static int fire(int x, int y, int w, int h)
{
    int me = L.index++;
    if (me + 1 > focus_count) focus_count = me + 1;
    if (focus_activate && me == focus_idx) return 1;
    if (!L.click || !hit(x, y, w, h)) return 0;
    L.fired = me;
    return 1;
}

/* THE KEYBOARD MARK, drawn by each firing widget at the end of its own draw
 * block. L.index has already been advanced by fire(), so `L.index - 1` is the
 * calling widget's own id - which means this needs no argument and cannot be
 * passed the wrong one.
 *
 * IT IS A BAR, NOT A RING, AND THAT IS THE WHOLE POINT. This used to draw a
 * 1px vermilion box on all four sides of the control. PRESSWORK spends
 * vermilion as an OVERPRINT with exactly four jobs and a hard width rule: it
 * is laid on a surface as a rule or a mark, never as an area and never as an
 * outline. A ring around a 96 x 44 px button is 276 px of vermilion drawing a
 * shape that is not the shape of anything; the focus bar on the same control
 * is 6 x 44 and says the same thing in the same ink as the focused window's
 * own bar and the register mark on the rail. One idiom, three scales.
 *
 * design.h already said so - ZD_RING_FOCUS_A, ZD_RING_BLUR_A and ZD_RING_OFF_A
 * are all 0 in PRESSWORK - and this function ignored all three, which is why
 * the ring survived the token change. A token nobody reads is not a token.
 *
 * The bar sits just OUTSIDE the control's left edge, in the gap the layout
 * cursor already leaves, so it never covers the label and it lands on the same
 * x for every widget in a column - a keyboard walking down a form draws a mark
 * that steps rather than one that jumps. */
static void focus_ring(int x, int y, int w, int h)
{
    (void)w;
    if (L.mode != UI_DRAW || focus_idx < 0 || L.index - 1 != focus_idx) return;
    int bw = theme.focus_bar;
    if (bw < 1) bw = 1;
    int bx = x - bw - UI_S1(&theme) / 2;
    if (bx < L.x) bx = L.x;             /* never outside the client rect */
    fb_fill_px(bx, y, bw, h, theme.accent);
}

/* ---- what uikit.c is allowed to see ----------------------------------------
 * The second half of the toolkit (uikit.c: pills, tabs, grids, overlays) is a
 * separate file because this one is the LAYOUT CURSOR and that one is a
 * catalogue - but it must not get its own copy of "did it fire". So the funnel
 * above is published, and nothing else is: no access to L, no way to set
 * L.fired, no second focus index.
 *
 * These are deliberately thin. If a future widget needs more of L than this,
 * the widget belongs in this file, not behind a wider window into it. */
int  ui_mode_get(void)  { return L.mode; }
int  ui_click_get(void) { return L.click; }
int  ui_ptr_x(void)     { return L.px; }
int  ui_ptr_y(void)     { return L.py; }
int  ui_hit(int x, int y, int w, int h)  { return hit(x, y, w, h); }
void ui_place(int w, int h, int *x, int *y) { place(w, h, x, y); }
int  ui_fire(int x, int y, int w, int h) { return fire(x, y, w, h); }
void ui_ring(int x, int y, int w, int h) { focus_ring(x, y, w, h); }

/* ---- INK ON THE ACCENT, COMPUTED --------------------------------------------
 * docs/reference/ui/widgets.md S21.8: "INK must be computed, not stored. Four widgets
 * already got this wrong in the reference (S20.1)." The reference's own
 * derivation is at ds-reference.html 3039-3045: WCAG relative luminance of the
 * background, contrast-compared against black-ish 0.0034 and against white 1,
 * and the winner becomes the ink.
 *
 * That comparison reduces to a single threshold. With contrast defined as
 * (L1+.05)/(L2+.05), dark ink wins when
 *
 *     (L+.05)/(0.0034+.05)  >  1.05/(L+.05)
 *     (L+.05)^2             >  1.05 * 0.0534 = 0.05607
 *      L                    >  0.18679
 *
 * which is the standard sRGB "is this a light colour" line. So the whole
 * decision is one luminance and one compare - no per-widget opinion, and no
 * way for a widget to write #fff on the lime the way three reference widgets
 * do.
 *
 * NO FLOATING POINT. sRGB de-gamma is a 2.4 power, so it is a 17-entry table
 * of the curve at every 16th code value, linearly interpolated, in Q16. The
 * interpolation overestimates on the convex low end (lin(56) reads 2648 where
 * the true value is 1514) and that error is weighted 0.0722, so it moves the
 * final luminance by well under a percent - nowhere near the threshold for any
 * colour a palette would use. Asserted both directions in hosttest/uitest.c. */
static const unsigned short srgb_lin[17] = {
        0,   340,   947,  1937,  3360,  5256,  7666, 10618, 14151,
    18286, 23042, 28448, 34537, 41333, 48853, 57104, 65535
};

static unsigned lin_q16(unsigned c)
{
    unsigned i = (c >> 4) & 15u, f = c & 15u;
    unsigned a = srgb_lin[i], b = srgb_lin[i + 1];
    return a + (b - a) * f / 16u;
}

unsigned ui_luminance_q16(unsigned rgb)
{
    unsigned r = lin_q16((rgb >> 16) & 0xFFu);
    unsigned g = lin_q16((rgb >>  8) & 0xFFu);
    unsigned b = lin_q16( rgb        & 0xFFu);
    return (2126u * r + 7152u * g + 722u * b) / 10000u;
}

unsigned ui_ink_on(unsigned bg)
{
    return ui_luminance_q16(bg) > 12242u ? (unsigned)ZD_INK_DARK
                                         : (unsigned)ZD_INK_LIGHT;
}

/* ---- WHAT TYPE IS ACTUALLY IN THIS IMAGE ------------------------------------
 * THE TYPE APP MUST NOT TYPE ITS OWN FACTS IN. Every number below is read off
 * the atlas arrays themselves with sizeof, so a regenerated font moves the
 * table and a font dropped from kernel/SOURCES stops the link rather than
 * printing a row about something that is not there.
 *
 * WHAT THE PROTOTYPE GOT WRONG, and the pane says so on screen. Its TYPE table
 * has three rows - 8x16, 16x32 and 24x48 - and design.h repeats the claim as a
 * "hard platform fact". Two of the three are true. font_big.c does hold a
 * 24x48 coverage atlas, but it IS NOT IN kernel/SOURCES, so it is not linked
 * into any target; fb.c's own note at the fb_text_aa2x block says why - "the
 * 24x48 huge font was removed - unused, and its 109 KiB pushed the raw kernel
 * past the 640 KiB low-memory limit our own bootloader loads into". The LG
 * register is drawn by resampling the 16x32 coverage atlas, not from a baked
 * 24x48 one. That row is carried here with a null pointer and reported as
 * ABSENT rather than quietly dropped, because "the design asked for three and
 * the image has two" is the fact worth putting on a screen.
 *
 * AND THERE ARE SIX MORE THAN THE PROTOTYPE COUNTS. The mono coverage atlases
 * are the CONSOLE's; every label, title, row and column head in the desktop is
 * drawn by fb_text_role out of the PROPORTIONAL atlases in font_prop.inc -
 * three cells at two weights. A type pane that listed only the console's font
 * would be describing a face the desktop hardly uses.
 *
 * Only the linked arrays are externed. Naming font24x48_aa here would make the
 * link fail, which is the opposite of what this row is for.
 */
extern const unsigned char font8x16_aa[95][16][8];
extern const unsigned char font16x32_aa[95][32][16];
extern const unsigned char prop16[95][16][16];
extern const unsigned char prop16b[95][16][18];
extern const unsigned char prop24[95][24][22];
extern const unsigned char prop24b[95][24][24];
extern const unsigned char prop32[95][32][30];
extern const unsigned char prop32b[95][32][33];

#define UI_FACE_MONO 0      /* DejaVu Sans Mono, fixed cell - the console   */
#define UI_FACE_SANS 1      /* DejaVu Sans, proportional - the whole desktop */
#define UI_FACE_SANSB 2     /* the same, bold                                */

/* A[] IS DERIVED, NOT DECLARED. w is the array's own last dimension, h its
 * middle one and n its first, so the only thing written by hand per row is
 * which array it is and which face it belongs to. `in` is 0 for the one row
 * whose atlas is not in this image. */
#define UI_ATLAS_ROW(a, face) \
    { (int)(sizeof (a)[0][0]), (int)(sizeof (a)[0] / sizeof (a)[0][0]), \
      (int)(sizeof (a) / sizeof (a)[0]), (face), 1 }

static const struct { int w, h, n, face, in; } ui_atlases[] = {
    UI_ATLAS_ROW(font8x16_aa,  UI_FACE_MONO),
    UI_ATLAS_ROW(font16x32_aa, UI_FACE_MONO),
    { 24, 48, 95, UI_FACE_MONO, 0 },     /* font_big.c - NOT in SOURCES     */
    UI_ATLAS_ROW(prop16,       UI_FACE_SANS),
    UI_ATLAS_ROW(prop16b,      UI_FACE_SANSB),
    UI_ATLAS_ROW(prop24,       UI_FACE_SANS),
    UI_ATLAS_ROW(prop24b,      UI_FACE_SANSB),
    UI_ATLAS_ROW(prop32,       UI_FACE_SANS),
    UI_ATLAS_ROW(prop32b,      UI_FACE_SANSB),
};
#define UI_ATLAS_N ((int)(sizeof ui_atlases / sizeof ui_atlases[0]))

static int atlas_ok(int i) { return i >= 0 && i < UI_ATLAS_N; }

int ui_atlas_n(void)        { return UI_ATLAS_N; }
int ui_atlas_w(int i)       { return atlas_ok(i) ? ui_atlases[i].w    : 0; }
int ui_atlas_h(int i)       { return atlas_ok(i) ? ui_atlases[i].h    : 0; }
int ui_atlas_glyphs(int i)  { return atlas_ok(i) ? ui_atlases[i].n    : 0; }
int ui_atlas_face(int i)    { return atlas_ok(i) ? ui_atlases[i].face : -1; }
int ui_atlas_in_image(int i){ return atlas_ok(i) ? ui_atlases[i].in   : 0; }

/* WHICH ATLAS A TYPE REGISTER IS DRAWING FROM RIGHT NOW, at whatever weight
 * the caller asks for. This is a LIVE answer and it moves with the UI scale:
 * fb_text_role_h() returns the exact pixel height the role resolves to, and
 * the atlas is then the nearest proportional cell to it.
 *
 * "NEAREST BY MIDPOINT" IS fb.c's OWN RULE, RE-DERIVED RATHER THAN COPIED.
 * prop_atlas_cell() there is written as `<=20 -> 16, <=28 -> 24, else 32`, and
 * 20 and 28 are the midpoints of 16/24 and 24/32. Restating those three
 * constants here would be a second copy that silently stops matching the day a
 * fourth atlas is generated; searching this table for the smallest |cell -
 * wanted| gives the same answer today and follows the table tomorrow.
 * hosttest/uitest.c asserts the two agree for every role at every shipped
 * scale, which is the only thing that makes that claim worth anything.
 *
 * Returns an index into the table above, or -1 if no proportional atlas is
 * linked at all. */
int fb_text_role_h(int role);

int ui_atlas_for_role(int role, int weight)
{
    int want = fb_text_role_h(role);
    int want_face = weight ? UI_FACE_SANSB : UI_FACE_SANS;
    int best = -1, best_d = 0;
    for (int i = 0; i < UI_ATLAS_N; i++) {
        if (!ui_atlases[i].in) continue;
        if (ui_atlases[i].face != want_face) continue;
        int d = ui_atlases[i].h - want;
        if (d < 0) d = -d;
        if (best < 0 || d < best_d) { best = i; best_d = d; }
    }
    return best;
}

/* A proportional layout cannot ask "length times cell" any more - it has to
 * MEASURE. That is the part of item 4 that touches every widget, and the
 * reason a toolkit needs one function for it rather than a multiply spread
 * across ten call sites. */
static int text_w(const char *s) { return fb_text_prop_w(s); }
static int text_h(void)          { return fb_text_prop_h(); }

/* ---- PRESSWORK'S DEPTH RECIPE, AND WHY IT LIVES HERE ------------------------
 * One raking light, entering off-screen upper left, that never moves. Every
 * object in the system is separated from its ground by the same four marks and
 * by nothing else: a 1px ring in the groove colour, a 1px STRUCK run along the
 * top (the side the light hits square), a 1px GRAZED run down the left, and a
 * 1px cut groove along the bottom. A pit is the same recipe with the sign
 * flipped - groove at the top, grazed run at the bottom, because the far wall
 * of a hole is the wall in shadow and the near wall is the one the light
 * reaches across the floor. There is no drop shadow at rest anywhere.
 *
 * This was written twice: uikit.c had `seat_face`/`run_top`/`run_bottom`/
 * `seat_raised`/`seat_sunken` as statics and ui.c had nothing, so ui.c's own
 * widgets - the button, the switch, the slider, the bar, the list row - stayed
 * on the predecessor's rounded-capsule idiom and were the majority of what a
 * rendered frame actually showed. Two copies of a depth grammar is two chances
 * to draw a run on the wrong edge, and the wrong edge means the lamp moved.
 *
 * So the recipe lives in ui.c, which is the lower layer and already owns the
 * palette, and it is published in ui.h. uikit.c's five statics are now one-line
 * forwarders onto these - the call sites there did not change, and there is
 * exactly one definition of where the light comes from.
 *
 * `r <= 0` (ZD_R_BOLT) takes fb_fill_px + fb_box rather than two fb_rrect
 * calls: at radius 0 they draw the same pixels and the box form is cheaper.
 * Above 0 it MUST be two fb_rrect calls, because that is what keeps the ring
 * and the face anti-aliased along the same arc - a fb_box ring around an
 * fb_rrect face would cut the corner square and lose the AA the whole system
 * depends on. */
void ui_seat_face(int x, int y, int w, int h, int r, unsigned face, unsigned ring)
{
    if (w <= 0 || h <= 0) return;
    if (r <= 0) {
        fb_fill_px(x, y, w, h, face);
        fb_box(x, y, w, h, ring);
        return;
    }
    fb_rrect(x, y, w, h, r, ring);
    if (w > 2 && h > 2) fb_rrect(x + 1, y + 1, w - 2, h - 2, r - 1, face);
}

/* Each run is 1px, sits INSIDE the ring, and stops where the arc starts - a
 * run that carried on into the corner would cross the curve instead of
 * following it. `edge` is the token, so the same two calls draw a raised
 * plate (ZD_LIT), a pressed one (ZD_CUT) and the primary action (ZD_VERM_BR). */
void ui_run_top(int x, int y, int w, int r, unsigned edge)
{
    int inset = r > 1 ? r : 1;
    if (w - 2 * inset > 0) fb_fill_px(x + inset, y + 1, w - 2 * inset, 1, edge);
}

void ui_run_bottom(int x, int y, int w, int h, int r, unsigned edge)
{
    int inset = r > 1 ? r : 1;
    if (h < 3) return;
    if (w - 2 * inset > 0) fb_fill_px(x + inset, y + h - 2, w - 2 * inset, 1, edge);
}

/* RAISED. `ring` is ZD_CUT at rest and ZD_EDGE_OVER under the pointer; `lit`
 * is 0 for a disabled control, which is how "this cannot be pressed" is said
 * in the design's own grammar rather than by greying the label. An unlit plate
 * is not raised, and a thing that is not raised cannot be pressed. */
void ui_seat_raised(int x, int y, int w, int h, int r, unsigned face,
                    unsigned ring, int lit)
{
    ui_seat_face(x, y, w, h, r, face, ring);
    if (lit) ui_run_top(x, y, w, r, theme.lit);
}

/* SUNKEN - the prototype's `.well`: `inset 0 1px 0 0 cut, inset 0 -1px 0 0
 * litsoft`. ZD_LITSOFT rather than ZD_LIT on the near wall, because the light
 * arrives there across the floor of the pit and not square on. */
void ui_seat_sunken(int x, int y, int w, int h, int r, unsigned face)
{
    ui_seat_face(x, y, w, h, r, face, theme.cut);
    ui_run_top(x, y, w, r, theme.cut);
    ui_run_bottom(x, y, w, h, r, theme.litsoft);
}

/* ---- THE INSTRUMENT TRACK, ONE SHAPE ---------------------------------------
 * `.mtrack` + `.mfill`: a pit with walls and a hard-edged fill inside them.
 * The meter, the progress bar and the slider are all this - the slider is two
 * design px taller and takes a click, and that is the whole difference. There
 * is no thumb: the fill's leading edge IS the readout, at a 1px ZD_STEEL /
 * ZD_WELL boundary, and a grabbable puck would have to be a raised object with
 * its own ring and run on a track with no room for either.
 *
 * SQUARE ENDS, ALWAYS (ZD_R_BOLT). A rounded end lies about where the value
 * stops, and an instrument that lies about its last few percent is worse than
 * no instrument. */
static void bar_track(int x, int y, int w, int h, int pct, unsigned fill)
{
    if (w <= 0 || h <= 0) return;
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    ui_seat_face(x, y, w, h, ZD_R_BOLT, theme.surf_1, theme.cut);
    /* inside the walls, so a full bar still reads as a bar in a box */
    int iw = w - 2, ih = h - 2;
    if (iw <= 0 || ih <= 0) return;
    int fw = iw * pct / 100;
    if (fw > 0) fb_fill_px(x + 1, y + 1, fw, ih, fill);
}

/* ---- the widgets ---------------------------------------------------------- */
void ui_label(const char *s)
{
    int x, y;
    place(text_w(s), text_h(), &x, &y);
    if (L.mode == UI_DRAW) fb_text_prop(x, y, s, theme.text);
}

void ui_label_dim(const char *s)
{
    int x, y;
    place(text_w(s), text_h(), &x, &y);
    if (L.mode == UI_DRAW) fb_text_prop(x, y, s, theme.text_dim);
}

/* A METER IN THE LAYOUT FLOW - the prototype's `.mtrack`, at the meter's own
 * height rather than at UI_S2. It was a rounded capsule of theme.panel_hi with
 * a rounded vermilion fill: two things wrong at once. The capsule made a
 * reading look like a control, and the fill spent the overprint on "how full
 * is the disk", which is exactly the job ZD_STEEL exists for. The two-ink
 * contract is that vermilion is the ONE thing to act on and steel is the
 * machine's own reading; a percentage is never something to act on. */
void ui_bar(int pct)
{
    int x, y, w = L.w, h = UI_DP(&theme, ZD_METER_H);
    if (h < 3) h = 3;
    place(w, h, &x, &y);
    if (L.mode != UI_DRAW) return;
    bar_track(x, y, w, h, pct, theme.steel);
}

/* THE BUTTON - the prototype's `.btn`. A raised plate on the plate: ZD_RAISE
 * face, 1px ZD_CUT ring, one struck ZD_LIT run along the top, ZD_R_CHIP.
 *
 * Three states and each is a different physical claim, which is what makes
 * them tellable apart without colour:
 *   rest   raised - ring is the groove, the run is lit
 *   hover  still raised, but the boundary lifts to ZD_EDGE_OVER and the ink
 *          goes up a rung. Nothing moves; the light just finds the edge.
 *   press  SEATED - the face drops to ZD_WELL and the top run flips to
 *          ZD_CUT. The run changing sign is the press: an object pushed into
 *          the plate has its lit edge in shadow.
 *
 * What it is NOT any more is "accent fill on press". Filling a button with the
 * overprint on mouse-down made every button in the system the one primary
 * action for as long as a finger was down. The primary action is a separate
 * kind (ui_pill's UI_BTN_PRIMARY) and there is one per view. */
int ui_button(const char *s)
{
    int px = UI_DP(&theme, ZD_BUTTON_PX);
    int w = text_w(s) + 2 * px, h = UI_DP(&theme, ZD_BUTTON_H);
    int x, y;
    place(w, h, &x, &y);
    int over = hit(x, y, w, h);
    int fired = fire(x, y, w, h);
    if (L.mode == UI_DRAW) {
        int down = over && L.click;
        int r = UI_DP(&theme, ZD_R_CHIP);
        if (down) {
            ui_seat_face(x, y, w, h, r, theme.surf_1, theme.cut);
            ui_run_top(x, y, w, r, theme.cut);
        } else {
            ui_seat_raised(x, y, w, h, r, theme.panel_hi,
                           over ? theme.edge_over : theme.cut, 1);
        }
        fb_text_prop(x + px, y + (h - text_h()) / 2, s,
                     over ? theme.text_hi : theme.text_2);
        focus_ring(x, y, w, h);
    }
    return fired;
}

void ui_sep(void)
{
    int x, y;
    place(L.w, 1, &x, &y);
    if (L.mode == UI_DRAW) fb_fill_px(x, y, L.w, 1, theme.border);
}

void ui_space(int n)
{
    int x, y;
    place(1, n > 0 ? n : theme.gap, &x, &y);
    (void)x; (void)y;
}

/* The app owns the settled boolean. This tiny table retains only the transient
 * edge needed to interpolate the authority's RISE transition; pointer identity
 * is stable because every shipped switch state is a global/static integer. */
#define TOGGLE_ANIM_N 16
struct toggle_anim_state { int *key; };
static struct toggle_anim_state toggle_anim[TOGGLE_ANIM_N];

static int toggle_anim_slot(int *key)
{
    for (int i = 0; i < TOGGLE_ANIM_N; i++)
        if (toggle_anim[i].key == key) return i;
    for (int i = 0; i < TOGGLE_ANIM_N; i++)
        if (!toggle_anim[i].key) { toggle_anim[i].key = key; return i; }
    return 0;
}

int ui_toggle(const char *s, int *on)
{
    /* A SWITCH IS A BOLTED RECTANGLE, NOT A PILL - the prototype's `.sw2`.
     * The predecessor drew a 40x20 capsule with a round white puck in it and
     * an asymmetric 3px travel, because it was ported from a stylesheet where
     * a switch is a physical rocker. PRESSWORK has no round objects and no
     * white: ZD_R_BOLT, because a switch cannot move on the plate - it only
     * changes state - and radius in this system encodes exactly how much an
     * object can move.
     *
     * OFF is a pit: ZD_WELL floor, ZD_CUT ring, and a ZD_TEXT_INERT knob. That
     * is the one place ZD_TEXT_INERT is allowed, and it is allowed because a
     * knob is structure and not a glyph - it is the same permission the resize
     * grip and the scrollbar thumb hold.
     *
     * ON IS THE KNOCKOUT, not the accent. This is the design's single focus /
     * selection / active gesture, at a third scale: the focused window header
     * inverts, the selected table row inverts, and so does an engaged switch.
     * Filling it with vermilion instead would put a fifth job on an overprint
     * that has four, and would make every switched-on preference compete with
     * the one action in the view.
     *
     * 34 - 13 - 2 == 19, so the travel is SYMMETRIC and 1px of pit shows on
     * each side in both states. The old asymmetry was the reference's, and the
     * reference is not this design. */
    int kw = UI_DP(&theme, ZD_SW_W), kh = UI_DP(&theme, ZD_SW_H);
    int pad = UI_DP(&theme, ZD_SW_INSET), d = UI_DP(&theme, ZD_SW_KNOB);
    int w = kw + theme.gap + text_w(s), h = theme.row_h;
    int x, y;
    if (pad < 1) pad = 1;
    if (kh > h) h = kh;
    place(w, h, &x, &y);
    int fired = fire(x, y, w, h);
    int ty = y + (h - kh) / 2;
    int as = toggle_anim_slot(on);
    int aid = WM_FX_USER - 96 - as;
    if (fired) {
        *on = !*on;
        wm_anim_at(aid, ANIM_FOCUS, x, ty, kw, kh);
    }
    if (L.mode == UI_DRAW) {
        int r  = UI_DP(&theme, ZD_SW_R);
        if (*on) ui_seat_face(x, ty, kw, kh, r, theme.knock, theme.knock);
        else     ui_seat_sunken(x, ty, kw, kh, r, theme.surf_1);
        int offx = x + pad, onx = x + kw - pad - d;
        int kx = *on ? onx : offx;
        int p = wm_anim_progress(aid, ANIM_FOCUS);
        if (p >= 0) {
            if (*on) kx = offx + (onx - offx) * p / 1000;
            else     kx = onx - (onx - offx) * p / 1000;
        }
        fb_fill_px(kx, ty + pad, d, d, *on ? theme.knock_ink : theme.surf_7);
        fb_text_prop(x + kw + theme.gap, y + (h - text_h()) / 2, s, theme.text);
        focus_ring(x, y, w, h);
    }
    return fired;
}

int ui_toggle_value(const char *s, int on)
{
    int v = on ? 1 : 0;
    ui_toggle(s, &v);
    return v;
}

/* The slider is what PROVES wm.c's pointer grab: once pressed it must keep
 * tracking after the pointer leaves its rectangle, which only works because
 * the window that owns the grab keeps receiving the events. */
int ui_slider(int *v, int lo, int hi)
{
    int w = L.w, h = theme.row_h;
    int x, y;
    place(w, h, &x, &y);
    int fired = fire(x, y, w, h);
    if (hi <= lo) hi = lo + 1;
    if (fired) {
        /* THE SPAN IS THE NUMBER OF VALUES, NOT THE GAP BETWEEN THE ENDS.
         *
         * This was (hi - lo), which divides the track into (hi - lo) buckets
         * for (hi - lo + 1) values - so the last value is only reachable at
         * px == x + w exactly, one pixel PAST the track's last pixel. The
         * maximum could not be selected at all, and both clamps below were
         * dead code because the expression could never exceed hi.
         *
         * Found by driving the Settings app's own scale slider (1..3) from a
         * harness: clicking the rightmost pixel of a 388 px track gave
         * 387*2/388 + 1 = 2. Every slider in the toolkit had it, so pointer
         * speed could not reach 400 either.
         *
         * With (hi - lo + 1) the track divides into equal buckets per value,
         * the last pixel selects hi, and the clamp catches the genuine
         * px == x + w case that a pointer grab can produce mid-drag. */
        int span = hi - lo + 1;
        int t = (L.px - x) * span / (w ? w : 1) + lo;
        if (t < lo) t = lo;
        if (t > hi) t = hi;
        *v = t;
    }
    if (L.mode == UI_DRAW) {
        /* THE SLIDER IS AN INSTRUMENT TRACK WITH NO THUMB, which is what the
         * prototype draws: `.mtrack` at 11dp, a ZD_STEEL fill, and the fill's
         * leading edge is the readout. The predecessor had a 4dp capsule with
         * a 13dp round vermilion puck riding on it - a puck is a raised object
         * and a raised object needs a ring, a run and a radius that a 4dp
         * track has no room for, so it was drawn as a flat disc and read as a
         * bubble floating over a wire.
         *
         * The hard ZD_STEEL / ZD_WELL boundary is 8.3765:1 and one pixel wide,
         * which locates the value more precisely than a 13dp puck centred on
         * the same coordinate ever did. The whole control is grabbable, so
         * losing the puck loses no target area - the pointer grab in wm.c is
         * what keeps a drag alive once it leaves the rect, not the thumb. */
        int track = UI_DP(&theme, ZD_SLIDER_H);
        if (track < 3) track = 3;
        int ty = y + (h - track) / 2;
        bar_track(x, ty, w, track, (*v - lo) * 100 / (hi - lo), theme.steel);
        focus_ring(x, y, w, h);
    }
    return fired;
}

int ui_slider_value(int v, int lo, int hi)
{
    ui_slider(&v, lo, hi);
    return v;
}

/* A number with a label, right-aligned - the System Monitor's readouts. No
 * itoa in this kernel, so it is built here, digits backwards into a buffer. */
void ui_num(const char *s, int v)
{
    char buf[16];
    int n = 0, neg = v < 0;
    unsigned u = neg ? (unsigned)(-v) : (unsigned)v;
    if (!u) buf[n++] = '0';
    while (u && n < 12) { buf[n++] = (char)('0' + u % 10u); u /= 10u; }
    if (neg && n < 15) buf[n++] = '-';
    for (int i = 0; i < n / 2; i++) {
        char t = buf[i]; buf[i] = buf[n - 1 - i]; buf[n - 1 - i] = t;
    }
    buf[n] = 0;

    int x, y;
    place(L.w, text_h(), &x, &y);
    if (L.mode != UI_DRAW) return;
    fb_text_prop(x, y, s, theme.text_dim);
    fb_text_prop(x + L.w - text_w(buf), y, buf, theme.text);
}

/* ---- ui_list_row and ui_scroll --------------------------------------------
 * Both were held back until clipping was settled, and for a real reason: a
 * scrolled list draws rows that are PARTLY outside their own viewport, so
 * without a scissor the overflow lands on whatever is next to it. fb_clip is
 * now proven - zero pixels escape a set scissor across every primitive - so
 * these are safe to build.
 *
 * They are still immediate mode. A list does not own its items and there is no
 * item array anywhere: the app calls ui_list_row once per thing it has, in a
 * loop it already writes, and the row returns whether it was clicked. That is
 * the only shape available without a heap, and it is also the nicer one.
 */
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);
void fb_clip_get(int *x0, int *y0, int *x1, int *y1);

static struct {
    int on;                  /* inside a scroll region?                    */
    int x, y, w, h;          /* the viewport, in screen coordinates        */
    int off;                 /* how far down the content is scrolled       */
    int content;             /* total content height, measured as we go    */
    int save_cy, save_h;     /* the cursor state ui_scroll_end restores     */
    int cx0, cy0, cx1, cy1;  /* the scissor to put back                     */
} S;

/* ---- THE REGISTER MARK - one of PRESSWORK's TWO row idioms -----------------
 * The predecessor had three treatments for a selected row and picked one: a
 * 15% accent wash across the whole row plus a left bar. PRESSWORK deletes the
 * wash, and not for taste. Vermilion is an OVERPRINT with a width rule - it is
 * laid on a surface as a rule or a mark, never as an area - and a 15% tint
 * across a 400 x 26 px row is 10,400 px of it. The bar it sat behind is 78.
 * The wash was also the loudest thing in this file that nothing measured.
 *
 * WHAT REPLACES IT IS THE PROTOTYPE'S `.slot`, which is the rail's own idiom:
 * the row's ground steps up to ZD_BASE and a 3dp vermilion mark - ZD_FOCUS_BAR
 * wide, the same width as the focused window's bar - stands on the leading
 * edge. An engaged row gets the FULL height in ZD_VERM_BR; the short form at
 * 35%..65% is the rail's "running but not focused" and is drawn by kernel.zl.
 *
 * THERE ARE TWO ROW IDIOMS AND THE SPLIT IS DELIBERATE. This one is the
 * navigation row - ui_nav_row, the rail slot - where the row is a POINTER at
 * something else and the thing it points at is elsewhere on screen. A data row
 * - ui_list_row here, ui_grid_row in uikit.c - is the thing itself, and it
 * takes the knockout (`tr.sel`), the same value inversion the focused header
 * makes. Pointer: mark. Object: invert. Nothing chooses between them at
 * runtime, so neither can drift into the other.
 *
 * `zebra` IS NOW IGNORED, and the parameter stays only because it is in the
 * published signature. PRESSWORK rules rows with a 1px ZD_CUT groove between
 * them; a 1% stripe under that is a second, weaker answer to a question the
 * groove already answers, and at 1% on this ladder it is 1.02:1 - present in
 * the token and absent on the screen. */
void ui_row_select(int x, int y, int w, int h, int selected, int zebra)
{
    (void)zebra;
    if (L.mode != UI_DRAW || !selected) return;
    fb_fill_px(x, y, w, h, theme.panel);
    int bw = UI_DP(&theme, ZD_SEL_BAR_W);
    if (bw < 1) bw = 1;
    fb_fill_px(x, y, bw, h, theme.accent_br);
}

/* A selectable row: full width, one row high, highlighted when the pointer is
 * over it. `selected` marks the current one - passed in rather than stored,
 * because the app already knows which of its things is selected and a second
 * copy here would be a second thing to keep in step. */
int ui_list_row(const char *s, int selected)
{
    int x, y, w = L.w, h = theme.row_h;
    place(w, h, &x, &y);

    /* Cheap rejection FIRST. A list can be thousands of rows and only ~20 are
     * ever visible; drawing the rest and letting the scissor throw them away
     * would make scrolling cost O(items) instead of O(visible). The scissor is
     * a correctness guarantee, not a substitute for not drawing. */
    if (S.on && (y + h < S.y || y > S.y + S.h)) {
        L.index++;                      /* keep widget identity stable */
        return 0;
    }

    int over = hit(x, y, w, h);
    int fired = fire(x, y, w, h);
    if (L.mode == UI_DRAW) {
        /* A DATA ROW IS THE OBJECT, SO SELECTION IS THE KNOCKOUT - `tr.sel`,
         * the same value inversion the focused window's header makes. It does
         * NOT call ui_row_select(), which is the other idiom (see above): this
         * row is a file, not a pointer at one.
         *
         * The ink has to move with it. ZD_TEXT_1 on ZD_KNOCK is 1.4571:1, so a
         * selected row drawn with the row ink and an inverted ground is an
         * empty light bar - which is what happens whenever a selection changes
         * the ground and a caller keeps its own colour. theme.knock_ink is
         * 8.53:1 the other way. */
        if (selected)
            fb_fill_px(x, y, w, h, theme.knock);
        else if (over)
            fb_fill_px(x, y, w, h, theme.panel_hi);
        /* the 1px groove between rows, which is what rules a list in this
         * design - there is no zebra and no row radius */
        fb_fill_px(x, y + h - 1, w, 1, selected ? theme.ko_edge : theme.cut);
        fb_text_prop(x + UI_S2(&theme), y + (h - text_h()) / 2, s,
                     selected ? theme.knock_ink : (over ? theme.text_hi : theme.text));
        focus_ring(x, y, w, h);
    }
    return fired;
}

/* Open a scrolling viewport `h` pixels tall. `off` is the app's scroll
 * position - again, the app's variable, not ours. Everything between this and
 * ui_scroll_end is clipped to the viewport and shifted up by off. */
void ui_scroll_begin(int h, int *off)
{
    int x, y;
    place(L.w, h, &x, &y);

    S.on = 1;
    S.x = x; S.y = y; S.w = L.w; S.h = h;
    S.off = off ? *off : 0;
    S.content = 0;
    S.save_cy = L.cy;
    S.save_h = L.h;

    /* NARROW THE SCISSOR, DO NOT REPLACE IT.
     *
     * wm_repaint has already clipped this app to its own client rectangle, and
     * that is the guarantee the whole layering rests on: an app which draws at
     * -500,-500 physically cannot produce a pixel. Calling fb_clip() with the
     * viewport alone throws that away and re-opens the app to anywhere the
     * viewport reaches, which at a scroll position past the window edge is
     * off the window entirely. Intersect instead. */
    if (L.mode == UI_DRAW) {
        fb_clip_get(&S.cx0, &S.cy0, &S.cx1, &S.cy1);
        int nx0 = x     > S.cx0 ? x     : S.cx0;
        int ny0 = y     > S.cy0 ? y     : S.cy0;
        int nx1 = x + L.w < S.cx1 ? x + L.w : S.cx1;
        int ny1 = y + h   < S.cy1 ? y + h   : S.cy1;
        fb_clip(nx0, ny0, nx1 - nx0, ny1 - ny0);
    }
    /* the cursor moves INTO the viewport, offset by the scroll position */
    L.cy = y - S.off;
    L.cx = L.x;
}

void ui_scroll_end(int *off)
{
    S.content = L.cy + S.off - S.y;     /* how tall the content turned out */
    if (L.mode == UI_DRAW) {
        /* RESTORE, NOT REMOVE. This was fb_clip_none(), which does not put the
         * caller's scissor back - it deletes it. wm_repaint sets the client
         * rectangle before calling an app; the moment that app closed a scroll
         * region every widget it drew afterwards was free to paint over the
         * whole screen, including other windows and the desktop furniture.
         *
         * S.cx0..cy1 have been declared "the scissor to put back" since this
         * function was written and nothing ever read them - the fix was half
         * built and then never wired up, which is exactly the project's
         * "the code exists is not the code works" class. */
        fb_clip(S.cx0, S.cy0, S.cx1 - S.cx0, S.cy1 - S.cy0);
        /* the scrollbar, only when there is something to scroll. A bar that
         * is always there but sometimes full-height is a bar that means
         * nothing. */
        if (S.content > S.h) {
            /* THE LADDER WAS INVERTED, AND THE TWO INERT DECISIONS SWAPPED.
             *
             * The prototype:
             *   ::-webkit-scrollbar       { width: calc(9px * var(--ui)); }
             *   ::-webkit-scrollbar-track { background: var(--zd-well); }
             *   ::-webkit-scrollbar-thumb { background: var(--zd-text-inert);
             *                               border-radius: var(--zd-r-chip); }
             *
             * The track is ZD_WELL, a SUNKEN rung - a groove the thumb runs in.
             * This drew theme.panel_hi, which is ZD_FLOAT, a RAISED one: the
             * track stood proud of the surface it was cut into. And the thumb
             * was theme.text_dim (ZD_TEXT_3, a TEXT rung) where the prototype
             * uses ZD_TEXT_INERT - which is one of the exactly four uses that
             * token is sanctioned for, the scrollbar thumb being the first
             * named. Taken with the rail's readouts, which drew glyphs with
             * ZD_TEXT_INERT against its own "never a glyph" rule, the two
             * decisions about that token were precisely swapped: used where
             * forbidden, unused where licensed.
             *
             * Width 9dp, not UI_S1's 4. Ends are ZD_R_CHIP, not a capsule -
             * bw/2 rounds a 9px bar into a lozenge. */
            int bw = UI_DP(&theme, 9);
            int bx = S.x + S.w - bw;
            int th2 = S.h * S.h / S.content;
            if (th2 < UI_S6(&theme)) th2 = UI_S6(&theme);
            int ty = S.y + (S.h - th2) * S.off / (S.content - S.h);
            int rc = UI_DP(&theme, ZD_R_CHIP);
            fb_rrect(bx, S.y, bw, S.h, 0, theme.surf_well);
            fb_rrect(bx, ty, bw, th2, rc, theme.surf_7);
        }
        /* THE TRIM RULE, WHICH NOTHING DREW.
         *
         *   .scroll::after { position: sticky; bottom: 0;
         *                    height: calc(2px * var(--ui));
         *                    background: var(--zd-lit); }
         *
         * The prototype states its purpose where it defines it: without a
         * struck line closing the viewport, a row cut off at the bottom "read
         * as a rendering fault rather than as more-below". Every scrolling
         * pane in this OS chopped its last row against bare ground, so the one
         * cue that says the list continues was the one thing missing.
         *
         * Drawn after the bar so the two agree at the corner, and drawn
         * whether or not the content overflows - it is the viewport's edge,
         * not an overflow indicator. */
        fb_fill_px(S.x, S.y + S.h - UI_DP(&theme, ZD_RULE_H), S.w,
                   UI_DP(&theme, ZD_RULE_H), theme.lit);
    }
    /* clamp the app's scroll position to what the content turned out to be -
     * it cannot know that before the loop it just ran */
    if (off) {
        int max = S.content - S.h;
        if (max < 0) max = 0;
        if (*off > max) *off = max;
        if (*off < 0) *off = 0;
    }
    L.cy = S.save_cy + S.h + theme.gap;
    L.cx = L.x;
    L.h = S.save_h;
    S.on = 0;
}

int ui_scroll_content(void) { return S.content; }

/* zl numbers cross the runtime boundary by value, while the C toolkit keeps a
 * scroll offset through an out-parameter. App draws are sequential and scroll
 * regions cannot nest, so one bridge slot preserves the exact C semantics
 * without inventing a retained widget object. */
static int zl_scroll_off;
void ui_scroll_begin_value(int h, int off)
{
    zl_scroll_off = off;
    ui_scroll_begin(h, &zl_scroll_off);
}
int ui_scroll_end_value(void)
{
    ui_scroll_end(&zl_scroll_off);
    return zl_scroll_off;
}
