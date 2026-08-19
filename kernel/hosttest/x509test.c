/* x509test.c - certificate parsing and chain validation.
 *
 * THIS IS THE FILE THAT DECIDES WHETHER HTTPS MEANS ANYTHING, so it is written
 * the way ecdsatest.c is: a validator that returns 1 unconditionally passes
 * every "the real chain is accepted" test ever written, and it is the bug that
 * makes every forged certificate valid. So the accept case is checked once and
 * then every way of being wrong is checked individually.
 *
 * The certificates are en.wikipedia.org's REAL chain, captured off the wire.
 * Nothing here was generated for the test - a parser that only meets
 * certificates made by its own author agrees with its author, not with the
 * authorities a browser will actually encounter.
 *
 * No kernel, no boot, no network.
 */
#include <stdio.h>
#include <string.h>
#include "../x509.h"
#include "chain_der.h"

static int passed, failed;

static void ok(const char *what, int cond)
{
    if (cond) { passed++; printf("  ok   %s\n", what); }
    else { failed++; printf("  FAIL %s   (%s)\n", what, x509_why()); }
}

/* the chain as TLS sends it: leaf first */
static const unsigned char *DERS[4] = { CERT0, CERT1, CERT2, CERT3 };
static const int LENS[4] = { (int)sizeof CERT0, (int)sizeof CERT1,
                             (int)sizeof CERT2, (int)sizeof CERT3 };

/* the trust store: ISRG Root X2, the top of the captured chain. In a browser
 * this is a compiled-in list; here it is one root, which is enough to prove
 * the anchor is CONSULTED rather than assumed. */
static struct x509_cert roots[1];

int main(void)
{
    printf("x509test: parsing and chain validation\n\n1. parsing a real certificate\n");

    struct x509_cert leaf, inter, root;
    ok("the leaf parses", x509_parse(CERT0, LENS[0], &leaf));
    ok("the intermediate parses", x509_parse(CERT1, LENS[1], &inter));
    ok("the root parses", x509_parse(CERT3, LENS[3], &root));

    ok("the leaf's key is P-256", leaf.curve_bits == 256);
    ok("the intermediate's key is P-384", inter.curve_bits == 384);
    ok("the leaf is signed with SHA-384", leaf.sig_hash == 384);
    ok("the leaf has a subjectAltName", leaf.san != 0 && leaf.sanlen > 0);
    ok("the leaf is NOT a CA", leaf.is_ca == 0);
    ok("the intermediate IS a CA", inter.is_ca == 1);
    ok("validity dates decoded", leaf.not_before[0] == '2' && leaf.not_after[0] == '2');
    printf("       leaf valid %s .. %s\n", leaf.not_before, leaf.not_after);

    printf("\n2. host name matching\n");
    ok("*.wikipedia.org matches en.wikipedia.org",
       x509_host_matches(&leaf, "en.wikipedia.org"));
    ok("...and matches www.wikipedia.org",
       x509_host_matches(&leaf, "www.wikipedia.org"));
    ok("a wildcard does NOT match a deeper name",
       !x509_host_matches(&leaf, "a.b.wikipedia.org"));
    /* The bare domain matches - but NOT via the wildcard. Wikimedia lists
     * `wikipedia.org` as its own dNSName alongside `*.wikipedia.org`, which is
     * exactly why a certificate needs both. My first version of this assertion
     * expected a refusal and was simply wrong about the certificate. */
    ok("the bare domain matches, because it is its own SAN entry",
       x509_host_matches(&leaf, "wikipedia.org"));
    ok("a different domain is refused",
       !x509_host_matches(&leaf, "en.wikipedia.org.evil.com"));
    ok("a lookalike domain is refused",
       !x509_host_matches(&leaf, "en.wikipedia.com"));
    ok("matching is case-insensitive",
       x509_host_matches(&leaf, "EN.WIKIPEDIA.ORG"));

    printf("\n3. signatures over the real chain\n");
    ok("the leaf was signed by the intermediate", x509_signed_by(&leaf, &inter));
    {
        struct x509_cert i2, r2;
        x509_parse(CERT2, LENS[2], &i2);
        ok("the intermediate was signed by the one above it",
           x509_signed_by(&inter, &i2));
        x509_parse(CERT3, LENS[3], &r2);
        ok("...and that one by the root", x509_signed_by(&i2, &r2));
    }
    ok("the leaf was NOT signed by the root directly",
       !x509_signed_by(&leaf, &root));

    printf("\n4. the whole chain\n");
    x509_parse(CERT3, LENS[3], &roots[0]);
    /* a date inside the leaf's window, taken from the leaf itself so this gate
     * does not start failing on a calendar boundary */
    char now[16];
    memcpy(now, leaf.not_before, 15);
    now[13] = '1';                          /* one second later, still inside */
    ok("the real chain validates against a trusted root",
       x509_chain_ok(DERS, LENS, 4, roots, 1, "en.wikipedia.org", now));

    printf("\n5. every way of being wrong\n");
    ok("the wrong host is refused",
       !x509_chain_ok(DERS, LENS, 4, roots, 1, "evil.example.com", now));

    ok("an EMPTY trust store refuses the chain",
       !x509_chain_ok(DERS, LENS, 4, roots, 0, "en.wikipedia.org", now));

    {
        /* the attack this whole file exists to stop: a self-consistent chain
         * whose root nobody trusts. Anchoring on "the last certificate" instead
         * of on the store accepts it. */
        struct x509_cert other[1];
        x509_parse(CERT1, LENS[1], &other[0]);   /* an intermediate, not a root */
        ok("a chain whose top is not in the store is refused",
           !x509_chain_ok(DERS, LENS, 4, other, 1, "en.wikipedia.org", now));
    }

    {
        /* one flipped bit anywhere in the signed body must break the link */
        static unsigned char tampered[sizeof CERT0];
        memcpy(tampered, CERT0, sizeof CERT0);
        tampered[200] ^= 0x01;
        const unsigned char *d2[4] = { tampered, CERT1, CERT2, CERT3 };
        ok("one flipped bit in the leaf is refused",
           !x509_chain_ok(d2, LENS, 4, roots, 1, "en.wikipedia.org", now));
    }

    {
        /* dropping the intermediate leaves a gap the chain cannot bridge */
        const unsigned char *d3[3] = { CERT0, CERT2, CERT3 };
        int l3[3] = { LENS[0], LENS[2], LENS[3] };
        ok("a chain with a missing link is refused",
           !x509_chain_ok(d3, l3, 3, roots, 1, "en.wikipedia.org", now));
    }

    {
        /* expiry, in both directions */
        ok("a date after notAfter is refused",
           !x509_chain_ok(DERS, LENS, 4, roots, 1, "en.wikipedia.org", "20991231235959Z"));
        ok("a date before notBefore is refused",
           !x509_chain_ok(DERS, LENS, 4, roots, 1, "en.wikipedia.org", "20000101000000Z"));
    }

    printf("\n6. malformed input must not fault\n");
    {
        struct x509_cert junk;
        ok("an empty buffer is refused", !x509_parse((const unsigned char *)"", 0, &junk));
        ok("random bytes are refused",
           !x509_parse((const unsigned char *)"\x30\x82\xff\xff not a cert", 16, &junk));
        /* every truncation of a real certificate: none may read past the end */
        int faults = 0;
        for (int cut = 1; cut < LENS[0]; cut += 7) {
            struct x509_cert t;
            x509_parse(CERT0, cut, &t);      /* must return, whatever it decides */
            faults++;
        }
        ok("every truncation of a real certificate returns safely", faults > 100);
        /* and a length field claiming more than exists */
        static unsigned char lying[64];
        memcpy(lying, CERT0, 64);
        lying[2] = 0xff; lying[3] = 0xff;
        ok("a length longer than the buffer is refused",
           !x509_parse(lying, 64, &junk));
    }

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
