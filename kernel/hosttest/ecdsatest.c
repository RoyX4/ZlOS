/* ecdsatest.c - ECDSA verification over P-256 and P-384.
 *
 * A VERIFIER IS TESTED BY WHAT IT REJECTS, not by what it accepts. An
 * implementation that returns 1 unconditionally passes every "valid signature
 * verifies" test ever written, and it is exactly the bug that matters: it
 * makes every forged certificate valid. So for each curve this checks the good
 * signature once and then eight ways of being wrong.
 *
 * The vectors come from an independent implementation (python-cryptography 47
 * over OpenSSL 3.6) rather than from a previous run of this code - the same
 * rule cryptotest.c sets. Signing needs randomness, which this kernel has none
 * of and ecdsa.c deliberately does not implement, so the signatures could not
 * have come from here in any case.
 */
#include <stdio.h>
#include <string.h>

int ecdsa_verify(int curve_bits, const unsigned char *pub,
                 const unsigned char *r, const unsigned char *s,
                 const unsigned char *hash, int hashlen);

static int passed, failed;

static void ok(const char *what, int cond)
{
    if (cond) { passed++; printf("  ok   %s\n", what); }
    else { failed++; printf("  FAIL %s\n", what); }
}

static int unhex(const char *h, unsigned char *o, int max)
{
    int n = 0;
    for (const char *p = h; p[0] && p[1] && n < max; p += 2) {
        int hi = (p[0] <= '9') ? p[0] - '0' : (p[0] | 32) - 'a' + 10;
        int lo = (p[1] <= '9') ? p[1] - '0' : (p[1] | 32) - 'a' + 10;
        o[n++] = (unsigned char)((hi << 4) | lo);
    }
    return n;
}

static void suite(int bits, const char *pubh, const char *rh,
                  const char *sh, const char *hh)
{
    unsigned char pub[96], r[48], s[48], h[48], t[96];
    int size = bits / 8;
    unhex(pubh, pub, 96);
    unhex(rh, r, 48);
    unhex(sh, s, 48);
    int hl = unhex(hh, h, 48);

    char lbl[64];
    snprintf(lbl, sizeof lbl, "P-%d: a valid signature verifies", bits);
    ok(lbl, ecdsa_verify(bits, pub, r, s, h, hl) == 1);

    /* ---- and now every way of being wrong --------------------------------- */
    snprintf(lbl, sizeof lbl, "P-%d: one flipped bit in r is rejected", bits);
    memcpy(t, r, (size_t)size); t[0] ^= 1;
    ok(lbl, ecdsa_verify(bits, pub, t, s, h, hl) == 0);

    snprintf(lbl, sizeof lbl, "P-%d: one flipped bit in s is rejected", bits);
    memcpy(t, s, (size_t)size); t[size - 1] ^= 1;
    ok(lbl, ecdsa_verify(bits, pub, r, t, h, hl) == 0);

    snprintf(lbl, sizeof lbl, "P-%d: a different message is rejected", bits);
    memcpy(t, h, (size_t)hl); t[0] ^= 0x80;
    ok(lbl, ecdsa_verify(bits, pub, r, s, t, hl) == 0);

    snprintf(lbl, sizeof lbl, "P-%d: a different public key is rejected", bits);
    memcpy(t, pub, (size_t)(size * 2)); t[1] ^= 0x40;
    ok(lbl, ecdsa_verify(bits, t, r, s, h, hl) == 0);

    /* r = 0 and s = 0 are the textbook forgeries: without the range check they
     * make the verification equation trivially satisfiable */
    snprintf(lbl, sizeof lbl, "P-%d: r = 0 is rejected", bits);
    memset(t, 0, (size_t)size);
    ok(lbl, ecdsa_verify(bits, pub, t, s, h, hl) == 0);

    snprintf(lbl, sizeof lbl, "P-%d: s = 0 is rejected", bits);
    memset(t, 0, (size_t)size);
    ok(lbl, ecdsa_verify(bits, pub, r, t, h, hl) == 0);

    /* a public key that is not on the curve - accepting one is an invalid-curve
     * attack, not a theoretical concern */
    snprintf(lbl, sizeof lbl, "P-%d: a point not on the curve is rejected", bits);
    memcpy(t, pub, (size_t)(size * 2));
    t[size] ^= 0x01;                      /* perturb y only */
    ok(lbl, ecdsa_verify(bits, t, r, s, h, hl) == 0);

    /* r and s swapped: both are in range and both are real numbers from a real
     * signature, so only the maths rejects this one */
    snprintf(lbl, sizeof lbl, "P-%d: r and s swapped is rejected", bits);
    ok(lbl, ecdsa_verify(bits, pub, s, r, h, hl) == 0);
}

int main(void)
{
    printf("ecdsatest: signature verification\n\n");

    printf("P-256 (the curve of every Let's Encrypt leaf)\n");
    suite(256,
        "68fbd2e8cb9f1e987dccd36085f8b33ba7b129d21dafeb3d20ca5c438838239b"
        "71c5dbef5cdb43dc206519d0353a0cede3b13693150364c4f0147c8204adffd6",
        "f24dc80b834dc61dc6e0ff4b23441b9d7253278d5c8013f5dd5189ede5796711",
        "af0004ffe4837a6ac2170deeaadaff0aaec33bc7eeb29fd66c76f851338c51d2",
        "ea1d8407a35e048f3684e85093ffde9ee60f5fc9de699971c0125a4b3cc5e51d");

    printf("\nP-384 (the curve of the intermediates above it)\n");
    suite(384,
        "d5a89e6fbfc63f595a198afe6e63801331ddff1f9a0b7a34a64446ab9d3ddc92"
        "ac82386c7fae9f0cd15b0dd083cf71ef64dbc04767fd13da54387bb72d4f56bb"
        "73ae9b93ea5f93286aa68c34f08b8a59ca15069d7bbaa53db8bea27fc0fe5ceb",
        "77e1f5d5a170761b8040064562a04ce566ba31e22abcfe4476094b2525af4d25"
        "eed3ee12274a9ba2a0cc92fe67448209",
        "4b4670d193b171aecc51dca82f6a6297a5e8490191d5ef10b66668ffeb563acc"
        "fa1c180266c941b233dec39554611e3c",
        "0707d4a777d9f8bc934cda6799b22d77b9d065d3d922bf4a46e259b30fc9a6fa"
        "9cda7668d785c6aa52b7dadce5f0a89d");

    /* ---- and now a certificate that was not made for this test -------------
     * The leaf of en.wikipedia.org's live chain, signed by Let's Encrypt's YE2
     * intermediate over P-384 with SHA-384. Public key, r, s and the SHA-384 of
     * the TBSCertificate, taken off the wire on 2026-08-19.
     *
     * This is the whole point of the file. The vectors above prove the maths
     * against an independent implementation; this proves it against a signature
     * produced years ago by a certificate authority that has never heard of
     * this kernel - which is the only kind of signature that will ever matter
     * to a browser. Embedded rather than fetched so the gate needs no network.
     * It will expire; when it does, replace it rather than delete it. */
    printf("\n a real certificate: en.wikipedia.org's leaf\n");
    {
        unsigned char pub[96], r[48], sg[48], h[48];
        unhex("719ab43391d6c410bdcca97b7774942db58738edac647434c7b9de533ea336cf"
              "9b920f97269345ab1e5597be796fa812ea881df8925bb121b98cc808aadff603"
              "e9ef34fef576e7d2bc5d021d8c55e5a1839ad26214ef57fb1d0d3d3ef603970f", pub, 96);
        unhex("4cdd35a15b9e47bc4aa8a9f7c7b5abcae410a989773a2bcb8f28a19ce9e9dc62"
              "69cda5caa0e5b0ea0620f11f8f824f00", r, 48);
        unhex("feb13447c1c93ad877f41b635f923dd4bf42535525deab9dad61662d3ffa618a"
              "2847e4fe11d201d32bf442e2c9e173a8", sg, 48);
        int hl = unhex("4d2552a120c1a9ac1b25327d09f6b66d2e5e94681cfc295431d07c574c96c5e2"
                       "177b5e5396872007f0bad2a2b9ff12aa", h, 48);
        ok("Let's Encrypt's signature on *.wikipedia.org verifies",
           ecdsa_verify(384, pub, r, sg, h, hl) == 1);
        unsigned char bad[48];
        memcpy(bad, h, (size_t)hl); bad[0] ^= 1;
        ok("...and one flipped bit of that certificate does not",
           ecdsa_verify(384, pub, r, sg, bad, hl) == 0);
    }

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
