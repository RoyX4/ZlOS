/* tcptest.c - the TCP state machine against scripted packet sequences.
 *
 * §4 item 3: "a host harness driving the state machine against scripted packet
 * sequences with no QEMU. Include the ugly ones: a SYN-ACK that never comes, a
 * duplicate ACK, a FIN mid-transfer, a segment arriving twice. This is exactly
 * the code where a harness beats a boot by an order of magnitude."
 *
 * It does, and by more than that. Every case below is one that either cannot
 * be produced on demand from a real peer (a segment arriving twice, a FIN in
 * the middle of a transfer, a RST with the wrong sequence number) or takes
 * minutes of wall clock to reach (five SYN retransmissions with exponential
 * backoff). Here they are instant and deterministic, because the harness IS
 * the peer and the clock is a variable.
 *
 * THE SEQUENCE-WRAP TEST IS THE ONE TO KEEP. TCP compares sequence numbers
 * modulo 2^32, and `a < b` on two u32s is correct until the numbers cross
 * 0x80000000 and then silently inverts. On a fast connection that is about an
 * hour in - far past when anyone is still watching - and the symptom is a peer
 * that appears to send garbage. Starting a connection just below the wrap
 * turns a bug nobody would find into one line of output.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

/* ---- the clock, as a variable ---------------------------------------------- */
static unsigned int v_ticks;
unsigned int idt_ticks(void) { return v_ticks; }
/* net.c wants these too; it is linked for net_checksum alone */
unsigned long long cpu_tsc(void) { return 0; }
unsigned int cpu_tsc_khz(void)   { return 0; }

/* ---- captured segments ------------------------------------------------------ */
#define CAP 64
struct cap {
    unsigned int seq, ack;
    unsigned char flags;
    int dlen;
    unsigned char data[1600];
};
static struct cap cap[CAP];
static int ncap;

#define LOCAL_IP 0x0A00020Fu
#define PEER_IP  0x0A000202u
#define PORT     80

static int capture(net_u32 dst, int proto, const net_u8 *p, int len)
{
    (void)dst; (void)proto;
    if (ncap >= CAP || len < 20) return 1;
    struct cap *c = &cap[ncap++];
    c->seq = ((unsigned)p[4] << 24) | ((unsigned)p[5] << 16) |
             ((unsigned)p[6] << 8) | p[7];
    c->ack = ((unsigned)p[8] << 24) | ((unsigned)p[9] << 16) |
             ((unsigned)p[10] << 8) | p[11];
    c->flags = p[13];
    int doff = (p[12] >> 4) * 4;
    c->dlen = len - doff;
    if (c->dlen > 0 && c->dlen < (int)sizeof c->data)
        memcpy(c->data, p + doff, c->dlen);
    else if (c->dlen < 0) c->dlen = 0;
    return 1;
}

static struct cap  none;
/* never NULL: a test that reads a segment which was not sent should FAIL an
 * assertion, not segfault the harness and lose every result after it. */
static struct cap *last(void)      { return ncap ? &cap[ncap - 1] : &none; }
static struct cap *nth(int i)      { return (i >= 0 && i < ncap) ? &cap[i] : &none; }

#define F_FIN 0x01
#define F_SYN 0x02
#define F_RST 0x04
#define F_PSH 0x08
#define F_ACK 0x10

/* Build a segment as the peer and hand it to tcp_input. The checksum uses the
 * same pseudo-header the driver computes - the address pair sums the same in
 * either direction, which is why one helper serves both. */
static void inject_bad(unsigned seq, unsigned ack, unsigned char flags,
                       const unsigned char *data, int dlen,
                       int sport, int dport, int corrupt)
{
    unsigned char s[1700];
    memset(s, 0, sizeof s);
    s[0] = (unsigned char)(sport >> 8); s[1] = (unsigned char)sport;
    s[2] = (unsigned char)(dport >> 8); s[3] = (unsigned char)dport;
    s[4] = (unsigned char)(seq >> 24); s[5] = (unsigned char)(seq >> 16);
    s[6] = (unsigned char)(seq >> 8);  s[7] = (unsigned char)seq;
    s[8] = (unsigned char)(ack >> 24); s[9] = (unsigned char)(ack >> 16);
    s[10] = (unsigned char)(ack >> 8); s[11] = (unsigned char)ack;
    s[12] = 5 << 4;
    s[13] = flags;
    s[14] = 0xFF; s[15] = 0xFF;              /* a wide window */
    if (dlen > 0) memcpy(s + 20, data, dlen);

    unsigned sum = 0;
    sum += (LOCAL_IP >> 16) & 0xFFFF; sum += LOCAL_IP & 0xFFFF;
    sum += (PEER_IP >> 16) & 0xFFFF;  sum += PEER_IP & 0xFFFF;
    sum += 6;
    sum += (unsigned)(20 + dlen);
    unsigned short ck = net_checksum(s, 20 + dlen, sum);
    if (corrupt) ck ^= 0xFFFF;
    s[16] = (unsigned char)(ck >> 8); s[17] = (unsigned char)ck;

    tcp_input(PEER_IP, 6, s, 20 + dlen);
}

/* the local port the stack chose, read back from the SYN it sent */
static int lport;

static void inject(unsigned seq, unsigned ack, unsigned char flags,
                   const unsigned char *data, int dlen)
{
    inject_bad(seq, ack, flags, data, dlen, PORT, lport, 0);
}

/* ---- open a connection and get to ESTABLISHED -------------------------------
 * EVERY TEST ABORTS FIRST. tcp_connect refuses while a connection is open -
 * correctly, there is exactly one slot - so without this the second test and
 * everything after it silently runs against the FIRST test's connection, with
 * its sequence numbers. That produced a page of failures that all looked like
 * state-machine bugs and were all one missing line here.
 */
static unsigned peer_isn;

/* tcp.c does not expose its local port, so read it out of the SYN it sent.
 * Reaching into the captured bytes is exactly what a harness is for. */
static unsigned char syn_sport_hi, syn_sport_lo;
static int capture_port(const net_u8 *p) { syn_sport_hi = p[0]; syn_sport_lo = p[1]; return 0; }

static int capture2(net_u32 dst, int proto, const net_u8 *p, int len)
{
    if (len >= 20 && ncap == 0) capture_port(p);
    return capture(dst, proto, p, len);
}

static unsigned start(unsigned their_isn)
{
    tcp_attach(capture2, LOCAL_IP);
    tcp_abort();                          /* release the single slot */
    ncap = 0;                             /* ...and do not capture its RST */
    v_ticks = 1000;
    int ok = tcp_connect(PEER_IP, PORT);
    if (!ok) { printf("  FATAL: tcp_connect refused (state %s)\n",
                      tcp_state_name(tcp_state())); exit(2); }
    lport = (syn_sport_hi << 8) | syn_sport_lo;
    peer_isn = their_isn;
    return their_isn;
}

static unsigned our_isn(void) { return nth(0) ? nth(0)->seq : 0; }

static void establish(unsigned their_isn)
{
    start(their_isn);
    inject(their_isn, our_isn() + 1, F_SYN | F_ACK, 0, 0);
}

/* ---- the tests --------------------------------------------------------------- */
static void t_open(void)
{
    printf("the active open\n");
    start(0x11110000u);
    CHECK(tcp_state() == TCP_SYN_SENT, "state %s after connect", tcp_state_name(tcp_state()));
    CHECK(ncap == 1, "%d segments sent for a connect", ncap);
    CHECK(last() && last()->flags == F_SYN, "flags %02X, wanted a bare SYN",
          last() ? last()->flags : 0);
    CHECK(last() && last()->dlen == 0, "the SYN carried data");

    inject(peer_isn, our_isn() + 1, F_SYN | F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_ESTABLISHED, "state %s after SYN-ACK",
          tcp_state_name(tcp_state()));
    CHECK(ncap == 2, "%d segments after the handshake", ncap);
    CHECK(last() && (last()->flags & F_ACK), "the handshake ACK was not sent");
    CHECK(last() && last()->ack == peer_isn + 1,
          "ACK %08X, wanted their ISN + 1 (%08X) - their SYN consumes a sequence number",
          last() ? last()->ack : 0, peer_isn + 1);
    CHECK(last() && last()->seq == our_isn() + 1,
          "our sequence did not advance past our own SYN");
}

static void t_syn_never_answered(void)
{
    printf("the SYN-ACK that never comes\n");
    start(0x22220000u);
    int syns = 1;
    /* run the clock far past every backoff. The connection must END, not
     * retry forever - a connect that never returns is the failure a browser
     * cannot recover from. */
    for (int i = 0; i < 200 && tcp_state() == TCP_SYN_SENT; i++) {
        v_ticks += 50;
        tcp_tick();
    }
    syns = ncap;
    CHECK(tcp_state() == TCP_CLOSED, "state %s - it never gave up",
          tcp_state_name(tcp_state()));
    CHECK(syns >= 3 && syns <= 8, "%d SYNs sent; wanted a handful with backoff", syns);
    CHECK(tcp_retransmits() > 0, "no retransmissions counted");
    /* every one of them must be a SYN with the same sequence number */
    for (int i = 0; i < ncap; i++) {
        CHECK(nth(i)->flags & F_SYN, "segment %d is not a SYN", i);
        CHECK(nth(i)->seq == nth(0)->seq, "SYN %d used a different sequence number", i);
    }
}

static void t_data_in_order(void)
{
    printf("data, in order\n");
    establish(0x33330000u);
    unsigned char buf[64];

    inject(peer_isn + 1, our_isn() + 1, F_ACK | F_PSH, (const unsigned char *)"hello ", 6);
    CHECK(tcp_available() == 6, "%d bytes available after 6 arrived", tcp_available());
    CHECK(last() && last()->ack == peer_isn + 7, "ACK %08X, wanted %08X",
          last() ? last()->ack : 0, peer_isn + 7);

    inject(peer_isn + 7, our_isn() + 1, F_ACK | F_PSH, (const unsigned char *)"world", 5);
    CHECK(tcp_available() == 11, "%d bytes after 11 arrived", tcp_available());

    int n = tcp_recv(buf, sizeof buf);
    buf[n] = 0;
    CHECK(n == 11 && !memcmp(buf, "hello world", 11), "got '%.*s'", n, buf);
    CHECK(tcp_available() == 0, "buffer not drained");
}

static void t_segment_twice(void)
{
    printf("a segment arriving twice\n");
    establish(0x44440000u);
    unsigned char buf[64];

    inject(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"abcd", 4);
    int acks_before = ncap;
    /* THE SAME SEGMENT AGAIN. The peer did not see our ACK. It must be
     * delivered ONCE and acknowledged AGAIN - staying silent is what turns one
     * lost ACK into a stalled connection. */
    inject(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"abcd", 4);

    CHECK(tcp_available() == 4, "%d bytes - the duplicate was delivered twice",
          tcp_available());
    CHECK(ncap > acks_before, "the duplicate was not re-acknowledged");
    CHECK(tcp_rx_dup() >= 1, "the duplicate was not counted");
    CHECK(last() && last()->ack == peer_isn + 5, "re-ACK is wrong: %08X",
          last() ? last()->ack : 0);

    /* a PARTIAL overlap: bytes we have plus bytes we do not */
    inject(peer_isn + 3, our_isn() + 1, F_ACK, (const unsigned char *)"cdef", 4);
    CHECK(tcp_available() == 6, "%d bytes after an overlapping segment",
          tcp_available());
    int n = tcp_recv(buf, sizeof buf);
    CHECK(n == 6 && !memcmp(buf, "abcdef", 6), "got '%.*s' - overlap mishandled", n, buf);
}

static void t_out_of_order(void)
{
    printf("out of order, and the one hole\n");
    establish(0x55550000u);
    unsigned char buf[64];

    /* the SECOND segment first */
    inject(peer_isn + 5, our_isn() + 1, F_ACK, (const unsigned char *)"WORLD", 5);
    CHECK(tcp_available() == 0, "out-of-order data was delivered early");
    CHECK(tcp_rx_ooo() == 1, "the out-of-order segment was not held");

    /* now the hole fills, and BOTH must appear, in order */
    inject(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"hell", 4);
    CHECK(tcp_available() == 9, "%d bytes after the hole filled", tcp_available());
    int n = tcp_recv(buf, sizeof buf);
    CHECK(n == 9 && !memcmp(buf, "hellWORLD", 9), "got '%.*s'", n, buf);

    /* a SECOND hole must be dropped and counted, not silently ignored */
    establish(0x56560000u);
    inject(peer_isn + 10, our_isn() + 1, F_ACK, (const unsigned char *)"xx", 2);
    inject(peer_isn + 20, our_isn() + 1, F_ACK, (const unsigned char *)"yy", 2);
    CHECK(tcp_rx_ooo() >= 1, "no out-of-order segment held");
    CHECK(tcp_rx_out_of_window() >= 1, "the second hole was not counted as dropped");
}

static void t_duplicate_ack(void)
{
    printf("a duplicate ACK\n");
    establish(0x66660000u);
    tcp_send((const unsigned char *)"0123456789", 10);
    int sent = ncap;
    unsigned una = our_isn() + 1;

    inject(peer_isn + 1, una, F_ACK, 0, 0);       /* acks nothing new */
    inject(peer_isn + 1, una, F_ACK, 0, 0);
    inject(peer_isn + 1, una, F_ACK, 0, 0);
    CHECK(tcp_dup_acks() >= 3, "%d duplicate ACKs counted, wanted 3", tcp_dup_acks());
    /* NO fast retransmit: the brief scopes it out, so three duplicate ACKs
     * must NOT trigger one. Asserting the absence is how the scope stays real
     * rather than aspirational. */
    CHECK(ncap == sent, "%d extra segments - something retransmitted on dup ACKs",
          ncap - sent);
    CHECK(tcp_state() == TCP_ESTABLISHED, "state %s", tcp_state_name(tcp_state()));
}

static void t_fin_mid_transfer(void)
{
    printf("a FIN in the middle of a transfer\n");
    establish(0x77770000u);
    unsigned char buf[64];

    inject(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"body", 4);
    /* data AND a FIN in one segment - which is what an HTTP/1.0 server does at
     * the end of a response, and the case that loses the tail of every page if
     * the FIN is handled before the data */
    inject(peer_isn + 5, our_isn() + 1, F_ACK | F_FIN, (const unsigned char *)"tail", 4);

    CHECK(tcp_available() == 8, "%d bytes - the FIN ate the data it carried",
          tcp_available());
    int n = tcp_recv(buf, sizeof buf);
    CHECK(n == 8 && !memcmp(buf, "bodytail", 8), "got '%.*s'", n, buf);
    CHECK(tcp_state() == TCP_CLOSE_WAIT, "state %s after their FIN",
          tcp_state_name(tcp_state()));
    CHECK(last() && last()->ack == peer_isn + 10,
          "ACK %08X - a FIN consumes a sequence number too", last() ? last()->ack : 0);

    /* and we can still close our half */
    tcp_close();
    CHECK(tcp_state() == TCP_LAST_ACK, "state %s after our close",
          tcp_state_name(tcp_state()));
    CHECK(last() && (last()->flags & F_FIN), "no FIN sent");
    inject(peer_isn + 10, last()->seq + 1, F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_CLOSED, "state %s after our FIN was acked",
          tcp_state_name(tcp_state()));
}

static void t_close_sequence(void)
{
    printf("closing, both orders\n");

    /* we close first: FIN_WAIT_1 -> FIN_WAIT_2 -> TIME_WAIT -> CLOSED */
    establish(0x88880000u);
    tcp_close();
    CHECK(tcp_state() == TCP_FIN_WAIT_1, "state %s", tcp_state_name(tcp_state()));
    unsigned our_fin = last()->seq;
    inject(peer_isn + 1, our_fin + 1, F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_FIN_WAIT_2, "state %s after our FIN was acked",
          tcp_state_name(tcp_state()));
    inject(peer_isn + 1, our_fin + 1, F_ACK | F_FIN, 0, 0);
    CHECK(tcp_state() == TCP_TIME_WAIT, "state %s after their FIN",
          tcp_state_name(tcp_state()));
    v_ticks += 1000;
    tcp_tick();
    CHECK(tcp_state() == TCP_CLOSED, "TIME_WAIT never expired (%s)",
          tcp_state_name(tcp_state()));

    /* SIMULTANEOUS close: their FIN arrives before our FIN is acknowledged.
     * That is CLOSING, and collapsing it into TIME_WAIT closes early and
     * leaves the peer retransmitting into nothing. */
    establish(0x99990000u);
    tcp_close();
    our_fin = last()->seq;
    inject(peer_isn + 1, our_isn() + 1, F_ACK | F_FIN, 0, 0);   /* no ack of our FIN */
    CHECK(tcp_state() == TCP_CLOSING, "state %s on a simultaneous close",
          tcp_state_name(tcp_state()));
    inject(peer_isn + 2, our_fin + 1, F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_TIME_WAIT, "state %s after our FIN was finally acked",
          tcp_state_name(tcp_state()));

    /* data arriving after WE closed must still be delivered - a half close is
     * legal and an HTTP/1.0 server relies on it */
    establish(0x9A9A0000u);
    tcp_close();
    our_fin = last()->seq;
    inject(peer_isn + 1, our_fin + 1, F_ACK, (const unsigned char *)"late", 4);
    CHECK(tcp_available() == 4, "data after our close was dropped (%d bytes)",
          tcp_available());
}

static void t_reset(void)
{
    printf("resets\n");
    establish(0xAAAA0000u);
    inject(peer_isn + 1, our_isn() + 1, F_RST | F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_CLOSED, "a RST did not close the connection (%s)",
          tcp_state_name(tcp_state()));
    CHECK(tcp_resets() >= 1, "the reset was not counted");

    /* a RST in SYN_SENT that does NOT acknowledge our SYN must be IGNORED -
     * otherwise anyone who can guess the port can refuse the connection */
    start(0xABAB0000u);
    inject(peer_isn, 0x12345678u, F_RST | F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_SYN_SENT,
          "a RST with the wrong acknowledgement killed the connect (%s)",
          tcp_state_name(tcp_state()));
    /* ...and one that does acknowledge it must be honoured */
    inject(peer_isn, our_isn() + 1, F_RST | F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_CLOSED, "a valid RST was ignored (%s)",
          tcp_state_name(tcp_state()));
}

static void t_rejects(void)
{
    printf("segments that must be refused\n");
    establish(0xBBBB0000u);
    int before = tcp_available();

    /* a corrupt checksum */
    inject_bad(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"junk", 4,
               PORT, lport, 1);
    CHECK(tcp_available() == before, "a segment with a bad checksum was accepted");
    CHECK(tcp_rx_bad_csum() >= 1, "bad checksum not counted");

    /* the wrong source port */
    inject_bad(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"junk", 4,
               PORT + 1, lport, 0);
    CHECK(tcp_available() == before, "a segment from the wrong port was accepted");

    /* the wrong destination port */
    inject_bad(peer_isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"junk", 4,
               PORT, lport + 1, 0);
    CHECK(tcp_available() == before, "a segment to the wrong port was accepted");

    /* an ACK for something we never sent */
    inject(peer_isn + 1, our_isn() + 900000u, F_ACK, 0, 0);
    CHECK(tcp_state() == TCP_ESTABLISHED, "an impossible ACK broke the connection");

    /* a runt - shorter than a TCP header */
    unsigned char runt[8] = { 0 };
    tcp_input(PEER_IP, 6, runt, 8);
    CHECK(tcp_state() == TCP_ESTABLISHED, "a runt segment broke the connection");

    /* a data offset pointing past the end of the segment */
    unsigned char bad[24];
    memset(bad, 0, sizeof bad);
    bad[12] = 15 << 4;                     /* 60 bytes of header in a 24-byte segment */
    tcp_input(PEER_IP, 6, bad, 24);
    CHECK(tcp_state() == TCP_ESTABLISHED, "an impossible data offset broke the connection");

    /* the right ports from the WRONG HOST */
    unsigned char s[20];
    memset(s, 0, sizeof s);
    tcp_input(0x0A0002FEu, 6, s, 20);
    CHECK(tcp_state() == TCP_ESTABLISHED, "a segment from another host was accepted");
}

static void t_retransmit(void)
{
    printf("retransmission on timeout\n");
    establish(0xCCCC0000u);
    tcp_send((const unsigned char *)"payload", 7);
    int sent = ncap;
    CHECK(sent >= 2, "the data was never sent");
    unsigned first_seq = last()->seq;
    int cw = tcp_cwnd();

    /* no acknowledgement, ever. The timer must resend the SAME bytes from the
     * SAME sequence number - a go-back-N, which is what a stack with no SACK
     * has - and collapse the congestion window. */
    v_ticks += 500;
    tcp_tick();
    CHECK(ncap > sent, "nothing was retransmitted");
    CHECK(last() && last()->seq == first_seq,
          "retransmitted from %08X, wanted %08X", last() ? last()->seq : 0, first_seq);
    CHECK(last() && last()->dlen == 7, "retransmitted %d bytes, wanted 7",
          last() ? last()->dlen : 0);
    CHECK(tcp_retransmits() >= 1, "the retransmission was not counted");
    CHECK(tcp_cwnd() <= cw, "the congestion window did not collapse (%d -> %d)",
          cw, tcp_cwnd());

    /* the backoff must GROW, and must be capped rather than doubling forever */
    int r1 = tcp_rto();
    v_ticks += 1000; tcp_tick();
    v_ticks += 1000; tcp_tick();
    CHECK(tcp_rto() > r1, "the retransmit timeout did not back off");
    for (int i = 0; i < 40; i++) { v_ticks += 100000; tcp_tick(); }
    CHECK(tcp_rto() <= 400, "the backoff is uncapped: %d", tcp_rto());

    /* and once acknowledged, the timer stops */
    establish(0xCDCD0000u);
    tcp_send((const unsigned char *)"x", 1);
    unsigned seqx = last()->seq;
    inject(peer_isn + 1, seqx + 1, F_ACK, 0, 0);
    int after = ncap;
    v_ticks += 100000;
    tcp_tick();
    CHECK(ncap == after, "it retransmitted data that had been acknowledged");
}

static void t_sequence_wrap(void)
{
    printf("the sequence number wrap\n");
    /* Their ISN is 16 bytes below the wrap, so the third segment's sequence
     * number is NUMERICALLY SMALLER than the first's. Every ordering decision
     * in tcp.c has to be modular for this to work, and `a < b` on two u32s -
     * the natural thing to write - fails here and nowhere else. */
    unsigned isn = 0xFFFFFFF0u;
    establish(isn);
    unsigned char buf[64];

    inject(isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"AAAA", 4);   /* ...F1 */
    inject(isn + 5, our_isn() + 1, F_ACK, (const unsigned char *)"BBBB", 4);   /* ...F5 */
    inject(isn + 9, our_isn() + 1, F_ACK, (const unsigned char *)"CCCCCCCC", 8); /* wraps */
    CHECK(tcp_available() == 16, "%d of 16 bytes across the wrap", tcp_available());
    int n = tcp_recv(buf, sizeof buf);
    CHECK(n == 16 && !memcmp(buf, "AAAABBBBCCCCCCCC", 16),
          "got '%.*s' across the sequence wrap", n, buf);

    /* a segment BEFORE the wrap must still be recognised as a duplicate after
     * it - which is the comparison that inverts if it is not modular */
    inject(isn + 1, our_isn() + 1, F_ACK, (const unsigned char *)"AAAA", 4);
    CHECK(tcp_available() == 0, "a pre-wrap duplicate was delivered as new data");
    CHECK(tcp_rx_dup() >= 1, "the pre-wrap duplicate was not counted");
}

static void t_send_path(void)
{
    printf("sending\n");
    establish(0xDDDD0000u);

    /* more than one segment's worth must be split, not truncated or dropped */
    static unsigned char big[4000];
    for (int i = 0; i < (int)sizeof big; i++) big[i] = (unsigned char)('A' + (i % 26));
    int taken = tcp_send(big, (int)sizeof big);
    CHECK(taken > 0, "tcp_send accepted nothing");
    CHECK(taken <= (int)sizeof big, "tcp_send claimed more than it was given");

    int total = 0;
    for (int i = 0; i < ncap; i++) total += nth(i)->dlen;
    CHECK(total > 0, "nothing went out");
    for (int i = 0; i < ncap; i++)
        CHECK(nth(i)->dlen <= 1400, "segment %d is %d bytes, over the MSS",
              i, nth(i)->dlen);

    /* sending before the connection is up must be refused, not queued into a
     * connection that may never exist */
    tcp_abort();
    CHECK(tcp_send((const unsigned char *)"x", 1) == 0,
          "tcp_send accepted data on a closed connection");
    CHECK(tcp_state() == TCP_CLOSED, "state %s", tcp_state_name(tcp_state()));
}

int main(void)
{
    printf("tcp.c against scripted packet sequences, no QEMU\n\n");
    t_open();
    t_syn_never_answered();
    t_data_in_order();
    t_segment_twice();
    t_out_of_order();
    t_duplicate_ack();
    t_fin_mid_transfer();
    t_close_sequence();
    t_reset();
    t_rejects();
    t_retransmit();
    t_sequence_wrap();
    t_send_path();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
