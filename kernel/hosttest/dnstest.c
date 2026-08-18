/* dnstest.c - the resolver, mostly fed answers it should refuse.
 *
 * A DNS response is unauthenticated data from a machine we did not choose,
 * parsed by a walk over length-prefixed labels with BACKWARD POINTERS in them.
 * Nothing in the format prevents a pointer aimed at itself. That single fact
 * has produced a long line of remote hangs and out-of-bounds reads in real
 * resolvers, so most of what is below is malicious rather than malformed:
 *
 *   - a compression pointer that points at itself
 *   - two pointers that point at each other
 *   - a pointer past the end of the packet
 *   - an answer to a question we did not ask
 *   - an answer from the wrong host, and from the right host on the wrong port
 *   - an answer with someone else's transaction id
 *   - an answer claiming 127.0.0.1, which would turn an outbound fetch into a
 *     request against ourselves
 *   - a record whose RDLENGTH runs off the end of the packet
 *
 * Every one of those is two lines here and none of them can be requested from
 * a real DNS server.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../dns.h"
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

static unsigned v_ticks;
unsigned int idt_ticks(void) { return v_ticks; }
unsigned long long cpu_tsc(void) { return 0; }
unsigned int cpu_tsc_khz(void)   { return 0; }

#define OUR_IP  0x0A00020Fu
#define GW_IP   0x0A000202u
#define DNS_IP  0x0A000203u

static const unsigned char OUR_MAC[6] = { 0x52,0x54,0x00,0x12,0x34,0x56 };
static const unsigned char GW_MAC[6]  = { 0x52,0x55,0x0A,0x00,0x02,0x02 };

/* ---- the fake link ------------------------------------------------------------ */
static unsigned char sent[16][1600];
static int sent_len[16], nsent;
static int fake_send(const net_u8 *f, int len)
{
    if (nsent < 16 && len < 1600) { memcpy(sent[nsent], f, len); sent_len[nsent] = len; nsent++; }
    return 1;
}
static unsigned char inq[1600];
static int inq_len;
static int fake_poll(net_u8 *out, int max)
{
    if (!inq_len) return 0;
    int n = inq_len; if (n > max) n = max;
    memcpy(out, inq, n);
    inq_len = 0;
    return n;
}

/* the query we last sent: its transaction id and the port it came from */
static int last_query(unsigned *id, int *sport)
{
    for (int i = nsent - 1; i >= 0; i--) {
        if (sent_len[i] < 14 + 20 + 8 + 12) continue;
        unsigned char *h = sent[i] + 14;
        if (h[9] != 17) continue;                    /* not UDP */
        unsigned char *u = h + (h[0] & 0x0F) * 4;
        *sport = (u[0] << 8) | u[1];
        *id = (u[8] << 8) | u[9];
        return 1;
    }
    return 0;
}

/* Deliver a DNS payload back to the stack as a real IPv4/UDP frame, so the
 * whole receive path is exercised rather than dns_input alone. */
static void deliver(const unsigned char *dns, int dlen, unsigned src,
                    int sport, int dport, int corrupt_len)
{
    unsigned char f[1600];
    memset(f, 0, sizeof f);
    memcpy(f, OUR_MAC, 6);
    memcpy(f + 6, GW_MAC, 6);
    f[12] = 0x08; f[13] = 0x00;
    unsigned char *h = f + 14;
    int ulen = 8 + dlen;
    h[0] = 0x45;
    h[2] = (unsigned char)((20 + ulen) >> 8); h[3] = (unsigned char)(20 + ulen);
    h[6] = 0x40;
    h[8] = 64; h[9] = 17;
    h[12] = (unsigned char)(src >> 24); h[13] = (unsigned char)(src >> 16);
    h[14] = (unsigned char)(src >> 8);  h[15] = (unsigned char)src;
    h[16] = (unsigned char)(OUR_IP >> 24); h[17] = (unsigned char)(OUR_IP >> 16);
    h[18] = (unsigned char)(OUR_IP >> 8);  h[19] = (unsigned char)OUR_IP;
    unsigned short c = net_checksum(h, 20, 0);
    h[10] = (unsigned char)(c >> 8); h[11] = (unsigned char)c;

    unsigned char *u = h + 20;
    u[0] = (unsigned char)(sport >> 8); u[1] = (unsigned char)sport;
    u[2] = (unsigned char)(dport >> 8); u[3] = (unsigned char)dport;
    int declared = corrupt_len ? corrupt_len : ulen;
    u[4] = (unsigned char)(declared >> 8); u[5] = (unsigned char)declared;
    memcpy(u + 8, dns, dlen);

    memcpy(inq, f, 14 + 20 + ulen);
    inq_len = 14 + 20 + ulen;
    net_poll_once();
}

/* ---- building answers ---------------------------------------------------------- */
static int put_name(unsigned char *p, const char *n)
{
    int o = 0, i = 0, len = (int)strlen(n);
    while (i < len) {
        int j = i;
        while (j < len && n[j] != '.') j++;
        p[o++] = (unsigned char)(j - i);
        for (int k = i; k < j; k++) p[o++] = (unsigned char)n[k];
        i = j + 1;
    }
    p[o++] = 0;
    return o;
}

/* A well-formed answer: one question echoed, one A record. */
static int build_answer(unsigned char *d, unsigned id, const char *name,
                        unsigned ip, unsigned ttl, int ancount, int type)
{
    int o = 0;
    d[0] = (unsigned char)(id >> 8); d[1] = (unsigned char)id;
    d[2] = 0x81; d[3] = 0x80;                      /* response, no error */
    d[4] = 0; d[5] = 1;                            /* one question */
    d[6] = 0; d[7] = (unsigned char)ancount;
    d[8] = 0; d[9] = 0; d[10] = 0; d[11] = 0;
    o = 12;
    o += put_name(d + o, name);
    d[o++] = 0; d[o++] = 1;                        /* QTYPE A  */
    d[o++] = 0; d[o++] = 1;                        /* QCLASS IN */
    for (int a = 0; a < ancount; a++) {
        d[o++] = 0xC0; d[o++] = 12;                /* a pointer back to the name */
        d[o++] = 0; d[o++] = (unsigned char)type;
        d[o++] = 0; d[o++] = 1;
        d[o++] = (unsigned char)(ttl >> 24); d[o++] = (unsigned char)(ttl >> 16);
        d[o++] = (unsigned char)(ttl >> 8);  d[o++] = (unsigned char)ttl;
        d[o++] = 0; d[o++] = 4;
        d[o++] = (unsigned char)(ip >> 24); d[o++] = (unsigned char)(ip >> 16);
        d[o++] = (unsigned char)(ip >> 8);  d[o++] = (unsigned char)ip;
    }
    return o;
}

static void bring_up(void)
{
    nsent = 0; inq_len = 0;
    v_ticks = 1000;
    net_link(fake_send, fake_poll, OUR_MAC);
    net_config(OUR_IP, 0xFFFFFF00u, GW_IP);
    net_set_proto_sink(17, dns_ip_sink);
    dns_server(DNS_IP);
    dns_reset();          /* a lookup left in flight refuses the next one */
    dns_cache_clear();
    /* teach net.c where the gateway is, so a query can actually be sent */
    unsigned char a[42];
    memset(a, 0, sizeof a);
    memcpy(a, OUR_MAC, 6); memcpy(a + 6, GW_MAC, 6);
    a[12] = 0x08; a[13] = 0x06;
    a[15] = 1; a[16] = 0x08; a[18] = 6; a[19] = 4; a[21] = 2;
    memcpy(a + 22, GW_MAC, 6);
    a[28] = 10; a[29] = 0; a[30] = 2; a[31] = 3;      /* the DNS server */
    memcpy(a + 32, OUR_MAC, 6);
    a[38] = 10; a[39] = 0; a[40] = 2; a[41] = 15;
    memcpy(inq, a, 42); inq_len = 42;
    net_poll_once();
    nsent = 0;
}

/* ---- the tests ------------------------------------------------------------------ */
static void t_query(void)
{
    printf("the query\n");
    bring_up();
    CHECK(dns_start("example.com", 11), "dns_start refused a good name");
    CHECK(nsent == 1, "%d frames sent for one lookup", nsent);
    if (nsent != 1) return;

    unsigned char *h = sent[0] + 14;
    CHECK(h[9] == 17, "the query is protocol %d, not UDP", h[9]);
    unsigned dst = ((unsigned)h[16]<<24)|((unsigned)h[17]<<16)|((unsigned)h[18]<<8)|h[19];
    CHECK(dst == DNS_IP, "the query went to %08X, not the server", dst);
    unsigned char *u = h + 20;
    CHECK(((u[2] << 8) | u[3]) == 53, "the query went to port %d", (u[2]<<8)|u[3]);
    CHECK(u[6] || u[7], "the UDP checksum was left at zero");

    unsigned char *d = u + 8;
    CHECK((d[2] & 0x01), "recursion desired is not set");
    CHECK(((d[4] << 8) | d[5]) == 1, "question count is %d", (d[4]<<8)|d[5]);
    /* the name, on the wire: 7 example 3 com 0 */
    CHECK(d[12] == 7 && !memcmp(d + 13, "example", 7), "the name is not encoded");
    CHECK(d[20] == 3 && !memcmp(d + 21, "com", 3), "the second label is wrong");
    CHECK(d[24] == 0, "the name is not terminated");
    CHECK(((d[25] << 8) | d[26]) == 1, "QTYPE is not A");

    /* two lookups must not reuse the same transaction id */
    unsigned id1 = 0, id2 = 0; int p1 = 0, p2 = 0;
    last_query(&id1, &p1);
    bring_up();
    dns_start("other.example", 13);
    last_query(&id2, &p2);
    CHECK(id1 != id2, "two lookups used the same transaction id %04X", id1);
}

static void t_answer(void)
{
    printf("a good answer\n");
    bring_up();
    dns_start("example.com", 11);
    unsigned id = 0; int sport = 0;
    CHECK(last_query(&id, &sport), "no query went out");

    unsigned char d[512];
    int n = build_answer(d, id, "example.com", 0x5DB8D822u, 300, 1, 1);
    deliver(d, n, DNS_IP, 53, sport, 0);

    CHECK(dns_state() == DNS_DONE, "state %d after a good answer", dns_state());
    CHECK(dns_result() == 0x5DB8D822u, "resolved to %08X", dns_result());
    CHECK(dns_cache_count() == 1, "the answer was not cached");

    /* ...and the second lookup must be answered from the cache with no traffic */
    nsent = 0;
    CHECK(dns_start("example.com", 11), "a cached lookup was refused");
    CHECK(dns_state() == DNS_DONE, "a cached lookup did not complete");
    CHECK(nsent == 0, "%d frames sent for a cached name", nsent);
    CHECK(dns_cache_hits() >= 1, "the cache hit was not counted");

    /* the name is case-insensitive */
    CHECK(dns_start("EXAMPLE.COM", 11) && dns_state() == DNS_DONE,
          "a cached name did not match in a different case");

    /* a TTL that has passed must NOT be served from the cache */
    v_ticks += 400 * 100;
    nsent = 0;
    dns_start("example.com", 11);
    CHECK(nsent == 1, "an expired entry was served from the cache");
}

static void t_hostile(void)
{
    printf("answers that must be refused\n");
    unsigned char d[512];
    unsigned id = 0; int sport = 0;
    int n;

    /* the wrong transaction id */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id ^ 0x5A5A, "example.com", 0x01020304u, 60, 1, 1);
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() == DNS_ASKING, "an answer with a forged id was accepted");

    /* the right id from the wrong host */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    deliver(d, n, 0x0A0002FEu, 53, sport, 0);
    CHECK(dns_state() == DNS_ASKING, "an answer from the wrong host was accepted");

    /* the right host, the wrong source port */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    deliver(d, n, DNS_IP, 9999, sport, 0);
    CHECK(dns_state() == DNS_ASKING, "an answer from the wrong port was accepted");

    /* the right everything, to the wrong destination port */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    deliver(d, n, DNS_IP, 53, sport ^ 0x0F, 0);
    CHECK(dns_state() == DNS_ASKING, "an answer to the wrong port was accepted");

    /* AN ANSWER TO A QUESTION WE DID NOT ASK. Without this check a server can
     * answer any query with a record for any name and have it cached. */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "evil.example", 0x01020304u, 60, 1, 1);
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() != DNS_DONE, "an answer for a different name was accepted");
    CHECK(dns_cache_count() == 0, "a name we never asked for was cached");

    /* 127.0.0.1 - which would turn an outbound fetch into a local one */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x7F000001u, 60, 1, 1);
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() != DNS_DONE, "127.0.0.1 was accepted as an answer");
    CHECK(dns_result() != 0x7F000001u, "a loopback address was handed back");

    /* 0.0.0.0 */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0, 60, 1, 1);
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() != DNS_DONE, "0.0.0.0 was accepted as an answer");

    /* NXDOMAIN is a real answer, not a malformed one */
    bring_up(); dns_start("nope.example", 12); last_query(&id, &sport);
    n = build_answer(d, id, "nope.example", 0, 60, 0, 1);
    d[3] = 0x83;                                   /* RCODE 3 */
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() == DNS_NXDOMAIN, "NXDOMAIN gave state %d", dns_state());
    CHECK(dns_cache_count() == 0, "NXDOMAIN was cached as an address");
}

static void t_pointers(void)
{
    printf("compression pointers\n");
    unsigned char d[512];
    unsigned id = 0; int sport = 0;
    int n;

    /* A POINTER THAT POINTS AT ITSELF. The classic hang. */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    d[12] = 0xC0; d[13] = 12;                      /* the question name -> itself */
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() != DNS_DONE, "a self-referential pointer was accepted");
    CHECK(1, "a self-referential pointer did not hang");

    /* TWO POINTERS THAT POINT AT EACH OTHER. */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    d[12] = 0xC0; d[13] = 14;
    d[14] = 0xC0; d[15] = 12;
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(1, "a pointer cycle did not hang");

    /* A POINTER PAST THE END OF THE PACKET. */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    d[12] = 0xC0; d[13] = 0xFF;
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() != DNS_DONE, "a pointer past the packet was accepted");

    /* A CHAIN of pointers, longer than the budget. */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    for (int i = 0; i < 100 && 12 + i * 2 + 1 < 400; i++) {
        d[12 + i * 2] = 0xC0;
        d[13 + i * 2] = (unsigned char)(12 + (i + 1) * 2);
    }
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(1, "a long pointer chain did not hang");

    /* A RECORD WHOSE RDLENGTH RUNS OFF THE END, two ways.
     *
     * The obvious one - a length of 65535 - is NOT the interesting case: it is
     * not 4, so the A-record branch skips it whether or not the bound exists,
     * and a test built only on it stays green with the bound deleted. The one
     * that bites claims a PLAUSIBLE four bytes and then ends the packet before
     * them, so a parser without the bound reads past the buffer. That is the
     * difference between a test that describes an intention and one that
     * checks it. */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    d[n - 6] = 0xFF; d[n - 5] = 0xFF;              /* RDLENGTH = 65535 */
    deliver(d, n, DNS_IP, 53, sport, 0);
    CHECK(dns_state() != DNS_DONE, "an over-long RDLENGTH was accepted");

    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    deliver(d, n - 3, DNS_IP, 53, sport, 0);       /* rdata cut short */
    CHECK(dns_state() != DNS_DONE,
          "a record claiming four bytes of address that are not there was accepted");

    /* a UDP length field that lies */
    bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
    n = build_answer(d, id, "example.com", 0x01020304u, 60, 1, 1);
    deliver(d, n, DNS_IP, 53, sport, 9999);
    CHECK(dns_state() != DNS_DONE, "a lying UDP length was accepted");

    /* every truncation of a good answer */
    n = build_answer(d, 0x1234, "example.com", 0x01020304u, 60, 1, 1);
    for (int cut = 0; cut < n; cut++) {
        bring_up(); dns_start("example.com", 11); last_query(&id, &sport);
        unsigned char t[512];
        memcpy(t, d, cut);
        if (cut >= 2) { t[0] = (unsigned char)(id >> 8); t[1] = (unsigned char)id; }
        deliver(t, cut, DNS_IP, 53, sport, 0);
        CHECK(dns_state() != DNS_DONE || dns_result() != 0,
              "a %d-byte truncation resolved to nothing", cut);
    }
    CHECK(1, "no truncation faulted");

    /* a CNAME before the A record - the normal case for a real site */
    bring_up(); dns_start("www.example.com", 15); last_query(&id, &sport);
    {
        int o = 0;
        d[0] = (unsigned char)(id >> 8); d[1] = (unsigned char)id;
        d[2] = 0x81; d[3] = 0x80;
        d[4] = 0; d[5] = 1; d[6] = 0; d[7] = 2;
        d[8]=0;d[9]=0;d[10]=0;d[11]=0;
        o = 12;
        o += put_name(d + o, "www.example.com");
        d[o++]=0; d[o++]=1; d[o++]=0; d[o++]=1;
        /* answer 1: CNAME */
        d[o++]=0xC0; d[o++]=12;
        d[o++]=0; d[o++]=5;                        /* TYPE CNAME */
        d[o++]=0; d[o++]=1;
        d[o++]=0;d[o++]=0;d[o++]=0;d[o++]=60;
        int rd = o; d[o++]=0; d[o++]=0;
        int st = o;
        o += put_name(d + o, "target.example.com");
        d[rd] = (unsigned char)((o - st) >> 8); d[rd+1] = (unsigned char)(o - st);
        /* answer 2: the A record */
        d[o++]=0xC0; d[o++]=12;
        d[o++]=0; d[o++]=1;
        d[o++]=0; d[o++]=1;
        d[o++]=0;d[o++]=0;d[o++]=0;d[o++]=60;
        d[o++]=0; d[o++]=4;
        d[o++]=93; d[o++]=184; d[o++]=216; d[o++]=34;
        deliver(d, o, DNS_IP, 53, sport, 0);
    }
    CHECK(dns_state() == DNS_DONE, "a CNAME before the A record broke it (state %d)",
          dns_state());
    CHECK(dns_result() == 0x5DB8D822u, "resolved to %08X through a CNAME",
          dns_result());
}

static void t_timeout(void)
{
    printf("no answer at all\n");
    bring_up();
    dns_start("example.com", 11);
    CHECK(dns_poll() == DNS_ASKING, "state %d immediately after asking", dns_poll());
    v_ticks += 100000;
    CHECK(dns_poll() == DNS_TIMEOUT, "state %d after the deadline", dns_poll());

    /* names that cannot be asked about */
    bring_up();
    CHECK(!dns_start("", 0), "an empty name was accepted");
    CHECK(!dns_start("x", -1), "a negative length was accepted");
    static char big[600];
    memset(big, 'a', sizeof big);
    CHECK(!dns_start(big, (int)sizeof big), "a 600-character name was accepted");
    CHECK(!dns_start("a..b", 4), "an empty label was accepted");
    static char longlab[100];
    memset(longlab, 'a', sizeof longlab);
    CHECK(!dns_start(longlab, 90), "a 90-character label was accepted");
}

int main(void)
{
    printf("dns.c, mostly fed answers it should refuse\n\n");
    t_query();
    t_answer();
    t_hostile();
    t_pointers();
    t_timeout();
    printf("\n%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
