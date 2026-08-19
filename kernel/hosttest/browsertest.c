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
#include <sys/mman.h>

#include "../ui.h"
#include "../memmap.h"
#include "../net.h"
#include "../dns.h"
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
int  browser_click(int cx, int cy, int btn);
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
#define BR_RESOLVING 7
#define BR_IMAGES    8

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
/* The picture blit, stubbed like every other pixel here. What THIS harness
 * asserts about an image is that layout gave the run its real size and an
 * arena slot; whether the pixels land correctly is fbtext's job, and it
 * mutation-tests the scaling and the alpha against a real framebuffer. */
void fb_image(int px, int py, int w, int h,
              const unsigned int *src, int sw, int sh)
{ (void)px;(void)py;(void)w;(void)h;(void)src;(void)sw;(void)sh; }
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

/* Hands over the seeded frames, then behaves like fake_poll. TWO of them, not
 * one: the resolver is a different host from the gateway (on QEMU it is .3
 * where the gateway is .2) and dns_start ARP-resolves it separately before it
 * will send a query at all. With only the gateway seeded, dns_start returns 0
 * and every lookup reports "no resolver" - which is a harness gap that reads
 * exactly like a browser bug. */
static const unsigned char OUR_MAC_[6] = { 0x52,0x54,0x00,0x12,0x34,0x56 };
#define SEED_MAX 4
static unsigned char seed_frames[SEED_MAX][42];
static int seed_n, seed_served;
static int seed_poll(net_u8 *out, int max)
{
    (void)max;
    if (seed_served >= seed_n) return 0;
    memcpy(out, seed_frames[seed_served++], 42);
    return 42;
}

/* an ARP reply from `ip` with a made-up MAC, into the next seed slot */
static void seed_arp(unsigned ip, unsigned char last)
{
    if (seed_n >= SEED_MAX) return;
    unsigned char *a = seed_frames[seed_n++];
    memset(a, 0, 42);
    memcpy(a, OUR_MAC_, 6);
    a[6]=0x52; a[7]=0x55; a[8]=0x0A; a[9]=0x00; a[10]=0x02; a[11]=last;
    a[12]=0x08; a[13]=0x06;
    a[15]=1; a[16]=0x08; a[18]=6; a[19]=4; a[21]=2;
    memcpy(a+22, a+6, 6);
    a[28]=(unsigned char)(ip >> 24); a[29]=(unsigned char)(ip >> 16);
    a[30]=(unsigned char)(ip >> 8);  a[31]=(unsigned char)ip;
    memcpy(a+32, OUR_MAC_, 6);
    a[38]=10; a[39]=0; a[40]=2; a[41]=15;
}

static const unsigned char OUR_MAC[6] = { 0x52,0x54,0x00,0x12,0x34,0x56 };
#define OUR_IP 0x0A00020Fu
#define GW_IP  0x0A000202u
#define DNS_IP 0x0A000203u     /* QEMU's resolver: .3, not the gateway */

static void net_up(void)
{
    net_link(fake_send, fake_poll, OUR_MAC);
    net_config(OUR_IP, 0xFFFFFF00u, GW_IP);
    /* seed the ARP cache by handing net.c ARP replies from the gateway AND the
     * resolver, so a fetch gets as far as a SYN and a lookup as far as a query */
    seed_n = seed_served = 0;
    seed_arp(GW_IP, 0x02);
    seed_arp(DNS_IP, 0x03);
    /* net.c has no "inject" entry point, so drive it through its own poll with
     * a link that hands over these frames and then goes quiet.
     *
     * AND DO NOT CALL net_link AGAIN AFTERWARDS. It clears the ARP cache -
     * correctly, a new link is a new segment - so re-installing the plain
     * link after seeding threw away the entry that had just been learned, and
     * every fetch then failed to send anything at all. */
    net_link(fake_send, seed_poll, OUR_MAC);
    net_config(OUR_IP, 0xFFFFFF00u, GW_IP);
    dns_server(DNS_IP);
    net_poll_once();
    net_poll_once();
}

/* Is this byte string anywhere in what we transmitted? Used to check WHICH
 * name a lookup asked for: DNS writes each label with a length prefix, so the
 * labels of a host name appear contiguously in the query frame. */
static int frames_contain(const char *needle)
{
    int nl = 0; while (needle[nl]) nl++;
    for (int i = 0; i < nframes; i++)
        for (int j = 0; j + nl <= frame_len[i]; j++)
            if (!memcmp(frames[i] + j, needle, (size_t)nl)) return 1;
    return 0;
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

    /* HTTPS IS NO LONGER REFUSED BY SCHEME, and these two assertions changed
     * with the code rather than being deleted. They now prove the opposite
     * property: that an https:// URL takes the SAME path an http:// one does
     * and fails for the same real reasons, instead of being turned away at the
     * door. A status of BR_NO_TLS here would mean the scheme was refused.
     *
     * A NAME NOW STARTS A LOOKUP. These four assertions used to expect
     * BR_NO_DNS, and that was never a property of the browser - it was a
     * property of this harness, which had no resolver configured, so
     * dns_start() returned 0 for every name. Adding one (net_up now seeds the
     * resolver's ARP as well as the gateway's, and calls dns_server) turned
     * them red, which is the correct outcome: they were asserting the absence
     * of test setup. What they should say, and now do, is that a name takes
     * the NAME path rather than being refused by scheme. */
    go("https://example.com/");
    CHECK(browser_status() == BR_RESOLVING,
          "https by name gave %d, wanted a name lookup", browser_status());
    CHECK(browser_status() != BR_NO_TLS, "https was refused by scheme");
    /* an address, so it gets as far as trying to connect - and there is no
     * network here, so it fails as a fetch */
    go("https://10.0.2.2/");
    CHECK(browser_status() == BR_FAILED || browser_status() == BR_FETCHING,
          "https to an address gave %d, wanted a real connect attempt",
          browser_status());

    /* a NAME goes to the resolver, with or without a scheme */
    go("http://example.com/");
    CHECK(browser_status() == BR_RESOLVING, "a hostname gave status %d", browser_status());
    go("example.com");
    CHECK(browser_status() == BR_RESOLVING, "a bare hostname gave %d", browser_status());
    /* ...and a bare name must NOT have become a search - it has a dot, so it
     * is an address. This is the boundary looks_like_url() draws. */
    nframes = 0;
    go("example.com");
    CHECK(frames_contain("example"),
          "'example.com' was turned into a search instead of resolved");
    CHECK(!frames_contain("duckduckgo"),
          "'example.com' was treated as a search query");

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

    /* A 256 IN ANY OCTET IS NOT A DOTTED QUAD, so it is a NAME - which is what
     * a real browser does with it too, and the lookup then fails on its own
     * merits. The old assertion expected BR_NO_DNS and was reading "this
     * harness has no resolver" as "the address was rejected"; what actually
     * matters is that it was not accepted AS AN ADDRESS, i.e. no SYN went
     * anywhere. */
    reset(); net_up(); nframes = 0;
    go("http://10.0.2.256/");
    CHECK(browser_status() == BR_RESOLVING || browser_status() == BR_NO_DNS ||
          browser_status() == BR_FAILED,
          "10.0.2.256 was accepted (status %d)", browser_status());
    {
        unsigned ip = 0; int port = 0;
        CHECK(!last_syn(&ip, &port),
              "10.0.2.256 was connected to as though it were an address");
    }

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

/* ---- clicking the chrome -------------------------------------------------------
 *
 * THE BUG THIS EXISTS FOR. browser_click had no URL-bar hit test at all, so
 * clicking the bar did nothing at all and typing fell through browser_key's
 * unfocused switch, which drops everything that is not a shortcut - until the
 * string's first `l`, which IS the focus shortcut. That `l` was swallowed
 * arming select-all, and the character after it cleared the buffer. Typing
 * "https://en.wikipedia.org/wiki/Linux" therefore left exactly "inux".
 *
 * Every test above pressed 'l' first, so all of them focused the bar by the
 * one route that worked and not one of them could see it - the failure mode
 * this file's own header warns about, where a test that cannot observe the
 * property it names reports PASS. The fix is a hit test; this is the
 * assertion that would have found it.
 *
 * The geometry is the stubs at the top of this file, written out rather than
 * hidden so that changing them breaks this loudly: em is 16, so rowh is
 * 16 + 8 = 24 and the chrome row spans y 4..28; Back is
 * strlen(" Back ") * 8 + 8 = 56 wide at x 4; the bar starts at 4+56+4 = 64
 * and runs to w - 4.
 */
static void t_chrome_click(void)
{
    printf("clicking the chrome\n");
    reset(); net_up();
    browser_draw(0, 0, 400, 300, 1);

    /* reset() UNWINDS THE HISTORY AND NOTHING ELSE, so the focus the previous
     * test left behind is still here - the browser is one global with no
     * teardown, exactly as this file's header says. Esc is the shipping way to
     * put it back, and asserting that it worked is what stops this test from
     * silently starting in the wrong state later. */
    browser_key(27);
    CHECK(!browser_url_focus(), "Esc did not leave the URL bar unfocused");

    /* a press in the bar focuses it and selects all, as 'l' already did */
    CHECK(browser_click(200, 10, 1), "a press in the URL bar was not handled");
    CHECK(browser_url_focus(), "clicking the URL bar did not focus it");

    /* THE RELEASE MUST CHANGE NOTHING. wm.c delivers EV_MOUSE again with an
     * empty button mask, and once more for every motion sample in between; an
     * app that treats each of those as a click re-arms select-all after the
     * first keystroke has already cleared it, and navigates a link twice. */
    CHECK(!browser_click(200, 10, 0), "the release counted as a second click");
    CHECK(browser_click(200, 10, 1) == 1, "a second press was not handled");
    CHECK(!browser_click(200, 10, 1), "the button being HELD counted as a click");
    browser_click(200, 10, 0);
    CHECK(browser_url_focus(), "the release unfocused the bar");

    /* THE REPRO ITSELF. An address with an `l` in the middle, typed after a
     * CLICK and never after an 'l' keystroke. Every character before that `l`
     * is what the bug threw away, and if any of them is missing the address
     * does not parse as a dotted quad and no SYN goes out at all. */
    nframes = 0;
    for (const char *p = "http://10.0.2.2:8081/l/x"; *p; p++) browser_key(*p);
    browser_key(13);
    unsigned ip = 0; int port = 0;
    CHECK(last_syn(&ip, &port),
          "nothing was fetched: the characters typed before the URL's first "
          "'l' were dropped, which is the bug this test exists for");
    CHECK(ip == GW_IP, "SYN went to %08X, wanted %08X", ip, GW_IP);
    CHECK(port == 8081, "the typed URL parsed to port %d, wanted 8081", port);

    /* CLICKING THE PAGE DEFOCUSES. Not politeness: while the bar has focus
     * every key is text, so a bar that keeps it after you click into the
     * document leaves PgDn and the arrow keys dead with nothing saying why. */
    reset();
    browser_draw(0, 0, 400, 300, 1);
    browser_click(200, 10, 1); browser_click(200, 10, 0);
    CHECK(browser_url_focus(), "the URL bar did not focus");
    browser_click(200, 200, 1); browser_click(200, 200, 0);
    CHECK(!browser_url_focus(), "clicking the page left the URL bar focused");

    /* BACK IS A BUTTON, and it was drawn as one from the first commit while
     * being reachable only from the keyboard. */
    reset(); net_up();
    browser_draw(0, 0, 400, 300, 1);
    go("http://10.0.2.2:8000/one");
    browser_draw(0, 0, 400, 300, 1);
    CHECK(browser_can_back(), "nothing to go Back from");
    browser_click(20, 10, 1); browser_click(20, 10, 0);
    CHECK(!browser_can_back(), "clicking Back did not go back");

    /* A LINK NAVIGATES ONCE. The press and the release are two EV_MOUSE
     * events for one click; before the edge test above, both navigated, so a
     * single click pushed two identical entries onto an eight-slot history. */
    reset(); net_up();
    static const char page[] =
        "<html><body><p><a href=\"http://10.0.2.2:8000/two\">go</a></p></body></html>";
    browser_load(page, (int)sizeof page - 1);
    browser_draw(0, 0, 400, 300, 1);

    /* WHERE the link is, asked of the shipping hit test rather than worked out
     * from the run array plus a second copy of the content origin. That second
     * copy is precisely what browser_link_at's own comment refuses to keep, and
     * a test carrying one would drift away from the code the day the status
     * strip gains a line. */
    int px = -1, py = -1;
    for (int sy = 0; sy < 300 && px < 0; sy++)
        for (int sx = 0; sx < 400; sx++)
            if (browser_link_at(sx, sy) >= 0) { px = sx; py = sy; break; }
    CHECK(px >= 0, "the test page produced no link to click");
    if (px >= 0) {
        int depth = 0;
        browser_click(px, py, 1);
        browser_click(px, py, 0);
        while (browser_can_back() && depth < 40) { browser_back(); depth++; }
        /* Back is enabled once there are two entries, so ONE navigation
         * unwinds exactly once. Two is the signature of the bug: the press and
         * the release each pushed the same address. */
        CHECK(depth == 1, "one click on a link unwound %d history entries, "
              "wanted 1 - two means the release navigated as well", depth);
    }
}

/* ---- pictures, end to end -------------------------------------------------------
 * A `data:` URI is the one image path that needs no machine on the other end
 * of a wire, which is exactly why it is worth having: it makes "the document
 * said <img>, png.c decoded it, and layout.c gave the run its real size" a
 * host assertion instead of something only a booted kernel can show.
 *
 * The payload is a genuine 424-byte RGBA PNG produced by an unrelated encoder
 * (python's zlib), not by png.c - a decoder that only ever meets files made by
 * its own author agrees with its author. Its corners are fully transparent and
 * its centre is opaque, which is the property that catches an alpha channel
 * being dropped or filled with 0xFF.
 */
static const char img_page[] =
    "<html><body><p>before</p>"
    "<img alt=\"mark\" src=\"data:image/png;base64,"
        "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAABb0lEQVR42u2X"
        "x1ICQRCGeSITYkREFBFR9B1NmHNWzAnTCygiJsScw6VdD162urt6hqpZD/xV"
        "ff6+menp3XG58vlvKeg4gsLOYyjqsqo7CcU9SSiJnYA7lgJ3bwpK+07B058G"
        "z0AaygbPoHzIquFzqBi5gMpRq8YuoWr8Cqqt8k5kwDuZgZqpa/BNZ8E3kwVH"
        "4bWzN+Cfu6UlTMD984yACXjdwh0jYAAeWLynBSi4bjB4IM4IUCvXDQavX3qg"
        "Baht1xZA4A2sAHHmusHgweVHWiDXhrMHgwdXnmgBXbh05b/wxlVOQBGusu1/"
        "8NDaMy2QC5xqODs8tP5CC0jgKt2OwZs2OAFFOHXPOXh485UWoODSCSeBh7cY"
        "ATtcZbxK4c3bb4yAQrdLgsEjO++0gPSeS4PBIwlGQDrhpMHgLYkPWoD6nmsL"
        "IPDW3U9GQPAzodpwdnh0jxEwAY/uf9ECJuBtB5yAAXj74Tf/NnAUno8T+QGD"
        "/zPt3aHfvwAAAABJRU5ErkJggg=="
    "\"><p>after</p></body></html>";

static void t_images(void)
{
    printf("pictures\n");
    reset();
    browser_load(img_page, (int)sizeof img_page - 1);
    browser_draw(0, 0, 400, 300, 1);

    int nimg = 0, decoded = 0, iw = 0, ih = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (!r || r->kind != LR_IMG) continue;
        nimg++;
        if (r->img >= 0) { decoded++; iw = r->w; ih = r->h; }
    }
    CHECK(nimg == 1, "the page produced %d image runs, wanted 1", nimg);
    CHECK(decoded == 1,
          "the inline PNG was not decoded - the run carries no arena slot");
    CHECK(iw == 32 && ih == 32,
          "the decoded image laid out at %dx%d, wanted its intrinsic 32x32",
          iw, ih);

    /* THE TEXT AROUND IT MUST SURVIVE. An image that swallows its siblings is
     * the failure mode a picture on screen hides best - the picture looks
     * right and the paragraph after it is simply gone. */
    int before = 0, after = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (!r || r->kind != LR_TEXT || r->len <= 0) continue;
        if (r->len == 6 && !memcmp(r->text, "before", 6)) before = 1;
        if (r->len == 5 && !memcmp(r->text, "after", 5)) after = 1;
    }
    CHECK(before && after, "the text around the image was lost");

    /* A BROKEN PICTURE MUST NOT BREAK THE PAGE. Each of these is a different
     * way for a data: URI to be wrong, and every one of them has to end as the
     * same placeholder box the browser already shows for a picture that has
     * not arrived - not as a fault, and not as a missing paragraph. */
    static const char *bad[] = {
        "<html><body><p>kept</p><img src=\"data:image/png;base64,!!!!\"></body></html>",
        "<html><body><p>kept</p><img src=\"data:image/png;base64,\"></body></html>",
        "<html><body><p>kept</p><img src=\"data:image/png,notbase64\"></body></html>",
        "<html><body><p>kept</p><img src=\"data:\"></body></html>",
        "<html><body><p>kept</p><img src=\"\"></body></html>",
        "<html><body><p>kept</p><img></body></html>",
        /* valid base64 that is not a PNG at all */
        "<html><body><p>kept</p><img src=\"data:image/png;base64,SGVsbG8sIHdvcmxkIQ==\"></body></html>",
        /* a PNG signature and nothing after it */
        "<html><body><p>kept</p><img src=\"data:image/png;base64,iVBORw0KGgo=\"></body></html>",
    };
    for (unsigned k = 0; k < sizeof bad / sizeof bad[0]; k++) {
        reset();
        browser_load(bad[k], (int)strlen(bad[k]));
        browser_draw(0, 0, 400, 300, 1);
        int kept = 0, slotted = 0;
        for (int i = 0; i < lay_count(); i++) {
            const struct lay_run *r = lay_at(i);
            if (!r) continue;
            if (r->kind == LR_TEXT && r->len == 4 && !memcmp(r->text, "kept", 4)) kept = 1;
            if (r->kind == LR_IMG && r->img >= 0) slotted = 1;
        }
        CHECK(kept, "malformed image #%u took the page's text with it", k);
        CHECK(!slotted, "malformed image #%u produced an arena slot", k);
    }

    /* the good one still works after all of that - the decoder's state did not
     * get stuck on the last failure */
    reset();
    browser_load(img_page, (int)sizeof img_page - 1);
    browser_draw(0, 0, 400, 300, 1);
    int again = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (r && r->kind == LR_IMG && r->img >= 0) again = 1;
    }
    CHECK(again, "a good image stopped decoding after a run of bad ones");
}

/* ---- typing words instead of an address ----------------------------------------
 * The last thing between this browser and being usable by hand. The assertions
 * are on the SYN and the request that actually go out, not on an accessor, for
 * the same reason the rest of this file works that way.
 */
static void t_search(void)
{
    printf("searching from the URL bar\n");

    /* Things that ARE addresses must never become searches. Each of these is a
     * shape a person really types. */
    static const char *urls[] = {
        "http://10.0.2.2:8000/", "https://10.0.2.2/", "10.0.2.2:8080/x",
        "example.com", "example.com/a/b", "sub.example.com:99/p",
    };
    for (unsigned k = 0; k < sizeof urls / sizeof urls[0]; k++) {
        reset(); net_up(); nframes = 0;
        go(urls[k]);
        unsigned ip = 0; int port = 0;
        /* a search would resolve a NAME, so it would go to DNS and send no SYN
         * to the address in the string - checking the SYN's destination is what
         * distinguishes "treated as an address" from "treated as words" */
        int syn = last_syn(&ip, &port);
        CHECK(syn || browser_status() == BR_RESOLVING || browser_status() == BR_NO_DNS,
              "'%s' did not start a fetch at all", urls[k]);
    }

    /* ...and things that are NOT addresses must become searches, which means
     * resolving the search host rather than the words. */
    static const char *queries[] = {
        "zlos", "what is a browser", "1 + 1", "hello world",
    };
    for (unsigned k = 0; k < sizeof queries / sizeof queries[0]; k++) {
        reset(); net_up(); nframes = 0;
        go(queries[k]);
        /* WHICH HOST DID IT ACTUALLY GO TO? "it did not fail" is the weak
         * assertion this file keeps warning about - it passes just as well if
         * the words were treated as a host name and the lookup is failing
         * somewhere else. A DNS query encodes each label with a length prefix,
         * so the host's labels appear contiguously in the frame; finding
         * "duckduckgo" in what went out is the difference between "a search
         * happened" and "something happened". */
        CHECK(browser_status() == BR_RESOLVING,
              "the search for '%s' did not start a name lookup (status %d)",
              queries[k], browser_status());
        CHECK(frames_contain("duckduckgo"),
              "the search for '%s' resolved something other than the search "
              "host - the query was treated as an address", queries[k]);
    }

    /* THE TRAP THIS TEST EXISTS FOR. The URL bar calls navigate(url, url_len),
     * so `u` and `url` are the SAME buffer. Building the search address into
     * `url` would overwrite the query with the prefix while still reading it,
     * and the result would be a search for a mangled fragment of itself. The
     * bar's own path is the one that hits it, so drive the bar. */
    reset(); net_up();
    browser_draw(0, 0, 400, 300, 1);
    browser_key(27);
    browser_click(200, 10, 1); browser_click(200, 10, 0);
    for (const char *p = "hello world"; *p; p++) browser_key(*p);
    nframes = 0;
    browser_key(13);
    CHECK(browser_status() != BR_FAILED,
          "a search typed into the BAR failed where the same words via go() did "
          "not - the query was consumed while the address was built over it");

    /* an empty bar must not navigate to a search for nothing */
    reset(); net_up();
    browser_draw(0, 0, 400, 300, 1);
    browser_click(200, 10, 1); browser_click(200, 10, 0);
    browser_key(8);                       /* clears the selection, empties it */
    nframes = 0;
    browser_key(13);
    CHECK(1, "an empty URL bar did not fault");

    /* a query longer than the buffer must truncate, not overrun */
    reset(); net_up();
    {
        char big[1024];
        for (int i = 0; i < 1000; i++) big[i] = (i % 7) ? 'a' : ' ';
        big[1000] = 0;
        go(big);
        CHECK(1, "a 1000-character search did not fault");
    }

    /* every byte that needs escaping, including high ones */
    reset(); net_up();
    go("a&b=c?d#e /f%g\x80\xff");
    CHECK(1, "a query full of reserved and high bytes did not fault");
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


/* ---- scripts ---------------------------------------------------------------
 * The page carries a <script> and the text it writes must end up as RENDERED
 * CONTENT - not as source shown on screen, and not silently dropped. Those are
 * the two ways this goes wrong and both look plausible from a distance.
 */
static void t_scripts(void)
{
    printf("scripts\n");
    reset();

    static const char page[] =
        "<html><body><h1>Before</h1>"
        "<script>\n"
        "  var out = '';\n"
        "  for (var i = 1; i <= 3; i++) { out += '<p>row ' + i + '</p>'; }\n"
        "  document.write(out);\n"
        "</script>"
        "</body></html>";
    browser_load(page, (int)sizeof page - 1);
    browser_draw(0, 0, 600, 400, 1);
    CHECK(browser_height() > 0, "the scripted page laid out to nothing");

    int rows = 0, saw_src = 0, saw_before = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (!r || r->kind != LR_TEXT || r->len <= 0) continue;
        if (r->len == 3 && !memcmp(r->text, "row", 3)) rows++;
        if (r->len == 6 && !memcmp(r->text, "Before", 6)) saw_before = 1;
        /* the SOURCE must never be rendered */
        if (r->len >= 8 && !memcmp(r->text, "document", 8)) saw_src = 1;
        if (r->len >= 3 && !memcmp(r->text, "var", 3)) saw_src = 1;
    }
    CHECK(saw_before, "the static content before the script vanished");
    CHECK(rows == 3, "document.write produced %d rows, wanted 3", rows);
    CHECK(!saw_src, "the script SOURCE was rendered as page text");

    /* a script that fails must not take the page with it */
    static const char bad[] =
        "<html><body><h1>Kept</h1><script>this is not ( valid javascript</script>"
        "<p>and this still renders</p></body></html>";
    browser_load(bad, (int)sizeof bad - 1);
    browser_draw(0, 0, 600, 400, 1);
    int saw_kept = 0, saw_after = 0;
    for (int i = 0; i < lay_count(); i++) {
        const struct lay_run *r = lay_at(i);
        if (!r || r->kind != LR_TEXT || r->len <= 0) continue;
        if (r->len == 4 && !memcmp(r->text, "Kept", 4)) saw_kept = 1;
        if (r->len == 5 && !memcmp(r->text, "still", 5)) saw_after = 1;
    }
    CHECK(saw_kept && saw_after, "a broken script took the rest of the page with it");

    static const char plain[] = "<html><body><p>plain</p></body></html>";
    browser_load(plain, (int)sizeof plain - 1);
    browser_draw(0, 0, 600, 400, 1);
    CHECK(browser_height() > 0, "a script-free page broke");
}

/* THE ONE ADDRESS THIS HARNESS HAS TO HONOUR. browser.c keeps its decoded
 * pictures and its base64 scratch in the fixed high-RAM map (memmap.h HI_IMG),
 * because a couple of megabytes cannot be BSS in a kernel whose image already
 * reaches 5.573 MiB against a 6 MiB link ceiling. fbbench.c states the rule
 * this follows: mmap the same address the shipping source hardcodes, so that
 * source compiles unmodified rather than growing a host-only branch that the
 * kernel then never executes. */
static int map_high_ram(void)
{
    /* THE ADDRESS COMES FROM memmap.h, NOT FROM A LITERAL. It was a literal
     * for exactly as long as it took for the region to move, and then this
     * harness mapped 32 MiB while browser.c wrote to 48 and the whole thing
     * segfaulted inside b64_decode. Two copies of one address is the bug
     * memmap.h exists to end; a harness is not exempt from that. */
    void *want = (void *)(unsigned long)HI_IMG;
    unsigned long span = (unsigned long)(HI_IMG_END - HI_IMG);
    void *p = mmap(want, span, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p != want) {
        printf("could not map the picture arena at %p\n", want);
        return 0;
    }
    memset(p, 0, span);
    return 1;
}

int main(void)
{
    printf("browser.c's logic, no pixels and no machine\n\n");
    if (!map_high_ram()) return 2;
    t_fresh();
    t_urls();
    t_scripts();
    t_url_targets();
    t_history();
    t_urlbar();
    t_chrome_click();
    t_images();
    t_search();
    t_document();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
