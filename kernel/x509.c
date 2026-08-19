/* x509.c - DER parsing and certificate chain validation.
 *
 * DER IS A HOSTILE FORMAT AND THIS INPUT IS UNTRUSTED. A certificate arrives
 * from whoever answered the connection - including, on the attack this file
 * exists to stop, from the attacker. So every length is checked against the
 * remaining buffer before it is used, nothing is read past its parent, and
 * there is no recursion whose depth an attacker chooses. A parser that faults
 * on a malformed certificate is a remote crash; one that reads past a buffer
 * is worse.
 *
 * THE SHAPE OF A CERTIFICATE, which is most of what this file knows:
 *
 *   Certificate ::= SEQUENCE {
 *     tbsCertificate       TBSCertificate,     <- exactly the signed bytes
 *     signatureAlgorithm   AlgorithmIdentifier,
 *     signatureValue       BIT STRING }        <- SEQUENCE { r INTEGER, s INTEGER }
 *
 *   TBSCertificate ::= SEQUENCE {
 *     [0] version, serialNumber, signature, issuer,
 *     validity, subject, subjectPublicKeyInfo, [3] extensions }
 *
 * THE TBS BYTES MUST BE THE ORIGINAL ONES. What the CA signed is the exact
 * encoding it emitted, so this keeps a pointer and a length into the source
 * rather than re-encoding anything. Re-serialising and hashing that would fail
 * on every certificate whose encoding differs from ours by a byte.
 */
#include "x509.h"

typedef unsigned char u8;

int ecdsa_verify(int curve_bits, const u8 *pub, const u8 *r, const u8 *s,
                 const u8 *hash, int hashlen);
void sha256(const u8 *d, unsigned int n, u8 *out);
void sha384(const u8 *d, unsigned int n, u8 *out);

static const char *why = "";
const char *x509_why(void) { return why; }
static int fail(const char *m) { why = m; return 0; }

static int xmemcmp(const u8 *a, const u8 *b, int n)
{
    for (int i = 0; i < n; i++) if (a[i] != b[i]) return 1;
    return 0;
}
static void xmemcpy(u8 *d, const u8 *s, int n) { for (int i = 0; i < n; i++) d[i] = s[i]; }
static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }

/* ---- the DER walker --------------------------------------------------------
 * One cursor over a bounded span. Every step re-checks the bound, so a length
 * field claiming more than exists ends the walk instead of running off.
 */
struct der { const u8 *p; int n, i; };

static void d_init(struct der *d, const u8 *p, int n) { d->p = p; d->n = n; d->i = 0; }

/* Read one TLV. On success *tag, *val and *vlen describe it and the cursor is
 * past it. Returns 0 on any malformation - including a length that would run
 * past the end, which is the whole point. */
static int d_next(struct der *d, int *tag, const u8 **val, int *vlen)
{
    if (d->i + 2 > d->n) return 0;
    int t = d->p[d->i++];
    int l = d->p[d->i++];
    if (l & 0x80) {
        int nb = l & 0x7F;
        /* a length of a length longer than 3 bytes is a 16 MB field; nothing
         * legitimate does this and allowing it invites an overflow */
        if (nb == 0 || nb > 3 || d->i + nb > d->n) return 0;
        l = 0;
        for (int k = 0; k < nb; k++) l = (l << 8) | d->p[d->i++];
    }
    if (l < 0 || d->i + l > d->n) return 0;
    *tag = t;
    *val = d->p + d->i;
    *vlen = l;
    d->i += l;
    return 1;
}

/* the same, but keeping the header too - needed for TBS, whose signed form
 * includes its own SEQUENCE tag and length */
static int d_next_full(struct der *d, int *tag, const u8 **full, int *flen,
                       const u8 **val, int *vlen)
{
    int start = d->i;
    if (!d_next(d, tag, val, vlen)) return 0;
    *full = d->p + start;
    *flen = d->i - start;
    return 1;
}

/* ---- the OIDs we know ------------------------------------------------------
 * Compared as raw bytes rather than decoded. An OID is an opaque identifier
 * here; decoding it would be more code and no more certainty.
 */
static const u8 OID_ECDSA_SHA256[] = { 0x2A,0x86,0x48,0xCE,0x3D,0x04,0x03,0x02 };
static const u8 OID_ECDSA_SHA384[] = { 0x2A,0x86,0x48,0xCE,0x3D,0x04,0x03,0x03 };
static const u8 OID_EC_PUBKEY[]    = { 0x2A,0x86,0x48,0xCE,0x3D,0x02,0x01 };
static const u8 OID_P256[]         = { 0x2A,0x86,0x48,0xCE,0x3D,0x03,0x01,0x07 };
static const u8 OID_P384[]         = { 0x2B,0x81,0x04,0x00,0x22 };
static const u8 OID_SAN[]          = { 0x55,0x1D,0x11 };
static const u8 OID_BASIC[]        = { 0x55,0x1D,0x13 };

static int oid_is(const u8 *v, int n, const u8 *want, int wn)
{
    return n == wn && !xmemcmp(v, want, wn);
}

/* AlgorithmIdentifier ::= SEQUENCE { algorithm OID, parameters ANY }
 * Returns the hash size in bits, or 0 if it is not an ECDSA algorithm we do. */
static int alg_hash(const u8 *p, int n)
{
    struct der d;
    int tag, vlen;
    const u8 *val;
    d_init(&d, p, n);
    if (!d_next(&d, &tag, &val, &vlen) || tag != 0x06) return 0;
    if (oid_is(val, vlen, OID_ECDSA_SHA256, sizeof OID_ECDSA_SHA256)) return 256;
    if (oid_is(val, vlen, OID_ECDSA_SHA384, sizeof OID_ECDSA_SHA384)) return 384;
    return 0;
}

/* An INTEGER as a fixed-width big-endian field. DER integers are signed, so a
 * value whose top bit is set carries a leading zero byte that must be dropped;
 * a short value must be left-padded. Getting either wrong shifts r or s by a
 * byte and every signature fails. */
static int int_to_fixed(const u8 *v, int n, u8 *out, int size)
{
    while (n > 0 && v[0] == 0) { v++; n--; }
    if (n > size) return 0;
    for (int i = 0; i < size - n; i++) out[i] = 0;
    xmemcpy(out + size - n, v, n);
    return 1;
}

int x509_parse(const u8 *der, int len, struct x509_cert *c)
{
    for (int i = 0; i < (int)sizeof *c; i++) ((u8 *)c)[i] = 0;
    c->der = der; c->derlen = len;

    struct der top, cert;
    int tag, vlen;
    const u8 *val;
    d_init(&top, der, len);
    if (!d_next(&top, &tag, &val, &vlen) || tag != 0x30) return fail("not a SEQUENCE");
    d_init(&cert, val, vlen);

    /* tbsCertificate - keep the WHOLE encoding, header included */
    const u8 *tbs_full; int tbs_flen;
    const u8 *tbs_val;  int tbs_vlen;
    if (!d_next_full(&cert, &tag, &tbs_full, &tbs_flen, &tbs_val, &tbs_vlen) || tag != 0x30)
        return fail("no tbsCertificate");
    c->tbs = tbs_full; c->tbslen = tbs_flen;

    /* signatureAlgorithm */
    if (!d_next(&cert, &tag, &val, &vlen) || tag != 0x30) return fail("no signatureAlgorithm");
    c->sig_hash = alg_hash(val, vlen);

    /* signatureValue: a BIT STRING wrapping SEQUENCE { r, s } */
    if (!d_next(&cert, &tag, &val, &vlen) || tag != 0x03 || vlen < 2)
        return fail("no signatureValue");
    if (val[0] != 0) return fail("signature has unused bits");
    /* A CERTIFICATE WHOSE OWN SIGNATURE WE CANNOT READ IS STILL PARSEABLE, and
     * refusing it here was wrong. A trust anchor is trusted because it is in
     * the store, so its self-signature is never checked - only its SUBJECT and
     * its KEY are ever used. ISRG Root X2 is exactly this case: a P-384 key,
     * cross-signed by X1 with RSA. Failing the parse made the real root
     * unusable and took the whole chain down with it.
     *
     * The refusal belongs in x509_signed_by, which checks sig_hash before
     * trusting anything - so a certificate we cannot verify can be an anchor
     * but can never be a LINK. */
    if (c->sig_hash) {
        struct der sig, rs;
        const u8 *sv; int sl;
        d_init(&sig, val + 1, vlen - 1);
        if (!d_next(&sig, &tag, &sv, &sl) || tag != 0x30) { c->sig_hash = 0; goto sig_done; }
        d_init(&rs, sv, sl);
        const u8 *rv, *svv; int rl, sl2;
        if (!d_next(&rs, &tag, &rv, &rl) || tag != 0x02) { c->sig_hash = 0; goto sig_done; }
        if (!d_next(&rs, &tag, &svv, &sl2) || tag != 0x02) { c->sig_hash = 0; goto sig_done; }
        /* the component size follows the SIGNER's curve, which we do not know
         * yet - size by the larger of the two and let the caller's curve fix
         * it. 48 covers P-384; P-256 signatures pad harmlessly. */
        int size = (rl > 33 || sl2 > 33) ? 48 : 32;
        c->sig_size = size;
        if (!int_to_fixed(rv, rl, c->sig_r, size)) return fail("r too large");
        if (!int_to_fixed(svv, sl2, c->sig_s, size)) return fail("s too large");
    }
sig_done:;

    /* ---- inside the TBS ---- */
    struct der tbs;
    d_init(&tbs, tbs_val, tbs_vlen);
    if (!d_next(&tbs, &tag, &val, &vlen)) return fail("empty tbs");
    if (tag == 0xA0) {                        /* [0] version, optional */
        if (!d_next(&tbs, &tag, &val, &vlen)) return fail("no serial");
    }
    /* val is now serialNumber */
    if (!d_next(&tbs, &tag, &val, &vlen) || tag != 0x30) return fail("no inner sigalg");

    /* issuer Name - keep the raw encoding, because chain matching compares
     * issuer to subject byte for byte */
    {
        const u8 *f; int fl;
        if (!d_next_full(&tbs, &tag, &f, &fl, &val, &vlen) || tag != 0x30)
            return fail("no issuer");
        c->issuer = f; c->issuerlen = fl;
    }
    /* validity SEQUENCE { notBefore, notAfter } */
    if (!d_next(&tbs, &tag, &val, &vlen) || tag != 0x30) return fail("no validity");
    {
        struct der v;
        const u8 *tv; int tl;
        d_init(&v, val, vlen);
        for (int k = 0; k < 2; k++) {
            if (!d_next(&v, &tag, &tv, &tl)) return fail("bad validity");
            char *dst = k ? c->not_after : c->not_before;
            /* UTCTime is YYMMDDHHMMSSZ and needs a century; GeneralizedTime is
             * already YYYYMMDDHHMMSSZ. RFC 5280: YY >= 50 means 19xx. */
            int o = 0;
            if (tag == 0x17 && tl >= 13) {
                int yy = (tv[0] - '0') * 10 + (tv[1] - '0');
                const char *cent = (yy >= 50) ? "19" : "20";
                dst[o++] = cent[0]; dst[o++] = cent[1];
                /* YYMMDDHHMMSS is TWELVE digits. Copying eleven produced a
                 * 13-character stamp, and since x509_time_ok compares the
                 * strings positionally that shifted every field left by one -
                 * a comparison that still "worked" against another equally
                 * broken stamp, which is why it took a printout to see. */
                for (int j = 0; j < 12 && o < X509_TIME_LEN - 1; j++) dst[o++] = (char)tv[j];
            } else if (tag == 0x18 && tl >= 15) {
                for (int j = 0; j < 14 && o < X509_TIME_LEN - 1; j++) dst[o++] = (char)tv[j];
            } else return fail("unsupported time format");
            dst[o] = 0;
        }
    }
    /* subject Name */
    {
        const u8 *f; int fl;
        if (!d_next_full(&tbs, &tag, &f, &fl, &val, &vlen) || tag != 0x30)
            return fail("no subject");
        c->subject = f; c->subjectlen = fl;
    }
    /* SubjectPublicKeyInfo ::= SEQUENCE { AlgorithmIdentifier, BIT STRING } */
    if (!d_next(&tbs, &tag, &val, &vlen) || tag != 0x30) return fail("no SPKI");
    {
        struct der spki;
        const u8 *av; int al;
        d_init(&spki, val, vlen);
        if (!d_next(&spki, &tag, &av, &al) || tag != 0x30) return fail("no key algorithm");
        {
            struct der a;
            const u8 *o1; int l1;
            d_init(&a, av, al);
            if (!d_next(&a, &tag, &o1, &l1) || tag != 0x06) return fail("no key OID");
            if (!oid_is(o1, l1, OID_EC_PUBKEY, sizeof OID_EC_PUBKEY))
                return fail("not an EC key (RSA is not supported)");
            if (!d_next(&a, &tag, &o1, &l1) || tag != 0x06) return fail("no curve OID");
            if (oid_is(o1, l1, OID_P256, sizeof OID_P256)) c->curve_bits = 256;
            else if (oid_is(o1, l1, OID_P384, sizeof OID_P384)) c->curve_bits = 384;
            else return fail("unsupported curve");
        }
        const u8 *kv; int kl;
        if (!d_next(&spki, &tag, &kv, &kl) || tag != 0x03 || kl < 2)
            return fail("no key bits");
        if (kv[0] != 0) return fail("key has unused bits");
        kv++; kl--;
        /* 0x04 = uncompressed. Compressed points are legal DER and we do not
         * decompress them, so they are refused rather than misread. */
        if (kl < 1 || kv[0] != 0x04) return fail("key point is not uncompressed");
        kv++; kl--;
        if (kl != c->curve_bits / 4) return fail("key point is the wrong size");
        c->pubkey = kv; c->pubkeylen = kl;
    }
    /* [3] extensions, optional */
    while (d_next(&tbs, &tag, &val, &vlen)) {
        if (tag != 0xA3) continue;
        struct der exts;
        const u8 *sv; int sl;
        d_init(&exts, val, vlen);
        if (!d_next(&exts, &tag, &sv, &sl) || tag != 0x30) break;
        struct der list;
        d_init(&list, sv, sl);
        const u8 *ev; int el;
        while (d_next(&list, &tag, &ev, &el)) {
            if (tag != 0x30) continue;
            struct der one;
            const u8 *ov; int ol;
            d_init(&one, ev, el);
            if (!d_next(&one, &tag, &ov, &ol) || tag != 0x06) continue;
            int is_san   = oid_is(ov, ol, OID_SAN, sizeof OID_SAN);
            int is_basic = oid_is(ov, ol, OID_BASIC, sizeof OID_BASIC);
            const u8 *nv; int nl;
            if (!d_next(&one, &tag, &nv, &nl)) continue;
            if (tag == 0x01) {                        /* critical BOOLEAN */
                if (!d_next(&one, &tag, &nv, &nl)) continue;
            }
            if (tag != 0x04) continue;                /* the OCTET STRING */
            if (is_san) { c->san = nv; c->sanlen = nl; }
            if (is_basic) {
                struct der bc;
                const u8 *bv; int bl;
                d_init(&bc, nv, nl);
                c->has_bc = 1;
                if (d_next(&bc, &tag, &bv, &bl) && tag == 0x30) {
                    struct der in;
                    d_init(&in, bv, bl);
                    const u8 *cv; int cl;
                    if (d_next(&in, &tag, &cv, &cl) && tag == 0x01 && cl >= 1)
                        c->is_ca = cv[0] != 0;
                }
            }
        }
    }
    return 1;
}

int x509_signed_by(const struct x509_cert *child, const struct x509_cert *issuer)
{
    if (!child->sig_hash) return fail("unsupported signature algorithm");
    if (!issuer->curve_bits || !issuer->pubkey) return fail("issuer has no usable key");
    /* the issuer's DN must be exactly the child's issuer field */
    if (child->issuerlen != issuer->subjectlen ||
        xmemcmp(child->issuer, issuer->subject, child->issuerlen))
        return fail("issuer name does not match");

    u8 h[48];
    int hl;
    if (child->sig_hash == 384) { sha384(child->tbs, (unsigned)child->tbslen, h); hl = 48; }
    else                        { sha256(child->tbs, (unsigned)child->tbslen, h); hl = 32; }

    /* The signature components are sized by the ISSUER's curve, and the stored
     * ones by whatever the certificate encoded.
     *
     * THE SUBTRACTION HERE WAS AN OUT-OF-BOUNDS READ. `child->sig_size - size`
     * is NEGATIVE whenever a certificate carries short r/s while naming a
     * larger issuer curve - a P-384 issuer with a signature whose integers
     * both fit in 33 bytes. That is attacker-controlled: the certificate bytes
     * come from whoever answered the connection. It read up to 16 bytes before
     * sig_r. Right-align instead, which is what a fixed-width big-endian field
     * means, and a value too large for the curve is refused. */
    int size = issuer->curve_bits / 8;
    if (size > 48 || child->sig_size > 48) return fail("signature size out of range");
    u8 r[48], s[48];
    for (int i = 0; i < 48; i++) { r[i] = 0; s[i] = 0; }
    int take = child->sig_size < size ? child->sig_size : size;
    for (int i = 0; i < take; i++) {
        r[size - take + i] = child->sig_r[child->sig_size - take + i];
        s[size - take + i] = child->sig_s[child->sig_size - take + i];
    }
    if (!ecdsa_verify(issuer->curve_bits, issuer->pubkey, r, s, h, hl))
        return fail("signature does not verify");
    return 1;
}

/* ---- the host name ---------------------------------------------------------
 * SubjectAltName ::= SEQUENCE OF GeneralName; dNSName is [2] IA5String.
 *
 * COMMON NAME IS NOT CONSULTED, deliberately. RFC 6125 deprecated it and every
 * browser stopped honouring it, because a CN that looks like a hostname in a
 * certificate with a SAN listing different names is precisely how mismatched
 * certificates got accepted for years.
 */
static int label_eq(const char *a, int an, const char *b, int bn)
{
    if (an != bn) return 0;
    for (int i = 0; i < an; i++) if (lower(a[i]) != lower(b[i])) return 0;
    return 1;
}

static int dns_matches(const char *pat, int pn, const char *host, int hn)
{
    /* a wildcard is only ever the WHOLE leftmost label: *.example.com matches
     * a.example.com and NOT a.b.example.com and NOT example.com. `w*.x.com`
     * and `*.com` are both refused. */
    if (pn > 2 && pat[0] == '*' && pat[1] == '.') {
        int dots = 0;
        for (int i = 2; i < pn; i++) if (pat[i] == '.') dots++;
        if (dots < 1) return 0;                   /* *.com is not a name */
        int hd = -1;
        for (int i = 0; i < hn; i++) if (host[i] == '.') { hd = i; break; }
        if (hd < 0) return 0;
        return label_eq(pat + 2, pn - 2, host + hd + 1, hn - hd - 1);
    }
    return label_eq(pat, pn, host, hn);
}

int x509_host_matches(const struct x509_cert *c, const char *host)
{
    if (!c->san || !host) return fail("certificate has no subjectAltName");
    int hn = 0;
    while (host[hn]) hn++;
    struct der d;
    int tag, vlen;
    const u8 *val;
    d_init(&d, c->san, c->sanlen);
    if (!d_next(&d, &tag, &val, &vlen) || tag != 0x30) return fail("malformed SAN");
    struct der list;
    d_init(&list, val, vlen);
    while (d_next(&list, &tag, &val, &vlen)) {
        if (tag != 0x82) continue;               /* [2] dNSName */
        if (dns_matches((const char *)val, vlen, host, hn)) return 1;
    }
    return fail("no subjectAltName matches the host");
}

int x509_time_ok(const struct x509_cert *c, const char *nowZ)
{
    if (!nowZ) return 1;
    /* the strings are fixed-width YYYYMMDDHHMMSS, so lexicographic order IS
     * chronological order and no date arithmetic is needed */
    for (int i = 0; i < 14; i++) {
        if (nowZ[i] < c->not_before[i]) return fail("certificate is not valid yet");
        if (nowZ[i] > c->not_before[i]) break;
    }
    for (int i = 0; i < 14; i++) {
        if (nowZ[i] > c->not_after[i]) return fail("certificate has expired");
        if (nowZ[i] < c->not_after[i]) break;
    }
    return 1;
}

int x509_chain_ok(const u8 *const *ders, const int *lens, int n,
                  const struct x509_cert *roots, int nroots,
                  const char *host, const char *nowZ)
{
    why = "";
    if (n <= 0 || n > X509_MAX_CHAIN) return fail("chain length out of range");

    struct x509_cert c[X509_MAX_CHAIN];
    for (int i = 0; i < n; i++)
        if (!x509_parse(ders[i], lens[i], &c[i])) return 0;

    /* the leaf must carry the name we asked for, and be current */
    if (!x509_host_matches(&c[0], host)) return 0;
    for (int i = 0; i < n; i++)
        if (!x509_time_ok(&c[i], nowZ)) return 0;

    /* every link, and every issuer must be allowed to be one */
    for (int i = 0; i + 1 < n; i++) {
        if (!c[i + 1].is_ca) return fail("an intermediate is not a CA");
        if (!x509_signed_by(&c[i], &c[i + 1])) return 0;
    }

    /* the top must be signed by something we already trust. The chain's own
     * last certificate is NOT trusted just because it is last - that is the
     * whole attack. */
    const struct x509_cert *top = &c[n - 1];
    for (int r = 0; r < nroots; r++) {
        if (!roots[r].is_ca && roots[r].has_bc) continue;
        /* THE CHAIN MAY INCLUDE THE ROOT ITSELF, and accepting that needs an
         * IDENTITY, which is the subject AND THE PUBLIC KEY - never the name
         * alone, and not the whole DER either.
         *
         * COMPARING SUBJECTS ALONE WAS A COMPLETE AUTHENTICATION BYPASS. It
         * was here until an adversarial pass built the certificate that
         * exploits it: a self-signed CA whose subject DN is byte-identical to
         * ISRG Root X2's - same PrintableString encoding and all - holding an
         * ATTACKER's key. Every link below verifies because the attacker
         * signed them, the top matched a trusted NAME, and the chain was
         * accepted. Any host, no warning.
         *
         * COMPARING THE WHOLE DER IS TOO STRICT, and that was the next
         * mistake: Wikimedia sends the CROSS-SIGNED X2 (issued by ISRG Root
         * X1) while the store holds the SELF-SIGNED one. Same subject, same
         * key, different bytes, same root. Byte equality refused the real web.
         *
         * Subject plus key is exactly right. An attacker can copy the name
         * freely; copying the KEY gains them nothing, because then every
         * signature below has to verify under a private key they do not
         * have. */
        if (top->subjectlen == roots[r].subjectlen &&
            top->pubkeylen == roots[r].pubkeylen && top->pubkeylen > 0 &&
            top->curve_bits == roots[r].curve_bits &&
            !xmemcmp(top->subject, roots[r].subject, top->subjectlen) &&
            !xmemcmp(top->pubkey, roots[r].pubkey, top->pubkeylen))
            return 1;
        if (x509_signed_by(top, &roots[r])) return 1;
    }
    return fail("chain does not reach a trusted root");
}
