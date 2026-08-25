/* tlstest.c - complete a real TLS 1.3 handshake against OpenSSL.
 *
 * WHY AGAINST A SERVER AND NOT A VECTOR. Every primitive underneath this is
 * already checked against published constants by tlscryptotest.c. What that
 * cannot check is the hundred small ways a handshake goes wrong: a length
 * written little-endian, an extension in the wrong order, a transcript hash
 * taken one message too late, a nonce that does not advance, the record header
 * omitted from the additional data. Each of those produces a handshake that
 * fails, and none of them produce a wrong constant anywhere.
 *
 * So this speaks to `openssl s_server`. If OpenSSL completes the handshake,
 * decrypts what we send and we decrypt its reply, then every one of those
 * details is right - and interoperability is the only property that actually
 * matters for a client whose job is to reach somebody else's server.
 *
 * NO KERNEL AND NO NETWORK STACK. tls.c holds no socket - bytes go in through
 * tls_feed and out through tls_take - so the whole protocol runs here over an
 * ordinary TCP socket to a local process.
 *
 * Skips (rather than fails) when openssl is not installed, because a gate that
 * fails for a missing tool teaches people to ignore it.
 *
 *   cd kernel/tests/host && ./build.sh && ./tlstest
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../../src/net/tls.h"
#include "../../src/net/x509.h"
const struct x509_cert *zl_roots(int *n);

static int passed, failed;
static void ok(const char *what, int cond)
{
    if (cond) { passed++; printf("  ok   %s\n", what); }
    else { failed++; printf("  FAIL %s\n", what); }
}

static int have(const char *cmd)
{
    char buf[256];
    snprintf(buf, sizeof buf, "command -v %s >/dev/null 2>&1", cmd);
    return system(buf) == 0;
}

/* a self-signed cert, made once into /tmp - we do not verify it (tls.c says so
 * out loud), but a server will not start without one */
static int make_cert(const char *dir)
{
    char cmd[512];
    snprintf(cmd, sizeof cmd,
             "openssl req -x509 -newkey rsa:2048 -keyout %s/k.pem -out %s/c.pem "
             "-days 1 -nodes -subj /CN=localhost >/dev/null 2>&1", dir, dir);
    return system(cmd) == 0;
}

int main(void)
{
    printf("tlstest: a real TLS 1.3 handshake\n\n");

    if (!have("openssl")) {
        printf("  skip  openssl is not installed - nothing to talk to\n");
        printf("\n0 passed, 0 failed (skipped)\n");
        return 0;
    }

    char dir[] = "/tmp/zlos-tls-XXXXXX";
    if (!mkdtemp(dir)) { printf("  FAIL mkdtemp\n"); return 1; }
    if (!make_cert(dir)) { printf("  FAIL could not make a test certificate\n"); return 1; }

    /* an ephemeral port, chosen by the kernel then handed to s_server */
    int port = 0;
    {
        int probe = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in a;
        memset(&a, 0, sizeof a);
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = 0;
        bind(probe, (struct sockaddr *)&a, sizeof a);
        socklen_t sl = sizeof a;
        getsockname(probe, (struct sockaddr *)&a, &sl);
        port = ntohs(a.sin_port);
        close(probe);
    }

    char cmd[768];
    snprintf(cmd, sizeof cmd,
             "openssl s_server -accept %d -cert %s/c.pem -key %s/k.pem "
             "-tls1_3 -ciphersuites TLS_AES_128_GCM_SHA256 -groups X25519 "
             "-www -quiet >/dev/null 2>&1 & echo $!",
             port, dir, dir);
    FILE *f = popen(cmd, "r");
    int pid = 0;
    if (f) { if (fscanf(f, "%d", &pid) != 1) pid = 0; pclose(f); }
    if (pid <= 0) { printf("  FAIL could not start s_server\n"); return 1; }

    /* wait for the port, by connecting - never a fixed sleep */
    int s = -1;
    for (int tries = 0; tries < 200; tries++) {
        s = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in a;
        memset(&a, 0, sizeof a);
        a.sin_family = AF_INET;
        a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        a.sin_port = htons((unsigned short)port);
        if (connect(s, (struct sockaddr *)&a, sizeof a) == 0) break;
        close(s); s = -1;
        struct timespec ts = { 0, 20 * 1000 * 1000 };
        nanosleep(&ts, 0);
    }
    if (s < 0) { printf("  FAIL s_server never accepted\n"); kill(pid, SIGKILL); return 1; }

    static struct tls_conn c;
    /* The ephemeral private key. tls.c does not generate one - there is no RNG
     * in the kernel - so the caller supplies it, and the harness reads real
     * entropy so this is a genuine ephemeral exchange rather than a fixed key. */
    {
        int u = open("/dev/urandom", O_RDONLY);
        if (u >= 0) { if (read(u, c.priv, 32) != 32) {} close(u); }
    }
    tls_start(&c, "localhost");
    ok("ClientHello was produced", c.outn > 0);

    unsigned char rx[8192];
    int guard = 0;
    while (tls_state(&c) != TLS_READY && tls_state(&c) != TLS_ERROR && guard++ < 400) {
        const unsigned char *p;
        int n = tls_take(&c, &p);
        if (n > 0) {
            int w = (int)write(s, p, (size_t)n);
            if (w > 0) tls_sent(&c, w);
        }
        if (tls_state(&c) == TLS_READY) break;
        struct timeval tv = { 2, 0 };
        fd_set r;
        FD_ZERO(&r); FD_SET(s, &r);
        if (select(s + 1, &r, 0, 0, &tv) <= 0) break;
        int got = (int)read(s, rx, sizeof rx);
        if (got <= 0) break;
        if (tls_feed(&c, rx, got) < 0) break;
    }

    ok("the ServerHello was parsed and a shared secret derived", c.saw_sh);
    ok("the server's Finished verified", c.saw_fin);
    ok("the handshake completed", tls_state(&c) == TLS_READY);
    if (tls_state(&c) != TLS_READY)
        printf("       state=%d err=%d\n", tls_state(&c), tls_error(&c));

    /* ...and application data flows both ways. -www makes s_server answer any
     * request with an HTTP page, so a real GET proves encrypt AND decrypt. */
    if (tls_state(&c) == TLS_READY) {
        const char *req = "GET / HTTP/1.0\r\n\r\n";
        tls_write(&c, (const unsigned char *)req, (int)strlen(req));
        const unsigned char *p;
        int n = tls_take(&c, &p);
        if (n > 0) { int w = (int)write(s, p, (size_t)n); if (w > 0) tls_sent(&c, w); }

        int total = 0;
        char body[16384];
        for (int i = 0; i < 200 && total < (int)sizeof body - 1; i++) {
            struct timeval tv = { 2, 0 };
            fd_set r;
            FD_ZERO(&r); FD_SET(s, &r);
            if (select(s + 1, &r, 0, 0, &tv) <= 0) break;
            int got = (int)read(s, rx, sizeof rx);
            if (got <= 0) break;
            if (tls_feed(&c, rx, got) < 0) break;
            unsigned char tmp[4096];
            int m;
            while ((m = tls_read(&c, tmp, sizeof tmp)) > 0 && total < (int)sizeof body - 1) {
                int room = (int)sizeof body - 1 - total;
                if (m > room) m = room;
                memcpy(body + total, tmp, (size_t)m);
                total += m;
            }
        }
        body[total > 0 ? total : 0] = 0;
        ok("the server decrypted our request and replied", total > 0);
        ok("the reply decrypts to an HTTP response",
           total > 0 && !memcmp(body, "HTTP/", 5));
        if (total > 0) {
            char first[80];
            int k = 0;
            while (k < 70 && body[k] && body[k] != '\r' && body[k] != '\n') { first[k] = body[k]; k++; }
            first[k] = 0;
            printf("       server said: %s\n", first);
        }
    }

    close(s);
    kill(pid, SIGKILL);
    waitpid(pid, 0, 0);
    {
        char rm[256];
        snprintf(rm, sizeof rm, "rm -rf %s", dir);
        if (system(rm) != 0) {}
    }

    /* ---- and now the real internet ------------------------------------
     * The loopback test proves interop with OpenSSL. This proves it against a
     * production server that has never heard of us: a different TLS stack, a
     * real certificate chain, SNI actually mattering because the address is
     * shared, and a response far larger than one record.
     *
     * OPT-IN, because a gate that needs the network is a gate that fails on a
     * train. ZLOS_NET_TESTS=1 turns it on; without it this says so and stops,
     * which is a skip rather than a pass. */
    if (!getenv("ZLOS_NET_TESTS")) {
        printf("\n  skip  the live-internet check (set ZLOS_NET_TESTS=1 to run it)\n");
    } else {
        printf("\n=== against the real internet ===\n");
        struct addrinfo hints, *res = 0;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo("en.wikipedia.org", "443", &hints, &res) == 0 && res) {
            int w = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(w, res->ai_addr, res->ai_addrlen) == 0) {
                static struct tls_conn wc;
                int u = open("/dev/urandom", O_RDONLY);
                if (u >= 0) { if (read(u, wc.priv, 32) != 32) {} close(u); }
                tls_start(&wc, "en.wikipedia.org");
                int g2 = 0;
                while (tls_state(&wc) != TLS_READY && tls_state(&wc) != TLS_ERROR && g2++ < 600) {
                    const unsigned char *q;
                    int n2 = tls_take(&wc, &q);
                    if (n2 > 0) { int k = (int)write(w, q, (size_t)n2); if (k > 0) tls_sent(&wc, k); }
                    if (tls_state(&wc) == TLS_READY) break;
                    int got = (int)read(w, rx, sizeof rx);
                    if (got <= 0) break;
                    if (tls_feed(&wc, rx, got) < 0) break;
                }
                ok("handshake with en.wikipedia.org completed",
                   tls_state(&wc) == TLS_READY);
                if (tls_state(&wc) == TLS_READY) {
                    char rq[256];
                    int rn = snprintf(rq, sizeof rq,
                        "GET /wiki/Linux HTTP/1.1\r\nHost: en.wikipedia.org\r\n"
                        "Connection: close\r\nUser-Agent: zlOS\r\n\r\n");
                    tls_write(&wc, (const unsigned char *)rq, rn);
                    const unsigned char *q;
                    int n2 = tls_take(&wc, &q);
                    if (n2 > 0) { int k = (int)write(w, q, (size_t)n2); if (k > 0) tls_sent(&wc, k); }
                    long total = 0;
                    char head[64];
                    int hn = 0;
                    for (int i = 0; i < 6000; i++) {
                        int got = (int)read(w, rx, sizeof rx);
                        if (got <= 0) break;
                        if (tls_feed(&wc, rx, got) < 0) break;
                        unsigned char tmp[16384];
                        int m;
                        while ((m = tls_read(&wc, tmp, sizeof tmp)) > 0) {
                            for (int k = 0; k < m && hn < (int)sizeof head - 1; k++) head[hn++] = (char)tmp[k];
                            total += m;
                        }
                    }
                    head[hn] = 0;
                    ok("the article decrypted to an HTTP 200",
                       !memcmp(head, "HTTP/1.1 200", 12));
                    ok("and it is a whole article, not one record",
                       total > 100000);
                    printf("       %ld bytes decrypted from Wikimedia\n", total);
                }
            }
            close(w);
            freeaddrinfo(res);
        }

        /* ---- and the same handshake, VERIFIED ---------------------------
         * The check above proves the bytes flow. This proves they flow to the
         * right server: the chain is validated to a real ISRG root and
         * CertificateVerify is checked, so an impostor replaying Wikipedia's
         * (entirely public) certificate chain without its private key is
         * rejected. Both directions are asserted, because a verifier that
         * accepts everything and one that accepts nothing each pass half. */
        printf("\n=== a VERIFIED handshake ===\n");
        {
            int nr;
            const struct x509_cert *rt = zl_roots(&nr);
            ok("the trust store parsed at least one root", nr >= 1);

            struct addrinfo h2, *r2 = 0;
            memset(&h2, 0, sizeof h2);
            h2.ai_family = AF_INET; h2.ai_socktype = SOCK_STREAM;
            if (getaddrinfo("en.wikipedia.org", "443", &h2, &r2) == 0 && r2) {
                int w = socket(AF_INET, SOCK_STREAM, 0);
                if (connect(w, r2->ai_addr, r2->ai_addrlen) == 0) {
                    static struct tls_conn vc;
                    int u = open("/dev/urandom", O_RDONLY);
                    if (u >= 0) { if (read(u, vc.priv, 32) != 32) {} close(u); }
                    tls_trust(&vc, rt, nr, "20260819000000Z");
                    tls_start(&vc, "en.wikipedia.org");
                    int g3 = 0;
                    while (tls_state(&vc) != TLS_READY && tls_state(&vc) != TLS_ERROR && g3++ < 600) {
                        const unsigned char *q;
                        int n3 = tls_take(&vc, &q);
                        if (n3 > 0) { int k = (int)write(w, q, (size_t)n3); if (k > 0) tls_sent(&vc, k); }
                        if (tls_state(&vc) == TLS_READY) break;
                        int got = (int)read(w, rx, sizeof rx);
                        if (got <= 0) break;
                        if (tls_feed(&vc, rx, got) < 0) break;
                    }
                    ok("the chain validated to a trusted root", vc.cert_ok == 1);
                    ok("CertificateVerify proved the server holds the key", vc.saw_cv == 1);
                    ok("the verified handshake completed", tls_state(&vc) == TLS_READY);
                    /* A DIAGNOSTIC THAT LIES ON THE SUCCESS PATH is worse than
                     * none, because nobody doubts it. x509_chain_ok tries every
                     * root and each miss writes a reason; a later root
                     * succeeding used to leave the last miss's complaint in
                     * place, so a perfectly verified handshake reported "RSA
                     * signature but the issuer key is not RSA". browser.c puts
                     * this string on the screen. */
                    if (tls_state(&vc) == TLS_READY)
                        ok("a verified chain leaves NO stale reason behind",
                           x509_why()[0] == 0);
                    if (tls_state(&vc) != TLS_READY)
                        printf("       err=%d why=%s\n", tls_error(&vc), x509_why());
                }
                close(w);
                freeaddrinfo(r2);
            }

            /* THE REFUSAL HALF, and it used to pick www.google.com as "a CA we
             * do not carry". That was true when this was written and stopped
             * being true the moment GTS Root R1 was added to roots.c - which
             * the project announced as a feature. The assertion then failed on
             * a correctly VERIFIED handshake, and it went unnoticed because
             * this whole half only runs under ZLOS_NET_TESTS=1.
             *
             * A test whose premise is "some third party still uses a CA we
             * chose not to carry" is a test that decays on somebody else's
             * schedule. So the store is narrowed instead of the host changed:
             * connect to a real site and offer a trust store that deliberately
             * EXCLUDES its root. The chain is genuine, the refusal is genuine,
             * and nothing about it depends on what the internet does next.
             *
             * `nr - 1` drops the last root, which roots.c orders as GTS - the
             * one Google chains to. If that order ever changes this assertion
             * fails loudly rather than passing for the wrong reason. */
            struct addrinfo h3, *r3 = 0;
            memset(&h3, 0, sizeof h3);
            h3.ai_family = AF_INET; h3.ai_socktype = SOCK_STREAM;
            if (getaddrinfo("www.google.com", "443", &h3, &r3) == 0 && r3) {
                int w = socket(AF_INET, SOCK_STREAM, 0);
                if (connect(w, r3->ai_addr, r3->ai_addrlen) == 0) {
                    static struct tls_conn bc;
                    int u = open("/dev/urandom", O_RDONLY);
                    if (u >= 0) { if (read(u, bc.priv, 32) != 32) {} close(u); }
                    tls_trust(&bc, rt, nr - 1, "20260819000000Z");
                    tls_start(&bc, "www.google.com");
                    int g4 = 0;
                    while (tls_state(&bc) != TLS_READY && tls_state(&bc) != TLS_ERROR && g4++ < 600) {
                        const unsigned char *q;
                        int n4 = tls_take(&bc, &q);
                        if (n4 > 0) { int k = (int)write(w, q, (size_t)n4); if (k > 0) tls_sent(&bc, k); }
                        if (tls_state(&bc) == TLS_READY) break;
                        int got = (int)read(w, rx, sizeof rx);
                        if (got <= 0) break;
                        if (tls_feed(&bc, rx, got) < 0) break;
                    }
                    ok("a chain to a CA we do not carry is REFUSED",
                       tls_state(&bc) == TLS_ERROR && bc.cert_ok == 0);
                    /* AND FOR THE RIGHT REASON. "it was refused" passes just
                     * as well when the refusal came from a parse error or a
                     * clock problem, which are different bugs with different
                     * fixes - the same argument this file makes about the
                     * accept case. */
                    ok("...and the reason names the trust store",
                       !strcmp(x509_why(), "chain does not reach a trusted root"));
                    printf("       refused with: %s\n", x509_why());
                }
                close(w);
                freeaddrinfo(r3);
            }
        }
    }

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
