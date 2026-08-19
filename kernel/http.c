/* http.c - HTTP/1.0, and the four things that removes.
 *
 * HTTP/1.0, NOT 1.1, and that is a decision rather than a limitation. 1.1
 * brings chunked transfer encoding, persistent connections, pipelining and
 * Host-based virtual hosting as REQUIREMENTS rather than options. Chunked
 * alone is a second parser with its own framing bugs. 1.0 ends a body by
 * closing the connection, which this stack already handles because a FIN
 * mid-transfer is one of the cases tcptest asserts.
 *
 * We still send `Host:` - 1.0 does not require it and every real server wants
 * it - and we still handle `Content-Length` when it is offered, because a
 * length is better than a close. Neither obliges us to speak 1.1.
 *
 * NO HEAP, so the response lands in a fixed buffer and a body larger than it
 * is TRUNCATED with a flag set. The browser says so on screen rather than
 * rendering half a page as if it were whole.
 *
 * NOTHING BLOCKS. http_start() sends the request and returns; http_poll() is
 * called from the same loop that pumps the link, and reports a state. A
 * blocking fetch would put a while-loop at the top of the system, which is the
 * exact inversion this desktop was rewritten to remove.
 */

#include "http.h"
#include "tls.h"
#include "x509.h"

static int use_tls;
static struct tls_conn tls;
static int tls_failed;
static int tls_err;
static int tls_rnd_quality;

int rnd_bytes(unsigned char *out, int n);
int zl_now_z(char *out);
const struct x509_cert *zl_roots(int *n);

#include "tcp.h"

typedef net_u8  u8;
typedef net_u32 u32;

#define REQ_MAX  512
#define URL_MAX  256

static u8   req[REQ_MAX];
static char host[URL_MAX];
static char path[URL_MAX];
static char ctype[64];
static u32  target_ip;
static int  target_port;

static int  state = HTTP_IDLE;
static int  status_code;
static int  redirects;
static int  hdr_done;
static int  content_len = -1;
static int  truncated;
static int  refused_type;
static char location[URL_MAX];

/* The response, headers and all. The body is a slice of it rather than a
 * second copy - there is no allocator and a 32 KB document does not need to
 * exist twice. */
static u8   resp[HTTP_BUF];
static int  resp_len;
static int  body_at;


static void scopy(char *d, const char *s, int max)
{
    int i = 0;
    while (s[i] && i < max - 1) { d[i] = s[i]; i++; }
    d[i] = 0;
}

static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

/* case-insensitive prefix match, which is what a header name needs */
static int hdr_is(const u8 *p, int len, const char *name)
{
    int i = 0;
    while (name[i]) {
        if (i >= len) return 0;
        if (lower(p[i]) != name[i]) return 0;
        i++;
    }
    return 1;
}

/* ---- the request ----------------------------------------------------------- */
static int build_request(void)
{
    int n = 0;
    const char *g = "GET ";
    while (*g) req[n++] = (u8)*g++;
    for (int i = 0; path[i] && n < REQ_MAX - 64; i++) req[n++] = (u8)path[i];
    const char *v = " HTTP/1.0\r\nHost: ";
    while (*v) req[n++] = (u8)*v++;
    for (int i = 0; host[i] && n < REQ_MAX - 32; i++) req[n++] = (u8)host[i];
    /* No Accept-Encoding: asking for gzip and then not having an inflate is
     * how a browser gets a body it cannot read. No User-Agent games either -
     * this is what it is. */
    const char *t = "\r\nUser-Agent: zlOS\r\nConnection: close\r\n\r\n";
    while (*t) req[n++] = (u8)*t++;
    return n;
}

int http_start(u32 ip, int port, const char *hostname, const char *p)
{
    if (state == HTTP_CONNECTING || state == HTTP_RECEIVING) return 0;
    target_ip = ip;
    target_port = port ? port : 80;
    scopy(host, hostname ? hostname : "", URL_MAX);
    scopy(path, (p && *p) ? p : "/", URL_MAX);

    use_tls = 0;
    tls_failed = 0;
    resp_len = 0;
    body_at = 0;
    status_code = 0;
    hdr_done = 0;
    content_len = -1;
    truncated = 0;
    refused_type = 0;
    ctype[0] = 0;
    location[0] = 0;

    tcp_abort();
    if (!tcp_connect(ip, target_port)) { state = HTTP_ERROR; return 0; }
    state = HTTP_CONNECTING;
    return 1;
}

int http_start_tls(net_u32 ip, int port, const char *hostname, const char *path)
{
    if (!http_start(ip, port ? port : 443, hostname, path)) return 0;
    use_tls = 1;
    tls_failed = 0;
    tls_err = 0;
    for (int i = 0; i < (int)sizeof tls; i++) ((unsigned char *)&tls)[i] = 0;

    /* THE EPHEMERAL KEY, and the one place its quality matters. rnd_bytes
     * reports which tier produced it; a key from nothing is refused outright
     * rather than used, because a predictable scalar makes the whole session
     * readable while every other check still passes. */
    int q = rnd_bytes(tls.priv, 32);
    tls_rnd_quality = q;
    if (q == 0) { state = HTTP_TLS_FAIL; tls_err = TLS_E_PROTOCOL; return 0; }

    int nroots = 0;
    const struct x509_cert *roots = zl_roots(&nroots);
    static char nowbuf[20];
    int have_clock = zl_now_z(nowbuf);
    tls_trust(&tls, roots, nroots, have_clock ? nowbuf : 0);
    return 1;
}

int http_tls_error(void) { return tls_err; }
const char *http_tls_why(void) { return x509_why(); }
int http_rnd_quality(void) { return tls_rnd_quality; }

/* ---- the response ----------------------------------------------------------
 * Headers are parsed once, in place, the moment the blank line arrives. There
 * is no header table: three fields matter and keeping them is cheaper than
 * storing all of them and searching later.
 */
static void parse_headers(void)
{
    /* the status line: HTTP/1.x SSS text */
    int i = 0;
    while (i < resp_len && resp[i] != ' ') i++;
    while (i < resp_len && resp[i] == ' ') i++;
    status_code = 0;
    while (i < resp_len && resp[i] >= '0' && resp[i] <= '9')
        status_code = status_code * 10 + (resp[i++] - '0');

    /* then each header line until the blank one */
    while (i < resp_len) {
        while (i < resp_len && resp[i] != '\n') i++;
        i++;
        if (i >= resp_len) break;
        if (resp[i] == '\r' || resp[i] == '\n') break;    /* the blank line */

        const u8 *h = resp + i;
        int avail = resp_len - i;

        if (hdr_is(h, avail, "content-length:")) {
            int j = 15;
            while (j < avail && h[j] == ' ') j++;
            int v = 0, any = 0;
            while (j < avail && h[j] >= '0' && h[j] <= '9') {
                v = v * 10 + (h[j++] - '0');
                any = 1;
                if (v > HTTP_BUF * 64) break;         /* absurd: stop counting */
            }
            if (any) content_len = v;
        } else if (hdr_is(h, avail, "content-type:")) {
            int j = 13;
            while (j < avail && h[j] == ' ') j++;
            int k = 0;
            while (j < avail && h[j] != '\r' && h[j] != '\n' && h[j] != ';' &&
                   k < (int)sizeof ctype - 1)
                ctype[k++] = (char)lower(h[j++]);
            ctype[k] = 0;
        } else if (hdr_is(h, avail, "location:")) {
            int j = 9;
            while (j < avail && h[j] == ' ') j++;
            int k = 0;
            while (j < avail && h[j] != '\r' && h[j] != '\n' &&
                   k < URL_MAX - 1)
                location[k++] = (char)h[j++];
            location[k] = 0;
        }
    }
}

/* Where does the body start? After the first blank line, and the blank line is
 * CRLFCRLF on a compliant server and LFLF on plenty of real ones. Accepting
 * only the first is how a parser works everywhere except where it matters. */
static int find_body(void)
{
    for (int i = 0; i + 1 < resp_len; i++) {
        if (resp[i] == '\n' && resp[i + 1] == '\n') return i + 2;
        if (i + 3 < resp_len && resp[i] == '\r' && resp[i + 1] == '\n' &&
            resp[i + 2] == '\r' && resp[i + 3] == '\n') return i + 4;
    }
    return -1;
}

static int type_acceptable(void)
{
    if (!ctype[0]) return 1;                       /* unstated: allow it */
    const char *ok1 = "text/html", *ok2 = "text/plain";
    int m1 = 1, m2 = 1;
    for (int i = 0; ok1[i]; i++) if (ctype[i] != ok1[i]) { m1 = 0; break; }
    for (int i = 0; ok2[i]; i++) if (ctype[i] != ok2[i]) { m2 = 0; break; }
    return m1 || m2;
}

/* ---- the TLS transport ------------------------------------------------------
 * HTTPS IS THE SAME HTTP OVER A DIFFERENT PIPE, and this is the whole of the
 * difference. The parsing, the redirect logic and the type check below do not
 * know which pipe they are on: they call xport_* instead of tcp_*, and when
 * TLS is on those move bytes through tls.c on the way past.
 *
 * tls_pump() is called once per poll and does both directions - drain whatever
 * the handshake wants to send into the socket, push whatever arrived into the
 * handshake. It has to run even while state is HTTP_CONNECTING, because that
 * is when the handshake happens.
 */

static void tls_pump(void)
{
    if (!use_tls) return;
    const tu8 *p;
    int n = tls_take(&tls, &p);
    if (n > 0) {
        int w = tcp_send(p, n);
        if (w > 0) tls_sent(&tls, w);
    }
    int avail = tcp_available();
    while (avail > 0) {
        net_u8 buf[4096];
        int take = avail > (int)sizeof buf ? (int)sizeof buf : avail;
        int got = tcp_recv(buf, take);
        if (got <= 0) break;
        if (tls_feed(&tls, buf, got) < 0) { tls_failed = 1; return; }
        avail = tcp_available();
    }
    /* the handshake may have produced a reply to what just arrived */
    n = tls_take(&tls, &p);
    if (n > 0) {
        int w = tcp_send(p, n);
        if (w > 0) tls_sent(&tls, w);
    }
}

static int xport_send(const net_u8 *d, int n)
{
    if (!use_tls) return tcp_send(d, n);
    int w = tls_write(&tls, d, n);
    tls_pump();
    return w;
}

static int xport_recv(net_u8 *out, int max)
{
    if (!use_tls) return tcp_recv(out, max);
    return tls_read(&tls, out, max);
}

static int xport_available(void)
{
    if (!use_tls) return tcp_available();
    return tls.appn - tls.appr;
}

/* "the peer is finished with us". Under TLS that is still a TCP condition -
 * a close_notify or a FIN - but any buffered plaintext must be drained first,
 * which is why this is not simply tcp_state(). */
static int xport_closing(void)
{
    int s = tcp_state();
    int tcp_done = (s == TCP_CLOSE_WAIT || s == TCP_CLOSED || s == TCP_TIME_WAIT);
    if (!use_tls) return tcp_done;
    return tcp_done || tls_state(&tls) == TLS_CLOSED || tls_failed;
}

int http_poll(void)
{
    tls_pump();
    if (use_tls && (tls_failed || tls_state(&tls) == TLS_ERROR)) {
        tls_err = tls_error(&tls);
        tcp_abort();
        state = HTTP_TLS_FAIL;
        return state;
    }

    if (state == HTTP_CONNECTING) {
        int s = tcp_state();
        if (s == TCP_ESTABLISHED && use_tls && tls_state(&tls) == TLS_START) {
            /* the socket is up; start the handshake now that there is
             * somewhere to put the ClientHello */
            tls_start(&tls, host);
            tls_pump();
            return state;
        }
        if (s == TCP_ESTABLISHED && (!use_tls || tls_state(&tls) == TLS_READY)) {
            int n = build_request();
            xport_send(req, n);
            state = HTTP_RECEIVING;
        } else if (s == TCP_CLOSED) {
            state = HTTP_ERROR;                    /* refused, or never answered */
        }
        return state;
    }

    if (state != HTTP_RECEIVING) return state;

    int avail = xport_available();
    if (avail > 0) {
        int room = HTTP_BUF - resp_len;
        if (avail > room) { avail = room; truncated = 1; }
        if (avail > 0) resp_len += xport_recv(resp + resp_len, avail);
    }

    /* A FULL BUFFER USED TO BE A DEADLOCK, and only an in-kernel fetch found
     * it. With no room left, `avail` clamps to zero, nothing is read, the peer
     * never gets its window back and never finishes, the content-length is
     * never reached and the connection never closes - so http_poll spins for
     * ever and the browser sits in FETCHING. Every host test passed because
     * every page in them is smaller than the buffer.
     *
     * A truncated page is the documented behaviour; hanging is not. Stop
     * reading, say so, and render what arrived. */
    if (hdr_done && resp_len >= HTTP_BUF) {
        truncated = 1;
        tcp_abort();
        state = HTTP_DONE;
        return state;
    }

    if (!hdr_done) {
        int b = find_body();
        if (b >= 0) {
            body_at = b;
            hdr_done = 1;
            parse_headers();
            if (!type_acceptable()) {
                /* Refusing early is the point: a 40 MB video would otherwise
                 * fill the buffer before anyone noticed it was not a page. */
                refused_type = 1;
                tcp_abort();
                state = HTTP_REFUSED;
                return state;
            }
        }
    }

    /* Done when the length says so, or when the peer closes - which is how
     * HTTP/1.0 ends a body and why no chunked decoder is needed. */
    if (hdr_done && content_len >= 0 && resp_len - body_at >= content_len) {
        resp_len = body_at + content_len;
        tcp_close();
        state = HTTP_DONE;
    } else if (xport_closing()) {
        if (xport_available() == 0) {
            if (!hdr_done) { state = HTTP_ERROR; return state; }
            state = HTTP_DONE;
            if (tcp_state() == TCP_CLOSE_WAIT) tcp_close();
        }
    }

    if (state == HTTP_DONE && status_code >= 300 && status_code < 400 &&
        location[0] && redirects < HTTP_MAX_REDIRECTS) {
        redirects++;
        state = HTTP_REDIRECT;
    }
    return state;
}

int http_state(void)      { return state; }
int http_status(void)     { return status_code; }
int http_truncated(void)  { return truncated; }
int http_refused(void)    { return refused_type; }
int http_redirects(void)  { return redirects; }
int http_body_len(void)   { return hdr_done ? resp_len - body_at : 0; }
u32 http_body_addr(void)  { return (u32)(unsigned long)(resp + body_at); }
int http_total(void)      { return resp_len; }

const char *http_content_type(void) { return ctype; }
const char *http_location(void)     { return location; }

int http_body_byte(int i)
{
    if (!hdr_done || i < 0 || i >= resp_len - body_at) return 0;
    return resp[body_at + i];
}

void http_reset(void)
{
    state = HTTP_IDLE;
    redirects = 0;
    resp_len = 0;
    body_at = 0;
    status_code = 0;
    hdr_done = 0;
    truncated = 0;
    refused_type = 0;
}
