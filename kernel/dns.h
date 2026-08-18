/* dns.h - names into addresses. The rung that reaches the real internet.
 *
 * Everything below this is addressed by number. A person types a NAME, and
 * without this the browser can only reach machines someone has already looked
 * up by hand - which is the difference between a demonstration and a thing you
 * use.
 *
 * UDP, not TCP, and that is not a shortcut: a query and its answer each fit in
 * one datagram, and a resolver that opened a connection per lookup would be
 * slower and larger for no gain. TCP fallback exists in the protocol for
 * answers over 512 bytes; an A record never is.
 *
 * NOTHING BLOCKS, same as every layer under it. dns_start() sends and returns;
 * dns_poll() is called from the frame loop and reports a state.
 */
#ifndef DNS_H
#define DNS_H

#include "net.h"

#define DNS_PORT 53
#define DNS_MAX_NAME 128

enum {
    DNS_IDLE = 0,
    DNS_ASKING,
    DNS_DONE,
    DNS_NXDOMAIN,   /* the server says there is no such name */
    DNS_TIMEOUT,
    DNS_REFUSED     /* malformed, or an answer we will not trust */
};

/* Which server to ask. On QEMU's user-mode network that is 10.0.2.3; on a real
 * link it is whatever DHCP would have told us, and we have no DHCP - so it is
 * asserted, exactly like the address is. */
void dns_server(net_u32 ip);
net_u32 dns_get_server(void);

/* Look `name` up. Returns 0 if a lookup is already running or the name is
 * unusable. A cached answer completes immediately - dns_poll() returns
 * DNS_DONE on the first call. */
int  dns_start(const char *name, int len);
int  dns_poll(void);
int  dns_state(void);
net_u32 dns_result(void);

/* Resolve, driving the pump until an answer lands or the bound expires. For
 * callers that are not already in a frame loop. */
int  dns_resolve(const char *name, int len, net_u32 *out, int ms);

/* net.c's UDP sink. Registered alongside tcp.c's. */
void dns_input(net_u32 src, const net_u8 *payload, int len);
/* the form net_set_proto_sink wants */
void dns_ip_sink(net_u32 src, int proto, const net_u8 *payload, int len);

void dns_reset(void);        /* abandon an in-flight lookup; keeps the cache */
void dns_cache_clear(void);
int  dns_cache_count(void);
int  dns_queries(void);
int  dns_replies(void);
int  dns_rejected(void);      /* answers refused as malformed or unsafe */
int  dns_cache_hits(void);

#endif
