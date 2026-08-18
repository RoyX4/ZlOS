/* tcp.c - the state machine, then the transitions.
 *
 * SEQUENCE NUMBERS ARE MODULAR AND COMPARISONS MUST BE TOO. Every ordering
 * question here goes through seq_lt/seq_le/seq_gt, which subtract and look at
 * the sign of a SIGNED 32-bit result. Writing `a < b` on two u32 sequence
 * numbers is correct for about 68 minutes of a fast connection and then
 * silently inverts, and the failure looks like a peer sending garbage. This is
 * the single most common way to get TCP wrong and it costs nothing to avoid.
 *
 * SYN AND FIN EACH CONSUME A SEQUENCE NUMBER. They are not data and they do
 * occupy sequence space, so a connection that sends SYN then 100 bytes has
 * snd_nxt = iss + 101. Getting this wrong makes the handshake work and every
 * close hang, which is a maddening combination to debug.
 *
 * ONE OUT-OF-ORDER SLOT, not a reassembly queue. When a segment arrives past
 * rcv_nxt it is held - once - and replayed the moment the hole fills. Two
 * holes and the second is dropped and counted. That is the brief's scope, and
 * it is enough for a stack whose peer is one HTTP server on a local link.
 *
 * SLOW START ONLY. cwnd opens by one segment per ACK and collapses to one on a
 * retransmit. There is no congestion avoidance, no fast retransmit and no
 * SACK. On a link whose round trip is measured in microseconds that is not a
 * meaningful loss; on the open internet it would be.
 *
 * NOTHING BLOCKS. tcp_connect returns immediately and the caller polls
 * tcp_state(); tcp_tick() drives the retransmit timer from the frame loop.
 * Same inversion ui.h imposes on apps, three layers down.
 */

#include "tcp.h"

typedef net_u8  u8;
typedef net_u16 u16;
typedef net_u32 u32;

unsigned int idt_ticks(void);

#define SND_BUF   8192
#define RCV_BUF   16384
#define OOO_BUF   2048
#define MSS       1400            /* under a 1500 MTU with room for headers */
#define TCP_PROTO 6

/* The PIT ticks at 100 Hz, so these are all in hundredths of a second. */
#define RTO_MIN   10              /* 100 ms */
#define RTO_MAX   400             /* 4 s    */
#define SYN_TRIES 5
#define TIME_WAIT_TICKS 100       /* 1 s. RFC says 2*MSL; this is a client on
                                     a local link, and holding the single
                                     connection slot for four minutes would
                                     make the browser unusable. Stated as the
                                     deviation it is. */

static tcp_out_fn out_fn;
static u32 local_ip;

static int  st = TCP_CLOSED;
static u32  peer_ip;
static u16  local_port = 40000, remote_port;

/* send sequence space */
static u32  iss, snd_una, snd_nxt;
static u16  snd_wnd = 1;
/* receive sequence space */
static u32  irs, rcv_nxt;

static u8   sndbuf[SND_BUF];
static int  snd_len;              /* bytes queued, unacknowledged first     */
static u8   rcvbuf[RCV_BUF];
static int  rcv_head, rcv_tail;

/* the ONE out-of-order slot */
static u8   ooo[OOO_BUF];
static int  ooo_len;
static u32  ooo_seq;
static int  ooo_fin;        /* the held segment carried a FIN      */
static int  ooo_fin_ready;  /* ...and its hole has now been filled  */

static int  fin_sent, fin_acked, fin_seen;
static int  fin_wanted;   /* close() was called; the FIN follows the data */
static int  seg_has_fin;  /* does the segment being handled carry a FIN? */
static int  fin_tries;
static int  rexmit_tries;   /* total retransmissions on this connection */
static u32  fin_seq;              /* the sequence number our FIN occupies   */

static u32  rt_deadline;          /* tick at which to retransmit            */
static int  rto = RTO_MIN;
static int  syn_tries;
static u32  tw_deadline;
static int  cwnd = 1;             /* in segments. Slow start only.          */
static u32  last_ack;
static int  dup_ack_run;

static int c_rx, c_tx, c_rexmit, c_badsum, c_oow, c_dup, c_ooo, c_dupack, c_rst;

/* ---- modular sequence comparison ------------------------------------------
 * The whole reason TCP works across a wrap. `(int)(a - b) < 0` is well defined
 * on unsigned inputs converted to a signed difference and is what every real
 * stack uses; `a < b` is not. */
static int seq_lt(u32 a, u32 b) { return (int)(a - b) < 0; }
static int seq_le(u32 a, u32 b) { return (int)(a - b) <= 0; }
static int seq_gt(u32 a, u32 b) { return (int)(a - b) > 0; }
static int seq_ge(u32 a, u32 b) { return (int)(a - b) >= 0; }

static u16 be16(const u8 *p) { return (u16)(((u16)p[0] << 8) | p[1]); }
static u32 be32(const u8 *p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}
static void put16(u8 *p, u16 v) { p[0] = (u8)(v >> 8); p[1] = (u8)v; }
static void put32(u8 *p, u32 v)
{
    p[0] = (u8)(v >> 24); p[1] = (u8)(v >> 16);
    p[2] = (u8)(v >> 8);  p[3] = (u8)v;
}

#define F_FIN 0x01
#define F_SYN 0x02
#define F_RST 0x04
#define F_PSH 0x08
#define F_ACK 0x10

const char *tcp_state_name(int s)
{
    switch (s) {
    case TCP_CLOSED:      return "CLOSED";
    case TCP_SYN_SENT:    return "SYN_SENT";
    case TCP_ESTABLISHED: return "ESTABLISHED";
    case TCP_FIN_WAIT_1:  return "FIN_WAIT_1";
    case TCP_FIN_WAIT_2:  return "FIN_WAIT_2";
    case TCP_CLOSING:     return "CLOSING";
    case TCP_TIME_WAIT:   return "TIME_WAIT";
    case TCP_CLOSE_WAIT:  return "CLOSE_WAIT";
    case TCP_LAST_ACK:    return "LAST_ACK";
    default:              return "?";
    }
}

void tcp_attach(tcp_out_fn out, u32 ip)
{
    out_fn = out;
    local_ip = ip;
}

/* ---- the receive buffer ----------------------------------------------------
 * A plain linear buffer with a head and a tail rather than a ring: a ring
 * would need two copies for a wrapped read and this is drained by a caller
 * that reads everything available. When the tail reaches the head both reset. */
static int rcv_space(void) { return RCV_BUF - rcv_tail; }

static void rcv_put(const u8 *p, int n)
{
    if (n > rcv_space()) n = rcv_space();
    for (int i = 0; i < n; i++) rcvbuf[rcv_tail + i] = p[i];
    rcv_tail += n;
}

int tcp_available(void) { return rcv_tail - rcv_head; }

int tcp_recv(u8 *out, int max)
{
    if (max <= 0) return 0;              /* a trust boundary, so it is checked */
    int n = tcp_available();
    if (n > max) n = max;
    for (int i = 0; i < n; i++) out[i] = rcvbuf[rcv_head + i];
    rcv_head += n;
    if (rcv_head == rcv_tail) rcv_head = rcv_tail = 0;   /* the compaction */
    return n;
}

/* ---- segments out ---------------------------------------------------------- */
static u8 seg[MSS + 20];

/* The TCP checksum covers a PSEUDO-HEADER that is not transmitted: source and
 * destination address, the protocol and the TCP length. Omitting it produces a
 * checksum that is self-consistent and that every peer rejects, which reads as
 * "the server is ignoring us". */
static u16 tcp_csum(const u8 *s, int len)
{
    u32 sum = 0;
    sum += (local_ip >> 16) & 0xFFFF; sum += local_ip & 0xFFFF;
    sum += (peer_ip >> 16) & 0xFFFF;  sum += peer_ip & 0xFFFF;
    sum += TCP_PROTO;
    sum += (u32)len;
    return net_checksum(s, len, sum);
}

static int send_seg(u32 seqno, u8 flags, const u8 *data, int dlen)
{
    if (!out_fn) return 0;
    if (dlen < 0) dlen = 0;
    if (dlen > MSS) dlen = MSS;

    put16(seg + 0, local_port);
    put16(seg + 2, remote_port);
    put32(seg + 4, seqno);
    put32(seg + 8, (flags & F_ACK) ? rcv_nxt : 0);
    seg[12] = 5 << 4;                       /* data offset: 5 words, no options */
    seg[13] = flags;
    /* THE WINDOW IS THE SPACE THAT ACTUALLY EXISTS. rcv_space() is what a new
     * segment can be written into; RCV_BUF - tcp_available() counts bytes the
     * reader has consumed but which have not been compacted away yet, and
     * advertising those invites the peer to send data there is no room for. */
    int freespace = rcv_space();
    if (freespace < 0) freespace = 0;
    if (freespace > 65535) freespace = 65535;
    put16(seg + 14, (u16)freespace);
    put16(seg + 16, 0);                     /* checksum, below */
    put16(seg + 18, 0);                     /* urgent pointer  */
    for (int i = 0; i < dlen; i++) seg[20 + i] = data[i];
    put16(seg + 16, tcp_csum(seg, 20 + dlen));

    c_tx++;
    return out_fn(peer_ip, TCP_PROTO, seg, 20 + dlen);
}

static void arm_timer(void)
{
    rt_deadline = idt_ticks() + (u32)rto;
}

/* How much may go out right now: what the peer's window allows, capped by the
 * congestion window. Slow start is the ONLY control here, so cwnd is in
 * segments and grows by one per acknowledgement. */
static int send_window(void)
{
    int w = (int)snd_wnd;
    int c = cwnd * MSS;
    if (c < w) w = c;
    if (w < 0) w = 0;
    return w;
}

/* Push whatever the window allows from the send buffer. Everything in sndbuf
 * from snd_una onward is unacknowledged and must be kept for retransmission. */
static void pump_send(void)
{
    if (st != TCP_ESTABLISHED && st != TCP_CLOSE_WAIT &&
        st != TCP_FIN_WAIT_1 && st != TCP_LAST_ACK) return;
    int inflight = (int)(snd_nxt - snd_una);
    int avail = snd_len - inflight;
    int win = send_window() - inflight;

    while (avail > 0 && win > 0) {
        int n = avail;
        if (n > MSS) n = MSS;
        if (n > win) n = win;
        if (n <= 0) break;
        send_seg(snd_nxt, F_ACK | F_PSH, sndbuf + (int)(snd_nxt - snd_una), n);
        snd_nxt += (u32)n;
        avail -= n;
        win -= n;
        if (rt_deadline == 0) arm_timer();
    }

    /* THE FIN GOES AFTER THE DATA, NOT INSTEAD OF IT. tcp_close() used to send
     * it at snd_nxt immediately, which on a slow-start connection is in the
     * MIDDLE of what the application handed over - everything still queued was
     * then abandoned, because the old pump_send refused to run once the state
     * had moved on. Closing after a large send silently truncated the stream. */
    if (fin_wanted && !fin_sent && snd_nxt == snd_una + (u32)snd_len) {
        fin_seq = snd_nxt;
        send_seg(snd_nxt, F_FIN | F_ACK, 0, 0);
        snd_nxt++;                          /* the FIN consumes one */
        fin_sent = 1;
        arm_timer();
    }
}

int tcp_send(const u8 *data, int len)
{
    if (st != TCP_ESTABLISHED && st != TCP_CLOSE_WAIT) return 0;
    if (len < 0) return 0;
    int room = SND_BUF - snd_len;
    if (len > room) len = room;
    for (int i = 0; i < len; i++) sndbuf[snd_len + i] = data[i];
    snd_len += len;
    pump_send();
    return len;
}

/* ---- the active open ------------------------------------------------------- */
int tcp_connect(u32 ip, int port)
{
    if (st != TCP_CLOSED) return 0;
    peer_ip = ip;
    remote_port = (u16)port;
    local_port++;
    if (local_port < 40000) local_port = 40000;

    /* Not random. There is no entropy source in this kernel and pretending
     * otherwise would be worse than saying so: the tick counter at least
     * differs between connections, which is what stops a stale segment from
     * the previous connection being accepted by this one. It is NOT a defence
     * against an off-path attacker guessing it, and on a local link to a
     * server we chose that is the honest trade. */
    iss = (idt_ticks() << 8) ^ 0x5A5A0000u;
    snd_una = iss;
    snd_nxt = iss + 1;                      /* SYN consumes one */
    rcv_nxt = 0;
    snd_len = 0;
    rcv_head = rcv_tail = 0;
    ooo_len = 0;
    fin_sent = fin_acked = fin_seen = 0;
    fin_wanted = 0;
    fin_tries = 0;
    rexmit_tries = 0;
    ooo_fin = ooo_fin_ready = 0;
    seg_has_fin = 0;
    cwnd = 1;
    rto = RTO_MIN;
    syn_tries = 0;
    dup_ack_run = 0;
    last_ack = 0;
    snd_wnd = MSS;

    st = TCP_SYN_SENT;
    send_seg(iss, F_SYN, 0, 0);
    arm_timer();
    return 1;
}

void tcp_abort(void)
{
    if (st != TCP_CLOSED) {
        send_seg(snd_nxt, F_RST | F_ACK, 0, 0);
        c_rst++;
    }
    st = TCP_CLOSED;
    rt_deadline = 0;
    snd_len = 0;
    ooo_len = 0;
}

void tcp_close(void)
{
    if (st == TCP_ESTABLISHED) {
        fin_wanted = 1;
        st = TCP_FIN_WAIT_1;
        pump_send();                        /* drains the buffer, then FINs */
        arm_timer();
    } else if (st == TCP_CLOSE_WAIT) {
        fin_wanted = 1;
        st = TCP_LAST_ACK;
        pump_send();
        arm_timer();
    } else if (st == TCP_SYN_SENT) {
        st = TCP_CLOSED;                    /* nothing was ever established */
        rt_deadline = 0;
    }
}

/* ---- delivering received data ---------------------------------------------- */
/* Returns 1 if the whole segment was taken. A PARTIAL TAKE IS NOT ALLOWED:
 * rcv_nxt may only advance by bytes that are actually in the buffer, or the
 * connection acknowledges data it discarded and the peer never resends it -
 * the stream then has a hole in it with no error anywhere. The correct answer
 * to a full buffer is to refuse the segment and advertise a smaller window,
 * which is what flow control is for. */
static int deliver(const u8 *p, int n)
{
    if (n > rcv_space()) return 0;            /* no room: do not ACK past it */
    rcv_put(p, n);
    rcv_nxt += (u32)n;

    /* the one out-of-order slot */
    for (;;) {
        if (!ooo_len) break;
        /* STALE: the hole was filled by a segment that overshot the held one,
         * so its sequence number can never equal rcv_nxt again and the slot
         * would stay occupied for the life of the connection. */
        if (seq_le(ooo_seq + (u32)ooo_len, rcv_nxt)) { ooo_len = 0; break; }
        if (ooo_seq == rcv_nxt) {
            if (ooo_len > rcv_space()) break;  /* no room yet; keep holding it */
            rcv_put(ooo, ooo_len);
            rcv_nxt += (u32)ooo_len;
            ooo_len = 0;
            /* a FIN that rode the out-of-order segment is only ours NOW */
            if (ooo_fin) { ooo_fin = 0; ooo_fin_ready = 1; }
            continue;
        }
        if (seq_lt(ooo_seq, rcv_nxt)) {       /* partial overlap: trim it */
            u32 skip = rcv_nxt - ooo_seq;
            if (skip >= (u32)ooo_len) { ooo_len = 0; break; }
            int keep = ooo_len - (int)skip;
            if (keep > rcv_space()) break;
            rcv_put(ooo + skip, keep);
            rcv_nxt += (u32)keep;
            ooo_len = 0;
            if (ooo_fin) { ooo_fin = 0; ooo_fin_ready = 1; }
            continue;
        }
        break;
    }
    return 1;
}

/* ---- the transitions -------------------------------------------------------
 * One function per state, called from tcp_input after the segment has been
 * validated. Writing this as a chain of ifs over flags is exactly the shortcut
 * §4 warns about; the states are the structure.
 */
static void on_ack(u32 ack)
{
    if (seq_le(ack, snd_una)) {
        /* not new. A DUPLICATE ACK is information - it means the peer got
         * something out of order - but with no fast retransmit there is
         * nothing to do with it but count it. Counting is not nothing: it is
         * how "the link is lossy" becomes visible instead of inferred. */
        if (ack == snd_una && snd_nxt != snd_una) {
            c_dupack++;
            if (++dup_ack_run > 100) dup_ack_run = 100;
        }
        return;
    }
    if (seq_gt(ack, snd_nxt)) return;       /* acking what we never sent */

    u32 acked = ack - snd_una;
    dup_ack_run = 0;

    /* consume acknowledged bytes from the send buffer. Our FIN occupies a
     * sequence number but no byte, so it must not be subtracted from snd_len. */
    int data_acked = (int)acked;
    if (fin_sent && seq_ge(ack, fin_seq + 1)) {
        fin_acked = 1;
        if (data_acked > 0) data_acked--;   /* the FIN's sequence number */
    }
    if (data_acked > snd_len) data_acked = snd_len;
    if (data_acked > 0) {
        for (int i = 0; i + data_acked < snd_len; i++)
            sndbuf[i] = sndbuf[i + data_acked];
        snd_len -= data_acked;
    }

    snd_una = ack;
    last_ack = ack;

    /* slow start: one more segment in flight per acknowledgement */
    if (cwnd < 64) cwnd++;

    /* the timer runs while anything is unacknowledged, and only then */
    if (snd_una == snd_nxt) rt_deadline = 0;
    else arm_timer();

    pump_send();
}

static void st_syn_sent(u32 seqno, u32 ack, u8 flags, const u8 *data, int dlen)
{
    if (flags & F_RST) {
        /* A RST is only acceptable here if it acknowledges our SYN - otherwise
         * anyone who can guess the port can tear the connection down. */
        if ((flags & F_ACK) && ack == iss + 1) {
            st = TCP_CLOSED; c_rst++; rt_deadline = 0;
        }
        return;
    }
    if (!(flags & F_SYN)) return;
    if (flags & F_ACK) {
        if (ack != iss + 1) {               /* not for our SYN */
            send_seg(ack, F_RST, 0, 0);
            c_rst++;
            return;
        }
        irs = seqno;
        rcv_nxt = seqno + 1;                /* their SYN consumes one */
        snd_una = ack;
        st = TCP_ESTABLISHED;
        rt_deadline = 0;
        rto = RTO_MIN;
        send_seg(snd_nxt, F_ACK, 0, 0);
        if (dlen > 0) { deliver(data, dlen); send_seg(snd_nxt, F_ACK, 0, 0); }
        pump_send();
    }
    /* A bare SYN with no ACK is a simultaneous open. A client that never
     * listens cannot be in one legitimately, so it is dropped rather than
     * handled - SYN_RECEIVED does not exist here and inventing it to serve a
     * case that cannot occur is how a state machine grows unreachable states. */
}

/* Take the data out of a segment. Returns 1 if the segment was in sequence -
 * i.e. whether a FIN riding on it is ours to act on yet - and 0 if it went
 * into the out-of-order slot, where its FIN must wait for the hole to fill. */
static int take_data(u32 seqno, const u8 *data, int dlen)
{
    /* A ZERO-LENGTH SEGMENT IS IN SEQUENCE ONLY AT rcv_nxt. `seq_le` accepted
     * the entire backward half of the sequence space, so a bare FIN a million
     * bytes in the past - or a spoofed one - moved the connection to
     * CLOSE_WAIT and pushed rcv_nxt past the real stream. */
    if (dlen <= 0) return seqno == rcv_nxt;

    if (seq_lt(seqno, rcv_nxt)) {
        /* HOW FAR BACK, AND IS THAT EVEN POSSIBLE? seq_lt is modular, so a
         * segment 2^31 AHEAD reads as one 2^31 behind. Converting that
         * difference straight to an int gives INT_MIN, `dlen - skip`
         * overflows, and rcv_put is handed a negative length that drives
         * rcv_tail two gigabytes negative - after which an ordinary segment
         * writes outside the buffer entirely. Found by an adversarial review
         * and reproduced under ASan and UBSan before this line existed.
         *
         * A real retransmission overlaps by at most its own length. Anything
         * further back is ancient or forged: re-acknowledge it and take
         * nothing from it. */
        u32 back = rcv_nxt - seqno;
        if (back > (u32)dlen) {
            c_dup++;
            send_seg(snd_nxt, F_ACK, 0, 0);
            return 0;                        /* nothing new, and no FIN either */
        }
        /* Already taken. This is a RETRANSMISSION, not an error: the peer did
         * not see our ACK. Re-ACK immediately - staying silent is what turns
         * one lost ACK into a stalled connection. It may still carry NEW bytes
         * past rcv_nxt, so the overlap is delivered rather than the whole
         * segment discarded. */
        c_dup++;
        int skip = (int)back;
        if (skip < dlen) deliver(data + skip, dlen - skip);
        send_seg(snd_nxt, F_ACK, 0, 0);
        return 1;
    }
    if (seqno == rcv_nxt) {
        if (!deliver(data, dlen)) {          /* buffer full: refuse and say so */
            c_oow++;
            send_seg(snd_nxt, F_ACK, 0, 0);  /* the window we advertise is now 0 */
            return 0;
        }
        send_seg(snd_nxt, F_ACK, 0, 0);
        return 1;
    }
    /* past the hole. ONE slot, and the second is dropped and counted rather
     * than silently ignored. */
    if (!ooo_len && dlen <= OOO_BUF) {
        for (int i = 0; i < dlen; i++) ooo[i] = data[i];
        ooo_len = dlen;
        ooo_seq = seqno;
        ooo_fin = seg_has_fin;      /* a FIN on a held segment is not ours yet */
        c_ooo++;
    } else {
        c_oow++;
    }
    send_seg(snd_nxt, F_ACK, 0, 0);         /* duplicate ACK, on purpose */
    return 0;
}

/* A FIN OCCUPIES THE SEQUENCE NUMBER AFTER THE SEGMENT'S DATA, not the
 * segment's own. Comparing `seqno` against rcv_nxt works for a bare FIN and
 * fails for every FIN that carries data - because delivering the data has
 * already advanced rcv_nxt past seqno. An HTTP/1.0 server ends every response
 * with exactly that segment, so the bug loses the tail of every page and
 * leaves the connection open. Caught by the harness, which is the whole reason
 * §4 asks for one. */
static int take_fin(u32 seqno, int dlen, int in_seq)
{
    if (!in_seq) return 0;
    if (fin_seen) { send_seg(snd_nxt, F_ACK, 0, 0); return 0; }
    u32 end = seqno + (u32)dlen;
    if (!seq_le(end, rcv_nxt)) return 0;    /* still behind a hole */
    fin_seen = 1;
    rcv_nxt++;                              /* the FIN consumes one */
    send_seg(snd_nxt, F_ACK, 0, 0);
    return 1;
}

static void st_established(u32 seqno, u32 ack, u8 flags, const u8 *data, int dlen)
{
    if (flags & F_ACK) on_ack(ack);
    int in_seq = take_data(seqno, data, dlen);
    if ((flags & F_FIN) && take_fin(seqno, dlen, in_seq)) { st = TCP_CLOSE_WAIT; return; }
    /* ...or a FIN that arrived on an out-of-order segment, whose hole the
     * segment just delivered has now filled. Without this the last segment of
     * an HTTP/1.0 response - data and FIN together - is silently stripped of
     * its FIN whenever it arrives before the segment ahead of it. */
    if (ooo_fin_ready) {
        ooo_fin_ready = 0;
        fin_seen = 1;
        rcv_nxt++;
        send_seg(snd_nxt, F_ACK, 0, 0);
        st = TCP_CLOSE_WAIT;
    }
}

static void st_fin_wait_1(u32 seqno, u32 ack, u8 flags, const u8 *data, int dlen)
{
    if (flags & F_ACK) on_ack(ack);

    /* Data may still arrive after we have closed our half - a half close is
     * legal and an HTTP/1.0 server does exactly this. Dropping it here would
     * lose the tail of every response. */
    int in_seq = take_data(seqno, data, dlen);

    if ((flags & F_FIN) && take_fin(seqno, dlen, in_seq)) {
        /* Both directions closed. Which state depends on whether OUR FIN has
         * been acknowledged yet - that is the whole difference between CLOSING
         * and TIME_WAIT, and collapsing them closes early and leaves the peer
         * retransmitting into nothing. */
        if (fin_acked) { st = TCP_TIME_WAIT; tw_deadline = idt_ticks() + TIME_WAIT_TICKS; }
        else             st = TCP_CLOSING;
        return;
    }
    if (fin_acked) st = TCP_FIN_WAIT_2;
}

static void st_fin_wait_2(u32 seqno, u32 ack, u8 flags, const u8 *data, int dlen)
{
    if (flags & F_ACK) on_ack(ack);
    int in_seq = take_data(seqno, data, dlen);
    if ((flags & F_FIN) && take_fin(seqno, dlen, in_seq)) {
        st = TCP_TIME_WAIT;
        tw_deadline = idt_ticks() + TIME_WAIT_TICKS;
    }
}

void tcp_input(u32 src, int proto, const u8 *p, int len)
{
    if (proto != TCP_PROTO || len < 20) return;
    if (st == TCP_CLOSED) return;
    if (src != peer_ip) { c_oow++; return; }

    peer_ip = src;
    if (tcp_csum(p, len) != 0) { c_badsum++; return; }

    u16 sport = be16(p + 0), dport = be16(p + 2);
    if (sport != remote_port || dport != local_port) { c_oow++; return; }

    int doff = (p[12] >> 4) * 4;
    if (doff < 20 || doff > len) { c_oow++; return; }

    u32 seqno = be32(p + 4);
    u32 ack   = be32(p + 8);
    u8  flags = p[13];
    u16 win   = be16(p + 14);
    const u8 *data = p + doff;
    int dlen = len - doff;

    c_rx++;
    seg_has_fin = (flags & F_FIN) ? 1 : 0;
    /* A ZERO WINDOW MEANS STOP. Rewriting it to MSS pushed another segment
     * into a receiver that had just said it had no room. The persist probe
     * in tcp_tick is what stops that being a deadlock. */
    if (flags & F_ACK) snd_wnd = win;

    /* A RST tears the connection down from any state except SYN_SENT, where it
     * has to be validated first - see st_syn_sent. */
    /* A RST MUST BE IN THE RECEIVE WINDOW. `seq_ge(seqno, rcv_nxt)` accepted
     * the entire forward half of the sequence space - two billion values - so
     * an off-path packet with a guessed port had a one-in-two chance of
     * tearing the connection down. Measured at 50% by an adversarial review.
     * The `|| seqno == rcv_nxt` clause was dead: seq_ge already includes it,
     * and its presence is the clearest evidence that an in-window test was
     * intended and mis-written. */
    if ((flags & F_RST) && st != TCP_SYN_SENT) {
        u32 w = (u32)rcv_space();
        if (seq_ge(seqno, rcv_nxt) && seq_lt(seqno, rcv_nxt + (w ? w : 1))) {
            c_rst++;
            st = TCP_CLOSED;
            rt_deadline = 0;
        } else {
            c_oow++;
        }
        return;
    }

    switch (st) {
    case TCP_SYN_SENT:
        st_syn_sent(seqno, ack, flags, data, dlen);
        break;
    case TCP_ESTABLISHED:
        st_established(seqno, ack, flags, data, dlen);
        break;
    case TCP_FIN_WAIT_1:
        st_fin_wait_1(seqno, ack, flags, data, dlen);
        break;
    case TCP_FIN_WAIT_2:
        st_fin_wait_2(seqno, ack, flags, data, dlen);
        break;
    case TCP_CLOSING:
        if (flags & F_ACK) on_ack(ack);
        if (fin_acked) { st = TCP_TIME_WAIT; tw_deadline = idt_ticks() + TIME_WAIT_TICKS; }
        break;
    case TCP_CLOSE_WAIT:
        if (flags & F_ACK) on_ack(ack);
        /* A RETRANSMITTED DATA SEGMENT MUST STILL BE ACKNOWLEDGED. Without
         * take_data here the peer - which never saw our ACK - retransmits
         * until it gives up, and we answer nothing at all. */
        take_data(seqno, data, dlen);
        if (flags & F_FIN) send_seg(snd_nxt, F_ACK, 0, 0);
        break;
    case TCP_LAST_ACK:
        if (flags & F_ACK) on_ack(ack);
        if (fin_acked) { st = TCP_CLOSED; rt_deadline = 0; }
        break;
    case TCP_TIME_WAIT:
        /* A retransmitted FIN is re-ACKed and the wait restarts - that is what
         * TIME_WAIT is for. But only an IN-WINDOW one: refreshing on any FIN
         * at all lets an off-path packet every second hold the single
         * connection slot open forever, so nothing else can be opened. */
        if ((flags & F_FIN) && seqno == rcv_nxt - 1) {
            send_seg(snd_nxt, F_ACK, 0, 0);
            tw_deadline = idt_ticks() + TIME_WAIT_TICKS;
        } else {
            c_oow++;
        }
        break;
    default:
        break;
    }
}

/* ---- the timers -------------------------------------------------------------
 * Retransmission is the ONLY recovery mechanism here - no fast retransmit, no
 * SACK - so this is the whole of it. The backoff doubles and is capped; an
 * uncapped doubling reaches minutes and looks like a hang.
 */
void tcp_tick(void)
{
    u32 now = idt_ticks();

    if (st == TCP_TIME_WAIT) {
        if ((int)(now - tw_deadline) >= 0) { st = TCP_CLOSED; rt_deadline = 0; }
        return;
    }
    if (!rt_deadline || (int)(now - rt_deadline) < 0) return;

    if (st == TCP_SYN_SENT) {
        if (++syn_tries >= SYN_TRIES) {
            /* THE SYN-ACK THAT NEVER COMES. Give up and say so, rather than
             * retrying forever - a connect that never returns is the failure
             * mode a browser cannot recover from. */
            st = TCP_CLOSED;
            rt_deadline = 0;
            return;
        }
        send_seg(iss, F_SYN, 0, 0);
        c_rexmit++;
        rto = rto * 2 > RTO_MAX ? RTO_MAX : rto * 2;
        arm_timer();
        return;
    }

    /* A ZERO-WINDOW PERSIST PROBE. Honouring a zero window is only safe if
     * something eventually asks again: the peer's window update can itself be
     * lost, and then both ends wait forever. One byte is enough to draw out a
     * fresh advertisement. */
    if (snd_wnd == 0 && snd_len > 0 && snd_una == snd_nxt) {
        send_seg(snd_una, F_ACK, sndbuf, 1);
        rto = rto * 2 > RTO_MAX ? RTO_MAX : rto * 2;
        arm_timer();
        return;
    }

    /* DATA BEFORE THE FIN, and this order is the whole fix. The FIN branch
     * used to come first and fire whenever a FIN was unacknowledged - so if a
     * data segment was lost before the close, the timer resent the FIN
     * forever and never the data. The peer, missing bytes, could never
     * cumulatively acknowledge the FIN, so the connection wedged in
     * FIN_WAIT_1 permanently holding the one connection slot. Sixty seconds
     * produced sixty bare FINs and not one byte of data. */
    if (seq_lt(snd_una, snd_nxt) && snd_len > 0) {
        int outstanding = (int)(snd_nxt - snd_una);
        if (fin_sent) outstanding--;        /* the FIN is not a byte */
        if (outstanding > snd_len) outstanding = snd_len;
        if (outstanding > 0) {
            /* AND EVENTUALLY GIVE UP. Retransmitting forever is the other way
             * to hold the single connection slot open for good - the peer is
             * gone and nothing above ever finds out. Twelve attempts at a
             * backoff capped at four seconds is about half a minute. */
            if (++rexmit_tries > 12) { tcp_abort(); return; }
            int n = outstanding > MSS ? MSS : outstanding;
            send_seg(snd_una, F_ACK | F_PSH, sndbuf, n);
            c_rexmit++;
            /* GO BACK N, properly. The old comment claimed this rewind and
             * the code never performed it, so recovery resent one segment per
             * timeout however much was outstanding. */
            snd_nxt = snd_una + (u32)n;
            if (fin_sent) fin_sent = 0;     /* the FIN follows the data again */
            cwnd = 1;                       /* slow start, from the beginning */
            rto = rto * 2 > RTO_MAX ? RTO_MAX : rto * 2;
            arm_timer();
            return;
        }
    }

    if (fin_sent && !fin_acked &&
        (st == TCP_FIN_WAIT_1 || st == TCP_LAST_ACK || st == TCP_CLOSING)) {
        if (++fin_tries > SYN_TRIES) {      /* give up rather than hold the slot */
            st = TCP_CLOSED;
            rt_deadline = 0;
            return;
        }
        send_seg(fin_seq, F_FIN | F_ACK, 0, 0);
        c_rexmit++;
        rto = rto * 2 > RTO_MAX ? RTO_MAX : rto * 2;
        arm_timer();
        return;
    }

    rt_deadline = 0;
}

/* ---- accessors --------------------------------------------------------------- */
int tcp_state(void)            { return st; }
int tcp_rx_segs(void)          { return c_rx; }
int tcp_tx_segs(void)          { return c_tx; }
int tcp_retransmits(void)      { return c_rexmit; }
int tcp_rx_bad_csum(void)      { return c_badsum; }
int tcp_rx_out_of_window(void) { return c_oow; }
int tcp_rx_dup(void)           { return c_dup; }
int tcp_rx_ooo(void)           { return c_ooo; }
int tcp_dup_acks(void)         { return c_dupack; }
int tcp_resets(void)           { return c_rst; }
int tcp_cwnd(void)             { return cwnd; }
int tcp_rto(void)              { return rto; }
