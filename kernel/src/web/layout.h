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
#define LR_RULE   1      /* <hr>, an h1's underline, AND a border edge      */
#define LR_BULLET 2      /* a list marker                                   */
#define LR_IMG    3      /* an image: decoded when `img` >= 0, else a frame */
#define LR_BOX    4      /* a block's own background - see below            */

/* A BORDER IS FOUR FILLED RECTANGLES, so it is four LR_RULE runs carrying an
 * `rgb`, not five new fields on every run in the document. struct lay_run is
 * already emitted 12,288 times into BSS; widening it by a border spec would
 * cost a quarter of a megabyte to describe something that four existing runs
 * describe exactly. LR_RULE with no rgb keeps meaning "the theme's rule
 * colour", which is what <hr> wants.
 *
 * LR_BOX exists for the same economy from the other side: a block-level
 * background is a rectangle behind its contents, and the painter ALREADY
 * fills `bg` behind any run with a positive w and h. So LR_BOX is a run that
 * carries only geometry and a background, draws nothing of its own, and is
 * emitted BEFORE its children so document order is paint order. */

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
    /* LR_IMG only: the decoded picture's slot in png.c's arena, or -1 for
     * "we know its box and not its pixels", which is what an <img> is between
     * the document arriving and the picture arriving. layout.c never calls
     * png.c - it is handed the slot, exactly as it is handed a measure
     * function, so the box model still links with no decoder present. */
    int img;
};

/* WHERE THE RUNS LIVE IS THE CALLER'S DECISION, for the reason the comment
 * above already half-states: struct lay_run was emitted 12,288 times into BSS,
 * and 12,288 was not a design, it was what fitted. The English Wikipedia
 * article produced 9,886 runs while html.c was DROPPING 7,807 of its nodes -
 * so the run array was never the binding limit, it was merely next in line,
 * and raising the node cap without raising this one just moves the truncation.
 *
 * Injected rather than fixed, because layout.c reaches for exactly two things
 * outside itself (a measure function and an image lookup) and that is the
 * property that makes the whole box model an ordinary Linux program. A third
 * reach - to a hardcoded physical address - would end it.
 *
 * LAY_RUN_BYTES is a sizeof rather than a literal, unlike html.h's and css.h's,
 * because struct lay_run is public here - and it is NOT the same number on
 * every target: 60 bytes on the 32-bit kernel, 72 on a 64-bit host, because of
 * the one pointer. A caller sizing a region needs the target's own answer, so
 * it gets the compiler's.
 *
 * UNTIL lay_set_arena IS CALLED, lay_run_doc LAYS OUT NOTHING, RETURNS 0 AND
 * SETS lay_overflowed(). png.h's argument, third time: a silent small fallback
 * renders the top of every page and looks like a short document. */
#define LAY_MAX_RUNS  65536
#define LAY_RUN_BYTES ((long)sizeof(struct lay_run))
#define LAY_RUNS_BYTES ((long)LAY_MAX_RUNS * LAY_RUN_BYTES)

void lay_set_arena(struct lay_run *runs, int max_runs);
int  lay_run_cap(void);      /* what was actually handed over */

/* How wide is s[0..len) at this size and style? The one thing layout cannot
 * work out for itself, so it is injected: the kernel passes the real font
 * metrics, the harness passes a synthetic one and gets deterministic numbers. */
typedef int (*lay_measure_fn)(const char *s, int len, int size, int style);

void lay_set_measure(lay_measure_fn f);

/* What are this <img>'s real pixels? Injected for the same reason the measure
 * function is: layout.c must keep linking with no decoder and no framebuffer,
 * so it asks rather than includes. `node` is the html node of the <img>;
 * fill *w and *h with the intrinsic size and return the arena slot, or return
 * -1 for "not here (yet)", leaving both out-parameters alone. A NULL hook
 * means every image lays out at its attribute size or the placeholder
 * default, which is what every host harness wants and what the browser shows
 * before a picture has been fetched. */
typedef int (*lay_image_fn)(int node, int *w, int *h);

void lay_set_image(lay_image_fn f);

/* Lay the parsed document out into `width` pixels with `base` as the body font
 * size. Returns the total content height. */
int  lay_run_doc(int width, int base);

int  lay_count(void);
const struct lay_run *lay_at(int i);
int  lay_height(void);
int  lay_lines(void);        /* how many line boxes the inline flow produced */
int  lay_overflowed(void);   /* runs dropped because the array filled        */

#endif
