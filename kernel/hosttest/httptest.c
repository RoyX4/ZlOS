/* httptest.c - HTTP/1.0 over the real TCP state machine, with no machine.
 *
 * http.c had no harness. It was gated end to end against a python http.server
 * in QEMU, which proves the happy path and nothing else - and the happy path
 * is the one case a server will reliably give you. Everything that makes a
 * parser wrong is what happens when the response is NOT what you expected:
 *
 *   - headers split across two segments, so the blank line arrives in pieces
 *   - LF LF instead of CRLF CRLF, which plenty of real servers send
 *   - a body with no Content-Length, ended only by the close
 *   - a Content-Length that lies, in both directions
 *   - a 302 with a Location, and a 302 without one
 *   - a body larger than the buffer
 *   - a Content-Type that is not a page, which must be refused BEFORE the
 *     body is read rather than after
 *
 * None of those can be produced on demand from a real server, and all of them
 * are two lines here. This drives the REAL tcp.c - the response arrives as
 * scripted TCP segments, so the test exercises the same reassembly path the
 * kernel uses rather than a mock of it.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../http.h"
#include "../tcp.h"

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

static unsigned v_ticks;
unsigned int idt_ticks(void) { return v_ticks; }
unsigned long long cpu_tsc(void) { return 0; }
unsigned int cpu_tsc_khz(void)   { return 0; }

#define LOCAL_IP 0x0A00020Fu
#define PEER_IP  0x0A000202u
#define PORT     8000

#define F_FIN 0x01
#define F_SYN 0x02
#define F_RST 0x04
#define F_ACK 0x10

/* ---- the peer ---------------------------------------------------------------
 * Captures what the stack sends and hands back whatever the test scripts.
 */
static unsigned char sent[64][2048];
static int sent_len[64], sent_flags[64], nsent;
static unsigned sent_seq[64];
static int lport;
static unsigned our_isn, peer_isn, peer_next, peer_ack;

static int capture(net_u32 dst, int proto, const net_u8 *p, int len)
{
    (void)dst; (void)proto;
    if (len < 20) return 1;
    if (!lport) lport = (p[0] << 8) | p[1];
    if (!our_isn) our_isn = ((unsigned)p[4] << 24) | ((unsigned)p[5] << 16) |
                            ((unsigned)p[6] << 8) | p[7];
    if (nsent < 64) {
        int doff = (p[12] >> 4) * 4;
        int dl = len - doff;
        sent_seq[nsent] = ((unsigned)p[4] << 24) |
                          ((unsigned)p[5] << 16) |
                          ((unsigned)p[6] << 8) | p[7];
        sent_flags[nsent] = p[13];
        if (dl > 0 && dl < 2048) { memcpy(sent[nsent], p + doff, dl); sent_len[nsent] = dl; }
        else sent_len[nsent] = 0;
        if (dl > 0) peer_ack = sent_seq[nsent] + (unsigned)dl;
        nsent++;
    }
    return 1;
}

static void inject(unsigned seq, unsigned ack, unsigned char flags,
                   const void *data, int dlen)
{
    unsigned char s[2048];
    memset(s, 0, sizeof s);
    s[0] = (unsigned char)(PORT >> 8); s[1] = (unsigned char)PORT;
    s[2] = (unsigned char)(lport >> 8); s[3] = (unsigned char)lport;
    s[4]=seq>>24; s[5]=seq>>16; s[6]=seq>>8; s[7]=seq;
    s[8]=ack>>24; s[9]=ack>>16; s[10]=ack>>8; s[11]=ack;
    s[12] = 5 << 4; s[13] = flags;
    s[14] = 0xFF; s[15] = 0xFF;
    if (dlen > 0) memcpy(s + 20, data, dlen);
    unsigned sum = 0;
    sum += (LOCAL_IP>>16)&0xFFFF; sum += LOCAL_IP&0xFFFF;
    sum += (PEER_IP>>16)&0xFFFF;  sum += PEER_IP&0xFFFF;
    sum += 6; sum += (unsigned)(20 + dlen);
    unsigned short ck = net_checksum(s, 20 + dlen, sum);
    s[16] = (unsigned char)(ck >> 8); s[17] = (unsigned char)ck;
    tcp_input(PEER_IP, 6, s, 20 + dlen);
}

/* Open a fetch and get as far as the request having been sent. */
static void begin(const char *path)
{
    tcp_attach(capture, LOCAL_IP);
    tcp_abort();
    http_reset();
    nsent = 0; lport = 0; our_isn = 0; peer_ack = 0;
    v_ticks = 1000;
    peer_isn = 0x30300000u;
    http_start(PEER_IP, PORT, "10.0.2.2", path);
    inject(peer_isn, our_isn + 1, F_SYN | F_ACK, 0, 0);
    peer_next = peer_isn + 1;
    http_poll();                      /* CONNECTING -> RECEIVING, request out */
}

/* Feed part of a response as one TCP segment. */
static void feed(const char *s, int n)
{
    if (n < 0) n = (int)strlen(s);
    inject(peer_next, peer_ack, F_ACK, s, n);
    peer_next += (unsigned)n;
    http_poll();
}

static void feed_fin(void)
{
    inject(peer_next, peer_ack, F_ACK | F_FIN, 0, 0);
    peer_next++;
    http_poll();
    http_poll();
}

static const char *body_str(void)
{
    static char b[HTTP_BUF + 1];
    int n = http_body_len();
    if (n > HTTP_BUF) n = HTTP_BUF;
    for (int i = 0; i < n; i++) b[i] = (char)http_body_byte(i);
    b[n] = 0;
    return b;
}

/* ---- the tests --------------------------------------------------------------- */
static void t_request(void)
{
    printf("the request\n");
    begin("/index.html");
    CHECK(nsent >= 2, "%d segments sent", nsent);
    /* the request is the last segment with a payload */
    int idx = -1;
    for (int i = 0; i < nsent; i++) if (sent_len[i] > 0) idx = i;
    CHECK(idx >= 0, "no request was sent");
    if (idx < 0) return;
    char req[2048];
    memcpy(req, sent[idx], sent_len[idx]);
    req[sent_len[idx]] = 0;

    CHECK(!memcmp(req, "GET /index.html HTTP/1.0\r\n", 26),
          "request line is '%.40s'", req);
    CHECK(strstr(req, "Host: 10.0.2.2\r\n") != 0, "no Host header");
    CHECK(strstr(req, "\r\n\r\n") != 0, "the request has no terminating blank line");
    /* asking for an encoding we cannot decode is how a browser gets a body it
     * cannot read - assert we do NOT ask */
    CHECK(strstr(req, "Accept-Encoding") == 0, "asked for a content encoding");
    CHECK(strstr(req, "HTTP/1.1") == 0, "claimed HTTP/1.1");
    CHECK(strstr(req, "Connection: keep-alive\r\n") != 0,
          "plain HTTP/1.0 did not offer bounded keep-alive reuse");
}

static void t_connection_reuse(void)
{
    printf("HTTP/1.0 connection reuse\n");
    begin("/one");
    feed("HTTP/1.0 200 OK\r\nContent-Length: 3\r\n"
         "Connection: keep-alive\r\n\r\none", -1);
    CHECK(http_state() == HTTP_DONE, "first response state %d", http_state());
    CHECK(tcp_state() == TCP_ESTABLISHED, "keep-alive socket state %s",
          tcp_state_name(tcp_state()));

    int before = nsent;
    int reused = http_connection_reuses();
    CHECK(http_start(PEER_IP, PORT, "10.0.2.2", "/two") == 1,
          "second request refused the idle matching socket");
    http_poll();
    CHECK(http_connection_reuses() == reused + 1,
          "reuse counter did not advance");
    CHECK(http_state() == HTTP_RECEIVING, "reused request state %d", http_state());
    int syns = 0, request = -1;
    for (int i = before; i < nsent; i++) {
        if (sent_flags[i] & F_SYN) syns++;
        if (sent_len[i] > 0) request = i;
    }
    CHECK(syns == 0, "reuse emitted %d new SYNs", syns);
    CHECK(request >= 0 && sent_len[request] >= 8 &&
          !memcmp(sent[request], "GET /two", 8),
          "the reused socket did not carry GET /two");

    feed("HTTP/1.0 200 OK\r\nContent-Length: 3\r\n"
         "Connection: close\r\n\r\ntwo", -1);
    CHECK(http_state() == HTTP_DONE && !strcmp(body_str(), "two"),
          "second response state/body %d '%s'", http_state(), body_str());
    CHECK(tcp_state() != TCP_ESTABLISHED,
          "Connection: close left a reusable socket open");
}

static void t_content_length(void)
{
    printf("a length-delimited body\n");
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\n", -1);
    feed("hello world", -1);
    CHECK(http_state() == HTTP_DONE, "state %d, wanted DONE", http_state());
    CHECK(http_status() == 200, "status %d", http_status());
    CHECK(http_body_len() == 11, "body is %d bytes", http_body_len());
    CHECK(!strcmp(body_str(), "hello world"), "body '%s'", body_str());

    /* ...and anything after the stated length must NOT be appended. A server
     * that sends trailing junk, or a keep-alive response we asked not to have,
     * would otherwise put it in the page. */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Length: 4\r\n\r\nbodyEXTRA", -1);
    CHECK(http_body_len() == 4, "body is %d bytes, wanted 4", http_body_len());
    CHECK(!strcmp(body_str(), "body"), "body '%s' - trailing bytes were kept", body_str());
}

static void t_close_delimited(void)
{
    printf("a close-delimited body\n");
    /* NO Content-Length. This is how HTTP/1.0 ends a body and the whole reason
     * no chunked decoder is needed - so it had better work. */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", -1);
    feed("<h1>hi</h1>", -1);
    CHECK(http_state() == HTTP_RECEIVING,
          "finished before the close (state %d)", http_state());
    feed_fin();
    CHECK(http_state() == HTTP_DONE, "state %d after the peer closed", http_state());
    CHECK(http_body_len() == 11, "body is %d bytes", http_body_len());
    CHECK(!strcmp(body_str(), "<h1>hi</h1>"), "body '%s'", body_str());
}

static void t_split_headers(void)
{
    printf("headers split across segments\n");
    /* The blank line arrives in pieces - the case a single-segment response
     * never produces and a real network produces constantly. */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Ty", -1);
    CHECK(http_state() == HTTP_RECEIVING, "finished mid-header");
    feed("pe: text/html\r\nContent-Length: 5\r", -1);
    CHECK(http_state() == HTTP_RECEIVING, "finished before the blank line");
    feed("\n\r\nabcde", -1);
    CHECK(http_state() == HTTP_DONE, "state %d", http_state());
    CHECK(http_body_len() == 5 && !strcmp(body_str(), "abcde"),
          "body '%s' (%d bytes)", body_str(), http_body_len());

    /* one byte at a time, which is the pathological version of the same thing */
    begin("/");
    const char *r = "HTTP/1.0 200 OK\r\nContent-Length: 3\r\n\r\nxyz";
    for (int i = 0; r[i]; i++) feed(r + i, 1);
    CHECK(http_state() == HTTP_DONE, "byte-at-a-time: state %d", http_state());
    CHECK(!strcmp(body_str(), "xyz"), "byte-at-a-time body '%s'", body_str());
}

static void t_lf_only(void)
{
    printf("bare LF line endings\n");
    /* Plenty of real servers - and every hand-written one - separate headers
     * with LF rather than CRLF. Accepting only CRLF is how a parser works
     * everywhere except where it matters. */
    begin("/");
    feed("HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: 2\n\nok", -1);
    CHECK(http_state() == HTTP_DONE, "state %d", http_state());
    CHECK(http_status() == 200, "status %d", http_status());
    CHECK(http_body_len() == 2 && !strcmp(body_str(), "ok"),
          "body '%s' (%d)", body_str(), http_body_len());
}

static void t_header_case(void)
{
    printf("header name case\n");
    /* Header names are case-insensitive and servers use every variation. */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nCONTENT-LENGTH: 3\r\nCoNtEnT-TyPe: TEXT/HTML\r\n\r\nabc", -1);
    CHECK(http_state() == HTTP_DONE, "state %d - a header case variant was missed",
          http_state());
    CHECK(http_body_len() == 3, "body is %d bytes", http_body_len());
}

static void t_status_codes(void)
{
    printf("status codes\n");
    begin("/missing");
    feed("HTTP/1.0 404 Not Found\r\nContent-Length: 9\r\n\r\nnot found", -1);
    CHECK(http_status() == 404, "status %d", http_status());
    CHECK(http_state() == HTTP_DONE, "a 404 is a response, not an error");
    CHECK(http_body_len() == 9, "the error page body was dropped");

    begin("/boom");
    feed("HTTP/1.0 500 Internal Server Error\r\nContent-Length: 1\r\n\r\nx", -1);
    CHECK(http_status() == 500, "status %d", http_status());

    /* a status line with no reason phrase at all */
    begin("/");
    feed("HTTP/1.0 204\r\nContent-Length: 0\r\n\r\n", -1);
    CHECK(http_status() == 204, "status %d with no reason phrase", http_status());
    CHECK(http_body_len() == 0, "204 produced %d body bytes", http_body_len());
}

static void t_redirect(void)
{
    printf("redirects\n");
    begin("/old");
    feed("HTTP/1.0 302 Found\r\nLocation: http://10.0.2.2:8000/new\r\n"
         "Content-Length: 0\r\n\r\n", -1);
    CHECK(http_status() == 302, "status %d", http_status());
    CHECK(http_state() == HTTP_REDIRECT, "state %d, wanted REDIRECT", http_state());
    CHECK(!strcmp(http_location(), "http://10.0.2.2:8000/new"),
          "location '%s'", http_location());

    /* a 3xx with NO Location is not a redirect - it is just a response, and
     * treating it as one would send the browser to the empty string */
    begin("/old");
    feed("HTTP/1.0 302 Found\r\nContent-Length: 0\r\n\r\n", -1);
    CHECK(http_state() == HTTP_DONE, "a 302 with no Location became state %d",
          http_state());
}

static void t_refused_type(void)
{
    printf("a content type that is not a page\n");
    /* REFUSED BEFORE THE BODY, which is the point: a 40 MB video would
     * otherwise fill the buffer before anyone noticed it was not a page. */
    begin("/cat.png");
    feed("HTTP/1.0 200 OK\r\nContent-Type: image/png\r\nContent-Length: 900000\r\n\r\n", -1);
    CHECK(http_state() == HTTP_REFUSED, "state %d, wanted REFUSED", http_state());
    CHECK(http_refused() == 1, "the refusal was not flagged");
    CHECK(tcp_state() == TCP_CLOSED, "the connection was left open (%s)",
          tcp_state_name(tcp_state()));

    /* text/plain is acceptable, and so is a charset parameter after it */
    begin("/readme.txt");
    feed("HTTP/1.0 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\n"
         "Content-Length: 2\r\n\r\nhi", -1);
    CHECK(http_state() == HTTP_DONE, "text/plain with a charset was refused (%d)",
          http_state());

    /* an unstated type is allowed - refusing it would break every server that
     * does not send one */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Length: 2\r\n\r\nhi", -1);
    CHECK(http_state() == HTTP_DONE, "an unstated content type was refused");
}

static void t_truncation(void)
{
    printf("a body larger than the buffer\n");
    begin("/big");
    feed("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n", -1);
    static char chunk[1000];
    memset(chunk, 'A', sizeof chunk);
    /* Overflow the REAL buffer, not a literal 40 KB. This said 40 when
     * HTTP_BUF was 32768; raising the buffer to hold a real page turned a
     * genuine bounds check into a test that fed less than the buffer and
     * asserted truncation anyway. Same drift HTML_MAX_NODES and
     * browser_doc_cap were fixed for. */
    for (int i = 0; i < (HTTP_BUF / 1000) + 8; i++) feed(chunk, (int)sizeof chunk);
    feed_fin();
    CHECK(http_truncated() == 1, "40 KB into a %d byte buffer was not flagged",
          HTTP_BUF);
    CHECK(http_body_len() <= HTTP_BUF, "body is %d bytes, buffer is %d",
          http_body_len(), HTTP_BUF);
    CHECK(http_total() <= HTTP_BUF, "the response overran the buffer (%d)",
          http_total());
}

static void t_malformed(void)
{
    printf("responses that are not responses\n");
    /* none of these may fault, and each must end in a state that is not
     * "still receiving" once the peer has gone */
    const char *junk[] = {
        "",
        "\r\n\r\n",
        "not http at all\r\n\r\nbody",
        "HTTP/1.0\r\n\r\n",
        "HTTP/1.0 \r\n\r\n",
        "HTTP/1.0 abc OK\r\n\r\nbody",
        "HTTP/1.0 200 OK\r\nContent-Length: \r\n\r\nbody",
        "HTTP/1.0 200 OK\r\nContent-Length: -5\r\n\r\nbody",
        "HTTP/1.0 200 OK\r\nContent-Length: 999999999999\r\n\r\nbody",
        "HTTP/1.0 200 OK\r\nContent-Length: 99999\r\n\r\nshort",
        "HTTP/1.0 200 OK\r\nLocation:\r\n\r\n",
        "HTTP/1.0 200 OK\r\nContent-Type:\r\n\r\nbody",
        "\0\0\0\0binary garbage",
        0
    };
    for (int i = 0; junk[i]; i++) {
        begin("/");
        feed(junk[i], (int)strlen(junk[i]));
        feed_fin();
        CHECK(http_state() != HTTP_RECEIVING,
              "case %d ('%.20s') left the fetch hanging", i, junk[i]);
        CHECK(http_body_len() >= 0, "case %d produced a negative body length", i);
        CHECK(http_body_len() <= HTTP_BUF, "case %d overran the buffer", i);
    }

    /* a Content-Length longer than the body, then a close: the body that DID
     * arrive should still be usable rather than discarded */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Length: 100\r\n\r\npartial", -1);
    feed_fin();
    CHECK(http_state() == HTTP_DONE, "a short body never completed (state %d)",
          http_state());
    CHECK(http_body_len() == 7, "kept %d of 7 bytes that arrived", http_body_len());
}

static void t_connection_failures(void)
{
    printf("connections that do not happen\n");
    /* the port is closed: a RST during the handshake */
    tcp_attach(capture, LOCAL_IP);
    tcp_abort();
    http_reset();
    nsent = 0; lport = 0; our_isn = 0;
    v_ticks = 1000;
    http_start(PEER_IP, PORT, "10.0.2.2", "/");
    inject(0x40400000u, our_isn + 1, F_RST | F_ACK, 0, 0);
    http_poll();
    CHECK(http_state() == HTTP_ERROR, "a refused connection gave state %d",
          http_state());

    /* the server accepts and then closes without saying anything at all */
    begin("/");
    feed_fin();
    CHECK(http_state() == HTTP_ERROR, "an empty response gave state %d",
          http_state());

    /* http_start must refuse to stack a second fetch on a live one */
    begin("/");
    feed("HTTP/1.0 200 OK\r\nContent-Length: 100\r\n\r\nx", -1);
    CHECK(http_state() == HTTP_RECEIVING, "state %d", http_state());
    CHECK(http_start(PEER_IP, PORT, "10.0.2.2", "/other") == 0,
          "a second fetch was started over a live one");
}

int main(void)
{
    printf("http.c over the real tcp.c, no machine\n\n");
    t_request();
    t_connection_reuse();
    t_content_length();
    t_close_delimited();
    t_split_headers();
    t_lf_only();
    t_header_case();
    t_status_codes();
    t_redirect();
    t_refused_type();
    t_truncation();
    t_malformed();
    t_connection_failures();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
