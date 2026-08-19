/* rsa.c - RSA signature VERIFICATION, PKCS#1 v1.5.
 *
 * WHY THIS EXISTS, measured rather than assumed. ecdsa.c covers Let's
 * Encrypt, which is what Wikipedia uses. It does not cover Google:
 *
 *   www.google.com   ECDSA key, signed with sha256WithRSAEncryption
 *   GTS WR2          RSA-2048 key, signed with RSA
 *   GTS Root R1      RSA-4096 key, signed with RSA
 *
 * Every signature in that chain is RSA, so an ECDSA-only verifier cannot check
 * a single link of it - and "most of the web outside Let's Encrypt" is the
 * honest size of that gap.
 *
 * VERIFY ONLY, and here the asymmetry is even starker than for ECDSA: signing
 * needs the private exponent, verification needs only the public one. There is
 * no private key anywhere in this kernel and there is no reason for one.
 *
 * THE ARITHMETIC IS THE SAME SHIFT-AND-ADD AS ecdsa.c, and for RSA that is not
 * even slow. The public exponent is essentially always 65537 = 2^16 + 1, which
 * is sixteen squarings and one multiply - SEVENTEEN modular multiplications for
 * a whole verification, against roughly eight hundred for an ECDSA one. The
 * numbers are bigger; there are far fewer of them.
 *
 * EVERYTHING HERE IS PUBLIC. A signature, a modulus and an exponent are all
 * published values, so unlike ECDSA signing there is no secret for a timing
 * side channel to leak. Data-dependent branches below are deliberate.
 */

typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long long u64;

#define RLIMBS 128                 /* 128 x 32 = 4096 bits */

typedef u32 rbn[RLIMBS];

static void rset0(rbn a) { for (int i = 0; i < RLIMBS; i++) a[i] = 0; }
static void rmov(rbn d, const rbn s) { for (int i = 0; i < RLIMBS; i++) d[i] = s[i]; }

static int rcmp(const rbn a, const rbn b)
{
    for (int i = RLIMBS - 1; i >= 0; i--) {
        if (a[i] > b[i]) return 1;
        if (a[i] < b[i]) return -1;
    }
    return 0;
}

static u32 radd(rbn r, const rbn a, const rbn b)
{
    u64 c = 0;
    for (int i = 0; i < RLIMBS; i++) {
        u64 t = (u64)a[i] + b[i] + c;
        r[i] = (u32)t;
        c = t >> 32;
    }
    return (u32)c;
}

static u32 rsub(rbn r, const rbn a, const rbn b)
{
    u64 br = 0;
    for (int i = 0; i < RLIMBS; i++) {
        u64 t = (u64)a[i] - b[i] - br;
        r[i] = (u32)t;
        br = (t >> 63) & 1;
    }
    return (u32)br;
}

static void rshl1(rbn a)
{
    u32 c = 0;
    for (int i = 0; i < RLIMBS; i++) {
        u32 nc = a[i] >> 31;
        a[i] = (a[i] << 1) | c;
        c = nc;
    }
}

static int rbit(const rbn a, int i) { return (a[i >> 5] >> (i & 31)) & 1; }

/* how many bits the value actually occupies - the loop bound below, so a
 * 2048-bit modulus does not pay for a 4096-bit one */
static int rbits(const rbn a)
{
    for (int i = RLIMBS - 1; i >= 0; i--)
        if (a[i]) {
            int n = i * 32;
            u32 v = a[i];
            while (v) { n++; v >>= 1; }
            return n;
        }
    return 0;
}

static void rmadd(rbn r, const rbn a, const rbn b, const rbn m)
{
    u32 c = radd(r, a, b);
    if (c || rcmp(r, m) >= 0) rsub(r, r, m);
}

/* r = a*b mod m, shift-and-add over the bits of b */
static void rmulmod(rbn r, const rbn a, const rbn b, const rbn m, int mbits)
{
    rbn acc, add;
    rset0(acc);
    rmov(add, a);
    /* `< mbits`, NOT `< mbits + 1`. b is always reduced mod m, so its highest
     * possible bit is mbits-1 and the extra iteration reads nothing useful -
     * except at exactly 4096 bits, where rbit(b, 4096) indexes limb 128 of a
     * 128-limb array and reads off the end. RSA-2048 passed anyway (limb 64 is
     * in bounds and happened to be zero); GTS Root R1 is 4096 and failed. The
     * largest supported size is exactly where an off-by-one stops being
     * harmless, which is the argument for testing at the boundary rather than
     * at a comfortable value. */
    if (mbits > RLIMBS * 32) mbits = RLIMBS * 32;
    for (int i = 0; i < mbits; i++) {
        if (rbit(b, i)) rmadd(acc, acc, add, m);
        u32 top = add[RLIMBS - 1] >> 31;
        rshl1(add);
        if (top || rcmp(add, m) >= 0) rsub(add, add, m);
    }
    rmov(r, acc);
}

/* r = a^e mod m. The exponent is public (65537, almost always), so the branch
 * on its bits leaks nothing. */
static void rexpmod(rbn r, const rbn a, const rbn e, const rbn m)
{
    int mb = rbits(m);
    int eb = rbits(e);
    rbn base, acc;
    rset0(acc);
    acc[0] = 1;
    rmov(base, a);
    for (int i = 0; i < eb; i++) {
        if (rbit(e, i)) rmulmod(acc, acc, base, m, mb);
        rmulmod(base, base, base, m, mb);
    }
    rmov(r, acc);
}

static void rfrom_be(rbn r, const u8 *p, int n)
{
    rset0(r);
    if (n > RLIMBS * 4) return;
    for (int i = 0; i < n; i++) {
        int byte = n - 1 - i;
        r[i >> 2] |= (u32)p[byte] << ((i & 3) * 8);
    }
}

static void rto_be(const rbn a, u8 *out, int n)
{
    for (int i = 0; i < n; i++) {
        int byte = n - 1 - i;
        out[byte] = (u8)(a[i >> 2] >> ((i & 3) * 8));
    }
}

/* ---- the DigestInfo prefixes ----------------------------------------------
 * PKCS#1 v1.5 does not sign the bare hash; it signs a DER SEQUENCE naming the
 * algorithm and wrapping the digest. These are those encodings with the digest
 * removed, and they are constants - which is why this is a memcmp and not a
 * parser. Accepting a signature whose DigestInfo says SHA-1 while the caller
 * asked for SHA-256 is a downgrade, so the prefix must match exactly.
 */
static const u8 DI_SHA256[] = {
    0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,
    0x05,0x00,0x04,0x20
};
static const u8 DI_SHA384[] = {
    0x30,0x41,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,
    0x05,0x00,0x04,0x30
};
static const u8 DI_SHA512[] = {
    0x30,0x51,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,
    0x05,0x00,0x04,0x40
};

/* Verify sig over `hash`. n/nlen and e/elen are the modulus and exponent as
 * big-endian bytes, straight out of the certificate. Returns 1 or 0; every
 * failure returns 0 with no distinction, because "bad padding" and "wrong
 * signature" must look identical to a caller deciding whether to trust.
 */
int rsa_verify(const u8 *n_be, int nlen, const u8 *e_be, int elen,
               const u8 *sig, int siglen, const u8 *hash, int hashlen)
{
    if (nlen <= 0 || nlen > RLIMBS * 4) return 0;
    if (siglen != nlen) return 0;              /* must be exactly modulus-sized */

    const u8 *di;
    int dilen;
    if (hashlen == 32)      { di = DI_SHA256; dilen = (int)sizeof DI_SHA256; }
    else if (hashlen == 48) { di = DI_SHA384; dilen = (int)sizeof DI_SHA384; }
    else if (hashlen == 64) { di = DI_SHA512; dilen = (int)sizeof DI_SHA512; }
    else return 0;

    /* the padded block must have room for 0x00 0x01, at least eight 0xFF, a
     * 0x00, then the DigestInfo and the hash */
    if (nlen < dilen + hashlen + 11) return 0;

    rbn N, E, S, M;
    rfrom_be(N, n_be, nlen);
    rfrom_be(E, e_be, elen);
    rfrom_be(S, sig, siglen);
    if (rcmp(S, N) >= 0) return 0;             /* s must be reduced mod n */

    rexpmod(M, S, E, N);

    static u8 em[RLIMBS * 4];
    rto_be(M, em, nlen);

    /* EM = 0x00 || 0x01 || 0xFF... || 0x00 || DigestInfo || H
     *
     * CHECKED STRICTLY, and that matters: a lax parser that hunts for the 0x00
     * separator and trusts whatever follows is the Bleichenbacher '06 forgery,
     * which let anyone forge a signature under a small exponent. The padding
     * length is DERIVED here, not searched for. */
    if (em[0] != 0x00 || em[1] != 0x01) return 0;
    int i = 2;
    int pad_end = nlen - dilen - hashlen - 1;
    if (pad_end < i + 8) return 0;             /* at least eight 0xFF */
    for (; i < pad_end; i++) if (em[i] != 0xFF) return 0;
    if (em[i++] != 0x00) return 0;
    for (int k = 0; k < dilen; k++) if (em[i + k] != di[k]) return 0;
    i += dilen;
    for (int k = 0; k < hashlen; k++) if (em[i + k] != hash[k]) return 0;
    return 1;
}
