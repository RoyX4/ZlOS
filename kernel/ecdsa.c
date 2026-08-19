/* ecdsa.c - ECDSA signature VERIFICATION over P-256 and P-384.
 *
 * WHY THESE TWO CURVES AND NOT RSA. Measured, not assumed: en.wikipedia.org's
 * certificate chain is four certificates and every one of them is
 * ecdsa-with-SHA384, over P-256 at the leaf and P-384 at both intermediates
 * and the root. A verifier built for RSA - the algorithm everybody names first
 * - could not check a single link of it. Let's Encrypt's ECDSA chain is now
 * the common case, not the exotic one.
 *
 * VERIFY ONLY. There is no signing here and there should not be: this kernel
 * has no entropy source, and ECDSA signing with a predictable nonce hands over
 * the private key in one signature. Verification needs no randomness at all.
 *
 * THE ARITHMETIC IS DELIBERATELY THE SIMPLE KIND. Modular multiplication is
 * shift-and-add rather than Montgomery, and reduction is conditional
 * subtraction rather than Barrett. That is perhaps 30x slower and it is the
 * right trade here: this runs a handful of times per connection, not per
 * packet, and Montgomery form is where the subtle bugs live - a wrong
 * conversion produces a verifier that accepts some signatures and rejects
 * others, which looks like a network problem. Measured cost is in the gate.
 *
 * NO HEAP. Every value is a fixed 12-limb array (384 bits), so P-256 simply
 * uses the low 8 limbs and the same code serves both curves.
 */

typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long long u64;

#define LIMBS 12                      /* 12 x 32 = 384 bits */

typedef u32 bn[LIMBS];

struct curve {
    int limbs;                        /* 8 for P-256, 12 for P-384 */
    bn p, n, gx, gy, b;
};

/* ---- the two curves, from FIPS 186-4 ---------------------------------------
 * Stored little-endian by limb: [0] is the least significant 32 bits. Written
 * out rather than parsed from hex at startup so a typo is a compile-time
 * constant a test can catch, not a runtime surprise.
 */
static const struct curve P256 = {
    8,
    { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000,
      0x00000000, 0x00000000, 0x00000001, 0xFFFFFFFF, 0, 0, 0, 0 },
    { 0xFC632551, 0xF3B9CAC2, 0xA7179E84, 0xBCE6FAAD,
      0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0, 0, 0, 0 },
    { 0xD898C296, 0xF4A13945, 0x2DEB33A0, 0x77037D81,
      0x63A440F2, 0xF8BCE6E5, 0xE12C4247, 0x6B17D1F2, 0, 0, 0, 0 },
    { 0x37BF51F5, 0xCBB64068, 0x6B315ECE, 0x2BCE3357,
      0x7C0F9E16, 0x8EE7EB4A, 0xFE1A7F9B, 0x4FE342E2, 0, 0, 0, 0 },
    { 0x27D2604B, 0x3BCE3C3E, 0xCC53B0F6, 0x651D06B0,
      0x769886BC, 0xB3EBBD55, 0xAA3A93E7, 0x5AC635D8, 0, 0, 0, 0 }
};

static const struct curve P384 = {
    12,
    { 0xFFFFFFFF, 0x00000000, 0x00000000, 0xFFFFFFFF,
      0xFFFFFFFE, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0xCCC52973, 0xECEC196A, 0x48B0A77A, 0x581A0DB2,
      0xF4372DDF, 0xC7634D81, 0xFFFFFFFF, 0xFFFFFFFF,
      0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF },
    { 0x72760AB7, 0x3A545E38, 0xBF55296C, 0x5502F25D,
      0x82542A38, 0x59F741E0, 0x8BA79B98, 0x6E1D3B62,
      0xF320AD74, 0x8EB1C71E, 0xBE8B0537, 0xAA87CA22 },
    { 0x90EA0E5F, 0x7A431D7C, 0x1D7E819D, 0x0A60B1CE,
      0xB5F0B8C0, 0xE9DA3113, 0x289A147C, 0xF8F41DBD,
      0x9292DC29, 0x5D9E98BF, 0x96262C6F, 0x3617DE4A },
    { 0xD3EC2AEF, 0x2A85C8ED, 0x8A2ED19D, 0xC656398D,
      0x5013875A, 0x0314088F, 0xFE814112, 0x181D9C6E,
      0xE3F82D19, 0x988E056B, 0xE23EE7E4, 0xB3312FA7 }
};

/* ---- plain bignum ---------------------------------------------------------- */
static void bset0(bn a) { for (int i = 0; i < LIMBS; i++) a[i] = 0; }
static void bnmov(bn d, const bn s) { for (int i = 0; i < LIMBS; i++) d[i] = s[i]; }
static int  bniszero(const bn a) { for (int i = 0; i < LIMBS; i++) if (a[i]) return 0; return 1; }

static int bncmp(const bn a, const bn b)
{
    for (int i = LIMBS - 1; i >= 0; i--) {
        if (a[i] > b[i]) return 1;
        if (a[i] < b[i]) return -1;
    }
    return 0;
}

static u32 bnadd(bn r, const bn a, const bn b)
{
    u64 c = 0;
    for (int i = 0; i < LIMBS; i++) {
        u64 t = (u64)a[i] + b[i] + c;
        r[i] = (u32)t;
        c = t >> 32;
    }
    return (u32)c;
}

static u32 bnsub(bn r, const bn a, const bn b)
{
    u64 br = 0;
    for (int i = 0; i < LIMBS; i++) {
        u64 t = (u64)a[i] - b[i] - br;
        r[i] = (u32)t;
        br = (t >> 63) & 1;
    }
    return (u32)br;
}

static void bshl1(bn a)
{
    u32 c = 0;
    for (int i = 0; i < LIMBS; i++) {
        u32 nc = a[i] >> 31;
        a[i] = (a[i] << 1) | c;
        c = nc;
    }
}

static int bbit(const bn a, int i) { return (a[i >> 5] >> (i & 31)) & 1; }

/* ---- modular ---------------------------------------------------------------
 * add/sub are one conditional fix-up. mul is shift-and-add: 384 doublings with
 * a conditional add, each reduced by comparison. Simple and constant-shaped.
 */
static void madd(bn r, const bn a, const bn b, const bn m)
{
    u32 c = bnadd(r, a, b);
    if (c || bncmp(r, m) >= 0) bnsub(r, r, m);
}

static void msub(bn r, const bn a, const bn b, const bn m)
{
    if (bnsub(r, a, b)) bnadd(r, r, m);
}

static void mmul(bn r, const bn a, const bn b, const bn m)
{
    bn acc, addend;
    bset0(acc);
    bnmov(addend, a);
    for (int i = 0; i < LIMBS * 32; i++) {
        if (bbit(b, i)) madd(acc, acc, addend, m);
        /* addend = 2*addend mod m */
        u32 top = addend[LIMBS - 1] >> 31;
        bshl1(addend);
        if (top || bncmp(addend, m) >= 0) bnsub(addend, addend, m);
    }
    bnmov(r, acc);
}

/* r = a^e mod m, square-and-multiply. Used only for inversion via Fermat,
 * where the exponent is a public curve constant - so a data-dependent branch
 * here leaks nothing. */
static void mexp(bn r, const bn a, const bn e, const bn m)
{
    bn base, acc;
    bset0(acc);
    acc[0] = 1;
    bnmov(base, a);
    for (int i = 0; i < LIMBS * 32; i++) {
        if (bbit(e, i)) mmul(acc, acc, base, m);
        mmul(base, base, base, m);
    }
    bnmov(r, acc);
}

/* a^-1 mod m for prime m, by Fermat: a^(m-2) */
static void minv(bn r, const bn a, const bn m)
{
    bn e, two;
    bset0(two);
    two[0] = 2;
    bnsub(e, m, two);
    mexp(r, a, e, m);
}

/* ---- points, in Jacobian coordinates ---------------------------------------
 * Affine inversion costs a full modular exponentiation, so doing one per point
 * addition would make a scalar multiplication hundreds of them. Jacobian
 * coordinates defer that to a single inversion at the end.
 */
struct pt { bn x, y, z; };

static void p_zero(struct pt *p) { bset0(p->x); bset0(p->y); bset0(p->z); }
static int  p_inf(const struct pt *p) { return bniszero(p->z); }

static void p_dbl(struct pt *r, const struct pt *a, const bn m)
{
    if (p_inf(a)) { p_zero(r); return; }
    bn s, mm, t, y2, x2, z2;
    mmul(y2, a->y, a->y, m);            /* y^2         */
    mmul(s, a->x, y2, m);
    madd(s, s, s, m);
    madd(s, s, s, m);                    /* S = 4xy^2   */
    mmul(z2, a->z, a->z, m);
    mmul(t, z2, z2, m);                  /* z^4         */
    mmul(x2, a->x, a->x, m);
    madd(mm, x2, x2, m);
    madd(mm, mm, x2, m);                 /* 3x^2        */
    {   /* a = -3 for both curves, so M = 3x^2 - 3z^4 */
        bn t3;
        madd(t3, t, t, m);
        madd(t3, t3, t, m);              /* 3z^4       */
        msub(mm, mm, t3, m);
    }
    mmul(r->x, mm, mm, m);
    {
        bn s2;
        madd(s2, s, s, m);
        msub(r->x, r->x, s2, m);         /* X' = M^2 - 2S */
    }
    mmul(r->z, a->y, a->z, m);
    madd(r->z, r->z, r->z, m);           /* Z' = 2yz     */
    {
        bn y4, t8;
        mmul(y4, y2, y2, m);
        madd(t8, y4, y4, m);
        madd(t8, t8, t8, m);
        madd(t8, t8, t8, m);             /* 8y^4        */
        bn d;
        msub(d, s, r->x, m);
        mmul(r->y, mm, d, m);
        msub(r->y, r->y, t8, m);         /* Y' = M(S-X') - 8y^4 */
    }
}

static void p_add(struct pt *r, const struct pt *a, const struct pt *b, const bn m)
{
    if (p_inf(a)) { bnmov(r->x, b->x); bnmov(r->y, b->y); bnmov(r->z, b->z); return; }
    if (p_inf(b)) { bnmov(r->x, a->x); bnmov(r->y, a->y); bnmov(r->z, a->z); return; }
    bn z1z1, z2z2, u1, u2, s1, s2, h, i, j, rr, v;
    mmul(z1z1, a->z, a->z, m);
    mmul(z2z2, b->z, b->z, m);
    mmul(u1, a->x, z2z2, m);
    mmul(u2, b->x, z1z1, m);
    mmul(s1, a->y, b->z, m); mmul(s1, s1, z2z2, m);
    mmul(s2, b->y, a->z, m); mmul(s2, s2, z1z1, m);
    if (bncmp(u1, u2) == 0) {
        if (bncmp(s1, s2) != 0) { p_zero(r); return; }   /* P + (-P) */
        p_dbl(r, a, m);
        return;
    }
    msub(h, u2, u1, m);
    madd(i, h, h, m); mmul(i, i, i, m);
    mmul(j, h, i, m);
    msub(rr, s2, s1, m);
    madd(rr, rr, rr, m);
    mmul(v, u1, i, m);
    mmul(r->x, rr, rr, m);
    msub(r->x, r->x, j, m);
    { bn v2; madd(v2, v, v, m); msub(r->x, r->x, v2, m); }
    { bn d; msub(d, v, r->x, m); mmul(r->y, rr, d, m); }
    { bn t; mmul(t, s1, j, m); madd(t, t, t, m); msub(r->y, r->y, t, m); }
    /* Z3 = 2*Z1*Z2*H, and the 2 is NOT optional.
     *
     * THE BUG THIS COMMENT EXISTS FOR. X3 and Y3 above are the add-2007-bl
     * formulas, which use I = (2H)^2 and r = 2(S2-S1) - so they compute a
     * point scaled by lambda = 2 relative to the older cmo formulas. Jacobian
     * coordinates are projective: (X, Y, Z) and (l^2 X, l^3 Y, l Z) are the
     * SAME point, but only if Z carries the same lambda. Pairing 2007-bl's X
     * and Y with cmo's Z3 = Z1*Z2*H gives a point that is wrong by exactly a
     * factor of two in the scaling - and the symptom is that every valid
     * signature is rejected while every invalid one is also rejected, so the
     * whole rejection half of the gate passes vacuously. */
    mmul(r->z, a->z, b->z, m);
    mmul(r->z, r->z, h, m);
    madd(r->z, r->z, r->z, m);
}

/* r = k*P, plain double-and-add. Constant time is NOT required here: every
 * scalar in a VERIFICATION is public (it comes from the signature and the
 * message hash), so there is no secret for timing to leak. */
static void p_mul(struct pt *r, const bn k, const struct pt *pin, const bn m, int limbs)
{
    struct pt acc, base;
    p_zero(&acc);
    bnmov(base.x, pin->x); bnmov(base.y, pin->y); bnmov(base.z, pin->z);
    for (int i = 0; i < limbs * 32; i++) {
        if (bbit(k, i)) p_add(&acc, &acc, &base, m);
        p_dbl(&base, &base, m);
    }
    bnmov(r->x, acc.x); bnmov(r->y, acc.y); bnmov(r->z, acc.z);
}

/* ---- decoding -------------------------------------------------------------- */
static void bn_from_be(bn r, const u8 *p, int n)
{
    bset0(r);
    for (int i = 0; i < n; i++) {
        int byte = n - 1 - i;
        r[i >> 2] |= (u32)p[byte] << ((i & 3) * 8);
    }
}

/* ---- the public entry ------------------------------------------------------
 * `curve` is 256 or 384. `pub` is the uncompressed point WITHOUT the 0x04
 * prefix (x||y, each `size` bytes). r and s are `size` bytes each. `hash` is
 * the message digest, truncated to the group order's bit length as FIPS 186-4
 * requires when the digest is longer than the order.
 *
 * Returns 1 if the signature verifies, 0 if it does not. Every failure path
 * returns 0 - there is no error code, because "the signature is bad" and "the
 * point was malformed" must be indistinguishable to a caller that is deciding
 * whether to trust a certificate.
 */
int ecdsa_verify(int curve_bits, const u8 *pub, const u8 *r_in, const u8 *s_in,
                 const u8 *hash, int hashlen)
{
    const struct curve *C = (curve_bits == 256) ? &P256 : &P384;
    int size = C->limbs * 4;

    bn r, s, e, w, u1, u2;
    bn_from_be(r, r_in, size);
    bn_from_be(s, s_in, size);

    /* r and s must be in [1, n-1]. A zero r or s is the classic forgery. */
    if (bniszero(r) || bniszero(s)) return 0;
    if (bncmp(r, C->n) >= 0 || bncmp(s, C->n) >= 0) return 0;

    /* the digest, left-truncated to the order's length */
    int use = hashlen > size ? size : hashlen;
    bn_from_be(e, hash, use);
    if (hashlen > size) {
        /* FIPS 186-4: take the LEFTMOST bits, which for byte-aligned orders is
         * simply the first `size` bytes - already done by using hash[0..size). */
    }
    if (bncmp(e, C->n) >= 0) bnsub(e, e, C->n);

    minv(w, s, C->n);
    mmul(u1, e, w, C->n);
    mmul(u2, r, w, C->n);

    struct pt G, Q, A, B, R;
    p_zero(&G); p_zero(&Q);
    bnmov(G.x, C->gx); bnmov(G.y, C->gy); bset0(G.z); G.z[0] = 1;
    bn_from_be(Q.x, pub, size);
    bn_from_be(Q.y, pub + size, size);
    bset0(Q.z); Q.z[0] = 1;

    /* the public key must actually be on the curve: y^2 == x^3 - 3x + b.
     * Skipping this accepts a point on a weaker curve, which is a real attack
     * and not a theoretical one. */
    {
        bn y2, x3, t;
        mmul(y2, Q.y, Q.y, C->p);
        mmul(x3, Q.x, Q.x, C->p);
        mmul(x3, x3, Q.x, C->p);
        madd(t, Q.x, Q.x, C->p);
        madd(t, t, Q.x, C->p);          /* 3x */
        msub(x3, x3, t, C->p);
        madd(x3, x3, C->b, C->p);
        if (bncmp(y2, x3) != 0) return 0;
    }

    p_mul(&A, u1, &G, C->p, C->limbs);
    p_mul(&B, u2, &Q, C->p, C->limbs);
    p_add(&R, &A, &B, C->p);
    if (p_inf(&R)) return 0;

    /* back to affine: x = X/Z^2 */
    bn zi, zi2, x;
    minv(zi, R.z, C->p);
    mmul(zi2, zi, zi, C->p);
    mmul(x, R.x, zi2, C->p);
    if (bncmp(x, C->n) >= 0) bnsub(x, x, C->n);
    return bncmp(x, r) == 0;
}
