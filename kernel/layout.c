/* layout.c - the box model: two box types, one flowing cursor.
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
 * TWO BOX TYPES, and no more:
 *   BLOCK   stacks vertically, takes the full content width, has margins
 *   INLINE  flows along a line and wraps at the content width
 * That is enough for a document. It is not enough for a web app, and the
 * difference between those two is float, flex, grid and positioning - i.e.
 * most of CSS. Declaring the boundary is more useful than pretending it is
 * further out than it is.
 *
 * MARGIN COLLAPSING IS HERE and it is four lines: adjacent vertical margins
 * take the larger of the two rather than adding. It is included because
 * without it every gap in the document is the sum of two arbitrary numbers and
 * nothing lines up; it is not the full CSS rule (which also collapses through
 * empty parents).
 *
 * NO PIXELS. Not one call into fb.c, by design - see layout.h. The output is
 * an array of runs, and the same array is what the host harness asserts
 * against and what browser.c paints.
 */

#include "layout.h"
#include "html.h"

#define MAX_RUNS 2048
#define GEN_SIZE 256

static struct lay_run runs[MAX_RUNS];
static int nruns, dropped;
static int doc_w, em, total_h, nlines;
static lay_measure_fn measure;

/* generated text - list markers, which exist in no source document and so
 * cannot point into html.c's arena */
static char gen[GEN_SIZE];
static int  gused;

/* ---- the cursor ----------------------------------------------------------- */
static int by;            /* block cursor: the y everything stacks at   */
static int left, right;   /* the content edges, moved by indentation    */
static int fx;            /* inline pen                                 */
static int fy;            /* top of the line box being built            */
static int line_h;        /* the tallest run on it                      */
static int line_start;    /* index of its first run                     */
static int line_open;
static int pend_m;        /* the collapsed margin waiting to be applied */

void lay_set_measure(lay_measure_fn f) { measure = f; }

static int meas(const char *s, int len, int size, int style)
{
    if (!measure || len <= 0) return 0;
    return measure(s, len, size, style);
}

/* NEVER ZERO. Integer division makes a small em round a size away entirely -
 * h6 is em*9/10, which is 0 for any em below 2 - and a zero line height makes
 * a run taller than the line box that contains it, so line_end computes a
 * NEGATIVE y and the document lays out above its own origin. Not reachable
 * with the kernel's 16 and 32 pixel fonts; found by the fuzzer, which picks
 * sizes nobody would type. A floor costs one comparison. */
static int line_height(int size) { int h = size * 5 / 4; return h > 0 ? h : 1; }

/* A margin is not applied when it is declared - it is applied when something
 * finally needs to be drawn, which is what lets two of them collapse. The
 * leading margin of the whole document is dropped: a page must not open with
 * a gap above its first heading. */
static void apply_margin(void)
{
    if (nruns > 0) by += pend_m;
    pend_m = 0;
}

static void margin(int m) { if (m > pend_m) pend_m = m; }

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
    int max_left = right - floor_w;
    if (max_left < 0) max_left = 0;
    left += px;
    if (left > max_left) left = max_left;
    if (left < 0) left = 0;
}

static void line_begin(void)
{
    if (line_open) return;
    apply_margin();
    line_open = 1;
    line_start = nruns;
    fy = by;
    line_h = 0;
    fx = left;
}

/* Close the current line box. Runs of different sizes on one line are
 * BOTTOM-aligned within it, which is a baseline alignment in disguise: every
 * glyph here comes from one atlas, so ascent is the same fraction of the cell
 * at every size and aligning the cell bottoms aligns the baselines. It would
 * be wrong the moment a second font with different metrics arrived. */
static void line_end(void)
{
    if (!line_open) return;
    for (int i = line_start; i < nruns; i++)
        runs[i].y = fy + line_h - runs[i].h;
    by = fy + line_h;
    nlines++;
    line_open = 0;
}

static struct lay_run *push_run(void)
{
    if (nruns >= MAX_RUNS) { dropped++; return 0; }
    struct lay_run *r = &runs[nruns++];
    r->kind = LR_TEXT;
    r->x = r->y = r->w = r->h = 0;
    r->text = ""; r->len = 0;
    r->size = em; r->style = 0; r->color = LC_TEXT;
    r->node = -1; r->link = -1;
    return r;
}

/* Place something of a known size on the current line, wrapping first if it
 * does not fit. The ONE function that moves the inline pen. */
static struct lay_run *place(int w, int h)
{
    line_begin();
    if (fx + w > right && fx > left) {
        line_end();
        line_begin();
    }
    struct lay_run *r = push_run();
    if (!r) return 0;
    r->x = fx;
    r->y = fy;                       /* provisional; line_end aligns it */
    r->w = w;
    r->h = h;
    fx += w;
    if (h > line_h) line_h = h;
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
    for (;;) {
        int w = meas(s, len, size, style);
        int room = right - left;
        if (w <= room) {
            struct lay_run *r = place(w, h);
            if (!r) return;
            r->kind = LR_TEXT;
            r->text = s; r->len = len;
            r->size = size; r->style = style; r->color = color;
            r->node = node; r->link = link;
            return;
        }
        /* longer than any line can be: break it, and keep going with the rest */
        line_begin();
        int avail = right - fx;
        if (avail < room / 4) { line_end(); line_begin(); avail = right - fx; }
        int n = fit_chars(s, len, size, style, avail);
        if (n >= len) n = len - 1;
        if (n < 1) n = 1;
        struct lay_run *r = place(meas(s, n, size, style), h);
        if (!r) return;
        r->kind = LR_TEXT;
        r->text = s; r->len = n;
        r->size = size; r->style = style; r->color = color;
        r->node = node; r->link = link;
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
            if (line_open && fx > left) fx += sw;
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
                int avail = right - fx;
                if (avail <= 0) { line_end(); line_begin(); avail = right - fx; }
                int take = j - at;
                if (meas(s + at, take, size, style) > avail) {
                    take = fit_chars(s + at, take, size, style, avail);
                    if (take < 1) take = 1;
                }
                struct lay_run *r = push_run();
                if (!r) break;
                r->kind = LR_TEXT;
                r->x = fx; r->y = fy;
                r->w = meas(s + at, take, size, style);
                r->h = line_height(size);
                r->text = s + at; r->len = take;
                r->size = size; r->style = style; r->color = color;
                r->node = node; r->link = link;
                fx += r->w;
                if (r->h > line_h) line_h = r->h;
                at += take;
                if (at < j) line_end();          /* the box edge is a break */
            }
        } else if (j < len) {
            line_begin();
            if (line_h < line_height(size)) line_h = line_height(size);
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
 * so they are parameters. Block properties (the content edges) are state,
 * because a block restores them on the way out.
 */
static void walk(int n, int size, int style, int color, int link,
                 int *item, int ordered, int pre)
{
    for (; n >= 0; n = html_next(n)) {
        if (html_kind(n) == HN_TEXT) {
            int len;
            const char *s = html_text(n, &len);
            /* MONOSPACE IS NOT PREFORMATTED, and conflating them is a bug this
             * had: <code> sets the same font as <pre> but is INLINE, so it
             * must wrap. Sharing one flag sent every <code> down the no-wrap
             * path, and inline code ran straight off the right edge - which
             * every numeric assertion missed and one look at a rendered page
             * found immediately. */
            if (pre) emit_pre(s, len, size, style, color, n, link);
            else     emit_text(s, len, size, style, color, n, link);
            continue;
        }

        int t = html_tag(n);
        int csize = size, cstyle = style, ccolor = color, clink = link;
        int save_left = left, save_right = right;
        int block = html_is_block(t);
        int my_item = 0;
        int child_ordered = ordered;
        int *child_item = item;
        int child_pre = pre;

        switch (t) {
        case HT_HEAD:
            continue;                                   /* never rendered */
        case HT_SCRIPT: case HT_STYLE:
            continue;
        case HT_BR:
            line_begin();
            if (line_h < line_height(size)) line_h = line_height(size);
            line_end();
            continue;
        case HT_HR: {
            line_end();
            margin(em * 3 / 4);
            apply_margin();
            struct lay_run *r = push_run();
            if (r) {
                r->kind = LR_RULE;
                r->x = left; r->y = by; r->w = right - left;
                r->h = em / 8 > 0 ? em / 8 : 1;
                r->color = LC_DIM;
                r->node = n;
                by += r->h;
            }
            margin(em * 3 / 4);
            continue;
        }
        case HT_IMG: {
            /* No decoder, and saying so is better than a blank space. The
             * placeholder is a real box in the flow, so the text around it
             * wraps as it would around a picture. */
            int box = em * 3 / 2;
            struct lay_run *r = place(box, box);
            if (r) { r->kind = LR_IMG; r->node = n; r->color = LC_DIM; }
            int alen;
            const char *alt = html_href(n, &alen);
            if (alen) {
                if (line_open && fx > left) fx += meas(" ", 1, size, style);
                emit_text(alt, alen, size, style | LS_ITALIC, LC_DIM, n, link);
            }
            continue;
        }
        case HT_A:
            ccolor = LC_ACCENT;
            cstyle = style | LS_UNDER;
            clink = n;
            break;
        case HT_STRONG: case HT_B:
            cstyle = style | LS_BOLD;
            break;
        case HT_EM: case HT_I:
            cstyle = style | LS_ITALIC;
            break;
        case HT_CODE:
            cstyle = style | LS_MONO;
            break;
        default:
            break;
        }

        if (is_heading(t)) {
            line_end();
            csize = head_size(t);
            cstyle = style | LS_BOLD;
            margin(csize * 2 / 3);
        } else if (t == HT_P) {
            line_end();
            margin(em * 3 / 4);
        } else if (t == HT_PRE) {
            line_end();
            margin(em * 3 / 4);
            cstyle = style | LS_MONO;
            child_pre = 1;
            indent(em / 2);
        } else if (t == HT_UL || t == HT_OL) {
            line_end();
            margin(em / 2);
            indent(em * 3 / 2);
            my_item = 0;
            child_item = &my_item;
            child_ordered = (t == HT_OL);
        } else if (t == HT_LI) {
            line_end();
            margin(em / 6);
            apply_margin();
            line_begin();
            /* the marker HANGS in the indent gutter, which is what makes a
             * wrapped list item's second line align with its first */
            if (ordered && item) {
                int v = ++(*item);
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
                    int mx = left - em * 5 / 4;
                    if (mx < 0) mx = 0;
                    int room_m = right - mx;
                    if (room_m > 0 && meas(gen + start, cnt, size, style) > room_m)
                        cnt = fit_chars(gen + start, cnt, size, style, room_m);
                    if (cnt < 0) cnt = 0;
                    int w = meas(gen + start, cnt, size, style);
                    struct lay_run *r = push_run();
                    if (r) {
                        r->kind = LR_TEXT;
                        r->x = mx;
                        r->y = fy;
                        r->w = w; r->h = line_height(size);
                        r->text = gen + start; r->len = cnt;
                        r->size = size; r->style = style; r->color = LC_DIM;
                        r->node = n; r->link = link;
                        if (r->h > line_h) line_h = r->h;
                    }
                }
            } else {
                int d = em / 3 > 0 ? em / 3 : 1;
                struct lay_run *r = push_run();
                if (r) {
                    r->kind = LR_BULLET;
                    r->x = left - em;
                    r->y = fy + (line_height(size) - d) / 2;
                    r->w = d; r->h = d;
                    r->color = LC_DIM;
                    r->node = n;
                    if (line_h < line_height(size)) line_h = line_height(size);
                    if (r->x < 0) r->x = 0;
                }
            }
        } else if (block) {
            line_end();
        }

        walk(html_first(n), csize, cstyle, ccolor, clink,
             child_item, child_ordered, child_pre);

        if (block) {
            line_end();
            left = save_left;
            right = save_right;
            if (is_heading(t))      margin(csize / 2);
            else if (t == HT_P)     margin(em * 3 / 4);
            else if (t == HT_PRE)   margin(em * 3 / 4);
            else if (t == HT_UL || t == HT_OL) margin(em / 2);
            else if (t == HT_LI)    margin(em / 6);
        }
    }
}

int lay_run_doc(int width, int base)
{
    nruns = 0;
    dropped = 0;
    gused = 0;
    nlines = 0;
    by = 0;
    pend_m = 0;
    line_open = 0;
    line_h = 0;
    line_start = 0;
    doc_w = width > 0 ? width : 1;
    em = base > 0 ? base : 16;
    left = 0;
    right = doc_w;
    fx = 0;
    fy = 0;

    int root = html_root();
    if (root >= 0) walk(root, em, 0, LC_TEXT, -1, 0, 0, 0);
    line_end();
    total_h = by;
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
