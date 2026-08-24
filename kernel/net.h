/* net.h - ARP, IPv4 and ICMP, over any link that can move a frame.
 *
 * THE LINK IS INJECTED, exactly as layout.c injects its text measurement and
 * for exactly the same payoff: net.c then has no dependency on virtio_net.c,
 * on PCI, or on a machine, so it links into a host harness and can be driven
 * with scripted packets. §4 item 3 demands that for TCP; it is just as useful
 * one layer down, where "a stack that works once and drops every fourth
 * packet" is otherwise indistinguishable from one that works.
 */
#ifndef NET_H
#define NET_H

typedef unsigned char  net_u8;
typedef unsigned short net_u16;
typedef unsigned int   net_u32;

/* Move one frame out. Returns non-zero on success. */
typedef int (*net_send_fn)(const net_u8 *frame, int len);
/* Take one frame in, or return 0 if none has arrived. Never blocks. */
typedef int (*net_poll_fn)(net_u8 *out, int max);

void net_link(net_send_fn send, net_poll_fn poll, const net_u8 mac[6]);
void net_config(net_u32 ip, net_u32 mask, net_u32 gw);
net_u32 net_ip(void);
int net_live(void);          /* has net.c been given the link?              */

/* Pump the link once. Returns 1 if a frame was taken and dispatched, 0 if
 * nothing had arrived. MUST NOT BLOCK: everything above it is a state machine
 * driven from a frame loop, which is the same inversion ui.h imposes on apps.
 */
int net_poll_once(void);

/* Resolve an address, driving net_poll_once until the reply lands or the
 * bound expires. Returns 1 on success and fills mac_out. */
int net_arp_resolve(net_u32 ip, net_u8 mac_out[6], int ms);

/* One echo request, one matched reply. Returns the round trip in MICROSECONDS,
 * or -1 on timeout. Microseconds because the PIT's 10 ms tick cannot see a
 * local round trip at all, let alone its variation. */
int net_ping(net_u32 ip, int ms);

/* n pings, and the numbers that distinguish a working stack from one that
 * drops every fourth packet. Results are read out with the accessors below. */
int net_ping_run(net_u32 ip, int n, int ms);
int net_ping_sent(void);
int net_ping_recv(void);
int net_ping_lost(void);
int net_ping_min(void);
int net_ping_max(void);
int net_ping_avg(void);
int net_ping_jitter(void);   /* mean |rtt[i] - rtt[i-1]|, RFC 3550's shape  */
int net_ping_mask(void);     /* bit i set = ping i was lost, oldest first   */

/* counters, so a failure says which layer dropped it */
int net_rx_frames(void);
int net_rx_arp(void);
int net_rx_ip(void);
int net_rx_icmp(void);
int net_rx_bad_csum(void);
int net_rx_not_ours(void);
int net_tx_frames(void);
int net_rx_stale_echo(void);  /* echo replies that matched no outstanding request */
int net_rx_short(void);       /* IPv4 frames shorter than a header               */
int net_rx_badver(void);
int net_rx_badihl(void);
int net_rx_frag(void);

/* Handed the IPv4 payload of anything that is not ICMP, so tcp.c can take TCP
 * without net.c knowing what TCP is. Unset until item 3 sets it. */
typedef void (*net_ip_sink_fn)(net_u32 src, int proto,
                               const net_u8 *payload, int len);
void net_set_ip_sink(net_ip_sink_fn f);
/* Route one protocol number somewhere specific. TCP and UDP both need this;
 * anything unclaimed still goes to the sink above. */
void net_set_proto_sink(int proto, net_ip_sink_fn f);

/* Build and send an IPv4 packet. tcp.c's only way out. */
int net_send_ip(net_u32 dst, int proto, const net_u8 *payload, int len);

/* DHCP must send before an address or ARP cache exists.  This is the one
 * deliberate L2-broadcast escape hatch; ordinary traffic still goes through
 * net_send_ip() and ARP. */
int net_send_ip_broadcast(net_u32 src, net_u32 dst, int proto,
                          const net_u8 *payload, int len);
int net_mac_byte(int i);

net_u16 net_checksum(const net_u8 *p, int len, net_u32 seed);

#endif
