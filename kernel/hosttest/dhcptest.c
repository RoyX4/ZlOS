/* DHCP client against a scripted server. No machine, sleeps, or real time. */
#include <stdio.h>
#include <string.h>
#include "../net.h"
#include "../dhcp.h"

static int checks, fails;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("  FAIL %d: ", __LINE__); printf(__VA_ARGS__); putchar('\n'); } } while (0)

static unsigned ticks_v;
unsigned int idt_ticks(void) { return ticks_v; }
unsigned long long cpu_tsc(void) { return 0x123456789ULL; }
unsigned int cpu_tsc_khz(void) { return 1000000; }

#define QN 8
static unsigned char q[QN][1600];
static int qlen[QN], qh, qt, respond, tx_n, discover_n, request_n;
static const unsigned char client_mac[6] = { 2, 0, 0, 0, 0, 1 };
static const unsigned char server_mac[6] = { 2, 0, 0, 0, 0, 2 };

static void p16(unsigned char *p, unsigned v) { p[0] = v >> 8; p[1] = v; }
static void p32(unsigned char *p, unsigned v)
{ p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }
static unsigned g32(const unsigned char *p)
{ return (unsigned)p[0] << 24 | (unsigned)p[1] << 16 | (unsigned)p[2] << 8 | p[3]; }

static void inject(const unsigned char *p, int n)
{
    if ((qt + 1) % QN == qh) return;
    memcpy(q[qt], p, n); qlen[qt] = n; qt = (qt + 1) % QN;
}

static int message_type(const unsigned char *f, int n)
{
    int at = 14 + 20 + 8 + 240;
    while (at < n) {
        int k = f[at++];
        if (k == 255) break;
        if (!k) continue;
        if (at >= n) break;
        int z = f[at++];
        if (at + z > n) break;
        if (k == 53 && z == 1) return f[at];
        at += z;
    }
    return 0;
}

static void answer(const unsigned char *req, int type)
{
    unsigned char f[600]; memset(f, 0, sizeof f);
    memcpy(f, client_mac, 6); memcpy(f + 6, server_mac, 6);
    f[12] = 8; f[13] = 0;
    unsigned char *ip = f + 14;
    unsigned char *udp = ip + 20;
    unsigned char *b = udp + 8;
    b[0] = 2; b[1] = 1; b[2] = 6;
    memcpy(b + 4, req + 14 + 20 + 8 + 4, 4);      /* xid */
    p32(b + 16, 0x0a00020f);                       /* yiaddr */
    memcpy(b + 28, client_mac, 6);
    p32(b + 236, 0x63825363);
    int at = 240;
#define OPT1(k,v) do { b[at++]=(k); b[at++]=1; b[at++]=(v); } while (0)
#define OPT4(k,v) do { b[at++]=(k); b[at++]=4; p32(b+at,(v)); at+=4; } while (0)
    OPT1(53, type); OPT4(54, 0x0a000202);
    OPT4(1, 0xffffff00); OPT4(3, 0x0a000202);
    OPT4(6, 0x0a000203); OPT4(51, 86400); b[at++] = 255;
#undef OPT1
#undef OPT4
    p16(udp, 67); p16(udp + 2, 68); p16(udp + 4, 8 + at);
    ip[0] = 0x45; p16(ip + 2, 20 + 8 + at); ip[8] = 64; ip[9] = 17;
    p32(ip + 12, 0x0a000202); p32(ip + 16, 0xffffffff);
    p16(ip + 10, net_checksum(ip, 20, 0));
    inject(f, 14 + 20 + 8 + at);
}

static int send_fake(const unsigned char *f, int n)
{
    tx_n++;
    CHECK(n >= 14 + 20 + 8 + 240, "short DHCP transmit: %d", n);
    CHECK(!memcmp(f, "\xff\xff\xff\xff\xff\xff", 6), "not L2 broadcast");
    CHECK(g32(f + 14 + 12) == 0, "source IP is not zero");
    int t = message_type(f, n);
    if (t == 1) { discover_n++; if (respond) answer(f, 2); }
    if (t == 3) { request_n++; if (respond) answer(f, 5); }
    return 1;
}

static int poll_fake(unsigned char *out, int max)
{
    if (qh == qt) return 0;
    int n = qlen[qh]; if (n > max) n = max;
    memcpy(out, q[qh], n); qh = (qh + 1) % QN; return n;
}

static void reset(int server_on)
{
    qh = qt = tx_n = discover_n = request_n = 0; ticks_v = 0; respond = server_on;
    net_link(send_fake, poll_fake, client_mac);
}

int main(void)
{
    puts("dhcp.c against a scripted server, no machine\n");
    reset(1);
    CHECK(dhcp_start(), "start failed");
    CHECK(dhcp_state() == DHCP_SELECTING, "state after discover = %d", dhcp_state());
    dhcp_poll();
    CHECK(dhcp_state() == DHCP_REQUESTING, "OFFER did not cause REQUEST");
    dhcp_poll();
    CHECK(dhcp_state() == DHCP_BOUND, "ACK did not bind: %d", dhcp_state());
    CHECK(discover_n == 1 && request_n == 1, "messages discover=%d request=%d", discover_n, request_n);
    CHECK(dhcp_address() == 0x0a00020f, "address %08x", dhcp_address());
    CHECK(dhcp_mask() == 0xffffff00, "mask %08x", dhcp_mask());
    CHECK(dhcp_gateway() == 0x0a000202, "gateway %08x", dhcp_gateway());
    CHECK(dhcp_dns() == 0x0a000203, "DNS %08x", dhcp_dns());
    CHECK(dhcp_lease_seconds() == 86400, "lease %u", dhcp_lease_seconds());

    reset(0);
    CHECK(dhcp_start(), "timeout start failed");
    for (int i = 0; i < 5; i++) { ticks_v += 101; dhcp_poll(); }
    CHECK(dhcp_state() == DHCP_FAILED, "retry bound did not fail: %d", dhcp_state());
    CHECK(dhcp_retries() == 5, "retry count %d", dhcp_retries());
    CHECK(tx_n == 5, "wanted initial + 4 retries, got %d", tx_n);

    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
