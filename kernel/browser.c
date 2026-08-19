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
#include "css.h"
#include "png.h"
#include "memmap.h"
#include "js.h"
#include "ui.h"
#include "http.h"
#include "tls.h"
int http_rnd_quality(void);
#include "tcp.h"
#include "dns.h"

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

/* fb.c - the pixels. See the "rich text" block there for the style bits. */
void fb_fill_px(int x, int y, int w, int h, unsigned int rgb);
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb);
void fb_box(int x, int y, int w, int h, unsigned int rgb);
void fb_clip(int x, int y, int w, int h);
void fb_clip_none(void);
void fb_text_prop(int px, int py, const char *s, unsigned int fg);
void fb_image(int px, int py, int w, int h,
              const unsigned int *src, int sw, int sh);
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

/* A REAL PAGE, NOT A DEMO PAGE. 24 KB was sized for the hand-written home
 * page, then 256 KB for a real one. Measured against the English Wikipedia
 * article on Linux - 982,395 bytes as served today, 8,239 open tags, 15,806
 * words, 16 <style> blocks - 256 KB kept 27% of it and truncated the rest at a
 * tag boundary, honestly and visibly.
 *
 * IT IS NOT BSS ANY MORE, so the sentence that used to be here - "the binding
 * limit is raw_entry.S's stack at 6 MiB, not the 128 MiB of free space above
 * it" - has stopped being true, which is exactly why it is quoted rather than
 * deleted. The document now lives in the free space above, with the tree, the
 * stylesheet and the runs. 2 MiB holds that article twice. */
#define DOC_MAX 0x200000

/* Exposed for the harness. browsertest hard-coded an 80 KB document against a
 * 24 KB cap; raising the cap turned "is a huge page truncated AND flagged"
 * into a test that quietly checked nothing. Same lesson as HTML_MAX_NODES. */
int browser_doc_cap(void) { return DOC_MAX; }

/* ---- THE ONE REGION, CARVED ONCE ------------------------------------------
 * memmap.h owns the base and the span; this is the only place the six
 * sub-arenas inside it are laid out, and every offset is a running sum of the
 * budgets the four headers state. ONE region rather than six bases in
 * memmap.h, because six bases is six subtractions to get wrong and memmap.h's
 * header is a list of people who got a subtraction wrong.
 *
 * ALIGNMENT IS CHECKED, NOT REASONED ABOUT. Every budget below happens to be a
 * multiple of 8 today; the day one is not, struct lay_run's pointer member
 * lands unaligned on the 64-bit build and the fault is nowhere near here. */
#define BR_OFF_DOC    0L
#define BR_OFF_NODES  (BR_OFF_DOC   + DOC_MAX)
#define BR_OFF_HARENA (BR_OFF_NODES + HTML_NODES_BYTES)
#define BR_OFF_SELS   (BR_OFF_HARENA + HTML_ARENA)
#define BR_OFF_DECLS  (BR_OFF_SELS  + CSS_SELS_BYTES)
#define BR_OFF_CARENA (BR_OFF_DECLS + CSS_DECLS_BYTES)
#define BR_OFF_RUNS   (BR_OFF_CARENA + CSS_ARENA)
#define BR_STORAGE_BYTES (BR_OFF_RUNS + LAY_RUNS_BYTES)

_Static_assert(BR_OFF_NODES  % 8 == 0, "html.c's node array lands unaligned");
_Static_assert(BR_OFF_HARENA % 8 == 0, "html.c's text arena lands unaligned");
_Static_assert(BR_OFF_SELS   % 8 == 0, "css.c's selector array lands unaligned");
_Static_assert(BR_OFF_DECLS  % 8 == 0, "css.c's decl array lands unaligned");
_Static_assert(BR_OFF_CARENA % 8 == 0, "css.c's string arena lands unaligned");
_Static_assert(BR_OFF_RUNS   % 8 == 0,
               "layout.c's run array lands unaligned - struct lay_run holds a "
               "pointer and the 64-bit build needs it 8-aligned");
/* THE SUBTRACTION THAT MATTERS, and it is the one memmap.h's header asks every
 * owning file to write down: does the highest byte land under the next base? */
_Static_assert(BR_STORAGE_BYTES <= (long)(HI_DOM_END - HI_DOM),
               "the browser's storage no longer fits its region in memmap.h - "
               "raise HI_DOM_END and check it against HI_BACK, do not shave a "
               "cap to make this line pass");

static char * const doc = (char *)(uptr)(HI_DOM + BR_OFF_DOC);
static int  doc_len;
static int  doc_truncated;

/* what the page's scripts produced, and why one stopped if it did */
#define JS_OUT_MAX 4096
static char js_out[JS_OUT_MAX];
static int  js_out_len;
static char js_err[128];

static int  laid_w;          /* the width the current layout was run at */
/* A PICTURE ARRIVING CHANGES THE LAYOUT AT AN UNCHANGED WIDTH, which the old
 * width-only test in relayout() could not express: it returned immediately
 * whenever the window had not been resized, so a decoded image would have kept
 * the placeholder's box until the user dragged the frame. Anything that
 * invalidates the boxes without changing the width sets this. */
static int  lay_dirty;
static int  scroll;
static int  content_h;
static int  view_h;          /* the last client height painted          */
static int  view_x, view_y;  /* ...and where its content started        */
static int  status;          /* BR_* below                              */

/* WHERE THE CHROME LANDED AT THE LAST PAINT. Recorded for exactly the reason
 * browser_link_at records view_x/view_y: the status strip and the truncation
 * banner move everything below them by a variable amount, so a hit test that
 * recomputed the rectangles would drift the moment one of those gained a line.
 * One place computes them; everything else reads them back. */
static int bar_x, bar_y, bar_w, bar_h;      /* the URL bar     */
static int back_x, back_y, back_w, back_h;  /* the Back button */

#define BR_OK        0
#define BR_NO_NET    1       /* an http:// URL with no network driver    */
#define BR_NO_TLS    2       /* an https:// fetch failed - see the reason */
#define BR_NO_DNS    3       /* a name, and there is no resolver         */
#define BR_FETCHING  4
#define BR_FAILED    5
#define BR_BAD_TYPE  6
#define BR_RESOLVING 7
#define BR_IMAGES    8       /* the page is up; its pictures are arriving */

/* ---- searching from the URL bar --------------------------------------------
 * TYPING WORDS INSTEAD OF AN ADDRESS SEARCHES, which is what every browser
 * does and the last thing between this one and being usable by hand.
 *
 * WHY NOT GOOGLE, given that Google demonstrably works. It does: the handshake
 * verifies to GTS Root R1, `https://www.google.com/search?q=...` returns HTTP
 * 200, and 71 KB of HTML arrives. **The HTML contains no results.** Measured
 * through this exact stack: 71,272 bytes parse to NINETEEN nodes and 151
 * characters of text, and the text is
 *
 *     "Please click /httpservice/retry/enablejs ... here if you are not
 *      redirected within a few seconds."
 *
 * - Google's <noscript> fallback. The results are assembled by JavaScript, so
 * there is no document to render. That is not a gap in this browser and it is
 * not something a bigger parser fixes.
 *
 * AND IT IS NOT A USER-AGENT PROBLEM, which was the obvious next guess and is
 * worth recording because it is wrong: the same request with a mainstream
 * Chrome User-Agent returns 91 KB and still parses to the same nineteen nodes.
 * So http.c's "no User-Agent games either - this is what it is" costs nothing
 * here, which is the kind of decision worth confirming rather than assuming.
 *
 * The endpoint below serves an ordinary document to an ordinary GET: 33 KB in,
 * 615 nodes, 361 text nodes, and 1,534 pixels of laid-out results with the
 * titles as real links. It is one constant to change. */
#define SEARCH_HOST "html.duckduckgo.com"
#define SEARCH_PATH "/html/?q="


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
/* 128 WAS SHORT FOR A REAL ADDRESS and far too short once the bar can also
 * take a search. A wikipedia article URL fits; a great many real ones do not,
 * and a percent-encoded query costs three bytes per awkward character. This is
 * BSS and the whole browser holds twelve of these buffers, so 256 costs about
 * 1.5 KB against the 151 KB the kernel has left under link.ld's ceiling -
 * measured, not guessed. Overflow still truncates rather than scribbling;
 * every writer here is bounded by URL_MAX. */
/* 1024, AND THAT IS NOT GENEROSITY. Measured on the English Wikipedia article
 * for Linux: its first <link rel=stylesheet> href is 522 characters after
 * entity decoding - `/w/load.php?lang=en&modules=` followed by twenty module
 * names. At 256 that path is truncated, and a truncated path is not a failed
 * fetch, it is a REQUEST FOR THE WRONG THING that returns 404 and looks like
 * the server's fault. Twelve buffers at 1 KB is ~12 KB of BSS. */
#define URL_MAX  1024
#define HIST_N   8

static char url[URL_MAX];
static int  url_len;
static int  url_focus;
static int  url_sel_all;   /* focus selects the lot, as a browser does */
static char hist[HIST_N][URL_MAX];
static int  hist_n;
static int  fetching;
static int  last_status_code;
/* Declared here rather than beside the rest of the picture state below,
 * because navigate() has to be able to abandon a fetch in flight and sits
 * above it. See the block headed "pictures". */
static int  img_cur = -1;   /* the imgs[] index being fetched, or -1 */
/* THE STYLESHEET PHASE, which runs before the pictures. A page's <link
 * rel=stylesheet> decides where every box goes; a picture only fills one in.
 * Fetching an image into a layout that a stylesheet is about to rearrange is
 * work thrown away, and on a real page the stylesheet is the difference
 * between an article and a column of navigation links. */
static int  css_cur = -1;   /* html_css_links() index in flight, or -1 */
static int  sub_is_css;     /* what the response in flight actually is  */
static void img_next(void);
static void img_arrived(const unsigned char *body, int len);

static void sset(char *d, const char *s, int max)
{
    int i = 0;
    while (s[i] && i < max - 1) { d[i] = s[i]; i++; }
    d[i] = 0;
}

/* ---- the measure ----------------------------------------------------------
 * The one thing layout.c cannot know. Everything else about the type - the
 * atlas, the gamma-correct blend, the synthesised bold - stays in fb.c. */
/* WHY AN HTTPS FETCH FAILED, in the user's words rather than an error number.
 * Each of these is a DIFFERENT problem with a different fix, and collapsing
 * them into "connection failed" is how a certificate error gets mistaken for a
 * network outage - which is exactly the confusion an attacker benefits from. */
static const char *browser_tls_reason(void)
{
    switch (http_tls_error()) {
    case TLS_E_CERT:
        /* the specific reason from x509.c: expired, wrong host, unknown CA */
        return http_tls_why();
    case TLS_E_CERTVERIFY:
        return "the server did not prove it owns that certificate";
    case TLS_E_VERSION:
        return "the server does not speak TLS 1.3";
    case TLS_E_SUITE:
    case TLS_E_GROUP:
        return "the server offered no cipher this kernel implements";
    case TLS_E_PROTOCOL:
        if (http_rnd_quality() == 0)
            return "no entropy source: a key would be predictable, so https is refused";
        return "the TLS handshake was malformed";
    default:
        return "the secure connection failed";
    }
}

static int measure(const char *s, int len, int size, int style)
{
    return fb_text_rich_w(s, len, size, fb_style(style));
}

/* ---- the page that ships in the kernel -------------------------------------
 * Not a placeholder: this is the gate document. It uses every element the
 * parser claims to support, so "the browser renders" and "the browser renders
 * what it says it does" are the same observation. It also states what the
 * browser cannot do, on the screen, where a user will see them - the same
 * standard §5 sets for the padlock and SYSTEM-PROMPT.md sets for the `net up`
 * label.
 *
 * THIS PAGE WENT STALE IN THE DIRECTION NOBODY CHECKS FOR. It was written
 * before the network landed and said, to the user's face, "There is no driver
 * yet, so nothing can be fetched" - while `virtio_net.c` sat in SOURCES at 763
 * lines and the browser fetched `http://example.com/` by name. A page whose
 * whole job is to be the honest surface had become dishonest by understating,
 * which is the failure mode a review looks for least: an overclaim gets
 * challenged the first time someone tries it, an underclaim is never tested by
 * anyone, because nobody tries what they have been told is absent.
 *
 * The HTTPS entry was wrong on its stated reason too. It claimed "there is no
 * cipher in this kernel - only hashes". There are no hashes either: nothing
 * crypto is in SOURCES on any ref. A `crypto.c` does exist - 543 lines of
 * SHA-1, SHA-256, HMAC, PBKDF2 and AES-128, with `cryptotest.c` against
 * published FIPS and RFC vectors - but only inside the `refs/wip` snapshots, and
 * a file that is in no build is not in the kernel. The refusal POLICY is
 * unchanged and correct; only the reason given for it was false.
 *
 * If you add a capability, the edit to this page is part of the change, not a
 * follow-up.
 */
static const char home_page[] =
"<html><head><title>zlOS</title></head><body>\n"
"<h1>Welcome to zlOS</h1>\n"
"<p><strong>The machine is ready.</strong> This page is parsed, laid out and "
"drawn inside the kernel by zlOS itself.</p>\n"
"<hr>\n"
/* ---- THE NEW BOX MODEL, ON THE PAGE THAT IS THE GATE ------------------
 * Everything above this point was renderable before flex, grid, borders,
 * block backgrounds and images existed, which is exactly why none of them
 * would show up in browsershot's picture unless the gate document asked
 * for them. A feature the visual gate does not exercise is a feature the
 * visual gate cannot catch a regression in - and this browser has already
 * shipped two text regressions with every assertion green because the one
 * thing that would have shown them was a picture nobody diffed. */
"<style>\n"
"  .cards { display: flex; gap: 12px; margin-top: 12px; }\n"
"  .card  { flex: 1 1 0; padding: 10px; border: 1px solid #3a4454;\n"
"           background: #202836; }\n"
"  .card h4 { margin: 0 0 6px 0; }\n"
"  .grid  { display: grid; grid-template-columns: repeat(3, 1fr);\n"
"           gap: 8px; margin-top: 12px; }\n"
"  .cell  { padding: 8px; background: #26303f; border: 1px solid #3a4454; }\n"
"  .wide  { max-width: 460px; margin: 12px auto; padding: 10px;\n"
"           border: 1px solid #55d6ff; }\n"
"  .mark  { width: 32px; height: 32px; }\n"
"</style>\n"
"<h2>The box model</h2>\n"
"<p>The three blocks below are a <code>flex</code> row, a "
"<code>grid</code> of three equal <code>1fr</code> columns, and a block "
"centred with <code>max-width</code> and <code>margin: 0 auto</code>. "
"Each has a border and a background, which are boxes rather than text. "
"Narrow the window with <code>[</code> and they re-arrange.</p>\n"
"<div class=\"cards\">\n"
"  <div class=\"card\"><h4>flex</h4>Three items sharing the row, each "
"growing into an equal share of what is left.</div>\n"
"  <div class=\"card\"><h4>gap</h4>The space between them is one "
"property, not a margin on every child.</div>\n"
"  <div class=\"card\"><h4>stretch</h4>All three are as tall as the "
"tallest, which is what makes a row of cards line up.</div>\n"
"</div>\n"
"<div class=\"grid\">\n"
"  <div class=\"cell\">one</div><div class=\"cell\">two</div>\n"
"  <div class=\"cell\">three</div><div class=\"cell\">four</div>\n"
"  <div class=\"cell\">five</div><div class=\"cell\">six</div>\n"
"</div>\n"
"<div class=\"wide\">\n"
"<p>This block is <code>max-width: 460px; margin: 0 auto</code> - the "
"idiom most of the web is laid out with. Beside it is a real decoded "
"PNG, carried inside this page as a <code>data:</code> URI, so it needs "
"no network at all: 424 bytes of RGBA with genuinely transparent "
"corners, inflated and un-filtered by <code>png.c</code>.</p>\n"
"<img class=\"mark\" alt=\"the zlOS mark\" src=\"data:image/png;base64,"
"iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAABb0lEQVR42u2Xx1"
"ICQRCGeSITYkREFBFR9B1NmHNWzAnTCygiJsScw6VdD162urt6hqpZD/xVff6+"
"menp3XG58vlvKeg4gsLOYyjqsqo7CcU9SSiJnYA7lgJ3bwpK+07B058Gz0Aayg"
"bPoHzIquFzqBi5gMpRq8YuoWr8Cqqt8k5kwDuZgZqpa/BNZ8E3kwVH4bWzN+Cf"
"u6UlTMD984yACXjdwh0jYAAeWLynBSi4bjB4IM4IUCvXDQavX3qgBaht1xZA4A"
"2sAHHmusHgweVHWiDXhrMHgwdXnmgBXbh05b/wxlVOQBGusu1/8NDaMy2QC5xq"
"ODs8tP5CC0jgKt2OwZs2OAFFOHXPOXh485UWoODSCSeBh7cYATtcZbxK4c3bb4"
"yAQrdLgsEjO++0gPSeS4PBIwlGQDrhpMHgLYkPWoD6nmsLIPDW3U9GQPAzodpw"
"dnh0jxEwAY/uf9ECJuBtB5yAAXj74Tf/NnAUno8T+QGD/zPt3aHfvwAAAABJRU"
"5ErkJggg==""\">\n"
"</div>\n"
"<h2>What works</h2>\n"
"<ul>\n"
"<li>headings <code>h1</code> to <code>h6</code>, with a real type scale</li>\n"
"<li>paragraphs that <strong>reflow</strong> when the window is resized</li>\n"
"<li><em>emphasis</em>, <strong>strong</strong>, <code>monospace</code></li>\n"
"<li>ordered and unordered lists, nested</li>\n"
"<li>links, entities (&amp; &lt; &gt;), and malformed markup</li>\n"
"<li><strong>the network</strong> - a real URL over <code>http://</code> is "
"resolved by name, fetched and drawn</li>\n"
"<li><strong>the page's own stylesheet</strong> - "
"<code>&lt;style&gt;</code> and <code>style=</code>, cascaded by "
"specificity</li>\n"
"<li><strong>HTTPS</strong> - TLS 1.3, and the certificate is checked: the "
"chain to a known root, the host name, the dates, and a proof the server "
"holds the key</li>\n"
"<li><strong>tables</strong>, with columns sized to their contents</li>\n"
"<li><strong>JavaScript</strong>, of a bounded kind - see below</li>\n"
"</ul>\n"
"<h2>What does not</h2>\n"
"<ol>\n"
"<li><strong>HTTPS with an RSA certificate.</strong> TLS 1.3 works and the "
"certificate is verified, but only ECDSA over P-256 and P-384 is implemented, "
"so a site chained to an RSA authority is <em>refused</em> rather than "
"trusted. Most of Let's Encrypt works; much of the rest of the web does "
"not.</li>\n"
"<li><strong>Certificate revocation.</strong> Not checked at all. A "
"certificate withdrawn by its authority is still accepted until it "
"expires.</li>\n"
"<li><strong>Any card but virtio-net.</strong> The network comes up "
"automatically at boot when one is present. The driver matches PCI "
"<code>1af4:1041</code> and <code>1af4:1000</code> and nothing else, so QEMU "
"needs <code>-device virtio-net-pci</code>, and the ThinkPad's Intel part is "
"not supported.</li>\n"
"<li><strong>JavaScript for the modern web.</strong> There is an "
"interpreter - functions, recursion, arrays, strings, loops, "
"<code>document.write</code> - and it runs the kind of script a document "
"carries. It is not an <em>engine</em>: no DOM, no events, no promises, no "
"prototypes, no regular expressions. A page that is an application rather "
"than a document will not run, and that part really is unbounded.</li>\n"
"<li><strong>The rest of CSS.</strong> Type, class, id and descendant "
"selectors, the cascade, colours, sizes, weights, alignment, the box "
"(width, min and max, box-sizing, padding, borders, backgrounds, auto "
"margins), float and clear, position, <strong>flex</strong> and "
"<strong>grid</strong> are all read and acted on. What is refused is named "
"rather than half-matched: pseudo-classes, <code>calc()</code>, media "
"queries, grid areas and spans, <code>order</code>, and baseline "
"alignment. <code>position: sticky</code> lays out as relative and "
"<code>fixed</code> as absolute, which is right until the page scrolls.</li>\n"
"<li><strong>Pixel parity with another browser.</strong> This is the one "
"genuinely unbounded thing in the list and it is refused on purpose. Flex "
"and grid have specifications, so they are finite and they are built; "
"matching Chrome exactly is not a specification, it is a moving "
"target.</li>\n"
"</ol>\n"
"<h3>Try it</h3>\n"
"<p><strong>Click the bar and type.</strong> Something with a dot in it is an "
"address and is fetched; anything else is a <strong>search</strong>. That is "
"the same rule every browser uses, and it is the whole of the difference "
"between a fetcher and something you can actually use.</p>\n"
"<p><strong>On Google specifically</strong>, since it is the obvious thing to "
"try: <code>google.com</code> is fetched over verified TLS 1.3 and it does "
"return HTTP 200. But <code>/search</code> serves a JavaScript bootstrap and "
"a <code>&lt;noscript&gt;</code> notice - measured here, 71 KB of HTML parses "
"to nineteen nodes and 151 characters reading <em>\"click here if you are not "
"redirected\"</em>. A mainstream User-Agent changes nothing. There is no "
"document to render, so search goes somewhere that serves one.</p>\n"
"<p>Press <code>[</code> and <code>]</code> to narrow and widen this window. "
"The text reflows: the line breaks are computed, not drawn. That is the "
"difference between a layout engine and a picture of one.</p>\n"
"<pre>  html.c    the tokenizer and the tree\n"
"  layout.c  block and inline boxes\n"
"  browser.c this window</pre>\n"
"<p>A <a href=\"http://example.com/\">link</a> is drawn in the accent colour "
"and underlined. Following one fetches it.</p>\n"
"</body></html>\n";

static void hist_push(const char *u);
static const char HOME_URL[];

/* ---- pictures ---------------------------------------------------------------
 * THE DECODER IS NOT THE HARD PART OF SHOWING AN IMAGE. png.c turns bytes into
 * pixels and it is testable in isolation; what only this file can do is decide
 * WHICH bytes, and when. A page's <img> elements are subresources - each one
 * is a second request the document did not ask for explicitly - and tcp.c
 * holds exactly ONE connection. So they are fetched strictly one at a time,
 * after the document, and the page is usable throughout: a picture that has
 * not arrived lays out at its declared size and paints as the same honest
 * empty frame it always did.
 *
 * That ordering is a real decision and not a limitation of effort. Fetching
 * them in parallel needs a connection pool, and a connection pool in a stack
 * with one connection slot is a different track.
 *
 * TWELVE, because the arena png.c holds is finite and a page with three
 * hundred images would otherwise decide how much of this kernel's RAM it
 * gets. Beyond twelve an <img> keeps its box and never gets pixels, which is
 * the same outcome as a picture that has not arrived yet - so the page does
 * not change shape when the cap is hit. */
#define IMG_N 12

#define IMG_WANTED   0    /* seen in the document, nothing done yet */
#define IMG_FETCHING 1
#define IMG_DONE     2
#define IMG_FAILED   3

static struct {
    int node;
    int slot;             /* png.c arena slot, or -1              */
    int state;
} imgs[IMG_N];
static int nimgs;

/* WHERE THE PIXELS AND THE SCRATCH LIVE, and it is not BSS. link.ld asserts
 * the kernel image ends under 6 MiB and it already reaches 5.573 MiB, so a
 * couple of megabytes of static array does not link - and the error says "the
 * kernel image has grown into the raw-boot stack", which names the stack
 * rather than the picture. So both go in the fixed high-RAM map, exactly as
 * fb.c's back buffer and every DMA arena do. memmap.h owns the addresses and
 * the compiler checks they do not overlap a neighbour.
 *
 * NO #ifdef FOR THE HOST. The harnesses mmap these same addresses with
 * MAP_FIXED_NOREPLACE, which is the rule fbbench.c states and the reason the
 * shipping source compiles unmodified in both places. */
static unsigned int * const png_arena = (unsigned int *)(uptr)HI_IMG;

/* base64, for `data:` URIs. A page can carry its own pictures inline, and
 * when it does they need no network at all - which is why this is here and
 * not left for the fetch path: it makes an image end-to-end testable in a
 * host harness with no machine on the other end of a wire. A data: URI is
 * bounded by the 256 KB document carrying it and base64 costs 4 bytes in for
 * 3 out, so 2 MiB can never be the binding limit. */
#define IMGBUF 0x200000
static unsigned char * const img_buf = (unsigned char *)(uptr)HI_IMG_SCRATCH;

static int b64val(int c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;                       /* '=' and every kind of whitespace */
}

/* Returns the decoded length, or -1 when the input does not fit. Whitespace
 * is skipped rather than rejected: a data: URI in real markup is wrapped
 * across lines, and a decoder that refuses those refuses most of them. */
static int b64_decode(const char *s, int len)
{
    /* UNSIGNED, AND MASKED AFTER EVERY BYTE. `acc` is a shift register holding
     * at most 12 bits of pending input, but nothing threw the consumed bits
     * away - so it grew six bits per character and overflowed a signed int on
     * the FIFTH one. Signed overflow is undefined behaviour, not a wrapped
     * number, and this was not an edge case: it happened on the home page's
     * own inline image, every single time.
     *
     * It decoded correctly regardless, because the value read back is always
     * the low bits - which is exactly why nothing noticed. Found by an
     * adversarial review from a different model family; UBSan confirms it in
     * four lines. The reason the harness missed it is that browsertest is not
     * built with the sanitizers, while the code either side of it is. */
    unsigned acc = 0;
    int out = 0, bits = 0;
    for (int i = 0; i < len; i++) {
        int v = b64val((unsigned char)s[i]);
        if (v < 0) continue;
        acc = (acc << 6) | (unsigned)v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (out >= IMGBUF) return -1;
            img_buf[out++] = (unsigned char)((acc >> bits) & 0xFFu);
        }
        acc &= (1u << bits) - 1u;      /* keep only what is still owed */
    }
    return out;
}

/* `data:[<type>][;base64],<payload>`. Anything that is not base64 is refused
 * rather than guessed at: a percent-encoded binary payload is legal and rare,
 * and decoding it wrong produces a corrupt image with no error, which is the
 * outcome this whole file is written to avoid. */
static int img_try_data_uri(const char *src, int slen)
{
    if (slen < 5) return -1;
    for (int k = 0; k < 5; k++) if (src[k] != "data:"[k]) return -1;
    int comma = -1, b64 = 0;
    for (int k = 5; k < slen; k++) if (src[k] == ',') { comma = k; break; }
    if (comma < 0) return -1;
    for (int k = 5; k + 6 <= comma; k++) {
        int m = 1;
        for (int j = 0; j < 6; j++) if (src[k + j] != "base64"[j]) { m = 0; break; }
        if (m) { b64 = 1; break; }
    }
    if (!b64) return -1;
    int n = b64_decode(src + comma + 1, slen - comma - 1);
    if (n <= 0) return -1;
    return png_decode(img_buf, n);
}

/* ---- handing the storage over ---------------------------------------------
 * ONCE, AND NOT AT BOOT. There is no browser_init in this app - it is
 * self-initialising by design (see browser_draw's header on why the compositor
 * must not decide its content), so every arena is handed over the first time a
 * document could possibly want one.
 *
 * ALL FOUR TOGETHER, and that is not tidiness. png.c's arena used to be set
 * from img_collect() alone, which was correct while it was the only injected
 * storage; html.c's has to be set BEFORE html_parse and css.c's before the
 * first css_add_sheet, both of which happen earlier in doc_set than
 * img_collect does. Four separate lazy initialisers ordered by hand is four
 * chances for the next person to add a fifth in the wrong place, so there is
 * one, and doc_set calls it first thing.
 *
 * Until it runs, html.c parses nothing, css.c takes no rules, layout.c emits
 * no runs and png.c fails every decode - each loudly, through the counter it
 * already exposes. That is deliberate: see the header of each.
 *
 * THE INVARIANT THAT MAKES ONE CALL SITE ENOUGH, written down because it is
 * load-bearing and lives in another function: browser_draw does
 * `if (doc_len == 0) browser_home()` before it lays anything out, and
 * browser_home goes through doc_set. So relayout() cannot be reached without
 * a document, and therefore cannot be reached before this has run. Break that
 * line in browser_draw and layout.c starts refusing every run - visibly, via
 * lay_overflowed(), rather than by faulting on a null array. */
static int storage_set;

static void storage_init(void)
{
    if (storage_set) return;
    storage_set = 1;
    html_set_arena((void *)(uptr)(HI_DOM + BR_OFF_NODES), HTML_MAX_NODES,
                   (char *)(uptr)(HI_DOM + BR_OFF_HARENA), HTML_ARENA);
    css_set_arena((void *)(uptr)(HI_DOM + BR_OFF_SELS),  CSS_MAX_SELS,
                  (void *)(uptr)(HI_DOM + BR_OFF_DECLS), CSS_MAX_DECLS,
                  (char *)(uptr)(HI_DOM + BR_OFF_CARENA), CSS_ARENA);
    lay_set_arena((struct lay_run *)(uptr)(HI_DOM + BR_OFF_RUNS), LAY_MAX_RUNS);
    png_set_arena(png_arena, PNG_ARENA_PX);
}

/* Collect the document's <img> elements. Called once per parse, from doc_set,
 * so the list can never disagree with the tree layout is walking. */
static void img_collect(void)
{
    storage_init();
    png_reset();
    nimgs = 0;
    img_cur = -1;
    css_cur = -1;
    sub_is_css = 0;
    for (int n = 0; n < html_count() && nimgs < IMG_N; n++) {
        if (html_kind(n) != HN_ELEM || html_tag(n) != HT_IMG) continue;
        int slen;
        const char *src = html_src(n, &slen);
        if (slen <= 0) continue;
        imgs[nimgs].node = n;
        imgs[nimgs].slot = -1;
        imgs[nimgs].state = IMG_WANTED;
        /* an inline picture needs nothing from the network, so it is resolved
         * here and never enters the fetch queue at all */
        int s = img_try_data_uri(src, slen);
        if (s >= 0) { imgs[nimgs].slot = s; imgs[nimgs].state = IMG_DONE; }
        nimgs++;
    }
}


/* A picture's bytes arrived. Decoding can fail for a dozen reasons and every
 * one of them leaves the page exactly as it was - png_why() records which,
 * and the <img> keeps its reserved box, so nothing on screen moves. */
static void img_arrived(const unsigned char *body, int len)
{
    if (img_cur < 0 || img_cur >= nimgs) return;
    int slot = (body && len > 0) ? png_decode(body, len) : -1;
    if (slot >= 0) {
        imgs[img_cur].slot = slot;
        imgs[img_cur].state = IMG_DONE;
        /* THE BOX CHANGES SIZE NOW, at an unchanged window width - which is
         * exactly the case relayout()'s old width-only test could not see. */
        lay_dirty = 1;
        laid_w = 0;
    } else {
        imgs[img_cur].state = IMG_FAILED;
    }
}

/* layout.c's hook. It asks by NODE and is told a slot and an intrinsic size,
 * or -1 - which is exactly the amount of coupling layout.h asks for: the box
 * model still links with no decoder present and no framebuffer. */
static int img_for_node(int node, int *w, int *h)
{
    for (int i = 0; i < nimgs; i++) {
        if (imgs[i].node != node || imgs[i].slot < 0) continue;
        if (w) *w = png_w(imgs[i].slot);
        if (h) *h = png_h(imgs[i].slot);
        return imgs[i].slot;
    }
    return -1;
}


/* ---- loading --------------------------------------------------------------- */
static void doc_set(const char *src, int len)
{
    /* FIRST, BEFORE THE COPY INTO `doc`. `doc` is itself part of the region
     * this hands out, and html_parse two lines below wants the tree array. */
    storage_init();
    doc_truncated = 0;
    if (len > DOC_MAX - 1) { len = DOC_MAX - 1; doc_truncated = 1; }
    for (int i = 0; i < len; i++) doc[i] = src[i];
    doc[len] = 0;
    doc_len = len;
    html_parse(doc, doc_len);

    /* THE DOCUMENT'S OWN STYLESHEETS, in document order, which is also their
     * cascade order. Reset first: a stylesheet from the PREVIOUS page styling
     * the next one is a bug that only shows on the second navigation, which is
     * exactly the kind nobody looks for. The sheets are spans of `doc`, so
     * they stay valid as long as this document is loaded - and doc_set is the
     * only thing that replaces it. */
    css_reset();
    /* THE WIDTH @media IS JUDGED AGAINST, set before any sheet is added
     * because css.c evaluates the queries at PARSE time. `laid_w` is the last
     * width actually laid out; on the very first document there is none yet,
     * and 1024 is a desktop-shaped guess rather than 0 - a 0 refuses every
     * width query, which is the pre-media behaviour and would make the first
     * paint of every page differ from the second. */
    css_viewport(laid_w > 0 ? laid_w : 1024);
    for (int k = 0; k < html_sheets(); k++) {
        int slen;
        const char *s = html_sheet(k, &slen);
        css_add_sheet(s, slen);
    }

    /* ---- the scripts -----------------------------------------------------
     * RUN AFTER THE PARSE, NOT DURING IT. A real browser executes a <script>
     * at the point the parser reaches it, because document.write injects text
     * INTO the parse. Doing that here would mean re-entering html_parse from
     * inside itself, and this parser is not re-entrant.
     *
     * So the bounded version: run every script once the tree is built, append
     * whatever they wrote to the document, and reparse ONCE if anything did.
     * That gets a script whose output is its whole purpose - the common case
     * in a document - and does NOT get a script that expects to interleave
     * with the parser. js.h says which is which.
     *
     * ONE REPARSE, not a loop: a script that writes a script that writes a
     * script is a fixed point nobody needs, and bounding it here is cheaper
     * than discovering the loop in a kernel. */
    js_out_len = 0;
    if (html_scripts() > 0) {
        int wrote = 0;
        for (int k = 0; k < html_scripts(); k++) {
            int sl;
            const char *sc = html_script(k, &sl);
            if (sl <= 0) continue;
            if (js_eval(sc, sl) != 0) {
                /* a script that fails is REPORTED, not hidden - a blank area
                 * where content should be is the least debuggable outcome */
                sset(js_err, js_error(), (int)sizeof js_err);
                continue;
            }
            int ol;
            const char *o = js_output(&ol);
            for (int i = 0; i < ol && js_out_len < JS_OUT_MAX - 1; i++)
                js_out[js_out_len++] = o[i];
            if (ol > 0) wrote = 1;
        }
        js_out[js_out_len] = 0;
        if (wrote && doc_len + js_out_len < DOC_MAX - 1) {
            for (int i = 0; i < js_out_len; i++) doc[doc_len + i] = js_out[i];
            doc_len += js_out_len;
            doc[doc_len] = 0;
            html_parse(doc, doc_len);
            css_reset();
            for (int k = 0; k < html_sheets(); k++) {
                int sl2;
                const char *s2 = html_sheet(k, &sl2);
                css_add_sheet(s2, sl2);
            }
        }
    }

    lay_set_measure(measure);
    /* THE HOOK IS INSTALLED HERE AND NOWHERE ELSE, beside lay_set_measure and
     * for the same reason: both are the app handing layout.c the two things it
     * refuses to include. img_collect must run before the first layout of this
     * document, or the first paint asks about nodes that are not in the list
     * yet and every inline picture renders once as an empty frame. */
    lay_set_image(img_for_node);
    img_collect();
    laid_w = 0;                       /* force a layout on the next paint */
    lay_dirty = 1;
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


/* The pointer form is the real one; the address form is the zl seam. Splitting
 * them is not test scaffolding - it is that `unsigned int` is the right type
 * for a zl number carrying a physical address and the WRONG type for a C
 * caller that already has a pointer, which on a 64-bit build it silently
 * truncates. The kernel's own buffers all live below 4 GiB, so the address
 * form is safe where it is used; a caller with a pointer should never have to
 * round-trip it through a narrower integer to find that out. */
void browser_load(const char *src, int len)
{
    if (!src || len <= 0) { browser_home(); return; }
    doc_set(src, len);
}

void browser_load_mem(unsigned int addr, int len)
{
    if (!addr || len <= 0) { browser_home(); return; }
    browser_load((const char *)(uptr)addr, len);
}

int browser_truncated(void) { return doc_truncated; }

/* THE DOCUMENT'S OWN status code and size, captured when it landed. The gates
 * used to read http_code()/http_len() straight from http.c, which was the same
 * thing right up until the browser learned to fetch SUBRESOURCES: by the time
 * a gate prints its result, http.c is describing a stylesheet or a picture, so
 * a perfectly good fetch reported "HTTP 0   body 0 bytes". The page's numbers
 * belong to the page. */
int browser_code(void)    { return last_status_code; }
int browser_doc_len(void) { return doc_len; }
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
static int req_needs_dns;   /* the host is a name, not a dotted quad */
static int req_tls;         /* the URL said https                     */

static int parse_url(const char *u, int len)
{
    int i = 0;
    req_tls = 0;
    if (len >= 8) {
        int https = 1;
        for (int k = 0; k < 8; k++) if (u[k] != "https://"[k]) { https = 0; break; }
        if (https) { req_tls = 1; i = 8; }
    }
    if (!req_tls && len >= 7) {
        int http = 1;
        for (int k = 0; k < 7; k++) if (u[k] != "http://"[k]) { http = 0; break; }
        if (http) i = 7;
    }
    int hs = i;
    while (i < len && u[i] != '/' && u[i] != ':') i++;
    int he = i;
    req_port = req_tls ? 443 : 80;
    if (i < len && u[i] == ':') {
        i++;
        int p = 0;
        while (i < len && is_digit(u[i])) {
            if (p > 65535) { p = 65536; i++; continue; }   /* saturate - see below */
            p = p * 10 + (u[i++] - '0');
        }
        /* PRE-EXISTING, and it sits on the one trust boundary in this file
         * that takes whatever a PERSON typed. `p * 10` was unbounded, so
         * http://10.0.2.2:99999999999/ overflowed a signed int - undefined
         * behaviour - and a wrapped result can land back inside 1..65535, at
         * which point the range check passes and a port nobody typed is
         * fetched. Saturating stops the arithmetic before it is undefined and
         * leaves the range check below meaning what it says. */
        if (p > 0 && p < 65536) req_port = p;
    }
    int ps = i;
    if (he <= hs) { status = BR_FAILED; return 0; }
    /* A NAME IS NOT AN ERROR ANY MORE. It used to be refused outright, because
     * there was no resolver; now it is a lookup, and the fetch waits for it. */
    req_needs_dns = !parse_quad(u + hs, he - hs, &req_ip);

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
/* IS THIS AN ADDRESS OR IS IT A QUESTION? Every browser answers this and none
 * of them get it perfectly right, because the two languages overlap. The rules
 * here are the conventional ones, in the order that matters:
 *
 *   a scheme          -> an address, always. "https://x" is never a search.
 *   a space           -> a search. No host name contains one.
 *   no dot in the host-> a search. "zlos" is a word; "zlos.com" is a host.
 *
 * "node.js" is therefore treated as an address, and that is what Chrome does
 * with it too. Getting that case "right" needs a public-suffix list, which is
 * a downloadable database that changes weekly - the unbounded version of this
 * question, and not one a browser has to answer to be useful. */
static int looks_like_url(const char *u, int len)
{
    if (len <= 0) return 0;
    if (len >= 7) {
        int http = 1, https = (len >= 8);
        for (int k = 0; k < 7; k++) if (u[k] != "http://"[k]) { http = 0; break; }
        if (https) for (int k = 0; k < 8; k++) if (u[k] != "https://"[k]) { https = 0; break; }
        if (http || https) return 1;
    }
    for (int i = 0; i < len; i++) if (u[i] == ' ' || u[i] == '\t') return 0;
    /* a dot before the first '/' or ':' is what makes it a host name */
    for (int i = 0; i < len; i++) {
        if (u[i] == '/' || u[i] == ':' || u[i] == '?') break;
        if (u[i] == '.') return 1;
    }
    return 0;
}

/* Percent-encode a query into `out`. Unreserved characters pass, a space
 * becomes '+', everything else becomes %XX - including, deliberately, every
 * byte above 127, because this browser has no idea what encoding the person
 * typing was thinking in and a raw high byte in a request line is how a
 * request gets split by something in the middle. Truncates rather than
 * overrunning, and says so by returning the length it managed. */
static int url_encode(const char *q, int len, char *out, int max)
{
    static const char hex[] = "0123456789ABCDEF";
    int n = 0;
    for (int i = 0; i < len && n < max - 4; i++) {
        unsigned char c = (unsigned char)q[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~')
            out[n++] = (char)c;
        else if (c == ' ')
            out[n++] = '+';
        else {
            out[n++] = '%';
            out[n++] = hex[(c >> 4) & 0xF];
            out[n++] = hex[c & 0xF];
        }
    }
    out[n] = 0;
    return n;
}

static int navigate(const char *u, int len, int record)
{
    status = BR_OK;
    /* ABANDON ANY PICTURE IN FLIGHT, FIRST. browser_tick decides what an
     * HTTP_DONE means by looking at img_cur, so leaving the old page's cursor
     * set while a new DOCUMENT is on its way would hand the new page's HTML to
     * the PNG decoder and leave the browser showing nothing with no error.
     * Found by reading the state machine, not by seeing it - which is the only
     * way this one is findable, because it needs a navigation timed inside a
     * subresource fetch. */
    img_cur = -1;
    /* ...AND CANCEL WHAT IT WAS DOING, which clearing the cursor alone does
     * not. The about:home branch below RETURNS EARLY, so `fetching` stayed 1
     * with an image response still in flight; when that response landed,
     * browser_tick saw img_cur == -1, took the DOCUMENT branch, and handed the
     * PNG's bytes to doc_set() - replacing the page with binary parsed as
     * HTML. Every other exit from this function resets http on its way past.
     * The early one did not, which is the shape of bug an early return makes. */
    if (fetching) { http_reset(); fetching = 0; }
    css_cur = -1; sub_is_css = 0;
    /* THE BAR SHOWS WHERE YOU ARE, however you got there. Only the typed path
     * used to set it, so a navigation from anywhere else - Back, a link, the
     * shell - left the previous address on screen while a different page
     * loaded underneath it. */
    if (u != url) { sset(url, u, URL_MAX); url_len = 0; while (url[url_len]) url_len++; }
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
    /* NOT AN ADDRESS? THEN IT IS A SEARCH. This has to happen after the
     * about:home check above and before parse_url, and it rewrites into a
     * scratch buffer rather than into `url` - because `u` very often IS `url`
     * (the URL bar calls navigate(url, url_len, 1)), and building the search
     * address on top of its own input would consume the query as it wrote. */
    static char searched[URL_MAX];
    if (!looks_like_url(u, len)) {
        int n = 0;
        const char *pre = "https://" SEARCH_HOST SEARCH_PATH;
        while (pre[n] && n < URL_MAX - 8) { searched[n] = pre[n]; n++; }
        n += url_encode(u, len, searched + n, URL_MAX - n);
        searched[n] = 0;
        /* the bar shows the ADDRESS it went to, not the words - same rule as
         * every other navigation here, and it is what makes Back coherent */
        sset(url, searched, URL_MAX);
        url_len = n;
        u = searched;
        len = n;
    }

    if (!parse_url(u, len)) return 0;
    if (!net_live()) { status = BR_NO_NET; return 0; }

    tcp_attach(net_send_ip, net_ip());
    net_set_proto_sink(6, tcp_input);
    net_set_proto_sink(17, dns_ip_sink);
    http_reset();

    if (req_needs_dns) {
        int hl = 0; while (req_host[hl]) hl++;
        dns_reset();
        if (!dns_start(req_host, hl)) { status = BR_NO_DNS; return 0; }
        if (record) hist_push(u);
        fetching = 1;
        status = BR_RESOLVING;
        return 1;
    }

    if (!(req_tls ? http_start_tls(req_ip, req_port, req_host, req_path)
                  : http_start(req_ip, req_port, req_host, req_path))) {
        status = BR_FAILED;
        return 0;
    }
    if (record) hist_push(u);
    fetching = 1;
    status = BR_FETCHING;
    return 1;
}

void browser_go(const char *u, int len) { navigate(u, len, 1); }

/* ---- resolving an <img src> against the page it came from -------------------
 * A page writes `src="/static/logo.png"` far more often than it writes the
 * whole address, so a fetcher that only understands absolute URLs fetches
 * almost nothing. Four forms, which is all of them that occur:
 *
 *   http://host/p  https://host/p   absolute
 *   //host/p                        protocol-relative
 *   /p                              root-relative
 *   p  or  ../p                     relative to the document's directory
 *
 * SAME HOST ONLY, and that is a bounded decision rather than an oversight.
 * A different host needs a second DNS lookup, and the resolver's state machine
 * lives in navigate()/browser_tick() as ONE lookup for ONE fetch - running a
 * second one underneath the first is a different shape, not a longer function.
 * A cross-host image keeps its box and its empty frame, which is the same
 * thing the user sees while any picture is still on its way, so the page does
 * not jump when this limit is hit. That is the next increment, not this one.
 *
 * Returns 1 and fills `out` with the path to GET, or 0 to refuse. */
static int img_resolve(const char *src, int slen, char *out, int max)
{
    if (slen <= 0) return 0;

    int i = 0;
    int had_scheme = 0, want_tls = req_tls;
    if (slen >= 8 && src[0] == 'h') {
        int https = 1, http = 1;
        for (int k = 0; k < 8; k++) if (src[k] != "https://"[k]) { https = 0; break; }
        for (int k = 0; k < 7; k++) if (src[k] != "http://"[k])  { http = 0;  break; }
        if (https)     { i = 8; had_scheme = 1; want_tls = 1; }
        else if (http) { i = 7; had_scheme = 1; want_tls = 0; }
    }
    if (!had_scheme && slen >= 2 && src[0] == '/' && src[1] == '/') {
        i = 2; had_scheme = 1;                 /* protocol-relative: keep ours */
    }

    if (had_scheme) {
        /* the authority must be OUR authority, or we cannot reach it */
        if (want_tls != req_tls) return 0;
        int hs = i;
        while (i < slen && src[i] != '/' && src[i] != ':') i++;
        int hlen = i - hs;
        int rl = 0; while (req_host[rl]) rl++;
        if (hlen != rl) return 0;
        for (int k = 0; k < hlen; k++) if (src[hs + k] != req_host[k]) return 0;
        int port = req_tls ? 443 : 80;
        if (i < slen && src[i] == ':') {
            i++;
            int p = 0;
            while (i < slen && is_digit(src[i])) {
                if (p > 65535) { p = 65536; i++; continue; }
                p = p * 10 + (src[i++] - '0');
            }
            /* SATURATED WHILE ACCUMULATING, not range-checked afterwards.
             * `:99999999999` overflows a signed int before the check can run,
             * and a wrapped value can land back inside 1..65535 - so the
             * malformed authority is not refused, it is silently fetched from
             * a DIFFERENT port. Same defect as parse_url's, which this was
             * modelled on and which is fixed there too. */
            if (p <= 0 || p >= 65536) return 0;
            port = p;
        }
        if (port != req_port) return 0;
        int n = 0;
        if (i >= slen) { out[n++] = '/'; out[n] = 0; return 1; }
        for (; i < slen && n < max - 1; i++) out[n++] = src[i];
        out[n] = 0;
        return 1;
    }

    if (src[0] == '/') {                        /* root-relative */
        int n = 0;
        for (; i < slen && n < max - 1; i++) out[n++] = src[i];
        out[n] = 0;
        return 1;
    }

    /* relative: everything up to and including the document's last '/' */
    int cut = 0;
    for (int k = 0; req_path[k]; k++) if (req_path[k] == '/') cut = k + 1;
    int n = 0;
    for (int k = 0; k < cut && n < max - 1; k++) out[n++] = req_path[k];
    for (int k = 0; k < slen && n < max - 1; k++) out[n++] = src[k];
    out[n] = 0;
    return 1;
}

static char img_path[URL_MAX];

/* Start the next picture, or stop. One at a time, because tcp.c holds one
 * connection - see the block header above. */
static void img_next(void)
{
    /* ---- stylesheets first ---- */
    while (css_cur + 1 < html_css_links()) {
        css_cur++;
        int slen;
        const char *u = html_css_link(css_cur, &slen);
        if (!img_resolve(u, slen, img_path, URL_MAX)) continue;
        http_reset();
        http_accept(HTTP_ACCEPT_CSS);
        int ok = req_tls ? http_start_tls(req_ip, req_port, req_host, img_path)
                         : http_start(req_ip, req_port, req_host, img_path);
        if (!ok) continue;
        sub_is_css = 1;
        img_cur = -1;
        fetching = 1;
        status = BR_IMAGES;
        return;
    }
    sub_is_css = 0;

    img_cur = -1;
    for (int i = 0; i < nimgs; i++) {
        if (imgs[i].state != IMG_WANTED) continue;
        int slen;
        const char *src = html_src(imgs[i].node, &slen);
        if (!img_resolve(src, slen, img_path, URL_MAX)) {
            imgs[i].state = IMG_FAILED;
            continue;
        }
        http_reset();
        /* THE ONLY FETCH IN THIS BROWSER THAT ASKS FOR A PICTURE. The default
         * is text-only and http_reset restores it, so the document fetch above
         * cannot inherit this by accident. */
        http_accept(HTTP_ACCEPT_IMAGE);
        int ok = req_tls ? http_start_tls(req_ip, req_port, req_host, img_path)
                         : http_start(req_ip, req_port, req_host, img_path);
        if (!ok) { imgs[i].state = IMG_FAILED; continue; }
        imgs[i].state = IMG_FETCHING;
        img_cur = i;
        fetching = 1;
        status = BR_IMAGES;
        return;
    }
    /* nothing left to ask for */
    if (status == BR_IMAGES) status = BR_OK;
    fetching = 0;
}

/* Called every frame. Returns 1 when something changed and the window needs
 * repainting - the app contract's tick, which must be cheap and must not draw. */
int browser_tick(void)
{
    if (!fetching) return 0;
    for (int i = 0; i < 64; i++) net_poll_once();

    /* The lookup comes first, and it is a separate state rather than a hidden
     * pause inside the fetch: "looking up the name" and "connecting" fail for
     * different reasons and the status strip says which. */
    if (status == BR_RESOLVING) {
        int d = dns_poll();
        if (d == DNS_DONE) {
            req_ip = dns_result();
            req_needs_dns = 0;
            if (!(req_tls ? http_start_tls(req_ip, req_port, req_host, req_path)
                  : http_start(req_ip, req_port, req_host, req_path))) {
                fetching = 0; status = BR_FAILED; return 1;
            }
            status = BR_FETCHING;
            return 1;
        }
        if (d == DNS_ASKING) return 0;
        fetching = 0;
        status = BR_NO_DNS;
        return 1;
    }

    tcp_tick();
    int s = http_poll();

    /* WHOSE RESPONSE IS THIS? The same three outcomes mean different things
     * for a document and for one of its pictures, and img_cur is the only
     * thing that distinguishes them - which is why navigate() clears it. A
     * failed picture is NOT a failed page: the document stays on screen and
     * that <img> keeps the empty frame it already had, because a logo that
     * did not load must not blank an article. */
    if (sub_is_css) {
        if (s == HTTP_DONE) {
            /* css.c INTERNS everything it keeps, so handing it the HTTP
             * buffer directly is safe - nothing here has to outlive the next
             * request. It refuses gracefully when its arena is full, which a
             * page the size of Wikipedia's skin will do, and css_overflowed()
             * says so rather than truncating a rule in half. */
            css_add_sheet((const char *)(uptr)http_body_addr(), http_body_len());
            lay_dirty = 1;
            laid_w = 0;
            img_next();
            return 1;
        }
        if (s == HTTP_REFUSED || s == HTTP_ERROR) {
            /* a stylesheet that will not load is not a failed page - the
             * document stays exactly as it is, styled by whatever did load */
            img_next();
            return 1;
        }
        return 0;
    }

    if (img_cur >= 0) {
        if (s == HTTP_DONE) {
            img_arrived((const unsigned char *)(uptr)http_body_addr(),
                        http_body_len());
            img_next();
            return 1;
        }
        if (s == HTTP_REFUSED || s == HTTP_ERROR) {
            imgs[img_cur].state = IMG_FAILED;
            img_next();
            return 1;
        }
        return 0;
    }

    if (s == HTTP_DONE) {
        fetching = 0;
        last_status_code = http_status();
        doc_set((const char *)(uptr)http_body_addr(), http_body_len());
        status = BR_OK;
        /* the document is on screen NOW; its pictures follow it one at a time
         * and each one repaints when it lands */
        img_next();
        return 1;
    }
    if (s == HTTP_REFUSED) { fetching = 0; status = BR_BAD_TYPE; return 1; }
    if (s == HTTP_ERROR)   { fetching = 0; status = BR_FAILED;   return 1; }
    /* FOLLOW THE REDIRECT. http.h defines HTTP_REDIRECT as "3xx with a Location,
     * under the redirect limit" - i.e. the state means the redirect is valid and
     * followable - and nothing consumed it, so every 3xx left `fetching` set and
     * this function returning 0 for ever, with no timeout. http.c has already
     * done the work: it found the Location, counted the hop and checked it
     * against HTTP_MAX_REDIRECTS before entering this state, so the loop guard
     * is upstream and this side just has to act.
     *
     * record = 0: a redirect is not a page the user navigated to, so it does not
     * belong in Back history - otherwise Back walks through every hop. */
    if (s == HTTP_REDIRECT) {
        const char *loc = http_location();
        int n = 0;
        while (loc[n]) n++;
        if (n > 0 && navigate(loc, n, 0)) return 1;   /* navigate() re-arms fetching */
        fetching = 0;
        status = BR_FAILED;
        return 1;
    }
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
    if (w == laid_w && !lay_dirty) return;
    laid_w = w;
    lay_dirty = 0;
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
const char *status_text(void)
{
    switch (status) {
    case BR_NO_TLS:   return browser_tls_reason();
    /* NO LONGER "run the network gate first". The network comes up at boot
     * now, so if it is not up there is no card - and telling someone to run a
     * diagnostic they cannot fix anything with is worse than telling them
     * what is actually absent. */
    case BR_NO_NET:   return "no network card - QEMU needs -device virtio-net-pci";
    case BR_NO_DNS:   return "that name does not resolve";
    case BR_RESOLVING: return "looking up the name...";
    case BR_FETCHING: return "fetching...";
    case BR_FAILED:   return "the fetch failed - is anything listening there?";
    case BR_BAD_TYPE: return "refused: not text/html or text/plain";
    case BR_IMAGES:   return "loading pictures...";
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
    back_x = x + em / 4; back_y = y + em / 4; back_w = backw; back_h = rowh;

    int ux = x + em / 4 + backw + em / 4;
    int uw = w - (ux - x) - em / 4;
    bar_x = ux; bar_y = y + em / 4; bar_w = uw; bar_h = rowh;
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
                     "click here or press l - type an address, or words to search",
                     t->text_dim);
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
        /* An author colour overrides the ROLE, and only the role - the theme
         * still owns everything the document did not ask for. This is the one
         * place the two meet, which is why layout.c can carry an RGB without
         * knowing a theme exists. */
        if (r->rgb != LR_NO_RGB) col = (unsigned int)r->rgb;
        /* the author's background, painted behind the run - what makes
         * <code> and highlighted spans read as boxes rather than bare text */
        if (r->bg != LR_NO_RGB && r->w > 0 && r->h > 0)
            fb_fill_px(rx, ry, r->w, r->h, (unsigned int)r->bg);

        /* A BLOCK'S BACKGROUND AND NOTHING ELSE. The generic bg fill above has
         * already painted it, and the run is emitted before its children, so
         * document order is paint order and there is nothing left to do. */
        if (r->kind == LR_BOX) continue;
        if (r->kind == LR_RULE) {
            /* An author's colour, when there is one - a border edge is an
             * LR_RULE carrying rgb, which is why this is no longer
             * unconditionally the theme's rule colour. <hr> still gets the
             * theme, because it asks for nothing. */
            fb_fill_px(rx, ry, r->w, r->h > 0 ? r->h : 1,
                       r->rgb != LR_NO_RGB ? (unsigned int)r->rgb : t->border);
            continue;
        }
        if (r->kind == LR_BULLET) {
            fb_rrect(rx, ry, r->w, r->h, r->w / 2, col);
            continue;
        }
        if (r->kind == LR_IMG) {
            const unsigned int *src = (r->img >= 0) ? png_pixels(r->img) : 0;
            if (src && r->w > 0 && r->h > 0) {
                fb_image(rx, ry, r->w, r->h, src,
                         png_w(r->img), png_h(r->img));
                continue;
            }
            /* not decoded, or not arrived: an honest empty frame, not a
             * broken picture. The box is the size layout already reserved, so
             * nothing moves when the picture lands. */
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

static int in_box(int cx, int cy, int bx, int by, int bw, int bh)
{
    return cx >= bx && cx < bx + bw && cy >= by && cy < by + bh;
}

/* THE PRESS EDGE HAS TO BE FOUND HERE, because wm.c does not hand one over.
 * route_mouse delivers EV_MOUSE three times per click - on the press, on every
 * motion sample while the button is held (the GRAB_APP path, which exists so a
 * slider keeps working when the pointer leaves it), and again on the release
 * with an empty mask. The app that wants "a click" has to difference the mask
 * itself. Without this every link navigated on the press AND on the release,
 * pushing the same page into the history twice.
 *
 * `last_btn` is wm.c's name for the same idea, one layer up; this is not a
 * second copy of that state because wm.c's is about grabs and this one is
 * about clicks, and the two are consumed by different code. */
static int click_btn;

/* Clicking the chrome. THE URL BAR HAD NO HIT TEST AT ALL, and that is the
 * whole of the bug that made hand-testing impossible: clicking it did nothing,
 * so the bar was still unfocused when typing started, every character fell
 * through browser_key's shortcut switch and was dropped - until the first `l`,
 * which IS the focus shortcut. It was swallowed arming select-all, and the
 * character after it cleared the buffer. Typing
 * "https://en.wikipedia.org/wiki/Linux" therefore left exactly "inux": the
 * tail after the string's first `l`, which in that URL is the L of Linux.
 *
 * That is why the survivors looked like "everything after the first SHIFTED
 * character" - a coincidence of that one URL, and a diagnosis that would have
 * sent the fix into the keyboard layer, where nothing is wrong. */
int browser_click(int cx, int cy, int btn)
{
    int down = (btn & 1) && !(click_btn & 1);
    click_btn = btn;
    if (!down) return 0;

    if (in_box(cx, cy, back_x, back_y, back_w, back_h)) {
        url_focus = 0; url_sel_all = 0;
        browser_back();
        return 1;              /* repaint regardless: the focus ring moved */
    }

    /* A click in the bar focuses it AND selects all, which is what every
     * browser does and what the keyboard shortcut already did. */
    if (in_box(cx, cy, bar_x, bar_y, bar_w, bar_h)) {
        url_focus = 1;
        url_sel_all = 1;
        return 1;
    }

    /* Anywhere else DEFOCUSES. Not politeness: with the bar focused every key
     * is text, so a bar that keeps focus after you click into the page leaves
     * PgDn and the arrow keys dead with nothing on screen saying why. */
    int was = url_focus;
    url_focus = 0; url_sel_all = 0;

    int n = browser_link_at(cx, cy);
    if (n < 0) return was;
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
const char *browser_title(void);

/* The title from <title>, as a nul-terminated string the window frame can use.
 * html.c hands out lengths rather than C strings - the arena holds no
 * terminators - so the copy happens here, once, into a fixed buffer. */
static char title_buf[64];

/* The reason the status line already shows, for a gate that needs to report
 * WHERE a fetch stopped rather than only that it did. */
const char *browser_why(void) { return status_text(); }

const char *browser_title(void)
{
    int len;
    const char *s = html_title(&len);
    if (len > (int)sizeof title_buf - 1) len = (int)sizeof title_buf - 1;
    for (int i = 0; i < len; i++) title_buf[i] = s[i];
    title_buf[len] = 0;
    return len ? title_buf : "untitled";
}
