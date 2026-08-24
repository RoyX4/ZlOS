/* dhcp.c - a bounded, non-blocking DHCPv4 state machine.
 *
 * dhcp_start() sends DISCOVER and returns.  dhcp_poll() consumes at most one
 * frame through net_poll_once(), advances OFFER -> REQUEST -> ACK, and retries
 * on a one-second deadline.  There is no spin loop and no frame/UI callback
 * waits for a server.  The UDP checksum is zero, which IPv4 explicitly permits;
 * IPv4/header checksums remain net.c's responsibility. */

#include "dhcp.h"
#include "net.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

unsigned int idt_ticks(void);
unsigned long long cpu_tsc(void);

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67
#define DHCP_FIXED       236
#define DHCP_COOKIE      0x63825363u
#define DHCP_PACKET_MAX  576
#define DHCP_RETRY_TICKS 100u
#define DHCP_RETRY_MAX   4

static int state, retry_n, bad_n;
static u32 xid, deadline;
static u32 offered, mask, gateway, dns, server, lease;
static u8 packet[DHCP_PACKET_MAX];

static u16 be16(const u8 *p) { return (u16)(((u16)p[0] << 8) | p[1]); }
static u32 be32(const u8 *p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}
static void put16(u8 *p, u16 v) { p[0] = (u8)(v >> 8); p[1] = (u8)v; }
static void put32(u8 *p, u32 v)
{
    p[0] = (u8)(v >> 24); p[1] = (u8)(v >> 16);
    p[2] = (u8)(v >> 8); p[3] = (u8)v;
}
static void zero(u8 *p, int n) { while (n-- > 0) *p++ = 0; }

static int option(u8 *o, int *at, int cap, int kind,
                  const u8 *data, int n)
{
    if (n < 0 || *at + 2 + n > cap) return 0;
    o[(*at)++] = (u8)kind; o[(*at)++] = (u8)n;
    for (int i = 0; i < n; i++) o[(*at)++] = data[i];
    return 1;
}

static int send_message(int type)
{
    /* UDP header followed by BOOTP/DHCP. */
    u8 *u = packet;
    u8 *b = packet + 8;
    zero(packet, DHCP_PACKET_MAX);
    b[0] = 1; b[1] = 1; b[2] = 6;                 /* BOOTREQUEST/Ethernet */
    put32(b + 4, xid);
    put16(b + 10, 0x8000);                        /* broadcast reply */
    for (int i = 0; i < 6; i++) b[28 + i] = (u8)net_mac_byte(i);
    put32(b + DHCP_FIXED, DHCP_COOKIE);

    int at = DHCP_FIXED + 4;
    u8 mt = (u8)type;
    if (!option(b, &at, DHCP_PACKET_MAX - 8, 53, &mt, 1)) return 0;
    u8 cid[7]; cid[0] = 1;
    for (int i = 0; i < 6; i++) cid[i + 1] = (u8)net_mac_byte(i);
    if (!option(b, &at, DHCP_PACKET_MAX - 8, 61, cid, 7)) return 0;
    if (type == 3) {
        u8 v[4]; put32(v, offered);
        if (!option(b, &at, DHCP_PACKET_MAX - 8, 50, v, 4)) return 0;
        put32(v, server);
        if (!option(b, &at, DHCP_PACKET_MAX - 8, 54, v, 4)) return 0;
    }
    u8 wanted[4] = { 1, 3, 6, 51 };
    if (!option(b, &at, DHCP_PACKET_MAX - 8, 55, wanted, 4)) return 0;
    if (at >= DHCP_PACKET_MAX - 8) return 0;
    b[at++] = 255;
    /* RFC 2131 clients send at least 300 octets of BOOTP/DHCP payload. */
    if (at < 300) at = 300;
    put16(u + 0, DHCP_CLIENT_PORT); put16(u + 2, DHCP_SERVER_PORT);
    put16(u + 4, (u16)(8 + at)); put16(u + 6, 0);
    if (!net_send_ip_broadcast(0, 0xffffffffu, 17, packet, 8 + at)) return 0;
    deadline = idt_ticks() + DHCP_RETRY_TICKS;
    return 1;
}

struct opts {
    int type;
    u32 mask, gateway, dns, server, lease;
};

static int parse_options(const u8 *p, int len, struct opts *out)
{
    if (len < DHCP_FIXED + 4 || be32(p + DHCP_FIXED) != DHCP_COOKIE) return 0;
    out->type = 0; out->mask = out->gateway = out->dns = 0;
    out->server = out->lease = 0;
    int at = DHCP_FIXED + 4;
    while (at < len) {
        int kind = p[at++];
        if (kind == 255) break;
        if (kind == 0) continue;
        if (at >= len) return 0;
        int n = p[at++];
        if (n < 0 || at + n > len) return 0;
        if (kind == 53 && n == 1) out->type = p[at];
        else if (kind == 1 && n == 4) out->mask = be32(p + at);
        else if (kind == 3 && n >= 4) out->gateway = be32(p + at);
        else if (kind == 6 && n >= 4) out->dns = be32(p + at);
        else if (kind == 54 && n == 4) out->server = be32(p + at);
        else if (kind == 51 && n == 4) out->lease = be32(p + at);
        at += n;
    }
    return out->type != 0;
}

static void receive(u32 src, int proto, const u8 *u, int len)
{
    (void)proto;
    if (len < 8 + DHCP_FIXED + 4 || be16(u) != DHCP_SERVER_PORT ||
        be16(u + 2) != DHCP_CLIENT_PORT) { bad_n++; return; }
    int ulen = be16(u + 4);
    if (ulen < 8 + DHCP_FIXED + 4 || ulen > len) { bad_n++; return; }
    const u8 *b = u + 8;
    if (b[0] != 2 || b[1] != 1 || b[2] != 6 || be32(b + 4) != xid) {
        bad_n++; return;
    }
    for (int i = 0; i < 6; i++)
        if (b[28 + i] != (u8)net_mac_byte(i)) { bad_n++; return; }
    struct opts o;
    if (!parse_options(b, ulen - 8, &o)) { bad_n++; return; }

    if (state == DHCP_SELECTING && o.type == 2) {       /* OFFER */
        offered = be32(b + 16);
        server = o.server ? o.server : src;
        if (!offered || !server) { bad_n++; return; }
        mask = o.mask; gateway = o.gateway; dns = o.dns; lease = o.lease;
        state = DHCP_REQUESTING; retry_n = 0;
        if (!send_message(3)) state = DHCP_FAILED;
        return;
    }
    if (state == DHCP_REQUESTING && o.type == 5) {      /* ACK */
        u32 address = be32(b + 16);
        if (!address || (o.server && server && o.server != server)) {
            bad_n++; return;
        }
        offered = address;
        if (o.mask) mask = o.mask;
        if (o.gateway) gateway = o.gateway;
        if (o.dns) dns = o.dns;
        if (o.server) server = o.server;
        if (o.lease) lease = o.lease;
        if (!mask) mask = 0xffffff00u;
        net_config(offered, mask, gateway);
        state = DHCP_BOUND;
        return;
    }
    if (o.type == 6) state = DHCP_FAILED;               /* NAK */
}

int dhcp_start(void)
{
    xid = (u32)cpu_tsc() ^ 0x5a4c4448u;
    if (!xid) xid = 0x5a4c0001u;
    state = DHCP_SELECTING; retry_n = bad_n = 0;
    offered = mask = gateway = dns = server = lease = 0;
    net_config(0, 0, 0);
    net_set_proto_sink(17, receive);
    if (!send_message(1)) { state = DHCP_FAILED; return 0; }
    return 1;
}

int dhcp_poll(void)
{
    int before = state;
    if (state == DHCP_SELECTING || state == DHCP_REQUESTING) {
        (void)net_poll_once();
        u32 now = idt_ticks();
        if ((int)(now - deadline) >= 0 &&
            (state == DHCP_SELECTING || state == DHCP_REQUESTING)) {
            if (++retry_n > DHCP_RETRY_MAX) state = DHCP_FAILED;
            else if (!send_message(state == DHCP_SELECTING ? 1 : 3))
                state = DHCP_FAILED;
        }
    }
    return state != before;
}

int dhcp_state(void) { return state; }
u32 dhcp_address(void) { return offered; }
u32 dhcp_mask(void) { return mask; }
u32 dhcp_gateway(void) { return gateway; }
u32 dhcp_dns(void) { return dns; }
u32 dhcp_server(void) { return server; }
u32 dhcp_lease_seconds(void) { return lease; }
int dhcp_retries(void) { return retry_n; }
int dhcp_bad_packets(void) { return bad_n; }
