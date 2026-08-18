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

int http_poll(void)
{
    if (state == HTTP_CONNECTING) {
        int s = tcp_state();
        if (s == TCP_ESTABLISHED) {
            int n = build_request();
            tcp_send(req, n);
            state = HTTP_RECEIVING;
        } else if (s == TCP_CLOSED) {
            state = HTTP_ERROR;                    /* refused, or never answered */
        }
        return state;
    }

    if (state != HTTP_RECEIVING) return state;

    int avail = tcp_available();
    if (avail > 0) {
        int room = HTTP_BUF - resp_len;
        if (avail > room) { avail = room; truncated = 1; }
        if (avail > 0) resp_len += tcp_recv(resp + resp_len, avail);
        else           tcp_recv(resp, 0);
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
    } else if (tcp_state() == TCP_CLOSE_WAIT || tcp_state() == TCP_CLOSED ||
               tcp_state() == TCP_TIME_WAIT) {
        if (tcp_available() == 0) {
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
