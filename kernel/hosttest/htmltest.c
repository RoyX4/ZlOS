/* htmltest.c - the parser and the box model, asserted, with no kernel.
 *
 * html.c and layout.c talk to exactly one thing outside themselves: a function
 * that measures a string. Inject a synthetic one and both files become
 * ordinary C programs, which is why this harness exists at all - the
 * alternative is a QEMU boot per change, and a boot cannot tell you that a
 * wrapped list item's second line is one pixel out.
 *
 * The measure is deliberately NOT the real font: every character is size/2
 * wide, so every number below is one that can be worked out by hand. A test
 * whose expected value came from running the code proves only that the code
 * still does what it did.
 *
 * WHAT THIS IS FOR, in order of how much debugging it saves:
 *   1. malformed input recovers rather than faults - the whole point of §4's
 *      item 5 gate, and unprovable by looking at a rendered page
 *   2. reflow: the same document at three widths, asserted numerically
 *   3. nothing ever escapes the content box, including a single word that is
 *      wider than the entire line
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../html.h"
#include "../layout.h"

static int fails, checks;

#define CHECK(cond, ...) do {                                    \
    checks++;                                                    \
    if (!(cond)) {                                               \
        fails++;                                                 \
        printf("  FAIL %s:%d  ", __FILE__, __LINE__);            \
        printf(__VA_ARGS__);                                     \
        printf("\n");                                            \
    }                                                            \
} while (0)

/* Every glyph half an em wide. Bold is 10% wider, which is what a double
 * strike really costs, and mono is a flat 0.6em - the same shape as the real
 * metrics without any of their data. */
static int fake_measure(const char *s, int len, int size, int style)
{
    (void)s;
    int w = len * size / 2;
    if (style & LS_MONO) w = len * size * 6 / 10;
    if (style & LS_BOLD) w = w * 11 / 10;
    return w;
}

/* ---- documents ------------------------------------------------------------ */
static const char *clean_doc =
    "<html><head><title>A Page</title></head><body>\n"
    "<h1>Heading</h1>\n"
    "<p>Some <strong>bold</strong> and <em>italic</em> text.</p>\n"
    "<ul><li>one</li><li>two</li></ul>\n"
    "<p>A <a href=\"http://example.com/x\">link</a> here.</p>\n"
    "<hr>\n"
    "</body></html>\n";

/* Every recovery path in one document, on purpose:
 *   unclosed <p>, stray </div> with nothing open, an attribute with no quotes,
 *   an attribute containing '>', a bare '<' that is not a tag, an unknown
 *   element, a </br>, an unterminated tag at EOF, and a <script> whose body
 *   would parse as markup if it were not skipped. */
static const char *broken_doc =
    "<p>first\n"
    "<p>second</div>\n"
    "<a href=http://x.example/y>unquoted</a>\n"
    "<a href=\"http://x/?a=1&amp;b=2\" title=\"a > b\">tricky</a>\n"
    "5 < 6 and 7 > 3\n"
    "<section>unknown element</section>\n"
    "<br></br>\n"
    "<script>var a = '<p>not markup</p>';</script>\n"
    "<style>p { color: red }</style>\n"
    "&amp; &lt; &gt; &quot; &#65; &#x42; &notreal;\n"
    "<em>never closed\n"
    "<p attr";

static void t_clean(void)
{
    printf("clean document\n");
    int n = html_parse(clean_doc, (int)strlen(clean_doc));
    CHECK(n > 0, "no nodes");

    /* the shape, counted by hand from the source above:
     *   html > head(dropped from render, still a node) body
     *   body > h1 p ul p hr
     *   h1 > text;  p > text strong text em text
     *   ul > li li;  li > text
     *   p > text a text ; a > text
     * head has no children because <title> is captured as a string, not a node */
    int root = html_root();
    CHECK(html_tag(root) == HT_HTML, "root is %d not html", html_tag(root));

    int body = -1;
    for (int c = html_first(root); c >= 0; c = html_next(c))
        if (html_tag(c) == HT_BODY) body = c;
    CHECK(body >= 0, "no body");

    int kids = 0, h1 = -1, hr = 0, ul = -1, np = 0;
    for (int c = html_first(body); c >= 0; c = html_next(c)) {
        kids++;
        if (html_tag(c) == HT_H1) h1 = c;
        if (html_tag(c) == HT_HR) hr++;
        if (html_tag(c) == HT_UL) ul = c;
        if (html_tag(c) == HT_P)  np++;
    }
    CHECK(h1 >= 0, "no h1");
    CHECK(hr == 1, "hr count %d", hr);
    CHECK(np == 2, "p count %d", np);
    CHECK(ul >= 0, "no ul");

    int li = 0;
    for (int c = html_first(ul); c >= 0; c = html_next(c))
        if (html_tag(c) == HT_LI) li++;
    CHECK(li == 2, "li count %d", li);

    int tl;
    const char *title = html_title(&tl);
    CHECK(tl == 6 && !memcmp(title, "A Page", 6), "title '%.*s'", tl, title);

    /* the href survived, decoded, and belongs to the <a> */
    int found_href = 0;
    for (int i = 0; i < html_count(); i++) {
        if (html_tag(i) != HT_A) continue;
        int hl;
        const char *h = html_href(i, &hl);
        if (hl == 22 && !memcmp(h, "http://example.com/x", 20)) found_href = 1;
        if (hl == 20 && !memcmp(h, "http://example.com/x", 20)) found_href = 1;
    }
    CHECK(found_href, "href not recovered");

    /* a well-formed document must need NO recovery at all - if this fires,
     * one of the recovery paths is being taken on valid markup */
    CHECK(html_dropped() == 0, "%d recoveries on a clean document", html_dropped());
}

static void t_broken(void)
{
    printf("malformed document\n");
    int n = html_parse(broken_doc, (int)strlen(broken_doc));

    /* §4 item 5's gate: assert the node count and the depth, not "it did not
     * crash". Both are properties of the RECOVERY, not of the input. */
    CHECK(n > 10, "only %d nodes from the broken document", n);
    CHECK(n < 100, "%d nodes - the recovery is inventing structure", n);

    int d = html_max_depth();
    CHECK(d >= 2, "depth %d - nothing nested", d);
    CHECK(d <= 6, "depth %d - a stray close tag unwound the stack", d);

    /* two unclosed <p> must be SIBLINGS, not one inside the other. That single
     * assertion is the entire implied-close rule. */
    int ps[8], np = 0;
    for (int i = 0; i < html_count() && np < 8; i++)
        if (html_tag(i) == HT_P) ps[np++] = i;
    CHECK(np >= 2, "found %d p elements", np);
    if (np >= 2)
        CHECK(html_parent(ps[0]) == html_parent(ps[1]),
              "unclosed <p> nested: parents %d and %d",
              html_parent(ps[0]), html_parent(ps[1]));

    /* the script and the style bodies must not be anywhere in the text */
    for (int i = 0; i < html_count(); i++) {
        if (html_kind(i) != HN_TEXT) continue;
        int len;
        const char *s = html_text(i, &len);
        CHECK(!memmem(s, len, "not markup", 10), "script body rendered");
        CHECK(!memmem(s, len, "color: red", 10), "style body rendered");
    }

    /* entities: the five named ones, decimal and hex, and one that is not an
     * entity at all and must survive as literal text */
    char all[4096];
    int alen = 0;
    for (int i = 0; i < html_count(); i++) {
        if (html_kind(i) != HN_TEXT) continue;
        int len;
        const char *s = html_text(i, &len);
        if (alen + len < (int)sizeof all) { memcpy(all + alen, s, len); alen += len; }
    }
    all[alen] = 0;
    CHECK(memmem(all, alen, "& < > \" A B", 11) != 0,
          "entities decoded wrong in: %.*s", alen, all);
    CHECK(memmem(all, alen, "&notreal;", 9) != 0, "unknown entity was eaten");
    CHECK(memmem(all, alen, "5 < 6 and 7 > 3", 15) != 0, "bare '<' broke the text");
    CHECK(memmem(all, alen, "unknown element", 15) != 0, "unknown tag ate its text");

    /* an unquoted href, and one holding a '>' inside quotes */
    int unq = 0, tricky = 0;
    for (int i = 0; i < html_count(); i++) {
        if (html_tag(i) != HT_A) continue;
        int hl;
        const char *h = html_href(i, &hl);
        if (hl && memmem(h, hl, "x.example/y", 11)) unq = 1;
        if (hl && memmem(h, hl, "?a=1&b=2", 8))     tricky = 1;
    }
    CHECK(unq, "unquoted attribute value lost");
    CHECK(tricky, "'>' inside a quoted attribute ended the tag");

    /* it recovered, and it knows it did */
    CHECK(html_dropped() > 0, "no recoveries recorded on a broken document");

    /* An unsupported element must be CLOSED by its own close tag. Every
     * unknown tag shares one id, so this only works because the name is
     * compared too - and when it did not, one <section> nested the entire
     * remainder of the document inside itself. The check is structural: the
     * text after </section> must not be a descendant of it. */
    int sect = -1;
    for (int i = 0; i < html_count(); i++)
        if (html_tag(i) == HT_UNKNOWN) { sect = i; break; }
    CHECK(sect >= 0, "unknown element was dropped entirely");
    if (sect >= 0) {
        int after = -1;
        for (int i = 0; i < html_count(); i++) {
            if (html_kind(i) != HN_TEXT) continue;
            int len;
            const char *s = html_text(i, &len);
            if (len && memmem(s, len, "never closed", 12)) after = i;
        }
        CHECK(after >= 0, "text after the unknown element vanished");
        for (int p = after; p >= 0; p = html_parent(p))
            CHECK(p != sect, "content after </section> is still inside it");
    }
}

/* A close tag for an element that is not the innermost open one must close
 * back THROUGH the ones in between, and a close tag whose name matches nothing
 * must change nothing at all. Those two are the whole stack discipline. */
static void t_close_matching(void)
{
    printf("close-tag matching\n");
    html_parse("<div><section><span>x</span></footer>y</section>z</div>", 54);
    int sect = -1, div = -1;
    for (int i = 0; i < html_count(); i++) {
        if (html_tag(i) == HT_UNKNOWN && sect < 0) sect = i;
        if (html_tag(i) == HT_DIV && div < 0) div = i;
    }
    CHECK(sect >= 0 && div >= 0, "missing div/section");
    int ty = -1, tz = -1;
    for (int i = 0; i < html_count(); i++) {
        if (html_kind(i) != HN_TEXT) continue;
        int len;
        const char *s = html_text(i, &len);
        if (len && s[0] == 'y') ty = i;
        if (len && s[0] == 'z') tz = i;
    }
    CHECK(ty >= 0 && tz >= 0, "text lost (y=%d z=%d)", ty, tz);
    /* </footer> matches nothing: 'y' is still inside the section */
    if (ty >= 0 && sect >= 0) {
        int inside = 0;
        for (int p = ty; p >= 0; p = html_parent(p)) if (p == sect) inside = 1;
        CHECK(inside, "an unmatched close tag popped the stack");
    }
    /* </section> closed it: 'z' is in the div, not the section */
    if (tz >= 0 && sect >= 0) {
        int inside = 0;
        for (int p = tz; p >= 0; p = html_parent(p)) if (p == sect) inside = 1;
        CHECK(!inside, "</section> did not close its element");
    }
}

/* ---- layout --------------------------------------------------------------- */
static int worst_overflow(int width)
{
    int worst = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->x < 0) worst = worst > -r->x ? worst : -r->x;
        int over = r->x + r->w - width;
        if (over > worst) worst = over;
    }
    return worst;
}

/* Two runs on the same line box must not sit on top of each other. Runs are
 * emitted in reading order, so "same line" is "same y, and the previous run's
 * right edge is at or before this one's left". */
static int overlaps(void)
{
    int bad = 0;
    for (int i = 1; i < lay_count(); i++) {
        const struct lay_run *a = lay_at(i - 1), *b = lay_at(i);
        if (a->kind == LR_BULLET || b->kind == LR_BULLET) continue;
        if (a->y != b->y) continue;
        if (b->x < a->x + a->w) bad++;
    }
    return bad;
}

static void t_layout(void)
{
    printf("layout and reflow\n");
    const char *doc =
        "<h1>Reflow</h1>"
        "<p>The quick brown fox jumps over the lazy dog and keeps on running "
        "well past the end of any one line so that the wrap has real work.</p>"
        /* INLINE <code> - it wraps. It shares a font with <pre> and nothing
         * else, and when it shared the no-wrap path too it ran off the right
         * edge on every width. The overflow assertion below is the one that
         * should have caught it and could not, because no test document
         * contained a <code>. */
        "<p>Call <code>fb_text_rich_w</code> then <code>lay_run_doc</code> and "
        "then <code>browser_draw</code> to put a document on a screen.</p>"
        "<ul><li>alpha beta gamma</li><li>delta</li></ul>"
        "<ol><li>first</li><li>second</li></ol>";
    html_parse(doc, (int)strlen(doc));
    lay_set_measure(fake_measure);

    int widths[3] = { 800, 400, 200 };
    int h[3], lines[3];
    for (int k = 0; k < 3; k++) {
        h[k] = lay_run_doc(widths[k], 16);
        lines[k] = lay_lines();
        CHECK(lay_overflowed() == 0, "run array overflowed at width %d", widths[k]);
        CHECK(worst_overflow(widths[k]) == 0,
              "content escaped the box by %dpx at width %d",
              worst_overflow(widths[k]), widths[k]);
        CHECK(overlaps() == 0, "%d overlapping runs at width %d",
              overlaps(), widths[k]);
        CHECK(h[k] > 0, "zero height at width %d", widths[k]);
    }

    /* THE ASSERTION THAT PROVES IT IS A LAYOUT ENGINE and not a fixed drawing:
     * narrower means more lines and a taller document, every time. */
    CHECK(lines[1] > lines[0], "400px gave %d lines, 800px gave %d",
          lines[1], lines[0]);
    CHECK(lines[2] > lines[1], "200px gave %d lines, 400px gave %d",
          lines[2], lines[1]);
    CHECK(h[1] > h[0] && h[2] > h[1], "heights %d %d %d not increasing",
          h[0], h[1], h[2]);

    /* ...and it is REPEATABLE: laying out at 800 again must reproduce the
     * first result exactly. A layout with leftover state does not. */
    int again = lay_run_doc(800, 16);
    CHECK(again == h[0], "re-layout at 800 gave %d, first gave %d", again, h[0]);
    CHECK(lay_lines() == lines[0], "re-layout line count drifted");

    /* the ordered list produced markers 1. and 2. */
    int m1 = 0, m2 = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind != LR_TEXT) continue;
        if (r->len == 2 && !memcmp(r->text, "1.", 2)) m1 = 1;
        if (r->len == 2 && !memcmp(r->text, "2.", 2)) m2 = 1;
    }
    CHECK(m1 && m2, "ordered list markers missing (%d %d)", m1, m2);

    /* a heading is bigger than body text, or the type scale does nothing */
    int hsize = 0, psize = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind != LR_TEXT) continue;
        if (r->len == 6 && !memcmp(r->text, "Reflow", 6)) hsize = r->size;
        if (r->len == 5 && !memcmp(r->text, "quick", 5)) psize = r->size;
    }
    CHECK(hsize > psize && psize > 0, "h1 %d body %d", hsize, psize);
}

static void t_long_word(void)
{
    printf("a word wider than the line\n");
    const char *doc =
        "<p>x http://example.com/a/very/long/path/that/cannot/possibly/fit/"
        "inside/two/hundred/pixels/at/all y</p>";
    html_parse(doc, (int)strlen(doc));
    lay_set_measure(fake_measure);
    lay_run_doc(200, 16);
    CHECK(worst_overflow(200) == 0,
          "long word escaped by %dpx", worst_overflow(200));
    CHECK(lay_count() > 3, "long word was dropped rather than broken");
    CHECK(lay_lines() > 1, "long word did not break across lines");
}

static void t_pre(void)
{
    printf("preformatted text\n");
    const char *doc = "<pre>a  b\n  c\nd</pre>";
    html_parse(doc, (int)strlen(doc));
    lay_set_measure(fake_measure);
    lay_run_doc(800, 16);
    /* the double space and the leading spaces survive: <pre> is the one place
     * the parser must not collapse whitespace */
    int found = 0, mono = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind != LR_TEXT) continue;
        if (r->style & LS_MONO) mono++;
        if (r->len == 4 && !memcmp(r->text, "a  b", 4)) found = 1;
    }
    CHECK(found, "pre collapsed its whitespace");
    CHECK(mono >= 2, "pre text not monospaced (%d runs)", mono);
    CHECK(lay_lines() >= 3, "pre gave %d lines, wanted 3", lay_lines());
}

static void t_empty(void)
{
    printf("degenerate input\n");
    /* none of these may fault, and all of them must produce a usable tree */
    html_parse("", 0);
    lay_set_measure(fake_measure);
    CHECK(lay_run_doc(300, 16) >= 0, "empty document");
    html_parse(0, 0);
    CHECK(lay_run_doc(300, 16) >= 0, "null document");
    html_parse("<", 1);
    CHECK(lay_run_doc(300, 16) >= 0, "a lone '<'");
    html_parse("<<<<>>>>", 8);
    CHECK(lay_run_doc(300, 16) >= 0, "angle bracket soup");
    html_parse("</p></div></a>", 14);
    CHECK(lay_run_doc(300, 16) >= 0, "close tags only");
    html_parse("<p", 2);
    CHECK(lay_run_doc(300, 16) >= 0, "a tag that never ends");
    html_parse("<a href=", 8);
    CHECK(lay_run_doc(300, 16) >= 0, "an attribute that never ends");
    html_parse("&", 1);
    CHECK(lay_run_doc(300, 16) >= 0, "a lone ampersand");
    html_parse("&#999999999999;", 15);
    CHECK(lay_run_doc(300, 16) >= 0, "an absurd numeric entity");
    /* a layout with a zero or negative width must still terminate */
    html_parse("<p>hello world</p>", 18);
    CHECK(lay_run_doc(0, 16) >= 0, "zero width");
    CHECK(lay_run_doc(-5, 16) >= 0, "negative width");
    CHECK(lay_run_doc(300, 0) >= 0, "zero font size");
}

static void t_deep(void)
{
    printf("pathological nesting\n");
    /* 400 nested divs against a 32-deep stack: the excess must be dropped
     * without corrupting anything, and the document must still render */
    static char buf[16384];
    int n = 0;
    for (int i = 0; i < 400; i++) { memcpy(buf + n, "<div>", 5); n += 5; }
    memcpy(buf + n, "deep", 4); n += 4;
    for (int i = 0; i < 400; i++) { memcpy(buf + n, "</div>", 6); n += 6; }
    html_parse(buf, n);
    CHECK(html_max_depth() <= 40, "depth %d escaped the stack", html_max_depth());
    lay_set_measure(fake_measure);
    CHECK(lay_run_doc(400, 16) > 0, "deeply nested document laid out to nothing");

    /* more nodes than the array holds: it must stop, not scribble */
    n = 0;
    for (int i = 0; i < 3000 && n < (int)sizeof(buf) - 8; i++) {
        memcpy(buf + n, "<p>x", 4); n += 4;
    }
    html_parse(buf, n);
    CHECK(html_count() <= 1024, "node array overran: %d", html_count());
    CHECK(lay_run_doc(400, 16) > 0, "overflowing document laid out to nothing");
}

int main(void)
{
    printf("html.c + layout.c, no kernel\n\n");
    t_clean();
    t_broken();
    t_close_matching();
    t_layout();
    t_long_word();
    t_pre();
    t_empty();
    t_deep();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
