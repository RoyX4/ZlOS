/* layout.h - boxes and runs, with no pixels in sight.
 *
 * layout.c computes WHERE things go and emits a flat array of runs. It never
 * draws. That split is not tidiness: it is what lets the whole box model be
 * asserted by a host harness with no framebuffer, and it is what makes reflow
 * a re-run of one function rather than a repaint of everything.
 *
 * Coordinates are CONTENT-RELATIVE: (0,0) is the top-left of the document, y
 * grows down, and nothing knows about a window, a scroll position or a screen.
 * The caller adds its own origin. That is the same position-purity rule the
 * app contract in ui.h imposes, applied one level lower.
 */
#ifndef LAYOUT_H
#define LAYOUT_H

#define LR_TEXT   0
#define LR_RULE   1      /* <hr>, and the line under an h1  */
#define LR_BULLET 2      /* a list marker                   */
#define LR_IMG    3      /* an image placeholder box        */

#define LS_BOLD   (1 << 0)
#define LS_ITALIC (1 << 1)
#define LS_MONO   (1 << 2)
#define LS_UNDER  (1 << 3)

/* COLOUR ROLES, NOT COLOURS. layout.c must not know the theme - ui.h keeps
 * every colour in one struct for exactly this reason, and a layout engine that
 * hardcoded 0x55D6FF would be a second place they live. */
#define LC_TEXT   0
#define LC_DIM    1
#define LC_ACCENT 2

/* AN AUTHOR COLOUR IS NOT A THEME COLOUR, which is why `rgb` can sit beside
 * `color` without breaking the rule above. The role says how the THEME should
 * paint this run; rgb is a value the DOCUMENT asked for by name. layout.c
 * still has no idea what LC_TEXT resolves to - it only carries through what
 * the stylesheet said. -1 means the document did not ask, so the role wins. */
#define LR_NO_RGB (-1)

struct lay_run {
    int kind;
    int x, y, w, h;
    const char *text;    /* LR_TEXT: not nul-terminated, use len */
    int len;
    int size;            /* the em size this was measured at     */
    int style;
    int color;           /* LC_* role, used when rgb is LR_NO_RGB */
    int rgb;             /* author colour from CSS, or LR_NO_RGB  */
    int bg;              /* author background, or LR_NO_RGB       */
    int node;            /* the html node it came from           */
    int link;            /* enclosing <a>, or -1                 */
};

/* How wide is s[0..len) at this size and style? The one thing layout cannot
 * work out for itself, so it is injected: the kernel passes the real font
 * metrics, the harness passes a synthetic one and gets deterministic numbers. */
typedef int (*lay_measure_fn)(const char *s, int len, int size, int style);

void lay_set_measure(lay_measure_fn f);

/* Lay the parsed document out into `width` pixels with `base` as the body font
 * size. Returns the total content height. */
int  lay_run_doc(int width, int base);

int  lay_count(void);
const struct lay_run *lay_at(int i);
int  lay_height(void);
int  lay_lines(void);        /* how many line boxes the inline flow produced */
int  lay_overflowed(void);   /* runs dropped because the array filled        */

#endif
