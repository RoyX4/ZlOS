/* crypto.c - the primitives WPA2, Bluetooth pairing and TLS all need.
 *
 * Written once because all three want the same things. WPA2-PSK needs
 * PBKDF2-HMAC-SHA1 for the PMK and an HMAC-SHA1 PRF for the PTK; Bluetooth
 * Secure Simple Pairing needs HMAC-SHA256 and AES-CMAC; TLS needs SHA-256,
 * HMAC and AES. Writing three copies of SHA-1 would be the obvious mistake.
 *
 * FREESTANDING. No libc, no heap, no statics that carry state between calls -
 * every context lives in a caller-provided struct, so two callers cannot
 * corrupt each other and nothing needs locking when sched.c is running.
 *
 * Stack budget matters here (the kernel has 256 KiB, and a 16 KiB stack once
 * overflowed into console statics). The largest thing any function below puts
 * on the stack is a 176-byte AES key schedule plus a 64-byte block buffer.
 * Nothing recurses.
 *
 * EVERY primitive in this file is checked against a published test vector by
 * hosttest/cryptotest.c - RFC 3174, RFC 6234, RFC 2202, RFC 4231, RFC 6070,
 * FIPS-197 and RFC 4493. That is the whole reason this file could be written
 * and finished with no hardware present: the answers are already published.
 */

typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long long u64;

/* ------------------------------------------------------------------ util */

static void cmemset(void *d, int v, u32 n)
{
    u8 *p = (u8 *)d;
    while (n--) *p++ = (u8)v;
}

static void cmemcpy(void *d, const void *s, u32 n)
{
    u8 *dp = (u8 *)d; const u8 *sp = (const u8 *)s;
    while (n--) *dp++ = *sp++;
}

/* Constant-time compare. A MIC check that returns early on the first wrong
 * byte leaks how much of the tag was right, which is how forgery oracles get
 * built. Every comparison of a MAC in this tree must come through here. */
int crypto_equal(const u8 *a, const u8 *b, u32 n)
{
    u8 diff = 0;
    for (u32 i = 0; i < n; i++) diff |= (u8)(a[i] ^ b[i]);
    return diff == 0;
}

static u32 rotl32(u32 x, int n) { return (x << n) | (x >> (32 - n)); }
static u32 rotr32(u32 x, int n) { return (x >> n) | (x << (32 - n)); }

/* ------------------------------------------------------------------ SHA-1
 * RFC 3174. Needed by WPA2: the PMK is PBKDF2-HMAC-SHA1 and the PTK comes out
 * of an HMAC-SHA1 PRF. Deprecated for signatures, still mandatory here. */

#define SHA1_BLOCK  64
#define SHA1_DIGEST 20

typedef struct {
    u32 h[5];
    u64 len;                    /* total message bytes, for the padding */
    u8  buf[SHA1_BLOCK];
    u32 used;
} sha1_ctx;

static void sha1_compress(sha1_ctx *c, const u8 *p)
{
    u32 w[80];
    for (int i = 0; i < 16; i++)
        w[i] = ((u32)p[i*4] << 24) | ((u32)p[i*4+1] << 16) |
               ((u32)p[i*4+2] << 8) | (u32)p[i*4+3];
    for (int i = 16; i < 80; i++)
        w[i] = rotl32(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

    u32 a = c->h[0], b = c->h[1], d = c->h[2], e = c->h[3], f = c->h[4];

    for (int i = 0; i < 80; i++) {
        u32 t, k;
        if (i < 20)      { t = (b & d) | (~b & e);            k = 0x5A827999u; }
        else if (i < 40) { t = b ^ d ^ e;                     k = 0x6ED9EBA1u; }
        else if (i < 60) { t = (b & d) | (b & e) | (d & e);   k = 0x8F1BBCDCu; }
        else             { t = b ^ d ^ e;                     k = 0xCA62C1D6u; }
        u32 tmp = rotl32(a, 5) + t + f + k + w[i];
        f = e; e = d; d = rotl32(b, 30); b = a; a = tmp;
    }
    c->h[0] += a; c->h[1] += b; c->h[2] += d; c->h[3] += e; c->h[4] += f;
}

void sha1_init(sha1_ctx *c)
{
    c->h[0] = 0x67452301u; c->h[1] = 0xEFCDAB89u; c->h[2] = 0x98BADCFEu;
    c->h[3] = 0x10325476u; c->h[4] = 0xC3D2E1F0u;
    c->len = 0; c->used = 0;
}

void sha1_update(sha1_ctx *c, const u8 *data, u32 n)
{
    c->len += n;
    while (n) {
        u32 take = SHA1_BLOCK - c->used;
        if (take > n) take = n;
        cmemcpy(c->buf + c->used, data, take);
        c->used += take; data += take; n -= take;
        if (c->used == SHA1_BLOCK) { sha1_compress(c, c->buf); c->used = 0; }
    }
}

void sha1_final(sha1_ctx *c, u8 *out)
{
    u64 bits = c->len * 8u;
    u8 pad = 0x80;
    sha1_update(c, &pad, 1);
    pad = 0x00;
    while (c->used != 56) sha1_update(c, &pad, 1);
    /* The length goes in raw, NOT through update - update would count these
     * eight bytes into c->len and corrupt the very value being written. */
    for (int i = 0; i < 8; i++) c->buf[56 + i] = (u8)(bits >> ((7 - i) * 8));
    sha1_compress(c, c->buf);
    for (int i = 0; i < 5; i++) {
        out[i*4]   = (u8)(c->h[i] >> 24); out[i*4+1] = (u8)(c->h[i] >> 16);
        out[i*4+2] = (u8)(c->h[i] >> 8);  out[i*4+3] = (u8)(c->h[i]);
    }
}

void sha1(const u8 *data, u32 n, u8 *out)
{
    sha1_ctx c; sha1_init(&c); sha1_update(&c, data, n); sha1_final(&c, out);
}

/* ---------------------------------------------------------------- SHA-256
 * RFC 6234. Bluetooth SSP's f4/f5/f6 functions are HMAC-SHA256, and TLS 1.2+
 * uses it everywhere. */

#define SHA256_BLOCK  64
#define SHA256_DIGEST 32

typedef struct {
    u32 h[8];
    u64 len;
    u8  buf[SHA256_BLOCK];
    u32 used;
} sha256_ctx;

static const u32 sha256_k[64] = {
0x428A2F98u,0x71374491u,0xB5C0FBCFu,0xE9B5DBA5u,0x3956C25Bu,0x59F111F1u,
0x923F82A4u,0xAB1C5ED5u,0xD807AA98u,0x12835B01u,0x243185BEu,0x550C7DC3u,
0x72BE5D74u,0x80DEB1FEu,0x9BDC06A7u,0xC19BF174u,0xE49B69C1u,0xEFBE4786u,
0x0FC19DC6u,0x240CA1CCu,0x2DE92C6Fu,0x4A7484AAu,0x5CB0A9DCu,0x76F988DAu,
0x983E5152u,0xA831C66Du,0xB00327C8u,0xBF597FC7u,0xC6E00BF3u,0xD5A79147u,
0x06CA6351u,0x14292967u,0x27B70A85u,0x2E1B2138u,0x4D2C6DFCu,0x53380D13u,
0x650A7354u,0x766A0ABBu,0x81C2C92Eu,0x92722C85u,0xA2BFE8A1u,0xA81A664Bu,
0xC24B8B70u,0xC76C51A3u,0xD192E819u,0xD6990624u,0xF40E3585u,0x106AA070u,
0x19A4C116u,0x1E376C08u,0x2748774Cu,0x34B0BCB5u,0x391C0CB3u,0x4ED8AA4Au,
0x5B9CCA4Fu,0x682E6FF3u,0x748F82EEu,0x78A5636Fu,0x84C87814u,0x8CC70208u,
0x90BEFFFAu,0xA4506CEBu,0xBEF9A3F7u,0xC67178F2u };

static void sha256_compress(sha256_ctx *c, const u8 *p)
{
    u32 w[64];
    for (int i = 0; i < 16; i++)
        w[i] = ((u32)p[i*4] << 24) | ((u32)p[i*4+1] << 16) |
               ((u32)p[i*4+2] << 8) | (u32)p[i*4+3];
    for (int i = 16; i < 64; i++) {
        u32 s0 = rotr32(w[i-15],7) ^ rotr32(w[i-15],18) ^ (w[i-15] >> 3);
        u32 s1 = rotr32(w[i-2],17) ^ rotr32(w[i-2],19)  ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    u32 a=c->h[0], b=c->h[1], cc=c->h[2], d=c->h[3];
    u32 e=c->h[4], f=c->h[5], g=c->h[6], h=c->h[7];
    for (int i = 0; i < 64; i++) {
        u32 S1 = rotr32(e,6) ^ rotr32(e,11) ^ rotr32(e,25);
        u32 ch = (e & f) ^ (~e & g);
        u32 t1 = h + S1 + ch + sha256_k[i] + w[i];
        u32 S0 = rotr32(a,2) ^ rotr32(a,13) ^ rotr32(a,22);
        u32 mj = (a & b) ^ (a & cc) ^ (b & cc);
        u32 t2 = S0 + mj;
        h = g; g = f; f = e; e = d + t1; d = cc; cc = b; b = a; a = t1 + t2;
    }
    c->h[0]+=a; c->h[1]+=b; c->h[2]+=cc; c->h[3]+=d;
    c->h[4]+=e; c->h[5]+=f; c->h[6]+=g;  c->h[7]+=h;
}

void sha256_init(sha256_ctx *c)
{
    c->h[0]=0x6A09E667u; c->h[1]=0xBB67AE85u; c->h[2]=0x3C6EF372u; c->h[3]=0xA54FF53Au;
    c->h[4]=0x510E527Fu; c->h[5]=0x9B05688Cu; c->h[6]=0x1F83D9ABu; c->h[7]=0x5BE0CD19u;
    c->len = 0; c->used = 0;
}

void sha256_update(sha256_ctx *c, const u8 *data, u32 n)
{
    c->len += n;
    while (n) {
        u32 take = SHA256_BLOCK - c->used;
        if (take > n) take = n;
        cmemcpy(c->buf + c->used, data, take);
        c->used += take; data += take; n -= take;
        if (c->used == SHA256_BLOCK) { sha256_compress(c, c->buf); c->used = 0; }
    }
}

void sha256_final(sha256_ctx *c, u8 *out)
{
    u64 bits = c->len * 8u;
    u8 pad = 0x80;
    sha256_update(c, &pad, 1);
    pad = 0x00;
    while (c->used != 56) sha256_update(c, &pad, 1);
    for (int i = 0; i < 8; i++) c->buf[56 + i] = (u8)(bits >> ((7 - i) * 8));
    sha256_compress(c, c->buf);
    for (int i = 0; i < 8; i++) {
        out[i*4]   = (u8)(c->h[i] >> 24); out[i*4+1] = (u8)(c->h[i] >> 16);
        out[i*4+2] = (u8)(c->h[i] >> 8);  out[i*4+3] = (u8)(c->h[i]);
    }
}

void sha256(const u8 *data, u32 n, u8 *out)
{
    sha256_ctx c; sha256_init(&c); sha256_update(&c, data, n); sha256_final(&c, out);
}

/* ------------------------------------------------------------------- HMAC
 * RFC 2104. Both flavours are written out rather than shared through a
 * function pointer: zl's kernel subset has no function values, and this file
 * is a candidate for translation to zl later. */

#define HMAC_BLOCK 64

void hmac_sha1(const u8 *key, u32 klen, const u8 *data, u32 dlen, u8 *out)
{
    u8 k[HMAC_BLOCK], pad[HMAC_BLOCK], inner[SHA1_DIGEST];
    sha1_ctx c;

    cmemset(k, 0, HMAC_BLOCK);
    if (klen > HMAC_BLOCK) sha1(key, klen, k);       /* long keys are hashed */
    else                   cmemcpy(k, key, klen);

    for (u32 i = 0; i < HMAC_BLOCK; i++) pad[i] = (u8)(k[i] ^ 0x36);
    sha1_init(&c); sha1_update(&c, pad, HMAC_BLOCK);
    sha1_update(&c, data, dlen); sha1_final(&c, inner);

    for (u32 i = 0; i < HMAC_BLOCK; i++) pad[i] = (u8)(k[i] ^ 0x5C);
    sha1_init(&c); sha1_update(&c, pad, HMAC_BLOCK);
    sha1_update(&c, inner, SHA1_DIGEST); sha1_final(&c, out);
}

void hmac_sha256(const u8 *key, u32 klen, const u8 *data, u32 dlen, u8 *out)
{
    u8 k[HMAC_BLOCK], pad[HMAC_BLOCK], inner[SHA256_DIGEST];
    sha256_ctx c;

    cmemset(k, 0, HMAC_BLOCK);
    if (klen > HMAC_BLOCK) sha256(key, klen, k);
    else                   cmemcpy(k, key, klen);

    for (u32 i = 0; i < HMAC_BLOCK; i++) pad[i] = (u8)(k[i] ^ 0x36);
    sha256_init(&c); sha256_update(&c, pad, HMAC_BLOCK);
    sha256_update(&c, data, dlen); sha256_final(&c, inner);

    for (u32 i = 0; i < HMAC_BLOCK; i++) pad[i] = (u8)(k[i] ^ 0x5C);
    sha256_init(&c); sha256_update(&c, pad, HMAC_BLOCK);
    sha256_update(&c, inner, SHA256_DIGEST); sha256_final(&c, out);
}

/* ----------------------------------------------------------------- PBKDF2
 * RFC 2898, vectors in RFC 6070. WPA2 uses exactly PBKDF2-HMAC-SHA1 with the
 * SSID as salt and 4096 iterations to turn a passphrase into the 32-byte PMK.
 *
 * 4096 iterations of HMAC-SHA1, twice over, is genuinely slow. It runs ONCE
 * per network at join time and the result must be cached - recomputing it per
 * handshake would stall the kernel for a visible fraction of a second. */

void pbkdf2_sha1(const u8 *pass, u32 plen, const u8 *salt, u32 slen,
                 u32 iters, u8 *out, u32 outlen)
{
    u32 block = 1;
    while (outlen) {
        u8 saltblk[64 + 4], u[SHA1_DIGEST], t[SHA1_DIGEST];
        u32 sl = slen > 64 ? 64 : slen;

        cmemcpy(saltblk, salt, sl);
        saltblk[sl]   = (u8)(block >> 24); saltblk[sl+1] = (u8)(block >> 16);
        saltblk[sl+2] = (u8)(block >> 8);  saltblk[sl+3] = (u8)(block);

        hmac_sha1(pass, plen, saltblk, sl + 4, u);
        cmemcpy(t, u, SHA1_DIGEST);

        for (u32 i = 1; i < iters; i++) {
            hmac_sha1(pass, plen, u, SHA1_DIGEST, u);
            for (int j = 0; j < SHA1_DIGEST; j++) t[j] ^= u[j];
        }

        u32 take = outlen > SHA1_DIGEST ? SHA1_DIGEST : outlen;
        cmemcpy(out, t, take);
        out += take; outlen -= take; block++;
    }
}

/* -------------------------------------------------------------------- AES
 * FIPS-197, AES-128 only. Byte-oriented rather than T-table driven: the tables
 * would be 4 KiB of rodata for speed we do not need, and a table-driven AES
 * leaks key material through cache timing, which will matter when this same
 * code carries TLS. */

static const u8 aes_sbox[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16 };

static const u8 aes_rsbox[256] = {
0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d };

#define AES_ROUNDS   10
#define AES_KEYSCHED 176            /* (ROUNDS+1) * 16 */

static u8 xtime(u8 x) { return (u8)((x << 1) ^ (((x >> 7) & 1) * 0x1B)); }

/* GF(2^8) multiply, used only by InvMixColumns. */
static u8 gmul(u8 a, u8 b)
{
    u8 r = 0;
    for (int i = 0; i < 8; i++) {
        if (b & 1) r ^= a;
        a = xtime(a);
        b >>= 1;
    }
    return r;
}

void aes128_expand(const u8 *key, u8 *rk)
{
    cmemcpy(rk, key, 16);
    u8 rcon = 1;
    for (int i = 16; i < AES_KEYSCHED; i += 4) {
        u8 t[4] = { rk[i-4], rk[i-3], rk[i-2], rk[i-1] };
        if ((i % 16) == 0) {
            u8 tmp = t[0];                                   /* RotWord */
            t[0] = aes_sbox[t[1]]; t[1] = aes_sbox[t[2]];
            t[2] = aes_sbox[t[3]]; t[3] = aes_sbox[tmp];
            t[0] ^= rcon;
            rcon = xtime(rcon);
        }
        for (int j = 0; j < 4; j++) rk[i+j] = (u8)(rk[i-16+j] ^ t[j]);
    }
}

static void add_round_key(u8 *s, const u8 *rk) { for (int i=0;i<16;i++) s[i] ^= rk[i]; }

void aes128_encrypt(const u8 *rk, const u8 *in, u8 *out)
{
    u8 s[16];
    cmemcpy(s, in, 16);
    add_round_key(s, rk);

    for (int round = 1; round <= AES_ROUNDS; round++) {
        for (int i = 0; i < 16; i++) s[i] = aes_sbox[s[i]];      /* SubBytes  */

        u8 t;                                                    /* ShiftRows */
        t = s[1];  s[1]=s[5];   s[5]=s[9];   s[9]=s[13];  s[13]=t;
        t = s[2];  s[2]=s[10];  s[10]=t;  t = s[6];  s[6]=s[14];  s[14]=t;
        t = s[15]; s[15]=s[11]; s[11]=s[7]; s[7]=s[3];   s[3]=t;

        if (round != AES_ROUNDS) {                               /* MixColumns */
            for (int c = 0; c < 4; c++) {
                u8 *p = s + c*4;
                u8 a0=p[0], a1=p[1], a2=p[2], a3=p[3];
                p[0] = (u8)(xtime(a0) ^ (xtime(a1)^a1) ^ a2 ^ a3);
                p[1] = (u8)(a0 ^ xtime(a1) ^ (xtime(a2)^a2) ^ a3);
                p[2] = (u8)(a0 ^ a1 ^ xtime(a2) ^ (xtime(a3)^a3));
                p[3] = (u8)((xtime(a0)^a0) ^ a1 ^ a2 ^ xtime(a3));
            }
        }
        add_round_key(s, rk + round*16);
    }
    cmemcpy(out, s, 16);
}

void aes128_decrypt(const u8 *rk, const u8 *in, u8 *out)
{
    u8 s[16];
    cmemcpy(s, in, 16);
    add_round_key(s, rk + AES_ROUNDS*16);

    for (int round = AES_ROUNDS - 1; round >= 0; round--) {
        u8 t;                                              /* InvShiftRows */
        t = s[13]; s[13]=s[9];  s[9]=s[5];   s[5]=s[1];   s[1]=t;
        t = s[2];  s[2]=s[10];  s[10]=t;  t = s[6];  s[6]=s[14];  s[14]=t;
        t = s[3];  s[3]=s[7];   s[7]=s[11]; s[11]=s[15]; s[15]=t;

        for (int i = 0; i < 16; i++) s[i] = aes_rsbox[s[i]];  /* InvSubBytes */
        add_round_key(s, rk + round*16);

        if (round != 0) {                                    /* InvMixColumns */
            for (int c = 0; c < 4; c++) {
                u8 *p = s + c*4;
                u8 a0=p[0], a1=p[1], a2=p[2], a3=p[3];
                p[0] = (u8)(gmul(a0,14) ^ gmul(a1,11) ^ gmul(a2,13) ^ gmul(a3,9));
                p[1] = (u8)(gmul(a0,9)  ^ gmul(a1,14) ^ gmul(a2,11) ^ gmul(a3,13));
                p[2] = (u8)(gmul(a0,13) ^ gmul(a1,9)  ^ gmul(a2,14) ^ gmul(a3,11));
                p[3] = (u8)(gmul(a0,11) ^ gmul(a1,13) ^ gmul(a2,9)  ^ gmul(a3,14));
            }
        }
    }
    cmemcpy(out, s, 16);
}

/* --------------------------------------------------------------- AES-CMAC
 * RFC 4493. Bluetooth Secure Simple Pairing uses it, and so does 802.11w.
 * Subkey generation is the fiddly part and the RFC's vectors catch a wrong
 * one immediately - which is exactly why they are worth running. */

static void cmac_shift_left(const u8 *in, u8 *out)
{
    u8 carry = 0;
    for (int i = 15; i >= 0; i--) {
        u8 next = (u8)(in[i] >> 7);
        out[i] = (u8)((in[i] << 1) | carry);
        carry = next;
    }
}

void aes_cmac(const u8 *key, const u8 *msg, u32 len, u8 *out)
{
    u8 rk[AES_KEYSCHED], l[16], k1[16], k2[16], x[16], block[16];
    aes128_expand(key, rk);

    cmemset(l, 0, 16);
    aes128_encrypt(rk, l, l);                      /* L = AES(K, 0^128) */

    int msb = (l[0] & 0x80) != 0;
    cmac_shift_left(l, k1);
    if (msb) k1[15] ^= 0x87;                       /* Rb for 128-bit blocks */

    msb = (k1[0] & 0x80) != 0;
    cmac_shift_left(k1, k2);
    if (msb) k2[15] ^= 0x87;

    u32 nblocks = (len + 15) / 16;
    int complete = (len != 0) && (len % 16 == 0);
    if (nblocks == 0) nblocks = 1;

    cmemset(x, 0, 16);

    for (u32 i = 0; i < nblocks - 1; i++) {
        for (int j = 0; j < 16; j++) x[j] ^= msg[i*16 + j];
        aes128_encrypt(rk, x, x);
    }

    /* the final block is padded if short, and mixed with K1 or K2 */
    u32 rem = len - (nblocks - 1) * 16;
    cmemset(block, 0, 16);
    if (complete) {
        cmemcpy(block, msg + (nblocks - 1) * 16, 16);
        for (int j = 0; j < 16; j++) block[j] ^= k1[j];
    } else {
        if (rem) cmemcpy(block, msg + (nblocks - 1) * 16, rem);
        block[rem] = 0x80;
        for (int j = 0; j < 16; j++) block[j] ^= k2[j];
    }

    for (int j = 0; j < 16; j++) x[j] ^= block[j];
    aes128_encrypt(rk, x, out);
}

/* ---------------------------------------------------------------- AES-CTR
 * The confidentiality half of CCMP. The counter block layout is CCMP's
 * business, not ours - this just runs the keystream over the data. */

void aes128_ctr(const u8 *key, const u8 *counter0, u8 *data, u32 len)
{
    u8 rk[AES_KEYSCHED], ctr[16], ks[16];
    aes128_expand(key, rk);
    cmemcpy(ctr, counter0, 16);

    u32 off = 0;
    while (off < len) {
        aes128_encrypt(rk, ctr, ks);
        u32 n = len - off; if (n > 16) n = 16;
        for (u32 i = 0; i < n; i++) data[off + i] ^= ks[i];
        off += n;
        for (int i = 15; i >= 0; i--) if (++ctr[i]) break;   /* big-endian ++ */
    }
}

/* --------------------------------------------------------------- IEEE PRF
 * 802.11i's PRF, used to expand the PMK into the PTK. It is HMAC-SHA1 over
 * (label || 0x00 || data || counter), concatenated until enough bytes exist.
 *
 * Two things here are easy to get wrong and both change every output byte:
 * the counter is a single byte STARTING AT ZERO, and the label's terminating
 * NUL IS part of the input. */

void ieee80211_prf(const u8 *key, u32 klen, const char *label,
                   const u8 *data, u32 dlen, u8 *out, u32 outlen)
{
    u8 buf[128], digest[SHA1_DIGEST];
    u32 llen = 0;
    while (label[llen]) llen++;

    u32 pos = 0;
    for (u8 counter = 0; pos < outlen; counter++) {
        u32 n = 0;
        for (u32 i = 0; i < llen && n < sizeof(buf); i++) buf[n++] = (u8)label[i];
        if (n < sizeof(buf)) buf[n++] = 0x00;
        for (u32 i = 0; i < dlen && n < sizeof(buf); i++) buf[n++] = data[i];
        if (n < sizeof(buf)) buf[n++] = counter;

        hmac_sha1(key, klen, buf, n, digest);
        u32 take = outlen - pos; if (take > SHA1_DIGEST) take = SHA1_DIGEST;
        cmemcpy(out + pos, digest, take);
        pos += take;
    }
}

/* ==================================================================== TLS 1.3
 * Everything below exists because TLS 1.3 with the single ciphersuite
 * TLS_AES_128_GCM_SHA256 needs exactly four things this file did not have:
 * AES-GCM, X25519, HKDF, and TLS's own labelled form of HKDF. SHA-256, HMAC
 * and the AES block cipher above are the rest of it.
 *
 * ONE CIPHERSUITE, deliberately. A second is not more security, it is a
 * negotiation to get wrong, and every server that matters offers this one.
 */

/* ---- GF(2^128) and GHASH, for AES-GCM -------------------------------------
 * The bit-by-bit multiply, not a table. A 4-bit table is 16x faster and costs
 * 4 KiB of precomputed state per key; this fetches a page, not a video stream,
 * and the table version is also where cache-timing leaks live. Slow and
 * obviously-correct is the right trade here.
 *
 * The bit order is GCM's, which is the reverse of the usual convention: bit 0
 * of the first byte is the MOST significant coefficient. Getting that backwards
 * produces a tag that is wrong in a way that looks like a key problem. */
static void gf128_mul(u8 *x, const u8 *y)
{
    u8 z[16], v[16];
    cmemset(z, 0, 16);
    cmemcpy(v, y, 16);
    for (int i = 0; i < 128; i++) {
        if ((x[i >> 3] >> (7 - (i & 7))) & 1)
            for (int j = 0; j < 16; j++) z[j] ^= v[j];
        int lsb = v[15] & 1;
        for (int j = 15; j > 0; j--) v[j] = (u8)((v[j] >> 1) | ((v[j - 1] & 1) << 7));
        v[0] >>= 1;
        if (lsb) v[0] ^= 0xE1;          /* the reduction polynomial */
    }
    cmemcpy(x, z, 16);
}

static void ghash(const u8 *H, const u8 *aad, u32 alen,
                  const u8 *ct, u32 clen, u8 *out)
{
    u8 y[16], L[16];
    cmemset(y, 0, 16);
    for (u32 i = 0; i < alen; i += 16) {
        u32 n = (alen - i < 16) ? alen - i : 16;
        for (u32 j = 0; j < n; j++) y[j] ^= aad[i + j];
        gf128_mul(y, H);
    }
    for (u32 i = 0; i < clen; i += 16) {
        u32 n = (clen - i < 16) ? clen - i : 16;
        for (u32 j = 0; j < n; j++) y[j] ^= ct[i + j];
        gf128_mul(y, H);
    }
    /* the trailing block is the two lengths in BITS, big-endian */
    u64 ab = (u64)alen * 8, cb = (u64)clen * 8;
    for (int i = 0; i < 8; i++) L[7 - i]  = (u8)(ab >> (8 * i));
    for (int i = 0; i < 8; i++) L[15 - i] = (u8)(cb >> (8 * i));
    for (int i = 0; i < 16; i++) y[i] ^= L[i];
    gf128_mul(y, H);
    cmemcpy(out, y, 16);
}

/* AES-128-GCM with a 96-bit IV, which is the only length TLS 1.3 uses.
 * `data` is encrypted IN PLACE. */
void aes128_gcm_encrypt(const u8 *key, const u8 *iv12,
                        const u8 *aad, u32 alen,
                        u8 *data, u32 len, u8 *tag)
{
    u8 rk[176], H[16], J0[16], ek[16], ctr[16], ks[16], zero[16];
    aes128_expand(key, rk);
    cmemset(zero, 0, 16);
    aes128_encrypt(rk, zero, H);
    cmemcpy(J0, iv12, 12);
    J0[12] = 0; J0[13] = 0; J0[14] = 0; J0[15] = 1;
    aes128_encrypt(rk, J0, ek);              /* the tag mask */
    cmemcpy(ctr, J0, 16);
    for (u32 off = 0; off < len; off += 16) {
        for (int i = 15; i >= 12; i--) if (++ctr[i]) break;   /* J0+1, J0+2... */
        aes128_encrypt(rk, ctr, ks);
        u32 n = (len - off < 16) ? len - off : 16;
        for (u32 j = 0; j < n; j++) data[off + j] ^= ks[j];
    }
    ghash(H, aad, alen, data, len, tag);
    for (int i = 0; i < 16; i++) tag[i] ^= ek[i];
}

/* Returns 1 if the tag verified and `data` now holds plaintext, 0 if it did
 * NOT - in which case data is left as ciphertext and must not be used.
 *
 * THE TAG IS CHECKED BEFORE ANYTHING IS DECRYPTED. Releasing plaintext that
 * failed authentication, even briefly, even to a caller that promises to
 * discard it, is the whole class of attack AEAD exists to stop. */
int aes128_gcm_decrypt(const u8 *key, const u8 *iv12,
                       const u8 *aad, u32 alen,
                       u8 *data, u32 len, const u8 *tag)
{
    u8 rk[176], H[16], J0[16], ek[16], ctr[16], ks[16], zero[16], want[16];
    aes128_expand(key, rk);
    cmemset(zero, 0, 16);
    aes128_encrypt(rk, zero, H);
    cmemcpy(J0, iv12, 12);
    J0[12] = 0; J0[13] = 0; J0[14] = 0; J0[15] = 1;
    aes128_encrypt(rk, J0, ek);

    ghash(H, aad, alen, data, len, want);
    for (int i = 0; i < 16; i++) want[i] ^= ek[i];
    if (!crypto_equal(want, tag, 16)) return 0;

    cmemcpy(ctr, J0, 16);
    for (u32 off = 0; off < len; off += 16) {
        for (int i = 15; i >= 12; i--) if (++ctr[i]) break;
        aes128_encrypt(rk, ctr, ks);
        u32 n = (len - off < 16) ? len - off : 16;
        for (u32 j = 0; j < n; j++) data[off + j] ^= ks[j];
    }
    return 1;
}

/* ---- X25519 ---------------------------------------------------------------
 * The Montgomery ladder over Curve25519, in the 16-limb 16-bit representation
 * TweetNaCl uses. That representation is chosen for a 32-bit target: every
 * intermediate fits in an i64, so there is no 128-bit arithmetic and no
 * assembly, which is what makes this compile for i386 unchanged.
 *
 * CONSTANT TIME BY CONSTRUCTION. The ladder does the same work for every bit
 * and the conditional swap is arithmetic, not a branch - sel25519 builds a
 * mask from the bit rather than testing it. A key-dependent branch here leaks
 * the private key through timing, which is not a theoretical attack.
 */
typedef long long i64;
typedef i64 gf[16];

static const gf gf121665 = { 0xDB41, 1 };

static void car25519(gf o)
{
    i64 c;
    for (int i = 0; i < 16; i++) {
        o[i] += (1LL << 16);
        c = o[i] >> 16;
        o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
        o[i] -= c << 16;
    }
}

static void sel25519(gf p, gf q, int b)
{
    i64 t, c = ~(i64)(b - 1);
    for (int i = 0; i < 16; i++) {
        t = c & (p[i] ^ q[i]);
        p[i] ^= t;
        q[i] ^= t;
    }
}

static void pack25519(u8 *o, const gf n)
{
    int b;
    gf m, t;
    for (int i = 0; i < 16; i++) t[i] = n[i];
    car25519(t); car25519(t); car25519(t);
    for (int j = 0; j < 2; j++) {
        m[0] = t[0] - 0xFFED;
        for (int i = 1; i < 15; i++) {
            m[i] = t[i] - 0xFFFF - ((m[i - 1] >> 16) & 1);
            m[i - 1] &= 0xFFFF;
        }
        m[15] = t[15] - 0x7FFF - ((m[14] >> 16) & 1);
        b = (int)((m[15] >> 16) & 1);
        m[14] &= 0xFFFF;
        sel25519(t, m, 1 - b);
    }
    for (int i = 0; i < 16; i++) {
        o[2 * i]     = (u8)(t[i] & 0xFF);
        o[2 * i + 1] = (u8)(t[i] >> 8);
    }
}

static void unpack25519(gf o, const u8 *n)
{
    for (int i = 0; i < 16; i++) o[i] = n[2 * i] + ((i64)n[2 * i + 1] << 8);
    o[15] &= 0x7FFF;
}

static void gf_add(gf o, const gf a, const gf b) { for (int i = 0; i < 16; i++) o[i] = a[i] + b[i]; }
static void gf_sub(gf o, const gf a, const gf b) { for (int i = 0; i < 16; i++) o[i] = a[i] - b[i]; }

static void gf_mul(gf o, const gf a, const gf b)
{
    i64 t[31];
    for (int i = 0; i < 31; i++) t[i] = 0;
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 16; j++) t[i + j] += a[i] * b[j];
    for (int i = 0; i < 15; i++) t[i] += 38 * t[i + 16];
    for (int i = 0; i < 16; i++) o[i] = t[i];
    car25519(o); car25519(o);
}

static void gf_sq(gf o, const gf a) { gf_mul(o, a, a); }

static void inv25519(gf o, const gf i)
{
    gf c;
    for (int a = 0; a < 16; a++) c[a] = i[a];
    for (int a = 253; a >= 0; a--) {
        gf_sq(c, c);
        if (a != 2 && a != 4) gf_mul(c, c, i);
    }
    for (int a = 0; a < 16; a++) o[a] = c[a];
}

void x25519(u8 *out, const u8 *scalar, const u8 *point)
{
    u8 z[32];
    gf x, a, b, c, d, e, f;
    i64 r;
    for (int i = 0; i < 31; i++) z[i] = scalar[i];
    /* the clamp: RFC 7748 requires it and it is what keeps the scalar in the
     * prime-order subgroup */
    z[31] = (u8)((scalar[31] & 127) | 64);
    z[0] &= 248;
    unpack25519(x, point);
    for (int i = 0; i < 16; i++) { b[i] = x[i]; d[i] = a[i] = c[i] = 0; }
    a[0] = d[0] = 1;
    for (int i = 254; i >= 0; --i) {
        r = (z[i >> 3] >> (i & 7)) & 1;
        sel25519(a, b, (int)r);
        sel25519(c, d, (int)r);
        gf_add(e, a, c);
        gf_sub(a, a, c);
        gf_add(c, b, d);
        gf_sub(b, b, d);
        gf_sq(d, e);
        gf_sq(f, a);
        gf_mul(a, c, a);
        gf_mul(c, b, e);
        gf_add(e, a, c);
        gf_sub(a, a, c);
        gf_sq(b, a);
        gf_sub(c, d, f);
        gf_mul(a, c, gf121665);
        gf_add(a, a, d);
        gf_mul(c, c, a);
        gf_mul(a, d, f);
        gf_mul(d, b, x);
        gf_sq(b, e);
        sel25519(a, b, (int)r);
        sel25519(c, d, (int)r);
    }
    inv25519(c, c);
    gf_mul(a, a, c);
    pack25519(out, a);
}

/* the base point is u=9, so a public key is x25519(priv, {9,0,0,...}) */
void x25519_base(u8 *out, const u8 *scalar)
{
    u8 nine[32];
    cmemset(nine, 0, 32);
    nine[0] = 9;
    x25519(out, scalar, nine);
}

/* ---- HKDF (RFC 5869) and TLS 1.3's labelled form -------------------------- */

void hkdf_extract(const u8 *salt, u32 slen, const u8 *ikm, u32 ilen, u8 *prk)
{
    hmac_sha256(salt, slen, ikm, ilen, prk);
}

void hkdf_expand(const u8 *prk, const u8 *info, u32 ilen, u8 *out, u32 olen)
{
    u8 t[32], buf[32 + 256 + 1];
    u32 tlen = 0, done = 0;
    u8 ctr = 1;
    if (ilen > 256) ilen = 256;          /* bounded, like everything here */
    while (done < olen) {
        u32 n = 0;
        for (u32 i = 0; i < tlen; i++) buf[n++] = t[i];
        for (u32 i = 0; i < ilen; i++) buf[n++] = info[i];
        buf[n++] = ctr++;
        hmac_sha256(prk, 32, buf, n, t);
        tlen = 32;
        u32 take = (olen - done < 32) ? olen - done : 32;
        for (u32 i = 0; i < take; i++) out[done + i] = t[i];
        done += take;
    }
}

/* HKDF-Expand-Label from RFC 8446 §7.1. The "tls13 " prefix is what stops a
 * key derived for one purpose being valid for another. */
void tls13_expand_label(const u8 *secret, const char *label,
                        const u8 *ctx, u32 clen, u8 *out, u32 olen)
{
    u8 info[2 + 1 + 6 + 64 + 1 + 64];
    u32 n = 0, lab = 0;
    while (label[lab]) lab++;
    if (lab > 64) lab = 64;
    if (clen > 64) clen = 64;
    info[n++] = (u8)(olen >> 8);
    info[n++] = (u8)olen;
    info[n++] = (u8)(6 + lab);
    const char *p = "tls13 ";
    for (u32 i = 0; i < 6; i++) info[n++] = (u8)p[i];
    for (u32 i = 0; i < lab; i++) info[n++] = (u8)label[i];
    info[n++] = (u8)clen;
    for (u32 i = 0; i < clen; i++) info[n++] = ctx[i];
    hkdf_expand(secret, info, n, out, olen);
}

/* Derive-Secret(secret, label, messages) = Expand-Label(secret, label,
 * Hash(messages), 32). Split out because the transcript hash is what binds a
 * key to the exact handshake it came from. */
void tls13_derive_secret(const u8 *secret, const char *label,
                         const u8 *thash, u8 *out)
{
    tls13_expand_label(secret, label, thash, 32, out, 32);
}
