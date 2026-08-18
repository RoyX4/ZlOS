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

static struct css_style compute(const char *path, int parent, const char *inl)
{
    struct css_style s;
    memset(&s, 0, sizeof s);
    s.rgb = -1; s.bg = -1; s.size = parent; s.display = CSS_DISP_INLINE;
    int n = path_of(path);
    css_compute(P, n, parent, inl, inl ? (int)strlen(inl) : 0, &s);
    return s;
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
    ok(m.margin_t == 0 && m.margin_l == 0,
       "margin: 0 auto does not abort the declaration");
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
        ok(css_arena_used() <= 12288, "the arena stayed inside itself");
        ok(css_decls() <= 1024, "the declaration array stayed inside itself");
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

    printf("\n%d checks, %d failed\n", checks, failures);
    return failures ? 1 : 0;
}
