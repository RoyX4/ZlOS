/* browser.c - the app: a document, a viewport, and the paint.
 *
 * WHAT THIS IS, HONESTLY. It renders HTML. It does not run JavaScript and it
 * will not fetch https - see docs/BROWSER-PROMPT.md §5 for why a half-TLS is
 * worse than none. What is here is a fetcher's worth of plumbing short of a
 * browser, and the parts that ARE here - the parse, the box model, the type -
 * are the parts the rest of this kernel had already almost built.
 *
 * THE SPLIT THAT MATTERS: html.c and layout.c contain no pixels and no theme,
 * so both run in a host harness with no framebuffer. This file is where they
 * meet fb.c, and it is the only file of the three that cannot be tested
 * without a screen. Keeping it thin is the point.
 *
 * NO HEAP, so the document is a fixed buffer and so is everything derived from
 * it. A page larger than DOC_MAX is TRUNCATED at a tag boundary and says so,
 * which is the one behaviour available that is neither a crash nor a lie.
 */

#include "html.h"
#include "layout.h"
#include "ui.h"

/* fb.c - the pixels. See the "rich text" block there for the style bits. */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
int  fb_text_prop_w(const char *s);
int  fb_text_prop_h(void);
int  fb_prop_em(void);
int  fb_text_rich_w(const char *s, int len, int size, int style);
void fb_text_rich(int px, int py, const char *s, int len, unsigned int fg,
                  int size, int style);

/* fb.c's private style encoding. Translated explicitly rather than shared:
 * two layers agreeing on a numeric encoding by coincidence is a bug waiting
 * for someone to insert a bit in one of them. */
#define FBT_BOLD 1
#define FBT_ITAL 2
#define FBT_MONO 4

static int fb_style(int ls)
{
    int s = 0;
    if (ls & LS_BOLD)   s |= FBT_BOLD;
    if (ls & LS_ITALIC) s |= FBT_ITAL;
    if (ls & LS_MONO)   s |= FBT_MONO;
    return s;
}

#define DOC_MAX 24576

static char doc[DOC_MAX];
static int  doc_len;
static int  doc_truncated;

static int  laid_w;          /* the width the current layout was run at */
static int  scroll;
static int  content_h;
static int  view_h;          /* the last client height painted          */
static int  view_x, view_y;  /* ...and where its content started        */
static int  status;          /* BR_* below                              */

#define BR_OK        0
#define BR_NO_NET    1       /* an http:// URL with no network driver    */
#define BR_NO_TLS    2       /* https:// - refused on purpose            */

/* ---- the measure ----------------------------------------------------------
 * The one thing layout.c cannot know. Everything else about the type - the
 * atlas, the gamma-correct blend, the synthesised bold - stays in fb.c. */
static int measure(const char *s, int len, int size, int style)
{
    return fb_text_rich_w(s, len, size, fb_style(style));
}

/* ---- the page that ships in the kernel -------------------------------------
 * Not a placeholder: this is the gate document. It uses every element the
 * parser claims to support, so "the browser renders" and "the browser renders
 * what it says it does" are the same observation. It also states the two
 * things the browser cannot do, on the screen, where a user will see them -
 * the same standard §5 sets for the padlock and SYSTEM-PROMPT.md sets for the
 * `net up` label.
 */
static const char home_page[] =
"<html><head><title>zlOS</title></head><body>\n"
"<h1>A browser, in zlOS</h1>\n"
"<p>This document was <strong>parsed</strong> and <em>laid out</em> by "
"<code>html.c</code> and <code>layout.c</code>, and drawn by the same "
"text renderer the rest of the desktop uses. There is no heap under any "
"of it: the tree is an array and the edges are indices.</p>\n"
"<hr>\n"
"<h2>What works</h2>\n"
"<ul>\n"
"<li>headings <code>h1</code> to <code>h6</code>, with a real type scale</li>\n"
"<li>paragraphs that <strong>reflow</strong> when the window is resized</li>\n"
"<li><em>emphasis</em>, <strong>strong</strong>, <code>monospace</code></li>\n"
"<li>ordered and unordered lists, nested</li>\n"
"<li>links, entities (&amp; &lt; &gt;), and malformed markup</li>\n"
"</ul>\n"
"<h2>What does not</h2>\n"
"<ol>\n"
"<li><strong>The network.</strong> There is no driver yet, so nothing can "
"be fetched. The header's <code>net up</code> is decorative and always was.</li>\n"
"<li><strong>HTTPS.</strong> Refused, deliberately. There is no cipher in "
"this kernel - only hashes - and a padlock that has not been earned is worse "
"than no padlock at all.</li>\n"
"<li><strong>JavaScript.</strong> An engine is its own multi-year project, "
"not a hard afternoon.</li>\n"
"</ol>\n"
"<h3>Try it</h3>\n"
"<p>Press <code>[</code> and <code>]</code> to narrow and widen this window. "
"The text reflows: the line breaks are computed, not drawn. That is the "
"difference between a layout engine and a picture of one.</p>\n"
"<pre>  html.c    the tokenizer and the tree\n"
"  layout.c  block and inline boxes\n"
"  browser.c this window</pre>\n"
"<p>A <a href=\"http://example.com/\">link</a> is drawn in the accent colour "
"and underlined. Following one needs the network.</p>\n"
"</body></html>\n";

/* ---- loading --------------------------------------------------------------- */
static void doc_set(const char *src, int len)
{
    doc_truncated = 0;
    if (len > DOC_MAX - 1) { len = DOC_MAX - 1; doc_truncated = 1; }
    for (int i = 0; i < len; i++) doc[i] = src[i];
    doc[len] = 0;
    doc_len = len;
    html_parse(doc, doc_len);
    lay_set_measure(measure);
    laid_w = 0;                       /* force a layout on the next paint */
    scroll = 0;
    status = BR_OK;
}

void browser_home(void)
{
    doc_set(home_page, (int)(sizeof home_page - 1));
}

/* Load from anywhere in memory - the RAM filesystem lives at a fixed address
 * and hands out (address, length), so this is all the coupling needed.
 *
 * The uptr typedef is the same one nvme.c, xhci.c and virtio_gpu.c use, and it
 * exists for the same reason: `unsigned long` is 8 bytes in the 64-bit build
 * and 4 in the UEFI one, which is LLP64. buildefi.sh makes an int-to-pointer
 * cast a hard error precisely because five pointer truncations once sat in the
 * boot path unseen. */
#ifdef ZL_64
typedef unsigned long long uptr;
#else
typedef unsigned int       uptr;
#endif

void browser_load_mem(unsigned int addr, int len)
{
    if (!addr || len <= 0) { browser_home(); return; }
    doc_set((const char *)(uptr)addr, len);
}

int browser_truncated(void) { return doc_truncated; }
int browser_status(void)    { return status; }

/* A URL, one character at a time, with no network under it. Both refusals are
 * deliberate and both SAY SO rather than failing silently. */
void browser_go(const char *url, int len)
{
    int https = len >= 8;
    for (int i = 0; i < 8 && https; i++)
        if (url[i] != "https://"[i]) https = 0;
    status = https ? BR_NO_TLS : BR_NO_NET;
}

/* ---- the layout ------------------------------------------------------------ */
static void relayout(int w)
{
    if (w == laid_w) return;
    laid_w = w;
    content_h = lay_run_doc(w, fb_prop_em());
    if (scroll > content_h - view_h) scroll = content_h - view_h;
    if (scroll < 0) scroll = 0;
}

int browser_height(void) { return content_h; }
int browser_scroll(void) { return scroll; }
int browser_lines(void)  { return lay_lines(); }
int browser_runs(void)   { return lay_count(); }

int browser_scroll_by(int d)
{
    int was = scroll;
    scroll += d;
    int max = content_h - view_h;
    if (max < 0) max = 0;
    if (scroll > max) scroll = max;
    if (scroll < 0) scroll = 0;
    return scroll != was;
}

/* ---- the paint --------------------------------------------------------------
 * POSITION-PURE, as the app contract requires: every coordinate below is
 * derived from the x,y,w,h passed in. The document's own coordinates are
 * content-relative and the origin is added here, once.
 */
#define PAD_EM 3                       /* page margin, in eighths of an em */

static const char *status_text(void)
{
    if (status == BR_NO_TLS)
        return "https is refused: this kernel has hashes but no cipher";
    if (status == BR_NO_NET)
        return "no network driver: nothing can be fetched yet";
    return 0;
}

void browser_draw(int x, int y, int w, int h, int focused)
{
    /* SELF-INITIALISING, and that is not laziness. The compositor used to call
     * browser_home() when it opened the window, which meant opening the
     * desktop AFTER fetching a page silently replaced the fetched page with
     * the built-in one - the fetch had worked, the screenshot showed the home
     * page, and nothing said why. An app that can be handed a document before
     * its window exists must not have its content decided by window order. */
    if (doc_len == 0) browser_home();

    const struct ui_theme *t = ui_theme();
    int em = fb_prop_em();
    int pad = em * PAD_EM / 8;

    fb_fill_px(x, y, w, h, t->panel);

    /* the chrome: one status strip, which is where the two refusals live */
    int bar = 0;
    const char *msg = status_text();
    if (msg) {
        bar = fb_text_prop_h() + em / 2;
        fb_fill_px(x, y, w, bar, t->panel_hi);
        fb_fill_px(x, y + bar - 1, w, 1, t->border);
        fb_text_prop(x + pad, y + em / 4, msg, t->danger);
    }
    if (doc_truncated) {
        int b2 = fb_text_prop_h() + em / 2;
        fb_fill_px(x, y + bar, w, b2, t->panel_hi);
        fb_text_prop(x + pad, y + bar + em / 4,
                     "document truncated: larger than this kernel's buffer",
                     t->text_dim);
        bar += b2;
    }

    int cx = x + pad, cy = y + bar + pad;
    int cw = w - 2 * pad, ch = h - bar - 2 * pad;
    if (cw < em) cw = em;
    if (ch < 1) ch = 1;
    view_h = ch;
    view_x = cx;
    view_y = cy;
    relayout(cw);

    /* The scissor is a correctness guarantee, not a substitute for not
     * drawing: runs outside the viewport are rejected below as well, because a
     * long document is thousands of runs and only a screenful is ever visible.
     * ui.c's scrolled list makes exactly the same argument. */
    fb_clip(cx, cy, cw, ch);

    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        int ry = cy + r->y - scroll;
        if (ry + r->h < cy || ry > cy + ch) continue;
        int rx = cx + r->x;

        unsigned int col = t->text;
        if (r->color == LC_DIM)    col = t->text_dim;
        if (r->color == LC_ACCENT) col = t->accent;

        if (r->kind == LR_RULE) {
            fb_fill_px(rx, ry, r->w, r->h > 0 ? r->h : 1, t->border);
            continue;
        }
        if (r->kind == LR_BULLET) {
            fb_rrect(rx, ry, r->w, r->h, r->w / 2, col);
            continue;
        }
        if (r->kind == LR_IMG) {
            /* no decoder: an honest empty frame, not a broken picture */
            fb_box(rx, ry, r->w, r->h, t->border);
            fb_fill_px(rx + r->w / 4, ry + r->h / 2, r->w / 2, 1, t->text_dim);
            continue;
        }

        /* the glyph cell is `size` tall inside a line box that is 5/4 of it;
         * bottom-aligning the cell puts the leading above, where it belongs */
        int gy = ry + (r->h - r->size);
        fb_text_rich(rx, gy, r->text, r->len, col, r->size, fb_style(r->style));
        if (r->style & LS_UNDER) {
            int uy = gy + r->size - r->size / 8;
            fb_fill_px(rx, uy, r->w, r->size >= 24 ? 2 : 1, col);
        }
    }

    fb_clip_none();

    /* the scrollbar, only when there is something to scroll - the same rule
     * ui_scroll_end uses, and for the same reason */
    if (content_h > ch) {
        int bw = UI_S1(t);
        int bx = x + w - bw;
        int th = ch * ch / content_h;
        if (th < UI_S6(t)) th = UI_S6(t);
        int ty = cy + (ch - th) * scroll / (content_h - ch);
        fb_fill_px(bx, cy, bw, ch, t->panel_hi);
        fb_rrect(bx, ty, bw, th, bw / 2, focused ? t->accent : t->text_dim);
    }
}

/* ---- links -----------------------------------------------------------------
 * Which link is under this point? The run array already carries the enclosing
 * <a> for every run, so a hit test is a walk over the runs rather than a
 * second structure that could disagree with the first. */
int browser_link_at(int cx, int cy)
{
    /* The content origin comes from the LAST PAINT, not from a second
     * calculation off the window rect - the status strip moves it down by a
     * variable amount and two copies of that arithmetic would drift the moment
     * one of them gained a line. The caller does not pass the rectangle at all,
     * so there is no second copy to drift. Same argument as ui.c's single
     * place() function. */
    int ox = view_x, oy = view_y;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r->link < 0) continue;
        int rx = ox + r->x, ry = oy + r->y - scroll;
        if (cx >= rx && cx < rx + r->w && cy >= ry && cy < ry + r->h)
            return r->link;
    }
    return -1;
}

/* Clicking a link cannot fetch anything yet, so it does the only honest thing:
 * it says which of the two reasons applies. That is the same message the URL
 * bar will give in item 7, from the same function. */
int browser_click(int cx, int cy)
{
    int n = browser_link_at(cx, cy);
    if (n < 0) return 0;
    int len;
    const char *href = html_href(n, &len);
    browser_go(href, len);
    return 1;
}

/* ---- keys ------------------------------------------------------------------
 * Returns 1 when something changed and the window needs repainting. */
#define KEY_UP    0x112
#define KEY_DOWN  0x113
#define KEY_HOME  0x114
#define KEY_END   0x115
#define KEY_PGUP  0x116
#define KEY_PGDN  0x117

int browser_key(int code)
{
    int line = fb_prop_em() * 5 / 4;
    switch (code) {
    case KEY_DOWN: return browser_scroll_by(line);
    case KEY_UP:   return browser_scroll_by(-line);
    case KEY_PGDN: return browser_scroll_by(view_h - line);
    case KEY_PGUP: return browser_scroll_by(-(view_h - line));
    case KEY_HOME: return browser_scroll_by(-content_h);
    case KEY_END:  return browser_scroll_by(content_h);
    case ' ':      return browser_scroll_by(view_h - line);
    default:       return 0;
    }
}

/* The title from <title>, as a nul-terminated string the window frame can use.
 * html.c hands out lengths rather than C strings - the arena holds no
 * terminators - so the copy happens here, once, into a fixed buffer. */
static char title_buf[64];

const char *browser_title(void)
{
    int len;
    const char *s = html_title(&len);
    if (len > (int)sizeof title_buf - 1) len = (int)sizeof title_buf - 1;
    for (int i = 0; i < len; i++) title_buf[i] = s[i];
    title_buf[len] = 0;
    return len ? title_buf : "untitled";
}
