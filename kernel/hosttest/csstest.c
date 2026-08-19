/* csstest.c - the CSS engine against hand-written stylesheets.
 *
 * css.c takes UNTRUSTED TEXT from a machine we did not choose and turns it
 * into numbers that move pixels. That is the same trust boundary dns.c sits
 * on, and most of this harness is malformed or hostile rather than merely
 * unusual, for the same reason: a stylesheet that is valid is the one case a
 * real site will reliably hand you, and every way a parser goes wrong is in
 * the other cases.
 *
 * WHAT IS BEING PROTECTED, in order of how badly it fails:
 *
 *   over-matching   `a:hover` matched as `a` styles every link permanently;
 *                   `x > y` matched as `x y` styles strictly more than asked.
 *                   Both are REFUSALS in css.c, and both are asserted here.
 *   the cascade     specificity wrong = the wrong rule wins, which looks like
 *                   a rendering bug three layers away from its cause
 *   recovery        one bad rule must not lose the rest of the sheet
 *   bounds          fixed arrays; a hostile sheet must truncate, not scribble
 *
 * No kernel, no boot, and NO html.c either - css.c matches against an explicit
 * ancestor path, so the engine is testable with nothing else linked.
 *
 *   cd kernel/hosttest && ./build.sh && ./csstest
 */
#include <stdio.h>
#include <string.h>
#include "../css.h"

static int checks, failures;

static void ok(int cond, const char *what)
{
    checks++;
    if (!cond) { failures++; printf("  FAIL  %s\n", what); }
}

static void okn(int got, int want, const char *what)
{
    checks++;
    if (got != want) { failures++; printf("  FAIL  %s   got %d want %d\n", what, got, want); }
}

/* build a path from a "tag#id.class tag#id.class" description, so the tests
 * read like the selectors they are matched against */
#define MAXP 8
static struct css_elem P[MAXP];
static char pbuf[MAXP][3][64];

static int path_of(const char *desc)
{
    int n = 0, i = 0;
    int len = (int)strlen(desc);
    while (i < len && n < MAXP) {
        while (i < len && desc[i] == ' ') i++;
        if (i >= len) break;
        int st = i;
        while (i < len && desc[i] != ' ') i++;
        const char *s = desc + st;
        int sl = i - st;
        char *tag = pbuf[n][0], *id = pbuf[n][1], *cls = pbuf[n][2];
        tag[0] = id[0] = cls[0] = 0;
        int k = 0, tl = 0, il = 0, cl = 0;
        while (k < sl && s[k] != '#' && s[k] != '.') tag[tl++] = s[k++];
        tag[tl] = 0;
        while (k < sl) {
            char kind = s[k++];
            char *dst = (kind == '#') ? id : cls;
            int *dl = (kind == '#') ? &il : &cl;
            if (kind == '.' && cl) cls[cl++] = ' ';
            while (k < sl && s[k] != '#' && s[k] != '.') dst[(*dl)++] = s[k++];
            dst[*dl] = 0;
        }
        P[n].tag = tl ? tag : 0;  P[n].tag_len = tl;
        P[n].id  = il ? id  : 0;  P[n].id_len  = il;
        P[n].cls = cl ? cls : 0;  P[n].cls_len = cl;
        n++;
    }
    return n;
}

/* THE DEFAULT CONTAINING BLOCK for everything below that does not say
 * otherwise. 400 is deliberately not 16 and not a round fraction of it: the
 * bug this whole parameter exists to stop is `width: 50%` resolving against
 * the FONT size, and with parent_width == parent_size that bug is invisible.
 * 50% of 400 is 200 and 50% of 16 is 8, and no assertion below can confuse
 * them. */
#define PW 400

static struct css_style compute_w(const char *path, int parent, int pwidth,
                                  const char *inl)
{
    struct css_style s;
    /* seeded by the engine's own initialiser rather than by a memset here -
     * that makes every test below also a test that css_style_init leaves a
     * usable struct, and it is the only way the harness and layout.c cannot
     * drift about what "unset" means */
    css_style_init(&s, parent);
    int n = path_of(path);
    css_compute(P, n, parent, pwidth, inl, inl ? (int)strlen(inl) : 0, &s);
    return s;
}

static struct css_style compute(const char *path, int parent, const char *inl)
{
    return compute_w(path, parent, PW, inl);
}

static void sheet(const char *css)
{
    css_reset();
    css_add_sheet(css, (int)strlen(css));
}

int main(void)
{
    printf("csstest: the bounded CSS engine\n");

    /* ---- 1. selectors ---------------------------------------------------- */
    printf("\n1. selectors\n");
    sheet("p { color: red }");
    okn(compute("html body p", 16, 0).rgb, 0xFF0000, "type selector matches");
    okn(compute("html body div", 16, 0).rgb, -1,     "type selector does not over-match");

    sheet(".note { color: blue }");
    okn(compute("html p.note", 16, 0).rgb, 0x0000FF, "class selector");
    okn(compute("html p.other", 16, 0).rgb, -1,      "wrong class does not match");
    okn(compute("html p.a.note.b", 16, 0).rgb, 0x0000FF, "class within a list");

    sheet("#main { color: lime }");
    okn(compute("html div#main", 16, 0).rgb, 0x00FF00, "id selector");
    okn(compute("html div#other", 16, 0).rgb, -1,      "wrong id");

    sheet("* { color: teal }");
    okn(compute("html whatever", 16, 0).rgb, 0x008080, "universal selector");

    sheet("div p { color: navy }");
    okn(compute("html div p", 16, 0).rgb,        0x000080, "descendant, direct");
    okn(compute("html div span em p", 16, 0).rgb, 0x000080, "descendant, distant");
    okn(compute("html p", 16, 0).rgb,            -1,       "descendant needs the ancestor");
    okn(compute("html p div", 16, 0).rgb,        -1,       "descendant order matters");

    sheet("h1, h2, .big { color: olive }");
    okn(compute("html h1", 16, 0).rgb, 0x808000, "comma group, first");
    okn(compute("html h2", 16, 0).rgb, 0x808000, "comma group, second");
    okn(compute("html p.big", 16, 0).rgb, 0x808000, "comma group, third");
    okn(compute("html h3", 16, 0).rgb, -1,       "comma group excludes the rest");

    sheet("div.card p { color: maroon }");
    okn(compute("html div.card span p", 16, 0).rgb, 0x800000, "compound + descendant");
    okn(compute("html div span p", 16, 0).rgb, -1, "compound needs the class too");

    /* ---- 2. refusals: the failures that are WORSE than not matching ------ */
    printf("\n2. refusals (over-matching is worse than no match)\n");
    sheet("a:hover { color: red }");
    okn(compute("html a", 16, 0).rgb, -1,
        "a pseudo-class selector is refused, not matched as its type");
    sheet("div > p { color: red }");
    okn(compute("html div p", 16, 0).rgb, -1, "child combinator refused");
    sheet("h1 + p { color: red }");
    okn(compute("html h1 p", 16, 0).rgb, -1, "adjacent combinator refused");
    sheet("input[type=text] { color: red }");
    okn(compute("html input", 16, 0).rgb, -1, "attribute selector refused");
    /* and a refusal must not take the rest of the sheet with it */
    sheet("a:hover { color: red } p { color: blue }");
    okn(compute("html p", 16, 0).rgb, 0x0000FF,
        "a refused selector does not lose the rules after it");

    /* ---- 3. the cascade -------------------------------------------------- */
    printf("\n3. cascade\n");
    sheet("p { color: red } p { color: blue }");
    okn(compute("html p", 16, 0).rgb, 0x0000FF, "later rule wins at equal specificity");

    sheet("#i { color: lime } .c { color: red } p { color: blue }");
    okn(compute("html p#i.c", 16, 0).rgb, 0x00FF00, "id beats class beats type");

    sheet(".c { color: red } p { color: blue }");
    okn(compute("html p.c", 16, 0).rgb, 0xFF0000, "class beats type regardless of order");

    sheet("div p { color: red } p { color: blue }");
    okn(compute("html div p", 16, 0).rgb, 0xFF0000, "two types beat one");

    sheet("p { color: red }");
    okn(compute("html p", 16, "color: lime").rgb, 0x00FF00, "style= beats any rule");
    sheet("#i { color: red }");
    okn(compute("html p#i", 16, "color: lime").rgb, 0x00FF00, "style= beats an id rule");

    /* the inline block must not leak into the stored sheet */
    sheet("p { color: red }");
    int d_before = css_decls();
    compute("html p", 16, "color: lime; font-size: 40px");
    okn(css_decls(), d_before, "style= declarations are not kept in the sheet");

    /* ---- 4. values ------------------------------------------------------- */
    printf("\n4. values\n");
    sheet("p { color: #ff8000 }");
    okn(compute("html p", 16, 0).rgb, 0xFF8000, "#rrggbb");
    sheet("p { color: #f80 }");
    okn(compute("html p", 16, 0).rgb, 0xFF8800, "#rgb expands");
    sheet("p { color: rgb(255, 128, 0) }");
    okn(compute("html p", 16, 0).rgb, 0xFF8000, "rgb()");
    sheet("p { color: #12345 }");
    okn(compute("html p", 16, 0).rgb, -1, "a malformed hex colour is refused");
    sheet("p { color: notacolour }");
    okn(compute("html p", 16, 0).rgb, -1, "an unknown colour name is refused");

    sheet("p { font-size: 24px }");
    okn(compute("html p", 16, 0).size, 24, "px font-size");
    sheet("p { font-size: 2em }");
    okn(compute("html p", 16, 0).size, 32, "em font-size resolves against the parent");
    sheet("p { font-size: 150% }");
    okn(compute("html p", 16, 0).size, 24, "% font-size");
    sheet("p { font-size: 1.5em }");
    okn(compute("html p", 16, 0).size, 24, "fractional em");
    sheet("p { font-size: 0 }");
    okn(compute("html p", 16, 0).size, 16, "a zero font-size is refused, not applied");

    sheet("p { font-weight: bold }");
    okn(compute("html p", 16, 0).bold, 1, "font-weight: bold");
    sheet("p { font-weight: 700 }");
    okn(compute("html p", 16, 0).bold, 1, "font-weight: 700");
    sheet("p { font-weight: normal }");
    okn(compute("html p", 16, 0).bold, 0, "font-weight: normal");
    sheet("p { font-style: italic }");
    okn(compute("html p", 16, 0).italic, 1, "font-style: italic");
    sheet("p { font-family: Menlo, monospace }");
    okn(compute("html p", 16, 0).mono, 1, "a monospace family");
    sheet("p { font-family: Georgia, serif }");
    okn(compute("html p", 16, 0).mono, 0, "a proportional family");

    sheet("p { text-align: center }");
    okn(compute("html p", 16, 0).align, CSS_ALIGN_CENTER, "text-align: center");
    sheet("p { text-decoration: underline }");
    okn(compute("html p", 16, 0).underline, 1, "text-decoration: underline");
    sheet("p { display: none }");
    okn(compute("html p", 16, 0).display, CSS_DISP_NONE, "display: none");

    sheet("p { margin: 10px }");
    struct css_style m = compute("html p", 16, 0);
    ok(m.margin_t == 10 && m.margin_r == 10 && m.margin_b == 10 && m.margin_l == 10,
       "margin shorthand, one value");
    sheet("p { margin: 1px 2px }");
    m = compute("html p", 16, 0);
    ok(m.margin_t == 1 && m.margin_r == 2 && m.margin_b == 1 && m.margin_l == 2,
       "margin shorthand, two values");
    sheet("p { margin: 1px 2px 3px 4px }");
    m = compute("html p", 16, 0);
    ok(m.margin_t == 1 && m.margin_r == 2 && m.margin_b == 3 && m.margin_l == 4,
       "margin shorthand, four values in CSS order");
    sheet("p { margin: 0 auto }");
    m = compute("html p", 16, 0);
    /* THIS ASSERTION USED TO SAY margin_l == 0, and it passed, and it was
     * asserting the bug: `auto` was flattened to zero, so the centring idiom
     * parsed cleanly and centred nothing. It now has to survive to layout.c as
     * CSS_AUTO. Section 16 covers the idiom whole. */
    ok(m.margin_t == 0 && m.margin_l == CSS_AUTO && m.margin_r == CSS_AUTO,
       "margin: 0 auto keeps auto as auto, not as zero");
    sheet("p { padding-left: 12px }");
    okn(compute("html p", 16, 0).pad_l, 12, "padding longhand");

    /* ---- 5. malformed input, which is the normal case -------------------- */
    printf("\n5. malformed and hostile input\n");
    sheet("p { color: red");
    okn(compute("html p", 16, 0).rgb, 0xFF0000, "an unclosed block still applies");
    sheet("p { ; ; color: red ; ; }");
    okn(compute("html p", 16, 0).rgb, 0xFF0000, "stray semicolons");
    sheet("p { bogus } p { color: red }");
    okn(compute("html p", 16, 0).rgb, 0xFF0000, "a declaration with no colon");
    sheet("p { color: }");
    okn(compute("html p", 16, 0).rgb, -1, "an empty value");
    sheet("} p { color: red }");
    okn(compute("html p", 16, 0).rgb, 0xFF0000, "a leading stray brace recovers");
    sheet("/* c */ p /* c */ { /* c */ color: red }");
    okn(compute("html p", 16, 0).rgb, 0xFF0000, "comments anywhere");
    sheet("/* unterminated p { color: red }");
    okn(compute("html p", 16, 0).rgb, -1, "an unterminated comment eats the sheet, safely");
    sheet("@media print { p { color: red } } p { color: blue }");
    okn(compute("html p", 16, 0).rgb, 0x0000FF,
        "@media is skipped whole - a print sheet must not reach the screen");
    sheet("@import url(x.css); p { color: red }");
    okn(compute("html p", 16, 0).rgb, 0xFF0000, "@import is skipped to its semicolon");
    sheet("");
    okn(compute("html p", 16, 0).rgb, -1, "an empty sheet");
    css_reset();
    okn(compute("html p", 16, 0).rgb, -1, "no sheet at all");

    /* NUL and high bytes in the middle of a sheet */
    css_reset();
    {
        char nasty[] = "p { color: red }\0 div { color: blue }";
        css_add_sheet(nasty, (int)sizeof nasty - 1);
        okn(compute("html p", 16, 0).rgb, 0xFF0000, "a NUL mid-sheet does not stop parsing");
    }
    css_reset();
    {
        char hi[64];
        for (int i = 0; i < 64; i++) hi[i] = (char)(0x80 | i);
        css_add_sheet(hi, 64);
        ok(1, "a sheet of high bytes does not fault");
    }

    /* ---- 6. bounds ------------------------------------------------------- */
    printf("\n6. bounds\n");
    css_reset();
    {
        /* far more rules than the arrays hold */
        static char big[400000];
        int n = 0;
        for (int i = 0; i < 4000 && n < (int)sizeof big - 64; i++)
            n += snprintf(big + n, sizeof big - n, ".c%d { color: red; margin: %dpx }\n", i, i);
        css_add_sheet(big, n);
        ok(css_overflowed(), "a sheet past the limits reports overflow");
        ok(css_arena_used() <= 24576, "the arena stayed inside itself");
        ok(css_decls() <= 3072, "the declaration array stayed inside itself");
        /* and the rules it DID take still work */
        okn(compute("html p.c0", 16, 0).rgb, 0xFF0000, "rules taken before the limit still apply");
    }
    css_reset();
    {
        /* a single selector deeper than MAX_COMP must be refused, not
         * truncated into a shorter selector that matches more */
        css_add_sheet("a b c d e f g h { color: red }", 30);
        okn(compute("html a b c d e f g h", 16, 0).rgb, -1,
            "a selector deeper than the limit is refused, not truncated");
    }

    /* ---- 7. a realistic sheet -------------------------------------------- */
    printf("\n7. a realistic document sheet\n");
    sheet(
        "body { color: #222222; font-size: 16px }\n"
        "h1 { font-size: 2em; font-weight: bold; color: #003366 }\n"
        "a { color: #0066cc; text-decoration: underline }\n"
        ".sidebar { display: none }\n"
        "article p { margin: 12px 0; font-size: 1em }\n"
        "code, pre { font-family: monospace; background: #f4f4f4 }\n"
        "#footer .fine { font-size: 85%; color: gray }\n");
    okn(compute("html body h1", 16, 0).size, 32, "h1 is 2em of the body size");
    okn(compute("html body h1", 16, 0).rgb, 0x003366, "h1 colour");
    okn(compute("html body a", 16, 0).underline, 1, "links are underlined");
    okn(compute("html body div.sidebar", 16, 0).display, CSS_DISP_NONE, "the sidebar is hidden");
    okn(compute("html body article p", 16, 0).margin_t, 12, "article paragraphs get a margin");
    okn(compute("html body code", 16, 0).mono, 1, "code is monospace");
    okn(compute("html body code", 16, 0).bg, 0xF4F4F4, "code has a background");
    okn(compute("html body div#footer span.fine", 16, 0).size, 13, "the fine print is 85%");
    okn(compute("html body div#footer span.fine", 16, 0).rgb, 0x808080, "and grey");
    okn(compute("html body p", 16, 0).margin_t, 0, "a paragraph outside article is unstyled");

    /* ---- 8. css_style_init ------------------------------------------------
     * The single source of truth for "unset", shared with layout.c. Checked
     * field by field, because a struct with forty fields is exactly how one of
     * them ends up garbage on one path and zero on the other - and checked
     * against a POISON PATTERN first, so a field nobody remembered to seed is
     * caught even though no assertion below names it. */
    printf("\n8. css_style_init\n");
    {
        struct css_style s;
        memset(&s, 0xAA, sizeof s);
        css_style_init(&s, 16);
        const int *w = (const int *)&s;
        int untouched = 0;
        for (int i = 0; i < (int)(sizeof s / sizeof(int)); i++)
            if (w[i] == (int)0xAAAAAAAA) untouched++;
        okn(untouched, 0, "init writes every word of the struct, not just the ones with tests");

        ok(CSS_AUTO != 0, "CSS_AUTO is not zero - the whole point of the sentinel");
        okn(s.width,  CSS_AUTO, "init: width is auto, NOT 0 (0 would be invisible)");
        okn(s.height, CSS_AUTO, "init: height is auto");
        okn(s.max_w,  CSS_AUTO, "init: max-width is auto - no constraint");
        okn(s.max_h,  CSS_AUTO, "init: max-height is auto");
        okn(s.min_w, 0, "init: min-width 0");
        okn(s.min_h, 0, "init: min-height 0");
        okn(s.has,  0, "init: nothing is marked as set");
        okn((int)s.has2, 0, "init: nothing is marked as set in has2 either");
        okn(s.rgb, -1, "init: colour is the theme's role");
        okn(s.bg,  -1, "init: no background");
        okn(s.size, 16, "init: size is what the caller passed");
        okn(s.bold, 0, "init: not bold");
        okn(s.italic, 0, "init: not italic");
        okn(s.mono, 0, "init: not mono");
        okn(s.underline, 0, "init: not underlined");
        okn(s.align, CSS_ALIGN_LEFT, "init: text-align left");
        okn(s.display, CSS_DISP_INLINE, "init: display inline");
        ok(s.margin_t == 0 && s.margin_r == 0 && s.margin_b == 0 && s.margin_l == 0,
           "init: no margin");
        ok(s.pad_t == 0 && s.pad_r == 0 && s.pad_b == 0 && s.pad_l == 0,
           "init: no padding");
        okn(s.box_sizing, CSS_BOX_CONTENT, "init: box-sizing content-box");
        ok(s.border_t == 0 && s.border_r == 0 && s.border_b == 0 && s.border_l == 0,
           "init: no border");
        okn(s.border_rgb, -1, "init: no border colour");
        okn(s.radius, 0, "init: no radius");
        okn(s.floatv, CSS_FLOAT_NONE, "init: float none");
        okn(s.clearv, CSS_CLEAR_NONE, "init: clear none");
        okn(s.position, CSS_POS_STATIC, "init: position static");
        ok(s.top == CSS_AUTO && s.right == CSS_AUTO &&
           s.bottom == CSS_AUTO && s.left == CSS_AUTO, "init: all four offsets auto");
        okn(s.overflow, CSS_OVER_VISIBLE, "init: overflow visible");
        okn(s.flex_dir, CSS_ROW, "init: flex-direction row");
        okn(s.flex_wrap, CSS_NOWRAP, "init: flex-wrap nowrap");
        okn(s.justify, CSS_J_START, "init: justify-content flex-start");
        okn(s.align_items, CSS_J_STRETCH, "init: align-items stretch");
        okn(s.align_self, CSS_J_AUTO, "init: align-self auto");
        ok(s.gap_row == 0 && s.gap_col == 0, "init: no gap");
        okn(s.grow, 0,     "init: flex-grow 0");
        okn(s.shrink, 100, "init: flex-shrink 1, in hundredths");
        okn(s.basis, CSS_AUTO, "init: flex-basis auto");
        okn(s.n_grid_cols, 0, "init: no grid tracks");
        css_style_init(&s, 0);
        okn(s.size, 16, "init: a nonsense size falls back to 16");
    }

    /* ---- 9. the box ------------------------------------------------------ */
    printf("\n9. the box\n");
    sheet("p { width: 300px }");
    okn(compute("html p", 16, 0).width, 300, "width in px");
    ok(compute("html p", 16, 0).has2 & CSS_Q_WIDTH, "width marks itself set");
    sheet("p { color: red }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_WIDTH), "an unset width leaves its bit clear");

    sheet("p { width: auto }");
    okn(compute("html p", 16, 0).width, CSS_AUTO, "width: auto is the sentinel, not 0");
    sheet("p { height: 120px }");
    okn(compute("html p", 16, 0).height, 120, "height in px");
    sheet("p { min-width: 10px; max-width: 960px }");
    m = compute("html p", 16, 0);
    ok(m.min_w == 10 && m.max_w == 960, "min-width and max-width");
    sheet("p { max-width: none }");
    okn(compute("html p", 16, 0).max_w, CSS_AUTO, "max-width: none is 'no constraint'");
    sheet("p { min-height: 2em }");
    okn(compute("html p", 16, 0).min_h, 32, "min-height in em resolves against the font");
    sheet("p { max-height: 50% }");
    okn(compute("html p", 16, 0).max_h, 200, "max-height in % resolves against the width");

    sheet("p { box-sizing: border-box }");
    okn(compute("html p", 16, 0).box_sizing, CSS_BOX_BORDER, "box-sizing: border-box");
    sheet("p { box-sizing: content-box }");
    okn(compute("html p", 16, 0).box_sizing, CSS_BOX_CONTENT, "box-sizing: content-box");
    sheet("p { box-sizing: padding-box }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_BOXSIZING),
       "an unsupported box-sizing value sets nothing");

    sheet("p { overflow: hidden }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_HIDDEN, "overflow: hidden");
    sheet("p { overflow: scroll }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_SCROLL, "overflow: scroll");
    sheet("p { overflow: auto }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_SCROLL, "overflow: auto maps to scroll");
    sheet("p { overflow: visible }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_VISIBLE, "overflow: visible");
    sheet("p { overflow-y: auto }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_SCROLL, "overflow-y writes the one field");
    sheet("p { overflow-x: hidden }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_HIDDEN, "overflow-x writes the one field");
    sheet("p { overflow: sideways }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_OVERFLOW), "an invented overflow value is refused");
    /* the documented simplification: css.h has ONE overflow field, so the
     * two-value form keeps the x axis and drops the y. Asserted rather than
     * left implicit, because a reader who assumes per-axis clipping is wrong. */
    sheet("p { overflow: hidden auto }");
    okn(compute("html p", 16, 0).overflow, CSS_OVER_HIDDEN,
        "the two-value overflow keeps the x axis - one field, stated in css.c");
    sheet("p { overflow: hidden auto scroll }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_OVERFLOW), "three values is malformed");

    sheet("p { width: 2rem }");
    okn(compute("html p", 16, 0).width, 32, "rem on a box property is still a font unit");
    sheet("p { min-width: 0 }");
    m = compute("html p", 16, 0);
    ok(m.min_w == 0 && (m.has2 & CSS_Q_MIN_W), "min-width: 0 is set, not refused as falsy");

    /* ---- 10. borders ----------------------------------------------------- */
    printf("\n10. borders\n");
    sheet("p { border: 1px solid #ccc }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 1 && m.border_r == 1 && m.border_b == 1 && m.border_l == 1,
       "border shorthand sets all four widths");
    okn(m.border_rgb, 0xCCCCCC, "border shorthand takes the colour");
    /* the three parts are order-independent in the spec and authors use every
     * order, so every order is checked rather than the one that reads best */
    sheet("p { border: solid #ccc 1px }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 1 && m.border_rgb == 0xCCCCCC, "border, style-colour-width order");
    sheet("p { border: #ccc 1px solid }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 1 && m.border_rgb == 0xCCCCCC, "border, colour-width-style order");
    sheet("p { border: solid 2px red }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 2 && m.border_rgb == 0xFF0000, "border, style-width-colour order");
    sheet("p { border: 1px solid }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 1 && m.border_rgb == -1,
       "border with no colour resets the colour to 'use the text colour'");
    sheet("p { border: solid red }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 3 && m.border_rgb == 0xFF0000,
       "border with no width is `medium`, which is 3px");
    sheet("p { border: none }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 0 && m.border_l == 0, "border: none is zero width");
    sheet("p { border: 2px }");
    okn(compute("html p", 16, 0).border_t, 2, "border with only a width");

    sheet("p { border-width: 1px 2px 3px 4px }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 1 && m.border_r == 2 && m.border_b == 3 && m.border_l == 4,
       "border-width, four values in CSS order");
    sheet("p { border-width: 1px 2px }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 1 && m.border_r == 2 && m.border_b == 1 && m.border_l == 2,
       "border-width, two values");
    sheet("p { border-width: thin }");
    okn(compute("html p", 16, 0).border_t, 1, "border-width: thin");
    sheet("p { border-width: medium }");
    okn(compute("html p", 16, 0).border_t, 3, "border-width: medium");
    sheet("p { border-width: thick }");
    okn(compute("html p", 16, 0).border_t, 5, "border-width: thick");

    /* THE PROPERTY DOING REAL WORK: with no style field in css.h, `none` and
     * `hidden` are honoured by computing the width to 0. A test that only
     * checked the declaration "parsed" would pass with the code deleted. */
    sheet("p { border-width: 4px; border-style: none }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 0 && m.border_r == 0 && m.border_b == 0 && m.border_l == 0,
       "border-style: none computes every width to 0");
    sheet("p { border-width: 4px; border-style: hidden }");
    okn(compute("html p", 16, 0).border_t, 0, "border-style: hidden computes the width to 0");
    sheet("p { border-width: 4px; border-style: solid }");
    okn(compute("html p", 16, 0).border_t, 4,
        "border-style: solid leaves the width the cascade set");
    sheet("p { border: 2px solid red; border-left-style: none }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 2 && m.border_l == 0, "border-left-style: none zeroes only the left");

    sheet("p { border-left: 3px dashed blue }");
    m = compute("html p", 16, 0);
    ok(m.border_l == 3 && m.border_t == 0 && m.border_r == 0 && m.border_b == 0,
       "border-left touches only the left width");
    okn(m.border_rgb, 0x0000FF, "border-left takes the colour");
    sheet("p { border-top-width: 5px }");
    m = compute("html p", 16, 0);
    ok(m.border_t == 5 && m.border_b == 0, "border-top-width longhand");
    sheet("p { border-bottom-width: 6px }");
    okn(compute("html p", 16, 0).border_b, 6, "border-bottom-width longhand");
    sheet("p { border-right-width: 7px }");
    okn(compute("html p", 16, 0).border_r, 7, "border-right-width longhand");
    sheet("p { border-left-width: 8px }");
    okn(compute("html p", 16, 0).border_l, 8, "border-left-width longhand");
    sheet("p { border-color: teal }");
    okn(compute("html p", 16, 0).border_rgb, 0x008080, "border-color");
    sheet("p { border-bottom-color: #123456 }");
    okn(compute("html p", 16, 0).border_rgb, 0x123456,
        "border-<side>-color writes the one shared colour");
    sheet("p { border-radius: 8px }");
    okn(compute("html p", 16, 0).radius, 8, "border-radius");
    sheet("p { border-radius: 8px 4px }");
    okn(compute("html p", 16, 0).radius, 8, "border-radius takes the first of several corners");

    /* ---- 11. out of flow ------------------------------------------------- */
    printf("\n11. float, clear, position, offsets\n");
    sheet("p { float: left }");
    okn(compute("html p", 16, 0).floatv, CSS_FLOAT_LEFT, "float: left");
    sheet("p { float: right }");
    okn(compute("html p", 16, 0).floatv, CSS_FLOAT_RIGHT, "float: right");
    sheet("p { float: none }");
    okn(compute("html p", 16, 0).floatv, CSS_FLOAT_NONE, "float: none");
    sheet("p { float: inline-start }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_FLOAT), "an unsupported float value is refused");
    sheet("p { clear: both }");
    okn(compute("html p", 16, 0).clearv, CSS_CLEAR_BOTH, "clear: both");
    sheet("p { clear: left }");
    okn(compute("html p", 16, 0).clearv, CSS_CLEAR_LEFT, "clear: left");
    sheet("p { clear: right }");
    okn(compute("html p", 16, 0).clearv, CSS_CLEAR_RIGHT, "clear: right");

    sheet("p { position: relative }");
    okn(compute("html p", 16, 0).position, CSS_POS_RELATIVE, "position: relative");
    sheet("p { position: absolute }");
    okn(compute("html p", 16, 0).position, CSS_POS_ABSOLUTE, "position: absolute");
    sheet("p { position: fixed }");
    okn(compute("html p", 16, 0).position, CSS_POS_FIXED, "position: fixed");
    sheet("p { position: sticky }");
    okn(compute("html p", 16, 0).position, CSS_POS_STICKY, "position: sticky");
    sheet("p { position: static }");
    okn(compute("html p", 16, 0).position, CSS_POS_STATIC, "position: static");

    sheet("p { top: 10px; right: -5px; bottom: 25%; left: auto }");
    m = compute("html p", 16, 0);
    okn(m.top, 10, "top in px");
    okn(m.right, -5, "a negative offset is legal and kept");
    okn(m.bottom, 100, "an offset in % resolves against the containing block's width");
    okn(m.left, CSS_AUTO, "left: auto is the sentinel");

    /* ---- 12. the new display values -------------------------------------- */
    printf("\n12. display\n");
    sheet("p { display: inline-block }");
    okn(compute("html p", 16, 0).display, CSS_DISP_INLINE_BLOCK, "display: inline-block");
    sheet("p { display: flex }");
    okn(compute("html p", 16, 0).display, CSS_DISP_FLEX, "display: flex");
    sheet("p { display: inline-flex }");
    okn(compute("html p", 16, 0).display, CSS_DISP_INLINE_FLEX, "display: inline-flex");
    sheet("p { display: grid }");
    okn(compute("html p", 16, 0).display, CSS_DISP_GRID, "display: grid");
    sheet("p { display: inline-grid }");
    okn(compute("html p", 16, 0).display, CSS_DISP_GRID, "display: inline-grid folds onto grid");
    sheet("p { display: list-item }");
    okn(compute("html p", 16, 0).display, CSS_DISP_BLOCK, "display: list-item falls back to block");
    sheet("p { display: table }");
    okn(compute("html p", 16, 0).display, CSS_DISP_BLOCK, "display: table falls back to block");
    sheet("p { display: wibble }");
    okn(compute("html p", 16, 0).display, CSS_DISP_BLOCK, "an invented display falls back to block");
    /* the three that shipped, re-asserted here because the fallback above is
     * the exact edit that would break them */
    sheet("p { display: none }");
    okn(compute("html p", 16, 0).display, CSS_DISP_NONE, "display: none still works");
    sheet("p { display: inline }");
    okn(compute("html p", 16, 0).display, CSS_DISP_INLINE, "display: inline still works");
    sheet("p { display: block }");
    okn(compute("html p", 16, 0).display, CSS_DISP_BLOCK, "display: block still works");

    /* ---- 13. flex -------------------------------------------------------- */
    printf("\n13. flex\n");
    sheet("p { flex-direction: column }");
    okn(compute("html p", 16, 0).flex_dir, CSS_COLUMN, "flex-direction: column");
    sheet("p { flex-direction: row-reverse }");
    okn(compute("html p", 16, 0).flex_dir, CSS_ROW_REVERSE, "flex-direction: row-reverse");
    sheet("p { flex-direction: column-reverse }");
    okn(compute("html p", 16, 0).flex_dir, CSS_COLUMN_REVERSE, "flex-direction: column-reverse");
    sheet("p { flex-wrap: wrap }");
    okn(compute("html p", 16, 0).flex_wrap, CSS_WRAP, "flex-wrap: wrap");
    sheet("p { flex-wrap: wrap-reverse }");
    okn(compute("html p", 16, 0).flex_wrap, CSS_WRAP_REVERSE, "flex-wrap: wrap-reverse");
    sheet("p { flex-flow: column wrap }");
    m = compute("html p", 16, 0);
    ok(m.flex_dir == CSS_COLUMN && m.flex_wrap == CSS_WRAP, "flex-flow sets both");
    sheet("p { flex-flow: wrap column }");
    m = compute("html p", 16, 0);
    ok(m.flex_dir == CSS_COLUMN && m.flex_wrap == CSS_WRAP,
       "flex-flow does not care which order they came in");
    sheet("p { flex-flow: row }");
    m = compute("html p", 16, 0);
    ok(m.flex_dir == CSS_ROW && !(m.has2 & CSS_Q_FLEX_WRAP),
       "flex-flow with one value sets only that one");

    sheet("p { justify-content: space-between }");
    okn(compute("html p", 16, 0).justify, CSS_J_BETWEEN, "justify-content: space-between");
    sheet("p { justify-content: center }");
    okn(compute("html p", 16, 0).justify, CSS_J_CENTER, "justify-content: center");
    sheet("p { justify-content: flex-end }");
    okn(compute("html p", 16, 0).justify, CSS_J_END, "justify-content: flex-end");
    sheet("p { justify-content: space-around }");
    okn(compute("html p", 16, 0).justify, CSS_J_AROUND, "justify-content: space-around");
    sheet("p { justify-content: space-evenly }");
    okn(compute("html p", 16, 0).justify, CSS_J_EVENLY, "justify-content: space-evenly");
    sheet("p { align-items: center }");
    okn(compute("html p", 16, 0).align_items, CSS_J_CENTER, "align-items: center");
    sheet("p { align-items: flex-start }");
    okn(compute("html p", 16, 0).align_items, CSS_J_START, "align-items: flex-start");
    sheet("p { align-items: baseline }");
    okn(compute("html p", 16, 0).align_items, CSS_J_BASELINE, "align-items: baseline");
    sheet("p { align-items: stretch }");
    okn(compute("html p", 16, 0).align_items, CSS_J_STRETCH, "align-items: stretch");
    sheet("p { align-self: center }");
    okn(compute("html p", 16, 0).align_self, CSS_J_CENTER, "align-self: center");
    sheet("p { align-self: auto }");
    okn(compute("html p", 16, 0).align_self, CSS_J_AUTO, "align-self: auto");
    /* css.h HAS NO align_content FIELD. The property is consumed rather than
     * aliased, and this asserts it does not quietly move align-items - which
     * is the wrong thing that costs nothing to write and is hard to see. */
    sheet("p { align-content: center }");
    m = compute("html p", 16, 0);
    ok(!(m.has2 & CSS_Q_ALIGN_IT) && m.align_items == CSS_J_STRETCH,
       "align-content does not corrupt align-items (css.h has no field for it)");

    sheet("p { gap: 8px }");
    m = compute("html p", 16, 0);
    ok(m.gap_row == 8 && m.gap_col == 8, "gap with one value sets both axes");
    sheet("p { gap: 8px 16px }");
    m = compute("html p", 16, 0);
    ok(m.gap_row == 8 && m.gap_col == 16, "gap: 8px 16px is ROW then COLUMN");
    sheet("p { row-gap: 4px }");
    m = compute("html p", 16, 0);
    ok(m.gap_row == 4 && !(m.has2 & CSS_Q_GAP_COL), "row-gap touches only the row axis");
    sheet("p { column-gap: 5px }");
    m = compute("html p", 16, 0);
    ok(m.gap_col == 5 && !(m.has2 & CSS_Q_GAP_ROW), "column-gap touches only the column axis");
    sheet("p { grid-gap: 6px }");
    m = compute("html p", 16, 0);
    ok(m.gap_row == 6 && m.gap_col == 6, "grid-gap is the old spelling of gap");
    sheet("p { grid-row-gap: 7px }");
    okn(compute("html p", 16, 0).gap_row, 7, "grid-row-gap");
    sheet("p { grid-column-gap: 9px }");
    okn(compute("html p", 16, 0).gap_col, 9, "grid-column-gap");

    sheet("p { flex-grow: 2 }");
    okn(compute("html p", 16, 0).grow, 200, "flex-grow: 2 is 200 hundredths");
    sheet("p { flex-grow: 0.5 }");
    okn(compute("html p", 16, 0).grow, 50, "flex-grow: 0.5 is 50 - real CSS with no float unit");
    sheet("p { flex-shrink: 0 }");
    okn(compute("html p", 16, 0).shrink, 0, "flex-shrink: 0");
    sheet("p { flex-shrink: 3 }");
    okn(compute("html p", 16, 0).shrink, 300, "flex-shrink: 3");
    sheet("p { flex-basis: 40px }");
    okn(compute("html p", 16, 0).basis, 40, "flex-basis in px");
    sheet("p { flex-basis: auto }");
    okn(compute("html p", 16, 0).basis, CSS_AUTO, "flex-basis: auto");
    sheet("p { flex-basis: 25% }");
    okn(compute("html p", 16, 0).basis, 100, "flex-basis in % resolves against the width");

    /* THE FIVE FORMS OF THE `flex` SHORTHAND. Each is a different triple and
     * each is asserted as a triple, because getting grow right and basis wrong
     * is the failure that looks like "flex almost works". */
    sheet("p { flex: 1 }");
    m = compute("html p", 16, 0);
    ok(m.grow == 100 && m.shrink == 100 && m.basis == 0, "flex: 1 is 1 1 0%");
    sheet("p { flex: auto }");
    m = compute("html p", 16, 0);
    ok(m.grow == 100 && m.shrink == 100 && m.basis == CSS_AUTO, "flex: auto is 1 1 auto");
    sheet("p { flex: none }");
    m = compute("html p", 16, 0);
    ok(m.grow == 0 && m.shrink == 0 && m.basis == CSS_AUTO, "flex: none is 0 0 auto");
    sheet("p { flex: initial }");
    m = compute("html p", 16, 0);
    ok(m.grow == 0 && m.shrink == 100 && m.basis == CSS_AUTO, "flex: initial is 0 1 auto");
    sheet("p { flex: 3 }");
    m = compute("html p", 16, 0);
    ok(m.grow == 300 && m.shrink == 100 && m.basis == 0, "flex: <number> is n 1 0%");
    sheet("p { flex: 0.5 }");
    m = compute("html p", 16, 0);
    ok(m.grow == 50 && m.shrink == 100 && m.basis == 0, "flex: 0.5 is 0.5 1 0%");
    sheet("p { flex: 2 3 }");
    m = compute("html p", 16, 0);
    ok(m.grow == 200 && m.shrink == 300 && m.basis == 0,
       "flex: 2 3 is grow then SHRINK, not grow then basis");
    sheet("p { flex: 1 1 0% }");
    m = compute("html p", 16, 0);
    ok(m.grow == 100 && m.shrink == 100 && m.basis == 0, "flex: 1 1 0%");
    sheet("p { flex: 0 0 200px }");
    m = compute("html p", 16, 0);
    ok(m.grow == 0 && m.shrink == 0 && m.basis == 200, "flex: 0 0 200px");
    sheet("p { flex: 1 200px }");
    m = compute("html p", 16, 0);
    ok(m.grow == 100 && m.shrink == 100 && m.basis == 200,
       "flex: 1 200px - a token with a unit is the basis, not the shrink");
    sheet("p { flex: 2 1 auto }");
    m = compute("html p", 16, 0);
    ok(m.grow == 200 && m.shrink == 100 && m.basis == CSS_AUTO, "flex: 2 1 auto");

    /* ---- 14. grid -------------------------------------------------------- */
    printf("\n14. grid\n");
    sheet("p { grid-template-columns: 1fr 2fr }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 2, "two fr tracks");
    ok(m.grid_col_v[0] == 1 && m.grid_col_unit[0] == CSS_TRACK_FR, "track 0 is 1fr");
    ok(m.grid_col_v[1] == 2 && m.grid_col_unit[1] == CSS_TRACK_FR, "track 1 is 2fr");
    ok(m.has2 & CSS_Q_GRID_COLS, "grid-template-columns marks itself set");

    sheet("p { grid-template-columns: repeat(3, 1fr) }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 3, "repeat(3, 1fr) expands to three tracks at PARSE time");
    ok(m.grid_col_v[0] == 1 && m.grid_col_v[1] == 1 && m.grid_col_v[2] == 1,
       "every repeated track is 1");
    ok(m.grid_col_unit[0] == CSS_TRACK_FR && m.grid_col_unit[2] == CSS_TRACK_FR,
       "every repeated track kept its fr unit");

    sheet("p { grid-template-columns: repeat(2, 100px 1fr) }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 4, "repeat over a two-track list gives four");
    ok(m.grid_col_v[0] == 100 && m.grid_col_unit[0] == CSS_TRACK_PX, "px track survives repeat");
    ok(m.grid_col_v[1] == 1 && m.grid_col_unit[1] == CSS_TRACK_FR, "fr track survives repeat");
    ok(m.grid_col_v[2] == 100 && m.grid_col_unit[2] == CSS_TRACK_PX, "and repeats in order");

    sheet("p { grid-template-columns: 100px 50% auto 1fr }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 4, "a mixed track list");
    ok(m.grid_col_unit[0] == CSS_TRACK_PX && m.grid_col_v[0] == 100, "px track");
    ok(m.grid_col_unit[1] == CSS_TRACK_PCT && m.grid_col_v[1] == 50,
       "a percentage track keeps its unit - it is not resolved here");
    okn(m.grid_col_unit[2], CSS_TRACK_AUTO, "auto track");
    ok(m.grid_col_unit[3] == CSS_TRACK_FR && m.grid_col_v[3] == 1, "fr track");
    sheet("p { grid-template-columns: none }");
    m = compute("html p", 16, 0);
    ok(m.n_grid_cols == 0 && (m.has2 & CSS_Q_GRID_COLS), "grid-template-columns: none clears");

    sheet("p { grid-template: \"a b\" 1fr / 100px 200px }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 2, "grid-template: the columns after the slash are parsed");
    ok(m.grid_col_v[0] == 100 && m.grid_col_v[1] == 200, "and they are the right two");
    sheet("p { grid: 1fr / repeat(2, 1fr) }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 2, "the `grid` shorthand's columns half");
    sheet("p { grid-template: none }");
    ok(compute("html p", 16, 0).n_grid_cols == 0,
       "a grid shorthand with no slash is ignored rather than guessed at");

    /* a later rule's track list must REPLACE the earlier one, not append to it */
    sheet("p { grid-template-columns: 1fr 1fr 1fr } p { grid-template-columns: 50% 50% }");
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, 2, "a later track list replaces the earlier one, it does not append");

    /* ---- 15. the % split, which is the whole reason for parent_width ------ */
    printf("\n15. percentages resolve against the right parent\n");
    sheet("p { width: 50% }");
    m = compute_w("html p", 16, 400, 0);
    okn(m.width, 200, "width: 50% of a 400px containing block is 200");
    ok(m.width != 8, "and is NOT 8 - the bug this parameter exists to stop");
    sheet("p { font-size: 150% }");
    okn(compute_w("html p", 16, 400, 0).size, 24,
        "font-size: 150% still resolves against the FONT size, not the width");
    /* THE SPLIT IS BY UNIT, NOT BY PROPERTY. An em on a box property is still
     * a share of the font; only a percentage is a share of the containing
     * block. Getting this wrong made `min-height: 2em` compute to 800. */
    sheet("p { font-size: 2em; width: 2em }");
    m = compute_w("html p", 16, 400, 0);
    ok(m.size == 32 && m.width == 32, "em resolves against the font size on both");
    sheet("p { min-height: 2em; max-width: 3em; top: 1em; gap: 0.5em;\n"
          "    border-width: 1em; flex-basis: 2em; border-radius: 1em }");
    m = compute_w("html p", 16, 400, 0);
    okn(m.min_h,    32, "em min-height is 2 * the font, not 2 * the width");
    okn(m.max_w,    48, "em max-width");
    okn(m.top,      16, "em offset");
    okn(m.gap_row,   8, "fractional em gap");
    okn(m.border_t, 16, "em border-width");
    okn(m.basis,    32, "em flex-basis");
    okn(m.radius,   16, "em border-radius");
    sheet("p { min-height: 50%; max-width: 25% }");
    m = compute_w("html p", 16, 400, 0);
    ok(m.min_h == 200 && m.max_w == 100, "...while the SAME properties in % use the width");
    /* the deliberate exception, asserted so that changing it is a visible
     * change and not a silent one */
    sheet("p { margin-left: 50%; padding-top: 50% }");
    m = compute_w("html p", 16, 400, 0);
    okn(m.margin_l, 8, "margin % still resolves against parent_size - wrong per spec, as shipped");
    okn(m.pad_t, 8, "padding % still resolves against parent_size - wrong per spec, as shipped");
    sheet("p { gap: 10% }");
    okn(compute_w("html p", 16, 400, 0).gap_row, 40, "gap % resolves against the width");
    sheet("p { width: 50% }");
    okn(compute_w("html p", 16, 0, 0).width, 0, "a zero-width containing block gives 0, not garbage");

    /* ---- 16. the page-centring idiom ------------------------------------- */
    printf("\n16. max-width plus margin: 0 auto\n");
    sheet(".wrap { max-width: 960px; margin: 0 auto }");
    m = compute_w("html body div.wrap", 16, 1280, 0);
    okn(m.max_w, 960, "the wrapper is capped at 960");
    okn(m.margin_l, CSS_AUTO, "left margin is auto");
    okn(m.margin_r, CSS_AUTO, "right margin is auto");
    okn(m.margin_t, 0, "top margin is 0");
    okn(m.margin_b, 0, "bottom margin is 0");
    sheet("p { margin: auto }");
    m = compute("html p", 16, 0);
    ok(m.margin_t == CSS_AUTO && m.margin_r == CSS_AUTO &&
       m.margin_b == CSS_AUTO && m.margin_l == CSS_AUTO, "margin: auto on all four sides");
    sheet("p { margin: auto auto auto auto }");
    m = compute("html p", 16, 0);
    ok(m.margin_t == CSS_AUTO && m.margin_r == CSS_AUTO &&
       m.margin_b == CSS_AUTO && m.margin_l == CSS_AUTO, "margin: auto auto auto auto");
    sheet("p { margin-left: auto }");
    okn(compute("html p", 16, 0).margin_l, CSS_AUTO, "margin-left: auto longhand");
    sheet("p { padding-left: auto }");
    ok(!(compute("html p", 16, 0).has & CSS_P_PAD_L),
       "padding-left: auto is invalid CSS and is refused");

    /* ---- 17. hostile input for the new properties ------------------------ */
    printf("\n17. malformed and hostile input, new properties\n");
    sheet("p { width: ; color: red }");
    m = compute("html p", 16, 0);
    ok(!(m.has2 & CSS_Q_WIDTH) && m.rgb == 0xFF0000,
       "an empty width sets nothing and does not lose the rest of the block");
    sheet("p { width: 99999999999999px }");
    m = compute("html p", 16, 0);
    ok(m.width > 0 && m.width <= 100000,
       "a fourteen-digit length clamps instead of overflowing into a negative");
    sheet("p { width: 99999999999999999999999999px }");
    ok(compute("html p", 16, 0).width > 0, "and so does a twenty-six digit one");
    sheet("p { width: 999999999999% }");
    m = compute_w("html p", 16, 400, 0);
    ok(m.width > 0 && m.width <= 100000, "a huge percentage clamps too");
    sheet("p { width: -10px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_WIDTH), "a negative width is refused");
    sheet("p { height: -1% }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_HEIGHT), "a negative height is refused");
    sheet("p { width: 1px solid red }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_WIDTH),
       "a width with junk after it is refused, not read as 1px");
    sheet("p { width: 10 px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_WIDTH), "a space before the unit is malformed");

    /* calc() is NOT SUPPORTED, and the failure mode that matters is being
     * half-parsed into a number - `calc(100% - 20px)` read as 100 would be a
     * width three times too big and no error anywhere */
    sheet("p { width: calc(100% - 20px) }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_WIDTH), "calc() is ignored, not half-parsed");
    sheet("p { width: calc(calc(calc(calc(calc(100px))))) }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_WIDTH), "nested calc() is ignored too");
    sheet("p { gap: calc(1px + 1px) }");
    m = compute("html p", 16, 0);
    ok(!(m.has2 & CSS_Q_GAP_ROW) && !(m.has2 & CSS_Q_GAP_COL), "calc() in a gap is ignored");
    sheet("p { grid-template-columns: calc(1px) 1fr }");
    okn(compute("html p", 16, 0).n_grid_cols, 0, "calc() in a track list drops the whole list");

    sheet("p { flex-grow: -1 }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_GROW), "flex-grow: -1 is refused");
    sheet("p { flex-shrink: -0.5 }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_SHRINK), "flex-shrink: -0.5 is refused");
    sheet("p { flex-basis: -20px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_BASIS), "a negative flex-basis is refused");
    sheet("p { flex: 1 2 3 4 5 }");
    m = compute("html p", 16, 0);
    ok(!(m.has2 & CSS_Q_GROW) && !(m.has2 & CSS_Q_SHRINK) && !(m.has2 & CSS_Q_BASIS),
       "flex with five values sets NOTHING - not its first three");
    sheet("p { flex: 1 2 3 }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_GROW), "flex with three bare numbers is refused");
    sheet("p { flex: }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_GROW), "an empty flex sets nothing");
    sheet("p { flex: wibble }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_GROW), "an unrecognised flex value sets nothing");
    sheet("p { gap: -4px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_GAP_ROW), "a negative gap is refused");
    sheet("p { gap: 1px 2px 3px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_GAP_ROW), "gap takes at most two values");
    sheet("p { border-width: -3px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_BORDER_T), "a negative border-width is refused");
    sheet("p { border-radius: -8px }");
    ok(!(compute("html p", 16, 0).has2 & CSS_Q_RADIUS), "a negative radius is refused");

    sheet("p { grid-template-columns: repeat(3 }");
    m = compute("html p", 16, 0);
    ok(m.n_grid_cols == 0 && !(m.has2 & CSS_Q_GRID_COLS),
       "an unterminated repeat( leaves NO tracks, not a half-built list");
    sheet("p { grid-template-columns: repeat(3, 1fr }");
    okn(compute("html p", 16, 0).n_grid_cols, 0, "a repeat missing its close paren is refused");
    sheet("p { grid-template-columns: repeat(0, 1fr) }");
    okn(compute("html p", 16, 0).n_grid_cols, 0, "repeat(0, ...) is refused");
    sheet("p { grid-template-columns: repeat(-2, 1fr) }");
    okn(compute("html p", 16, 0).n_grid_cols, 0, "repeat with a negative count is refused");
    sheet("p { grid-template-columns: repeat(3) }");
    okn(compute("html p", 16, 0).n_grid_cols, 0, "repeat with no comma is refused");
    sheet("p { grid-template-columns: 2em 1fr }");
    okn(compute("html p", 16, 0).n_grid_cols, 0,
        "an em track is refused rather than resolved against the wrong parent");

    css_reset();
    css_add_sheet("p { grid-template-columns: repeat(999999, 1fr) }", 48);
    m = compute("html p", 16, 0);
    okn(m.n_grid_cols, CSS_GRID_MAX, "repeat(999999, 1fr) truncates at the ceiling");
    ok(css_overflowed(), "...and says so through css_overflowed()");

    css_reset();
    {
        static char many[4096];
        int n = 0;
        n += snprintf(many + n, sizeof many - n, "p { grid-template-columns:");
        for (int i = 0; i < 200; i++)
            n += snprintf(many + n, sizeof many - n, " %dpx", i + 1);
        n += snprintf(many + n, sizeof many - n, " }");
        css_add_sheet(many, n);
        m = compute("html p", 16, 0);
        okn(m.n_grid_cols, CSS_GRID_MAX, "200 tracks truncate to the ceiling");
        ok(css_overflowed(), "...and report overflow rather than overrunning the array");
        okn(m.grid_col_v[0], 1, "and the tracks kept are the FIRST ones");
        okn(m.grid_col_v[CSS_GRID_MAX - 1], CSS_GRID_MAX, "up to the ceiling, in order");
    }

    css_reset();
    {
        /* a border shorthand nobody would write and a parser has to survive */
        static char forty[512];
        int n = snprintf(forty, sizeof forty, "p { border:");
        for (int i = 0; i < 40; i++)
            n += snprintf(forty + n, sizeof forty - n, " %s",
                          (i % 3 == 0) ? "1px" : (i % 3 == 1) ? "solid" : "red");
        n += snprintf(forty + n, sizeof forty - n, " }");
        css_add_sheet(forty, n);
        m = compute("html p", 16, 0);
        ok(!(m.has2 & CSS_Q_BORDER_T) && m.border_t == 0,
           "a forty-token border shorthand is refused whole, not read as its first three");
    }

    css_reset();
    {
        /* FIVE THOUSAND DECLARATIONS IN ONE BLOCK. MAX_DECLS is 3072, so this
         * must stop taking them and say so - and the ones it did take must
         * still work, because a limit that loses the whole sheet is a limit
         * that renders a blank page. */
        static char huge[200000];
        int n = snprintf(huge, sizeof huge, "p { color: red;");
        for (int i = 0; i < 5000 && n < (int)sizeof huge - 64; i++)
            n += snprintf(huge + n, sizeof huge - n, " padding-top: 3px;");
        n += snprintf(huge + n, sizeof huge - n, " }");
        css_add_sheet(huge, n);
        ok(css_overflowed(), "5000 declarations in one block reports overflow");
        ok(css_decls() <= 3072, "and the declaration array stayed inside itself");
        m = compute("html p", 16, 0);
        okn(m.rgb, 0xFF0000, "and the declarations it DID take still apply");
        okn(m.pad_t, 3, "including the repeated one");
    }

    /* the two-hundred-selector case, where the ARENA is what runs out */
    css_reset();
    {
        static char sel[200000];
        int n = 0;
        for (int i = 0; i < 3000 && n < (int)sizeof sel - 128; i++)
            n += snprintf(sel + n, sizeof sel - n,
                          "div.wrapper%d span.inner%d { width: %dpx; border: 1px solid red }\n",
                          i, i, i + 1);
        css_add_sheet(sel, n);
        ok(css_overflowed(), "a sheet that exhausts the arena reports overflow");
        ok(css_arena_used() <= 24576, "and the arena stayed inside itself");
        okn(compute("html div.wrapper0 span.inner0", 16, 0).width, 1,
            "rules taken before the arena filled still apply");
    }

    /* ---- 18. css_overflowed() stays FALSE when nothing overflowed -------- */
    printf("\n18. overflow is not reported when nothing overflowed\n");
    sheet("p { color: red }");
    ok(!css_overflowed(), "a one-rule sheet does not report overflow");
    sheet("p { width: 99999999999999px; flex: 1 2 3 4 5; border: none none none }");
    ok(!css_overflowed(),
       "malformed values are REFUSED, which is not the same as a limit being hit");
    sheet("p { grid-template-columns: repeat(8, 1fr) }");
    m = compute("html p", 16, 0);
    ok(m.n_grid_cols == CSS_GRID_MAX && !css_overflowed(),
       "exactly CSS_GRID_MAX tracks fit, so no overflow is reported");
    sheet("p { grid-template-columns: repeat(9, 1fr) }");
    m = compute("html p", 16, 0);
    ok(m.n_grid_cols == CSS_GRID_MAX && css_overflowed(),
       "one more than fits does report overflow");
    sheet(
        "body { color: #222; font-size: 16px }\n"
        ".page { max-width: 960px; margin: 0 auto; padding: 0 16px }\n"
        ".row { display: flex; flex-flow: row wrap; gap: 12px 24px;\n"
        "       justify-content: space-between; align-items: center }\n"
        ".row .col { flex: 1 1 0%; min-width: 220px; box-sizing: border-box;\n"
        "            border: 1px solid #e0e0e0; border-radius: 6px; padding: 12px }\n"
        ".grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 16px }\n"
        "aside { float: right; width: 30%; clear: both }\n"
        ".sticky { position: sticky; top: 0; overflow: hidden }\n");
    ok(!css_overflowed(), "a realistic modern sheet fits inside the limits");

    /* ---- 19. and that sheet's values, end to end ------------------------- */
    printf("\n19. a realistic modern sheet\n");
    m = compute_w("html body div.page", 16, 1280, 0);
    ok(m.max_w == 960 && m.margin_l == CSS_AUTO && m.pad_l == 16, "the centred page wrapper");
    m = compute_w("html body div.page div.row", 16, 960, 0);
    ok(m.display == CSS_DISP_FLEX && m.flex_dir == CSS_ROW && m.flex_wrap == CSS_WRAP,
       "the flex row");
    ok(m.gap_row == 12 && m.gap_col == 24, "its gaps, row then column");
    ok(m.justify == CSS_J_BETWEEN && m.align_items == CSS_J_CENTER, "its alignment");
    m = compute_w("html body div.page div.row div.col", 16, 960, 0);
    ok(m.grow == 100 && m.shrink == 100 && m.basis == 0, "the columns are flex: 1 1 0%");
    ok(m.min_w == 220 && m.box_sizing == CSS_BOX_BORDER, "and border-box with a min-width");
    ok(m.border_t == 1 && m.border_rgb == 0xE0E0E0 && m.radius == 6, "and a 1px rounded border");
    m = compute_w("html body div.grid", 16, 960, 0);
    ok(m.display == CSS_DISP_GRID && m.n_grid_cols == 3, "the grid has three columns");
    ok(m.grid_col_unit[0] == CSS_TRACK_FR && m.grid_col_v[0] == 1, "each of them 1fr");
    m = compute_w("html body aside", 16, 1000, 0);
    ok(m.floatv == CSS_FLOAT_RIGHT && m.width == 300 && m.clearv == CSS_CLEAR_BOTH,
       "the floated aside is 30% of 1000");
    m = compute_w("html body div.sticky", 16, 960, 0);
    ok(m.position == CSS_POS_STICKY && m.top == 0 && m.overflow == CSS_OVER_HIDDEN,
       "the sticky bar");

    /* ---- 20. the API's own edges ----------------------------------------- */
    printf("\n20. the API's own edges\n");
    sheet("p { width: 100px }");
    {
        struct css_style s;
        css_style_init(&s, 16);
        css_compute(P, 0, 16, PW, 0, 0, &s);       /* an empty ancestor path */
        okn(s.width, CSS_AUTO, "an empty path matches nothing and changes nothing");
        css_compute(0, 0, 16, PW, 0, 0, 0);        /* a null out: must return */
        ok(1, "a null output style does not fault");
        css_style_init(&s, 16);
        css_compute(P, 1, 0, -50, "width: 40px", 11, &s);
        okn(s.width, 40, "a negative parent_width does not fault");
        css_style_init(&s, 16);
        css_compute(P, 1, -3, PW, "font-size: 2em", 14, &s);
        okn(s.size, 32, "a nonsense parent_size falls back to 16");
    }
    /* ---- 21. @media, evaluated rather than skipped ----------------------
     * Measured on the English Wikipedia skin: 103,793 bytes across 64 blocks
     * in 16 conditions - 38% of the sheet - was being discarded wholesale.
     * Skipping was right while nothing could read a condition. These assert
     * both directions, because a matcher that says yes to everything passes
     * every "it applied" test and one that says no to everything passes every
     * "it did not" test. */
    printf("\n21. @media\n");
    {
        struct css_style st;
        /* a matching query applies */
        css_reset(); css_viewport(1000);
        css_add_sheet("@media screen and (min-width:640px){p{color:#ff0000}}", 52);
        st = compute_w("p", 16, PW, 0);
        ok((st.has & CSS_P_COLOR) && st.rgb == 0xFF0000,
              "a matching min-width did not apply");

        /* ...and the same sheet at a width that does not match does not */
        css_reset(); css_viewport(400);
        css_add_sheet("@media screen and (min-width:640px){p{color:#ff0000}}", 52);
        st = compute_w("p", 16, PW, 0);
        ok(!(st.has & CSS_P_COLOR),
              "a min-width that does not match applied anyway");

        /* max-width, both ways */
        css_reset(); css_viewport(400);
        css_add_sheet("@media (max-width:640px){p{color:#00ff00}}", 41);
        st = compute_w("p", 16, PW, 0);
        ok((st.has & CSS_P_COLOR) && st.rgb == 0x00FF00,
              "a matching max-width did not apply");

        /* THE CONSERVATIVE HALF, and it is the one that matters: anything the
         * matcher cannot judge must REFUSE, never guess. A print sheet applied
         * to the screen is the failure this skipping existed to prevent. */
        static const char *refuse[] = {
            "@media print{p{color:#ff0000}}",
            "@media not screen{p{color:#ff0000}}",
            "@media speech{p{color:#ff0000}}",
            "@media (max-width:calc(640px - 1px)){p{color:#ff0000}}",
            "@media (orientation:landscape){p{color:#ff0000}}",
            "@media (prefers-color-scheme:dark){p{color:#ff0000}}",
            "@media screen and (min-resolution:2dppx){p{color:#ff0000}}",
            "@media (min-width:20em){p{color:#ff0000}}",
        };
        for (unsigned k = 0; k < sizeof refuse / sizeof refuse[0]; k++) {
            css_reset(); css_viewport(1000);
            css_add_sheet(refuse[k], (int)strlen(refuse[k]));
        st = compute_w("p", 16, PW, 0);
            ok(!(st.has & CSS_P_COLOR),
              "an unjudgeable query applied anyway");
        }

        /* with no viewport set, width queries refuse - the behaviour that
         * shipped before any of this existed */
        css_reset(); css_viewport(0);
        css_add_sheet("@media (min-width:1px){p{color:#ff0000}}", 39);
        st = compute_w("p", 16, PW, 0);
        ok(!(st.has & CSS_P_COLOR),
              "a width query applied with no viewport set");

        /* RULES AFTER A MEDIA BLOCK MUST STILL PARSE - the block's closing
         * brace has to be consumed as the block's, not treated as damage that
         * eats the next rule. Both the taken and the skipped path. */
        css_reset(); css_viewport(1000);
        css_add_sheet("@media screen{p{color:#ff0000}} a{color:#0000ff}", 47);
        st = compute_w("a", 16, PW, 0);
        ok((st.has & CSS_P_COLOR) && st.rgb == 0x0000FF,
              "a rule after a TAKEN media block was lost");
        css_reset(); css_viewport(1000);
        css_add_sheet("@media print{p{color:#ff0000}} a{color:#0000ff}", 46);
        st = compute_w("a", 16, PW, 0);
        ok((st.has & CSS_P_COLOR) && st.rgb == 0x0000FF,
              "a rule after a SKIPPED media block was lost");

        /* a comma list matches if ANY query does */
        css_reset(); css_viewport(1000);
        css_add_sheet("@media print,screen{p{color:#00ffff}}", 36);
        st = compute_w("p", 16, PW, 0);
        ok((st.has & CSS_P_COLOR) && st.rgb == 0x00FFFF,
              "a comma list did not match on its second query");

        /* malformed conditions must not fault or run away */
        static const char *bad[] = {
            "@media", "@media {", "@media (", "@media ()", "@media (:)",
            "@media screen and", "@media ((((", "@media (min-width:)",
            "@media (min-width:1px", "@media{p{color:red}}",
        };
        for (unsigned k = 0; k < sizeof bad / sizeof bad[0]; k++) {
            css_reset(); css_viewport(1000);
            css_add_sheet(bad[k], (int)strlen(bad[k]));
        }
        ok(1,
              "malformed @media did not fault");
    }


    printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
