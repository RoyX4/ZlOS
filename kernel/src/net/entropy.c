/* entropy.c - where a TLS private key comes from.
 *
 * THIS IS THE MOST SECURITY-CRITICAL FILE IN THE BROWSER AND IT IS ALSO THE
 * SHORTEST, which is exactly why it needs the longest comment.
 *
 * An ephemeral X25519 key that an attacker can predict makes the whole
 * handshake readable: the certificate still verifies, the padlock would still
 * be earned, and every byte is still decryptable by anyone who can guess the
 * scalar. A weak RNG does not fail loudly. It produces a session that looks
 * perfect from both ends and is transparent to a third party. So this file
 * REPORTS ITS QUALITY and the browser shows it, rather than pretending.
 *
 * THREE TIERS, and the caller is told which one it got:
 *
 *   RND_HW    RDRAND. A real hardware entropy source, reseeded by the CPU.
 *             This is the only tier where "confidential" is an honest word.
 *   RND_WEAK  A hash of whatever varies on this machine: the TSC sampled
 *             across a jittery loop, the RTC, the boot-time counters. On real
 *             hardware TSC jitter carries genuine unpredictability; under an
 *             emulator it may carry almost none. Usable, NOT trustworthy.
 *   RND_NONE  Nothing varies at all. The caller must refuse to connect.
 *
 * WHY NOT JUST USE THE WEAK PATH SILENTLY. Because that is the decision that
 * turns a security hole into a secret one. Every other honesty rule in this
 * project - the browser's home page, the `net up` label, the feature catalogue
 * - is the same rule as this: a capability that is absent must SAY it is
 * absent, and the one place that matters most is the one where the failure is
 * invisible.
 */

typedef unsigned char      u8;
typedef unsigned int       u32;
typedef unsigned long long u64;

int cpu_has_rdrand(void);
int cpu_rdrand32(u32 *out);
u64 cpu_tsc(void);
int rtc_present(void);
int rtc_read(void);
int rtc_year(void); int rtc_month(void); int rtc_day(void);
int rtc_hour(void); int rtc_min(void);   int rtc_sec(void);
void sha256(const u8 *d, u32 n, u8 *out);

#define RND_NONE 0
#define RND_WEAK 1
#define RND_HW   2

static int quality = -1;

/* The weak pool. TSC deltas across a loop whose length the CPU decides:
 * cache misses, interrupts and frequency changes all perturb it. On metal that
 * is a few bits per sample and 64 samples is enough to matter; under TCG the
 * timing is far more regular and it may be nearly nothing, which is why this
 * tier is called WEAK and not "good enough". */
static void weak_pool(u8 *out32)
{
    u64 acc[16];
    for (int i = 0; i < 16; i++) acc[i] = 0;
    for (int i = 0; i < 256; i++) {
        u64 a = cpu_tsc();
        /* a loop the compiler cannot fold away and whose cost varies */
        volatile int spin = 0;
        for (int k = 0; k < (int)(a & 0x3F) + 8; k++) spin += k;
        u64 b = cpu_tsc();
        acc[i & 15] = acc[i & 15] * 1000003ULL + (b - a) + (u64)spin;
    }
    if (rtc_present() && rtc_read()) {
        acc[0] += (u64)rtc_year() * 10000000000ULL + (u64)rtc_month() * 100000000ULL +
                  (u64)rtc_day() * 1000000ULL + (u64)rtc_hour() * 10000ULL +
                  (u64)rtc_min() * 100ULL + (u64)rtc_sec();
    }
    u8 buf[128];
    for (int i = 0; i < 16; i++)
        for (int j = 0; j < 8; j++) buf[i * 8 + j] = (u8)(acc[i] >> (8 * j));
    sha256(buf, 128, out32);
}

int rnd_quality(void)
{
    if (quality >= 0) return quality;
    if (cpu_has_rdrand()) {
        u32 v;
        if (cpu_rdrand32(&v)) { quality = RND_HW; return quality; }
    }
    /* is there any variation at all? Two TSC reads that agree mean the counter
     * is not running, and then there is nothing here worth calling entropy. */
    u64 a = cpu_tsc();
    volatile int spin = 0;
    for (int k = 0; k < 1000; k++) spin += k;
    u64 b = cpu_tsc();
    quality = (b != a) ? RND_WEAK : RND_NONE;
    return quality;
}

/* Fill `out` with `n` bytes. Returns the quality that produced them, so a
 * caller cannot obtain bytes without also learning what they are worth. */
int rnd_bytes(u8 *out, int n)
{
    int q = rnd_quality();
    if (q == RND_NONE) {
        for (int i = 0; i < n; i++) out[i] = 0;
        return q;
    }
    if (q == RND_HW) {
        int i = 0;
        while (i < n) {
            u32 v;
            if (!cpu_rdrand32(&v)) { q = RND_WEAK; break; }   /* it dried up */
            for (int k = 0; k < 4 && i < n; k++) out[i++] = (u8)(v >> (8 * k));
        }
        if (i >= n) return RND_HW;
    }
    /* the weak path, also used to finish a short RDRAND run. Each 32-byte
     * block is a fresh hash of a fresh pool plus a counter, so blocks do not
     * repeat even if the pool is poor. */
    u8 block[32], mix[64];
    int done = 0, ctr = 0;
    while (done < n) {
        weak_pool(block);
        for (int i = 0; i < 32; i++) mix[i] = block[i];
        for (int i = 0; i < 4; i++) mix[32 + i] = (u8)(ctr >> (8 * i));
        for (int i = 36; i < 64; i++) mix[i] = (u8)(done + i);
        sha256(mix, 64, block);
        for (int i = 0; i < 32 && done < n; i++) out[done++] = block[i];
        ctr++;
    }
    return RND_WEAK;
}

/* ---- the clock, as x509.c wants it ----------------------------------------
 * YYYYMMDDHHMMSSZ, or a zero-length string when there is no clock worth
 * trusting - and x509_time_ok treats null as "do not check dates" rather than
 * as "every date is fine". A wrong clock is worse than no clock: one that runs
 * slow accepts certificates that expired, and one that runs fast rejects the
 * whole web. CLAUDE.md records that this machine's RTC runs about two days
 * behind, so the implausibility check below is not theoretical.
 */
int zl_now_z(char *out)
{
    out[0] = 0;
    if (!rtc_present() || !rtc_read()) return 0;
    int y = rtc_year();
    /* a year outside this range means the RTC is not telling the time - a dead
     * battery reads 2000 or 1980, and QEMU with no -rtc reads the host */
    if (y < 2020 || y > 2100) return 0;
    int v[6] = { y, rtc_month(), rtc_day(), rtc_hour(), rtc_min(), rtc_sec() };
    int w[6] = { 4, 2, 2, 2, 2, 2 };
    int o = 0;
    for (int f = 0; f < 6; f++) {
        int val = v[f];
        for (int d = w[f] - 1; d >= 0; d--) {
            int p = 1;
            for (int k = 0; k < d; k++) p *= 10;
            out[o++] = (char)('0' + (val / p) % 10);
        }
    }
    out[o++] = 'Z';
    out[o] = 0;
    return 1;
}
