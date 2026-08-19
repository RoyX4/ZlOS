/* tls.h - TLS 1.3 client, one ciphersuite.
 *
 * SHAPED LIKE http.c SITS ON tcp.c. This holds no socket: bytes arrive through
 * tls_feed() and leave through tls_take(), so the whole protocol runs in a host
 * harness against a real server over a plain Unix socket, with no kernel and no
 * network stack linked. That is what let it be gated against OpenSSL rather
 * than against my own idea of what OpenSSL would say.
 *
 * TLS_AES_128_GCM_SHA256 AND X25519, and nothing else is offered. A second
 * ciphersuite is not more security, it is a negotiation to get wrong.
 *
 * WHAT THIS DOES NOT DO, stated here because the whole point of TLS is a claim
 * about who you are talking to:
 *
 *   IT DOES NOT VERIFY THE SERVER'S CERTIFICATE.
 *
 * The handshake completes, the traffic is encrypted, and an attacker who can
 * intercept the connection can present any certificate at all and be believed.
 * That is confidentiality against a passive eavesdropper and NOTHING against
 * an active one. browser.c must not show a padlock, must not call this
 * "secure", and must say on screen what it is. BROWSER-PROMPT.md §5 forbids a
 * half-TLS that pretends otherwise, and this header exists partly to make that
 * impossible to forget: x509.c is the other half and is not written yet.
 */
#ifndef ZL_TLS_H
#define ZL_TLS_H

typedef unsigned char tu8;

#define TLS_HS_MAX   16384   /* one handshake message we will reassemble */
#define TLS_REC_MAX  16640   /* a record: 16384 payload + expansion       */
#define TLS_OUT_MAX  4096
#define TLS_HOST_MAX 128

enum {
    TLS_START = 0,       /* nothing sent yet                       */
    TLS_WAIT_SH,         /* ClientHello sent, waiting on ServerHello */
    TLS_WAIT_FIN,        /* reading encrypted handshake            */
    TLS_READY,           /* handshake done, application data flows  */
    TLS_CLOSED,
    TLS_ERROR
};

struct tls_conn {
    int state;
    int err;                        /* a TLS_E_* below                 */
    char host[TLS_HOST_MAX];

    tu8 priv[32], pub[32], shared[32];

    /* the running transcript hash, which binds every key to this exact
     * handshake. Kept as the raw message bytes because the hash must be taken
     * at several intermediate points. */
    tu8 tx[TLS_HS_MAX];
    int txn;

    tu8 hs_secret[32], master[32];
    tu8 c_hs[32], s_hs[32];         /* handshake traffic secrets        */
    tu8 c_ap[32], s_ap[32];         /* application traffic secrets      */
    tu8 c_key[16], c_iv[12];
    tu8 s_key[16], s_iv[12];
    unsigned long long c_seq, s_seq;

    /* the byte stream in both directions */
    tu8 in[TLS_REC_MAX];
    int inn;
    tu8 out[TLS_OUT_MAX];
    int outn;

    /* decrypted application data waiting for the caller */
    tu8 app[TLS_REC_MAX];
    int appn, appr;

    int saw_sh, saw_fin;
};

#define TLS_E_NONE      0
#define TLS_E_VERSION   1     /* the peer is not TLS 1.3                */
#define TLS_E_SUITE     2     /* it picked a suite we did not offer     */
#define TLS_E_GROUP     3     /* no x25519 key share came back          */
#define TLS_E_DECRYPT   4     /* a record failed its tag - fatal        */
#define TLS_E_FINISHED  5     /* the server's Finished did not verify   */
#define TLS_E_ALERT     6     /* the peer sent a fatal alert            */
#define TLS_E_OVERFLOW  7     /* a message larger than we will hold     */
#define TLS_E_PROTOCOL  8     /* malformed                              */

/* Begin a handshake with `host` (used for SNI, which Wikipedia and every
 * other shared-IP server requires). After this, tls_take() has a ClientHello. */
void tls_start(struct tls_conn *c, const char *host);

/* Hand it bytes that arrived from the peer. Returns how many it consumed, or
 * -1 on a fatal error (see c->err). */
int  tls_feed(struct tls_conn *c, const tu8 *data, int len);

/* Bytes that must be sent to the peer. Returns the count and points *p at
 * them; call tls_sent() once they are gone. */
int  tls_take(struct tls_conn *c, const tu8 **p);
void tls_sent(struct tls_conn *c, int n);

/* Application data, once state is TLS_READY. */
int  tls_write(struct tls_conn *c, const tu8 *data, int len);
int  tls_read(struct tls_conn *c, tu8 *out, int max);

int  tls_state(struct tls_conn *c);
int  tls_error(struct tls_conn *c);

#endif
