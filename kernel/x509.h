/* x509.h - certificate parsing and chain validation.
 *
 * This is the half of TLS that makes it mean something. ecdsa.c can check a
 * signature; this decides WHOSE signature it was, whether that someone was
 * allowed to sign it, whether the name on the certificate is the name we asked
 * for, and whether it has expired. Any one of those missing turns "encrypted"
 * into "encrypted to somebody, possibly not them".
 *
 * NO HEAP. A parsed certificate holds POINTERS INTO the DER it was parsed
 * from, so the caller must keep that buffer alive - which tls.c does anyway,
 * because the chain arrives in its receive buffer.
 *
 * WHAT IT SUPPORTS: ECDSA over P-256 and P-384, signed with SHA-256 or
 * SHA-384. That is measured rather than chosen - it is what Let's Encrypt
 * issues and what en.wikipedia.org's whole chain uses. An RSA certificate is
 * REFUSED rather than skipped: a chain we cannot check is a chain we do not
 * trust, and silently accepting one would be the exact failure this file
 * exists to prevent.
 */
#ifndef ZL_X509_H
#define ZL_X509_H

typedef unsigned char xu8;

#define X509_MAX_CHAIN 8
#define X509_TIME_LEN  16      /* YYYYMMDDHHMMSSZ */

struct x509_cert {
    const xu8 *der;  int derlen;
    const xu8 *tbs;  int tbslen;     /* exactly the bytes that were signed */

    const xu8 *issuer;  int issuerlen;   /* raw DER of the Name, for matching */
    const xu8 *subject; int subjectlen;

    const xu8 *pubkey; int pubkeylen;    /* the EC point, x||y, no 04 prefix  */
    int curve_bits;                      /* 256 or 384; 0 = not ECDSA        */

    int sig_hash;                        /* 256 or 384; 0 = unsupported alg  */
    xu8 sig_r[48], sig_s[48];
    int sig_size;                        /* bytes per component              */

    const xu8 *san; int sanlen;          /* SubjectAltName extension value   */
    char not_before[X509_TIME_LEN];
    char not_after[X509_TIME_LEN];
    int is_ca;
    int has_bc;                          /* basicConstraints was present     */
};

/* Returns 1 on success. A certificate this cannot fully understand fails here
 * rather than being returned half-parsed. */
int x509_parse(const xu8 *der, int len, struct x509_cert *c);

/* Was `child` signed by `issuer`'s key? Returns 1 only if the signature
 * verifies AND the algorithms are ones we support. */
int x509_signed_by(const struct x509_cert *child, const struct x509_cert *issuer);

/* Does this certificate carry `host`? Checks subjectAltName dNSName entries,
 * including a leading wildcard. Returns 1 on a match. */
int x509_host_matches(const struct x509_cert *c, const char *host);

/* Is `nowZ` (YYYYMMDDHHMMSSZ) inside the validity window? */
int x509_time_ok(const struct x509_cert *c, const char *nowZ);

/* The whole job: parse `n` DER certificates as a chain (leaf first, as TLS
 * sends them), check every link, check the top is signed by a trusted root,
 * check the host and the clock. Returns 1 only if all of it holds.
 *
 * `roots` is the trust store. `nowZ` may be null to skip the clock check -
 * which the caller must only do knowingly, and tls.c says so where it does. */
int x509_chain_ok(const xu8 *const *ders, const int *lens, int n,
                  const struct x509_cert *roots, int nroots,
                  const char *host, const char *nowZ);

const char *x509_why(void);   /* why the last chain_ok failed */

#endif
