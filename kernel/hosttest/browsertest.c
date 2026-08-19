/* browsertest.c - the browser app's logic, with no pixels and no machine.
 *
 * browsershot renders browser.c and asserts NOTHING about it. Everything the
 * app does that is not drawing has been untested: URL parsing, which is a
 * trust boundary taking whatever a user types; the history stack, which is
 * eight fixed slots and a cap; and the URL bar's key state machine, which
 * already shipped one bug that appended what you typed to what was already
 * there and then blamed the DNS.
 *
 * THE DRAWING IS STUBBED, NOT LINKED. browser.c reaches fb.c for pixels and
 * ui.c for the theme; replacing both with a dozen one-line stubs keeps this
 * harness a few milliseconds and means it can run anywhere. What is under test
 * is the logic, and the logic never needed a framebuffer.
 *
 * THE NETWORK IS REAL, though - net.c, tcp.c and http.c are all linked, with a
 * fake link at the bottom that captures frames. So "did it parse the port"
 * is answered by looking at the SYN that went out, rather than by exposing an
 * accessor that exists only for the test.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../ui.h"
#include "../net.h"

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

/* ---- browser.c's interface -------------------------------------------------- */
void browser_home(void);
void browser_load_mem(unsigned int addr, int len);
void browser_load(const char *src, int len);
void browser_go(const char *u, int len);
int  browser_back(void);
int  browser_can_back(void);
int  browser_key(int code);
int  browser_url_focus(void);
int  browser_status(void);
int  browser_truncated(void);
int  browser_doc_cap(void);
int  browser_tick(void);
int  browser_scroll(void);
int  browser_scroll_by(int d);
int  browser_height(void);
int  browser_link_at(int cx, int cy);
void browser_draw(int x, int y, int w, int h, int focused);
const char *browser_title(void);

/* browser.c's private status codes, restated. They are not exported, and a
 * test that only checked "did it fail" would not distinguish "https refused"
 * from "no resolver" - which is exactly the distinction the app makes. */
#define BR_OK        0
#define BR_NO_NET    1
#define BR_NO_TLS    2
#define BR_NO_DNS    3
#define BR_FETCHING  4
#define BR_FAILED    5
#define BR_BAD_TYPE  6

/* ---- the clock and the drawing, stubbed ------------------------------------- */
static unsigned v_ticks;
unsigned int idt_ticks(void) { return v_ticks; }
unsigned long long cpu_tsc(void) { return 0; }
unsigned int cpu_tsc_khz(void)   { return 0; }

static struct ui_theme theme;
const struct ui_theme *ui_theme(void) { return &theme; }
void ui_theme_set(const struct ui_theme *t) { theme = *t; }
void ui_theme_init(int scale)
{
    memset(&theme, 0, sizeof theme);
    theme.scale = scale < 1 ? 1 : scale;
    theme.pad = 12; theme.gap = 8; theme.row_h = 24;
    theme.radius = 5; theme.title_h = 28;
}

void fb_fill_px(int x, int y, int w, int h, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)rgb; }
void fb_rrect(int x, int y, int w, int h, int r, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)r;(void)rgb; }
void fb_box(int x, int y, int w, int h, unsigned int rgb)
{ (void)x;(void)y;(void)w;(void)h;(void)rgb; }
void fb_clip(int x, int y, int w, int h) { (void)x;(void)y;(void)w;(void)h; }
void fb_clip_none(void) { }
void fb_text_prop(int px, int py, const char *s, unsigned int fg)
{ (void)px;(void)py;(void)s;(void)fg; }
int  fb_text_prop_h(void) { return 16; }
int  fb_prop_em(void)     { return 16; }
int  fb_text_prop_w(const char *s) { int n=0; while (s[n]) n++; return n * 8; }
/* half an em per character, so every expected width can be worked out by hand */
int  fb_text_rich_w(const char *s, int len, int size, int style)
{ (void)s;(void)style; return len * size / 2; }
void fb_text_rich(int px, int py, const char *s, int len, unsigned int fg,
                  int size, int style)
{ (void)px;(void)py;(void)s;(void)len;(void)fg;(void)size;(void)style; }

/* ---- the fake link ----------------------------------------------------------- */
#define CAP 32
static unsigned char frames[CAP][1600];
static int frame_len[CAP], nframes;

static int fake_send(const net_u8 *f, int len)
{
    if (nframes < CAP && len > 0 && len < 1600) {
        memcpy(frames[nframes], f, len);
        frame_len[nframes] = len;
        nframes++;
    }
    return 1;
}
static int fake_poll(net_u8 *out, int max) { (void)out; (void)max; return 0; }

/* hands over one seeded frame, then behaves like fake_poll */
static unsigned char seed_frame[42];
static int seed_served;
static int seed_poll(net_u8 *out, int max)
{
    (void)max;
    if (seed_served) return 0;
    seed_served = 1;
    memcpy(out, seed_frame, 42);
    return 42;
}

static const unsigned char OUR_MAC[6] = { 0x52,0x54,0x00,0x12,0x34,0x56 };
#define OUR_IP 0x0A00020Fu
#define GW_IP  0x0A000202u

static void net_up(void)
{
    net_link(fake_send, fake_poll, OUR_MAC);
    net_config(OUR_IP, 0xFFFFFF00u, GW_IP);
    /* seed the ARP cache by handing net.c an ARP reply from the gateway, so a
     * fetch can actually get as far as sending a SYN */
    unsigned char a[42];
    memset(a, 0, sizeof a);
    memcpy(a, OUR_MAC, 6);
    a[6]=0x52; a[7]=0x55; a[8]=0x0A; a[9]=0x00; a[10]=0x02; a[11]=0x02;
    a[12]=0x08; a[13]=0x06;
    a[15]=1; a[16]=0x08; a[18]=6; a[19]=4; a[21]=2;
    memcpy(a+22, a+6, 6);
    a[28]=10; a[29]=0; a[30]=2; a[31]=2;
    memcpy(a+32, OUR_MAC, 6);
    a[38]=10; a[39]=0; a[40]=2; a[41]=15;
    /* net.c has no "inject" entry point, so drive it through its own poll with
     * a link that hands over this one frame and then goes quiet.
     *
     * AND DO NOT CALL net_link AGAIN AFTERWARDS. It clears the ARP cache -
     * correctly, a new link is a new segment - so re-installing the plain
     * link after seeding threw away the entry that had just been learned, and
     * every fetch then failed to send anything at all. */
    memcpy(seed_frame, a, 42);
    seed_served = 0;
    net_link(fake_send, seed_poll, OUR_MAC);
    net_config(OUR_IP, 0xFFFFFF00u, GW_IP);
    net_poll_once();
}

/* the last TCP SYN we sent: destination address and port */
static int last_syn(unsigned *ip, int *port)
{
    for (int i = nframes - 1; i >= 0; i--) {
        unsigned char *f = frames[i];
        if (frame_len[i] < 14 + 20 + 20) continue;
        if (f[12] != 0x08 || f[13] != 0x00) continue;
        unsigned char *h = f + 14;
        if (h[9] != 6) continue;
        unsigned char *t = h + (h[0] & 0x0F) * 4;
        if (!(t[13] & 0x02)) continue;              /* not a SYN */
        *ip = ((unsigned)h[16]<<24)|((unsigned)h[17]<<16)|((unsigned)h[18]<<8)|h[19];
        *port = (t[2] << 8) | t[3];
        return 1;
    }
    return 0;
}

/* The browser is a single global with no teardown - which is right for a
 * kernel that has one of them, and means a test has to unwind the history
 * itself rather than assume a fresh one. Forgetting that made every history
 * assertion fail against perfectly good code. */
static void reset(void)
{
    for (int i = 0; i < 40 && browser_can_back(); i++) browser_back();
    nframes = 0;
    v_ticks = 1000;
    ui_theme_init(1);
    browser_home();
}

static void go(const char *u) { browser_go(u, (int)strlen(u)); }

/* ---- the fresh browser --------------------------------------------------------
 * MUST RUN FIRST. The browser is a single global with no teardown, so "what
 * does it look like before anything has happened" is observable exactly once
 * per process. Every property that depends on an empty history has to be
 * asserted here or not at all - and a test that quietly cannot observe the
 * property it names is worse than no test, because it reports PASS.
 */
static void t_fresh(void)
{
    printf("a browser that has done nothing yet\n");
    ui_theme_init(1);
    CHECK(!browser_can_back(), "Back is enabled before anything has happened");

    browser_home();
    CHECK(!browser_can_back(),
          "Back is enabled with only the home page in the history");

    net_up();
    go("http://10.0.2.2:8000/one");
    /* THIS is what makes about:home a real history entry rather than a special
     * case: after exactly one navigation from a fresh browser, Back must be
     * able to return to where it started. */
    CHECK(browser_can_back(),
          "after one navigation Back is still disabled - the home page was "
          "never pushed, so there is nowhere to go back TO");
    CHECK(browser_back(), "Back failed");
    CHECK(!strcmp(browser_title(), "zlOS"),
          "Back landed on '%s', not the home page", browser_title());
}

/* ---- URL parsing -------------------------------------------------------------
 * The one place in the browser that takes whatever a person typed.
 */
static void t_urls(void)
{
    printf("URL parsing\n");
    reset();

    /* https is refused BY NAME, not by failing to connect */
    go("https://example.com/");
    CHECK(browser_status() == BR_NO_TLS, "https gave status %d", browser_status());
    go("https://10.0.2.2/");
    CHECK(browser_status() == BR_NO_TLS, "https to an address gave %d",
          browser_status());

    /* a NAME is refused by name too - there is no resolver, and failing to
     * connect would blame the wrong thing */
    go("http://example.com/");
    CHECK(browser_status() == BR_NO_DNS, "a hostname gave status %d", browser_status());
    go("example.com");
    CHECK(browser_status() == BR_NO_DNS, "a bare hostname gave %d", browser_status());

    /* malformed addresses are not addresses */
    const char *bad[] = { "http://", "http:///path", "http://999.1.1.1/",
                          "http://10.0.2/", "http://10.0.2.2.5/",
                          "http://1.2.3.4.5.6/", "http://10.0.2.-1/",
                          "http://10..2.2/", "", "/", ":", "://", 0 };
    for (int i = 0; bad[i]; i++) {
        go(bad[i]);
        CHECK(browser_status() != BR_OK && browser_status() != BR_FETCHING,
              "'%s' was accepted as an address (status %d)",
              bad[i], browser_status());
    }

    /* a 256 in any octet is not a dotted quad */
    go("http://10.0.2.256/");
    CHECK(browser_status() == BR_NO_DNS || browser_status() == BR_FAILED,
          "10.0.2.256 was accepted (status %d)", browser_status());

    /* an absurdly long URL must not run off the buffer */
    static char longu[4096];
    memcpy(longu, "http://10.0.2.2/", 16);
    memset(longu + 16, 'a', sizeof longu - 17);
    longu[sizeof longu - 1] = 0;
    go(longu);
    CHECK(1, "a 4 KB URL did not fault");
}

static void t_url_targets(void)
{
    printf("what the address actually resolves to\n");
    /* With the network up, a valid URL must produce a SYN to the right place.
     * Checking the SYN rather than an accessor means the test cannot pass
     * while the value never reaches the socket. */
    reset();
    net_up();

    nframes = 0;
    go("http://10.0.2.2:8000/index.html");
    unsigned ip = 0; int port = 0;
    CHECK(last_syn(&ip, &port), "no SYN was sent for a valid URL");
    CHECK(ip == GW_IP, "SYN went to %08X, wanted %08X", ip, GW_IP);
    CHECK(port == 8000, "SYN went to port %d, wanted 8000", port);

    /* no port means 80 */
    reset(); net_up(); nframes = 0;
    go("http://10.0.2.2/");
    CHECK(last_syn(&ip, &port), "no SYN for a URL without a port");
    CHECK(port == 80, "default port is %d, wanted 80", port);

    /* the scheme is optional */
    reset(); net_up(); nframes = 0;
    go("10.0.2.2:8080/x");
    CHECK(last_syn(&ip, &port), "no SYN for a URL with no scheme");
    CHECK(port == 8080, "port %d, wanted 8080", port);

    /* an out-of-range port must not be taken */
    reset(); net_up(); nframes = 0;
    go("http://10.0.2.2:99999/");
    if (last_syn(&ip, &port))
        CHECK(port == 80, "port 99999 was accepted as %d", port);

    /* every octet must reach the address, not just the first */
    reset(); net_up(); nframes = 0;
    go("http://10.0.2.2/");
    CHECK(last_syn(&ip, &port) && ip == 0x0A000202u,
          "address parsed to %08X", ip);
}

/* ---- history ------------------------------------------------------------------ */
static void t_history(void)
{
    printf("history and Back\n");
    reset();

    /* about:home is a real entry, or Back can never return to where the
     * browser started - which is not "Back is disabled", it is "Back silently
     * does nothing", and those look identical on screen */
    CHECK(!browser_can_back(), "Back is enabled with only the home page");

    net_up();
    go("http://10.0.2.2:8000/one");
    CHECK(browser_can_back(), "Back is not enabled after navigating");
    CHECK(browser_back(), "Back failed");
    CHECK(!browser_can_back(), "Back is still enabled at the bottom of the stack");
    CHECK(!browser_back(), "Back succeeded with nowhere to go");

    /* the cap: eight slots, and the ninth push drops the oldest rather than
     * overrunning the array */
    reset(); net_up();
    for (int i = 0; i < 30; i++) {
        char u[64];
        snprintf(u, sizeof u, "http://10.0.2.2:8000/p%d", i);
        go(u);
    }
    int depth = 0;
    while (browser_can_back() && depth < 100) { browser_back(); depth++; }
    CHECK(depth > 0 && depth <= 8, "history unwound %d deep, cap is 8", depth);
    CHECK(!browser_can_back(), "the stack never emptied");
}

/* ---- the URL bar's key machine ------------------------------------------------ */
static void t_urlbar(void)
{
    printf("the URL bar\n");
    reset();

    CHECK(!browser_url_focus(), "the URL bar starts focused");
    browser_key('l');
    CHECK(browser_url_focus(), "'l' did not focus the URL bar");

    /* FOCUS SELECTS ALL and the first keystroke replaces it. The bar is
     * pre-filled with the current address, and appending to it produced
     * "about:homehttp://..." - which then failed to parse and reported "no
     * resolver", a message about entirely the wrong thing. */
    for (const char *p = "http://10.0.2.2:8000/"; *p; p++) browser_key(*p);
    net_up();
    nframes = 0;
    browser_key(13);                       /* Enter */
    CHECK(!browser_url_focus(), "Enter left the bar focused");
    unsigned ip = 0; int port = 0;
    CHECK(last_syn(&ip, &port), "Enter did not start a fetch");
    CHECK(port == 8000, "the typed URL parsed to port %d - the old text was kept",
          port);

    /* Esc abandons */
    reset();
    browser_key('l');
    browser_key('x');
    browser_key(27);
    CHECK(!browser_url_focus(), "Esc did not unfocus");

    /* backspace as the first key after focus clears the selection and does not
     * then also delete a character from the empty buffer */
    reset(); net_up();
    browser_key('l');
    browser_key(8);
    for (const char *p = "10.0.2.2:1234/"; *p; p++) browser_key(*p);
    nframes = 0;
    browser_key(13);
    CHECK(last_syn(&ip, &port) && port == 1234,
          "after a leading backspace the URL parsed to port %d", port);

    /* A KEY THAT IS NOT PRINTABLE MUST NOT ENTER THE BUFFER.
     *
     * The first printable keystroke clears the selection, so control codes
     * sent BEFORE it are wiped either way and the test cannot see them. Type
     * a real character first, so the selection is already gone and anything
     * that follows lands in the buffer for good if it is not filtered. */
    reset(); net_up();
    browser_key('l');
    browser_key('1');                       /* clears the selection */
    browser_key(1); browser_key(200); browser_key(0x112); browser_key(7);
    for (const char *p = "0.0.2.2:99/"; *p; p++) browser_key(*p);
    nframes = 0;
    browser_key(13);
    CHECK(last_syn(&ip, &port) && port == 99,
          "control codes reached the URL buffer (port %d, status %d)",
          port, browser_status());

    /* more characters than the buffer holds must not overrun it */
    reset();
    browser_key('l');
    for (int i = 0; i < 4000; i++) browser_key('a');
    browser_key(27);
    CHECK(1, "a 4000-character URL bar entry did not fault");

    /* while the bar has focus, scroll keys must NOT scroll - they are text */
    reset();
    browser_draw(0, 0, 400, 300, 1);
    browser_scroll_by(1000);
    int before = browser_scroll();
    browser_key('l');
    browser_key(' ');
    CHECK(browser_scroll() == before, "space scrolled while typing a URL");
}

/* ---- documents and scrolling --------------------------------------------------- */
static void t_document(void)
{
    printf("documents and scrolling\n");
    reset();
    browser_draw(0, 0, 400, 300, 1);
    CHECK(browser_height() > 0, "the home page laid out to nothing");
    CHECK(!strcmp(browser_title(), "zlOS"), "title '%s'", browser_title());

    /* scrolling is clamped at both ends */
    browser_scroll_by(-1000);
    CHECK(browser_scroll() == 0, "scrolled above the top (%d)", browser_scroll());
    browser_scroll_by(1000000);
    browser_draw(0, 0, 400, 300, 1);
    CHECK(browser_scroll() <= browser_height(),
          "scrolled past the end (%d of %d)", browser_scroll(), browser_height());

    /* a document larger than the buffer is truncated AND says so. Sized from
     * the REAL cap, not a literal - see browser_doc_cap(). */
    {
        int cap = browser_doc_cap();
        int bign = cap + 4096;
        char *big = malloc((size_t)bign);
        CHECK(big != 0, "could not allocate the oversize document");
        if (big) {
            memset(big, 'x', (size_t)bign);
            memcpy(big, "<html><body><p>", 15);
            browser_load(big, bign);
            CHECK(browser_truncated() == 1,
                  "a %d-byte document (cap %d) was not flagged as truncated", bign, cap);
            free(big);
        }
    }
    browser_draw(0, 0, 400, 300, 1);
    CHECK(browser_height() > 0, "the truncated document laid out to nothing");

    /* a zero-length or null document falls back to the home page rather than
     * rendering nothing at all */
    browser_load(0, 0);
    browser_draw(0, 0, 400, 300, 1);
    CHECK(browser_height() > 0, "a null document left an empty window");
    browser_load("", 0);
    browser_draw(0, 0, 400, 300, 1);
    CHECK(browser_height() > 0, "a null document left an empty window");

    /* a hit test outside any link */
    CHECK(browser_link_at(-100, -100) == -1, "a link was found outside the window");
}

int main(void)
{
    printf("browser.c's logic, no pixels and no machine\n\n");
    t_fresh();
    t_urls();
    t_url_targets();
    t_history();
    t_urlbar();
    t_document();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
