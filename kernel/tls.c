/* tls.c - a TLS 1.3 client handshake, one ciphersuite.
 *
 * See tls.h for what this deliberately does NOT do (verify certificates) and
 * why saying so is part of the code rather than a footnote.
 *
 * THE SHAPE. TLS 1.3 is three things stacked:
 *
 *   1. A RECORD LAYER. Everything is a 5-byte header and a payload. After the
 *      handshake keys exist, the payload is AES-GCM with the header itself as
 *      the additional data - which is what stops a record being replayed as a
 *      different content type.
 *   2. A KEY SCHEDULE. A chain of HKDF derivations, each one salted by the
 *      hash of every handshake byte so far. That transcript binding is the
 *      whole security argument: a key is only valid for the exact conversation
 *      that produced it.
 *   3. A STATE MACHINE. ClientHello out; ServerHello in (which is where the
 *      shared secret appears); then everything else arrives encrypted.
 *
 * NO HEAP, like everything here: one struct per connection, all fixed arrays,
 * and a message larger than the buffer is a refused connection rather than a
 * smashed stack.
 *
 * ENDIANNESS IS EXPLICIT EVERYWHERE. Every length in TLS is big-endian and this
 * kernel is little-endian, so every read and write goes through be16/be24 -
 * never a cast to a u16 pointer.
 */
#include "tls.h"
#include "crypto.h"
#include "x509.h"

typedef unsigned char u8;
typedef unsigned int  u32;

int ecdsa_verify(int curve_bits, const u8 *pub, const u8 *r, const u8 *s,
                 const u8 *hash, int hashlen);

static void tmemcpy(void *d, const void *s, int n)
{
    u8 *dp = (u8 *)d; const u8 *sp = (const u8 *)s;
    while (n-- > 0) *dp++ = *sp++;
}
static void tmemset(void *d, int v, int n)
{
    u8 *p = (u8 *)d;
    while (n-- > 0) *p++ = (u8)v;
}
static int tstrlen(const char *s) { int n = 0; while (s[n]) n++; return n; }

/* ---- big-endian readers and writers --------------------------------------- */
static int be16(const u8 *p) { return (p[0] << 8) | p[1]; }
static int be24(const u8 *p) { return (p[0] << 16) | (p[1] << 8) | p[2]; }

/* a growing byte buffer with a hard ceiling; every put checks it */
struct buf { u8 *p; int n, max; int over; };

static void put(struct buf *b, const void *src, int n)
{
    if (b->n + n > b->max) { b->over = 1; return; }
    tmemcpy(b->p + b->n, src, n);
    b->n += n;
}
static void put8(struct buf *b, int v)  { u8 x = (u8)v; put(b, &x, 1); }
static void put16(struct buf *b, int v) { u8 x[2] = { (u8)(v >> 8), (u8)v }; put(b, x, 2); }
static void put24(struct buf *b, int v) { u8 x[3] = { (u8)(v >> 16), (u8)(v >> 8), (u8)v }; put(b, x, 3); }

/* ---- the transcript --------------------------------------------------------
 * Every handshake message, in order, exactly as it went on or came off the
 * wire. The hash of a PREFIX of this is what each secret is bound to, so the
 * bytes are kept rather than a running hash context. */
static void tx_add(struct tls_conn *c, const u8 *p, int n)
{
    if (c->txn + n > TLS_HS_MAX) { c->err = TLS_E_OVERFLOW; c->state = TLS_ERROR; return; }
    tmemcpy(c->tx + c->txn, p, n);
    c->txn += n;
}
static void tx_hash(struct tls_conn *c, u8 *out) { sha256(c->tx, (u32)c->txn, out); }

/* ---- the key schedule (RFC 8446 SS7.1) -------------------------------------
 * Every arrow in the RFC's diagram is one line here. The zero-filled `zero`
 * is the RFC's own convention for "no PSK" and "no IKM".
 */
static void derive_hs_keys(struct tls_conn *c)
{
    u8 zero[32], early[32], derived[32], thash[32], empty_hash[32];
    tmemset(zero, 0, 32);
    sha256((const u8 *)"", 0, empty_hash);

    hkdf_extract(zero, 0, zero, 32, early);              /* Early Secret     */
    tls13_derive_secret(early, "derived", empty_hash, derived);
    hkdf_extract(derived, 32, c->shared, 32, c->hs_secret);   /* Handshake   */

    tx_hash(c, thash);                                   /* Hash(CH..SH)     */
    tls13_derive_secret(c->hs_secret, "c hs traffic", thash, c->c_hs);
    tls13_derive_secret(c->hs_secret, "s hs traffic", thash, c->s_hs);

    tls13_expand_label(c->c_hs, "key", zero, 0, c->c_key, 16);
    tls13_expand_label(c->c_hs, "iv",  zero, 0, c->c_iv,  12);
    tls13_expand_label(c->s_hs, "key", zero, 0, c->s_key, 16);
    tls13_expand_label(c->s_hs, "iv",  zero, 0, c->s_iv,  12);
    c->c_seq = c->s_seq = 0;
}

static void derive_app_keys(struct tls_conn *c)
{
    u8 zero[32], derived[32], thash[32], empty_hash[32];
    tmemset(zero, 0, 32);
    sha256((const u8 *)"", 0, empty_hash);

    tls13_derive_secret(c->hs_secret, "derived", empty_hash, derived);
    hkdf_extract(derived, 32, zero, 32, c->master);      /* Master Secret    */

    tx_hash(c, thash);                                   /* Hash(CH..SF)     */
    tls13_derive_secret(c->master, "c ap traffic", thash, c->c_ap);
    tls13_derive_secret(c->master, "s ap traffic", thash, c->s_ap);
}

static void install_keys(struct tls_conn *c, int app)
{
    u8 zero[32];
    tmemset(zero, 0, 32);
    const u8 *cs = app ? c->c_ap : c->c_hs;
    const u8 *ss = app ? c->s_ap : c->s_hs;
    tls13_expand_label(cs, "key", zero, 0, c->c_key, 16);
    tls13_expand_label(cs, "iv",  zero, 0, c->c_iv,  12);
    tls13_expand_label(ss, "key", zero, 0, c->s_key, 16);
    tls13_expand_label(ss, "iv",  zero, 0, c->s_iv,  12);
    c->c_seq = c->s_seq = 0;
}

/* The per-record nonce is the static IV XOR the sequence number in the last 8
 * bytes. Reusing a nonce with the same key is catastrophic for GCM - it leaks
 * the authentication key - which is why the counter lives in the connection
 * and is only ever reset alongside a new key. */
static void nonce_of(const u8 *iv, unsigned long long seq, u8 *out)
{
    tmemcpy(out, iv, 12);
    for (int i = 0; i < 8; i++)
        out[11 - i] ^= (u8)(seq >> (8 * i));
}

/* ---- sending ---------------------------------------------------------------
 * A plaintext record before the keys exist; an encrypted one after. In TLS 1.3
 * the outer content type of an encrypted record is always application_data
 * (23) and the REAL type is the last byte of the plaintext - that indirection
 * is what hides handshake structure from an observer.
 */
static void send_plain(struct tls_conn *c, int type, const u8 *p, int n)
{
    if (c->outn + 5 + n > TLS_OUT_MAX) { c->err = TLS_E_OVERFLOW; c->state = TLS_ERROR; return; }
    u8 *o = c->out + c->outn;
    o[0] = (u8)type; o[1] = 3; o[2] = 3;
    o[3] = (u8)(n >> 8); o[4] = (u8)n;
    tmemcpy(o + 5, p, n);
    c->outn += 5 + n;
}

static void send_enc(struct tls_conn *c, int type, const u8 *p, int n)
{
    /* payload || real type, then 16 bytes of tag */
    int len = n + 1 + 16;
    if (c->outn + 5 + len > TLS_OUT_MAX) { c->err = TLS_E_OVERFLOW; c->state = TLS_ERROR; return; }
    u8 *o = c->out + c->outn;
    o[0] = 23; o[1] = 3; o[2] = 3;
    o[3] = (u8)(len >> 8); o[4] = (u8)len;
    tmemcpy(o + 5, p, n);
    o[5 + n] = (u8)type;
    u8 nonce[12];
    nonce_of(c->c_iv, c->c_seq++, nonce);
    /* THE HEADER IS THE ADDITIONAL DATA. That is what binds the length and the
     * outer type to the ciphertext. */
    aes128_gcm_encrypt(c->c_key, nonce, o, 5, o + 5, n + 1, o + 5 + n + 1);
    c->outn += 5 + len;
}

/* ---- ClientHello ----------------------------------------------------------- */
static void send_client_hello(struct tls_conn *c)
{
    u8 body[512];
    struct buf b = { body, 0, (int)sizeof body, 0 };

    put16(&b, 0x0303);                       /* legacy_version = TLS 1.2     */
    /* 32 bytes of client random. THIS IS NOT A SECRET but it must be unique
     * per connection, or two handshakes derive the same keys. There is no RNG
     * in this kernel yet, so it is derived from the ephemeral private key,
     * which IS random - see tls_start. */
    put(&b, c->pub, 32);
    put8(&b, 0);                             /* no legacy session id         */
    put16(&b, 2); put16(&b, 0x1301);         /* TLS_AES_128_GCM_SHA256 only  */
    put8(&b, 1); put8(&b, 0);                /* no compression               */

    /* extensions */
    u8 ext[384];
    struct buf e = { ext, 0, (int)sizeof ext, 0 };

    /* server_name (SNI) - mandatory for any shared-IP server, i.e. most */
    int hl = tstrlen(c->host);
    if (hl > 0) {
        put16(&e, 0x0000);
        put16(&e, hl + 5);
        put16(&e, hl + 3);
        put8(&e, 0);                          /* host_name                   */
        put16(&e, hl);
        put(&e, c->host, hl);
    }
    /* supported_groups: x25519 */
    put16(&e, 0x000A); put16(&e, 4); put16(&e, 2); put16(&e, 0x001D);
    /* signature_algorithms - we do not verify, but a server will abort
     * without it. Offering the common set keeps us interoperable. */
    {
        static const int algs[] = { 0x0403, 0x0804, 0x0401, 0x0503, 0x0805,
                                    0x0501, 0x0806, 0x0601 };
        int n = (int)(sizeof algs / sizeof algs[0]);
        put16(&e, 0x000D); put16(&e, 2 + n * 2); put16(&e, n * 2);
        for (int i = 0; i < n; i++) put16(&e, algs[i]);
    }
    /* supported_versions: TLS 1.3 only */
    put16(&e, 0x002B); put16(&e, 3); put8(&e, 2); put16(&e, 0x0304);
    /* key_share: our x25519 public key */
    put16(&e, 0x0033); put16(&e, 38); put16(&e, 36);
    put16(&e, 0x001D); put16(&e, 32); put(&e, c->pub, 32);

    put16(&b, e.n);
    put(&b, ext, e.n);

    if (b.over || e.over) { c->err = TLS_E_OVERFLOW; c->state = TLS_ERROR; return; }

    /* wrap in a handshake header, record it in the transcript, send it */
    u8 msg[600];
    struct buf m = { msg, 0, (int)sizeof msg, 0 };
    put8(&m, 1);                              /* client_hello               */
    put24(&m, b.n);
    put(&m, body, b.n);
    if (m.over) { c->err = TLS_E_OVERFLOW; c->state = TLS_ERROR; return; }

    tx_add(c, msg, m.n);
    send_plain(c, 22, msg, m.n);
    c->state = TLS_WAIT_SH;
}

void tls_trust(struct tls_conn *c, const struct x509_cert *roots, int nroots,
               const char *nowZ)
{
    c->verify = 1;
    c->roots = roots;
    c->nroots = nroots;
    c->nowZ = nowZ;
}

void tls_start(struct tls_conn *c, const char *host)
{
    /* the trust settings survive tls_start, because a caller naturally calls
     * tls_trust first and would otherwise silently lose them */
    int v = c->verify;
    const struct x509_cert *rt = c->roots;
    int nr = c->nroots;
    const char *nz = c->nowZ;
    tmemset(c, 0, (int)sizeof *c);
    c->verify = v; c->roots = rt; c->nroots = nr; c->nowZ = nz;
    int i = 0;
    while (host && host[i] && i < TLS_HOST_MAX - 1) { c->host[i] = host[i]; i++; }
    c->host[i] = 0;

    /* THE EPHEMERAL KEY. There is no entropy source in this kernel, and that
     * is a REAL limitation rather than an oversight: a predictable private key
     * makes the whole exchange readable by anyone who can guess it. The caller
     * is expected to have filled c->priv with something unpredictable before
     * calling; tls_start only clamps and derives. A caller that leaves it zero
     * gets a working handshake with NO confidentiality, which is why this is
     * stated here and on the browser's screen rather than hidden. */
    x25519_base(c->pub, c->priv);
    send_client_hello(c);
}

/* ---- ServerHello ----------------------------------------------------------- */
static int parse_server_hello(struct tls_conn *c, const u8 *p, int n)
{
    if (n < 38) { c->err = TLS_E_PROTOCOL; return -1; }
    int i = 2 + 32;                            /* version + random           */
    int sid = p[i++];
    i += sid;
    if (i + 3 > n) { c->err = TLS_E_PROTOCOL; return -1; }
    int suite = be16(p + i); i += 2;
    i += 1;                                    /* compression                */
    if (suite != 0x1301) { c->err = TLS_E_SUITE; return -1; }
    if (i + 2 > n) { c->err = TLS_E_PROTOCOL; return -1; }
    int elen = be16(p + i); i += 2;
    int end = i + elen;
    if (end > n) { c->err = TLS_E_PROTOCOL; return -1; }

    int got_share = 0, got_ver = 0;
    while (i + 4 <= end) {
        int et = be16(p + i), el = be16(p + i + 2);
        i += 4;
        if (i + el > end) { c->err = TLS_E_PROTOCOL; return -1; }
        if (et == 0x002B) {                    /* supported_versions         */
            if (el >= 2 && be16(p + i) == 0x0304) got_ver = 1;
        } else if (et == 0x0033) {             /* key_share                  */
            if (el >= 4 && be16(p + i) == 0x001D && be16(p + i + 2) == 32 && el >= 36) {
                x25519(c->shared, c->priv, p + i + 4);
                got_share = 1;
            }
        }
        i += el;
    }
    if (!got_ver)   { c->err = TLS_E_VERSION; return -1; }
    if (!got_share) { c->err = TLS_E_GROUP;   return -1; }
    return 0;
}

/* ---- the client's Finished -------------------------------------------------
 * finished_key = Expand-Label(traffic_secret, "finished", "", 32)
 * verify_data  = HMAC(finished_key, Hash(transcript so far))
 * It proves we hold the handshake secret, which proves we completed the same
 * key exchange - and because the transcript is in the hash, that we saw the
 * same messages, unmodified. */
static void send_finished(struct tls_conn *c)
{
    u8 zero[32], fkey[32], thash[32], verify[32], msg[4 + 32];
    tmemset(zero, 0, 32);
    tls13_expand_label(c->c_hs, "finished", zero, 0, fkey, 32);
    tx_hash(c, thash);
    hmac_sha256(fkey, 32, thash, 32, verify);
    msg[0] = 20;                               /* finished                   */
    msg[1] = 0; msg[2] = 0; msg[3] = 32;
    tmemcpy(msg + 4, verify, 32);
    send_enc(c, 22, msg, 36);
}

static int check_server_finished(struct tls_conn *c, const u8 *body, int n,
                                 const u8 *thash_before)
{
    u8 zero[32], fkey[32], verify[32];
    tmemset(zero, 0, 32);
    if (n != 32) { c->err = TLS_E_FINISHED; return -1; }
    tls13_expand_label(c->s_hs, "finished", zero, 0, fkey, 32);
    hmac_sha256(fkey, 32, thash_before, 32, verify);
    if (!crypto_equal(verify, body, 32)) { c->err = TLS_E_FINISHED; return -1; }
    return 0;
}

/* ---- the certificate, and the proof the server holds its key ---------------
 * TWO SEPARATE CHECKS, and skipping either makes the other pointless:
 *
 *   THE CHAIN says the certificate is genuine - a real CA issued it, for this
 *   host, and it has not expired. x509.c does that.
 *
 *   CERTIFICATEVERIFY says the peer HOLDS THE PRIVATE KEY for it. Certificates
 *   are public: anyone can download Wikipedia's whole chain and replay it. A
 *   client that checks only the chain accepts an impostor presenting a real
 *   certificate, which is the entire attack TLS exists to stop.
 *
 * The signed content is fixed by RFC 8446 SS4.4.3: 64 spaces, a context string,
 * a zero byte, then the transcript hash up to and including Certificate. The 64
 * spaces exist so a signature made here can never be mistaken for one made over
 * a certificate's TBS - a cross-protocol confusion that was a real attack.
 */
static int verify_cert_chain(struct tls_conn *c, const u8 *thash);

static int check_cert_verify(const u8 *body, int len,
                             const u8 *thash, const struct x509_cert *leaf)
{
    if (len < 4) return -1;
    int scheme = be16(body);
    int siglen = be16(body + 2);
    if (4 + siglen > len) return -1;
    const u8 *sig = body + 4;

    int hbits;
    if (scheme == 0x0403)      hbits = 256;    /* ecdsa_secp256r1_sha256 */
    else if (scheme == 0x0503) hbits = 384;    /* ecdsa_secp384r1_sha384 */
    else if (scheme == 0x0603) hbits = 512;    /* ecdsa_secp521r1 - unsupported */
    else return -1;                            /* RSA schemes: unsupported */
    if (hbits == 512) return -1;

    /* the content that was signed */
    static u8 sc[64 + 34 + 32];
    int n = 0;
    for (int i = 0; i < 64; i++) sc[n++] = 0x20;
    const char *ctx = "TLS 1.3, server CertificateVerify";
    for (int i = 0; ctx[i]; i++) sc[n++] = (u8)ctx[i];
    sc[n++] = 0;
    for (int i = 0; i < 32; i++) sc[n++] = thash[i];

    u8 h[48];
    int hl;
    if (hbits == 384) { sha384(sc, (u32)n, h); hl = 48; }
    else              { sha256(sc, (u32)n, h); hl = 32; }

    /* the DER SEQUENCE { r, s } - reuse x509.c's decoder shape inline */
    if (siglen < 8 || sig[0] != 0x30) return -1;
    int p = 2;
    if (sig[1] & 0x80) p = 2 + (sig[1] & 0x7F);
    if (p + 2 > siglen || sig[p] != 0x02) return -1;
    int rl = sig[p + 1];
    const u8 *rv = sig + p + 2;
    p = p + 2 + rl;
    if (p + 2 > siglen || sig[p] != 0x02) return -1;
    int sl = sig[p + 1];
    const u8 *sv = sig + p + 2;
    if (p + 2 + sl > siglen) return -1;

    int size = leaf->curve_bits / 8;
    u8 r[48], s[48];
    tmemset(r, 0, 48); tmemset(s, 0, 48);
    while (rl > 0 && rv[0] == 0) { rv++; rl--; }
    while (sl > 0 && sv[0] == 0) { sv++; sl--; }
    if (rl > size || sl > size) return -1;
    tmemcpy(r + size - rl, rv, rl);
    tmemcpy(s + size - sl, sv, sl);

    return ecdsa_verify(leaf->curve_bits, leaf->pubkey, r, s, h, hl) ? 0 : -1;
}

static int verify_cert_chain(struct tls_conn *c, const u8 *thash)
{
    if (!c->verify) return 0;              /* the caller knowingly opted out */
    if (!c->saw_cert || c->ncerts <= 0) {
        c->err = TLS_E_CERT; c->state = TLS_ERROR; return -1;
    }
    const u8 *ders[8];
    int lens[8];
    for (int i = 0; i < c->ncerts; i++) {
        ders[i] = c->tx + c->cert_off[i];
        lens[i] = c->cert_len[i];
    }
    if (!x509_chain_ok(ders, lens, c->ncerts, c->roots, c->nroots,
                       c->host, c->nowZ)) {
        c->err = TLS_E_CERT; c->state = TLS_ERROR; return -1;
    }
    struct x509_cert leaf;
    if (!x509_parse(ders[0], lens[0], &leaf)) {
        c->err = TLS_E_CERT; c->state = TLS_ERROR; return -1;
    }
    c->cert_ok = 1;
    /* the CertificateVerify body is handled by the caller, which has it */
    c->cv_hashlen = 32;
    tmemcpy(c->cv_hash, thash, 32);
    return 0;
}

/* ---- handshake messages inside the encrypted stream ------------------------ */
static int handle_handshake(struct tls_conn *c, const u8 *p, int n)
{
    int i = 0;
    while (i + 4 <= n) {
        int type = p[i];
        int len = be24(p + i + 1);
        if (i + 4 + len > n) break;            /* a partial message          */
        const u8 *body = p + i + 4;

        if (type == 20) {                      /* server Finished            */
            /* the hash must cover everything BEFORE this message */
            u8 thash[32];
            tx_hash(c, thash);
            if (check_server_finished(c, body, len, thash) < 0) return -1;
            tx_add(c, p + i, 4 + len);
            /* A VERIFYING CONNECTION THAT NEVER SAW A CERTIFICATE MUST NOT
             * COMPLETE. Without this a server that simply omits Certificate
             * and CertificateVerify gets an encrypted, unauthenticated session
             * - the check would be skipped rather than failed. */
            if (c->verify && (!c->cert_ok || !c->saw_cv)) {
                c->err = TLS_E_CERT; c->state = TLS_ERROR; return -1;
            }
            derive_app_keys(c);                /* master, from CH..SF        */
            send_finished(c);                  /* still under handshake keys */
            install_keys(c, 1);                /* now switch to application  */
            c->state = TLS_READY;
            c->saw_fin = 1;
            i += 4 + len;
            continue;
        }
        if (type == 11) {                      /* Certificate */
            /* Certificate ::= { context<1>, list<3>, {cert<3>, ext<2>}... }
             * The certificates are recorded as offsets into the TRANSCRIPT,
             * which already keeps every handshake byte - so tx_add first and
             * then point into it. */
            int base = c->txn + 4;             /* where the body will land */
            tx_add(c, p + i, 4 + len);
            if (c->state == TLS_ERROR) return -1;
            int j = 0;
            if (j >= len) { c->err = TLS_E_PROTOCOL; return -1; }
            int ctxlen = body[j]; j += 1 + ctxlen;
            if (j + 3 > len) { c->err = TLS_E_PROTOCOL; return -1; }
            int listlen = be24(body + j); j += 3;
            int end = j + listlen;
            if (end > len) { c->err = TLS_E_PROTOCOL; return -1; }
            c->ncerts = 0;
            while (j + 3 <= end && c->ncerts < 8) {
                int clen = be24(body + j); j += 3;
                if (j + clen > end) break;
                c->cert_off[c->ncerts] = base + j;
                c->cert_len[c->ncerts] = clen;
                c->ncerts++;
                j += clen;
                if (j + 2 > end) break;
                int xl = (body[j] << 8) | body[j + 1];
                j += 2 + xl;
            }
            c->saw_cert = 1;
            i += 4 + len;
            continue;
        }

        if (type == 15) {                      /* CertificateVerify */
            /* THE SIGNATURE IS OVER THE TRANSCRIPT UP TO AND INCLUDING THE
             * CERTIFICATE - so the hash must be taken BEFORE this message is
             * added. This is the message that proves the server HOLDS the
             * private key. Without it a chain is worthless: certificates are
             * public, so anyone can replay Wikipedia's real chain. */
            u8 th[32];
            tx_hash(c, th);
            if (verify_cert_chain(c, th) < 0) return -1;
            if (c->verify) {
                struct x509_cert leaf;
                if (!x509_parse(c->tx + c->cert_off[0], c->cert_len[0], &leaf) ||
                    check_cert_verify(body, len, th, &leaf) < 0) {
                    c->err = TLS_E_CERTVERIFY; c->state = TLS_ERROR; return -1;
                }
                c->saw_cv = 1;
            }
            tx_add(c, p + i, 4 + len);
            i += 4 + len;
            continue;
        }

        /* EncryptedExtensions(8), NewSessionTicket(4): transcript only. */
        tx_add(c, p + i, 4 + len);
        i += 4 + len;
    }
    return i;
}

/* ---- one record ------------------------------------------------------------ */
static int handle_record(struct tls_conn *c, const u8 *rec, int len)
{
    int type = rec[0];
    int n = be16(rec + 3);

    if (type == 21 && c->state == TLS_WAIT_SH) {   /* plaintext alert        */
        c->err = TLS_E_ALERT; c->state = TLS_ERROR; return -1;
    }
    if (type == 20) return 0;                      /* change_cipher_spec: ignore */

    if (type == 22 && c->state == TLS_WAIT_SH) {
        const u8 *p = rec + 5;
        if (n < 4 || p[0] != 2) { c->err = TLS_E_PROTOCOL; c->state = TLS_ERROR; return -1; }
        int hl = be24(p + 1);
        if (hl + 4 > n) { c->err = TLS_E_PROTOCOL; c->state = TLS_ERROR; return -1; }
        if (parse_server_hello(c, p + 4, hl) < 0) { c->state = TLS_ERROR; return -1; }
        tx_add(c, p, 4 + hl);
        derive_hs_keys(c);
        c->saw_sh = 1;
        c->state = TLS_WAIT_FIN;
        return 0;
    }

    if (type == 23) {                              /* encrypted              */
        static u8 pt[TLS_REC_MAX];
        if (n < 17 || n > TLS_REC_MAX) { c->err = TLS_E_PROTOCOL; c->state = TLS_ERROR; return -1; }
        int body = n - 16;
        tmemcpy(pt, rec + 5, body);
        u8 nonce[12];
        nonce_of(c->s_iv, c->s_seq++, nonce);
        if (!aes128_gcm_decrypt(c->s_key, nonce, rec, 5, pt, (u32)body, rec + 5 + body)) {
            c->err = TLS_E_DECRYPT; c->state = TLS_ERROR; return -1;
        }
        /* strip the zero padding to find the real content type */
        int e = body - 1;
        while (e > 0 && pt[e] == 0) e--;
        int inner = pt[e];
        if (inner == 22) {
            if (handle_handshake(c, pt, e) < 0) { c->state = TLS_ERROR; return -1; }
        } else if (inner == 23) {
            if (c->appn + e <= TLS_REC_MAX) { tmemcpy(c->app + c->appn, pt, e); c->appn += e; }
        } else if (inner == 21) {
            /* close_notify is 1, anything else at level fatal ends it */
            if (e >= 2 && pt[1] == 0) c->state = TLS_CLOSED;
            else { c->err = TLS_E_ALERT; c->state = TLS_ERROR; return -1; }
        }
        return 0;
    }
    (void)len;
    return 0;
}

int tls_feed(struct tls_conn *c, const tu8 *data, int len)
{
    int used = 0;
    while (used < len) {
        int room = TLS_REC_MAX - c->inn;
        if (room <= 0) { c->err = TLS_E_OVERFLOW; c->state = TLS_ERROR; return -1; }
        int take = len - used;
        if (take > room) take = room;
        tmemcpy(c->in + c->inn, data + used, take);
        c->inn += take;
        used += take;

        /* drain whole records */
        for (;;) {
            if (c->inn < 5) break;
            int n = be16(c->in + 3);
            if (n < 0 || n > TLS_REC_MAX - 5) { c->err = TLS_E_PROTOCOL; c->state = TLS_ERROR; return -1; }
            if (c->inn < 5 + n) break;
            if (handle_record(c, c->in, 5 + n) < 0) return -1;
            int rest = c->inn - (5 + n);
            for (int i = 0; i < rest; i++) c->in[i] = c->in[5 + n + i];
            c->inn = rest;
            if (c->state == TLS_ERROR) return -1;
        }
    }
    return used;
}

int tls_take(struct tls_conn *c, const tu8 **p) { *p = c->out; return c->outn; }

void tls_sent(struct tls_conn *c, int n)
{
    if (n >= c->outn) { c->outn = 0; return; }
    for (int i = 0; i < c->outn - n; i++) c->out[i] = c->out[n + i];
    c->outn -= n;
}

int tls_write(struct tls_conn *c, const tu8 *data, int len)
{
    if (c->state != TLS_READY) return -1;
    int done = 0;
    while (done < len) {
        int chunk = len - done;
        if (chunk > 1024) chunk = 1024;         /* stay inside TLS_OUT_MAX   */
        send_enc(c, 23, data + done, chunk);
        if (c->state == TLS_ERROR) return -1;
        done += chunk;
    }
    return done;
}

int tls_read(struct tls_conn *c, tu8 *out, int max)
{
    int avail = c->appn - c->appr;
    if (avail <= 0) return 0;
    int n = avail < max ? avail : max;
    tmemcpy(out, c->app + c->appr, n);
    c->appr += n;
    if (c->appr == c->appn) c->appn = c->appr = 0;
    return n;
}

int tls_state(struct tls_conn *c) { return c->state; }
int tls_error(struct tls_conn *c) { return c->err; }
