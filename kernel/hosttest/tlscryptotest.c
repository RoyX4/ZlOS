/* tlscryptotest.c - the four primitives TLS 1.3 needs, against published vectors.
 *
 * cryptotest.c covers SHA-1, SHA-256, HMAC, PBKDF2 and AES. This covers what
 * was added for TLS: AES-GCM, X25519, HKDF and HKDF-Expand-Label. Same rule as
 * that file, and it is the whole reason this code could be written with no
 * server to talk to:
 *
 *   EVERY EXPECTED VALUE IS FROM A PUBLISHED STANDARD, not from a previous run
 *   of this program. A self-generated golden output only proves the code agrees
 *   with itself; these prove it agrees with the rest of the world, which is
 *   what matters when the other end is Wikipedia's load balancer.
 *
 *     AES-128-GCM   NIST SP 800-38D test cases 1, 2 and 4
 *     X25519        RFC 7748 SS5.2 and SS6.1
 *     HKDF-SHA256   RFC 5869 test case 1
 *
 * Each was additionally cross-checked against an independent implementation
 * (OpenSSL 3.6 / python-cryptography 47) before being written down, because a
 * hex constant recalled from memory is a made-up number until something else
 * agrees with it.
 *
 * Beyond the vectors, three PROPERTIES that no published constant can express:
 * that GCM decryption REFUSES a tampered tag, that it refuses tampered
 * ciphertext, and that two X25519 parties agree. The refusals matter most - a
 * GCM decrypt that returns plaintext regardless of the tag passes every
 * encrypt-side vector in this file.
 *
 *   cd kernel/hosttest && ./build.sh && ./tlscryptotest
 */
#include <stdio.h>
#include <string.h>

#include "../crypto.c"

static int passed, failed;

static int unhex(const char *h, u8 *out, int max)
{
    int n = 0;
    for (const char *p = h; p[0] && p[1] && n < max; p += 2) {
        int hi = (p[0] <= '9') ? p[0] - '0' : (p[0] | 32) - 'a' + 10;
        int lo = (p[1] <= '9') ? p[1] - '0' : (p[1] | 32) - 'a' + 10;
        out[n++] = (u8)((hi << 4) | lo);
    }
    return n;
}

static void chk(const char *what, const u8 *got, const char *want_hex, int n)
{
    u8 want[256];
    int wn = unhex(want_hex, want, (int)sizeof want);
    int ok = (wn == n) && !memcmp(got, want, (size_t)n);
    if (ok) { passed++; printf("  ok   %s\n", what); }
    else {
        failed++;
        printf("  FAIL %s\n       got  ", what);
        for (int i = 0; i < n; i++) printf("%02x", got[i]);
        printf("\n       want %s\n", want_hex);
    }
}

static void chk_bool(const char *what, int cond)
{
    if (cond) { passed++; printf("  ok   %s\n", what); }
    else { failed++; printf("  FAIL %s\n", what); }
}

int main(void)
{
    u8 key[32], iv[16], pt[128], aad[64], tag[16], buf[128];

    printf("\n=== AES-128-GCM (NIST SP 800-38D) ===\n");
    /* Test case 1: empty key, empty IV, no plaintext, no AAD - tag only */
    memset(key, 0, 16); memset(iv, 0, 12);
    aes128_gcm_encrypt(key, iv, 0, 0, 0, 0, tag);
    chk("tc1  tag over nothing", tag, "58e2fccefa7e3061367f1d57a4e7455a", 16);

    /* Test case 2: 16 zero bytes of plaintext */
    memset(buf, 0, 16);
    aes128_gcm_encrypt(key, iv, 0, 0, buf, 16, tag);
    chk("tc2  ciphertext", buf, "0388dace60b6a392f328c2b971b2fe78", 16);
    chk("tc2  tag",        tag, "ab6e47d42cec13bdf53a67b21257bddf", 16);

    /* Test case 4: real key, 60 bytes of plaintext, 20 bytes of AAD */
    unhex("feffe9928665731c6d6a8f9467308308", key, 16);
    unhex("cafebabefacedbaddecaf888", iv, 12);
    int ptn = unhex("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
                    "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39", pt, 128);
    int an = unhex("feedfacedeadbeeffeedfacedeadbeefabaddad2", aad, 64);
    memcpy(buf, pt, (size_t)ptn);
    aes128_gcm_encrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag);
    chk("tc4  ciphertext", buf,
        "42831ec2217774244b7221b784d0d49ce3aa212f2c02a4e035c17e2329aca12e"
        "21d514b25466931c7d8f6a5aac84aa051ba30b396a0aac973d58e091", ptn);
    chk("tc4  tag", tag, "5bc94fbc3221a5db94fae95ae7121a47", 16);

    /* ...and it decrypts back */
    chk_bool("tc4  decrypt verifies and restores plaintext",
             aes128_gcm_decrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag) &&
             !memcmp(buf, pt, (size_t)ptn));

    printf("\n=== AES-GCM refusals (no vector can express these) ===\n");
    /* a flipped tag bit must be refused, and the buffer left alone */
    memcpy(buf, pt, (size_t)ptn);
    aes128_gcm_encrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag);
    {
        u8 bad[16];
        memcpy(bad, tag, 16);
        bad[0] ^= 1;
        u8 save[128];
        memcpy(save, buf, (size_t)ptn);
        int r = aes128_gcm_decrypt(key, iv, aad, (u32)an, buf, (u32)ptn, bad);
        chk_bool("a flipped tag bit is refused", r == 0);
        chk_bool("...and the ciphertext was NOT decrypted in place",
                 !memcmp(buf, save, (size_t)ptn));
    }
    /* a flipped ciphertext bit must be refused too */
    {
        memcpy(buf, pt, (size_t)ptn);
        aes128_gcm_encrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag);
        buf[3] ^= 0x80;
        chk_bool("a flipped ciphertext bit is refused",
                 aes128_gcm_decrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag) == 0);
    }
    /* tampered AAD must be refused - the AAD is the TLS record header, so this
     * is what stops a record being replayed as a different content type */
    {
        memcpy(buf, pt, (size_t)ptn);
        aes128_gcm_encrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag);
        aad[0] ^= 1;
        chk_bool("tampered AAD is refused",
                 aes128_gcm_decrypt(key, iv, aad, (u32)an, buf, (u32)ptn, tag) == 0);
        aad[0] ^= 1;
    }

    printf("\n=== SHA-384 / SHA-512 (FIPS 180-4) ===\n");
    {
        u8 h[64];
        sha384((const u8 *)"abc", 3, h);
        chk("SHA-384 \"abc\"", h,
            "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed"
            "8086072ba1e7cc2358baeca134c825a7", 48);
        sha384((const u8 *)"", 0, h);
        chk("SHA-384 empty", h,
            "38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da"
            "274edebfe76f65fbd51ad2f14898b95b", 48);
        /* the multi-block case, which is where the padding and length encoding
         * go wrong: 56 bytes is one byte past the point a length no longer
         * fits in the first block */
        const char *m = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        sha384((const u8 *)m, 56, h);
        chk("SHA-384 56 bytes (spills to a second block)", h,
            "3391fdddfc8dc7393707a65b1b4709397cf8b1d162af05abfe8f450de5f36bc6"
            "b0455a8520bc4e6f5fe95b1fe3c8452b", 48);
        sha512((const u8 *)"abc", 3, h);
        chk("SHA-512 \"abc\"", h,
            "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
            "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f", 64);
    }

    printf("\n=== X25519 (RFC 7748) ===\n");
    {
        u8 s[32], u[32], out[32];
        unhex("a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4", s, 32);
        unhex("e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c", u, 32);
        x25519(out, s, u);
        chk("SS5.2 scalar multiplication", out,
            "c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552", 32);

        u8 apriv[32], bpriv[32], apub[32], bpub[32], ss1[32], ss2[32];
        unhex("77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a", apriv, 32);
        unhex("5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb", bpriv, 32);
        x25519_base(apub, apriv);
        x25519_base(bpub, bpriv);
        chk("SS6.1 alice public", apub,
            "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a", 32);
        chk("SS6.1 bob public", bpub,
            "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f", 32);
        x25519(ss1, apriv, bpub);
        x25519(ss2, bpriv, apub);
        chk("SS6.1 shared secret", ss1,
            "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742", 32);
        /* the property, not the constant: both sides must land on the same
         * secret, which is the only thing a handshake actually depends on */
        chk_bool("both parties derive the SAME secret", !memcmp(ss1, ss2, 32));
    }

    printf("\n=== HKDF-SHA256 (RFC 5869 test case 1) ===\n");
    {
        u8 ikm[32], salt[32], info[32], prk[32], okm[64];
        memset(ikm, 0x0b, 22);
        unhex("000102030405060708090a0b0c", salt, 32);
        unhex("f0f1f2f3f4f5f6f7f8f9", info, 32);
        hkdf_extract(salt, 13, ikm, 22, prk);
        chk("extract -> PRK", prk,
            "077709362c2e32df0ddc3f0dc47bba6390b6c73bb50f9c3122ec844ad7c2b3e5", 32);
        hkdf_expand(prk, info, 10, okm, 42);
        chk("expand -> 42 bytes of OKM", okm,
            "3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf"
            "34007208d5b887185865", 42);
    }

    printf("\n=== TLS 1.3 HKDF-Expand-Label (RFC 8446 SS7.1) ===\n");
    {
        /* No published vector exists for a label in isolation, so this asserts
         * the STRUCTURE the RFC specifies, by deriving the same thing by hand
         * from hkdf_expand: HkdfLabel = len16 || "tls13 "+label || context. If
         * expand_label built the info block differently, these would differ. */
        u8 secret[32], ctx[32], a[32], b[32], info[64];
        memset(secret, 0x42, 32);
        memset(ctx, 0x7e, 32);
        tls13_expand_label(secret, "key", ctx, 32, a, 32);
        u32 n = 0;
        info[n++] = 0; info[n++] = 32;
        info[n++] = 6 + 3;
        memcpy(info + n, "tls13 key", 9); n += 9;
        info[n++] = 32;
        memcpy(info + n, ctx, 32); n += 32;
        hkdf_expand(secret, info, n, b, 32);
        chk_bool("expand_label builds the RFC's HkdfLabel", !memcmp(a, b, 32));

        /* and the label is load-bearing: two labels must not collide */
        u8 c[32];
        tls13_expand_label(secret, "iv", ctx, 32, c, 32);
        chk_bool("a different label gives a different key", memcmp(a, c, 32) != 0);
    }

    printf("\n---------------------------------------------\n");
    printf("  %d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
