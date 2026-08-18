/* nettest.c - ARP, IPv4 and ICMP against scripted packets, with no machine.
 *
 * net.c reaches for exactly three things outside itself - a send, a poll and a
 * clock - so replacing those makes it an ordinary Linux program. That is worth
 * far more here than it was for the layout engine, because the interesting
 * failures of a network stack are all invisible from a successful ping:
 *
 *   - a checksum that is only right for even-length payloads
 *   - an ARP reply we send to a request that was not for us
 *   - an echo reply matched on "the next ICMP packet" rather than on its
 *     sequence number, which puts every measurement one behind after the
 *     first loss and reports the WRONG number rather than no number
 *   - a stack that drops every fourth packet
 *
 * The last one is exactly what §4 item 2 warns about, and the only way to
 * assert it is to be the thing on the other end. So this harness is a
 * responder: it answers ARP and ICMP, and it can be told to lose one packet in
 * four or to vary its delay, which makes loss and jitter MEASURED numbers with
 * a known right answer rather than whatever the network happened to do.
 *
 * THE CLOCK IS VIRTUAL. Real time would make the jitter assertion flaky, which
 * is the property this project has already decided a gate must never have.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../net.h"

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

/* ---- the virtual clock ----------------------------------------------------
 * net.c asks for the TSC and the PIT. Both come from here, so a ping's round
 * trip is whatever the responder decides it is - to the microsecond.
 */
static unsigned long long v_tsc;
static unsigned int       v_ticks;
#define TSC_KHZ 1000u                     /* 1000 kHz -> 1 tick = 1 us */

unsigned long long cpu_tsc(void)    { return v_tsc; }
unsigned int       cpu_tsc_khz(void){ return TSC_KHZ; }
unsigned int       idt_ticks(void)  { return v_ticks; }

static void advance_us(unsigned n)
{
    v_tsc += (unsigned long long)n * TSC_KHZ / 1000u;
    /* the PIT runs at 100 Hz: one tick per 10,000 us */
    static unsigned carry;
    carry += n;
    while (carry >= 10000) { carry -= 10000; v_ticks++; }
}

/* ---- the fake link --------------------------------------------------------- */
#define QN 64
#define FR 1600

static unsigned char inq[QN][FR];
static int inq_len[QN], inq_head, inq_tail;

static unsigned char sent[QN][FR];
static int sent_len[QN], n_sent;

/* responder behaviour, set per test */
static int  resp_on;          /* answer ARP and ICMP                        */
static int  resp_drop_every;  /* 0 = never; 4 = lose one echo in four       */
static int  resp_delay_us;    /* base round trip                            */
static int  resp_jitter_us;   /* added to alternate replies                 */
static int  resp_seen;        /* echo requests observed                     */
static int  resp_alt;

static const unsigned char PEER_MAC[6] = { 0x52, 0x55, 0x0A, 0x00, 0x02, 0x02 };
static const unsigned char OUR_MAC[6]  = { 0x52, 0x54, 0x00, 0x12, 0x34, 0x56 };
#define OUR_IP  0x0A00020Fu               /* 10.0.2.15 */
#define GW_IP   0x0A000202u               /* 10.0.2.2  */
#define MASK    0xFFFFFF00u

static void inject(const unsigned char *f, int n)
{
    if ((inq_tail + 1) % QN == inq_head) return;      /* full: drop */
    memcpy(inq[inq_tail], f, n);
    inq_len[inq_tail] = n;
    inq_tail = (inq_tail + 1) % QN;
}

static unsigned short ck(const unsigned char *p, int n)
{
    return net_checksum(p, n, 0);
}

static void respond_to(const unsigned char *f, int n);

static int fake_send(const unsigned char *f, int len)
{
    if (n_sent < QN) { memcpy(sent[n_sent], f, len); sent_len[n_sent] = len; n_sent++; }
    if (resp_on) respond_to(f, len);
    return 1;
}

static int fake_poll(unsigned char *out, int max)
{
    advance_us(1);                        /* time passes even when idle */
    if (inq_head == inq_tail) return 0;
    int n = inq_len[inq_head];
    if (n > max) n = max;
    memcpy(out, inq[inq_head], n);
    inq_head = (inq_head + 1) % QN;
    return n;
}

/* The other end of the wire. Answers an ARP request for the gateway and an
 * echo request to it, with whatever loss and delay the test asked for. */
static void respond_to(const unsigned char *f, int n)
{
    if (n < 14) return;
    unsigned type = ((unsigned)f[12] << 8) | f[13];

    if (type == 0x0806 && n >= 42) {
        if (((unsigned)f[20] << 8 | f[21]) != 1) return;      /* not a request */
        unsigned target = ((unsigned)f[38] << 24) | ((unsigned)f[39] << 16) |
                          ((unsigned)f[40] << 8)  | f[41];
        if (target != GW_IP) return;
        unsigned char r[42];
        memset(r, 0, sizeof r);
        memcpy(r, f + 6, 6);                       /* to the sender */
        memcpy(r + 6, PEER_MAC, 6);
        r[12] = 0x08; r[13] = 0x06;
        r[14] = 0; r[15] = 1; r[16] = 0x08; r[17] = 0x00;
        r[18] = 6; r[19] = 4;
        r[20] = 0; r[21] = 2;                      /* reply */
        memcpy(r + 22, PEER_MAC, 6);
        r[28] = (unsigned char)(GW_IP >> 24); r[29] = (unsigned char)(GW_IP >> 16);
        r[30] = (unsigned char)(GW_IP >> 8);  r[31] = (unsigned char)GW_IP;
        memcpy(r + 32, f + 6, 6);
        memcpy(r + 38, f + 28, 4);
        inject(r, 42);
        return;
    }

    if (type != 0x0800 || n < 34) return;
    const unsigned char *ip = f + 14;
    if (ip[9] != 1) return;                        /* not ICMP */
    const unsigned char *ic = ip + (ip[0] & 0x0F) * 4;
    if (ic[0] != 8) return;                        /* not an echo request */

    resp_seen++;
    if (resp_drop_every && (resp_seen % resp_drop_every) == 0) return;  /* lost */

    int iclen = (((int)ip[2] << 8) | ip[3]) - (ip[0] & 0x0F) * 4;
    if (iclen < 8 || iclen > 512) return;

    /* the round trip the test asked for */
    int d = resp_delay_us + (resp_jitter_us && (resp_alt++ & 1) ? resp_jitter_us : 0);
    advance_us((unsigned)d);

    unsigned char r[FR];
    memset(r, 0, sizeof r);
    memcpy(r, f + 6, 6);
    memcpy(r + 6, PEER_MAC, 6);
    r[12] = 0x08; r[13] = 0x00;
    unsigned char *o = r + 14;
    o[0] = 0x45; o[1] = 0;
    o[2] = (unsigned char)((20 + iclen) >> 8); o[3] = (unsigned char)(20 + iclen);
    o[4] = 0; o[5] = 9; o[6] = 0x40; o[7] = 0;
    o[8] = 64; o[9] = 1;
    o[10] = 0; o[11] = 0;
    memcpy(o + 12, ip + 16, 4);                    /* from the target */
    memcpy(o + 16, ip + 12, 4);                    /* back to us      */
    unsigned short s = ck(o, 20);
    o[10] = (unsigned char)(s >> 8); o[11] = (unsigned char)s;

    unsigned char *e = o + 20;
    memcpy(e, ic, iclen);
    e[0] = 0;                                      /* echo reply */
    e[2] = 0; e[3] = 0;
    s = ck(e, iclen);
    e[2] = (unsigned char)(s >> 8); e[3] = (unsigned char)s;

    inject(r, 14 + 20 + iclen);
}

static void reset(int responder)
{
    inq_head = inq_tail = n_sent = 0;
    resp_on = responder;
    resp_drop_every = 0;
    resp_delay_us = 0;
    resp_jitter_us = 0;
    resp_seen = 0;
    resp_alt = 0;
    v_tsc = 0;
    v_ticks = 0;
    net_link(fake_send, fake_poll, OUR_MAC);
    net_config(OUR_IP, MASK, GW_IP);
}

/* ---- checksum -------------------------------------------------------------- */
static void t_checksum(void)
{
    printf("the internet checksum\n");
    /* RFC 1071's own worked example: the sum of these bytes is 0xDDF2, so the
     * checksum is its complement, 0x220D. A value from the specification, not
     * one this code produced. */
    unsigned char ex[] = { 0x00, 0x01, 0xF2, 0x03, 0xF4, 0xF5, 0xF6, 0xF7 };
    CHECK(net_checksum(ex, 8, 0) == 0x220D, "RFC 1071 example: got %04X",
          net_checksum(ex, 8, 0));

    /* a real IPv4 header with its checksum in place must sum to zero - that is
     * the property the receive path actually relies on */
    unsigned char h[20] = {
        0x45,0x00,0x00,0x54,0x00,0x09,0x40,0x00,0x40,0x01,
        0x00,0x00, 0x0A,0x00,0x02,0x0F, 0x0A,0x00,0x02,0x02
    };
    unsigned short s = net_checksum(h, 20, 0);
    h[10] = (unsigned char)(s >> 8); h[11] = (unsigned char)s;
    CHECK(net_checksum(h, 20, 0) == 0, "header with its own checksum != 0");

    /* ODD LENGTH. The tail byte is the HIGH half of the last word, and getting
     * that backwards is a checksum that is right for every even-length payload
     * and wrong for every odd one - which is most of them, in ICMP. */
    unsigned char odd[] = { 0x12, 0x34, 0x56 };
    unsigned char pad[] = { 0x12, 0x34, 0x56, 0x00 };
    CHECK(net_checksum(odd, 3, 0) == net_checksum(pad, 4, 0),
          "odd tail: %04X vs %04X", net_checksum(odd, 3, 0), net_checksum(pad, 4, 0));

    CHECK(net_checksum(0, 0, 0) == 0xFFFF, "empty checksum");
}

/* ---- ARP -------------------------------------------------------------------- */
static void arp_request_frame(unsigned char *r, unsigned target)
{
    memset(r, 0, 42);
    memset(r, 0xFF, 6);
    memcpy(r + 6, PEER_MAC, 6);
    r[12] = 0x08; r[13] = 0x06;
    r[14] = 0; r[15] = 1; r[16] = 0x08; r[17] = 0x00;
    r[18] = 6; r[19] = 4;
    r[20] = 0; r[21] = 1;
    memcpy(r + 22, PEER_MAC, 6);
    r[28] = (unsigned char)(GW_IP >> 24); r[29] = (unsigned char)(GW_IP >> 16);
    r[30] = (unsigned char)(GW_IP >> 8);  r[31] = (unsigned char)GW_IP;
    r[38] = (unsigned char)(target >> 24); r[39] = (unsigned char)(target >> 16);
    r[40] = (unsigned char)(target >> 8);  r[41] = (unsigned char)target;
}

static void t_arp(void)
{
    printf("ARP\n");
    unsigned char r[42];

    /* a request FOR US is answered, and the answer is a well-formed reply */
    reset(0);
    arp_request_frame(r, OUR_IP);
    inject(r, 42);
    net_poll_once();
    CHECK(n_sent == 1, "%d frames sent in reply to an ARP for us", n_sent);
    if (n_sent == 1) {
        unsigned char *s = sent[0];
        CHECK(sent_len[0] >= 42, "reply is %d bytes", sent_len[0]);
        CHECK(!memcmp(s, PEER_MAC, 6), "reply not addressed to the asker");
        CHECK(!memcmp(s + 6, OUR_MAC, 6), "reply not from our MAC");
        CHECK(s[12] == 0x08 && s[13] == 0x06, "reply is not ARP");
        CHECK(s[20] == 0 && s[21] == 2, "opcode %d, wanted 2 (reply)", s[21]);
        CHECK(!memcmp(s + 22, OUR_MAC, 6), "sender MAC is not ours");
        unsigned sip = ((unsigned)s[28] << 24) | ((unsigned)s[29] << 16) |
                       ((unsigned)s[30] << 8) | s[31];
        CHECK(sip == OUR_IP, "sender IP %08X, wanted %08X", sip, OUR_IP);
    }

    /* a request for SOMEONE ELSE must be silent. Answering it is how a host
     * poisons every ARP cache on the segment. */
    reset(0);
    arp_request_frame(r, 0x0A000201u);
    inject(r, 42);
    net_poll_once();
    CHECK(n_sent == 0, "replied to an ARP request that was not for us");

    /* ...but the sender is still learned, because it told us where it is */
    unsigned char mac[6];
    CHECK(net_arp_resolve(GW_IP, mac, 50), "sender not cached from its request");
    CHECK(!memcmp(mac, PEER_MAC, 6), "cached the wrong MAC");

    /* resolution over the wire, with the responder answering */
    reset(1);
    CHECK(net_arp_resolve(GW_IP, mac, 200), "ARP resolution timed out");
    CHECK(!memcmp(mac, PEER_MAC, 6), "resolved to the wrong MAC");

    /* an unanswered address must TIME OUT rather than hang or succeed */
    reset(1);
    CHECK(!net_arp_resolve(0x0A0002FEu, mac, 50), "resolved an address nobody answers");
}

/* ---- ICMP ------------------------------------------------------------------- */
static void echo_request_frame(unsigned char *r, int *outlen, unsigned to,
                               int id, int seq, int payload, int good_csum)
{
    int iclen = 8 + payload;
    memset(r, 0, FR);
    memcpy(r, OUR_MAC, 6);
    memcpy(r + 6, PEER_MAC, 6);
    r[12] = 0x08; r[13] = 0x00;
    unsigned char *o = r + 14;
    o[0] = 0x45;
    o[2] = (unsigned char)((20 + iclen) >> 8); o[3] = (unsigned char)(20 + iclen);
    o[6] = 0x40;
    o[8] = 64; o[9] = 1;
    o[12] = (unsigned char)(GW_IP >> 24); o[13] = (unsigned char)(GW_IP >> 16);
    o[14] = (unsigned char)(GW_IP >> 8);  o[15] = (unsigned char)GW_IP;
    o[16] = (unsigned char)(to >> 24); o[17] = (unsigned char)(to >> 16);
    o[18] = (unsigned char)(to >> 8);  o[19] = (unsigned char)to;
    unsigned short s = ck(o, 20);
    o[10] = (unsigned char)(s >> 8); o[11] = (unsigned char)s;

    unsigned char *e = o + 20;
    e[0] = 8;
    e[4] = (unsigned char)(id >> 8); e[5] = (unsigned char)id;
    e[6] = (unsigned char)(seq >> 8); e[7] = (unsigned char)seq;
    for (int i = 0; i < payload; i++) e[8 + i] = (unsigned char)(0x30 + (i & 15));
    s = ck(e, iclen);
    if (!good_csum) s ^= 0xFFFF;
    e[2] = (unsigned char)(s >> 8); e[3] = (unsigned char)s;

    *outlen = 14 + 20 + iclen;
}

static void t_icmp(void)
{
    printf("ICMP echo\n");
    unsigned char r[FR];
    int n;

    /* an echo request to us is answered, and the answer echoes the payload */
    reset(0);
    echo_request_frame(r, &n, OUR_IP, 0x1234, 7, 16, 1);
    inject(r, n);
    net_poll_once();
    CHECK(n_sent == 1, "%d frames sent for one echo request", n_sent);
    if (n_sent == 1) {
        unsigned char *e = sent[0] + 14 + 20;
        CHECK(e[0] == 0, "reply type %d, wanted 0", e[0]);
        CHECK(((e[4] << 8) | e[5]) == 0x1234, "id not echoed");
        CHECK(((e[6] << 8) | e[7]) == 7, "sequence not echoed");
        CHECK(e[8] == 0x30 && e[9] == 0x31, "payload not echoed");
        CHECK(net_checksum(e, 8 + 16, 0) == 0, "reply checksum is wrong");
        /* the IP header of the reply must also be valid, and go back where it
         * came from */
        unsigned char *ip = sent[0] + 14;
        CHECK(net_checksum(ip, 20, 0) == 0, "reply IP checksum is wrong");
        unsigned dst = ((unsigned)ip[16] << 24) | ((unsigned)ip[17] << 16) |
                       ((unsigned)ip[18] << 8) | ip[19];
        CHECK(dst == GW_IP, "reply sent to %08X, not the sender", dst);
    }

    /* a bad ICMP checksum is DROPPED and COUNTED. Answering it would make us
     * a reflector for corrupted packets. */
    reset(0);
    echo_request_frame(r, &n, OUR_IP, 0x1234, 8, 16, 0);
    inject(r, n);
    net_poll_once();
    CHECK(n_sent == 0, "answered an echo request with a bad checksum");
    CHECK(net_rx_bad_csum() == 1, "bad checksum not counted (%d)", net_rx_bad_csum());

    /* a packet for someone else is dropped and counted separately - "not ours"
     * and "corrupt" are different failures and a single counter hides which */
    reset(0);
    echo_request_frame(r, &n, 0x0A0002FEu, 0x1234, 9, 16, 1);
    inject(r, n);
    net_poll_once();
    CHECK(n_sent == 0, "answered an echo request addressed to someone else");
    CHECK(net_rx_not_ours() == 1, "not-ours not counted (%d)", net_rx_not_ours());

    /* a fragment is not handled, and must be dropped rather than parsed as if
     * it were whole */
    reset(0);
    echo_request_frame(r, &n, OUR_IP, 0x1234, 10, 16, 1);
    r[14 + 6] = 0x20;                     /* MF set, offset 0 */
    unsigned short s2 = 0;
    r[14 + 10] = 0; r[14 + 11] = 0;
    s2 = ck(r + 14, 20);
    r[14 + 10] = (unsigned char)(s2 >> 8); r[14 + 11] = (unsigned char)s2;
    inject(r, n);
    net_poll_once();
    CHECK(n_sent == 0, "answered a fragment");

    /* an ODD-length payload, because the checksum's tail byte is the thing
     * most likely to be wrong and every test above used an even one */
    reset(0);
    echo_request_frame(r, &n, OUR_IP, 0x1234, 11, 15, 1);
    inject(r, n);
    net_poll_once();
    CHECK(n_sent == 1, "odd-length echo request not answered");
    if (n_sent == 1) {
        unsigned char *e = sent[0] + 14 + 20;
        CHECK(net_checksum(e, 8 + 15, 0) == 0, "odd-length reply checksum is wrong");
    }
}

/* ---- malformed input -------------------------------------------------------- */
static void t_malformed(void)
{
    printf("malformed and truncated frames\n");
    reset(0);
    unsigned char r[FR];

    /* every truncation of a valid echo request, and every truncation of a
     * valid ARP request. None may fault and none may answer. */
    int n;
    echo_request_frame(r, &n, OUR_IP, 1, 1, 16, 1);
    for (int cut = 0; cut < n; cut++) {
        reset(0);
        inject(r, cut);
        net_poll_once();
        CHECK(n_sent == 0, "answered a %d-byte truncation of an echo request", cut);
    }

    unsigned char a[42];
    arp_request_frame(a, OUR_IP);
    for (int cut = 0; cut < 42; cut++) {
        reset(0);
        inject(a, cut);
        net_poll_once();
        CHECK(n_sent == 0, "answered a %d-byte truncation of an ARP request", cut);
    }

    /* garbage of every plausible length */
    reset(0);
    for (int len = 1; len < 200; len++) {
        unsigned char g[200];
        for (int i = 0; i < len; i++) g[i] = (unsigned char)(i * 37 + len);
        inject(g, len);
        net_poll_once();
    }
    CHECK(1, "garbage survived");          /* reaching here IS the assertion */

    /* an IPv4 header claiming an ihl smaller than the minimum, and one
     * claiming a total length longer than the frame */
    reset(0);
    echo_request_frame(r, &n, OUR_IP, 1, 1, 16, 1);
    r[14] = 0x43;                          /* ihl = 3 words = 12 bytes */
    inject(r, n);
    net_poll_once();
    CHECK(n_sent == 0, "accepted an ihl below the minimum");

    reset(0);
    echo_request_frame(r, &n, OUR_IP, 1, 1, 16, 1);
    r[14 + 2] = 0xFF; r[14 + 3] = 0xFF;    /* total length 65535 */
    r[14 + 10] = 0; r[14 + 11] = 0;
    unsigned short s = ck(r + 14, 20);
    r[14 + 10] = (unsigned char)(s >> 8); r[14 + 11] = (unsigned char)s;
    inject(r, n);
    net_poll_once();
    CHECK(1, "an over-long total length did not fault");
}

/* ---- ping, loss and jitter --------------------------------------------------- */
static void t_ping(void)
{
    printf("ping, loss and jitter\n");

    /* a clean run: 20 sent, 20 received, and the round trip is the one the
     * responder was told to produce */
    reset(1);
    resp_delay_us = 250;
    int got = net_ping_run(GW_IP, 20, 200);
    CHECK(got == 20, "%d of 20 replies", got);
    CHECK(net_ping_lost() == 0, "%d lost on a clean link", net_ping_lost());
    CHECK(net_ping_min() >= 250, "min %d us, responder delay was 250",
          net_ping_min());
    CHECK(net_ping_avg() >= 250 && net_ping_avg() < 400,
          "avg %d us is not near the 250 us the responder produced",
          net_ping_avg());
    CHECK(net_ping_jitter() < 50, "jitter %d us on a constant-delay link",
          net_ping_jitter());

    /* THE ONE §4 ITEM 2 IS ABOUT. One packet in four is lost. A single ping
     * would very likely have succeeded and reported a healthy link. */
    reset(1);
    resp_delay_us = 250;
    resp_drop_every = 4;
    got = net_ping_run(GW_IP, 20, 60);
    CHECK(got == 15, "%d of 20 with one-in-four loss, wanted 15", got);
    CHECK(net_ping_lost() == 5, "%d lost, wanted 5", net_ping_lost());

    /* ...and after a loss, the NEXT reply must still be matched correctly.
     * Matching on "the next ICMP reply" rather than on the sequence number
     * survives this test's loss count and reports wrong times. Alternating
     * delay makes that visible: every measurement would be one behind. */
    reset(1);
    resp_delay_us = 200;
    resp_jitter_us = 600;
    got = net_ping_run(GW_IP, 20, 200);
    CHECK(got == 20, "%d of 20 on a jittery link", got);
    CHECK(net_ping_jitter() > 300,
          "jitter %d us on a link alternating 200 and 800 us", net_ping_jitter());
    CHECK(net_ping_max() - net_ping_min() > 300,
          "min %d max %d on an alternating link", net_ping_min(), net_ping_max());

    /* an address nobody answers must report total loss, not hang */
    reset(1);
    got = net_ping_run(0x0A0002FEu, 3, 30);
    CHECK(got == 0, "%d replies from an address nobody answers", got);
    CHECK(net_ping_lost() == 3, "%d lost of 3", net_ping_lost());
}

/* ---- the sink ---------------------------------------------------------------- */
static int sink_hits, sink_proto, sink_len;
static void sink(net_u32 src, int proto, const net_u8 *p, int len)
{
    (void)src; (void)p;
    sink_hits++; sink_proto = proto; sink_len = len;
}

static void t_sink(void)
{
    printf("the TCP hand-off\n");
    reset(0);
    net_set_ip_sink(sink);
    sink_hits = 0;

    unsigned char r[FR];
    int n;
    echo_request_frame(r, &n, OUR_IP, 1, 1, 20, 1);
    r[14 + 9] = 6;                         /* protocol: TCP */
    r[14 + 10] = 0; r[14 + 11] = 0;
    unsigned short s = ck(r + 14, 20);
    r[14 + 10] = (unsigned char)(s >> 8); r[14 + 11] = (unsigned char)s;
    inject(r, n);
    net_poll_once();
    CHECK(sink_hits == 1, "TCP not delivered to the sink (%d)", sink_hits);
    CHECK(sink_proto == 6, "sink got proto %d", sink_proto);
    CHECK(sink_len == 28, "sink got %d bytes, wanted 28", sink_len);
    CHECK(n_sent == 0, "net.c answered a TCP packet itself");
    net_set_ip_sink(0);
}

int main(void)
{
    printf("net.c against scripted packets, no machine\n\n");
    t_checksum();
    t_arp();
    t_icmp();
    t_malformed();
    t_ping();
    t_sink();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
