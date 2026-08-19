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
#include "../css.h"

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
    /* enough <p>x to overflow HTML_MAX_NODES twice over - each pair is two
     * nodes, so this must outrun the cap however the cap is set */
    static char over[HTML_MAX_NODES * 8];
    for (int i = 0; i < HTML_MAX_NODES * 2 && n < (int)sizeof(over) - 8; i++) {
        memcpy(over + n, "<p>x", 4); n += 4;
    }
    html_parse(over, n);
    CHECK(html_count() <= HTML_MAX_NODES, "node array overran: %d", html_count());
    CHECK(lay_run_doc(400, 16) > 0, "overflowing document laid out to nothing");
}


/* ---- CSS, end to end ------------------------------------------------------
 * csstest asserts the ENGINE - selectors, cascade, values - with no html.c and
 * no layout.c linked. This asserts the WIRING, which is a different thing and
 * has already been wrong once in a way csstest could not see: text-align was
 * computed correctly and applied to nothing, because layout.c restored the
 * parent's alignment before closing the line box that needed it. Every
 * per-property assertion passed while the page rendered left-aligned.
 *
 * So these run a whole styled document and look at the runs that come out.
 */
static void t_css(void)
{
    static const char page[] =
        "<html><head><style>\n"
        "  body { color: #222222 }\n"
        "  h1 { color: #003366; font-size: 3em }\n"
        "  .hide { display: none }\n"
        "  .big { font-size: 200%; font-weight: bold }\n"
        "  p.lead { text-align: center }\n"
        "  code { background: #f4f4f4 }\n"
        "</style></head><body>\n"
        "<h1>Styled</h1>\n"
        "<p class=\"lead\">centred</p>\n"
        "<p class=\"hide\">MUST NOT APPEAR</p>\n"
        "<p>plain <span class=\"big\">big</span> <code>mono</code></p>\n"
        "<p style=\"color: #00ff00\">inline</p>\n"
        "</body></html>\n";

    html_parse(page, (int)sizeof page - 1);
    CHECK(html_sheets() == 1, "the <style> block was not captured (%d)", html_sheets());
    css_reset();
    for (int k = 0; k < html_sheets(); k++) {
        int l;
        const char *sh = html_sheet(k, &l);
        css_add_sheet(sh, l);
    }
    CHECK(css_rules() >= 6, "only %d rules parsed from the page", css_rules());

    lay_set_measure(fake_measure);
    CHECK(lay_run_doc(400, 16) > 0, "the styled document laid out to nothing");

    int saw_h1 = 0, saw_big = 0, saw_bg = 0, saw_inline = 0, saw_hidden = 0;
    int lead_x = -1, plain_rgb = -1;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind != LR_TEXT || r->len <= 0) continue;
        if (r->len == 6 && !memcmp(r->text, "Styled", 6)) {
            saw_h1 = 1;
            CHECK(r->size == 48, "h1 font-size: 3em of 16 should be 48, got %d", r->size);
            CHECK(r->rgb == 0x003366, "h1 colour, got %d", r->rgb);
        }
        if (r->len == 7 && !memcmp(r->text, "centred", 7)) lead_x = r->x;
        if (r->len == 3 && !memcmp(r->text, "big", 3)) {
            saw_big = 1;
            CHECK(r->size == 32, "200%% of 16 should be 32, got %d", r->size);
            CHECK((r->style & LS_BOLD) != 0, "font-weight: bold did not reach the run");
        }
        if (r->len == 4 && !memcmp(r->text, "mono", 4)) {
            saw_bg = 1;
            CHECK(r->bg == 0xF4F4F4, "code background, got %d", r->bg);
        }
        if (r->len == 6 && !memcmp(r->text, "inline", 6)) {
            saw_inline = 1;
            CHECK(r->rgb == 0x00FF00, "style= colour, got %d", r->rgb);
        }
        if (r->len == 5 && !memcmp(r->text, "plain", 5)) plain_rgb = r->rgb;
        if (r->len == 4 && !memcmp(r->text, "MUST", 4)) saw_hidden = 1;
    }
    CHECK(saw_h1, "the h1 produced no run");
    CHECK(saw_big, "the .big span produced no run");
    CHECK(saw_bg, "the <code> produced no run");
    CHECK(saw_inline, "the style= paragraph produced no run");
    CHECK(!saw_hidden, "display:none rendered its text anyway");
    CHECK(plain_rgb == 0x222222, "body colour did not inherit, got %d", plain_rgb);

    /* THE ONE THAT REGRESSED. Centred in 400px, so it must not start at the
     * left edge - and it must not be pushed outside the box either. */
    CHECK(lead_x > 0, "text-align: center did not move the line (x=%d)", lead_x);
    CHECK(lead_x < 400, "text-align: center pushed the line out of the box (x=%d)", lead_x);

    /* A SECOND DOCUMENT MUST NOT INHERIT THE FIRST ONE'S SHEET. This is the
     * bug that only ever shows on the second navigation. */
    static const char plain_page[] = "<html><body><h1>Unstyled</h1></body></html>";
    html_parse(plain_page, (int)sizeof plain_page - 1);
    css_reset();
    for (int k = 0; k < html_sheets(); k++) {
        int l;
        const char *sh = html_sheet(k, &l);
        css_add_sheet(sh, l);
    }
    lay_run_doc(400, 16);
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind == LR_TEXT && r->len == 8 && !memcmp(r->text, "Unstyled", 8))
            CHECK(r->rgb == LR_NO_RGB,
                  "the previous page's stylesheet leaked into the next (%d)", r->rgb);
    }
}


/* ---- tables ---------------------------------------------------------------
 * A table is the first thing in this layout engine that CANNOT be done in one
 * pass: a column's width depends on cells that have not been read yet. So the
 * assertions here are mostly about geometry rather than about parsing - that
 * cells in a row share a top, that columns line up down the table, and that
 * nothing escapes the content box, which is the failure a one-pass
 * implementation produces.
 *
 * The markup is deliberately written WITHOUT closing tags, because that is how
 * real tables are written and it is what the implied-close rules exist for.
 */
static void t_table(void)
{
    static const char doc[] =
        "<html><body><table>"
        "<tr><th>Name<th>Value"
        "<tr><td>alpha<td>1"
        "<tr><td>a much longer cell than the others<td>2"
        "</table></body></html>";
    html_parse(doc, (int)sizeof doc - 1);
    css_reset();
    lay_set_measure(fake_measure);
    int h = lay_run_doc(400, 16);
    CHECK(h > 0, "the table laid out to nothing");

    int nfound = 0, minx = 1 << 30, maxx = 0;
    int name_x = -1, value_x = -1, alpha_x = -1, one_x = -1;
    int name_y = -1, value_y = -1;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind != LR_TEXT || r->len <= 0) continue;
        nfound++;
        if (r->x < minx) minx = r->x;
        if (r->x + r->w > maxx) maxx = r->x + r->w;
        if (r->len == 4 && !memcmp(r->text, "Name", 4))  { name_x = r->x;  name_y = r->y; }
        if (r->len == 5 && !memcmp(r->text, "Value", 5)) { value_x = r->x; value_y = r->y; }
        if (r->len == 5 && !memcmp(r->text, "alpha", 5)) alpha_x = r->x;
        if (r->len == 1 && r->text[0] == '1')            one_x = r->x;
    }
    CHECK(nfound > 0, "the table produced no text runs");
    CHECK(name_x >= 0 && value_x >= 0, "the header cells did not render");
    CHECK(alpha_x >= 0 && one_x >= 0, "the body cells did not render");

    /* the two columns must actually be two columns */
    CHECK(value_x > name_x, "column 2 is not right of column 1 (%d vs %d)", value_x, name_x);
    /* cells of one row share a top */
    CHECK(name_y == value_y, "cells in a row do not share a top (%d vs %d)", name_y, value_y);
    /* and the columns line up down the table */
    CHECK(alpha_x == name_x, "column 1 does not line up (%d vs %d)", alpha_x, name_x);
    CHECK(one_x == value_x, "column 2 does not line up (%d vs %d)", one_x, value_x);
    /* nothing escapes - this is what a one-pass table gets wrong */
    CHECK(maxx <= 400, "a cell escaped the content box (%d > 400)", maxx);
    CHECK(minx >= 0, "a cell started left of the box (%d)", minx);

    /* a header cell is bold, a body cell is not */
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind != LR_TEXT) continue;
        if (r->len == 4 && !memcmp(r->text, "Name", 4))
            CHECK((r->style & LS_BOLD) != 0, "<th> is not bold");
        if (r->len == 5 && !memcmp(r->text, "alpha", 5))
            CHECK((r->style & LS_BOLD) == 0, "<td> is bold");
    }

    /* a narrow window must still not overflow - the scale-down path */
    lay_run_doc(80, 16);
    int over = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->x + r->w > 80) over++;
    }
    CHECK(over == 0, "%d runs escaped an 80px table", over);

    /* degenerate tables must not fault */
    static const char junk[] = "<table><tr><tr></table><table></table>"
                               "<table><td>orphan cell</table>";
    html_parse(junk, (int)sizeof junk - 1);
    lay_run_doc(300, 16);
    CHECK(1, "degenerate tables did not fault");
}

/* ---- the box model, flex, grid, floats and positioning ---------------------
 * EVERYTHING BELOW ASSERTS A NUMBER, and every number is one that can be
 * worked out by hand from the document above it - fake_measure makes a glyph
 * exactly size/2 wide, so at the 16px body size used here a character is 8px
 * and a line box is 20px tall. A test whose expected value came from running
 * the code proves only that the code still does what it did, and that is worth
 * approximately nothing when the code is new.
 *
 * The other half of this is the ADVERSARIAL section at the end. Five of six
 * bugs last session were invisible to a passing suite and four only showed at
 * a boundary; the comfortable case has passed every time it was tried. So a
 * flex container narrower than one item's minimum, a grid of eight fractions
 * in three pixels, a float array that fills up, and an image with a zero
 * intrinsic dimension all get a case of their own.
 */
static void load(const char *doc, int len)
{
    html_parse(doc, len);
    css_reset();
    for (int k = 0; k < html_sheets(); k++) {
        int l;
        const char *sh = html_sheet(k, &l);
        css_add_sheet(sh, l);
    }
    lay_set_measure(fake_measure);
}

static int find_text(const char *s)
{
    int n = (int)strlen(s);
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind == LR_TEXT && r->len == n && !memcmp(r->text, s, n)) return i;
    }
    return -1;
}
static int tx(const char *s) { int i = find_text(s); return i < 0 ? -1 : lay_at(i)->x; }
static int ty(const char *s) { int i = find_text(s); return i < 0 ? -1 : lay_at(i)->y; }

/* A BLOCK'S BOX IS FOUND BY ITS BACKGROUND COLOUR, which is why every document
 * below gives each box a different one: a run index is not a stable name for a
 * box when the thing under test is how many runs get emitted. */
static int find_box(int rgb)
{
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind == LR_BOX && r->bg == rgb) return i;
    }
    return -1;
}
static int count_kind(int kind)
{
    int c = 0;
    for (int i = 0; i < lay_count(); i++) if (lay_at(i)->kind == kind) c++;
    return c;
}
/* the four border edges of a box, identified by their author colour */
static int count_rule_rgb(int rgb)
{
    int c = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->kind == LR_RULE && r->rgb == rgb && r->w > 0 && r->h > 0) c++;
    }
    return c;
}

static void t_box(void)
{
    printf("the box: width, min/max, box-sizing, auto margins, borders\n");

    /* THE ONE THAT MATTERS MOST. `max-width: 200px; margin: 0 auto` is the
     * page-layout idiom of the modern web, and it used to centre nothing at
     * all - auto margins were computed and then thrown away. 600 - 200 = 400
     * of slack, half on each side, so the content starts at exactly 200. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .page { max-width: 200px; margin-left: auto; margin-right: auto;"
            "          background: #010101 }"
            "</style></head><body><div class=\"page\">HI</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("HI") == 200, "margin:0 auto did not centre: x=%d, wanted 200", tx("HI"));
        int b = find_box(0x010101);
        CHECK(b >= 0, "the centred block emitted no background box");
        if (b >= 0) {
            CHECK(lay_at(b)->x == 200, "box x=%d, wanted 200", lay_at(b)->x);
            CHECK(lay_at(b)->w == 200, "box w=%d, wanted 200", lay_at(b)->w);
        }
        CHECK(worst_overflow(600) == 0, "the centred block escaped the box");
    }

    /* one auto margin pushes the whole slack to that side */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .r { width: 100px; margin-left: auto; background: #020202 }"
            "</style></head><body><div class=\"r\">R</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        CHECK(tx("R") == 300, "a single auto margin: x=%d, wanted 300", tx("R"));
    }

    /* PADDING ACTUALLY INSETS. Before this it affected nothing at all on a
     * block: pad_l called indent() and pad_r, pad_t and pad_b did nothing. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .p { padding: 5px 10px 7px 30px; background: #030303 }"
            "</style></head><body><p>lead</p><div class=\"p\">P</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int b = find_box(0x030303);
        CHECK(b >= 0, "the padded block emitted no box");
        CHECK(tx("P") == 30, "padding-left did not inset the content: x=%d", tx("P"));
        if (b >= 0) {
            const struct lay_run *r = lay_at(b);
            CHECK(r->w == 600, "a padded box is still full width: w=%d", r->w);
            CHECK(ty("P") == r->y + 5, "padding-top did not inset: text y=%d box y=%d",
                  ty("P"), r->y);
            /* one 20px line, 5 above and 7 below */
            CHECK(r->h == 32, "box height %d, wanted 5+20+7", r->h);
        }
    }

    /* BOX-SIZING, ASSERTED AS THE TWO DIFFERENT NUMBERS IT PRODUCES. A
     * content-box 100 with 10 of padding each side is a 120 wide box; a
     * border-box 100 is a 100 wide box with 80 of content. Using content-box
     * arithmetic for border-box is a 20px error that no "did it render" check
     * can see. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  div { padding-left: 10px; padding-right: 10px; width: 100px }"
            "  .cb { background: #040404 }"
            "  .bb { box-sizing: border-box; background: #050505 }"
            "</style></head><body>"
            "<div class=\"cb\">C</div><div class=\"bb\">B</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int c = find_box(0x040404), b = find_box(0x050505);
        CHECK(c >= 0 && b >= 0, "box-sizing boxes missing (%d %d)", c, b);
        if (c >= 0) CHECK(lay_at(c)->w == 120, "content-box w=%d, wanted 120", lay_at(c)->w);
        if (b >= 0) CHECK(lay_at(b)->w == 100, "border-box w=%d, wanted 100", lay_at(b)->w);
    }

    /* MIN BEATS MAX when a document sets them the wrong way round. Not a
     * curiosity: min-width is a guarantee and max-width is a preference. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .mm { width: 500px; min-width: 300px; max-width: 100px;"
            "        background: #060606 }"
            "</style></head><body><div class=\"mm\">M</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int b = find_box(0x060606);
        CHECK(b >= 0, "the clamped block emitted no box");
        if (b >= 0) CHECK(lay_at(b)->w == 300,
                          "max-width beat min-width: w=%d, wanted 300", lay_at(b)->w);
    }

    /* A BORDER IS FOUR LR_RULE RUNS, and a zero-width side emits nothing. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .b4 { border: 3px solid #ff00ff; background: #070707 }"
            "  .b3 { border-width: 0 4px 6px 8px; border-color: #00ffff }"
            "</style></head><body>"
            "<div class=\"b4\">F</div><div class=\"b3\">T</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(count_rule_rgb(0xFF00FF) == 4, "a 4-sided border gave %d edges",
              count_rule_rgb(0xFF00FF));
        CHECK(count_rule_rgb(0x00FFFF) == 3,
              "a border with one 0-width side gave %d edges, wanted 3",
              count_rule_rgb(0x00FFFF));
        /* the content is inside the border, not on top of it */
        CHECK(tx("F") == 3, "a 3px border did not inset the content: x=%d", tx("F"));
        CHECK(tx("T") == 8, "an 8px left border did not inset: x=%d", tx("T"));
        int b = find_box(0x070707);
        if (b >= 0) CHECK(lay_at(b)->h == 26, "3+20+3 = 26, got %d", lay_at(b)->h);
    }

    /* THE BACKGROUND IS EMITTED BEFORE ITS CHILDREN, because document order is
     * paint order and a box painted after its text erases it. */
    {
        static const char doc[] =
            "<html><head><style> .z { background: #080808 } </style></head>"
            "<body><div class=\"z\">Z</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int b = find_box(0x080808), z = find_text("Z");
        CHECK(b >= 0 && z >= 0, "background test runs missing (%d %d)", b, z);
        if (b >= 0 && z >= 0)
            CHECK(b < z, "the background box (%d) came AFTER its text (%d)", b, z);
        /* and an unstyled document still emits no boxes at all */
        static const char plain[] = "<html><body><p>nothing here</p></body></html>";
        load(plain, (int)sizeof plain - 1);
        lay_run_doc(600, 16);
        CHECK(count_kind(LR_BOX) == 0, "%d background boxes on an unstyled page",
              count_kind(LR_BOX));
    }

    /* OVERFLOW: HIDDEN CLAMPS THE CHILDREN, and it is asserted here rather
     * than trusted to fb_clip, because a run outside its box is invisible with
     * no explanation - the exact failure indent()'s clamp already exists for. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .clip { height: 20px; overflow: hidden; background: #090909 }"
            "</style></head><body><div class=\"clip\">"
            "one two three four five six seven eight nine ten eleven twelve"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(120, 16);
        int b = find_box(0x090909);
        CHECK(b >= 0, "the clipped block emitted no box");
        if (b >= 0) {
            const struct lay_run *box = lay_at(b);
            int bot = box->y + box->h;
            CHECK(box->h == 20, "a height:20px box is %d tall", box->h);
            int escaped = 0;
            for (int i = 0; i < lay_count(); i++) {
                const struct lay_run *r = lay_at(i);
                if (r->kind == LR_BOX || r->w == 0 || r->h == 0) continue;
                if (r->y + r->h > bot) escaped++;
            }
            CHECK(escaped == 0, "%d runs escaped an overflow:hidden box", escaped);
        }
        /* without it, the same document is taller than its declared height */
        static const char vis[] =
            "<html><head><style>"
            "  .v { height: 20px; background: #0A0A0A }"
            "</style></head><body><div class=\"v\">"
            "one two three four five six seven eight nine ten eleven twelve"
            "</div></body></html>";
        load(vis, (int)sizeof vis - 1);
        lay_run_doc(120, 16);
        int over = 0, vb = find_box(0x0A0A0A);
        if (vb >= 0) {
            int bot = lay_at(vb)->y + lay_at(vb)->h;
            for (int i = 0; i < lay_count(); i++) {
                const struct lay_run *r = lay_at(i);
                if (r->kind == LR_BOX) continue;
                if (r->y + r->h > bot) over++;
            }
        }
        CHECK(over > 0, "overflow:visible clipped anyway - the clip test proves nothing");
    }

    /* WHAT A CHILD CHANGED, A CHILD PUTS BACK. Both of these are the same bug
     * class and both were live: text-align leaking out of a centred paragraph
     * onto its siblings, and a list's indentation leaking onto the block after
     * it. They were caught by mutation testing rather than by reading -
     * "forget to restore one field" produced a suite that stayed green.
     *
     * The whole reason the cursor is split into flowpos and flowenv is to make
     * this unrepresentable, so it gets an assertion that would notice if the
     * split were ever undone. */
    {
        static const char doc[] =
            "<html><head><style> .c { text-align: center } </style></head><body>"
            "<p class=\"c\">centred</p><p>plain</p>"
            "<ul><li>item</li></ul><p>after</p>"
            "</body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        CHECK(tx("centred") > 0, "the centred paragraph did not centre (x=%d)",
              tx("centred"));
        CHECK(tx("plain") == 0,
              "text-align leaked out of its element: the next paragraph is at %d",
              tx("plain"));
        CHECK(tx("item") == 24, "the list item is at %d, wanted the 1.5em indent",
              tx("item"));
        CHECK(tx("after") == 0,
              "a list's indent leaked onto the block after it: x=%d", tx("after"));
    }

    /* 100% inside 100% inside 100% must still be the window, not a third of
     * it and not three times it */
    {
        static const char doc[] =
            "<html><head><style> .p { width: 100%; } .m { background: #0B0B0B }"
            "</style></head><body>"
            "<div class=\"p\"><div class=\"p\"><div class=\"p m\">N</div></div></div>"
            "</body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        int b = find_box(0x0B0B0B);
        CHECK(b >= 0, "nested percentage box missing");
        if (b >= 0) CHECK(lay_at(b)->w == 400, "100%% of 100%% of 100%% = %d, wanted 400",
                          lay_at(b)->w);
        CHECK(tx("N") == 0, "nested percentages moved the content to %d", tx("N"));
        CHECK(worst_overflow(400) == 0, "nested percentages escaped the box");
    }
}

/* ---- images ---------------------------------------------------------------- */
static int img_w = 40, img_h = 20, img_slot = 7, img_off;
static int fake_image(int node, int *w, int *h)
{
    (void)node;
    if (img_off) return -1;
    *w = img_w; *h = img_h;
    return img_slot;
}
static int first_img(void)
{
    for (int i = 0; i < lay_count(); i++) if (lay_at(i)->kind == LR_IMG) return i;
    return -1;
}

static void t_image(void)
{
    printf("images: the hook, the four sizing sources, the aspect ratio\n");
    static const char plain[] = "<html><body><p><img src=\"a.png\"></p></body></html>";

    /* WITH NO HOOK, EXACTLY WHAT IT DID BEFORE. Every existing assertion and
     * both host harnesses depend on that, so it is checked first and by
     * number: em*3/2 square, and img = -1 for "we know its box and not its
     * pixels". */
    lay_set_image(0);
    load(plain, (int)sizeof plain - 1);
    lay_run_doc(600, 16);
    int i = first_img();
    CHECK(i >= 0, "no image run with a NULL hook");
    if (i >= 0) {
        CHECK(lay_at(i)->w == 24 && lay_at(i)->h == 24,
              "NULL-hook placeholder is %dx%d, wanted 24x24", lay_at(i)->w, lay_at(i)->h);
        CHECK(lay_at(i)->img == -1, "NULL hook produced an arena slot %d", lay_at(i)->img);
    }

    /* the hook's intrinsic size, and its slot, reach the run */
    lay_set_image(fake_image);
    img_w = 40; img_h = 20; img_slot = 7; img_off = 0;
    load(plain, (int)sizeof plain - 1);
    lay_run_doc(600, 16);
    i = first_img();
    CHECK(i >= 0 && lay_at(i)->w == 40 && lay_at(i)->h == 20,
          "intrinsic size not used: %dx%d", i < 0 ? -1 : lay_at(i)->w, i < 0 ? -1 : lay_at(i)->h);
    CHECK(i >= 0 && lay_at(i)->img == 7, "the arena slot did not reach the run");

    /* A HOOK THAT SAYS "NOT HERE YET" MUST NOT CHANGE THE ANSWER, because that
     * is what every <img> is between the document arriving and the picture
     * arriving - i.e. what the browser shows for most of a page load. */
    img_off = 1;
    load(plain, (int)sizeof plain - 1);
    lay_run_doc(600, 16);
    i = first_img();
    CHECK(i >= 0 && lay_at(i)->w == 24 && lay_at(i)->img == -1,
          "a hook returning -1 did not fall back to the placeholder");
    img_off = 0;

    /* THE ASPECT RATIO IS THE ONE RULE THAT STOPS EVERY PHOTOGRAPH ON A REAL
     * PAGE BEING A SQUASHED RECTANGLE. 40x20 asked for 100 wide is 50 tall. */
    {
        static const char d1[] = "<html><body><img src=\"a\" width=\"100\"></body></html>";
        load(d1, (int)sizeof d1 - 1);
        lay_run_doc(600, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->w == 100 && lay_at(i)->h == 50,
              "width= alone: %dx%d, wanted 100x50",
              i < 0 ? -1 : lay_at(i)->w, i < 0 ? -1 : lay_at(i)->h);
    }
    {
        static const char d2[] = "<html><body><img src=\"a\" height=\"10\"></body></html>";
        load(d2, (int)sizeof d2 - 1);
        lay_run_doc(600, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->w == 20 && lay_at(i)->h == 10,
              "height= alone: %dx%d, wanted 20x10",
              i < 0 ? -1 : lay_at(i)->w, i < 0 ? -1 : lay_at(i)->h);
    }
    /* both given: the ratio is the author's business, not ours */
    {
        static const char d3[] =
            "<html><body><img src=\"a\" width=\"100\" height=\"7\"></body></html>";
        load(d3, (int)sizeof d3 - 1);
        lay_run_doc(600, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->w == 100 && lay_at(i)->h == 7,
              "width= and height= together were not both honoured");
    }
    /* CSS beats the attribute, which beats the intrinsic size */
    {
        static const char d4[] =
            "<html><body><img src=\"a\" width=\"200\" style=\"width: 80px\">"
            "</body></html>";
        load(d4, (int)sizeof d4 - 1);
        lay_run_doc(600, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->w == 80 && lay_at(i)->h == 40,
              "CSS width did not beat the width= attribute: %dx%d",
              i < 0 ? -1 : lay_at(i)->w, i < 0 ? -1 : lay_at(i)->h);
    }
    /* wider than its container scales DOWN proportionally rather than
     * overflowing: 4000x2000 in a 200px window is 200x100 */
    {
        img_w = 4000; img_h = 2000;
        load(plain, (int)sizeof plain - 1);
        lay_run_doc(200, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->w == 200 && lay_at(i)->h == 100,
              "an oversized image is %dx%d, wanted 200x100",
              i < 0 ? -1 : lay_at(i)->w, i < 0 ? -1 : lay_at(i)->h);
        CHECK(worst_overflow(200) == 0, "an oversized image escaped the box");
    }

    /* A ZERO INTRINSIC DIMENSION COMES FROM A DECODER THAT FAILED HALFWAY and
     * it arrives here as an ordinary number. DO NOT DIVIDE BY IT. */
    {
        img_w = 0; img_h = 0;
        static const char d5[] = "<html><body><img src=\"a\" width=\"100\"></body></html>";
        load(d5, (int)sizeof d5 - 1);
        lay_run_doc(600, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->w == 100 && lay_at(i)->h == 100,
              "a zero intrinsic size did not fall back: %dx%d",
              i < 0 ? -1 : lay_at(i)->w, i < 0 ? -1 : lay_at(i)->h);
        img_w = 40; img_h = 0;
        load(d5, (int)sizeof d5 - 1);
        lay_run_doc(600, 16);
        CHECK(lay_count() > 0, "a zero intrinsic HEIGHT faulted");
        img_w = 0; img_h = 20;
        static const char d6[] = "<html><body><img src=\"a\" height=\"33\"></body></html>";
        load(d6, (int)sizeof d6 - 1);
        lay_run_doc(600, 16);
        i = first_img();
        CHECK(i >= 0 && lay_at(i)->h == 33, "a zero intrinsic WIDTH broke height=");
    }

    img_w = 40; img_h = 20;
    lay_set_image(0);
}

/* ---- flexbox --------------------------------------------------------------- */
static void t_flex(void)
{
    printf("flexbox: grow, shrink, justify, align, wrap, gap\n");

    /* GROW 1/2/1 OVER 600 IS 150/300/150. Nothing about that number can come
     * out right by accident, and swapping grow for shrink makes it 200/200/200
     * because shrink defaults to 1 on every item. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex }"
            "  .a { flex-grow: 1; flex-basis: 0 } .b { flex-grow: 2; flex-basis: 0 }"
            "  .c { flex-grow: 1; flex-basis: 0 }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"a\">A</div><div class=\"b\">B</div><div class=\"c\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("A") == 0,   "grow: A at %d, wanted 0", tx("A"));
        CHECK(tx("B") == 150, "grow: B at %d, wanted 150", tx("B"));
        CHECK(tx("C") == 450, "grow: C at %d, wanted 450", tx("C"));
        /* AND THEY ARE ON ONE ROW, which is the entire point - this document
         * renders as a stacked column without flex */
        CHECK(ty("A") == ty("B") && ty("B") == ty("C"),
              "flex items are not on one row: %d %d %d", ty("A"), ty("B"), ty("C"));
    }

    /* grow summing to 0 leaves every item at its hypothetical size */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex } .i { width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("A") == 0 && tx("B") == 100 && tx("C") == 200,
              "grow 0: %d %d %d, wanted 0 100 200", tx("A"), tx("B"), tx("C"));
    }

    /* SHRINK IS SCALED BY THE BASIS, so 300/300/300 with shrink 1/2/1 losing
     * 300 gives 225/150/225 and not 200/200/200. A shrink factor applied
     * unscaled, or read from `grow` instead, both produce the latter. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex }"
            "  .a { flex-basis: 300px; flex-shrink: 1 }"
            "  .b { flex-basis: 300px; flex-shrink: 2 }"
            "  .c { flex-basis: 300px; flex-shrink: 1 }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"a\">A</div><div class=\"b\">B</div><div class=\"c\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("A") == 0,   "shrink: A at %d, wanted 0", tx("A"));
        CHECK(tx("B") == 225, "shrink: B at %d, wanted 225", tx("B"));
        CHECK(tx("C") == 375, "shrink: C at %d, wanted 375", tx("C"));
    }

    /* AN ITEM WITH NO WIDTH IS AS WIDE AS ITS CONTENT, not as wide as the box
     * it was measured in. Same defect as the grid auto-track case and found
     * the same way: give the item a background and the measuring pass counts
     * the background rectangle, which always spans the whole probe. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex } .n { background: #667788 } .e { width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"n\">ABCD</div><div class=\"e\">E</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("E") == 32,
              "a content-sized flex item measured its background: E at %d, wanted 32",
              tx("E"));
        int b = find_box(0x667788);
        CHECK(b >= 0 && lay_at(b)->w == 32, "the content-sized item is %d wide, wanted 32",
              b < 0 ? -1 : lay_at(b)->w);
    }

    /* JUSTIFY-CONTENT, all six, on three 100px items in 600 (300 of slack) */
    {
        static const char pre[] =
            "<html><head><style>"
            "  .row { display: flex; justify-content: ";
        static const char post[] =
            " } .i { width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        struct { const char *kw; int a, b, c; } want[] = {
            { "flex-start",    0,   100, 200 },
            { "flex-end",      300, 400, 500 },
            { "center",        150, 250, 350 },
            { "space-between", 0,   250, 500 },
            { "space-around",  50,  250, 450 },
            { "space-evenly",  75,  250, 425 },
        };
        for (int k = 0; k < 6; k++) {
            char buf[512];
            int n = 0;
            memcpy(buf + n, pre, sizeof pre - 1); n += (int)sizeof pre - 1;
            int kl = (int)strlen(want[k].kw);
            memcpy(buf + n, want[k].kw, kl); n += kl;
            memcpy(buf + n, post, sizeof post - 1); n += (int)sizeof post - 1;
            load(buf, n);
            lay_run_doc(600, 16);
            CHECK(tx("A") == want[k].a && tx("B") == want[k].b && tx("C") == want[k].c,
                  "justify-content: %s gave %d %d %d, wanted %d %d %d",
                  want[k].kw, tx("A"), tx("B"), tx("C"),
                  want[k].a, want[k].b, want[k].c);
        }
    }

    /* THE GAP COUNT IS THE CLASSIC OFF-BY-ONE: n columns have n-1 gaps. With
     * justify-content: flex-end the last item's right edge lands on the
     * container's right edge exactly when the count is right, and 20px short
     * when it is not. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex; gap: 20px; justify-content: flex-end }"
            "  .i { width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("A") == 260, "gap: A at %d, wanted 260", tx("A"));
        CHECK(tx("B") == 380, "gap: B at %d, wanted 380", tx("B"));
        CHECK(tx("C") == 500, "gap: C at %d, wanted 500 (right edge at 600)", tx("C"));
    }

    /* flex-direction: all four */
    {
        static const char row_rev[] =
            "<html><head><style>"
            "  .row { display: flex; flex-direction: row-reverse } .i { width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(row_rev, (int)sizeof row_rev - 1);
        lay_run_doc(600, 16);
        CHECK(tx("C") == 0 && tx("B") == 100 && tx("A") == 200,
              "row-reverse gave A=%d B=%d C=%d, wanted 200 100 0",
              tx("A"), tx("B"), tx("C"));
    }
    {
        /* A COLUMN'S MAIN SIZE IS ITS HEIGHT, and only an explicit one. 300
         * tall, three items of basis 50 and grow 1: 150 of slack shared three
         * ways is 100 each, so they sit at 0, 100 and 200. */
        static const char col[] =
            "<html><head><style>"
            "  .col { display: flex; flex-direction: column; height: 300px }"
            "  .i { flex-basis: 50px; flex-grow: 1 }"
            "</style></head><body><div class=\"col\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(col, (int)sizeof col - 1);
        lay_run_doc(600, 16);
        CHECK(ty("A") == 0 && ty("B") == 100 && ty("C") == 200,
              "column flex gave y %d %d %d, wanted 0 100 200",
              ty("A"), ty("B"), ty("C"));
        CHECK(tx("A") == 0 && tx("B") == 0, "column flex moved items sideways");
    }
    {
        static const char colr[] =
            "<html><head><style>"
            "  .col { display: flex; flex-direction: column-reverse; height: 300px }"
            "  .i { flex-basis: 100px }"
            "</style></head><body><div class=\"col\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(colr, (int)sizeof colr - 1);
        lay_run_doc(600, 16);
        CHECK(ty("C") == 0 && ty("B") == 100 && ty("A") == 200,
              "column-reverse gave y A=%d B=%d C=%d, wanted 200 100 0",
              ty("A"), ty("B"), ty("C"));
    }

    /* WRAP, and the cross-axis stacking it implies. Three 250px items in 600:
     * two fit on the first line, the third starts a second one at y = 20. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex; flex-wrap: wrap } .i { width: 250px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(ty("A") == 0 && ty("B") == 0, "wrap put the first line at %d/%d",
              ty("A"), ty("B"));
        CHECK(ty("C") == 20, "wrap put the second line at %d, wanted 20", ty("C"));
        CHECK(tx("C") == 0, "the wrapped item is at x=%d, wanted 0", tx("C"));

        static const char nowrap[] =
            "<html><head><style>"
            "  .row { display: flex } .i { width: 250px; flex-shrink: 0 }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(nowrap, (int)sizeof nowrap - 1);
        lay_run_doc(600, 16);
        CHECK(ty("A") == ty("C"), "nowrap wrapped anyway (%d vs %d)", ty("A"), ty("C"));
        CHECK(worst_overflow(600) == 0, "an unshrinkable nowrap row escaped the box");

        static const char wrev[] =
            "<html><head><style>"
            "  .row { display: flex; flex-wrap: wrap-reverse } .i { width: 250px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(wrev, (int)sizeof wrev - 1);
        lay_run_doc(600, 16);
        CHECK(ty("C") == 0 && ty("A") == 20,
              "wrap-reverse did not flip the lines: A=%d C=%d", ty("A"), ty("C"));
    }

    /* ALIGN-ITEMS: STRETCH IS THE INITIAL VALUE AND IT IS WHAT MAKES A ROW OF
     * CARDS LINE UP. The card has one line of text in it; beside a 60px item
     * its BOX must still be 60 tall. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex }"
            "  .tall { height: 60px; width: 100px }"
            "  .card { width: 100px; background: #112233 }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"tall\">T</div><div class=\"card\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int b = find_box(0x112233);
        CHECK(b >= 0, "the card emitted no box");
        if (b >= 0) CHECK(lay_at(b)->h == 60,
                          "align-items: stretch left the card %d tall, wanted 60",
                          lay_at(b)->h);
    }
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex; align-items: center }"
            "  .tall { height: 60px; width: 100px }"
            "  .short { height: 20px; width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"tall\">T</div><div class=\"short\">S</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(ty("T") == 0, "align-items: center moved the tall item to %d", ty("T"));
        CHECK(ty("S") == 20, "align-items: center put the short item at %d, wanted 20",
              ty("S"));

        static const char e[] =
            "<html><head><style>"
            "  .row { display: flex; align-items: flex-end }"
            "  .tall { height: 60px; width: 100px }"
            "  .short { height: 20px; width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"tall\">T</div><div class=\"short\">S</div>"
            "</div></body></html>";
        load(e, (int)sizeof e - 1);
        lay_run_doc(600, 16);
        CHECK(ty("S") == 40, "align-items: flex-end put it at %d, wanted 40", ty("S"));

        /* align-self overrides the container for one item */
        static const char s[] =
            "<html><head><style>"
            "  .row { display: flex; align-items: flex-end }"
            "  .tall { height: 60px; width: 100px }"
            "  .short { height: 20px; width: 100px; align-self: flex-start }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"tall\">T</div><div class=\"short\">S</div>"
            "</div></body></html>";
        load(s, (int)sizeof s - 1);
        lay_run_doc(600, 16);
        CHECK(ty("S") == 0, "align-self: flex-start put it at %d, wanted 0", ty("S"));
    }

    /* row-gap and column-gap are separate axes and must not be swapped */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex; flex-wrap: wrap; row-gap: 30px; column-gap: 8px }"
            "  .i { width: 250px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div><div class=\"i\">C</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("B") == 258, "column-gap: B at %d, wanted 250+8", tx("B"));
        CHECK(ty("C") == 50, "row-gap: second line at %d, wanted 20+30", ty("C"));
    }

    /* THE NAMED TARGET, in the shape of wikipedia.org's portal: a logo beside
     * a list of languages. It renders as one giant stacked column with no
     * flex, and as a row with it. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .portal { display: flex; align-items: center }"
            "  .brand { width: 200px } .langs { flex-grow: 1 }"
            "</style></head><body><div class=\"portal\">"
            "<div class=\"brand\">Wikipedia</div>"
            "<div class=\"langs\">English</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(800, 16);
        CHECK(ty("Wikipedia") == ty("English"),
              "the portal is still stacked: y %d vs %d",
              ty("Wikipedia"), ty("English"));
        CHECK(tx("English") == 200, "the languages column starts at %d, wanted 200",
              tx("English"));
    }
}

static void t_flex_adversarial(void)
{
    printf("flexbox at the boundary\n");

    /* a container narrower than one item's declared minimum */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex; width: 50px }"
            "  .i { flex-basis: 400px; min-width: 200px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        int h = lay_run_doc(600, 16);
        CHECK(h >= 0 && lay_count() > 0, "a too-narrow flex row laid out to nothing");
        CHECK(worst_overflow(600) == 0, "a too-narrow flex row escaped by %d",
              worst_overflow(600));
    }

    /* flex-basis: 0 on every item with no grow: every target is 0 */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex } .i { flex-basis: 0 }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">A</div><div class=\"i\">B</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        CHECK(lay_run_doc(600, 16) >= 0, "flex-basis: 0 everywhere faulted");
        CHECK(worst_overflow(600) == 0, "flex-basis: 0 escaped by %d", worst_overflow(600));
    }

    /* 200 items, then 400 - the second must REFUSE and RECORD rather than
     * scribble past the pool */
    {
        static char buf[64000];
        for (int count = 200; count <= 400; count += 200) {
            int n = 0;
            const char *head =
                "<html><head><style>.row{display:flex;flex-wrap:wrap}.i{width:20px}"
                "</style></head><body><div class=\"row\">";
            int hl = (int)strlen(head);
            memcpy(buf + n, head, hl); n += hl;
            for (int k = 0; k < count; k++) {
                memcpy(buf + n, "<div class=\"i\">x</div>", 22); n += 22;
            }
            memcpy(buf + n, "</div></body></html>", 20); n += 20;
            load(buf, n);
            int h = lay_run_doc(600, 16);
            CHECK(h > 0, "%d flex items laid out to nothing", count);
            CHECK(worst_overflow(600) == 0, "%d flex items escaped by %d",
                  count, worst_overflow(600));
            if (count == 400)
                CHECK(lay_overflowed() > 0,
                      "400 flex items overran the pool without recording it");
        }
    }

    /* NESTED FLEX INSIDE GRID INSIDE FLEX. The whole design rests on a child
     * being laid out at the origin and translated, and that only composes if
     * it composes at depth. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .outer { display: flex } .cell { flex-grow: 1; flex-basis: 0 }"
            "  .g { display: grid; grid-template-columns: 1fr 1fr }"
            "  .inner { display: flex } .p { flex-grow: 1; flex-basis: 0 }"
            "</style></head><body>"
            "<div class=\"outer\">"
            "  <div class=\"cell\"><div class=\"g\">"
            "    <div class=\"inner\"><div class=\"p\">Q</div><div class=\"p\">R</div></div>"
            "    <div>S</div></div></div>"
            "  <div class=\"cell\">T</div>"
            "</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        int h = lay_run_doc(800, 16);
        CHECK(h > 0, "nested flex/grid/flex laid out to nothing");
        /* outer: two 400px cells. cell 1 holds a 2-column grid of 200 each.
         * the inner flex fills column 1, so Q is at 0 and R at 100. */
        CHECK(tx("Q") == 0,   "nested: Q at %d, wanted 0", tx("Q"));
        CHECK(tx("R") == 100, "nested: R at %d, wanted 100", tx("R"));
        CHECK(tx("S") == 200, "nested: S at %d, wanted 200", tx("S"));
        CHECK(tx("T") == 400, "nested: T at %d, wanted 400", tx("T"));
        CHECK(worst_overflow(800) == 0, "nested flex escaped by %d", worst_overflow(800));
    }

    /* MONOTONICITY FOR FLEX, the same claim t_layout makes for text: a
     * narrower window wraps more items and makes a taller document. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .row { display: flex; flex-wrap: wrap } .i { width: 100px }"
            "</style></head><body><div class=\"row\">"
            "<div class=\"i\">a</div><div class=\"i\">b</div><div class=\"i\">c</div>"
            "<div class=\"i\">d</div><div class=\"i\">e</div><div class=\"i\">f</div>"
            "<div class=\"i\">g</div><div class=\"i\">h</div>"
            "</div></body></html>";
        int h[3], w[3] = { 800, 400, 200 };
        for (int k = 0; k < 3; k++) {
            load(doc, (int)sizeof doc - 1);
            h[k] = lay_run_doc(w[k], 16);
            CHECK(worst_overflow(w[k]) == 0, "flex escaped at width %d", w[k]);
        }
        CHECK(h[1] > h[0] && h[2] > h[1],
              "a wrapping flex row is not taller when narrower: %d %d %d",
              h[0], h[1], h[2]);
    }
}

/* ---- grid ------------------------------------------------------------------ */
static void t_grid(void)
{
    printf("grid: px, %%, fr and auto tracks, gaps, implicit rows\n");

    /* 100px + 200px + 1fr in 600 with a 10px gap: 20 of gap, 580 to share,
     * 300 fixed, so the fraction is 280 and the columns start at 0, 110, 320. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 100px 200px 1fr; gap: 10px }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div><div>C</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("A") == 0,   "grid: A at %d, wanted 0", tx("A"));
        CHECK(tx("B") == 110, "grid: B at %d, wanted 110", tx("B"));
        CHECK(tx("C") == 320, "grid: C at %d, wanted 320", tx("C"));
        CHECK(ty("A") == ty("B") && ty("B") == ty("C"),
              "grid row is not one row: %d %d %d", ty("A"), ty("B"), ty("C"));
    }

    /* repeat(3, 1fr) with no gap in 600 is three 200s */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: repeat(3, 1fr) }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div><div>C</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("A") == 0 && tx("B") == 200 && tx("C") == 400,
              "repeat(3,1fr) gave %d %d %d, wanted 0 200 400",
              tx("A"), tx("B"), tx("C"));
    }

    /* unequal fractions: 1fr 3fr over 800 is 200 and 600 */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 1fr 3fr }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(800, 16);
        CHECK(tx("B") == 200, "1fr 3fr put B at %d, wanted 200", tx("B"));
    }

    /* percentage tracks arrive unresolved and are resolved against the
     * container's content width: 25% of 800 is 200 */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 25% 75% }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(800, 16);
        CHECK(tx("B") == 200, "25%% track put B at %d, wanted 200", tx("B"));
    }

    /* an auto track takes its widest item - which is the only sense in which
     * it differs from 1fr for a document. "AAAA" is 4 glyphs of 8px. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: auto auto }"
            "</style></head><body><div class=\"g\">"
            "<div>AAAA</div><div>BB</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("AAAA") == 0, "auto track: A at %d", tx("AAAA"));
        CHECK(tx("BB") == 32, "auto track sized to %d, wanted 32", tx("BB"));
    }

    /* A BLOCK'S OWN BACKGROUND SPANS WHATEVER BOX IT WAS MEASURED IN, so
     * counting it as part of the item's natural width answers "the probe
     * width" every time and makes every auto track as wide as the container.
     * Without the item carrying a background this is invisible - which is
     * exactly what mutation testing found: removing the exclusion in
     * natural_width left the whole suite green. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: auto auto }"
            "  .k { background: #445566 }"
            "</style></head><body><div class=\"g\">"
            "<div class=\"k\">AAAA</div><div>BB</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(tx("BB") == 32,
              "an auto track sized to its item's BACKGROUND, not its text: BB at %d",
              tx("BB"));
        int b = find_box(0x445566);
        CHECK(b >= 0 && lay_at(b)->w == 32,
              "the auto track's box is %d wide, wanted 32",
              b < 0 ? -1 : lay_at(b)->w);
    }

    /* IMPLICIT ROWS: items flow into the columns in order and wrap. Six items
     * in three columns is two rows, and the second one starts below the first
     * plus the row gap. Placing them the other way round - down the columns
     * first - puts D at the top of column 2 instead. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div><div>C</div>"
            "<div>D</div><div>E</div><div>F</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(620, 16);
        /* 620 - 20 of gaps = 600, three 200s at 0, 210, 420 */
        CHECK(tx("A") == 0 && tx("B") == 210 && tx("C") == 420,
              "grid row 1 at %d %d %d", tx("A"), tx("B"), tx("C"));
        CHECK(ty("A") == 0 && ty("B") == 0 && ty("C") == 0, "grid row 1 is not flat");
        CHECK(tx("D") == 0, "D is at %d - items were placed down the columns, not across",
              tx("D"));
        CHECK(ty("D") == 30, "row 2 at y=%d, wanted 20+10", ty("D"));
        CHECK(ty("D") == ty("E") && ty("E") == ty("F"), "grid row 2 is not flat");
    }

    /* a row is as tall as its TALLEST item */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 1fr 1fr }"
            "  .t { height: 70px }"
            "</style></head><body><div class=\"g\">"
            "<div class=\"t\">A</div><div>B</div>"
            "<div>C</div><div>D</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        CHECK(ty("C") == 70, "row 2 at %d - the row did not take its tallest item", ty("C"));
    }

    /* MORE ITEMS THAN TRACKS AND MORE ROWS THAN ITEMS: seven items in two
     * columns is four rows, the last with one item in it */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 1fr 1fr }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div><div>C</div><div>D</div>"
            "<div>E</div><div>F</div><div>G</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        int h = lay_run_doc(600, 16);
        CHECK(ty("G") == 60, "the 7th item is at y=%d, wanted 60 (row 4)", ty("G"));
        CHECK(tx("G") == 0, "the 7th item is at x=%d, wanted column 1", tx("G"));
        CHECK(h == 80, "a 4-row grid is %d tall, wanted 80", h);
    }

    /* MONOTONICITY FOR GRID: narrower columns wrap their text more, so the
     * document gets taller. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: repeat(3, 1fr) }"
            "</style></head><body><div class=\"g\">"
            "<div>the quick brown fox jumps over the lazy dog again and again</div>"
            "<div>the quick brown fox jumps over the lazy dog again and again</div>"
            "<div>the quick brown fox jumps over the lazy dog again and again</div>"
            "</div></body></html>";
        int h[3], w[3] = { 900, 450, 240 };
        for (int k = 0; k < 3; k++) {
            load(doc, (int)sizeof doc - 1);
            h[k] = lay_run_doc(w[k], 16);
            CHECK(worst_overflow(w[k]) == 0, "grid escaped at width %d by %d",
                  w[k], worst_overflow(w[k]));
        }
        CHECK(h[1] > h[0] && h[2] > h[1],
              "a grid is not taller when narrower: %d %d %d", h[0], h[1], h[2]);
    }
}

static void t_grid_adversarial(void)
{
    printf("grid at the boundary\n");

    /* repeat(8, 1fr) in three pixels. Every share rounds to zero and the
     * remainder lands somewhere; what must NOT happen is a division by a zero
     * total, a negative width, or anything outside the box. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: repeat(8, 1fr) }"
            "</style></head><body><div class=\"g\">"
            "<div>a</div><div>b</div><div>c</div><div>d</div>"
            "<div>e</div><div>f</div><div>g</div><div>h</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        int h = lay_run_doc(3, 16);
        CHECK(h >= 0, "repeat(8,1fr) in 3px faulted");
        for (int i = 0; i < lay_count(); i++) {
            const struct lay_run *r = lay_at(i);
            CHECK(r->w >= 0 && r->h >= 0, "run %d is %dx%d", i, r->w, r->h);
            CHECK(r->x >= 0, "run %d at x=%d", i, r->x);
        }
    }

    /* zero fractions: the total is 0 and nothing may be divided by it */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 0fr 0fr }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        CHECK(lay_run_doc(600, 16) >= 0, "0fr 0fr faulted");
    }

    /* tracks that add up to more than the container are scaled down, not
     * overflowed - there is no horizontal scrolling here */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .g { display: grid; grid-template-columns: 400px 400px 400px }"
            "</style></head><body><div class=\"g\">"
            "<div>A</div><div>B</div><div>C</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(300, 16);
        CHECK(worst_overflow(300) == 0, "oversized grid tracks escaped by %d",
              worst_overflow(300));
        CHECK(tx("C") > tx("B") && tx("B") > tx("A"),
              "scaled tracks stopped being three columns: %d %d %d",
              tx("A"), tx("B"), tx("C"));
    }

    /* no template at all: one implicit column, which is a plain stack */
    {
        static const char doc[] =
            "<html><head><style> .g { display: grid } </style></head>"
            "<body><div class=\"g\"><div>A</div><div>B</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        CHECK(lay_run_doc(600, 16) > 0, "a grid with no template laid out to nothing");
        CHECK(ty("B") > ty("A"), "a single-column grid did not stack (%d vs %d)",
              ty("A"), ty("B"));
    }
}

/* ---- floats ---------------------------------------------------------------- */
static void t_float(void)
{
    printf("float and clear\n");

    /* a 100x50 left float: the text beside it starts at x=100, and the first
     * line BELOW it starts at x=0 again */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .f { float: left; width: 100px; height: 50px; background: #FF1010 }"
            "</style></head><body>"
            "<div class=\"f\"></div>"
            "<p>alpha bravo charlie delta echo foxtrot golf hotel india juliet "
            "kilo lima mike november oscar papa quebec romeo sierra tango</p>"
            "</body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        int b = find_box(0xFF1010);
        CHECK(b >= 0, "the float emitted no box");
        if (b >= 0) {
            CHECK(lay_at(b)->x == 0 && lay_at(b)->y == 0,
                  "the left float is at %d,%d", lay_at(b)->x, lay_at(b)->y);
            CHECK(lay_at(b)->w == 100 && lay_at(b)->h == 50,
                  "the float is %dx%d, wanted 100x50", lay_at(b)->w, lay_at(b)->h);
        }
        CHECK(tx("alpha") == 100, "text beside a left float starts at %d, wanted 100",
              tx("alpha"));
        /* every run whose line is above y=50 must clear the float */
        int bad = 0, below = 0;
        for (int i = 0; i < lay_count(); i++) {
            const struct lay_run *r = lay_at(i);
            if (r->kind != LR_TEXT) continue;
            if (r->y < 50 && r->x < 100) bad++;
            if (r->y >= 50 && r->x == 0) below++;
        }
        CHECK(bad == 0, "%d text runs sit underneath the left float", bad);
        CHECK(below > 0, "no line ever came back to the left edge below the float");
        CHECK(worst_overflow(400) == 0, "a float pushed content out of the box");
    }

    /* a right float leaves the left edge alone and shortens the line */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .f { float: right; width: 100px; height: 50px; background: #10FF10 }"
            "</style></head><body>"
            "<div class=\"f\"></div>"
            "<p>alpha bravo charlie delta echo foxtrot golf hotel india juliet</p>"
            "</body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        int b = find_box(0x10FF10);
        CHECK(b >= 0 && lay_at(b)->x == 300, "the right float is at x=%d, wanted 300",
              b < 0 ? -1 : lay_at(b)->x);
        CHECK(tx("alpha") == 0, "text beside a right float starts at %d, wanted 0",
              tx("alpha"));
        int bad = 0;
        for (int i = 0; i < lay_count(); i++) {
            const struct lay_run *r = lay_at(i);
            if (r->kind != LR_TEXT) continue;
            if (r->y < 50 && r->x + r->w > 300) bad++;
        }
        CHECK(bad == 0, "%d text runs sit underneath the right float", bad);
    }

    /* CLEAR moves the block past the float rather than beside it */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .f { float: left; width: 100px; height: 50px; background: #1010FF }"
            "  .c { clear: left }"
            "</style></head><body>"
            "<div class=\"f\"></div><p>beside</p><p class=\"c\">after</p>"
            "</body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        CHECK(tx("beside") == 100, "the uncleared paragraph is at %d", tx("beside"));
        CHECK(ty("after") >= 50, "clear:left put the paragraph at y=%d, wanted >= 50",
              ty("after"));
        CHECK(tx("after") == 0, "the cleared paragraph is at x=%d, wanted 0", tx("after"));
    }

    /* A FLOAT WIDER THAN THE CONTENT BOX is clamped to it. Left alone it takes
     * every line beside it out of the window with it. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .f { float: left; width: 900px; height: 20px; background: #FFFF10 }"
            "</style></head><body><div class=\"f\"></div><p>text here</p></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(400, 16);
        int b = find_box(0xFFFF10);
        CHECK(b >= 0 && lay_at(b)->w <= 400, "an oversized float is %d wide in a 400 box",
              b < 0 ? -1 : lay_at(b)->w);
        CHECK(worst_overflow(400) == 0, "an oversized float escaped by %d",
              worst_overflow(400));
        /* a float as wide as the box leaves no room beside it, so the text
         * goes BELOW it - which is the whole reason line_begin walks down past
         * float bottoms rather than opening a line it cannot fill */
        CHECK(ty("text") >= 20, "text beside a full-width float is at y=%d, wanted >= 20",
              ty("text"));
        CHECK(tx("text") == 0, "text below a full-width float is at x=%d, wanted 0",
              tx("text"));
    }

    /* THE ARRAY FILLS UP. Past its cap it must stop taking floats and record
     * it, not scribble past the end. */
    {
        static char buf[8192];
        int n = 0;
        const char *head =
            "<html><head><style>.f{float:left;width:10px;height:10px;background:#222222}"
            "</style></head><body>";
        int hl = (int)strlen(head);
        memcpy(buf + n, head, hl); n += hl;
        for (int k = 0; k < 40; k++) { memcpy(buf + n, "<div class=\"f\"></div>", 21); n += 21; }
        memcpy(buf + n, "<p>tail</p></body></html>", 25); n += 25;
        load(buf, n);
        int h = lay_run_doc(400, 16);
        CHECK(h > 0, "40 floats laid out to nothing");
        CHECK(lay_overflowed() > 0, "40 floats did not record hitting the cap");
        CHECK(worst_overflow(400) == 0, "40 floats escaped by %d", worst_overflow(400));
        for (int i = 0; i < lay_count(); i++)
            CHECK(lay_at(i)->x >= 0 && lay_at(i)->y >= 0,
                  "float overflow put run %d at %d,%d", i, lay_at(i)->x, lay_at(i)->y);
    }
}

/* ---- positioning ----------------------------------------------------------- */
static void t_position(void)
{
    printf("position: relative, absolute, fixed\n");

    /* relative is a translate of a run range: it moves and the flow does not */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .r { position: relative; left: 30px; top: 5px; background: #A0A0A0 }"
            "  .n { background: #B0B0B0 }"
            "</style></head><body>"
            "<div class=\"r\">R</div><div class=\"n\">N</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int r = find_box(0xA0A0A0), nx = find_box(0xB0B0B0);
        CHECK(r >= 0 && nx >= 0, "relative test boxes missing (%d %d)", r, nx);
        if (r >= 0) {
            CHECK(lay_at(r)->x == 30, "relative left: box x=%d, wanted 30", lay_at(r)->x);
            CHECK(lay_at(r)->y == 5,  "relative top: box y=%d, wanted 5", lay_at(r)->y);
        }
        /* THE FLOW DID NOT MOVE: the next block is still at 20, where the
         * relative one would have ended if it had never been offset */
        if (nx >= 0) CHECK(lay_at(nx)->y == 20,
                           "relative positioning moved the FLOW: next box at %d, wanted 20",
                           lay_at(nx)->y);
    }

    /* absolute with NO positioned ancestor is measured against the document */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .a { position: absolute; left: 50px; top: 60px; width: 40px;"
            "       background: #C0C0C0 }"
            "</style></head><body><p>flow</p><div class=\"a\">A</div>"
            "<p>more</p></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int a = find_box(0xC0C0C0);
        CHECK(a >= 0, "the absolute box is missing");
        if (a >= 0) {
            CHECK(lay_at(a)->x == 50 && lay_at(a)->y == 60,
                  "absolute with no ancestor: %d,%d wanted 50,60",
                  lay_at(a)->x, lay_at(a)->y);
            CHECK(lay_at(a)->w == 40, "absolute width %d, wanted 40", lay_at(a)->w);
        }
        /* IT IS OUT OF FLOW: the paragraph after it is where it would be if
         * the absolute element were not in the document at all */
        static const char without[] =
            "<html><body><p>flow</p><p>more</p></body></html>";
        int with_y = ty("more");
        load(without, (int)sizeof without - 1);
        lay_run_doc(600, 16);
        CHECK(ty("more") == with_y,
              "the absolute element reserved space: %d with, %d without",
              with_y, ty("more"));
    }

    /* against the nearest POSITIONED ancestor, not the document */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .outer { position: relative; padding: 20px; background: #D0D0D0 }"
            "  .inner { position: absolute; left: 10px; top: 5px; width: 30px;"
            "           background: #E0E0E0 }"
            "</style></head><body><p>lead</p>"
            "<div class=\"outer\">O<div class=\"inner\">I</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int o = find_box(0xD0D0D0), in = find_box(0xE0E0E0);
        CHECK(o >= 0 && in >= 0, "positioned-ancestor boxes missing (%d %d)", o, in);
        if (o >= 0 && in >= 0) {
            CHECK(lay_at(in)->x == lay_at(o)->x + 10,
                  "absolute x=%d, ancestor x=%d + 10", lay_at(in)->x, lay_at(o)->x);
            CHECK(lay_at(in)->y == lay_at(o)->y + 5,
                  "absolute y=%d, ancestor y=%d + 5", lay_at(in)->y, lay_at(o)->y);
            CHECK(lay_at(o)->y > 0, "the ancestor should not be at the top of the page");
        }
    }

    /* `right` is measured from the containing block's right edge */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .a { position: absolute; right: 20px; top: 0; width: 40px;"
            "       background: #F0F0F0 }"
            "</style></head><body><div class=\"a\">A</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int a = find_box(0xF0F0F0);
        CHECK(a >= 0 && lay_at(a)->x == 540, "right: 20px put the box at %d, wanted 540",
              a < 0 ? -1 : lay_at(a)->x);
    }

    /* FIXED IS ABSOLUTE AGAINST THE VIEWPORT, which is what this file says it
     * does. Asserted so the claim is checked rather than merely written. */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .outer { position: relative; padding: 40px }"
            "  .f { position: fixed; left: 7px; top: 9px; width: 30px;"
            "       background: #111111 }"
            "</style></head><body><p>lead</p>"
            "<div class=\"outer\"><div class=\"f\">F</div></div></body></html>";
        load(doc, (int)sizeof doc - 1);
        lay_run_doc(600, 16);
        int f = find_box(0x111111);
        CHECK(f >= 0 && lay_at(f)->x == 7 && lay_at(f)->y == 9,
              "fixed is not against the viewport: %d,%d wanted 7,9",
              f < 0 ? -1 : lay_at(f)->x, f < 0 ? -1 : lay_at(f)->y);
    }

    /* absolute must not escape the document even when the document says so */
    {
        static const char doc[] =
            "<html><head><style>"
            "  .a { position: absolute; left: 9000px; top: 5px; width: 40px }"
            "  .b { position: relative; left: -9000px; top: -9000px }"
            "</style></head><body>"
            "<div class=\"a\">A</div><div class=\"b\">B</div></body></html>";
        load(doc, (int)sizeof doc - 1);
        int h = lay_run_doc(300, 16);
        CHECK(h >= 0, "absurd offsets faulted");
        for (int i = 0; i < lay_count(); i++) {
            const struct lay_run *r = lay_at(i);
            CHECK(r->x >= 0 && r->y >= 0, "run %d at %d,%d after an absurd offset",
                  i, r->x, r->y);
            CHECK(r->x <= 300, "run %d starts outside the box at %d", i, r->x);
        }
    }
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
    t_css();
    t_table();
    t_box();
    t_image();
    t_flex();
    t_flex_adversarial();
    t_grid();
    t_grid_adversarial();
    t_float();
    t_position();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
