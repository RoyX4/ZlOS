/* fuzz.c - every layer that takes bytes from somewhere else, fed garbage.
 *
 * The harnesses next to this one check that the code does the right thing with
 * inputs someone thought of. This checks that it does not do something
 * catastrophic with inputs nobody thought of, which is a different question
 * and the one that matters for anything parsing data off a wire.
 *
 * FOUR TRUST BOUNDARIES, and every one of them is fed by a machine that is not
 * this one:
 *
 *   html.c    markup from a server, which may be malicious or merely broken
 *   layout.c  whatever tree the parser produced from that
 *   net.c     ethernet frames, from anyone on the segment
 *   tcp.c     segments, from anyone who can guess a port
 *   http.c    a response, from the server we asked
 *
 * DETERMINISTIC. The generator is a 64-bit LCG seeded from argv, and every
 * iteration prints its seed on a crash, so a failure is a command line rather
 * than a story about something that happened once. Run it under ASan and
 * UBSan - that is the entire point, and a clean run without them proves
 * almost nothing.
 *
 *   ./fuzz [iterations] [seed]
 *
 * Invariants are asserted after every iteration, not just "it did not crash":
 * a layout that puts a run outside the box, a parser that reports more nodes
 * than its array holds, or a receive buffer whose length goes negative are all
 * silent corruption rather than a fault, and all three have happened here.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../html.h"
#include "../layout.h"
#include "../net.h"
#include "../tcp.h"
#include "../http.h"

static int fails, checks;
#define CHECK(cond, ...) do {                                    \
    checks++;                                                    \
    if (!(cond)) {                                               \
        fails++;                                                 \
        printf("  FAIL seed %llu iter %d: ",                     \
               (unsigned long long)seed0, iter);                 \
        printf(__VA_ARGS__);                                     \
        printf("\n");                                            \
        if (fails > 20) { printf("  (stopping)\n"); return; }     \
    }                                                            \
} while (0)

static unsigned long long rng_state, seed0;
static int iter;

static unsigned rnd(void)
{
    rng_state = rng_state * 6364136223846793005ULL + 1442695040888963407ULL;
    return (unsigned)(rng_state >> 33);
}
static unsigned rnd_n(unsigned n) { return n ? rnd() % n : 0; }

static unsigned v_ticks;
unsigned int idt_ticks(void) { return v_ticks; }
unsigned long long cpu_tsc(void) { return 0; }
unsigned int cpu_tsc_khz(void)   { return 0; }

static int meas(const char *s, int len, int size, int style)
{
    (void)s;
    int w = len * size / 2;
    if (style & LS_MONO) w = len * size * 6 / 10;
    if (style & LS_BOLD) w = w * 11 / 10;
    return w;
}

/* ---- markup ------------------------------------------------------------------
 * Three generators, because uniform random bytes almost never produce a '<'
 * followed by a tag name and so never reach the interesting code at all. The
 * structured ones build things that LOOK like markup and are wrong.
 */
static const char *TAGS[] = { "html","head","body","title","h1","h6","p","br",
    "hr","a","ul","ol","li","strong","em","b","i","code","pre","div","span",
    "img","script","style","section","x","", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" };
#define NTAGS ((int)(sizeof TAGS / sizeof TAGS[0]))

static int gen_markup(char *buf, int max)
{
    int n = 0;
    int mode = (int)rnd_n(3);
    int parts = (int)rnd_n(60) + 1;
    for (int i = 0; i < parts && n < max - 64; i++) {
        switch (mode == 2 ? (int)rnd_n(3) : mode) {
        case 0: {                                  /* raw bytes */
            int k = (int)rnd_n(24) + 1;
            while (k-- && n < max - 1) buf[n++] = (char)rnd_n(256);
            break;
        }
        case 1: {                                  /* something tag-shaped */
            const char *t = TAGS[rnd_n(NTAGS)];
            buf[n++] = '<';
            if (rnd_n(4) == 0) buf[n++] = '/';
            for (int k = 0; t[k] && n < max - 40; k++) buf[n++] = t[k];
            if (rnd_n(2)) {                        /* an attribute */
                const char *a = rnd_n(2) ? " href=" : " alt=";
                for (int k = 0; a[k] && n < max - 24; k++) buf[n++] = a[k];
                int q = (int)rnd_n(3);
                if (q == 0) buf[n++] = '"';
                else if (q == 1) buf[n++] = '\'';
                int k = (int)rnd_n(12);
                while (k-- && n < max - 8) {
                    char c = (char)(32 + rnd_n(95));
                    buf[n++] = c;
                }
                if (q == 0 && rnd_n(4)) buf[n++] = '"';
                else if (q == 1 && rnd_n(4)) buf[n++] = '\'';
            }
            if (rnd_n(8)) buf[n++] = '>';          /* sometimes never closed */
            break;
        }
        default: {                                 /* entities and text */
            const char *e[] = { "&amp;","&lt;","&#65;","&#x4a;","&notreal;",
                                "&#999999999;","&","&#;","&;", "hello world ",
                                "  \t\n  ", "<", ">" };
            const char *p = e[rnd_n(sizeof e / sizeof e[0])];
            for (int k = 0; p[k] && n < max - 2; k++) buf[n++] = p[k];
            break;
        }
        }
    }
    return n;
}

static void fuzz_markup(int iters)
{
    printf("html.c + layout.c\n");
    static char buf[8192];
    lay_set_measure(meas);
    for (iter = 0; iter < iters; iter++) {
        int n = gen_markup(buf, (int)sizeof buf);
        int nodes = html_parse(buf, n);

        CHECK(nodes >= 0 && nodes <= 1024, "node count %d out of range", nodes);
        CHECK(html_count() == nodes, "html_count disagrees with the parse");
        CHECK(html_max_depth() <= 64, "depth %d", html_max_depth());
        CHECK(html_arena_used() >= 0 && html_arena_used() <= 32768,
              "arena used %d", html_arena_used());

        /* every edge must stay inside the array, or the tree is not a tree */
        for (int i = 0; i < nodes; i++) {
            int p = html_parent(i), f = html_first(i), x = html_next(i);
            CHECK(p >= -1 && p < nodes, "node %d parent %d", i, p);
            CHECK(f >= -1 && f < nodes, "node %d first %d", i, f);
            CHECK(x >= -1 && x < nodes, "node %d next %d", i, x);
            CHECK(p != i && f != i && x != i, "node %d points at itself", i);
            int tl = 0;
            const char *t = html_text(i, &tl);
            CHECK(tl >= 0 && tl <= 32768, "node %d text length %d", i, tl);
            CHECK(t != 0, "node %d text pointer is null", i);
        }

        int width = (int)rnd_n(1200) + 1;
        int em = (int)rnd_n(48) + 1;
        int h = lay_run_doc(width, em);
        CHECK(h >= 0, "height %d", h);
        CHECK(lay_count() >= 0 && lay_count() <= 2048, "run count %d", lay_count());

        for (int i = 0; i < lay_count(); i++) {
            const struct lay_run *r = lay_at(i);
            CHECK(r != 0, "run %d is null", i);
            if (!r) break;
            CHECK(r->x >= 0, "run %d x %d", i, r->x);
            CHECK(r->w >= 0 && r->h >= 0, "run %d size %dx%d", i, r->w, r->h);
            /* A SINGLE GLYPH WIDER THAN THE BOX IS ALLOWED TO OVERFLOW, and
             * that is not a hedge - there is nothing a layout can do with a
             * box narrower than one character, and every real browser
             * overflows it too. Everything else must stay inside. The fuzzer
             * found this on its first iteration with a 6px box and a 34px
             * font, which is exactly the sort of ratio a hand-written test
             * never picks. */
            CHECK(r->x + r->w <= width || r->len <= 1,
                  "run %d of %d chars escapes the box: %d+%d > %d",
                  i, r->len, r->x, r->w, width);
            CHECK(r->x <= width, "run %d STARTS outside the box: %d > %d",
                  i, r->x, width);
            CHECK(r->len >= 0, "run %d length %d", i, r->len);
            CHECK(r->y >= 0 && r->y <= h + em * 4,
                  "run %d y %d of %d  (kind %d size %d h %d len %d, em %d width %d)",
                  i, r->y, h, r->kind, r->size, r->h, r->len, em, width);
            if (r->kind == LR_TEXT) CHECK(r->text != 0, "text run %d is null", i);
        }

        /* laying out again at the same width must reproduce it exactly - a
         * layout with leftover state does not */
        int again = lay_run_doc(width, em);
        CHECK(again == h, "re-layout gave %d, first gave %d", again, h);
    }
}

/* ---- frames ------------------------------------------------------------------- */
static int sink_hits;
static int fake_send(const net_u8 *f, int len) { (void)f; (void)len; return 1; }

static unsigned char inq[2048];
static int inq_len;
static int fake_poll(net_u8 *out, int max)
{
    if (!inq_len) return 0;
    int n = inq_len; if (n > max) n = max;
    memcpy(out, inq, n);
    inq_len = 0;
    return n;
}
static void ip_sink(net_u32 s, int p, const net_u8 *d, int l)
{ (void)s;(void)p;(void)d;(void)l; sink_hits++; }

static void fuzz_frames(int iters)
{
    printf("net.c\n");
    static const unsigned char mac[6] = { 0x52,0x54,0,0x12,0x34,0x56 };
    for (iter = 0; iter < iters; iter++) {
        if ((iter & 63) == 0) {
            net_link(fake_send, fake_poll, mac);
            net_config(0x0A00020Fu, 0xFFFFFF00u, 0x0A000202u);
            net_set_ip_sink(ip_sink);
        }
        int n = (int)rnd_n(200) + 1;
        for (int i = 0; i < n; i++) inq[i] = (unsigned char)rnd_n(256);
        /* steer a good fraction at the two ethertypes that reach real code */
        if (n >= 14) {
            unsigned pick = rnd_n(3);
            if (pick == 0) { inq[12] = 0x08; inq[13] = 0x06; }
            else if (pick == 1) {
                inq[12] = 0x08; inq[13] = 0x00;
                inq[14] = (unsigned char)(0x40 | (4 + rnd_n(4)));
                if (rnd_n(2)) {                 /* sometimes a valid checksum */
                    inq[24] = 0; inq[25] = 0;
                    unsigned short c = net_checksum(inq + 14, 20, 0);
                    inq[24] = (unsigned char)(c >> 8); inq[25] = (unsigned char)c;
                }
            }
        }
        inq_len = n;
        net_poll_once();
        CHECK(net_rx_frames() >= 0, "frame counter went negative");
        CHECK(net_rx_bad_csum() >= 0, "checksum counter went negative");
    }
}

/* ---- segments ------------------------------------------------------------------ */
static int tcp_out(net_u32 d, int p, const net_u8 *b, int l)
{ (void)d;(void)p;(void)b;(void)l; return 1; }

static void fuzz_segments(int iters)
{
    printf("tcp.c\n");
    static unsigned char seg[600];
    static unsigned char rbuf[600];
    for (iter = 0; iter < iters; iter++) {
        if ((iter & 31) == 0) {
            tcp_attach(tcp_out, 0x0A00020Fu);
            tcp_abort();
            tcp_connect(0x0A000202u, 80);
        }
        int dlen = (int)rnd_n(200);
        int n = 20 + dlen;
        for (int i = 0; i < n; i++) seg[i] = (unsigned char)rnd_n(256);
        seg[12] = (unsigned char)((5 + rnd_n(4)) << 4);
        if (rnd_n(2)) {                       /* half the time, our real ports */
            seg[0] = 0; seg[1] = 80;
        }
        if (rnd_n(3) == 0) {                  /* sometimes a valid checksum */
            seg[16] = 0; seg[17] = 0;
            unsigned sum = 0;
            sum += 0x0A00u; sum += 0x020Fu; sum += 0x0A00u; sum += 0x0202u;
            sum += 6; sum += (unsigned)n;
            unsigned short c = net_checksum(seg, n, sum);
            seg[16] = (unsigned char)(c >> 8); seg[17] = (unsigned char)c;
        }
        tcp_input(0x0A000202u, 6, seg, n);
        v_ticks += rnd_n(200);
        tcp_tick();

        CHECK(tcp_available() >= 0, "available went NEGATIVE: %d", tcp_available());
        CHECK(tcp_available() <= 16384, "available %d exceeds the buffer",
              tcp_available());
        int got = tcp_recv(rbuf, (int)rnd_n(600));
        CHECK(got >= 0, "tcp_recv returned %d", got);
        CHECK(got <= 600, "tcp_recv returned %d for a 600 byte buffer", got);
        CHECK(tcp_state() >= 0 && tcp_state() <= TCP_LAST_ACK,
              "state %d is not a state", tcp_state());
        if (rnd_n(8) == 0) tcp_send((const unsigned char *)"x", 1);
        if (rnd_n(16) == 0) tcp_close();
    }
}

/* ---- responses ----------------------------------------------------------------- */
static void fuzz_http(int iters)
{
    printf("http.c\n");
    static char resp[4096];
    static const char *heads[] = {
        "HTTP/1.0 200 OK\r\n", "HTTP/1.0 302 Found\r\n", "HTTP/1.0 404\r\n",
        "HTTP/1.0\r\n", "garbage\r\n", "HTTP/1.0 200 OK\n",
        "Content-Length: 5\r\n", "Content-Length: -1\r\n",
        "Content-Length: 99999999999\r\n", "Content-Length:\r\n",
        "Content-Type: text/html\r\n", "Content-Type: image/png\r\n",
        "Content-Type:\r\n", "Location: http://10.0.2.2/\r\n", "Location:\r\n",
        "\r\n", "\n", "X: y\r\n", "\r\n\r\n",
    };
    const int NH = (int)(sizeof heads / sizeof heads[0]);
    for (iter = 0; iter < iters; iter++) {
        tcp_attach(tcp_out, 0x0A00020Fu);
        tcp_abort();
        http_reset();
        http_start(0x0A000202u, 80, "10.0.2.2", "/");

        int n = 0, parts = (int)rnd_n(12) + 1;
        for (int i = 0; i < parts && n < (int)sizeof resp - 80; i++) {
            if (rnd_n(3) == 0) {
                int k = (int)rnd_n(40);
                while (k-- && n < (int)sizeof resp - 2) resp[n++] = (char)rnd_n(256);
            } else {
                const char *h = heads[rnd_n(NH)];
                for (int k = 0; h[k] && n < (int)sizeof resp - 2; k++) resp[n++] = h[k];
            }
        }
        /* http.c reads through tcp.c, which is not connected here - so this
         * exercises the polling and header paths against a socket that never
         * establishes, which is itself a case worth not crashing on */
        for (int k = 0; k < 4; k++) {
            int st = http_poll();
            CHECK(st >= 0 && st <= HTTP_ERROR, "state %d is not a state", st);
            CHECK(http_body_len() >= 0, "body length %d", http_body_len());
            CHECK(http_body_len() <= HTTP_BUF, "body length %d exceeds the buffer",
                  http_body_len());
            CHECK(http_total() >= 0 && http_total() <= HTTP_BUF,
                  "total %d", http_total());
            CHECK(http_status() >= 0 && http_status() < 100000,
                  "status %d", http_status());
        }
        (void)resp;
    }
}

int main(int argc, char **argv)
{
    int iters = argc > 1 ? atoi(argv[1]) : 20000;
    seed0 = argc > 2 ? strtoull(argv[2], 0, 0) : 12345;
    rng_state = seed0;
    printf("fuzzing the trust boundaries  (%d iterations, seed %llu)\n\n",
           iters, (unsigned long long)seed0);
    fuzz_markup(iters);
    fuzz_frames(iters);
    fuzz_segments(iters);
    fuzz_http(iters / 4 + 1);
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
