/* cryptotest.c - crypto.c against published test vectors.
 *
 * Compiles the SHIPPING crypto.c unmodified, the same way hosttest/ compiles
 * the shipping intel.c and fb.c. If this passes, the code in the kernel is
 * the code that passed.
 *
 * Every expected value below is copied from a published standard, not from a
 * previous run of this program. That distinction is the entire point: a
 * self-generated "golden" output only proves the code is consistent with
 * itself. These prove it is consistent with the rest of the world, which is
 * what matters when the other end of the handshake is somebody's router.
 *
 *   SHA-1        FIPS 180-1 / RFC 3174
 *   SHA-256      FIPS 180-4
 *   HMAC-SHA1    RFC 2202
 *   HMAC-SHA256  RFC 4231
 *   PBKDF2       RFC 6070
 *   AES-128      FIPS-197 Appendix C.1
 *   AES-CMAC     RFC 4493
 *   WPA2 PMK     IEEE 802.11i-2004
 *
 *   cd kernel/hosttest && ./build.sh && ./cryptotest
 */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../crypto.c"

static int passed = 0, failed = 0;

static int unhex(const char *hex, u8 *out)
{
    int n = 0;
    for (const char *p = hex; *p; ) {
        if (*p == ' ') { p++; continue; }
        int hi = (*p <= '9') ? *p - '0' : (*p | 32) - 'a' + 10; p++;
        int lo = (*p <= '9') ? *p - '0' : (*p | 32) - 'a' + 10; p++;
        out[n++] = (u8)((hi << 4) | lo);
    }
    return n;
}

static void check(const char *name, const u8 *got, const char *want_hex, u32 n)
{
    u8 want[128];
    unhex(want_hex, want);
    if (memcmp(got, want, n) == 0) {
        passed++;
        printf("  ok   %s\n", name);
    } else {
        failed++;
        printf("  FAIL %s\n       want ", name);
        for (u32 i = 0; i < n; i++) printf("%02x", want[i]);
        printf("\n       got  ");
        for (u32 i = 0; i < n; i++) printf("%02x", got[i]);
        printf("\n");
    }
}

int main(void)
{
    u8 d[64], key[64], msg[128], out[64];

    printf("\n=== SHA-1 (FIPS 180-1) ===\n");
    sha1((const u8 *)"abc", 3, d);
    check("\"abc\"", d, "a9993e364706816aba3e25717850c26c9cd0d89d", 20);
    sha1((const u8 *)"", 0, d);
    check("empty", d, "da39a3ee5e6b4b0d3255bfef95601890afd80709", 20);
    {
        const char *s = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        sha1((const u8 *)s, (u32)strlen(s), d);
        check("56-byte (spans the pad boundary)", d,
              "84983e441c3bd26ebaae4aa1f95129e5e54670f1", 20);
    }
    {   /* the million-a case: exercises multi-block update and the 64-bit len */
        sha1_ctx c; sha1_init(&c);
        u8 a[1000]; memset(a, 'a', sizeof a);
        for (int i = 0; i < 1000; i++) sha1_update(&c, a, 1000);
        sha1_final(&c, d);
        check("1,000,000 x 'a'", d, "34aa973cd4c4daa4f61eeb2bdbad27316534016f", 20);
    }

    printf("\n=== SHA-256 (FIPS 180-4) ===\n");
    sha256((const u8 *)"abc", 3, d);
    check("\"abc\"", d,
          "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", 32);
    sha256((const u8 *)"", 0, d);
    check("empty", d,
          "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855", 32);
    {
        const char *s = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
        sha256((const u8 *)s, (u32)strlen(s), d);
        check("56-byte", d,
              "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1", 32);
    }

    printf("\n=== HMAC-SHA1 (RFC 2202) ===\n");
    memset(key, 0x0b, 20);
    hmac_sha1(key, 20, (const u8 *)"Hi There", 8, d);
    check("case 1", d, "b617318655057264e28bc0b6fb378c8ef146be00", 20);

    hmac_sha1((const u8 *)"Jefe", 4,
              (const u8 *)"what do ya want for nothing?", 28, d);
    check("case 2", d, "effcdf6ae5eb2fa2d27416d5f184df9c259a7c79", 20);

    memset(key, 0xaa, 20); memset(msg, 0xdd, 50);
    hmac_sha1(key, 20, msg, 50, d);
    check("case 3", d, "125d7342b9ac11cd91a39af48aa17b4f63f175d3", 20);

    {   /* a key longer than the block, which takes the hash-the-key branch */
        u8 longkey[80]; memset(longkey, 0xaa, 80);
        hmac_sha1(longkey, 80,
                  (const u8 *)"Test Using Larger Than Block-Size Key - Hash Key First", 54, d);
        check("case 6 (80-byte key)", d,
              "aa4ae5e15272d00e95705637ce8a3b55ed402112", 20);
    }

    printf("\n=== HMAC-SHA256 (RFC 4231) ===\n");
    memset(key, 0x0b, 20);
    hmac_sha256(key, 20, (const u8 *)"Hi There", 8, d);
    check("case 1", d,
          "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7", 32);

    hmac_sha256((const u8 *)"Jefe", 4,
                (const u8 *)"what do ya want for nothing?", 28, d);
    check("case 2", d,
          "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843", 32);

    printf("\n=== PBKDF2-HMAC-SHA1 (RFC 6070) ===\n");
    pbkdf2_sha1((const u8 *)"password", 8, (const u8 *)"salt", 4, 1, out, 20);
    check("c=1", out, "0c60c80f961f0e71f3a9b524af6012062fe037a6", 20);
    pbkdf2_sha1((const u8 *)"password", 8, (const u8 *)"salt", 4, 2, out, 20);
    check("c=2", out, "ea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957", 20);
    pbkdf2_sha1((const u8 *)"password", 8, (const u8 *)"salt", 4, 4096, out, 20);
    check("c=4096", out, "4b007901b765489abead49d926f721d065a429c1", 20);
    pbkdf2_sha1((const u8 *)"passwordPASSWORDpassword", 24,
                (const u8 *)"saltSALTsaltSALTsaltSALTsaltSALTsalt", 36,
                4096, out, 25);
    check("c=4096, dkLen=25 (multi-block)", out,
          "3d2eec4fe41c849b80c8d83662c0e44a8b291a964cf2f07038", 25);

    printf("\n=== AES-128 (FIPS-197 C.1) ===\n");
    {
        u8 rk[176], ct[16], pt[16];
        unhex("000102030405060708090a0b0c0d0e0f", key);
        unhex("00112233445566778899aabbccddeeff", msg);
        aes128_expand(key, rk);
        aes128_encrypt(rk, msg, ct);
        check("encrypt", ct, "69c4e0d86a7b0430d8cdb78070b4c55a", 16);
        aes128_decrypt(rk, ct, pt);
        check("decrypt round-trips", pt, "00112233445566778899aabbccddeeff", 16);
    }

    printf("\n=== AES-CMAC (RFC 4493) ===\n");
    {
        unhex("2b7e151628aed2a6abf7158809cf4f3c", key);
        int n;

        aes_cmac(key, msg, 0, d);
        check("len 0", d, "bb1d6929e95937287fa37d129b756746", 16);

        n = unhex("6bc1bee22e409f96e93d7e117393172a", msg);
        aes_cmac(key, msg, (u32)n, d);
        check("len 16", d, "070a16b46b4d4144f79bdd9dd04a287c", 16);

        n = unhex("6bc1bee22e409f96e93d7e117393172a"
                  "ae2d8a571e03ac9c9eb76fac45af8e51"
                  "30c81c46a35ce411", msg);
        aes_cmac(key, msg, (u32)n, d);
        check("len 40 (partial final block)", d,
              "dfa66747de9ae63030ca32611497c827", 16);

        n = unhex("6bc1bee22e409f96e93d7e117393172a"
                  "ae2d8a571e03ac9c9eb76fac45af8e51"
                  "30c81c46a35ce411e5fbc1191a0a52ef"
                  "f69f2445df4f9b17ad2b417be66c3710", msg);
        aes_cmac(key, msg, (u32)n, d);
        check("len 64 (complete final block)", d,
              "51f0bebf7e3b9d92fc49741779363cfe", 16);
    }

    printf("\n=== AES-CTR round-trip ===\n");
    {
        u8 ctr[16], data[40], orig[40];
        unhex("000102030405060708090a0b0c0d0e0f", key);
        unhex("f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff", ctr);
        for (int i = 0; i < 40; i++) data[i] = orig[i] = (u8)i;
        aes128_ctr(key, ctr, data, 40);
        int changed = memcmp(data, orig, 40) != 0;
        aes128_ctr(key, ctr, data, 40);          /* CTR is its own inverse */
        if (changed && memcmp(data, orig, 40) == 0) {
            passed++; printf("  ok   encrypt then decrypt restores plaintext\n");
        } else {
            failed++; printf("  FAIL CTR round-trip\n");
        }
    }

    printf("\n=== WPA2 PMK (IEEE 802.11i-2004) ===\n");
    {   /* This is the real thing: passphrase + SSID -> the 256-bit PMK that
         * every WPA2 join derives before the 4-way handshake starts. */
        struct { const char *pass, *ssid, *pmk; } v[] = {
          { "password", "IEEE",
            "f42c6fc52df0ebef9ebb4b90b38a5f902e83fe1b135a70e23aed762e9710a12e" },
          { "ThisIsAPassword", "ThisIsASSID",
            "0dc0d6eb90555ed6419756b9a15ec3e3209b63df707dd508d14581f8982721af" },
        };
        for (int i = 0; i < 2; i++) {
            pbkdf2_sha1((const u8 *)v[i].pass, (u32)strlen(v[i].pass),
                        (const u8 *)v[i].ssid, (u32)strlen(v[i].ssid),
                        4096, out, 32);
            char nm[96];
            snprintf(nm, sizeof nm, "PMK(\"%s\", \"%s\")", v[i].pass, v[i].ssid);
            check(nm, out, v[i].pmk, 32);
        }
    }

    printf("\n=== constant-time compare ===\n");
    {
        u8 a[16], b[16];
        memset(a, 0x5a, 16); memset(b, 0x5a, 16);
        int ok = crypto_equal(a, b, 16);
        b[15] ^= 1;  ok &= !crypto_equal(a, b, 16);
        b[15] ^= 1; b[0] ^= 0x80; ok &= !crypto_equal(a, b, 16);
        if (ok) { passed++; printf("  ok   equal/unequal both correct\n"); }
        else    { failed++; printf("  FAIL crypto_equal\n"); }
    }

    /* How slow is the PMK really? The claim in wireless-plan.md is that this
     * must be cached, so measure it rather than assert it. */
    printf("\n=== cost of one PMK (4096 iterations) ===\n");
    {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (int i = 0; i < 10; i++)
            pbkdf2_sha1((const u8 *)"password", 8, (const u8 *)"IEEE", 4, 4096, out, 32);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double ms = ((double)(t1.tv_sec - t0.tv_sec) * 1000.0 +
                     (double)(t1.tv_nsec - t0.tv_nsec) / 1e6) / 10.0;
        printf("  %.1f ms per PMK  -> cache it; do not recompute per handshake\n", ms);
    }

    printf("\n---------------------------------------------\n");
    printf("  %d passed, %d failed\n\n", passed, failed);
    return failed ? 1 : 0;
}
