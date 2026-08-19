/* crypto.h - the primitives, declared once.
 *
 * crypto.c had no header because its only consumer was a host test that
 * #included the .c. The kernel needs one, and a header is also the place to
 * say what these are FOR: WPA2 (PBKDF2, HMAC-SHA1, AES-CMAC), Bluetooth SSP
 * (HMAC-SHA256, CMAC) and TLS 1.3 (SHA-256, HMAC, AES-GCM, X25519, HKDF).
 *
 * Every function here is checked against a published vector by
 * hosttest/cryptotest.c and hosttest/tlscryptotest.c.
 */
#ifndef ZL_CRYPTO_H
#define ZL_CRYPTO_H

typedef unsigned char      cu8;
typedef unsigned int       cu32;

/* constant-time compare - every MAC comparison must come through here */
int  crypto_equal(const cu8 *a, const cu8 *b, cu32 n);

void sha256(const cu8 *data, cu32 n, cu8 *out);
/* Real certificate chains need these: en.wikipedia.org's four certificates are
 * all ecdsa-with-SHA384. SHA-384 is SHA-512 with different initial values and
 * a truncated output, not a separate algorithm. */
void sha384(const cu8 *data, cu32 n, cu8 *out);
void sha512(const cu8 *data, cu32 n, cu8 *out);
void hmac_sha256(const cu8 *key, cu32 klen, const cu8 *data, cu32 dlen, cu8 *out);

void aes128_expand(const cu8 *key, cu8 *rk);
void aes128_encrypt(const cu8 *rk, const cu8 *in, cu8 *out);

/* AES-128-GCM with a 96-bit IV, which is the only length TLS 1.3 uses.
 * `data` is transformed IN PLACE. decrypt returns 1 when the tag verified and
 * 0 when it did not - and when it returns 0 the buffer is untouched, because
 * releasing unauthenticated plaintext is the attack AEAD exists to stop. */
void aes128_gcm_encrypt(const cu8 *key, const cu8 *iv12,
                        const cu8 *aad, cu32 alen,
                        cu8 *data, cu32 len, cu8 *tag);
int  aes128_gcm_decrypt(const cu8 *key, const cu8 *iv12,
                        const cu8 *aad, cu32 alen,
                        cu8 *data, cu32 len, const cu8 *tag);

/* X25519. `out`, `scalar` and `point` are 32 bytes each. */
void x25519(cu8 *out, const cu8 *scalar, const cu8 *point);
void x25519_base(cu8 *out, const cu8 *scalar);

void hkdf_extract(const cu8 *salt, cu32 slen, const cu8 *ikm, cu32 ilen, cu8 *prk);
void hkdf_expand(const cu8 *prk, const cu8 *info, cu32 ilen, cu8 *out, cu32 olen);
void tls13_expand_label(const cu8 *secret, const char *label,
                        const cu8 *ctx, cu32 clen, cu8 *out, cu32 olen);
void tls13_derive_secret(const cu8 *secret, const char *label,
                         const cu8 *thash, cu8 *out);

#endif
