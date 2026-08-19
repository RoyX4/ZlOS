/* layout.c - the box model.
 *
 * THIS IS ui.c's ALGORITHM WITH A DIFFERENT CALLER, and that is the whole
 * reason a browser is reachable here at all. ui.c places a widget at a cursor,
 * advances the cursor by the widget's size, and wraps the row at the content
 * width - which is precisely inline text flow. The toolkit had already solved
 * the hard half of a layout engine before anyone called it one.
 *
 * ui.c is not reused DIRECTLY, and the reason is worth stating rather than
 * hiding: its cursor is a single global tuned for widgets - one row height, no
 * nesting, no per-run font size, and no way to lay out without drawing. A
 * browser needs all four. What is taken is the algorithm and the discipline,
 * not the code. The look track owns ui.c; nothing here touches it.
 *
 * ~~TWO BOX TYPES, and no more:
 *     BLOCK   stacks vertically, takes the full content width, has margins
 *     INLINE  flows along a line and wraps at the content width
 *   That is enough for a document. It is not enough for a web app, and the
 *   difference between those two is float, flex, grid and positioning - i.e.
 *   most of CSS. Declaring the boundary is more useful than pretending it is
 *   further out than it is.~~
 *
 * THAT PARAGRAPH IS KEPT BECAUSE THE SHAPE OF ITS MISTAKE IS THE REUSABLE
 * PART, which is the same standard css.h's own header now applies to itself
 * and `docs/browser-status.md` applies to "a browser is unbounded".
 *
 * Everything it says about the CODE was true. Two box types WERE all there
 * were, and the gap to a web app IS float, flex, grid and positioning. What
 * was wrong was the last sentence, and it was wrong in a specific way worth
 * naming: it declared a boundary where there was only an ABSENCE. Nobody had
 * written flex; that is not the same claim as flex being out of reach, and
 * conflating the two is how a project stops trying the thing it could do. Flex
 * and grid have SPECIFICATIONS. They are finite, they are how every modern
 * page is arranged, and without them wikipedia.org renders as one giant
 * stacked word. What is genuinely unbounded is PIXEL PARITY WITH CHROME, and
 * that is still refused - see the list at the bottom of this comment.
 *
 * SO THERE ARE NOW SIX WAYS A BOX IS LAID OUT, and the count is the honest
 * measure of what changed:
 *   INLINE    flows along a line and wraps at the content width  (unchanged)
 *   BLOCK     stacks vertically, and now has a REAL box: width/height with
 *             min and max, box-sizing, padding that insets, borders, a
 *             background, auto margins that centre, and overflow
 *   FLEX      a main axis and a cross axis, with grow and shrink
 *   GRID      fixed, percentage, fr and auto column tracks; implicit rows
 *   FLOAT     out of the block flow, in the line breaker's way
 *   ABSOLUTE  out of the flow entirely, against a positioned ancestor
 *
 * HOW ALL OF THAT FITS WITH NO HEAP - the one design idea this file rests on.
 * The obvious implementation of flex is "build a box tree, measure it, then
 * position it", and a box tree needs an allocator this kernel does not have.
 * It is also not needed, because runs are PURE POSITION DATA IN A FLAT ARRAY:
 *
 *     int start = nruns;            lay the child out at the origin
 *     ...                           however complicated it is
 *     int end = nruns;              runs[start..end) are exactly its runs
 *     measure_range(start, end)     its bounding box, for free
 *     translate_range(start, end, dx, dy)   and now it is somewhere else
 *
 * A child is laid out at the origin, measured, and MOVED. When a flex item's
 * final main size differs from the size it was measured at, `nruns` is simply
 * truncated back to `start` and the item is laid out again - re-layout is
 * cheap and exact where a heap is not available at all. That composes
 * recursively, which is why nested flex inside grid inside flex needs no code
 * of its own. sub_layout() below is the whole of it.
 *
 * THE CURSOR IS TWO STRUCTS, NOT TWELVE GLOBALS, and that is a bug class being
 * closed rather than a tidy-up. Every one of these layouts saves cursor state,
 * lays a child out, and restores it; a field that is added later and forgotten
 * in one of those restores produces a document that is subtly wrong somewhere
 * else, with nothing to point at. Splitting the state by LIFETIME makes the
 * mistake unrepresentable:
 *   struct flowpos   advances and is never restored (the y cursor, the pen)
 *   struct flowenv   is scoped and is ALWAYS restored, as one assignment
 * `env = save` cannot forget a field. That is the point.
 *
 * MARGIN COLLAPSING IS HERE and it is four lines: adjacent vertical margins
 * take the larger of the two rather than adding. It is included because
 * without it every gap in the document is the sum of two arbitrary numbers and
 * nothing lines up; it is not the full CSS rule (which also collapses through
 * empty parents).
 *
 * NO PIXELS. Not one call into fb.c, by design - see layout.h. The output is
 * an array of runs, and the same array is what the host harness asserts
 * against and what browser.c paints. Two functions are reached for outside
 * this file: the text measure, and now the image hook. Both are injected
 * pointers, so this file still links and still runs as an ordinary Linux
 * program with no font, no decoder and no framebuffer - and that property is
 * worth more than any feature added here.
 *
 * NO HORIZONTAL SCROLLING, EVER, and it is load-bearing for several decisions
 * below. emit_pre already argued it: a line that runs past the right edge is
 * not "preserved", it is INVISIBLE with nothing on screen to say so. So a box
 * wider than its container is CLAMPED to the container rather than allowed to
 * overflow, flex items that cannot shrink are scaled down anyway, and grid
 * tracks that do not fit are scaled. Every one of those is a deliberate
 * departure from CSS, made for the same reason, and the harness asserts the
 * invariant it buys: no run outside the content box.
 *
 * WHAT IS REFUSED, stated so nobody has to find out by reading the code:
 *   grid: named lines, grid-template-areas, grid-row/grid-column, spans, and
 *         explicit placement. css.h carries no field for any of them, so they
 *         cannot reach here at all; items flow into columns in order.
 *   grid: no explicit ROW template - rows are implicit and sized to content.
 *   flex: `order`, `align-content` on a multi-line container, and baseline
 *         alignment. The first two have no field in css.h; baseline has a
 *         CSS_J_BASELINE constant and is treated as flex-start, because one
 *         glyph atlas means every baseline is the same fraction of the line
 *         box and a true baseline pass would agree with start most of the time
 *         and be wrong subtly the rest.
 *   inline-flex and inline-grid lay out as BLOCK-level flex and grid. Making
 *         them inline needs an inline-block atom, which is the next thing here
 *         and is not this change.
 *   position: sticky lays out as `relative`. `fixed` lays out as `absolute`
 *         against the viewport, which is right until the page scrolls.
 *   table: colspan, rowspan and per-cell alignment, unchanged from before.
 *   pixel parity with any other browser. Still unbounded, still refused.
 */

#include "layout.h"
#include "html.h"
#include "css.h"

#define MAX_RUNS   12288   /* 15,806 words in a Wikipedia article */
#define GEN_SIZE   256

/* THE OUT-OF-FLOW LIMITS. Each of these refuses and records through
 * lay_overflowed() rather than growing or scribbling, which is the same rule
 * html.c and css.c already state for their own arrays.
 *
 * MAX_FLOATS is 16 because a document's floats are an image beside a
 * paragraph, a pull-quote, or an infobox - a page with seventeen simultaneously
 * active floats is a layout framework, not a document. When the array is full
 * the element lays out IN FLOW instead of floating: wrong-looking, and visible,
 * which is the failure mode to prefer.
 *
 * MAX_FITEMS is 256 flex items pooled across ALL nesting levels, not per
 * container - a per-container array would be 11 ints on the stack times the
 * recursion depth, which is exactly the thing the stack budget below cannot
 * afford. 256 * 11 ints = 11,264 bytes of BSS. */
#define MAX_FLOATS 16
#define MAX_FITEMS 256
#define MAX_FLINES 64
#define GRID_ROWS  512

/* Recursion, and THE MEASURED STACK COST OF IT, because "see the report" is
 * how a number gets lost. html.c caps its own tree at 32 deep; float and
 * absolute positioning RE-ENTER walk_node for the same node, so the real bound
 * is about twice the tree depth, which is where 40 comes from. Past it the
 * subtree is dropped and recorded - a browser eats hostile markup and the
 * kernel stack is finite.
 *
 * MEASURED, by ulimit -s bisection against 60 nested flex divs, not derived:
 *
 *   walk_node frame   2,336 bytes (64-bit) / 2,272 (32-bit), with lay_flex
 *                     and lay_grid inlined into it
 *   worst case        ~92 KB at LAY_DEPTH_MAX 40
 *   survives          92 KB;  faults at 88 KB
 *
 * THIS IS THE WEAKEST NUMBER IN THIS FILE and it is stated rather than buried:
 * the compositor runs on a 256 KiB stack (see kernel/docs/browser-status.md's
 * note on the zl compositor needing 256 KiB after 16 KiB overflowed into the
 * console statics), so a hostile page can take a THIRD of it and still be
 * inside this limit. LAY_DEPTH_MAX is the only knob and the cost is linear in
 * it - halving it halves the worst case and costs nothing a real document
 * would notice, because a real document is nowhere near 20 deep. */
#define LAY_DEPTH_MAX 40

/* EVERY DIMENSION IS CLAMPED TO THIS BEFORE IT IS MULTIPLIED BY ANOTHER ONE.
 * There is no floating point and no 64-bit division on the kernel's i386
 * path, so every ratio here - an image's aspect, an fr share, a percentage of
 * a percentage - is a product of two dimensions divided by a third. 32000 *
 * 32000 is 1.024e9, which fits in a signed 32-bit int with room; 32001 squared
 * does not overflow either but the round number is easier to check against.
 * The existing line_height() floor comment is the same class of trap from the
 * other end: there, division rounding a size to zero; here, multiplication
 * wrapping a product negative. Both come from a document, so both are hostile
 * input rather than a formality. */
#define LAY_MAX_DIM 32000

/* Sub-layout passes per document. Flex measures, then sizes, then places -
 * three passes per item - and nesting multiplies. A pathological document of
 * flex inside flex inside flex is bounded by this rather than by patience;
 * past it the measure passes are skipped and items take their containing
 * block's size, which is wrong-looking and finite. Recorded like every other
 * refusal. */
#define LAY_MAX_PASSES 20000

static struct lay_run runs[MAX_RUNS];
static int nruns, dropped;
static int doc_w, em, total_h, nlines;
static int passes;
static lay_measure_fn measure;
static lay_image_fn   image_of;

/* generated text - list markers, which exist in no source document and so
 * cannot point into html.c's arena */
static char gen[GEN_SIZE];
static int  gused;

/* ---- the cursor, split by LIFETIME ------------------------------------------
 * See the header. flowpos ADVANCES; nothing ever restores it, because "where
 * the next thing goes" is the one piece of state a document accumulates.
 * flowenv is SCOPED; every element restores it with one assignment, and a
 * field added to it later is restored by that same assignment for free. */
struct flowpos {
    int by;            /* block cursor: the y everything stacks at   */
    int fx;            /* inline pen                                 */
    int fy;            /* top of the line box being built            */
    int line_h;        /* the tallest run on it                      */
    int line_start;    /* index of its first run                     */
    int line_open;
    int ll, lr;        /* the open line's edges AFTER floats         */
    int pend_m;        /* the collapsed margin waiting to be applied */
    int nfloat;        /* floats taken so far, across the whole doc  */
    int nfrozen;       /* run ranges this line must not re-align     */
};

struct flowenv {
    int left, right;   /* the content edges of the containing block  */
    int rgb, bg;       /* what the stylesheet is currently saying    */
    int align;
    int float_base;    /* the first float THIS formatting context owns */
    int frozen_base;   /* ...and the same for frozen run ranges        */
    int cb_x, cb_y, cb_w, cb_h;   /* nearest positioned ancestor's box;
                                   * cb_h is -1 while it is still open   */
};

static struct flowpos pos;
static struct flowenv env;

/* AN ACTIVE FLOAT IS A RECTANGLE THE LINE BREAKER MUST AVOID, and that is all
 * it is. Floats above float_base belong to an enclosing formatting context and
 * are invisible here, which is what makes a flex item or a table cell a clean
 * slate without clearing anyone else's state. */
struct frect { int x0, x1, y0, y1, side; };
static struct frect floats[MAX_FLOATS];

/* RUNS THAT line_end() MUST LEAVE ALONE. An absolutely positioned element does
 * not break the line it appears in - that is the whole point of taking it out
 * of flow - so its runs are appended in the MIDDLE of an open line box, and
 * line_end's bottom-alignment pass then drags them back onto the baseline it
 * is aligning and text-align shifts them sideways with the rest of the line.
 *
 * Found by an assertion rather than by reading: `position: absolute; top: 5px`
 * inside a padded positioned ancestor rendered at the ancestor's CONTENT top
 * instead of five pixels below its border box - and only when some text
 * preceded it on the same line, which is why it looked like a containing-block
 * bug and was not one. Sixteen ranges per line is far more than a document
 * needs; past that the element is placed and then re-aligned, which is the
 * old behaviour, and it is recorded. */
#define MAX_FROZEN 16
static struct { int a, b; } frozen[MAX_FROZEN];

/* the ancestor path a descendant selector is matched against */
#define CSS_PATH_MAX 32
static struct css_elem cpath[CSS_PATH_MAX];
static int cdepth;
static int wdepth;

/* ---- consume-once channels --------------------------------------------------
 * Flex and grid have to tell the NEXT block box "your used width is exactly
 * this" without that instruction leaking into the box after it, and absolute
 * positioning has to lay an element out as though it were in flow without
 * re-entering the out-of-flow branch and recursing forever. Three ints, each
 * read and cleared at its single consumption point, are cheaper and far easier
 * to reason about than threading three more parameters through every layer. */
static int impose_w = -1, impose_h = -1;
static int force_inflow;

void lay_set_measure(lay_measure_fn f) { measure = f; }
void lay_set_image(lay_image_fn f)     { image_of = f; }

static int meas(const char *s, int len, int size, int style)
{
    if (!measure || len <= 0) return 0;
    return measure(s, len, size, style);
}

static int clampdim(int v)
{
    if (v < 0) return 0;
    if (v > LAY_MAX_DIM) return LAY_MAX_DIM;
    return v;
}

/* NEVER ZERO. Integer division makes a small em round a size away entirely -
 * h6 is em*9/10, which is 0 for any em below 2 - and a zero line height makes
 * a run taller than the line box that contains it, so line_end computes a
 * NEGATIVE y and the document lays out above its own origin. Not reachable
 * with the kernel's 16 and 32 pixel fonts; found by the fuzzer, which picks
 * sizes nobody would type. A floor costs one comparison. */
static int line_height(int size) { int h = size * 5 / 4; return h > 0 ? h : 1; }

/* ---- run ranges: measure, move, clip ---------------------------------------
 * The three operations the whole design in the header rests on. They are
 * deliberately the ONLY way anything here relocates a subtree: a second way to
 * move runs is a second place to forget a run kind. */
static void measure_range(int a, int b, int *x0, int *y0, int *x1, int *y1)
{
    int lo_x = 0, lo_y = 0, hi_x = 0, hi_y = 0, any = 0;
    if (a < 0) a = 0;
    if (b > nruns) b = nruns;
    for (int i = a; i < b; i++) {
        const struct lay_run *r = &runs[i];
        if (!any) { lo_x = r->x; lo_y = r->y; hi_x = r->x + r->w; hi_y = r->y + r->h; any = 1; continue; }
        if (r->x < lo_x) lo_x = r->x;
        if (r->y < lo_y) lo_y = r->y;
        if (r->x + r->w > hi_x) hi_x = r->x + r->w;
        if (r->y + r->h > hi_y) hi_y = r->y + r->h;
    }
    if (x0) *x0 = lo_x;
    if (y0) *y0 = lo_y;
    if (x1) *x1 = hi_x;
    if (y1) *y1 = hi_y;
}

/* the natural width of a laid-out subtree: how wide it WANTS to be.
 *
 * A block's own background and border runs span the whole box it was measured
 * in, so counting them would answer "the probe width" every time and make
 * every flex item exactly as wide as its container - which is precisely the
 * bug that makes a flex row look like a column. They are counted only when
 * they are NARROWER than the probe, which is the case where the author set an
 * explicit width and that width is the honest answer. */
static int natural_width(int a, int b, int probe)
{
    int hi = 0;
    if (a < 0) a = 0;
    if (b > nruns) b = nruns;
    for (int i = a; i < b; i++) {
        const struct lay_run *r = &runs[i];
        if ((r->kind == LR_BOX || r->kind == LR_RULE) && r->x + r->w >= probe) continue;
        if (r->x + r->w > hi) hi = r->x + r->w;
    }
    return hi;
}

static void translate_range(int a, int b, int dx, int dy)
{
    if (a < 0) a = 0;
    if (b > nruns) b = nruns;
    if (!dx && !dy) return;
    for (int i = a; i < b; i++) { runs[i].x += dx; runs[i].y += dy; }
}

/* OVERFLOW IS CLIPPED HERE, NOT IN THE PAINTER, and the difference matters.
 * fb_clip is a correctness net: it stops a stray rectangle corrupting the
 * screen. It is not an implementation of `overflow: hidden`, because a run
 * that is outside its box is invisible with NO explanation - which is exactly
 * the failure mode indent()'s clamp exists to prevent, and which the fuzzer
 * already caught once as a blank window. Clipping the geometry here means the
 * harness can assert it with no framebuffer linked, which is the only place it
 * can ever be checked.
 *
 * A run that ends up entirely outside is zeroed rather than left with a
 * negative size: fuzz.c asserts w >= 0 and h >= 0 on every run. */
static void clip_range(int a, int b, int x, int y, int w, int h)
{
    if (a < 0) a = 0;
    if (b > nruns) b = nruns;
    if (w < 0) w = 0;
    if (h < 0) h = 0;
    for (int i = a; i < b; i++) {
        struct lay_run *r = &runs[i];
        int rx0 = r->x, ry0 = r->y, rx1 = r->x + r->w, ry1 = r->y + r->h;
        if (rx0 < x) rx0 = x;
        if (ry0 < y) ry0 = y;
        if (rx1 > x + w) rx1 = x + w;
        if (ry1 > y + h) ry1 = y + h;
        if (rx1 <= rx0 || ry1 <= ry0) { r->w = 0; r->h = 0; continue; }
        r->x = rx0; r->y = ry0; r->w = rx1 - rx0; r->h = ry1 - ry0;
    }
}

/* horizontal only: used where a child box has been placed at an x the parent
 * chose, so vertical extent is the parent's business and horizontal escape is
 * the invariant the harness checks */
static void clip_x_range(int a, int b, int x0, int x1)
{
    if (a < 0) a = 0;
    if (b > nruns) b = nruns;
    for (int i = a; i < b; i++) {
        struct lay_run *r = &runs[i];
        int a0 = r->x, a1 = r->x + r->w;
        if (a0 < x0) a0 = x0;
        if (a1 > x1) a1 = x1;
        if (a1 <= a0) { if (r->x < x0) r->x = x0; if (r->x > x1) r->x = x1; r->w = 0; continue; }
        r->x = a0; r->w = a1 - a0;
    }
}

/* SHARES THAT CANNOT OVERFLOW WHEN MULTIPLIED. free * share_i / total wraps a
 * 32-bit int the moment share_i is large, and a flex-grow or an fr count comes
 * from a document. Halving every share until the TOTAL fits in 32767 makes
 * free * share_i at most 32000 * 32767 = 1.05e9, which fits. Rounding UP keeps
 * a nonzero share nonzero, so an item with grow: 1 among a thousand does not
 * silently drop to zero and take its space with it. Returns the new total. */
static int normalize_shares(int *v, int n, int cap)
{
    int total = 0, guard = 0;
    for (int i = 0; i < n; i++) { if (v[i] < 0) v[i] = 0; total += v[i]; }
    while (total > cap && guard++ < 64) {
        total = 0;
        for (int i = 0; i < n; i++) { v[i] = v[i] ? (v[i] + 1) / 2 : 0; total += v[i]; }
    }
    return total;
}

/* ---- floats -----------------------------------------------------------------
 * The standard bounded implementation: a fixed array of active rectangles that
 * the line breaker consults for the left and right edge at each y. */
static void float_edges(int y, int h, int *l, int *r)
{
    *l = env.left; *r = env.right;
    if (pos.nfloat <= env.float_base) return;
    int y1 = y + (h > 0 ? h : 1);
    for (int i = env.float_base; i < pos.nfloat; i++) {
        const struct frect *f = &floats[i];
        if (f->y1 <= y || f->y0 >= y1) continue;
        if (f->side == CSS_FLOAT_LEFT) { if (f->x1 > *l) *l = f->x1; }
        else                           { if (f->x0 < *r) *r = f->x0; }
    }
    if (*l < env.left) *l = env.left;
    if (*r > env.right) *r = env.right;
    if (*r < *l) *r = *l;
}

/* the next y at which the available width could possibly grow */
static int next_float_bottom(int y)
{
    int best = -1;
    for (int i = env.float_base; i < pos.nfloat; i++) {
        int b = floats[i].y1;
        if (b > y && (best < 0 || b < best)) best = b;
    }
    return best < 0 ? y : best;
}

static int float_bottom_all(void)
{
    int b = 0;
    for (int i = env.float_base; i < pos.nfloat; i++)
        if (floats[i].y1 > b) b = floats[i].y1;
    return b;
}

static void clear_floats(int kind)
{
    if (kind == CSS_CLEAR_NONE) return;
    for (int i = env.float_base; i < pos.nfloat; i++) {
        int want = (floats[i].side == CSS_FLOAT_LEFT)
                 ? (kind == CSS_CLEAR_LEFT  || kind == CSS_CLEAR_BOTH)
                 : (kind == CSS_CLEAR_RIGHT || kind == CSS_CLEAR_BOTH);
        if (want && floats[i].y1 > pos.by) pos.by = floats[i].y1;
    }
}

/* A margin is not applied when it is declared - it is applied when something
 * finally needs to be drawn, which is what lets two of them collapse. The
 * leading margin of the whole document is dropped: a page must not open with
 * a gap above its first heading. */
static void apply_margin(void)
{
    if (nruns > 0) pos.by += pos.pend_m;
    pos.pend_m = 0;
}

static void margin(int m) { if (m > pos.pend_m) pos.pend_m = m; }

/* INDENTATION IS CLAMPED AGAINST THE CONTENT WIDTH. Nested lists add to the
 * left edge, and nothing stopped that edge from walking past the right one -
 * so a page with a dozen nested <ul> on a narrow window pushed its content
 * entirely outside the box. In the kernel fb_clip then hides it, and the
 * result is a blank window with no explanation at all.
 *
 * Two characters of content is the floor. Below that a list is unreadable
 * anyway, and the alternative - a negative content width - makes every
 * wrapping decision below meaningless. Found by the fuzzer, which picks
 * width/font ratios no hand-written test would. */
static void indent(int px)
{
    int floor_w = em * 2;
    int max_left = env.right - floor_w;
    if (max_left < 0) max_left = 0;
    env.left += px;
    if (env.left > max_left) env.left = max_left;
    if (env.left < 0) env.left = 0;
}

static void line_end(void);

static void line_begin(void)
{
    if (pos.line_open) return;
    apply_margin();
    /* A FLOAT CAN LEAVE A LINE TOO NARROW TO HOLD ANYTHING, so before opening
     * one, walk down past float bottoms until there is room. Bounded by the
     * number of active floats - there are only that many y values where the
     * answer changes. */
    int l = env.left, r = env.right;
    for (int guard = 0; guard <= MAX_FLOATS; guard++) {
        float_edges(pos.by, line_height(em), &l, &r);
        if (r - l >= em || r - l >= env.right - env.left) break;
        int nb = next_float_bottom(pos.by);
        if (nb <= pos.by) break;
        pos.by = nb;
    }
    pos.line_open = 1;
    pos.line_start = nruns;
    pos.fy = pos.by;
    pos.line_h = 0;
    pos.ll = l; pos.lr = r;
    pos.fx = l;
    pos.nfrozen = env.frozen_base;
}

static void freeze_range(int a, int b)
{
    if (!pos.line_open || b <= a) return;
    if (pos.nfrozen >= MAX_FROZEN) { dropped++; return; }
    frozen[pos.nfrozen].a = a;
    frozen[pos.nfrozen].b = b;
    pos.nfrozen++;
}

static int is_frozen(int i)
{
    for (int k = env.frozen_base; k < pos.nfrozen; k++)
        if (i >= frozen[k].a && i < frozen[k].b) return 1;
    return 0;
}

/* abandon an OPEN BUT EMPTY line box. line_end() on one would count a line
 * that never held anything, and the monotonicity assertion in htmltest - and
 * the whole claim that this is a layout engine - is a claim about that count. */
static void discard_line(void)
{
    if (pos.line_open && nruns == pos.line_start) pos.line_open = 0;
}

/* Close the current line box. Runs of different sizes on one line are
 * BOTTOM-aligned within it, which is a baseline alignment in disguise: every
 * glyph here comes from one atlas, so ascent is the same fraction of the cell
 * at every size and aligning the cell bottoms aligns the baselines. It would
 * be wrong the moment a second font with different metrics arrived. */
static void line_end(void)
{
    if (!pos.line_open) return;
    for (int i = pos.line_start; i < nruns; i++) {
        if (is_frozen(i)) continue;
        runs[i].y = pos.fy + pos.line_h - runs[i].h;
    }

    /* TEXT-ALIGN IS A LINE-BOX PROPERTY, so this is the only place it can be
     * applied: the slack is not known until the line is closed. Shift the
     * whole line rather than each run, or the inter-word spacing changes with
     * the alignment. A list marker hangs OUTSIDE the content edge on purpose
     * and is left where it is - moving it would detach it from its item.
     *
     * The edges are the LINE's, not the block's: with a float in the way the
     * line is narrower than its container, and centring against the container
     * would push the text under the float. */
    if (env.align != CSS_ALIGN_LEFT && nruns > pos.line_start) {
        int end = pos.ll;
        for (int i = pos.line_start; i < nruns; i++) {
            if (runs[i].kind == LR_BULLET || runs[i].x < pos.ll) continue;
            if (is_frozen(i)) continue;
            if (runs[i].x + runs[i].w > end) end = runs[i].x + runs[i].w;
        }
        int slack = (pos.lr - pos.ll) - (end - pos.ll);
        if (slack > 0) {
            int shift = (env.align == CSS_ALIGN_CENTER) ? slack / 2 : slack;
            for (int i = pos.line_start; i < nruns; i++) {
                if (runs[i].kind == LR_BULLET || runs[i].x < pos.ll) continue;
                if (is_frozen(i)) continue;
                runs[i].x += shift;
            }
        }
    }

    pos.by = pos.fy + pos.line_h;
    nlines++;
    pos.line_open = 0;
}

static struct lay_run *push_run(void)
{
    if (nruns >= MAX_RUNS) { dropped++; return 0; }
    struct lay_run *r = &runs[nruns++];
    r->kind = LR_TEXT;
    r->x = r->y = r->w = r->h = 0;
    r->text = ""; r->len = 0;
    r->size = em; r->style = 0; r->color = LC_TEXT;
    /* the author's colours ride along on every run, so nothing downstream has
     * to re-derive them from the node */
    r->rgb = env.rgb; r->bg = env.bg;
    r->node = -1; r->link = -1;
    r->img = -1;
    return r;
}

/* the room a word has RIGHT NOW: the open line's edges, or what a line opened
 * here would get */
static void line_room(int *l, int *r)
{
    if (pos.line_open) { *l = pos.ll; *r = pos.lr; return; }
    float_edges(pos.by, line_height(em), l, r);
}

/* Place something of a known size on the current line, wrapping first if it
 * does not fit. The ONE function that moves the inline pen. */
static struct lay_run *place(int w, int h)
{
    line_begin();
    if (pos.fx + w > pos.lr && pos.fx > pos.ll) {
        line_end();
        line_begin();
    }
    struct lay_run *r = push_run();
    if (!r) return 0;
    r->x = pos.fx;
    r->y = pos.fy;                   /* provisional; line_end aligns it */
    r->w = w;
    r->h = h;
    pos.fx += w;
    if (h > pos.line_h) pos.line_h = h;
    return r;
}

/* How many characters of s fit in `avail`? Used only when a single word is
 * wider than a whole line - a URL in a narrow window - where the choice is to
 * break mid-word or to let it escape the content box. Escaping is not an
 * option: the harness asserts x + w <= width on every run. */
static int fit_chars(const char *s, int len, int size, int style, int avail)
{
    int lo = 1, hi = len, best = 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (meas(s, mid, size, style) <= avail) { best = mid; lo = mid + 1; }
        else hi = mid - 1;
    }
    return best;
}

static void emit_word(const char *s, int len, int size, int style, int color,
                      int node, int link)
{
    int h = line_height(size);
    int guard = 0;
    for (;;) {
        int w = meas(s, len, size, style);
        int l, r;
        line_room(&l, &r);
        if (w <= r - l) {
            struct lay_run *rr = place(w, h);
            if (!rr) return;
            rr->kind = LR_TEXT;
            rr->text = s; rr->len = len;
            rr->size = size; rr->style = style; rr->color = color;
            rr->node = node; rr->link = link;
            return;
        }
        /* IT DOES NOT FIT HERE - but does it fit anywhere? A float narrows the
         * line without narrowing the block, so a word that is too wide for
         * this line and fine for a full one belongs BELOW the float, not
         * broken in half. Bounded by the float count: each step clears at
         * least one of them. */
        if (w <= env.right - env.left && guard++ <= MAX_FLOATS) {
            if (pos.line_open && nruns > pos.line_start) { line_end(); continue; }
            discard_line();
            int nb = next_float_bottom(pos.by);
            if (nb > pos.by) { pos.by = nb; continue; }
        }
        break;
    }
    for (;;) {
        int w = meas(s, len, size, style);
        int l, r;
        line_room(&l, &r);
        int room = r - l;
        if (w <= room) {
            struct lay_run *rr = place(w, h);
            if (!rr) return;
            rr->kind = LR_TEXT;
            rr->text = s; rr->len = len;
            rr->size = size; rr->style = style; rr->color = color;
            rr->node = node; rr->link = link;
            return;
        }
        /* longer than any line can be: break it, and keep going with the rest */
        line_begin();
        int avail = pos.lr - pos.fx;
        if (avail < room / 4) { line_end(); line_begin(); avail = pos.lr - pos.fx; }
        int n = fit_chars(s, len, size, style, avail);
        if (n >= len) n = len - 1;
        if (n < 1) n = 1;
        struct lay_run *rr = place(meas(s, n, size, style), h);
        if (!rr) return;
        rr->kind = LR_TEXT;
        rr->text = s; rr->len = n;
        rr->size = size; rr->style = style; rr->color = color;
        rr->node = node; rr->link = link;
        s += n; len -= n;
        line_end();
    }
}

/* Flowing text. html.c has already collapsed the whitespace, so a space here
 * is a real word separator and there is never more than one. */
static void emit_text(const char *s, int len, int size, int style, int color,
                      int node, int link)
{
    int sw = meas(" ", 1, size, style);
    int i = 0;
    while (i < len) {
        if (s[i] == ' ') {
            if (pos.line_open && pos.fx > pos.ll) pos.fx += sw;
            i++;
            continue;
        }
        int j = i;
        while (j < len && s[j] != ' ') j++;
        emit_word(s + i, j - i, size, style, color, node, link);
        i = j;
    }
}

/* <pre>: no word wrapping and no whitespace collapsing. Every newline is a
 * line break and every space was already preserved by the parser.
 *
 * IT DOES STILL BREAK AT THE BOX EDGE, as a last resort, and that is a
 * deliberate departure from what <pre> means. Preformatted says "do not
 * collapse the spacing and do not re-flow at word boundaries"; it does not
 * oblige a renderer to put text where it cannot be seen. This browser has no
 * horizontal scrolling and is never going to have any, so a line that runs
 * past the edge is not preserved - it is INVISIBLE, with nothing on screen to
 * say so. Breaking it keeps the spacing and keeps the text reachable.
 *
 * The fuzzer is what forced the question: unbroken <pre> made the layout's
 * central invariant - no run outside the content box - simply untrue, and an
 * invariant with an undocumented exception is not one. */
static void emit_pre(const char *s, int len, int size, int style, int color,
                     int node, int link)
{
    int i = 0;
    while (i <= len) {
        int j = i;
        while (j < len && s[j] != '\n') j++;
        if (j > i) {
            int at = i;
            while (at < j) {
                line_begin();
                int avail = pos.lr - pos.fx;
                if (avail <= 0) { line_end(); line_begin(); avail = pos.lr - pos.fx; }
                int take = j - at;
                if (meas(s + at, take, size, style) > avail) {
                    take = fit_chars(s + at, take, size, style, avail);
                    if (take < 1) take = 1;
                }
                struct lay_run *r = push_run();
                if (!r) break;
                r->kind = LR_TEXT;
                r->x = pos.fx; r->y = pos.fy;
                r->w = meas(s + at, take, size, style);
                r->h = line_height(size);
                r->text = s + at; r->len = take;
                r->size = size; r->style = style; r->color = color;
                r->node = node; r->link = link;
                pos.fx += r->w;
                if (r->h > pos.line_h) pos.line_h = r->h;
                at += take;
                if (at < j) line_end();          /* the box edge is a break */
            }
        } else if (j < len) {
            line_begin();
            if (pos.line_h < line_height(size)) pos.line_h = line_height(size);
        }
        if (j >= len) break;
        line_end();
        i = j + 1;
    }
}

/* ---- per-tag metrics -------------------------------------------------------
 * A small fixed table, expressed as fractions of the body size rather than as
 * pixels, so the whole document scales with one number. */
static int head_size(int tag)
{
    int sz;
    switch (tag) {
    case HT_H1: sz = em * 2;        break;
    case HT_H2: sz = em * 3 / 2;    break;
    case HT_H3: sz = em * 5 / 4;    break;
    case HT_H4: sz = em * 11 / 10;  break;
    case HT_H5: sz = em;            break;
    case HT_H6: sz = em * 9 / 10;   break;
    default:    sz = em;            break;
    }
    return sz > 0 ? sz : 1;         /* see line_height: a zero size is not one */
}

static int is_heading(int t) { return t >= HT_H1 && t <= HT_H6; }

/* ---- the walk --------------------------------------------------------------
 * Inline properties (size, style, colour, the enclosing link) are INHERITED,
 * so they travel in one struct rather than as seven parameters - flex and grid
 * both have to hand the same set to an item they are laying out for the third
 * time, and seven positional arguments repeated at nine call sites is how one
 * of them ends up in the wrong order. Block properties (the content edges) are
 * flowenv, because a block restores them on the way out.
 */
struct inh {
    int size, style, color, link;
    int *item;
    int ordered, pre;
};

static void walk(int n, const struct inh *in);
static void walk_node(int n, const struct inh *in, const struct css_style *pre);

/* ---- laying a subtree out somewhere else ------------------------------------
 * The primitive from the header comment. Lay `n` out at the ORIGIN in a box
 * `w` wide, and hand back the run range it produced so the caller can measure
 * it and move it. `keep == 0` throws the runs away again, which is what a
 * measuring pass wants.
 *
 * THREE COUNTERS ARE RESTORED ON A DISCARDED PASS and each of them was worth
 * a bug on its own: `gused` is the list-marker arena and 256 bytes of it are
 * exhausted in a few passes over an <ol>; `nlines` is the number the reflow
 * assertion is built on, and a measuring pass that inflates it makes "narrower
 * means more lines" true for the wrong reason; `pend_m` and the rest of
 * flowpos are restored wholesale because a measuring pass must leave NOTHING
 * behind. `dropped` is deliberately NOT restored: a pass that hit MAX_RUNS
 * produced a wrong measurement and the document should say so. */
static int sub_layout(int n, const struct inh *in, const struct css_style *pre,
                      int w, int *r0, int *r1, int *natw, int keep)
{
    struct flowpos savep = pos;
    struct flowenv savee = env;
    int save_gused = gused, save_lines = nlines;
    int start = nruns;

    passes++;
    env.left = 0;
    env.right = w > 0 ? w : 1;
    env.float_base = pos.nfloat;      /* a new formatting context: no floats */
    env.frozen_base = pos.nfrozen;
    pos.by = 0;
    pos.fx = 0; pos.fy = 0;
    pos.line_h = 0; pos.line_start = start; pos.line_open = 0;
    pos.pend_m = 0;
    pos.ll = 0; pos.lr = env.right;

    walk_node(n, in, pre);
    line_end();
    int fb = float_bottom_all();
    if (fb > pos.by) pos.by = fb;     /* a box contains the floats it made */
    int h = pos.by;

    if (r0) *r0 = start;
    if (r1) *r1 = nruns;
    /* THE NATURAL WIDTH IS TAKEN HERE, NOT BY THE CALLER, and that is a bug
     * fix rather than a convenience. A discarded pass truncates `nruns` back
     * to `start`, and natural_width clamps its upper bound to `nruns` - so a
     * caller that measured after the call measured an EMPTY range and got
     * zero. Symptom: every `grid-template-columns: auto` track came out zero
     * wide and its items were laid out one character per line. Nothing about
     * that is visible in a "did it render" check, and the flex max-content
     * path had the same defect without a test that could see it. */
    if (natw) *natw = natural_width(start, nruns, env.right);

    impose_w = impose_h = -1;         /* never leaks past its one consumer */
    pos = savep;
    env = savee;
    if (!keep) { nruns = start; gused = save_gused; nlines = save_lines; }
    return h;
}

/* ---- the box ----------------------------------------------------------------
 * width/height with min and max, box-sizing, padding, border, and the auto
 * margins that centre a block. This is the highest-value function in the file:
 * `max-width: 960px; margin: 0 auto` is THE page-layout idiom of the modern
 * web, and until now `margin: 0 auto` did nothing at all here. */
struct boxm {
    int mt, mb, ml, mr;
    int bt, bb, bl, br;
    int pt, pb, pl, pr;
    int cw;
    int ch, have_h;
};

static void compute_box(const struct css_style *st, int avail, int iw, struct boxm *b)
{
    b->pt = clampdim(st->pad_t); b->pb = clampdim(st->pad_b);
    b->pl = clampdim(st->pad_l); b->pr = clampdim(st->pad_r);
    b->bt = clampdim(st->border_t); b->bb = clampdim(st->border_b);
    b->bl = clampdim(st->border_l); b->br = clampdim(st->border_r);
    int extra = b->bl + b->br + b->pl + b->pr;

    int mla = (st->margin_l == CSS_AUTO), mra = (st->margin_r == CSS_AUTO);
    b->ml = mla ? 0 : clampdim(st->margin_l);
    b->mr = mra ? 0 : clampdim(st->margin_r);
    b->mt = (st->margin_t == CSS_AUTO) ? 0 : clampdim(st->margin_t);
    b->mb = (st->margin_b == CSS_AUTO) ? 0 : clampdim(st->margin_b);

    int border_box = (st->box_sizing == CSS_BOX_BORDER);
    int w;
    if (iw >= 0) {
        /* a flex or grid target. It is the item's OUTER size, so the margins
         * and the border come out of it - the same arithmetic border-box does,
         * one level further out. */
        w = iw - b->ml - b->mr - extra;
    } else if (st->width != CSS_AUTO) {
        w = clampdim(st->width);
        if (border_box) w -= extra;
    } else {
        w = avail - b->ml - b->mr - extra;
    }

    /* MAX FIRST, THEN MIN, so min wins when they contradict. That order is the
     * spec's and it is not arbitrary: min-width is a guarantee about
     * readability and max-width is a preference about line length, and a
     * document that sets max < min means the guarantee. */
    if (st->max_w != CSS_AUTO) {
        int m = clampdim(st->max_w);
        if (border_box) m -= extra;
        if (w > m) w = m;
    }
    if (st->min_w != CSS_AUTO && st->min_w > 0) {
        int m = clampdim(st->min_w);
        if (border_box) m -= extra;
        if (w < m) w = m;
    }
    if (w < 0) w = 0;
    b->cw = w;

    /* AUTO MARGINS TAKE WHAT IS LEFT, and this runs after the clamps on
     * purpose: `max-width: 960px; margin: 0 auto` has no leftover space until
     * max-width has already shrunk the box, and computing the margins first is
     * exactly why that idiom used to centre nothing. */
    int freew = avail - (w + extra + b->ml + b->mr);
    if (freew < 0) freew = 0;
    if (mla && mra)  { b->ml = freew / 2; b->mr = freew - b->ml; }
    else if (mla)    { b->ml = freew; }
    else if (mra)    { b->mr = freew; }

    b->have_h = 0; b->ch = 0;
    if (st->height != CSS_AUTO) {
        b->have_h = 1;
        b->ch = clampdim(st->height);
        if (border_box) b->ch -= b->bt + b->bb + b->pt + b->pb;
        if (b->ch < 0) b->ch = 0;
    }
}

struct blockbox {
    struct boxm m;
    int x, y, w;
    int content_top;
    int rall;           /* first run of the whole box, background included */
    int r0;             /* first CHILD run                                 */
    int bgrun;
    int brun[4];        /* top, right, bottom, left                        */
};

static void block_open(const struct css_style *st, struct blockbox *bb, int iw)
{
    int avail = env.right - env.left;
    if (avail < 0) avail = 0;
    compute_box(st, avail, iw, &bb->m);

    apply_margin();
    bb->y = pos.by;
    bb->x = env.left + bb->m.ml;
    bb->w = bb->m.cw + bb->m.bl + bb->m.br + bb->m.pl + bb->m.pr;

    /* NO HORIZONTAL SCROLLING: a box wider than its container is clamped, not
     * overflowed. See the header - the alternative is content that is
     * invisible with nothing on screen to say so. */
    if (bb->x < env.left) bb->x = env.left;
    if (bb->x + bb->w > env.right) bb->w = env.right - bb->x;
    if (bb->w < 0) bb->w = 0;
    if (bb->m.cw > bb->w - bb->m.bl - bb->m.br - bb->m.pl - bb->m.pr)
        bb->m.cw = bb->w - bb->m.bl - bb->m.br - bb->m.pl - bb->m.pr;
    if (bb->m.cw < 0) bb->m.cw = 0;

    bb->rall = nruns;
    bb->bgrun = -1;
    bb->brun[0] = bb->brun[1] = bb->brun[2] = bb->brun[3] = -1;

    /* THE BACKGROUND IS A RUN, EMITTED BEFORE THE CHILDREN, so document order
     * is paint order and the painter needs no z ordering at all - see the
     * LR_BOX note in layout.h. Only when the element ITSELF set a background:
     * `bg` is seeded from the parent so that text runs carry it, and emitting
     * a box for an inherited value would paint every descendant's rectangle
     * over its ancestor's. */
    if ((st->has & CSS_P_BG) && st->bg >= 0 && bb->w > 0) {
        struct lay_run *r = push_run();
        if (r) {
            r->kind = LR_BOX;
            r->x = bb->x; r->y = bb->y; r->w = bb->w; r->h = 0;
            r->bg = st->bg;
            r->rgb = LR_NO_RGB;
            bb->bgrun = nruns - 1;
        }
    }
    /* A BORDER IS FOUR FILLED RECTANGLES - four LR_RULE runs carrying rgb, per
     * layout.h. A zero-width side emits nothing at all, which is what keeps an
     * unbordered document's run count exactly what it was. push_run() stamps
     * the inherited colour, and that is right: border-color's initial value is
     * currentColor. */
    {
        const int bw[4] = { bb->m.bt, bb->m.br, bb->m.bb, bb->m.bl };
        for (int k = 0; k < 4; k++) {
            if (bw[k] <= 0 || bb->w <= 0) continue;
            struct lay_run *r = push_run();
            if (!r) break;
            r->kind = LR_RULE;
            r->x = bb->x; r->y = bb->y; r->w = 0; r->h = 0;
            r->bg = LR_NO_RGB;
            if (st->border_rgb >= 0) r->rgb = st->border_rgb;
            r->color = LC_DIM;
            bb->brun[k] = nruns - 1;
        }
    }

    env.left = bb->x + bb->m.bl + bb->m.pl;
    env.right = env.left + bb->m.cw;
    pos.by = bb->y + bb->m.bt + bb->m.pt;
    bb->content_top = pos.by;
    bb->r0 = nruns;

    /* OVERFLOW OTHER THAN VISIBLE ESTABLISHES A NEW FORMATTING CONTEXT, which
     * is the one bit of CSS's float model that is genuinely load-bearing here:
     * without it a float inside a clipped box goes on narrowing lines after
     * the box has ended. */
    if (st->overflow != CSS_OVER_VISIBLE) env.float_base = pos.nfloat;

    if (st->position != CSS_POS_STATIC) {
        env.cb_x = bb->x; env.cb_y = bb->y; env.cb_w = bb->w; env.cb_h = -1;
    }
}

static void block_close(const struct css_style *st, struct blockbox *bb, int ih)
{
    line_end();
    int fb = float_bottom_all();
    if (fb > pos.by && st->overflow != CSS_OVER_VISIBLE) pos.by = fb;

    int content_h = pos.by - bb->content_top;
    if (content_h < 0) content_h = 0;
    if (bb->m.have_h) content_h = bb->m.ch;
    else if (ih >= 0) {
        int inner = ih - bb->m.bt - bb->m.bb - bb->m.pt - bb->m.pb;
        if (inner > content_h) content_h = inner;      /* align-items: stretch */
    }
    if (st->max_h != CSS_AUTO) { int m = clampdim(st->max_h); if (content_h > m) content_h = m; }
    if (st->min_h != CSS_AUTO && st->min_h > 0) { int m = clampdim(st->min_h); if (content_h < m) content_h = m; }

    pos.by = bb->content_top + content_h + bb->m.pb + bb->m.bb;
    int box_h = pos.by - bb->y;
    if (box_h < 0) box_h = 0;

    if (bb->bgrun >= 0) runs[bb->bgrun].h = box_h;
    if (bb->brun[0] >= 0) { struct lay_run *r = &runs[bb->brun[0]];
        r->x = bb->x; r->y = bb->y; r->w = bb->w; r->h = bb->m.bt; }
    if (bb->brun[1] >= 0) { struct lay_run *r = &runs[bb->brun[1]];
        r->x = bb->x + bb->w - bb->m.br; r->y = bb->y; r->w = bb->m.br; r->h = box_h; }
    if (bb->brun[2] >= 0) { struct lay_run *r = &runs[bb->brun[2]];
        r->x = bb->x; r->y = bb->y + box_h - bb->m.bb; r->w = bb->w; r->h = bb->m.bb; }
    if (bb->brun[3] >= 0) { struct lay_run *r = &runs[bb->brun[3]];
        r->x = bb->x; r->y = bb->y; r->w = bb->m.bl; r->h = box_h; }

    if (st->overflow == CSS_OVER_HIDDEN)
        clip_range(bb->r0, nruns, bb->x + bb->m.bl, bb->y + bb->m.bt,
                   bb->w - bb->m.bl - bb->m.br, box_h - bb->m.bt - bb->m.bb);
}

/* ---- images -----------------------------------------------------------------
 * SIZING IS FOUR SOURCES IN PRECEDENCE ORDER and the order is the whole rule:
 * CSS width/height, then the width=/height= attributes, then the intrinsic
 * size the hook reports, then the placeholder box that was here before.
 *
 * PRESERVING THE ASPECT RATIO WHEN EXACTLY ONE IS GIVEN is the single rule
 * that stops every photograph on a real page being a squashed rectangle -
 * `<img src=x width=200>` is how half the web sizes an image, and taking 200
 * for both dimensions is worse than not sizing it at all.
 *
 * NOT ONE DIVISION HERE IS UNGUARDED. An intrinsic dimension of zero comes
 * from a decoder that failed halfway, and it reaches this function as an
 * ordinary number; the ratio is only taken when BOTH intrinsic dimensions are
 * positive. Every product is of two clampdim()ed values, so the largest is
 * 32000 * 32000 and fits. */
static void emit_img(int n, const struct css_style *st, const struct inh *in)
{
    int iw = 0, ih = 0, slot = -1;
    if (image_of) {
        int w = 0, h = 0;
        slot = image_of(n, &w, &h);
        if (slot >= 0) { iw = clampdim(w); ih = clampdim(h); }
        else slot = -1;
    }

    int cw = (st && st->width  != CSS_AUTO) ? clampdim(st->width)  : 0;
    int ch = (st && st->height != CSS_AUTO) ? clampdim(st->height) : 0;
    if (cw <= 0) { int a = html_attr_w(n); if (a > 0) cw = clampdim(a); }
    if (ch <= 0) { int a = html_attr_h(n); if (a > 0) ch = clampdim(a); }

    int w, h;
    if (cw > 0 && ch > 0)              { w = cw; h = ch; }
    else if (cw > 0)                   { w = cw; h = (iw > 0 && ih > 0) ? cw * ih / iw : cw; }
    else if (ch > 0)                   { h = ch; w = (iw > 0 && ih > 0) ? ch * iw / ih : ch; }
    else if (iw > 0 && ih > 0)         { w = iw; h = ih; }
    else                               { w = h = em * 3 / 2; }

    /* wider than the box it sits in scales DOWN proportionally. It does not
     * overflow, for the same reason nothing else here does. */
    {
        int l, r;
        line_room(&l, &r);
        int avail = r - l;
        if (avail > 0 && w > avail) { h = w > 0 ? h * avail / w : h; w = avail; }
    }
    if (w < 0) w = 0;
    if (h < 0) h = 0;

    struct lay_run *r = place(w, h);
    if (r) { r->kind = LR_IMG; r->node = n; r->color = LC_DIM; r->img = slot; }

    int alen;
    const char *alt = html_href(n, &alen);
    if (alen) {
        if (pos.line_open && pos.fx > pos.ll) pos.fx += meas(" ", 1, in->size, in->style);
        emit_text(alt, alen, in->size, in->style | LS_ITALIC, LC_DIM, n, in->link);
    }
}

/* ---- flexbox ----------------------------------------------------------------
 * lay_table() is this file's template for a real two-pass algorithm and it
 * says why one pass cannot work: a column's width depends on cells that have
 * not been read yet. Flex has the same shape for the same reason - an item's
 * final main size depends on the sum of every other item's hypothetical size -
 * except that flex needs THREE passes rather than two, and the third one is
 * worth naming because it is not obvious:
 *
 *   1 MEASURE   each item's hypothetical main size, from its flex-basis or,
 *               failing that, from its own content laid out unconstrained.
 *               Runs discarded.
 *   2 SIZE      sum them, wrap into lines, and distribute the free space by
 *               grow when it is positive or by shrink * basis when it is
 *               negative. Then measure each item's CROSS size at its final
 *               main size, because an item that just got narrower is taller.
 *               Runs discarded.
 *   3 PLACE     lay each item out for real at its final main AND cross size,
 *               and translate its run range into position.
 *
 * Pass 2 exists because align-items: stretch - the INITIAL value, and the
 * thing that makes a row of cards line up - needs the tallest item on the line
 * before any item's box height can be decided.
 *
 * GROW AND SHRINK ARE IN HUNDREDTHS. 100 means 1.0, because there is no
 * floating point on any path in this kernel and `flex-grow: 0.5` is real CSS a
 * document will contain. Getting that wrong is a factor of a hundred, which is
 * not subtle - but confusing grow with shrink IS subtle, so the harness
 * asserts the two separately with different numbers. */
struct fitem {
    int node;
    int grow, shrink;
    int hyp;              /* hypothetical main size                       */
    int target;           /* main size after grow/shrink                  */
    int cross;
    int align;
    int minmain;
};

static struct fitem fpool[MAX_FITEMS];
static int fused;

struct fline { int first, count, main, cross, y; };

/* an element's computed style, with the inherited seeds this file always
 * applies. Factored out because flex and grid both need an item's style BEFORE
 * they lay it out, and walk_node needs the identical answer afterwards - two
 * spellings of the cascade is how they drift. */
static void style_of(int n, const struct inh *in, struct css_style *st)
{
    css_style_init(st, in->size);
    st->rgb = env.rgb; st->bg = env.bg;
    st->size = in->size;
    st->bold      = (in->style & LS_BOLD)   != 0;
    st->italic    = (in->style & LS_ITALIC) != 0;
    st->mono      = (in->style & LS_MONO)   != 0;
    st->underline = (in->style & LS_UNDER)  != 0;
    st->align = env.align;
    if (html_kind(n) == HN_ELEM)
        st->display = html_is_block(html_tag(n)) ? CSS_DISP_BLOCK : CSS_DISP_INLINE;

    int pushed = 0;
    if (html_kind(n) == HN_ELEM && cdepth < CSS_PATH_MAX) {
        cpath[cdepth].tag = html_tagname(n, &cpath[cdepth].tag_len);
        cpath[cdepth].id  = html_id(n, &cpath[cdepth].id_len);
        cpath[cdepth].cls = html_class(n, &cpath[cdepth].cls_len);
        cdepth++;
        pushed = 1;
    }
    if (html_kind(n) == HN_ELEM) {
        int slen;
        const char *sattr = html_style_attr(n, &slen);
        int pw = env.right - env.left;
        css_compute(cpath, cdepth, in->size, pw > 0 ? pw : 1, sattr, slen, st);
    }
    cdepth -= pushed;
}

/* whitespace-only text between two elements is not a flex item. html.c has
 * already collapsed it to a single space, and a browser that made an item of
 * it would put a gap between every pair of cards. */
static int blank_text(int n)
{
    if (html_kind(n) != HN_TEXT) return 0;
    int len;
    const char *s = html_text(n, &len);
    for (int i = 0; i < len; i++)
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r') return 0;
    return 1;
}

static void lay_abs(int n, const struct css_style *st, const struct inh *in);

static void lay_flex(int n, const struct css_style *cst, const struct inh *in)
{
    int dir  = cst->flex_dir;
    int horiz = (dir == CSS_ROW || dir == CSS_ROW_REVERSE);
    int rev   = (dir == CSS_ROW_REVERSE || dir == CSS_COLUMN_REVERSE);
    int wrap  = cst->flex_wrap;
    int gap_main  = clampdim(horiz ? cst->gap_col : cst->gap_row);
    int gap_cross = clampdim(horiz ? cst->gap_row : cst->gap_col);

    int cw = env.right - env.left;
    if (cw < 0) cw = 0;
    int top = pos.by;

    int base = fused;
    int count = 0;
    for (int c = html_first(n); c >= 0; c = html_next(c)) {
        if (blank_text(c)) continue;
        if (html_kind(c) == HN_ELEM) {
            int t = html_tag(c);
            if (t == HT_HEAD || t == HT_SCRIPT || t == HT_STYLE) continue;
        }
        struct css_style ist;
        style_of(c, in, &ist);
        if (ist.display == CSS_DISP_NONE) continue;
        if (ist.position == CSS_POS_ABSOLUTE || ist.position == CSS_POS_FIXED) {
            lay_abs(c, &ist, in);
            continue;
        }
        if (fused >= MAX_FITEMS) { dropped++; break; }
        struct fitem *it = &fpool[fused++];
        it->node = c;
        it->grow   = ist.grow   < 0 ? 0 : (ist.grow   > 10000 ? 10000 : ist.grow);
        it->shrink = ist.shrink < 0 ? 0 : (ist.shrink > 10000 ? 10000 : ist.shrink);
        it->align  = (ist.align_self == CSS_J_AUTO) ? cst->align_items : ist.align_self;
        it->minmain = 0;
        if (horiz && ist.min_w != CSS_AUTO && ist.min_w > 0) it->minmain = clampdim(ist.min_w);
        if (!horiz && ist.min_h != CSS_AUTO && ist.min_h > 0) it->minmain = clampdim(ist.min_h);
        it->cross = 0; it->target = 0;

        /* ---- pass 1: the hypothetical main size ---- */
        int hyp;
        if (ist.basis != CSS_AUTO) hyp = clampdim(ist.basis);
        else if (horiz && ist.width != CSS_AUTO) hyp = clampdim(ist.width);
        else if (!horiz && ist.height != CSS_AUTO) hyp = clampdim(ist.height);
        else if (passes >= LAY_MAX_PASSES) { dropped++; hyp = horiz ? cw : 0; }
        else if (horiz) {
            /* max-content: lay it out in a box wide enough that nothing wraps,
             * and ask how wide the result actually is */
            int probe = cw * 2 + em * 8;
            if (probe > LAY_MAX_DIM) probe = LAY_MAX_DIM;
            if (probe < 1) probe = 1;
            int a, b;
            sub_layout(c, in, &ist, probe, &a, &b, &hyp, 0);
        } else {
            int a, b;
            hyp = sub_layout(c, in, &ist, cw, &a, &b, 0, 0);
        }
        hyp = clampdim(hyp);
        if (horiz) {
            if (ist.max_w != CSS_AUTO && hyp > clampdim(ist.max_w)) hyp = clampdim(ist.max_w);
            if (it->minmain && hyp < it->minmain) hyp = it->minmain;
        } else {
            if (ist.max_h != CSS_AUTO && hyp > clampdim(ist.max_h)) hyp = clampdim(ist.max_h);
            if (it->minmain && hyp < it->minmain) hyp = it->minmain;
        }
        it->hyp = hyp;
        count++;
    }
    if (!count) { fused = base; return; }

    /* THE MAIN SIZE OF THE CONTAINER. For a row it is the content width, which
     * is always known. For a column it is the content height, which is only
     * known when the author set one - so an unconstrained column does not
     * grow, shrink or wrap, it just stacks. Saying so is better than inventing
     * a height and distributing space against it. */
    int main_size;
    if (horiz) main_size = cw;
    else if (impose_h >= 0) main_size = impose_h;
    else main_size = -1;

    /* ---- wrap into lines ---- */
    struct fline lines[MAX_FLINES];
    int nl = 0;
    if (wrap == CSS_NOWRAP || main_size < 0) {
        lines[0].first = base; lines[0].count = count;
        nl = 1;
    } else {
        int i = base, last = base + count;
        while (i < last && nl < MAX_FLINES) {
            int used = 0, k = 0;
            while (i + k < last) {
                int add = fpool[i + k].hyp + (k ? gap_main : 0);
                if (k > 0 && used + add > main_size) break;
                used += add;
                k++;
            }
            if (!k) k = 1;
            lines[nl].first = i; lines[nl].count = k;
            nl++;
            i += k;
        }
        if (i < last) {                /* more lines than the array holds */
            dropped++;
            lines[nl - 1].count += last - i;
        }
    }

    /* ---- pass 2: size, then measure the cross axis ---- */
    for (int L = 0; L < nl; L++) {
        struct fline *ln = &lines[L];
        int used = 0;
        for (int k = 0; k < ln->count; k++) used += fpool[ln->first + k].hyp;
        used += (ln->count > 1 ? (ln->count - 1) * gap_main : 0);
        int freem = (main_size < 0) ? 0 : main_size - used;

        for (int k = 0; k < ln->count; k++) fpool[ln->first + k].target = fpool[ln->first + k].hyp;

        if (freem > 0) {
            int sh[MAX_FITEMS > 64 ? 64 : MAX_FITEMS];
            int nn = ln->count > 64 ? 64 : ln->count;
            for (int k = 0; k < nn; k++) sh[k] = fpool[ln->first + k].grow;
            int total = normalize_shares(sh, nn, 32767);
            if (total > 0) {
                int given = 0;
                for (int k = 0; k < nn; k++) {
                    int add = freem * sh[k] / total;
                    fpool[ln->first + k].target += add;
                    given += add;
                }
                /* the rounding remainder goes to the last GROWING item, so a
                 * row of three items sharing 600px covers exactly 600 */
                for (int k = nn - 1; k >= 0; k--)
                    if (sh[k] > 0) { fpool[ln->first + k].target += freem - given; break; }
            }
        } else if (freem < 0) {
            int neg = -freem;
            int sf[MAX_FITEMS > 64 ? 64 : MAX_FITEMS];
            int nn = ln->count > 64 ? 64 : ln->count;
            /* SCALED SHRINK: an item shrinks in proportion to shrink * basis,
             * not to shrink alone - otherwise a 20px item and a 600px item
             * with the same shrink factor lose the same absolute amount and
             * the small one vanishes. The /100 turns hundredths into units and
             * keeps the product inside 32 bits. */
            for (int k = 0; k < nn; k++) {
                int s = fpool[ln->first + k].shrink;
                int h = fpool[ln->first + k].hyp;
                sf[k] = (s > 0 && h > 0) ? (s * h / 100) : 0;
                if (s > 0 && h > 0 && sf[k] == 0) sf[k] = 1;
            }
            int total = normalize_shares(sf, nn, 32767);
            if (total > 0) {
                for (int k = 0; k < nn; k++) {
                    int cut = neg * sf[k] / total;
                    struct fitem *it = &fpool[ln->first + k];
                    it->target -= cut;
                    if (it->target < it->minmain) it->target = it->minmain;
                    if (it->target < 0) it->target = 0;
                }
            }
            /* NOTHING COULD SHRINK AND IT STILL DOES NOT FIT. Real browsers
             * overflow here. This one has no horizontal scrolling, so the row
             * is scaled to fit instead - wrong per the spec, visible rather
             * than invisible, and consistent with every other clamp in this
             * file. */
            int now = 0;
            for (int k = 0; k < ln->count; k++) now += fpool[ln->first + k].target;
            now += (ln->count > 1 ? (ln->count - 1) * gap_main : 0);
            if (main_size >= 0 && now > main_size) {
                int room = main_size - (ln->count > 1 ? (ln->count - 1) * gap_main : 0);
                int sum = 0;
                for (int k = 0; k < ln->count; k++) sum += fpool[ln->first + k].target;
                if (room < 0) room = 0;
                if (sum > 0)
                    for (int k = 0; k < ln->count; k++)
                        fpool[ln->first + k].target = fpool[ln->first + k].target * room / sum;
            }
        }

        /* the cross size each item wants at the size it just got */
        int cross = 0;
        for (int k = 0; k < ln->count; k++) {
            struct fitem *it = &fpool[ln->first + k];
            int c;
            if (passes >= LAY_MAX_PASSES) { dropped++; c = line_height(in->size); }
            else if (horiz) {
                int a, b;
                c = sub_layout(it->node, in, 0, it->target > 0 ? it->target : 1, &a, &b, 0, 0);
            } else {
                int a, b, w = cw;
                sub_layout(it->node, in, 0, w > 0 ? w : 1, &a, &b, &c, 0);
            }
            it->cross = clampdim(c);
            if (it->cross > cross) cross = it->cross;
        }
        ln->cross = cross;
        ln->main = used;
    }

    /* ---- pass 3: place ---- */
    int cross_at = 0;
    for (int L = 0; L < nl; L++) {
        int idx = (wrap == CSS_WRAP_REVERSE) ? (nl - 1 - L) : L;
        struct fline *ln = &lines[idx];

        int sum = 0;
        for (int k = 0; k < ln->count; k++) sum += fpool[ln->first + k].target;
        int gaps = (ln->count > 1 ? (ln->count - 1) * gap_main : 0);
        int span = (main_size < 0) ? sum + gaps : main_size;
        int freem = span - sum - gaps;
        if (freem < 0) freem = 0;

        int start = 0, extra = 0;
        switch (cst->justify) {
        case CSS_J_END:     start = freem; break;
        case CSS_J_CENTER:  start = freem / 2; break;
        case CSS_J_BETWEEN: extra = (ln->count > 1) ? freem / (ln->count - 1) : 0; break;
        case CSS_J_AROUND: {
            int unit = ln->count ? freem / (2 * ln->count) : 0;
            start = unit; extra = 2 * unit;
            break;
        }
        case CSS_J_EVENLY: {
            int unit = freem / (ln->count + 1);
            start = unit; extra = unit;
            break;
        }
        default: break;                /* START, STRETCH and BASELINE: packed */
        }

        int mainpen = start;
        for (int k = 0; k < ln->count; k++) {
            int src = rev ? (ln->count - 1 - k) : k;
            struct fitem *it = &fpool[ln->first + src];

            int ic = it->cross;
            int off = 0;
            switch (it->align) {
            case CSS_J_END:     off = ln->cross - ic; break;
            case CSS_J_CENTER:  off = (ln->cross - ic) / 2; break;
            case CSS_J_STRETCH: ic = ln->cross; break;
            default: break;             /* START and BASELINE: at the edge */
            }
            if (off < 0) off = 0;

            int a, b;
            if (horiz) { impose_w = it->target; impose_h = ic; }
            else       { impose_w = ic;         impose_h = it->target; }
            int boxw = horiz ? it->target : ic;
            sub_layout(it->node, in, 0, boxw > 0 ? boxw : 1, &a, &b, 0, 1);

            int dx, dy;
            if (horiz) { dx = env.left + mainpen; dy = top + cross_at + off; }
            else       { dx = env.left + cross_at + off; dy = top + mainpen; }
            if (dx < 0) dx = 0;
            if (dy < 0) dy = 0;
            translate_range(a, b, dx, dy);
            clip_x_range(a, b, env.left, env.right);

            mainpen += (horiz ? it->target : it->target) + gap_main + extra;
        }
        cross_at += ln->cross + ((L + 1 < nl) ? gap_cross : 0);
    }

    /* the container's own extent */
    if (horiz) pos.by = top + cross_at;
    else {
        int widest = 0;
        for (int L = 0; L < nl; L++) widest += lines[L].cross;
        (void)widest;
        int tall = 0;
        for (int L = 0; L < nl; L++) {
            int sum = 0;
            for (int k = 0; k < lines[L].count; k++) sum += fpool[lines[L].first + k].target;
            sum += (lines[L].count > 1 ? (lines[L].count - 1) * gap_main : 0);
            if (sum > tall) tall = sum;
        }
        pos.by = top + tall;
    }
    fused = base;
}

/* ---- grid -------------------------------------------------------------------
 * BOUNDED ON PURPOSE, and the boundary is stated in the file header rather
 * than discovered: columns come from grid-template-columns, items flow into
 * them in order and wrap to a new row, and each row is as tall as its tallest
 * item. Named lines, grid-template-areas, explicit placement and spans have no
 * field in css.h, so they cannot arrive here at all - which is the right way
 * to refuse something, because there is no half-implemented version to meet.
 *
 * fr IS A SHARE OF WHAT IS LEFT, so it is resolved last: after the px and %
 * tracks and AFTER the gaps, which is the classic off-by-one. n columns have
 * n-1 gaps between them, not n. */
static void lay_grid(int n, const struct css_style *cst, const struct inh *in)
{
    int ncols = cst->n_grid_cols;
    if (ncols <= 0) ncols = 1;
    if (ncols > CSS_GRID_MAX) ncols = CSS_GRID_MAX;

    int cw = env.right - env.left;
    if (cw < 0) cw = 0;
    int gap_c = clampdim(cst->gap_col);
    int gap_r = clampdim(cst->gap_row);
    int top = pos.by;

    int gaps = (ncols > 1) ? (ncols - 1) * gap_c : 0;
    int avail = cw - gaps;
    if (avail < 0) avail = 0;

    /* collect the items first: an auto track is sized from the items that land
     * in it, and which items those are is only known once they are counted */
    int base = fused;
    int count = 0;
    for (int c = html_first(n); c >= 0; c = html_next(c)) {
        if (blank_text(c)) continue;
        if (html_kind(c) == HN_ELEM) {
            int t = html_tag(c);
            if (t == HT_HEAD || t == HT_SCRIPT || t == HT_STYLE) continue;
        }
        struct css_style ist;
        style_of(c, in, &ist);
        if (ist.display == CSS_DISP_NONE) continue;
        if (ist.position == CSS_POS_ABSOLUTE || ist.position == CSS_POS_FIXED) {
            lay_abs(c, &ist, in);
            continue;
        }
        if (count / ncols >= GRID_ROWS) { dropped++; break; }
        if (fused >= MAX_FITEMS) { dropped++; break; }
        struct fitem *it = &fpool[fused++];
        it->node = c;
        it->grow = it->shrink = 0;
        it->hyp = it->target = it->cross = 0;
        it->align = (ist.align_self == CSS_J_AUTO) ? cst->align_items : ist.align_self;
        it->minmain = 0;
        count++;
    }
    if (!count) { fused = base; return; }

    int colw[CSS_GRID_MAX], frv[CSS_GRID_MAX];
    int nfr = 0, fixed = 0;
    for (int i = 0; i < ncols; i++) {
        int v = (i < cst->n_grid_cols) ? cst->grid_col_v[i] : 0;
        int u = (i < cst->n_grid_cols) ? cst->grid_col_unit[i] : CSS_TRACK_AUTO;
        frv[i] = 0;
        switch (u) {
        case CSS_TRACK_PX:  colw[i] = clampdim(v); break;
        case CSS_TRACK_PCT: colw[i] = clampdim(avail) * (v < 0 ? 0 : (v > 1000 ? 1000 : v)) / 100; break;
        case CSS_TRACK_FR:  colw[i] = 0; frv[i] = v < 0 ? 0 : (v > 10000 ? 10000 : v); nfr++; break;
        default:            colw[i] = -1; break;      /* auto: measured below */
        }
        if (colw[i] > 0) fixed += colw[i];
    }

    /* auto tracks take their widest item, which is the only sense in which
     * `auto` differs from `1fr` for a document */
    for (int i = 0; i < ncols; i++) {
        if (colw[i] != -1) continue;
        int want = 0;
        for (int k = i; k < count; k += ncols) {
            if (passes >= LAY_MAX_PASSES) { dropped++; break; }
            int a, b, w = 0;
            int probe = avail > 0 ? avail : 1;
            sub_layout(fpool[base + k].node, in, 0, probe, &a, &b, &w, 0);
            if (w > want) want = w;
        }
        colw[i] = clampdim(want);
        fixed += colw[i];
    }

    /* they do not fit: scale down proportionally, exactly as lay_table does */
    if (fixed > avail && fixed > 0) {
        for (int i = 0; i < ncols; i++) colw[i] = colw[i] * avail / fixed;
        fixed = 0;
        for (int i = 0; i < ncols; i++) fixed += colw[i];
    }

    if (nfr) {
        int room = avail - fixed;
        if (room < 0) room = 0;
        int sh[CSS_GRID_MAX];
        for (int i = 0; i < ncols; i++) sh[i] = frv[i];
        int total = normalize_shares(sh, ncols, 32767);
        /* BOTH GUARDS, and mutation testing says only the inner one is load
         * bearing: normalize_shares rounds up, so total == 0 implies every
         * share is 0, and `if (!sh[i]) continue` already skips all of them.
         * Removing `total > 0` alone changes no behaviour and no assertion can
         * see it - that is an equivalent mutant, not a weak test. Removing
         * BOTH makes `grid-template-columns: 0fr 0fr` divide by zero and the
         * harness dies with SIGFPE, which is what proves the case is reached.
         * The outer guard stays: it costs one comparison and it is the one a
         * reader looks for. */
        if (total > 0) {
            int given = 0, lastfr = -1;
            for (int i = 0; i < ncols; i++) {
                if (!sh[i]) continue;
                colw[i] = room * sh[i] / total;
                given += colw[i];
                lastfr = i;
            }
            if (lastfr >= 0) colw[lastfr] += room - given;
        }
    }

    int colx[CSS_GRID_MAX];
    int x = env.left;
    for (int i = 0; i < ncols; i++) {
        if (colw[i] < 0) colw[i] = 0;
        if (x > env.right) x = env.right;
        if (x + colw[i] > env.right) colw[i] = env.right - x;
        if (colw[i] < 0) colw[i] = 0;
        colx[i] = x;
        x += colw[i] + gap_c;
    }

    int rowy = top;
    for (int r0 = 0; r0 < count; r0 += ncols) {
        int rowh = 0;
        /* measure the row, then place it: a row is as tall as its tallest
         * item, and align-items: stretch - the initial value for grid - needs
         * that number before any item's box can be closed */
        for (int k = 0; k < ncols && r0 + k < count; k++) {
            if (passes >= LAY_MAX_PASSES) { dropped++; continue; }
            int a, b;
            int h = sub_layout(fpool[base + r0 + k].node, in, 0,
                               colw[k] > 0 ? colw[k] : 1, &a, &b, 0, 0);
            fpool[base + r0 + k].cross = clampdim(h);
            if (fpool[base + r0 + k].cross > rowh) rowh = fpool[base + r0 + k].cross;
        }
        for (int k = 0; k < ncols && r0 + k < count; k++) {
            struct fitem *it = &fpool[base + r0 + k];
            int ih = it->cross, off = 0;
            switch (it->align) {
            case CSS_J_END:    off = rowh - ih; break;
            case CSS_J_CENTER: off = (rowh - ih) / 2; break;
            case CSS_J_STRETCH: ih = rowh; break;
            default: break;
            }
            if (off < 0) off = 0;
            int a, b;
            impose_w = colw[k]; impose_h = ih;
            sub_layout(it->node, in, 0, colw[k] > 0 ? colw[k] : 1, &a, &b, 0, 1);
            translate_range(a, b, colx[k], rowy + off);
            clip_x_range(a, b, colx[k], colx[k] + (colw[k] > 0 ? colw[k] : 0));
        }
        rowy += rowh;
        if (r0 + ncols < count) rowy += gap_r;
    }
    pos.by = rowy;
    fused = base;
}

/* ---- float ------------------------------------------------------------------
 * A float comes out of the block flow and becomes a rectangle the LINE
 * BREAKER has to route around. Everything that makes that work is already
 * here: sub_layout puts the element somewhere, measure_range says how big it
 * is, translate_range puts it where it belongs, and float_edges is what the
 * lines consult.
 *
 * WHEN THE ARRAY IS FULL IT STOPS TAKING FLOATS rather than scribbling, and
 * the element lays out in flow instead. That is a visible wrongness, which is
 * the failure mode to prefer over a silent one. */
static void lay_float(int n, const struct css_style *st, const struct inh *in)
{
    if (pos.nfloat >= MAX_FLOATS) {
        dropped++;
        force_inflow = 1;
        walk_node(n, in, st);
        return;
    }
    line_end();
    clear_floats(st->clearv);

    int avail = env.right - env.left;
    if (avail < 1) avail = 1;

    int w;
    if (st->width != CSS_AUTO) w = clampdim(st->width);
    else {
        int a, b;
        if (passes >= LAY_MAX_PASSES) { dropped++; w = avail; }
        else {
            force_inflow = 1;
            sub_layout(n, in, st, avail, &a, &b, &w, 0);
        }
    }
    if (st->max_w != CSS_AUTO && w > clampdim(st->max_w)) w = clampdim(st->max_w);
    if (st->min_w != CSS_AUTO && st->min_w > 0 && w < clampdim(st->min_w)) w = clampdim(st->min_w);
    /* WIDER THAN THE CONTENT BOX: clamped. A float that escapes its container
     * takes every line beside it with it. */
    if (w > avail) w = avail;
    if (w < 1) w = 1;

    int a, b;
    force_inflow = 1;
    int h = sub_layout(n, in, st, w, &a, &b, 0, 1);
    if (h < 1) h = 1;

    /* find the first y at which a w-wide box fits on the requested side */
    int y = pos.by, x = 0;
    for (int guard = 0; guard <= MAX_FLOATS; guard++) {
        int l, r;
        float_edges(y, h, &l, &r);
        if (r - l >= w || r - l >= env.right - env.left) {
            x = (st->floatv == CSS_FLOAT_LEFT) ? l : r - w;
            break;
        }
        int nb = next_float_bottom(y);
        if (nb <= y) { x = (st->floatv == CSS_FLOAT_LEFT) ? env.left : env.right - w; break; }
        y = nb;
    }
    if (x < env.left) x = env.left;
    if (x + w > env.right) x = env.right - w;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    translate_range(a, b, x, y);
    clip_x_range(a, b, x, x + w);

    struct frect *f = &floats[pos.nfloat++];
    f->x0 = x; f->x1 = x + w;
    f->y0 = y; f->y1 = y + h;
    f->side = st->floatv;
}

/* ---- out of the flow entirely -----------------------------------------------
 * `absolute` is measured against the nearest POSITIONED ancestor's box, which
 * flowenv already carries and restores for free. `fixed` is measured against
 * the viewport, which is right until the page scrolls - the runs are in
 * document coordinates and nothing here knows about a scroll position, so a
 * fixed element scrolls with the page. Saying so is better than pretending.
 *
 * WITH NO POSITIONED ANCESTOR the containing block is the whole document,
 * which is what lay_run_doc seeds flowenv with - so this case needs no code
 * and cannot be a null dereference.
 *
 * `bottom` and `right` need the ancestor's HEIGHT, and an ancestor that is
 * still open does not have one. What is used instead is how far it has got so
 * far, which is exact when the absolute element is the last thing in it and an
 * underestimate otherwise. Recorded here rather than discovered later. */
static void lay_abs(int n, const struct css_style *st, const struct inh *in)
{
    int cbx = env.cb_x, cby = env.cb_y, cbw = env.cb_w, cbh = env.cb_h;
    if (st->position == CSS_POS_FIXED) { cbx = 0; cby = 0; cbw = doc_w; cbh = -1; }
    if (cbw < 1) cbw = 1;

    int w;
    if (st->width != CSS_AUTO) w = clampdim(st->width);
    else if (passes >= LAY_MAX_PASSES) { dropped++; w = cbw; }
    else {
        int a, b;
        force_inflow = 1;
        sub_layout(n, in, st, cbw, &a, &b, &w, 0);
    }
    if (w > cbw) w = cbw;
    if (w < 1) w = 1;

    int a, b;
    force_inflow = 1;
    int h = sub_layout(n, in, st, w, &a, &b, 0, 1);
    /* see MAX_FROZEN: these runs are inside whatever line box is open, and
     * line_end() must not treat them as part of it */
    freeze_range(a, b);

    int x, y;
    if (st->left != CSS_AUTO)       x = cbx + st->left;
    else if (st->right != CSS_AUTO) x = cbx + cbw - w - st->right;
    else                            x = cbx;
    if (st->top != CSS_AUTO)         y = cby + st->top;
    else if (st->bottom != CSS_AUTO) {
        int base = (cbh >= 0) ? cbh : (pos.by - cby);
        if (base < 0) base = 0;
        y = cby + base - h - st->bottom;
    } else                           y = pos.by;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > doc_w) x = doc_w;
    translate_range(a, b, x, y);
    clip_x_range(a, b, 0, doc_w);
    /* it is OUT OF FLOW: pos.by does not move */
}

/* ---- tables ----------------------------------------------------------------
 * A REAL TWO-PASS TABLE, because the one-pass alternative does not work: a
 * column's width depends on every cell in it, including ones that have not
 * been read yet, so nothing can be placed until the whole table has been
 * measured. That is why this is a separate function rather than more cases in
 * walk().
 *
 *   pass 1  measure each cell's NATURAL width - the width it would take with
 *           no wrapping - and keep the widest per column
 *   pass 2  scale the columns to the content width and lay each cell out as an
 *           ordinary block flow inside its own column
 *
 * SCALED PROPORTIONALLY, NOT EQUALLY. Equal columns are one line of code and
 * they make a table of one long cell and four short ones unreadable. When the
 * natural widths already fit, they are used as-is and the table looks like the
 * author intended; only when they overflow are they scaled down, which is the
 * case that matters on a narrow window.
 *
 * WHAT IT DOES NOT DO: colspan, rowspan, borders, or per-cell alignment.
 * Wikipedia's infoboxes use colspan, so those cells will be narrower than the
 * author meant rather than spanning - wrong-looking, not broken, and the honest
 * limit for a first table implementation.
 */
#define TBL_COLS 16
#define TBL_ROWS 256

static int cell_natural(int n, int size, int style)
{
    /* the sum of this subtree's text at its own size - what it wants before
     * anything wraps it */
    int w = 0;
    for (; n >= 0; n = html_next(n)) {
        if (html_kind(n) == HN_TEXT) {
            int len;
            const char *t = html_text(n, &len);
            w += meas(t, len, size, style);
            continue;
        }
        int tag = html_tag(n);
        if (tag == HT_HEAD || tag == HT_SCRIPT || tag == HT_STYLE) continue;
        int cs = size, cy = style;
        if (tag == HT_STRONG || tag == HT_B || tag == HT_TH) cy |= LS_BOLD;
        if (tag == HT_EM || tag == HT_I) cy |= LS_ITALIC;
        if (tag == HT_CODE) cy |= LS_MONO;
        if (is_heading(tag)) { cs = head_size(tag); cy |= LS_BOLD; }
        w += cell_natural(html_first(n), cs, cy);
    }
    return w;
}

/* rows are HT_TR anywhere under the table - thead/tbody/tfoot are transparent */
static int collect_rows(int n, int *rows, int nrows, int max)
{
    for (; n >= 0; n = html_next(n)) {
        if (html_kind(n) != HN_ELEM) continue;
        int t = html_tag(n);
        if (t == HT_TR) {
            if (nrows < max) rows[nrows++] = n;
        } else if (t == HT_THEAD || t == HT_TBODY || t == HT_TFOOT) {
            nrows = collect_rows(html_first(n), rows, nrows, max);
        }
    }
    return nrows;
}

static void lay_table(int tnode, const struct inh *in)
{
    int rows[TBL_ROWS];
    int nrows = collect_rows(html_first(tnode), rows, 0, TBL_ROWS);
    if (nrows <= 0) return;

    int size = in->size, style = in->style;
    int colw[TBL_COLS];
    for (int i = 0; i < TBL_COLS; i++) colw[i] = 0;
    int ncols = 0;

    /* pass 1: the widest natural cell per column */
    for (int r = 0; r < nrows; r++) {
        int col = 0;
        for (int cnode = html_first(rows[r]); cnode >= 0 && col < TBL_COLS;
             cnode = html_next(cnode)) {
            if (html_kind(cnode) != HN_ELEM) continue;
            int ct = html_tag(cnode);
            if (ct != HT_TD && ct != HT_TH) continue;
            int cstyle = style | (ct == HT_TH ? LS_BOLD : 0);
            int w = cell_natural(html_first(cnode), size, cstyle);
            if (w > colw[col]) colw[col] = w;
            col++;
        }
        if (col > ncols) ncols = col;
    }
    if (ncols <= 0) return;

    int pad = size / 2 > 0 ? size / 2 : 1;
    int avail = env.right - env.left;
    int total = 0;
    for (int i = 0; i < ncols; i++) { if (colw[i] < size) colw[i] = size; total += colw[i] + pad; }

    /* scale down only if they do not already fit */
    if (total > avail && total > 0) {
        int room = avail - ncols * pad;
        if (room < ncols) room = ncols;
        int sum = 0;
        for (int i = 0; i < ncols; i++) sum += colw[i];
        if (sum <= 0) sum = 1;
        for (int i = 0; i < ncols; i++) {
            colw[i] = colw[i] * room / sum;
            if (colw[i] < size) colw[i] = size;
        }
    }

    int save_left = env.left, save_right = env.right;
    line_end();
    margin(size / 2);
    apply_margin();

    for (int r = 0; r < nrows; r++) {
        int row_top = pos.by, row_bottom = pos.by;
        int x = save_left, col = 0;
        for (int cnode = html_first(rows[r]); cnode >= 0 && col < ncols;
             cnode = html_next(cnode)) {
            if (html_kind(cnode) != HN_ELEM) continue;
            int ct = html_tag(cnode);
            if (ct != HT_TD && ct != HT_TH) continue;

            struct flowenv cellsave = env;
            env.left = x;
            env.right = x + colw[col];
            if (env.right > save_right) env.right = save_right;
            env.float_base = pos.nfloat;    /* a cell is its own context */
            pos.by = row_top;
            /* a cell is an ordinary block flow in its own column box */
            int item = 0;
            struct inh cin = *in;
            cin.style = style | (ct == HT_TH ? LS_BOLD : 0);
            cin.item = &item;
            cin.ordered = 0;
            cin.pre = 0;
            walk(html_first(cnode), &cin);
            line_end();
            env = cellsave;
            if (pos.by > row_bottom) row_bottom = pos.by;
            x += colw[col] + pad;
            col++;
        }
        pos.by = row_bottom;
        /* a hairline under every row but the last - without one a table reads
         * as a paragraph with strange spacing */
        if (r + 1 < nrows) {
            struct lay_run *rl = push_run();
            if (rl) {
                rl->kind = LR_RULE;
                rl->x = save_left; rl->y = pos.by;
                rl->w = save_right - save_left;
                rl->h = 1;
                rl->color = LC_DIM;
                rl->node = rows[r];
                pos.by += 1 + size / 4;
            }
        }
    }

    env.left = save_left;
    env.right = save_right;
    margin(size / 2);
}

/* ---- one element ------------------------------------------------------------ */
static void walk_node(int n, const struct inh *in, const struct css_style *pre)
{
    if (n < 0) return;

    if (html_kind(n) == HN_TEXT) {
        int len;
        const char *s = html_text(n, &len);
        /* MONOSPACE IS NOT PREFORMATTED, and conflating them is a bug this
         * had: <code> sets the same font as <pre> but is INLINE, so it must
         * wrap. Sharing one flag sent every <code> down the no-wrap path, and
         * inline code ran straight off the right edge - which every numeric
         * assertion missed and one look at a rendered page found immediately. */
        if (in->pre) emit_pre(s, len, in->size, in->style, in->color, n, in->link);
        else         emit_text(s, len, in->size, in->style, in->color, n, in->link);
        return;
    }

    int t = html_tag(n);

    /* These three render nothing and have no children worth walking, so they
     * are handled before the path is pushed - which keeps the push and the pop
     * adjacent, and that is what stops the path leaking on an early exit. */
    if (t == HT_HEAD || t == HT_SCRIPT || t == HT_STYLE) return;

    /* HOSTILE MARKUP HAS A DEPTH. html.c caps its tree, but sub_layout re-enters
     * this function without adding tree depth, so the guard belongs here too.
     * Refused and recorded, never scribbled. */
    if (wdepth >= LAY_DEPTH_MAX) { dropped++; return; }
    wdepth++;

    int inflow_only = force_inflow;
    force_inflow = 0;

    struct flowenv save = env;
    struct inh cin = *in;
    int block = html_is_block(t);
    int my_item = 0;
    int pushed = 0;

    if (cdepth < CSS_PATH_MAX) {
        cpath[cdepth].tag = html_tagname(n, &cpath[cdepth].tag_len);
        cpath[cdepth].id  = html_id(n, &cpath[cdepth].id_len);
        cpath[cdepth].cls = html_class(n, &cpath[cdepth].cls_len);
        cdepth++;
        pushed = 1;
    }

    switch (t) {
    case HT_BR:
        line_begin();
        if (pos.line_h < line_height(in->size)) pos.line_h = line_height(in->size);
        line_end();
        goto done;
    case HT_A:
        cin.color = LC_ACCENT;
        cin.style = in->style | LS_UNDER;
        cin.link = n;
        break;
    case HT_STRONG: case HT_B:
        cin.style = in->style | LS_BOLD;
        break;
    case HT_EM: case HT_I:
        cin.style = in->style | LS_ITALIC;
        break;
    case HT_CODE:
        cin.style = in->style | LS_MONO;
        break;
    default:
        break;
    }

    /* ---- the cascade, over the built-in rules ---------------------------
     * Seeded from the INHERITED values, so a property no rule mentions is
     * inherited rather than reset - which is what `has` is for. Computed here
     * but applied AFTER the block rules below, because those set a heading's
     * size and `h1 { font-size: 1.2em }` has to beat head_size(). */
    struct css_style st;
    if (pre) st = *pre;
    else {
        css_style_init(&st, in->size);
        st.rgb = env.rgb; st.bg = env.bg;
        st.size = in->size;
        st.bold      = (in->style & LS_BOLD)   != 0;
        st.italic    = (in->style & LS_ITALIC) != 0;
        st.mono      = (in->style & LS_MONO)   != 0;
        st.underline = (in->style & LS_UNDER)  != 0;
        st.align = env.align;
        st.display = block ? CSS_DISP_BLOCK : CSS_DISP_INLINE;
        {
            int slen;
            const char *sattr = html_style_attr(n, &slen);
            int pw = env.right - env.left;
            css_compute(cpath, cdepth, in->size, pw > 0 ? pw : 1, sattr, slen, &st);
        }
    }

    /* display:none removes the element AND its subtree from the flow - before
     * any margin or line break it would otherwise have caused. */
    if (st.display == CSS_DISP_NONE) goto done;

    /* ---- out of the flow, before anything reserves space for it ---------- */
    if (!inflow_only) {
        if (st.position == CSS_POS_ABSOLUTE || st.position == CSS_POS_FIXED) {
            lay_abs(n, &st, &cin);
            goto done;
        }
        if (st.floatv != CSS_FLOAT_NONE && t != HT_BR) {
            lay_float(n, &st, &cin);
            goto done;
        }
    }

    /* <img> is an atom: it has a size and no children */
    if (t == HT_IMG) {
        emit_img(n, &st, &cin);
        goto done;
    }
    if (t == HT_HR) {
        line_end();
        margin(em * 3 / 4);
        apply_margin();
        struct lay_run *r = push_run();
        if (r) {
            r->kind = LR_RULE;
            r->x = env.left; r->y = pos.by; r->w = env.right - env.left;
            r->h = em / 8 > 0 ? em / 8 : 1;
            r->color = LC_DIM;
            r->node = n;
            pos.by += r->h;
        }
        margin(em * 3 / 4);
        goto done;
    }
    if (t == HT_TABLE) {
        lay_table(n, &cin);
        goto done;
    }

    /* ---- the built-in per-tag block metrics ------------------------------
     * Unchanged, and deliberately: these are what every existing assertion in
     * htmltest is written against, and a stylesheet still overrides them the
     * way it always did. */
    if (is_heading(t)) {
        line_end();
        cin.size = head_size(t);
        cin.style = in->style | LS_BOLD;
        margin(cin.size * 2 / 3);
    } else if (t == HT_P) {
        line_end();
        margin(em * 3 / 4);
    } else if (t == HT_PRE) {
        line_end();
        margin(em * 3 / 4);
        cin.style = in->style | LS_MONO;
        cin.pre = 1;
        indent(em / 2);
    } else if (t == HT_UL || t == HT_OL) {
        line_end();
        margin(em / 2);
        indent(em * 3 / 2);
        cin.item = &my_item;
        cin.ordered = (t == HT_OL);
    } else if (t == HT_LI) {
        line_end();
        margin(em / 6);
    }

    /* ---- now the stylesheet wins ---------------------------------------
     * Only properties a rule actually SET are applied; everything else keeps
     * the inherited value it was seeded with. */
    if (st.has & CSS_P_SIZE)   cin.size = st.size > 0 ? st.size : cin.size;
    if (st.has & CSS_P_WEIGHT) cin.style = st.bold      ? (cin.style | LS_BOLD)   : (cin.style & ~LS_BOLD);
    if (st.has & CSS_P_STYLE)  cin.style = st.italic    ? (cin.style | LS_ITALIC) : (cin.style & ~LS_ITALIC);
    if (st.has & CSS_P_FAMILY) cin.style = st.mono      ? (cin.style | LS_MONO)   : (cin.style & ~LS_MONO);
    if (st.has & CSS_P_DECOR)  cin.style = st.underline ? (cin.style | LS_UNDER)  : (cin.style & ~LS_UNDER);
    if (st.has & CSS_P_COLOR)  env.rgb   = st.rgb;
    if (st.has & CSS_P_BG)     env.bg    = st.bg;
    if (st.has & CSS_P_ALIGN)  env.align = st.align;

    if (st.display == CSS_DISP_BLOCK || st.display == CSS_DISP_FLEX ||
        st.display == CSS_DISP_INLINE_FLEX || st.display == CSS_DISP_GRID)
        block = 1;

    if (!block) {
        /* AN INLINE ELEMENT RESTORES ITS EDGES TOO, and until now it did not:
         * padding-left on a <span> called indent(), and only the `block`
         * branch put `left` back. So one styled span shifted the left edge of
         * every remaining line in its parent, and the further down the page
         * the worse it got. `env = save` at the single exit is what makes that
         * unrepresentable rather than remembered. */
        if (st.has & CSS_P_PAD_L)    indent(st.pad_l);
        if (st.has & CSS_P_MARGIN_L) indent(st.margin_l);
        walk(html_first(n), &cin);
        goto done;
    }

    /* ---- a block-level box ---------------------------------------------- */
    line_end();
    clear_floats(st.clearv);

    /* A margin the author gave REPLACES the built-in one rather than maxing
     * with it - margin() collapses, and `p { margin: 0 }` has to be able to
     * CLOSE a gap, not merely fail to widen it. */
    if (st.has & CSS_P_MARGIN_T) pos.pend_m = st.margin_t > 0 ? st.margin_t : 0;

    {
        int iw = impose_w, ih = impose_h;
        impose_w = impose_h = -1;

        struct blockbox bb;
        block_open(&st, &bb, iw);

        if (t == HT_LI) {
            apply_margin();
            line_begin();
            /* the marker HANGS in the indent gutter, which is what makes a
             * wrapped list item's second line align with its first */
            if (in->ordered && in->item) {
                int v = ++(*in->item);
                int start = gused, cnt = 0;
                char tmp[12];
                int k = 0;
                if (v <= 0) tmp[k++] = '0';
                while (v > 0 && k < 10) { tmp[k++] = (char)('0' + v % 10); v /= 10; }
                if (gused + k + 2 < GEN_SIZE) {
                    for (int q = k - 1; q >= 0; q--) { gen[gused++] = tmp[q]; cnt++; }
                    gen[gused++] = '.'; cnt++;
                } else start = -1;
                if (start >= 0) {
                    /* The marker hangs in the gutter and was the one thing
                     * placed with no width check at all - in a box narrower
                     * than "10." it was the last run still escaping. Trimmed
                     * like everything else rather than exempted. */
                    int mx = env.left - em * 5 / 4;
                    if (mx < 0) mx = 0;
                    int room_m = env.right - mx;
                    if (room_m > 0 && meas(gen + start, cnt, in->size, in->style) > room_m)
                        cnt = fit_chars(gen + start, cnt, in->size, in->style, room_m);
                    if (cnt < 0) cnt = 0;
                    int w = meas(gen + start, cnt, in->size, in->style);
                    struct lay_run *r = push_run();
                    if (r) {
                        r->kind = LR_TEXT;
                        r->x = mx;
                        r->y = pos.fy;
                        r->w = w; r->h = line_height(in->size);
                        r->text = gen + start; r->len = cnt;
                        r->size = in->size; r->style = in->style; r->color = LC_DIM;
                        r->node = n; r->link = in->link;
                        if (r->h > pos.line_h) pos.line_h = r->h;
                    }
                }
            } else {
                int d = em / 3 > 0 ? em / 3 : 1;
                struct lay_run *r = push_run();
                if (r) {
                    r->kind = LR_BULLET;
                    r->x = env.left - em;
                    r->y = pos.fy + (line_height(in->size) - d) / 2;
                    r->w = d; r->h = d;
                    r->color = LC_DIM;
                    r->node = n;
                    if (pos.line_h < line_height(in->size)) pos.line_h = line_height(in->size);
                    if (r->x < 0) r->x = 0;
                }
            }
        }

        if (st.display == CSS_DISP_FLEX || st.display == CSS_DISP_INLINE_FLEX) {
            /* A COLUMN FLEX CONTAINER'S MAIN SIZE IS ITS CONTENT HEIGHT, and
             * it only has one when somebody said so - the element's own
             * `height`, or a height imposed on it as an item of an outer flex
             * or grid. Without one there is no free space to distribute, so a
             * column stacks at its items' hypothetical sizes and neither grows
             * nor shrinks. Inventing a height here and distributing against it
             * is how a column of three cards ends up 3000 pixels tall. */
            if (bb.m.have_h) impose_h = bb.m.ch;
            else if (ih >= 0) impose_h = ih - bb.m.bt - bb.m.bb - bb.m.pt - bb.m.pb;
            else impose_h = -1;
            lay_flex(n, &st, &cin);
            impose_h = -1;
        } else if (st.display == CSS_DISP_GRID) {
            lay_grid(n, &st, &cin);
        } else {
            walk(html_first(n), &cin);
        }

        block_close(&st, &bb, ih);

        /* RELATIVE IS A TRANSLATE OF A RUN RANGE, which is exactly the
         * primitive this file is built on - so it is four lines rather than a
         * second positioning model. The offset does not affect the flow, which
         * is the whole point of `relative`. */
        if (st.position == CSS_POS_RELATIVE || st.position == CSS_POS_STICKY) {
            int dx = 0, dy = 0;
            if (st.left != CSS_AUTO)       dx = st.left;
            else if (st.right != CSS_AUTO) dx = -st.right;
            if (st.top != CSS_AUTO)         dy = st.top;
            else if (st.bottom != CSS_AUTO) dy = -st.bottom;
            if (dx || dy) {
                int x0, y0;
                measure_range(bb.rall, nruns, &x0, &y0, 0, 0);
                if (x0 + dx < 0) dx = -x0;
                if (y0 + dy < 0) dy = -y0;
                translate_range(bb.rall, nruns, dx, dy);
                clip_x_range(bb.rall, nruns, 0, doc_w);
            }
        }
    }

    /* the trailing margin. Same rule as the leading one. */
    if (st.has & CSS_P_MARGIN_B) pos.pend_m = st.margin_b > 0 ? st.margin_b : 0;
    else if (is_heading(t)) margin(cin.size / 2);
    else if (t == HT_P)     margin(em * 3 / 4);
    else if (t == HT_PRE)   margin(em * 3 / 4);
    else if (t == HT_UL || t == HT_OL) margin(em / 2);
    else if (t == HT_LI)    margin(em / 6);

done:
    cdepth -= pushed;
    wdepth--;
    /* THE RESTORE HAPPENS AFTER line_end(), NOT BEFORE IT. This block's last
     * line box is closed by block_close() above, and line_end() is what
     * applies text-align to it - restoring env.align first aligned that line
     * with the PARENT's setting, so `p { text-align: center }` centred nothing
     * while its colour applied correctly. Found end to end, not by a unit
     * test: every per-property assertion passed.
     *
     * One assignment, every field. See the header. */
    env = save;
}

static void walk(int n, const struct inh *in)
{
    for (; n >= 0; n = html_next(n)) walk_node(n, in, 0);
}

int lay_run_doc(int width, int base)
{
    nruns = 0;
    dropped = 0;
    gused = 0;
    nlines = 0;
    passes = 0;
    fused = 0;
    cdepth = 0;
    wdepth = 0;
    impose_w = impose_h = -1;
    force_inflow = 0;

    doc_w = width > 0 ? width : 1;
    em = base > 0 ? base : 16;

    pos.by = 0;
    pos.fx = 0; pos.fy = 0;
    pos.line_h = 0; pos.line_start = 0; pos.line_open = 0;
    pos.pend_m = 0;
    pos.ll = 0; pos.lr = doc_w;
    pos.nfloat = 0;
    pos.nfrozen = 0;

    env.left = 0; env.right = doc_w;
    env.rgb = LR_NO_RGB; env.bg = LR_NO_RGB;
    env.align = CSS_ALIGN_LEFT;
    env.float_base = 0;
    env.frozen_base = 0;
    /* the INITIAL containing block, so `position: absolute` with no positioned
     * ancestor has one rather than being a special case */
    env.cb_x = 0; env.cb_y = 0; env.cb_w = doc_w; env.cb_h = -1;

    int root = html_root();
    if (root >= 0) {
        struct inh in;
        in.size = em; in.style = 0; in.color = LC_TEXT; in.link = -1;
        in.item = 0; in.ordered = 0; in.pre = 0;
        walk(root, &in);
    }
    line_end();
    int fb = float_bottom_all();
    if (fb > pos.by) pos.by = fb;
    total_h = pos.by;
    return total_h;
}

int lay_count(void)      { return nruns; }
int lay_height(void)     { return total_h; }
int lay_lines(void)      { return nlines; }
int lay_overflowed(void) { return dropped; }

const struct lay_run *lay_at(int i)
{
    if ((unsigned)i >= (unsigned)nruns) return 0;
    return &runs[i];
}
