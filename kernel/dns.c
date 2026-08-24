/* dns.c - a resolver, and UDP under it.
 *
 * THIS IS THE RUNG THAT REACHES THE REAL INTERNET. Everything below is
 * addressed by number; a person types a name. Without this the browser can
 * only reach machines someone already looked up by hand.
 *
 * A DNS RESPONSE IS ATTACKER-CONTROLLED DATA. It arrives unauthenticated from
 * a server we did not choose over a protocol with no handshake, and it is
 * parsed by a state machine walking length-prefixed data with BACKWARD
 * POINTERS in it. That combination has produced more remote holes than almost
 * anything else in networking, so the parsing here is written defensively on
 * purpose:
 *
 *   - COMPRESSION POINTERS ARE BOUNDED. A label may be replaced by a pointer
 *     to an earlier offset, and nothing in the format stops that pointer
 *     pointing at itself or at a cycle. The classic bug is a parser that
 *     follows them until it hangs. Every walk here has a hard step budget and
 *     every offset is checked against the length of the packet it came in.
 *   - THE TRANSACTION ID, THE SOURCE, THE PORT AND THE QUESTION are all
 *     checked. Matching on "the next UDP packet" is the same trap tcp.c
 *     documents for segments: anyone who can reach us could answer.
 *   - NOTHING IS COPIED WITHOUT A BOUND, and the name in the cache is a fixed
 *     array, so a 4 KB name truncates rather than writing past it.
 *
 * WHAT IT IS NOT: there is no DNSSEC and no 0x20 encoding, so an off-path
 * attacker who can guess the transaction id can poison this. The id is drawn
 * from the tick counter and a sequence, which is better than a constant and is
 * NOT a random number - this kernel has no entropy source. Said out loud
 * rather than implied, on the same standard as the padlock.
 */

#include "dns.h"
#include "telemetry.h"

typedef net_u8  u8;
typedef net_u16 u16;
typedef net_u32 u32;

unsigned int idt_ticks(void);

#define CACHE_N   8
#define PKT_MAX   512          /* an A record answer is never larger */
#define MAX_STEPS 128          /* the budget for any walk over a name */

static u32  server_ip;
static int  state = DNS_IDLE;
static u32  answer;
static u16  txid;
static u16  local_port = 50000;
static char asking[DNS_MAX_NAME];
static int  asking_len;
static u32  deadline;

static struct {
    char name[DNS_MAX_NAME];
    int  len;
    u32  ip;
    u32  expires;             /* in PIT ticks; 0 means never looked up */
    int  valid;
} cache[CACHE_N];
static int cache_next;

static int c_queries, c_replies, c_rejected, c_hits;
static unsigned dns_operation, dns_query_id;

enum {
    DNS_R_CACHE = 1, DNS_R_START, DNS_R_SEND_FAIL, DNS_R_NXDOMAIN,
    DNS_R_BAD_REPLY, DNS_R_ANSWER, DNS_R_TIMEOUT, DNS_R_RESET
};

static void dns_set_state(int next, unsigned reason)
{
    int old = state;
    if (old == next) return;
    state = next;
    zlt_event(ZLLOG_SUB_NET, ZLLOG_EV_NET_STATE,
              (next == DNS_REFUSED || next == DNS_TIMEOUT)
                  ? ZLLOG_WARN : ZLLOG_INFO,
              DNS_PORT, ((unsigned)old << 16) | (unsigned)next, reason);
    if (next == DNS_DONE) {
        zlt_lifecycle(ZLLOG_SUB_NET, ZLLOG_OBJ_DNS, dns_query_id,
                      ZLLOG_LIFE_READY, 0u, answer);
        zlt_operation_result(ZLLOG_SUB_NET, dns_operation,
                             ZLLOG_OP_DNS_RESOLVE, 1, 0u, answer);
        dns_operation = 0;
    } else if (next == DNS_REFUSED || next == DNS_NXDOMAIN ||
               next == DNS_TIMEOUT || next == DNS_IDLE) {
        unsigned error = next == DNS_NXDOMAIN ? 2u :
                         next == DNS_TIMEOUT ? 110u :
                         next == DNS_IDLE ? 125u : 5u;
        if (dns_operation) {
            zlt_operation_result(ZLLOG_SUB_NET, dns_operation,
                                 ZLLOG_OP_DNS_RESOLVE, -(int)error,
                                 error, reason);
            dns_operation = 0;
        }
        if (dns_query_id)
            zlt_lifecycle(ZLLOG_SUB_NET, ZLLOG_OBJ_DNS, dns_query_id,
                          next == DNS_IDLE ? ZLLOG_LIFE_EXIT : ZLLOG_LIFE_FAULT,
                          0u, reason);
    }
}

void dns_server(net_u32 ip) { server_ip = ip; }
net_u32 dns_get_server(void) { return server_ip; }

static u16 be16(const u8 *p) { return (u16)(((u16)p[0] << 8) | p[1]); }
static void put16(u8 *p, u16 v) { p[0] = (u8)(v >> 8); p[1] = (u8)v; }

static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

static unsigned dns_name_token(const char *name, int len)
{
    u32 h = 2166136261u;
    if (!name || len <= 0) return 0u;
    for (int i = 0; i < len; i++) { h ^= (u32)lower(name[i]); h *= 16777619u; }
    return h;
}

static int name_eq(const char *a, int alen, const char *b, int blen)
{
    if (alen != blen) return 0;
    for (int i = 0; i < alen; i++) if (lower(a[i]) != lower(b[i])) return 0;
    return 1;
}

/* ---- the cache ---------------------------------------------------------------
 * Eight fixed slots. A TTL is honoured because a resolver that never expires
 * is a resolver that keeps sending traffic to an address that moved, and the
 * failure looks like the site being down.
 */
void dns_cache_clear(void)
{
    for (int i = 0; i < CACHE_N; i++) cache[i].valid = 0;
    cache_next = 0;
}

int dns_cache_count(void)
{
    int n = 0;
    for (int i = 0; i < CACHE_N; i++) if (cache[i].valid) n++;
    return n;
}

static int cache_find(const char *n, int len)
{
    u32 now = idt_ticks();
    for (int i = 0; i < CACHE_N; i++) {
        if (!cache[i].valid) continue;
        if ((int)(now - cache[i].expires) >= 0) { cache[i].valid = 0; continue; }
        if (name_eq(cache[i].name, cache[i].len, n, len)) return i;
    }
    return -1;
}

static void cache_put(const char *n, int len, u32 ip, u32 ttl_secs)
{
    if (len <= 0 || len > DNS_MAX_NAME - 1) return;
    int i = cache_find(n, len);
    if (i < 0) { i = cache_next; cache_next = (cache_next + 1) % CACHE_N; }
    for (int k = 0; k < len; k++) cache[i].name[k] = n[k];
    cache[i].len = len;
    cache[i].ip = ip;
    /* Cap the lifetime. A server offering a TTL of four billion seconds is
     * either broken or trying to pin an address in place for good. */
    if (ttl_secs > 3600) ttl_secs = 3600;
    if (ttl_secs < 1) ttl_secs = 1;
    cache[i].expires = idt_ticks() + ttl_secs * 100;   /* the PIT is 100 Hz */
    cache[i].valid = 1;
}

/* ---- UDP ---------------------------------------------------------------------
 * Eight bytes on top of IP. The checksum is optional in IPv4 and computed
 * anyway: it is four lines, net.c already has the routine, and a resolver that
 * accepts corrupted answers is worse than one that is slightly slower.
 */
#define IP_PROTO_UDP 17

static u8 udpbuf[PKT_MAX + 8];

static int udp_send(u32 dst, int sport, int dport, const u8 *data, int len)
{
    if (len < 0 || len > PKT_MAX) return 0;
    put16(udpbuf + 0, (u16)sport);
    put16(udpbuf + 2, (u16)dport);
    put16(udpbuf + 4, (u16)(8 + len));
    put16(udpbuf + 6, 0);
    for (int i = 0; i < len; i++) udpbuf[8 + i] = data[i];

    /* the pseudo-header, exactly as TCP's checksum uses one */
    u32 sum = 0;
    u32 src = net_ip();
    sum += (src >> 16) & 0xFFFF; sum += src & 0xFFFF;
    sum += (dst >> 16) & 0xFFFF; sum += dst & 0xFFFF;
    sum += IP_PROTO_UDP;
    sum += (u32)(8 + len);
    u16 ck = net_checksum(udpbuf, 8 + len, sum);
    /* a computed checksum of zero is transmitted as all ones - zero means
     * "no checksum" on the wire and the two must not be confused */
    put16(udpbuf + 6, ck ? ck : 0xFFFF);

    return net_send_ip(dst, IP_PROTO_UDP, udpbuf, 8 + len);
}

/* ---- names on the wire --------------------------------------------------------
 * "example.com" becomes 7 e x a m p l e 3 c o m 0.
 */
static int encode_name(const char *n, int len, u8 *out, int max)
{
    int o = 0, i = 0;
    while (i < len) {
        int j = i;
        while (j < len && n[j] != '.') j++;
        int lab = j - i;
        if (lab <= 0 || lab > 63) return 0;          /* not a name we can send */
        if (o + lab + 1 >= max) return 0;
        out[o++] = (u8)lab;
        for (int k = i; k < j; k++) out[o++] = (u8)n[k];
        i = (j < len) ? j + 1 : j;
    }
    if (o + 1 > max) return 0;
    out[o++] = 0;
    return o;
}

/* Step over a name in a response, following compression pointers only far
 * enough to know where the name ENDS. Returns the offset just past the name in
 * the packet, or -1 if it is malformed.
 *
 * A pointer ends the name as far as the containing record is concerned, which
 * is why this does not need to follow the chain at all - and not following it
 * is also the simplest way to be immune to a cycle. */
static int skip_name(const u8 *p, int len, int off)
{
    int steps = 0;
    while (off >= 0 && off < len) {
        if (++steps > MAX_STEPS) return -1;
        u8 b = p[off];
        if ((b & 0xC0) == 0xC0) {
            if (off + 1 >= len) return -1;
            return off + 2;                          /* a pointer ends it */
        }
        if (b & 0xC0) return -1;                     /* reserved label type */
        if (b == 0) return off + 1;
        off += 1 + b;
    }
    return -1;
}

/* Read a name into `out`, following compression pointers, with a hard budget.
 * Used only to check that the answer echoes the question we asked. */
static int read_name(const u8 *p, int len, int off, char *out, int max)
{
    int o = 0, steps = 0, jumps = 0;
    while (off >= 0 && off < len) {
        if (++steps > MAX_STEPS) return -1;
        u8 b = p[off];
        if ((b & 0xC0) == 0xC0) {
            if (off + 1 >= len) return -1;
            /* A POINTER MAY POINT ANYWHERE, INCLUDING AT ITSELF.
             *
             * TWO INDEPENDENT BUDGETS, and either alone terminates every
             * cycle - the step count above bounds total iterations, and this
             * bounds redirections specifically. That redundancy is deliberate
             * for a parser reading hostile data, and it has a consequence
             * worth writing down: a test CANNOT distinguish them. Deleting
             * this line leaves the harness green, because the step budget
             * catches the same inputs. Kept anyway; belt and braces on a
             * remote parser is not waste. */
            if (++jumps > 8) return -1;
            int t = ((int)(b & 0x3F) << 8) | p[off + 1];
            if (t >= len) return -1;
            off = t;
            continue;
        }
        if (b & 0xC0) return -1;
        if (b == 0) { out[o] = 0; return o; }
        if (off + 1 + b > len) return -1;
        if (o && o < max - 1) out[o++] = '.';
        for (int k = 0; k < b; k++) {
            if (o < max - 1) out[o++] = (char)p[off + 1 + k];
        }
        off += 1 + b;
    }
    return -1;
}

/* ---- asking -------------------------------------------------------------------- */
static u8 qbuf[PKT_MAX];

static int send_query(void)
{
    u8 name[DNS_MAX_NAME + 2];
    int nl = encode_name(asking, asking_len, name, (int)sizeof name);
    if (!nl) return 0;
    if (12 + nl + 4 > PKT_MAX) return 0;

    /* NOT A RANDOM NUMBER, and pretending otherwise would be the dishonest
     * part. There is no entropy source in this kernel; the tick counter mixed
     * with a sequence differs between queries, which stops a stale answer
     * matching a new question. It is NOT a defence against an off-path
     * attacker who can guess it - that needs real randomness and 0x20
     * encoding, and neither is here. */
    static u16 seq;
    txid = (u16)((idt_ticks() << 3) ^ (u16)(0x9E37u * ++seq));
    local_port = (u16)(50000 + (seq & 0x0FFF));

    put16(qbuf + 0, txid);
    put16(qbuf + 2, 0x0100);          /* standard query, recursion desired */
    put16(qbuf + 4, 1);               /* one question */
    put16(qbuf + 6, 0);
    put16(qbuf + 8, 0);
    put16(qbuf + 10, 0);
    for (int i = 0; i < nl; i++) qbuf[12 + i] = name[i];
    put16(qbuf + 12 + nl, 1);         /* QTYPE  A     */
    put16(qbuf + 14 + nl, 1);         /* QCLASS IN    */

    c_queries++;
    return udp_send(server_ip, local_port, DNS_PORT, qbuf, 12 + nl + 4);
}

int dns_start(const char *name, int len)
{
    unsigned token = dns_name_token(name, len);
    unsigned operation = zlt_operation_begin(
        ZLLOG_SUB_NET, ZLLOG_OBJ_KERNEL, 0u,
        ZLLOG_OP_DNS_RESOLVE, token);
    if (state == DNS_ASKING || !name || len <= 0 ||
        len > DNS_MAX_NAME - 1 || !server_ip) {
        unsigned error = state == DNS_ASKING ? 16u : 22u;
        zlt_operation_result(ZLLOG_SUB_NET, operation,
                             ZLLOG_OP_DNS_RESOLVE, -(int)error, error, token);
        return 0;
    }
    dns_operation = operation;
    dns_query_id++;
    if (!dns_query_id) dns_query_id++;
    zlt_lifecycle(ZLLOG_SUB_NET, ZLLOG_OBJ_DNS, dns_query_id,
                  ZLLOG_LIFE_START, 0u, token);

    int ci = cache_find(name, len);
    if (ci >= 0) {
        answer = cache[ci].ip;
        dns_set_state(DNS_DONE, DNS_R_CACHE);
        c_hits++;
        return 1;
    }

    /* The resolver is usually NOT the gateway - on QEMU it is .3 where the
     * gateway is .2 - so its hardware address has to be learned separately.
     * net_ping resolves before it sends for the same reason. */
    if (!net_arp_resolve(server_ip, 0, 500)) {
        zlt_operation_result(ZLLOG_SUB_NET, dns_operation,
                             ZLLOG_OP_DNS_RESOLVE, -113, 113u, token);
        zlt_lifecycle(ZLLOG_SUB_NET, ZLLOG_OBJ_DNS, dns_query_id,
                      ZLLOG_LIFE_FAULT, 0u, 113u);
        dns_operation = 0;
        return 0;
    }

    for (int i = 0; i < len; i++) asking[i] = name[i];
    asking_len = len;
    answer = 0;
    dns_set_state(DNS_ASKING, DNS_R_START);
    deadline = idt_ticks() + 300;              /* three seconds */
    if (!send_query()) { dns_set_state(DNS_REFUSED, DNS_R_SEND_FAIL); return 0; }
    return 1;
}

/* ---- the answer ----------------------------------------------------------------- */
void dns_input(u32 src, const u8 *p, int len)
{
    if (state != DNS_ASKING) return;
    if (len < 8 + 12) { c_rejected++; return; }

    /* Anyone can send us a UDP packet. It must come from the server we asked,
     * to the port we asked from, or it is not an answer to our question. */
    if (src != server_ip)              { c_rejected++; return; }
    if (be16(p + 0) != DNS_PORT)       { c_rejected++; return; }
    if (be16(p + 2) != local_port)     { c_rejected++; return; }

    int ulen = (int)be16(p + 4);
    if (ulen < 8 || ulen > len)        { c_rejected++; return; }

    const u8 *d = p + 8;
    int dlen = ulen - 8;
    if (dlen < 12)                     { c_rejected++; return; }
    if (dlen > PKT_MAX) dlen = PKT_MAX;

    if (be16(d + 0) != txid)           { c_rejected++; return; }
    u16 flags = be16(d + 2);
    if (!(flags & 0x8000))             { c_rejected++; return; }   /* not a reply */

    c_replies++;
    int rcode = flags & 0x000F;
    if (rcode == 3) { dns_set_state(DNS_NXDOMAIN, DNS_R_NXDOMAIN); return; }
    if (rcode != 0) { dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY); return; }

    int qd = (int)be16(d + 4);
    int an = (int)be16(d + 6);
    if (qd != 1 || an < 1)             { dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY); c_rejected++; return; }

    /* THE ANSWER MUST BE TO THE QUESTION WE ASKED. Without this a server can
     * answer any query with a record for any name and have it cached. */
    char echoed[DNS_MAX_NAME];
    if (read_name(d, dlen, 12, echoed, (int)sizeof echoed) < 0) {
        dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY); c_rejected++; return;
    }
    int el = 0; while (echoed[el]) el++;
    if (!name_eq(echoed, el, asking, asking_len)) {
        dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY); c_rejected++; return;
    }

    int off = skip_name(d, dlen, 12);
    if (off < 0 || off + 4 > dlen)     { dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY); c_rejected++; return; }
    off += 4;                                   /* the question's type + class */

    for (int i = 0; i < an && i < 32; i++) {
        off = skip_name(d, dlen, off);
        if (off < 0 || off + 10 > dlen) break;
        u16 type  = be16(d + off + 0);
        u16 class = be16(d + off + 2);
        u32 ttl   = ((u32)d[off+4] << 24) | ((u32)d[off+5] << 16) |
                    ((u32)d[off+6] << 8)  | d[off+7];
        int rdlen = (int)be16(d + off + 8);
        off += 10;
        if (rdlen < 0 || off + rdlen > dlen) break;

        if (type == 1 && class == 1 && rdlen == 4) {
            answer = ((u32)d[off] << 24) | ((u32)d[off+1] << 16) |
                     ((u32)d[off+2] << 8) | d[off+3];
            /* 0.0.0.0 and 127.x are not answers a resolver should hand to a
             * browser - the first is meaningless and the second points the
             * fetch back at ourselves, which is how a hostile answer turns an
             * outbound request into a local one. */
            if (answer == 0 || (answer >> 24) == 127) {
                answer = 0;
                dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY);
                c_rejected++;
                return;
            }
            cache_put(asking, asking_len, answer, ttl);
            dns_set_state(DNS_DONE, DNS_R_ANSWER);
            return;
        }
        off += rdlen;                           /* a CNAME, or something else */
    }
    dns_set_state(DNS_REFUSED, DNS_R_BAD_REPLY);
    c_rejected++;
}

/* net.c's sink hands over (src, proto, payload, len); the resolver only ever
 * wants UDP. The shim is here rather than a wider dns_input signature because
 * dns_input's caller in a test should not have to invent a protocol number. */
void dns_ip_sink(net_u32 src, int proto, const net_u8 *p, int len)
{
    if (proto != IP_PROTO_UDP) return;
    dns_input(src, p, len);
}

int dns_poll(void)
{
    if (state == DNS_ASKING && (int)(idt_ticks() - deadline) >= 0) {
        zlt_trigger(ZLLOG_SUB_NET, ZLLOG_EV_TIMEOUT, ZLLOG_ERROR,
                    DNS_PORT, txid, 300u);
        dns_set_state(DNS_TIMEOUT, DNS_R_TIMEOUT);
    }
    return state;
}

/* Abandon whatever is in flight. The browser needs this the moment someone
 * types a second address before the first has resolved - without it the
 * resolver refuses the new lookup and the address bar appears to do nothing.
 * The cache is deliberately kept: a cancelled lookup does not invalidate
 * answers that already arrived. */
void dns_reset(void)
{
    dns_set_state(DNS_IDLE, DNS_R_RESET);
    answer = 0;
    asking_len = 0;
}

int dns_state(void)      { return state; }
net_u32 dns_result(void) { return answer; }
int dns_queries(void)    { return c_queries; }
int dns_replies(void)    { return c_replies; }
int dns_rejected(void)   { return c_rejected; }
int dns_cache_hits(void) { return c_hits; }

int dns_resolve(const char *name, int len, u32 *out, int ms)
{
    if (!dns_start(name, len)) return 0;
    u32 t0 = idt_ticks();
    u32 ticks = (u32)(ms / 10) + 1;
    long spins = (long)ms * 20000;
    while (spins-- > 0) {
        net_poll_once();
        int s = dns_poll();
        if (s != DNS_ASKING) break;
        if (idt_ticks() - t0 >= ticks) break;
    }
    if (state == DNS_DONE) { if (out) *out = answer; return 1; }
    return 0;
}
