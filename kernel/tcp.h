/* tcp.h - one connection, client only.
 *
 * THE STATE ENUM IS FIRST, and that is not a formatting choice. §4 item 3:
 * "TCP is a state machine and the tempting shortcut is to write it as a
 * straight line of ifs. Write the state enum first, then the transitions."
 * Every bug avoided here is one that would otherwise be found with a packet
 * capture and a bad afternoon - and this session already spent one of those on
 * a single lost ICMP reply.
 *
 * SCOPED HARD, exactly as the brief scopes it:
 *   - ONE connection at a time. No table, no port allocation beyond a counter.
 *   - CLIENT ONLY. There is no listen, no accept, no passive open, and
 *     SYN_RCVD does not exist.
 *   - a fixed receive buffer, in-order plus ONE out-of-order hole
 *   - timeout retransmission plus triple-duplicate-ACK fast retransmit. No
 *     SACK, window scaling, or timestamps.
 *   - congestion control: slow start only. No AIMD, no congestion avoidance.
 *
 * Everything on that list is a decision. A window-scaled, SACK-capable stack
 * is not a harder version of this one, it is a different project - and the
 * useful thing to state is which of the two you have.
 */
#ifndef TCP_H
#define TCP_H

#include "net.h"

/* RFC 793's states, minus the ones a client cannot reach.
 * LISTEN and SYN_RECEIVED are absent because there is no passive open; saying
 * so here is better than leaving two enum values that no transition sets. */
enum tcp_state {
    TCP_CLOSED = 0,
    TCP_SYN_SENT,       /* active open sent, waiting for SYN-ACK       */
    TCP_ESTABLISHED,
    TCP_FIN_WAIT_1,     /* we closed; our FIN is unacknowledged        */
    TCP_FIN_WAIT_2,     /* our FIN is acknowledged; waiting for theirs */
    TCP_CLOSING,        /* both closed at once; our FIN unacknowledged */
    TCP_TIME_WAIT,      /* both closed; waiting out stray segments     */
    TCP_CLOSE_WAIT,     /* they closed; we may still send              */
    TCP_LAST_ACK        /* we closed after them; our FIN unacknowledged*/
};

/* Where segments go out. Injected for the same reason net.c's link is: the
 * whole state machine then runs in a host harness against scripted packet
 * sequences, which is the gate §4 item 3 asks for and is worth an order of
 * magnitude more than a boot. */
typedef int (*tcp_out_fn)(net_u32 dst, int proto,
                          const net_u8 *payload, int len);

void tcp_attach(tcp_out_fn out, net_u32 local_ip);

int  tcp_connect(net_u32 ip, int port);
int  tcp_state(void);
const char *tcp_state_name(int s);

/* Queue data for transmission. Returns how many bytes were accepted - the send
 * buffer is fixed and a short write is normal, not an error. */
int  tcp_send(const net_u8 *data, int len);
/* True only for an idle, fully acknowledged established connection to this
 * exact peer tuple. */
int  tcp_can_reuse(net_u32 ip, int port);

/* Take delivered bytes out of the receive buffer. */
int  tcp_recv(net_u8 *out, int max);
/* Drop up to `max` bytes from the receive buffer without copying them. Use this
 * to keep a connection draining when the consumer has nowhere to put the data -
 * tcp_recv(buf, 0) does NOT do that, it returns 0 and drains nothing. */
int  tcp_discard(int max);
int  tcp_available(void);

void tcp_close(void);
void tcp_abort(void);                 /* RST and go straight to CLOSED */

/* Drive the timers. Called from the same frame loop that pumps net_poll_once;
 * nothing here blocks and nothing here owns a loop. */
void tcp_tick(void);

/* net.c's IP sink. Registered with net_set_ip_sink. */
void tcp_input(net_u32 src, int proto, const net_u8 *payload, int len);

/* counters - every drop path is named, because item 2 spent six hypotheses
 * on a frame that was discarded by a bare `return` */
int tcp_rx_segs(void);
int tcp_tx_segs(void);
int tcp_retransmits(void);
int tcp_rx_bad_csum(void);
int tcp_rx_out_of_window(void);
int tcp_rx_dup(void);                 /* a segment we had already taken     */
int tcp_rx_ooo(void);                 /* held in the one out-of-order slot  */
int tcp_dup_acks(void);
int tcp_resets(void);
int tcp_cwnd(void);
int tcp_rto(void);

#endif
