/* css.h - a stylesheet, and what it computes to for one element.
 *
 * SCOPE, stated up front because a CSS engine is the classic unbounded task
 * and this one is deliberately not. It supports what a DOCUMENT uses: type,
 * class, id, universal and descendant selectors, comma-separated groups, and
 * the properties layout.c can actually act on. No media queries beyond
 * skipping them, and no pseudo-classes.
 *
 * ~~No floats, no positioning, no flexbox, no grid. Those are not "coming
 * later" - they are outside the bounded version of this capability.~~
 *
 * THAT PARAGRAPH WAS WRONG IN THE ONE DIRECTION THIS PROJECT HAS ALREADY HAD
 * TO CORRECT ONCE. It named the maximal version of a capability and reported
 * it as the capability - the same mistake `browser-status.md` documents about
 * "a browser is unbounded", and the same mistake DECISIONS.md #26 records
 * about "95% achievable". Flex and grid have SPECIFICATIONS: they are finite,
 * they are the way modern pages are arranged, and without them Wikipedia's
 * portal renders as one giant stacked word. What is genuinely unbounded is
 * PIXEL PARITY WITH CHROME, which is a different claim and is still refused.
 *
 * So the scope now includes the box properties (width, height, min/max,
 * border, box-sizing), float and clear, position, flexbox and a bounded grid.
 * The text above is kept rather than deleted because the SHAPE of the mistake
 * is the reusable part.
 *
 * NO HEAP, like everything else here: rules, selectors and declarations live
 * in fixed arrays and every string is an offset into one arena. Overflow
 * STOPS TAKING RULES and says so through css_overflowed(); it never scribbles.
 *
 * DECOUPLED FROM html.c ON PURPOSE. Matching takes an explicit ancestor path
 * of {tag, id, class} rather than a tree, so the whole engine is testable with
 * no parser, no layout, no framebuffer and no kernel - which is the same rule
 * htmltest and fbtext already follow. layout.c builds the path as it walks.
 */
#ifndef CSS_H
#define CSS_H

/* which properties a computed style actually had set - the cascade needs to
 * know "unset" from "set to the default", or a child inherits a value its
 * parent never specified */
#define CSS_P_COLOR      (1 << 0)
#define CSS_P_BG         (1 << 1)
#define CSS_P_SIZE       (1 << 2)
#define CSS_P_WEIGHT     (1 << 3)
#define CSS_P_STYLE      (1 << 4)
#define CSS_P_FAMILY     (1 << 5)
#define CSS_P_ALIGN      (1 << 6)
#define CSS_P_DECOR      (1 << 7)
#define CSS_P_DISPLAY    (1 << 8)
#define CSS_P_MARGIN_T   (1 << 9)
#define CSS_P_MARGIN_B   (1 << 10)
#define CSS_P_MARGIN_L   (1 << 11)
#define CSS_P_MARGIN_R   (1 << 12)
#define CSS_P_PAD_L      (1 << 13)
#define CSS_P_PAD_R      (1 << 14)
#define CSS_P_PAD_T      (1 << 15)
#define CSS_P_PAD_B      (1 << 16)

/* THE SECOND BITMASK. `has` above is an int and seventeen of its usable
 * thirty-one bits were already spent; the box, float, position, flex and grid
 * properties need about thirty more. Splitting into a second word is cheaper
 * and far less error-prone than renumbering every CSS_P_* that layout.c and
 * csstest.c already name. `has2` is unsigned so bit 31 is not a sign bit. */
#define CSS_Q_WIDTH      (1u << 0)
#define CSS_Q_HEIGHT     (1u << 1)
#define CSS_Q_MIN_W      (1u << 2)
#define CSS_Q_MAX_W      (1u << 3)
#define CSS_Q_MIN_H      (1u << 4)
#define CSS_Q_MAX_H      (1u << 5)
#define CSS_Q_BOXSIZING  (1u << 6)
#define CSS_Q_BORDER_T   (1u << 7)
#define CSS_Q_BORDER_R   (1u << 8)
#define CSS_Q_BORDER_B   (1u << 9)
#define CSS_Q_BORDER_L   (1u << 10)
#define CSS_Q_BORDER_RGB (1u << 11)
#define CSS_Q_RADIUS     (1u << 12)
#define CSS_Q_FLOAT      (1u << 13)
#define CSS_Q_CLEAR      (1u << 14)
#define CSS_Q_POSITION   (1u << 15)
#define CSS_Q_TOP        (1u << 16)
#define CSS_Q_RIGHT      (1u << 17)
#define CSS_Q_BOTTOM     (1u << 18)
#define CSS_Q_LEFT       (1u << 19)
#define CSS_Q_FLEX_DIR   (1u << 20)
#define CSS_Q_FLEX_WRAP  (1u << 21)
#define CSS_Q_JUSTIFY    (1u << 22)
#define CSS_Q_ALIGN_IT   (1u << 23)
#define CSS_Q_ALIGN_SELF (1u << 24)
#define CSS_Q_GAP_ROW    (1u << 25)
#define CSS_Q_GAP_COL    (1u << 26)
#define CSS_Q_GROW       (1u << 27)
#define CSS_Q_SHRINK     (1u << 28)
#define CSS_Q_BASIS      (1u << 29)
#define CSS_Q_GRID_COLS  (1u << 30)
#define CSS_Q_OVERFLOW   (1u << 31)

#define CSS_ALIGN_LEFT   0
#define CSS_ALIGN_CENTER 1
#define CSS_ALIGN_RIGHT  2

/* `auto` IS NOT A NUMBER AND MUST NOT BE ENCODED AS ONE THAT COULD OCCUR.
 * `margin: 0 auto` is the single most common centring idiom on the web and
 * `width: auto` is the initial value of every block, so this sentinel is on
 * the hot path, not an edge case. INT_MIN is chosen because it is the one
 * value no length can legitimately resolve to and because negating it is the
 * only arithmetic that could produce it by accident - which css.c must not
 * do, and csstest.c must check. */
#define CSS_AUTO         (-0x7FFFFFFF - 1)

#define CSS_DISP_INLINE  0
#define CSS_DISP_BLOCK   1
#define CSS_DISP_NONE    2
#define CSS_DISP_INLINE_BLOCK 3
#define CSS_DISP_FLEX    4
#define CSS_DISP_INLINE_FLEX  5
#define CSS_DISP_GRID    6

#define CSS_BOX_CONTENT  0
#define CSS_BOX_BORDER   1

#define CSS_FLOAT_NONE   0
#define CSS_FLOAT_LEFT   1
#define CSS_FLOAT_RIGHT  2
#define CSS_CLEAR_NONE   0
#define CSS_CLEAR_LEFT   1
#define CSS_CLEAR_RIGHT  2
#define CSS_CLEAR_BOTH   3

#define CSS_POS_STATIC   0
#define CSS_POS_RELATIVE 1
#define CSS_POS_ABSOLUTE 2
#define CSS_POS_FIXED    3
#define CSS_POS_STICKY   4

#define CSS_ROW          0
#define CSS_ROW_REVERSE  1
#define CSS_COLUMN       2
#define CSS_COLUMN_REVERSE 3

#define CSS_NOWRAP       0
#define CSS_WRAP         1
#define CSS_WRAP_REVERSE 2

/* justify-content and align-content share this set; align-items/align-self
 * use START/END/CENTER/STRETCH/BASELINE from it and nothing else. */
#define CSS_J_START      0
#define CSS_J_END        1
#define CSS_J_CENTER     2
#define CSS_J_BETWEEN    3
#define CSS_J_AROUND     4
#define CSS_J_EVENLY     5
#define CSS_J_STRETCH    6
#define CSS_J_BASELINE   7
#define CSS_J_AUTO       8      /* align-self only: "whatever the parent said" */

#define CSS_OVER_VISIBLE 0
#define CSS_OVER_HIDDEN  1
#define CSS_OVER_SCROLL  2

/* One grid track. `fr` is not a length - it is a share of what is left over -
 * so it cannot be resolved to pixels until the container's width is known,
 * which is why the unit survives into the computed style instead of being
 * collapsed here the way font-size is. */
#define CSS_TRACK_PX     0
#define CSS_TRACK_PCT    1
#define CSS_TRACK_FR     2
#define CSS_TRACK_AUTO   3

/* Eight columns. A document's grid is a nav bar, a card row or a two-column
 * article; a 24-column framework layout is a web app, which is the boundary
 * this file's header draws. This costs 8 * 2 ints inside a struct that
 * layout.c holds one of per recursion level (32 deep), so it is 2 KB of stack
 * at the deepest - raise it only after checking that number again. */
#define CSS_GRID_MAX     8

struct css_style {
    int has;              /* CSS_P_* bitmask                                */
    unsigned has2;        /* CSS_Q_* bitmask - see the note above           */
    int rgb;              /* colour, or -1 for "the theme's role"           */
    int bg;               /* background, or -1                              */
    int size;             /* px, already resolved against the parent        */
    int bold, italic, mono, underline;
    int align;            /* CSS_ALIGN_*                                    */
    int display;          /* CSS_DISP_*                                     */
    int margin_t, margin_b, margin_l, margin_r;   /* or CSS_AUTO            */
    int pad_t, pad_b, pad_l, pad_r;

    /* ---- the box ---- */
    int width, height;            /* px, or CSS_AUTO                        */
    int min_w, max_w;             /* max_w is CSS_AUTO when there is none    */
    int min_h, max_h;
    int box_sizing;               /* CSS_BOX_*                              */
    int border_t, border_r, border_b, border_l;   /* widths, px             */
    int border_rgb;               /* -1 when the author gave none           */
    int radius;

    /* ---- out of flow ---- */
    int floatv;                   /* CSS_FLOAT_*                            */
    int clearv;                   /* CSS_CLEAR_*                            */
    int position;                 /* CSS_POS_*                              */
    int top, right, bottom, left; /* px, or CSS_AUTO                        */
    int overflow;                 /* CSS_OVER_*                             */

    /* ---- flex: the container's properties, then the item's ---- */
    int flex_dir;                 /* CSS_ROW / CSS_COLUMN / ...             */
    int flex_wrap;                /* CSS_NOWRAP / CSS_WRAP / ...            */
    int justify;                  /* CSS_J_*                                */
    int align_items;              /* CSS_J_*                                */
    int align_self;               /* CSS_J_*, CSS_J_AUTO when unset         */
    int gap_row, gap_col;         /* px                                     */
    /* GROW AND SHRINK ARE IN HUNDREDTHS, for the same reason em is: this
     * kernel has no floating point on any path, and `flex-grow: 0.5` is real
     * CSS that a document will contain. 100 means 1. */
    int grow, shrink;
    int basis;                    /* px, or CSS_AUTO for "use the content"  */

    /* ---- grid ---- */
    int n_grid_cols;
    int grid_col_v[CSS_GRID_MAX];
    int grid_col_unit[CSS_GRID_MAX];   /* CSS_TRACK_*                       */
};

/* one element on the ancestor path. Lengths, not NULs: these point into the
 * document buffer, exactly as html.c's text does. */
struct css_elem {
    const char *tag; int tag_len;
    const char *id;  int id_len;
    const char *cls; int cls_len;    /* the raw space-separated class list */
};

void css_reset(void);

/* Add a stylesheet. Later sheets win ties against earlier ones, so the UA
 * sheet goes in first and the document's <style> after it. Returns the number
 * of rules taken. */
int  css_add_sheet(const char *src, int len);

/* Compute the style for path[n-1], given its ancestors in path[0..n-2].
 *
 * `parent_size` resolves em on every property and % on the FONT properties.
 * `parent_width` resolves % on the BOX properties - width, height, min/max,
 * the offsets, gap and flex-basis - because a percentage width is a share of
 * the containing block, not of the font. Those two used to be the same number
 * and the result was `width: 50%` computing to 8px against a 16px parent.
 * Margin and padding percentages are deliberately left resolving against
 * parent_size: it is wrong per the spec, it is what shipped, and changing it
 * is a separate change with its own regression risk.
 *
 * `inline_style` is the element's own style= attribute, which beats every
 * rule. Fields not set by any rule are left at the caller's defaults, so seed
 * *out before calling - and seed the new fields too: css.c does not know what
 * a sensible default width is, layout.c does. */
void css_compute(const struct css_elem *path, int n,
                 int parent_size, int parent_width,
                 const char *inline_style, int inline_len,
                 struct css_style *out);

/* Seed *out with the initial value of every property - CSS_AUTO widths,
 * CSS_DISP_INLINE, no border, grow 0, shrink 100. Exists so that the two
 * callers (layout.c and the harness) cannot drift about what "unset" means,
 * which is exactly how a struct with forty fields acquires a field that is
 * garbage on one path and zero on the other. */
void css_style_init(struct css_style *out, int size);

/* THE WIDTH @media IS JUDGED AGAINST. Set it before adding sheets; a width of
 * 0 (the default) makes every width-based query refuse, which is exactly the
 * behaviour that shipped before media queries were evaluated at all.
 * Evaluated at PARSE time - see the note in css.c - so a resize does not
 * re-run the queries. */
void css_viewport(int w);
int  css_viewport_get(void);

int  css_rules(void);
int  css_decls(void);
int  css_arena_used(void);
int  css_overflowed(void);    /* rules refused because a limit was hit */

#endif
