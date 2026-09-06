/* tlsstatetest.c - the TLS 1.3 client state machine against crafted records,
 * reaching statics by #including tls.c. No network, no openssl.
 *
 * Every case here is one tlstest (a real handshake against openssl) cannot
 * see, because a well-behaved server never produces it:
 *
 *   1. tls_start keeps the caller's private key. A handshake succeeds with
 *      ANY scalar, so the openssl interop gate stayed green for the whole
 *      time tls_start's reset memset was wiping priv and every session ran
 *      on clamp(0), a public constant (found 2026-09-04).
 *   2. An encrypted record before ServerHello is refused, not "decrypted"
 *      under the all-zero key - under which a forged Finished verifies,
 *      because every input to it is public.
 *   3. Finished outside WAIT_FIN is refused.
 *   4. A handshake message split across two records is reassembled.
 *   5. Application data past the buffer is an error, not a silent drop.
 */
#include <stdio.h>
#include <string.h>
#include "tls.c"

static int passed, failed;
static void ok(const char *what, int cond)
{
    if (cond) { passed++; printf("  ok   %s\n", what); }
    else { failed++; printf("  FAIL %s\n", what); }
}

/* one encrypted record under key/iv/seq = 0 carrying `pt` (inner type last) */
static int zero_key_record(unsigned char *rec, const unsigned char *pt, int n)
{
    unsigned char zero[32] = {0}, nonce[12] = {0};
    rec[0] = 23; rec[1] = 3; rec[2] = 3; rec[3] = (unsigned char)((n + 16) >> 8); rec[4] = (unsigned char)(n + 16);
    memcpy(rec + 5, pt, n);
    aes128_gcm_encrypt(zero, nonce, rec, 5, rec + 5, (u32)n, rec + 5 + n);
    return 5 + n + 16;
}

int main(void)
{
    static struct tls_conn c;
    printf("tls.c state machine against crafted records\n\n");

    /* ---- 1. the private key survives tls_start --------------------------- */
    memset(&c, 0, sizeof c);
    unsigned char supplied[32];
    for (int i = 0; i < 32; i++) supplied[i] = c.priv[i] = (unsigned char)(0x11 * (i + 1) ^ 0xA5);
    tls_trust(&c, 0, 0, 0);
    tls_start(&c, "example.com");
    ok("tls_start keeps the caller's private key", memcmp(c.priv, supplied, 32) == 0);
    {
        unsigned char zero[32] = {0}, pub0[32];
        x25519_base(pub0, zero);
        ok("the public key is not x25519_base(0)", memcmp(c.pub, pub0, 32) != 0);
    }

    /* ---- 2. encrypted record before ServerHello -------------------------- */
    /* forge Finished under the zero handshake secret over the transcript so
     * far (just our ClientHello, which is on the wire in clear) */
    unsigned char zero[32] = {0}, fkey[32], th[32], vfy[32];
    tls13_expand_label(zero, "finished", zero, 0, fkey, 32);
    sha256(c.tx, (u32)c.txn, th);
    hmac_sha256(fkey, 32, th, 32, vfy);
    unsigned char pt[4 + 32 + 1];
    pt[0] = 20; pt[1] = 0; pt[2] = 0; pt[3] = 32;
    memcpy(pt + 4, vfy, 32);
    pt[36] = 22;
    unsigned char rec[5 + 64 + 16];
    int rl = zero_key_record(rec, pt, 37);
    ok("state is WAIT_SH after tls_start", c.state == TLS_WAIT_SH);
    tls_feed(&c, rec, rl);
    ok("an encrypted record before ServerHello is refused (verify=0)",
       c.state == TLS_ERROR && c.err == TLS_E_PROTOCOL);
    ok("...and the zero key was never used to decrypt it", c.s_seq == 0);

    memset(&c, 0, sizeof c);
    for (int i = 0; i < 32; i++) c.priv[i] = (unsigned char)i;
    c.verify = 1;
    tls_start(&c, "example.com");
    sha256(c.tx, (u32)c.txn, th);
    hmac_sha256(fkey, 32, th, 32, vfy);
    memcpy(pt + 4, vfy, 32);
    rl = zero_key_record(rec, pt, 37);
    tls_feed(&c, rec, rl);
    ok("an encrypted record before ServerHello is refused (verify=1)",
       c.state == TLS_ERROR && c.err == TLS_E_PROTOCOL);

    /* ---- 3. Finished outside WAIT_FIN ------------------------------------ */
    memset(&c, 0, sizeof c);
    c.state = TLS_READY;
    unsigned char fin[4 + 32] = { 20, 0, 0, 32 };
    int r = handle_handshake(&c, fin, sizeof fin);
    ok("a second Finished after READY is refused", r < 0 && c.err == TLS_E_PROTOCOL);

    /* ---- 4. a message split across two records --------------------------- */
    memset(&c, 0, sizeof c);
    c.state = TLS_WAIT_FIN;
    /* EncryptedExtensions(8), length 10, split 5 + 5 - fed through the record
     * path (inner type 22) so the carry buffer is what is under test */
    unsigned char ee1[] = { 8, 0, 0, 10, 1, 2, 3, 4, 5, 22 };
    unsigned char ee2[] = { 6, 7, 8, 9, 10, 22 };
    /* s_key/s_iv zero, saw_sh must be set for the encrypted path */
    c.saw_sh = 1;
    rl = zero_key_record(rec, ee1, sizeof ee1);
    tls_feed(&c, rec, rl);
    ok("the head of a split message is carried, not dropped", c.hsn == 9 && c.txn == 0);
    /* second record: s_seq advanced, so the nonce is 1 */
    {
        unsigned char zk[32] = {0}, nonce[12] = {0};
        nonce[11] = 1;
        rec[0] = 23; rec[1] = 3; rec[2] = 3; rec[3] = 0; rec[4] = (unsigned char)(sizeof ee2 + 16);
        memcpy(rec + 5, ee2, sizeof ee2);
        aes128_gcm_encrypt(zk, nonce, rec, 5, rec + 5, sizeof ee2, rec + 5 + sizeof ee2);
        tls_feed(&c, rec, 5 + (int)sizeof ee2 + 16);
    }
    ok("the message is reassembled across records", c.txn == 14 && c.hsn == 0 && c.state == TLS_WAIT_FIN);

    /* ---- 5. application data overflow is an error ------------------------ */
    memset(&c, 0, sizeof c);
    c.state = TLS_READY; c.saw_sh = 1;
    c.appn = TLS_REC_MAX - 2;
    unsigned char app[] = { 'h', 'i', '!', 23 };
    rl = zero_key_record(rec, app, sizeof app);
    tls_feed(&c, rec, rl);
    ok("application data past the buffer is TLS_E_OVERFLOW, not a silent drop",
       c.state == TLS_ERROR && c.err == TLS_E_OVERFLOW);

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
