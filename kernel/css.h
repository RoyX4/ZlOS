/* css.h - a stylesheet, and what it computes to for one element.
 *
 * SCOPE, stated up front because a CSS engine is the classic unbounded task
 * and this one is deliberately not. It supports what a DOCUMENT uses: type,
 * class, id, universal and descendant selectors, comma-separated groups, and
 * the seventeen properties layout.c can actually act on. No media queries
 * beyond skipping them, no pseudo-classes, no floats, no positioning, no
 * flexbox, no grid. Those are not "coming later" - they are outside the
 * bounded version of this capability, and saying so is the point.
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

#define CSS_ALIGN_LEFT   0
#define CSS_ALIGN_CENTER 1
#define CSS_ALIGN_RIGHT  2

#define CSS_DISP_INLINE  0
#define CSS_DISP_BLOCK   1
#define CSS_DISP_NONE    2

struct css_style {
    int has;              /* CSS_P_* bitmask                                */
    int rgb;              /* colour, or -1 for "the theme's role"           */
    int bg;               /* background, or -1                              */
    int size;             /* px, already resolved against the parent        */
    int bold, italic, mono, underline;
    int align;            /* CSS_ALIGN_*                                    */
    int display;          /* CSS_DISP_*                                     */
    int margin_t, margin_b, margin_l, margin_r;
    int pad_t, pad_b, pad_l, pad_r;
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
 * `parent_size` resolves em and %; `inline_style` is the element's own style=
 * attribute, which beats every rule. Fields not set by any rule are left at
 * the caller's defaults, so seed *out before calling. */
void css_compute(const struct css_elem *path, int n, int parent_size,
                 const char *inline_style, int inline_len,
                 struct css_style *out);

int  css_rules(void);
int  css_decls(void);
int  css_arena_used(void);
int  css_overflowed(void);    /* rules refused because a limit was hit */

#endif
