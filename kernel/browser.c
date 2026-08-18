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
#include "http.h"
#include "tcp.h"

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
#define BR_NO_DNS    3       /* a name, and there is no resolver         */
#define BR_FETCHING  4
#define BR_FAILED    5
#define BR_BAD_TYPE  6

/* ---- the URL bar and the history -------------------------------------------
 * The line editing is term.c's SHAPE, not term.c's code: one character per
 * call, no loop, state in a fixed buffer. term.c belongs to the platform track
 * and a browser's URL bar is not a shell prompt - it has no scrollback, no
 * command table and no echo. What is shared is the discipline.
 *
 * HISTORY IS AN ARRAY OF FIXED STRINGS. There is no heap, so Back is an index
 * into eight slots and the ninth push drops the oldest. That is a real limit
 * and it is the one this kernel's constraints produce.
 */
#define URL_MAX  128
#define HIST_N   8

static char url[URL_MAX];
static int  url_len;
static int  url_focus;
static int  url_sel_all;   /* focus selects the lot, as a browser does */
static char hist[HIST_N][URL_MAX];
static int  hist_n;
static int  fetching;
static int  last_status_code;

static void sset(char *d, const char *s, int max)
{
    int i = 0;
    while (s[i] && i < max - 1) { d[i] = s[i]; i++; }
    d[i] = 0;
}

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

static void hist_push(const char *u);
static const char HOME_URL[];

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

/* about:home is a REAL history entry, not a special case bolted on. Without
 * it the first navigation is the bottom of the stack and Back can never return
 * to where the browser started - which is not "Back is disabled", it is "Back
 * silently does nothing", and those look identical on screen. */
static const char HOME_URL[] = "about:home";

void browser_home(void)
{
    doc_set(home_page, (int)(sizeof home_page - 1));
    if (hist_n == 0) hist_push(HOME_URL);
    sset(url, HOME_URL, URL_MAX);
    url_len = 10;
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

/* ---- URLs ------------------------------------------------------------------
 * scheme://host[:port]/path, and the host must be a dotted quad.
 *
 * THERE IS NO RESOLVER. A name needs DNS, which is a UDP client, a query
 * encoder, a response parser and a cache - a track of its own, and one that is
 * not in this brief. So a hostname is REFUSED BY NAME rather than silently
 * failing to connect, which is the same standard §5 sets for the padlock.
 */
static int is_digit(char c) { return c >= '0' && c <= '9'; }

static int parse_quad(const char *s, int len, unsigned *out)
{
    unsigned v = 0;
    int part = 0, digits = 0, i = 0;
    unsigned acc = 0;
    for (; i <= len; i++) {
        if (i < len && is_digit(s[i])) {
            acc = acc * 10 + (unsigned)(s[i] - '0');
            if (acc > 255) return 0;
            digits++;
        } else if (i == len || s[i] == '.') {
            if (!digits) return 0;
            v = (v << 8) | acc;
            acc = 0; digits = 0; part++;
            if (i == len) break;
        } else {
            return 0;
        }
    }
    if (part != 4) return 0;
    *out = v;
    return 1;
}

static char req_host[URL_MAX], req_path[URL_MAX];
static unsigned req_ip;
static int req_port;

static int parse_url(const char *u, int len)
{
    int i = 0;
    if (len >= 8) {
        int https = 1;
        for (int k = 0; k < 8; k++) if (u[k] != "https://"[k]) { https = 0; break; }
        if (https) { status = BR_NO_TLS; return 0; }
    }
    if (len >= 7) {
        int http = 1;
        for (int k = 0; k < 7; k++) if (u[k] != "http://"[k]) { http = 0; break; }
        if (http) i = 7;
    }
    int hs = i;
    while (i < len && u[i] != '/' && u[i] != ':') i++;
    int he = i;
    req_port = 80;
    if (i < len && u[i] == ':') {
        i++;
        int p = 0;
        while (i < len && is_digit(u[i])) p = p * 10 + (u[i++] - '0');
        if (p > 0 && p < 65536) req_port = p;
    }
    int ps = i;
    if (he <= hs) { status = BR_FAILED; return 0; }
    if (!parse_quad(u + hs, he - hs, &req_ip)) { status = BR_NO_DNS; return 0; }

    int n = 0;
    for (int k = hs; k < he && n < URL_MAX - 1; k++) req_host[n++] = u[k];
    req_host[n] = 0;
    n = 0;
    if (ps >= len) req_path[n++] = '/';
    else for (int k = ps; k < len && n < URL_MAX - 1; k++) req_path[n++] = u[k];
    req_path[n] = 0;
    return 1;
}

static void hist_push(const char *u)
{
    if (hist_n >= HIST_N) {
        for (int i = 0; i + 1 < HIST_N; i++) sset(hist[i], hist[i + 1], URL_MAX);
        hist_n = HIST_N - 1;
    }
    sset(hist[hist_n++], u, URL_MAX);
}

/* Navigate. Every refusal SAYS WHICH ONE it is - "https is refused", "no
 * resolver", "no network driver" - because a browser that just says "could not
 * load" makes the user guess which of three different things went wrong. */
static int navigate(const char *u, int len, int record)
{
    status = BR_OK;
    if (len == 10) {
        int home = 1;
        for (int k = 0; k < 10; k++) if (u[k] != HOME_URL[k]) { home = 0; break; }
        if (home) {
            doc_set(home_page, (int)(sizeof home_page - 1));
            sset(url, HOME_URL, URL_MAX);
            url_len = 10;
            if (record) hist_push(HOME_URL);
            return 1;
        }
    }
    if (!parse_url(u, len)) return 0;
    if (!net_live()) { status = BR_NO_NET; return 0; }

    tcp_attach(net_send_ip, net_ip());
    net_set_ip_sink(tcp_input);
    http_reset();
    if (!http_start(req_ip, req_port, req_host, req_path)) {
        status = BR_FAILED;
        return 0;
    }
    if (record) hist_push(u);
    fetching = 1;
    status = BR_FETCHING;
    return 1;
}

void browser_go(const char *u, int len) { navigate(u, len, 1); }

/* Called every frame. Returns 1 when something changed and the window needs
 * repainting - the app contract's tick, which must be cheap and must not draw. */
int browser_tick(void)
{
    if (!fetching) return 0;
    for (int i = 0; i < 64; i++) net_poll_once();
    tcp_tick();
    int s = http_poll();
    if (s == HTTP_DONE) {
        fetching = 0;
        last_status_code = http_status();
        doc_set((const char *)(uptr)http_body_addr(), http_body_len());
        status = BR_OK;
        return 1;
    }
    if (s == HTTP_REFUSED) { fetching = 0; status = BR_BAD_TYPE; return 1; }
    if (s == HTTP_ERROR)   { fetching = 0; status = BR_FAILED;   return 1; }
    return 0;
}

int browser_back(void)
{
    if (hist_n < 2) return 0;
    hist_n--;                                   /* drop where we are */
    char *prev = hist[hist_n - 1];
    int n = 0; while (prev[n]) n++;
    sset(url, prev, URL_MAX);
    url_len = n;
    return navigate(prev, n, 0);
}

int browser_can_back(void) { return hist_n >= 2; }

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

/* Every refusal says WHICH one it is. A browser that only says "could not
 * load" makes the user guess between four different causes. */
static const char *status_text(void)
{
    switch (status) {
    case BR_NO_TLS:   return "https is refused: this kernel has hashes but no cipher";
    case BR_NO_NET:   return "the network is not up - run the network gate first";
    case BR_NO_DNS:   return "no resolver: use an address, not a name (there is no DNS)";
    case BR_FETCHING: return "fetching...";
    case BR_FAILED:   return "the fetch failed - is anything listening there?";
    case BR_BAD_TYPE: return "refused: not text/html or text/plain";
    default:          return 0;
    }
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
    int pad = fb_prop_em() * PAD_EM / 8;

    fb_fill_px(x, y, w, h, t->panel);

    /* ---- the chrome: Back, the URL bar, then the status strip -------------
     * Drawn from the top down, each one advancing `bar`, so the document's
     * client area is whatever is left. Nothing below computes a second copy of
     * that offset - browser_link_at reads it back from here. */
    int em = fb_prop_em();
    int rowh = fb_text_prop_h() + em / 2;
    int backw = fb_text_prop_w(" Back ") + em / 2;

    /* Back is drawn disabled when there is nowhere to go, rather than hidden -
     * a control that vanishes makes the layout jump. */
    unsigned bfg = browser_can_back() ? t->text : t->text_dim;
    fb_rrect(x + em / 4, y + em / 4, backw, rowh, UI_S1(t), t->panel_hi);
    fb_text_prop(x + em / 4 + em / 4, y + em / 4 + em / 8, " Back ", bfg);

    int ux = x + em / 4 + backw + em / 4;
    int uw = w - (ux - x) - em / 4;
    fb_rrect(ux, y + em / 4, uw, rowh, UI_S1(t),
             url_focus ? t->panel_hi : t->bg);
    if (url_focus) fb_fill_px(ux, y + em / 4 + rowh - 2, uw, 2, t->accent);
    url[url_len] = 0;
    fb_clip(ux + em / 4, y + em / 4, uw - em / 2, rowh);
    if (url_len) {
        if (url_focus && url_sel_all)
            fb_fill_px(ux + em / 4, y + em / 4 + em / 8,
                       fb_text_prop_w(url), fb_text_prop_h(), t->title);
        fb_text_prop(ux + em / 4, y + em / 4 + em / 8, url, t->text);
    }
    else
        fb_text_prop(ux + em / 4, y + em / 4 + em / 8,
                     "press l, then type http://10.0.2.2:8000/", t->text_dim);
    if (url_focus) {
        int cx0 = ux + em / 4 + fb_text_prop_w(url);
        fb_fill_px(cx0, y + em / 4 + em / 8, 2, fb_text_prop_h(), t->accent);
    }
    fb_clip_none();

    int bar = rowh + em / 2;
    const char *msg = status_text();
    if (msg) {
        int b2 = fb_text_prop_h() + em / 2;
        unsigned col = (status == BR_FETCHING) ? t->accent : t->danger;
        fb_fill_px(x, y + bar, w, b2, t->panel_hi);
        fb_fill_px(x, y + bar + b2 - 1, w, 1, t->border);
        fb_text_prop(x + pad, y + bar + em / 4, msg, col);
        bar += b2;
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

/* ONE CHARACTER PER CALL, NO LOOP - term.c's shape, and the whole inversion
 * this desktop exists to have. A URL bar that read a line would block, and a
 * blocking read is why every demo in this kernel used to end in "press any key
 * to exit". */
int browser_key(int code)
{
    int line = fb_prop_em() * 5 / 4;

    if (url_focus) {
        /* FOCUS SELECTS ALL, and the first keystroke replaces it. Without this
         * the bar is pre-filled with the current address and typing APPENDS to
         * it - "about:homehttp://10.0.2.2:8000/" - which then fails to parse
         * and reports "no resolver", a message about the wrong thing entirely.
         * Found by looking at the screenshot, not the log. */
        if (url_sel_all && ((code >= 32 && code < 127) || code == 8 || code == 127)) {
            url_len = 0;
            url[0] = 0;
            url_sel_all = 0;
            if (code == 8 || code == 127) return 1;
        }
        if (code == 13 || code == 10) {          /* Enter: go */
            url[url_len] = 0;
            url_focus = 0;
            url_sel_all = 0;
            navigate(url, url_len, 1);
            return 1;
        }
        if (code == 27) { url_focus = 0; url_sel_all = 0; return 1; }   /* Esc */
        if (code == 8 || code == 127) {                     /* Backspace */
            if (url_len > 0) url_len--;
            return 1;
        }
        if (code >= 32 && code < 127 && url_len < URL_MAX - 2) {
            url[url_len++] = (char)code;
            return 1;
        }
        return 0;
    }

    switch (code) {
    case KEY_DOWN: return browser_scroll_by(line);
    case KEY_UP:   return browser_scroll_by(-line);
    case KEY_PGDN: return browser_scroll_by(view_h - line);
    case KEY_PGUP: return browser_scroll_by(-(view_h - line));
    case KEY_HOME: return browser_scroll_by(-content_h);
    case KEY_END:  return browser_scroll_by(content_h);
    case ' ':      return browser_scroll_by(view_h - line);
    case 'l': case 'L': url_focus = 1; url_sel_all = 1; return 1;
    case 8:   case 127: return browser_back();
    default:  return 0;
    }
}

int browser_url_focus(void) { return url_focus; }

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
