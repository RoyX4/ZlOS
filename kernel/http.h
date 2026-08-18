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

#define HTTP_BUF 32768
#define HTTP_MAX_REDIRECTS 5

enum {
    HTTP_IDLE = 0,
    HTTP_CONNECTING,
    HTTP_RECEIVING,
    HTTP_DONE,
    HTTP_REDIRECT,     /* 3xx with a Location, under the redirect limit */
    HTTP_REFUSED,      /* not text/html or text/plain - refused, not fetched */
    HTTP_ERROR
};

int http_start(net_u32 ip, int port, const char *hostname, const char *path);
int http_poll(void);
void http_reset(void);

int http_state(void);
int http_status(void);
int http_body_len(void);
int http_body_byte(int i);
net_u32 http_body_addr(void);
int http_total(void);
int http_truncated(void);
int http_refused(void);
int http_redirects(void);
const char *http_content_type(void);
const char *http_location(void);

#endif
