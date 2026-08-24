/* http.h - a fetch, as a state machine.
 *
 * NOT a function that returns a page. A blocking fetch would put a while-loop
 * at the top of the system, which is exactly the inversion this desktop was
 * rewritten to remove: http_start() sends and returns, http_poll() is called
 * from the frame loop, and the browser repaints when the state changes.
 */
#ifndef HTTP_H
#define HTTP_H

#include "net.h"

/* Sized against a REAL page, not a demo one. 32 KB predates the browser
 * fetching anything but hand-written test pages; www.wikipedia.org's front
 * page is 120 KB and every https target worth reaching is bigger than 32 KB.
 * Still under browser.c's DOC_MAX so a page that fits here fits there. */
#define HTTP_BUF 131072
#define HTTP_MAX_REDIRECTS 5

enum {
    HTTP_IDLE = 0,
    HTTP_CONNECTING,
    HTTP_RECEIVING,
    HTTP_DONE,
    HTTP_REDIRECT,     /* 3xx with a Location, under the redirect limit */
    HTTP_REFUSED,      /* not text/html or text/plain - refused, not fetched */
    HTTP_ERROR,
    HTTP_TLS_FAIL      /* the handshake or the certificate check failed */
};

int http_start(net_u32 ip, int port, const char *hostname, const char *path);

/* The same fetch over TLS. Certificate verification is ALWAYS ON here - there
 * is no unverified https, because an https:// URL is a promise to the user and
 * a "just connect anyway" path is how that promise gets quietly broken. */
int http_start_tls(net_u32 ip, int port, const char *hostname, const char *path);
int http_tls_error(void);       /* a TLS_E_* when state is HTTP_TLS_FAIL */
const char *http_tls_why(void); /* the certificate reason, if that is why  */
int http_poll(void);
void http_reset(void);

/* What content types THIS fetch will accept. Call it after http_reset() and
 * before http_start(); http_reset puts it back to text-only, so forgetting is
 * the strict behaviour and not the loose one. */
#define HTTP_ACCEPT_TEXT  (1 << 0)
#define HTTP_ACCEPT_IMAGE (1 << 1)
#define HTTP_ACCEPT_CSS   (1 << 2)
void http_accept(int mask);

int http_state(void);
int http_status(void);
int http_body_len(void);
int http_body_byte(int i);
net_u32 http_body_addr(void);
int http_total(void);
int http_truncated(void);
int http_refused(void);
int http_redirects(void);
int http_connection_reuses(void); /* completed plain HTTP/1.0 socket reuses */
const char *http_content_type(void);
const char *http_location(void);

#endif
