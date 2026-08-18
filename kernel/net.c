/* net.c - ARP, IPv4 and ICMP. No link driver, on purpose.
 *
 * THE LINK IS TWO FUNCTION POINTERS (net.h). That is not indirection for its
 * own sake: it is what lets hosttest/nettest.c drive this file with scripted
 * frames on a Linux box, which is the only way to test the cases that matter.
 * A stack that works once and drops every fourth packet looks identical to a
 * working one on a single ping, and a malformed header is not something QEMU
 * will send you on request.
 *
 * BYTE ORDER IS EXPLICIT EVERYWHERE. There is no htons here and there is not
 * going to be one: every multi-byte field is assembled and taken apart a byte
 * at a time, big-endian, at the point of use. A macro would hide the one thing
 * in this file most likely to be wrong, and the kernel has no <arpa/inet.h> to
 * borrow a correct one from.
 *
 * NOTHING BLOCKS. net_poll_once() takes at most one frame and returns.
 * Everything that waits - ARP resolution, a ping - is a bounded loop around
 * it, so the day this moves into the compositor's frame loop it does not need
 * rewriting. Same inversion ui.h imposes on apps, one layer down.
 *
 * WHAT IS NOT HERE, deliberately: no IP fragmentation (reassembly is a table
 * and a timer for a case a 1500-byte MTU on a local link does not produce),
 * no routing table beyond "on my subnet or via the gateway", no DHCP - the
 * address is asserted, not leased, and item 2's gate says so out loud.
 */

#include "net.h"

typedef net_u8  u8;
typedef net_u16 u16;
typedef net_u32 u32;

/* the clock. cpu.c calibrates the TSC against the PIT once and caches it, so
 * this is real microseconds rather than a spin count. Weakly referenced
 * through a wrapper below so the host harness can supply its own. */
unsigned long long cpu_tsc(void);
unsigned int       cpu_tsc_khz(void);
unsigned int       idt_ticks(void);

#define ETH_HDR   14
#define FRAME_MAX 1514
#define ARP_N     8

static net_send_fn link_send;
static net_poll_fn link_poll;
static u8  my_mac[6];
static u32 my_ip, my_mask, my_gw;

static struct { u32 ip; u8 mac[6]; int valid; } arp_tab[ARP_N];
static int arp_next;

static int c_rx, c_arp, c_ip, c_icmp, c_badsum, c_notours, c_tx;
/* echo replies that arrived and did NOT match what we were waiting for. This
 * is the counter that separates "the reply never came" from "the reply came
 * and we did not recognise it", which are opposite bugs with identical
 * symptoms - a ping that reports a loss. */
static int c_echo_stale;
/* EVERY DROP PATH GETS A COUNTER. This is not bookkeeping: handle_ip used to
 * drop a short or malformed header with a bare `return`, and one such frame
 * per bring-up cost six wrong hypotheses to find - the TX ring, the RX ring
 * size, the sequence number, a second queue consumer, the peer's ARP, and the
 * peer dropping the packet. A packet capture proved the peer answered every
 * request; the frame was arriving and being discarded in silence. A drop with
 * no counter is a bug that cannot be located from the outside. */
static int c_short, c_ver, c_ihl, c_frag;

/* the one outstanding echo, matched on id AND sequence - matching on "the next
 * ICMP reply" is the same trap xhci.c's cmd_wait documents for command
 * completions: a reply to a request that already timed out arrives later, the
 * next waiter consumes it, and every measurement after that is one behind. */
static u16 echo_id = 0x7A10, echo_seq;
static int echo_got;
static u32 echo_from;

void net_link(net_send_fn send, net_poll_fn poll, const u8 mac[6])
{
    link_send = send;
    link_poll = poll;
    for (int i = 0; i < 6; i++) my_mac[i] = mac ? mac[i] : 0;
    for (int i = 0; i < ARP_N; i++) arp_tab[i].valid = 0;
    arp_next = 0;
    c_rx = c_arp = c_ip = c_icmp = c_badsum = c_notours = c_tx = 0;
    c_echo_stale = 0;
    c_short = c_ver = c_ihl = c_frag = 0;
    /* echo_seq is NOT reset. It is an identity, not a counter, and restarting
     * it re-uses (id, sequence) pairs from the previous bring-up - which the
     * peer's ICMP layer may still hold. Measured: with the reset, bringing the
     * stack up a second time lost exactly one echo reply, reproducibly, every
     * time. Without it, none. The same rule as xhci.c's "match on the address
     * of the TRB that produced the completion, and nothing else". */
    echo_got = 0;
}

static int arp_send(u32 target, int reply, const u8 *to_mac);

void net_config(u32 ip, u32 mask, u32 gw)
{
    my_ip = ip; my_mask = mask; my_gw = gw;

    /* NO GRATUITOUS ARP HERE, and the absence is deliberate. One was added on
     * the hypothesis that the peer was re-resolving us mid-run and dropping a
     * packet while it did. A packet capture disproved that - the peer answered
     * every one of 42 echo requests - so the announcement was reverted rather
     * than left in place looking like it did something. */
}

u32 net_ip(void) { return my_ip; }

/* Has the stack been given the link? ONE LAYER MAY OWN A RECEIVE QUEUE.
 * virtio_net_arp_probe drains the queue itself and discards anything that is
 * not the ARP reply it wants - which is correct as a driver bring-up test and
 * catastrophic once net.c is also draining it, because the frames it throws
 * away are the ones net.c is waiting for. The caller asks this before running
 * any raw-link probe. */
int net_live(void) { return link_poll != 0; }

/* ---- byte order, spelled out --------------------------------------------- */
static u16 be16(const u8 *p)      { return (u16)(((u16)p[0] << 8) | p[1]); }
static u32 be32(const u8 *p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}
static void put16(u8 *p, u16 v)   { p[0] = (u8)(v >> 8); p[1] = (u8)v; }
static void put32(u8 *p, u32 v)
{
    p[0] = (u8)(v >> 24); p[1] = (u8)(v >> 16);
    p[2] = (u8)(v >> 8);  p[3] = (u8)v;
}

/* ---- the internet checksum ------------------------------------------------
 * One's complement of the one's complement sum of 16-bit words. The end-around
 * carry is what makes it that rather than an ordinary sum, and folding twice
 * is not paranoia: the first fold can itself carry. */
u16 net_checksum(const u8 *p, int len, u32 seed)
{
    u32 sum = seed;
    int i = 0;
    for (; i + 1 < len; i += 2) sum += ((u32)p[i] << 8) | p[i + 1];
    if (i < len) sum += (u32)p[i] << 8;         /* odd tail, high byte */
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (u16)(~sum & 0xFFFF);
}

/* ---- the ARP cache -------------------------------------------------------- */
static int arp_find(u32 ip)
{
    for (int i = 0; i < ARP_N; i++)
        if (arp_tab[i].valid && arp_tab[i].ip == ip) return i;
    return -1;
}

static void arp_put(u32 ip, const u8 *mac)
{
    int i = arp_find(ip);
    if (i < 0) { i = arp_next; arp_next = (arp_next + 1) % ARP_N; }
    arp_tab[i].ip = ip;
    for (int k = 0; k < 6; k++) arp_tab[i].mac[k] = mac[k];
    arp_tab[i].valid = 1;
}

/* ---- frames out -----------------------------------------------------------
 * TWO BUFFERS, and the second one is not waste. net_send_ip assembles a header
 * into txbuf and then copies the caller's payload in after it. If the caller
 * built that payload inside txbuf - at exactly the offset the header ends,
 * which is the tempting thing to do - the copy becomes a self-copy that
 * happens to work, and stops working the moment the header length changes.
 * Callers build payloads HERE. */
static u8 txbuf[FRAME_MAX];
static u8 paybuf[FRAME_MAX];

static int eth_send(const u8 dst[6], u16 type, int payload_len)
{
    if (!link_send) return 0;
    if (payload_len < 0 || payload_len > FRAME_MAX - ETH_HDR) return 0;
    for (int i = 0; i < 6; i++) txbuf[i] = dst[i];
    for (int i = 0; i < 6; i++) txbuf[6 + i] = my_mac[i];
    put16(txbuf + 12, type);
    if (!link_send(txbuf, ETH_HDR + payload_len)) return 0;
    c_tx++;
    return 1;
}

static const u8 bcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static int arp_send(u32 target, int reply, const u8 *to_mac)
{
    u8 *a = txbuf + ETH_HDR;
    put16(a + 0, 1);                     /* hardware type: Ethernet */
    put16(a + 2, 0x0800);                /* protocol type: IPv4     */
    a[4] = 6; a[5] = 4;
    put16(a + 6, reply ? 2 : 1);
    for (int i = 0; i < 6; i++) a[8 + i] = my_mac[i];
    put32(a + 14, my_ip);
    for (int i = 0; i < 6; i++) a[18 + i] = reply ? to_mac[i] : 0;
    put32(a + 24, target);
    return eth_send(reply ? to_mac : bcast, 0x0806, 28);
}

/* Which MAC does a packet for `dst` go to? Off-subnet traffic goes to the
 * gateway - that is the entire routing table, and saying so is better than
 * implying there is one. */
static u32 next_hop(u32 dst)
{
    if (my_mask && ((dst ^ my_ip) & my_mask) == 0) return dst;
    return my_gw ? my_gw : dst;
}

static u16 ip_id = 1;

int net_send_ip(u32 dst, int proto, const u8 *payload, int len)
{
    if (len < 0 || len > FRAME_MAX - ETH_HDR - 20) return 0;
    u32 hop = next_hop(dst);
    int ai = arp_find(hop);
    if (ai < 0) {
        /* ASK, then fail. The caller still has to retry - there is no queue
         * here and a packet held pending an ARP reply is a buffer and a timer
         * this stack does not have - but sending the request means the retry
         * will succeed instead of failing identically forever. Without it, the
         * first packet to any address we have not already talked to is a
         * permanent failure rather than a one-off one. */
        arp_send(hop, 0, 0);
        return 0;
    }

    u8 *h = txbuf + ETH_HDR;
    h[0] = 0x45;                         /* IPv4, 5 words of header */
    h[1] = 0;                            /* DSCP/ECN                */
    put16(h + 2, (u16)(20 + len));
    put16(h + 4, ip_id++);
    put16(h + 6, 0x4000);                /* Don't Fragment, offset 0 */
    h[8] = 64;                           /* TTL                      */
    h[9] = (u8)proto;
    put16(h + 10, 0);                    /* checksum, computed below */
    put32(h + 12, my_ip);
    put32(h + 16, dst);
    for (int i = 0; i < len; i++) h[20 + i] = payload[i];
    put16(h + 10, net_checksum(h, 20, 0));

    return eth_send(arp_tab[ai].mac, 0x0800, 20 + len);
}

/* ---- frames in ------------------------------------------------------------ */
static u8 rxbuf[FRAME_MAX];

/* A POINTER, not a weak symbol. wmglue.c publishes its contract in advance
 * with __attribute__((weak)), which is exactly right for ELF - and this kernel
 * also builds as a PE32+ UEFI application through lld-link, where weak
 * definitions do not behave the same way. A null pointer means the same thing
 * on all four builds. tcp.c sets this in item 3. */
static net_ip_sink_fn ip_sink;

/* A SMALL PROTOCOL TABLE, not a second pointer. TCP and UDP both need to be
 * routed now, and bolting on a `udp_sink` beside the first one would mean a
 * third protocol needs a third global. Four slots is enough for everything
 * this stack will ever carry and it costs a linear scan of four entries. */
#define PROTO_N 4
static struct { int proto; net_ip_sink_fn fn; } proto_sink[PROTO_N];

void net_set_ip_sink(net_ip_sink_fn f) { ip_sink = f; }

void net_set_proto_sink(int proto, net_ip_sink_fn f)
{
    for (int i = 0; i < PROTO_N; i++)
        if (proto_sink[i].fn && proto_sink[i].proto == proto) {
            proto_sink[i].fn = f;
            return;
        }
    for (int i = 0; i < PROTO_N; i++)
        if (!proto_sink[i].fn) {
            proto_sink[i].proto = proto;
            proto_sink[i].fn = f;
            return;
        }
}

static net_ip_sink_fn sink_for(int proto)
{
    for (int i = 0; i < PROTO_N; i++)
        if (proto_sink[i].fn && proto_sink[i].proto == proto)
            return proto_sink[i].fn;
    return ip_sink;
}

static void handle_arp(const u8 *a, int len)
{
    if (len < 28) return;
    if (be16(a + 0) != 1 || be16(a + 2) != 0x0800) return;
    if (a[4] != 6 || a[5] != 4) return;
    c_arp++;

    u16 op  = be16(a + 6);
    u32 sip = be32(a + 14);
    u32 tip = be32(a + 24);

    /* Cache the sender either way. A request for us is also the cheapest
     * possible answer to "where is the machine that just asked". */
    if (sip) arp_put(sip, a + 8);

    if (op == 1 && tip == my_ip && my_ip) arp_send(sip, 1, a + 8);
}

static void handle_icmp(u32 src, const u8 *p, int len)
{
    if (len < 8) return;
    if (net_checksum(p, len, 0) != 0) { c_badsum++; return; }
    c_icmp++;

    if (p[0] == 8) {                          /* echo request -> reply */
        int cap = FRAME_MAX - ETH_HDR - 20;
        int n = len < cap ? len : cap;
        u8 *o = paybuf;
        for (int i = 0; i < n; i++) o[i] = p[i];   /* same id, seq, payload */
        o[0] = 0;                             /* type: echo reply */
        o[1] = 0;
        put16(o + 2, 0);                      /* zero before computing */
        put16(o + 2, net_checksum(o, n, 0));
        net_send_ip(src, 1, o, n);
        return;
    }
    if (p[0] == 0) {                          /* echo reply */
        if (be16(p + 4) == echo_id && be16(p + 6) == echo_seq) {
            echo_got = 1;
            echo_from = src;
        } else {
            c_echo_stale++;
        }
    }
}

/* `from_mac` is the ethernet source of the frame this header arrived in.
 *
 * LEARNING IT IS WHAT MAKES A REPLY POSSIBLE AT ALL. Answering an echo request
 * means sending to the asker, and net_send_ip will not send to an address it
 * has no MAC for - so without this, a ping to us fails unless we happened to
 * have ARPed the sender first, which for an unsolicited request we never have.
 * The MAC is sitting in the frame we are already holding.
 *
 * The cost is honest and worth stating: anyone on the segment can seed our ARP
 * cache by sending us one packet with a forged source address. They could
 * equally answer our ARP request, so this does not open a door that was shut -
 * but it is a passive-learning stack, not a validating one, and the difference
 * matters the day this runs anywhere but a QEMU user-mode network. */
static void handle_ip(const u8 *h, int len, const u8 *from_mac)
{
    if (len < 20)           { c_short++; return; }
    if ((h[0] >> 4) != 4)   { c_ver++;   return; }
    int ihl = (h[0] & 0x0F) * 4;
    if (ihl < 20 || ihl > len) { c_ihl++; return; }

    int total = (int)be16(h + 2);
    if (total < ihl || total > len) total = len;   /* trailing padding is
                                                      normal on short frames */
    if (net_checksum(h, ihl, 0) != 0) { c_badsum++; return; }
    c_ip++;

    /* A fragment is not an error, it is a case this stack does not handle.
     * Dropping it silently would be indistinguishable from losing it. */
    if (be16(h + 6) & 0x3FFF) { c_frag++; return; }

    u32 dst = be32(h + 16);
    if (my_ip && dst != my_ip && dst != 0xFFFFFFFFu) { c_notours++; return; }

    u32 src = be32(h + 12);
    int proto = h[9];
    if (src && from_mac) arp_put(src, from_mac);
    const u8 *payload = h + ihl;
    int plen = total - ihl;

    if (proto == 1) { handle_icmp(src, payload, plen); return; }
    net_ip_sink_fn f = sink_for(proto);
    if (f) f(src, proto, payload, plen);
}

int net_poll_once(void)
{
    if (!link_poll) return 0;
    int n = link_poll(rxbuf, (int)sizeof rxbuf);
    if (n < ETH_HDR) return 0;
    c_rx++;

    u16 type = be16(rxbuf + 12);
    if (type == 0x0806)      handle_arp(rxbuf + ETH_HDR, n - ETH_HDR);
    else if (type == 0x0800) handle_ip(rxbuf + ETH_HDR, n - ETH_HDR, rxbuf + 6);
    return 1;
}

/* ---- bounded waits --------------------------------------------------------
 * Two bounds, whichever trips first: real milliseconds where the PIT is
 * running, a spin count where it is not. A wait that depends on a timer which
 * is not ticking is also a hang - xhci.c's wait_bit makes the same argument.
 */
static int pump_until(int *flag, int ms)
{
    u32 t0 = idt_ticks();
    u32 ticks = (u32)(ms / 10) + 1;
    long spins = (long)ms * 20000;
    while (spins-- > 0) {
        net_poll_once();
        if (*flag) return 1;
        if (idt_ticks() - t0 >= ticks) break;
    }
    return 0;
}

int net_arp_resolve(u32 ip, u8 mac_out[6], int ms)
{
    u32 hop = next_hop(ip);
    int i = arp_find(hop);
    if (i < 0) {
        arp_send(hop, 0, 0);
        u32 t0 = idt_ticks();
        u32 ticks = (u32)(ms / 10) + 1;
        long spins = (long)ms * 20000;
        while (spins-- > 0) {
            net_poll_once();
            i = arp_find(hop);
            if (i >= 0) break;
            if (idt_ticks() - t0 >= ticks) break;
        }
    }
    if (i < 0) return 0;
    if (mac_out) for (int k = 0; k < 6; k++) mac_out[k] = arp_tab[i].mac[k];
    return 1;
}

/* THE DELTA IS CONVERTED, NOT THE TIMESTAMP, and that is not a style choice:
 * dividing a raw 64-bit TSC by the frequency needs __udivmoddi4, which this
 * kernel does not have. divmod.c supplies __divdi3, __moddi3, __udivdi3 and
 * __umoddi3 - but NOT __udivmoddi4, which is what gcc reaches for when it can
 * see that the same operands need both a quotient and a remainder, and it
 * folds `x / k` and `x % k` into one call. So the routine that IS missing is
 * exactly the one this arithmetic asks for. Adding it to a shared file to
 * measure a round trip would be the wrong trade.
 *
 * A delta fits in 32 bits with room to spare: at 3 GHz, 2^32 cycles is 1.4
 * seconds, and anything slower than that is a timeout rather than a
 * measurement. So the subtraction stays 64-bit and every division is 32-bit,
 * which the target does natively. */
static unsigned long long now_tsc(void) { return cpu_tsc(); }

static u32 tsc_to_us(unsigned long long delta)
{
    unsigned int khz = cpu_tsc_khz();
    if (!khz) return 0;
    if (delta > 0xFFFFFFFFull) return 0xFFFFFFFFu;   /* saturate, do not wrap */
    u32 d = (u32)delta;
    return d / khz * 1000u + (d % khz) * 1000u / khz;
}

int net_ping(u32 ip, int ms)
{
    u8 mac[6];
    if (!net_arp_resolve(ip, mac, ms)) return -1;

    u8 *p = paybuf;
    int len = 8 + 16;                     /* header plus a small payload */
    p[0] = 8; p[1] = 0;
    put16(p + 2, 0);
    echo_seq++;
    put16(p + 4, echo_id);
    put16(p + 6, echo_seq);
    for (int i = 8; i < len; i++) p[i] = (u8)('a' + (i & 15));
    put16(p + 2, net_checksum(p, len, 0));

    echo_got = 0;
    unsigned long long t0 = now_tsc();
    if (!net_send_ip(ip, 1, p, len)) return -1;
    if (!pump_until(&echo_got, ms)) return -1;
    if (echo_from != ip) return -1;
    return (int)tsc_to_us(now_tsc() - t0);
}

/* ---- the gate -------------------------------------------------------------
 * §4 item 2: "run it 20 times and report loss and jitter - a stack that works
 * once and drops every fourth packet looks identical on a single ping."
 */
static int st_sent, st_recv, st_min, st_max, st_avg, st_jit;
/* WHICH ones were lost, not just how many. A bit per ping, oldest first. "5 of
 * 20 lost" and "the 1st was lost" are different bugs and a count cannot tell
 * them apart - the first says the link is lossy, the second says something
 * about the state the run starts in. */
static u32 st_mask;

int net_ping_run(u32 ip, int n, int ms)
{
    if (n <= 0) return 0;
    st_sent = st_recv = 0;
    st_min = st_max = st_avg = st_jit = 0;
    st_mask = 0;

    long total = 0, jsum = 0;
    int prev = -1, njit = 0;

    for (int i = 0; i < n; i++) {
        st_sent++;
        int rtt = net_ping(ip, ms);
        if (rtt < 0) {
            if (i < 32) st_mask |= 1u << i;
            prev = -1;                          /* a loss breaks the pairing */
            continue;
        }
        st_recv++;
        total += rtt;
        if (st_recv == 1 || rtt < st_min) st_min = rtt;
        if (rtt > st_max) st_max = rtt;
        if (prev >= 0) {
            int d = rtt - prev;
            if (d < 0) d = -d;
            jsum += d;
            njit++;
        }
        prev = rtt;
    }
    if (st_recv) st_avg = (int)(total / st_recv);
    if (njit)    st_jit = (int)(jsum / njit);
    return st_recv;
}

int net_ping_sent(void)   { return st_sent; }
int net_ping_recv(void)   { return st_recv; }
int net_ping_lost(void)   { return st_sent - st_recv; }
int net_ping_min(void)    { return st_min; }
int net_ping_max(void)    { return st_max; }
int net_ping_avg(void)    { return st_avg; }
int net_ping_jitter(void) { return st_jit; }
int net_ping_mask(void)   { return (int)st_mask; }

int net_rx_frames(void)   { return c_rx; }
int net_rx_arp(void)      { return c_arp; }
int net_rx_ip(void)       { return c_ip; }
int net_rx_icmp(void)     { return c_icmp; }
int net_rx_bad_csum(void) { return c_badsum; }
int net_rx_not_ours(void) { return c_notours; }
int net_tx_frames(void)   { return c_tx; }
int net_rx_stale_echo(void) { return c_echo_stale; }
int net_rx_short(void)      { return c_short; }
int net_rx_badver(void)     { return c_ver; }
int net_rx_badihl(void)     { return c_ihl; }
int net_rx_frag(void)       { return c_frag; }
