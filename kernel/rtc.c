/* rtc.c - a clock that knows what time it is.
 *
 * The desktop header has always drawn a number that looks like a time and is
 * not one: it comes from ticks(), the 100 Hz PIT counter, which is uptime
 * since boot. Boot the machine at nine in the morning and it says 00:00:03.
 *
 * There is a real clock on every PC, it is battery-backed, and it is eighty
 * lines away: the MC146818 in CMOS, addressed through ports 0x70 and 0x71.
 * An OS knows what time it is.
 *
 * THE TRAP, WHICH EVERYONE HITS EXACTLY ONCE
 * ------------------------------------------
 * The chip updates its registers once a second, and while it is doing so they
 * are inconsistent. Read at the wrong moment and you get 10:59:60, or worse
 * 10:00:00 for the second that should have been 11:00:00 - the minutes and
 * hours already carried, the seconds not yet. Status register A bit 7 (UIP,
 * update in progress) says when that is happening.
 *
 * Waiting for UIP to clear is necessary and NOT SUFFICIENT. The spec grants
 * 244 microseconds after it clears, and reading seven registers through two
 * port accesses each can exceed that on an emulated machine under load - which
 * is precisely the machine this runs on. So this reads the whole set TWICE and
 * accepts the value only when two consecutive reads agree. If they never
 * agree, it REFUSES rather than returning a time from the middle of a carry.
 *
 * THREE ENCODINGS, ALL OPTIONAL
 * -----------------------------
 * Status register B says which: bit 2 clear means the values are BCD (0x59 is
 * fifty-nine, not eighty-nine), and bit 1 clear means 12-hour with bit 7 of
 * the hour as a PM flag. QEMU hands out BCD/24-hour, real firmware usually the
 * same, and assuming it is how you get a clock that is wrong by twelve hours
 * on somebody else's machine and right on yours.
 *
 * WHY THIS FILE CAN BE TESTED WITHOUT A MACHINE
 * ---------------------------------------------
 * The two port instructions are the only hardware here; everything else is
 * decoding. Under -DRTC_HOSTTEST they come from hosttest/rtctest.c, which
 * serves a scriptable CMOS that can hold UIP high, can hand out a DIFFERENT
 * time on the second read, and can claim any of the encodings. The torn read
 * this file exists to defeat is not reproducible on demand any other way -
 * it happens for a few microseconds a second.
 */

typedef unsigned long long u64;
typedef unsigned int       u32;
typedef unsigned char      u8;

void zl_putc_pub(char c);

/* ---- the hardware seam --------------------------------------------------- */
#ifdef RTC_HOSTTEST
unsigned char rtc_port_in(unsigned short port);
void          rtc_port_out(unsigned short port, unsigned char val);
#else
extern unsigned char zl_inb(unsigned short port);
extern void          zl_outb(unsigned short port, unsigned char val);
static unsigned char rtc_port_in(unsigned short p)            { return zl_inb(p); }
static void          rtc_port_out(unsigned short p, u8 v)     { zl_outb(p, v); }
#endif

#define CMOS_ADDR   0x70
#define CMOS_DATA   0x71

#define R_SEC       0x00
#define R_MIN       0x02
#define R_HOUR      0x04
#define R_DAY       0x07
#define R_MONTH     0x08
#define R_YEAR      0x09
#define R_STATUS_A  0x0A
#define R_STATUS_B  0x0B
#define R_CENTURY   0x32          /* where the FADT points on every PC that
                                     has one; absent machines read 0 or 0xFF */

#define A_UIP       0x80
#define B_BINARY    0x04
#define B_24HOUR    0x02

/* Bounded, because an absent or wedged RTC must not hang the boot. 100000
 * port pairs is far longer than the ~1 ms the flag is ever set for, and still
 * finite. */
#define UIP_SPINS   100000
#define READ_TRIES  8

struct rtc_raw { u8 s, mi, h, d, mo, y, cent; };

static int rtc_ok;
static int rtc_last_fail;         /* 1 = UIP never cleared, 2 = never agreed */
static struct rtc_raw last;
static int  l_year, l_month, l_day, l_hour, l_min, l_sec;

static u8 cmos(int reg)
{
    /* Bit 7 of port 0x70 is the NMI DISABLE line, not part of the address.
     * Writing the register number with that bit set leaves non-maskable
     * interrupts switched off for the rest of the machine's life, which is a
     * famous way to make an unrelated subsystem mysteriously stop reporting
     * faults. Masking to 0x7F keeps NMI enabled. */
    rtc_port_out(CMOS_ADDR, (u8)(reg & 0x7F));
    return rtc_port_in(CMOS_DATA);
}

static int uip_clear(void)
{
    for (long i = 0; i < UIP_SPINS; i++)
        if (!(cmos(R_STATUS_A) & A_UIP)) return 1;
    return 0;
}

static void grab(struct rtc_raw *r)
{
    r->s    = cmos(R_SEC);
    r->mi   = cmos(R_MIN);
    r->h    = cmos(R_HOUR);
    r->d    = cmos(R_DAY);
    r->mo   = cmos(R_MONTH);
    r->y    = cmos(R_YEAR);
    r->cent = cmos(R_CENTURY);
}

static int same(const struct rtc_raw *a, const struct rtc_raw *b)
{
    return a->s == b->s && a->mi == b->mi && a->h == b->h &&
           a->d == b->d && a->mo == b->mo && a->y == b->y && a->cent == b->cent;
}

static int bcd(u8 v) { return (v & 0x0F) + ((v >> 4) * 10); }

/* ---- read it ------------------------------------------------------------- */
int rtc_read(void)
{
    struct rtc_raw a, b;

    rtc_ok = 0;
    for (int attempt = 0; attempt < READ_TRIES; attempt++) {
        if (!uip_clear()) { rtc_last_fail = 1; return 0; }
        grab(&a);
        if (!uip_clear()) { rtc_last_fail = 1; return 0; }
        grab(&b);
        if (same(&a, &b)) goto decode;
    }
    rtc_last_fail = 2;
    return 0;

decode:;
    u8 sb = cmos(R_STATUS_B);
    int binary = (sb & B_BINARY) != 0;
    int h24    = (sb & B_24HOUR) != 0;

    int sec = binary ? a.s  : bcd(a.s);
    int min = binary ? a.mi : bcd(a.mi);
    int day = binary ? a.d  : bcd(a.d);
    int mon = binary ? a.mo : bcd(a.mo);
    int yr  = binary ? a.y  : bcd(a.y);

    /* The hour is the awkward one. In 12-hour mode bit 7 is the PM flag and it
     * sits ON TOP of the BCD digits, so it has to come off BEFORE decoding or
     * 0x81 (1 PM) decodes as eighty-one. */
    int pm  = (!h24 && (a.h & 0x80)) ? 1 : 0;
    int hr  = binary ? (a.h & 0x7F) : bcd((u8)(a.h & 0x7F));
    if (!h24) {
        /* 12 AM is midnight (0) and 12 PM is noon (12) - the one case where
         * the obvious `hr + 12` is wrong in both directions. */
        if (hr == 12) hr = 0;
        if (pm) hr += 12;
    }

    /* The century register is not present on every machine. 0x00 and 0xFF are
     * the two ways it says so; anything outside a plausible range is treated
     * the same way, rather than believing a year 6400. */
    int cent = binary ? a.cent : bcd(a.cent);
    if (cent < 19 || cent > 21) cent = 20;

    l_year  = cent * 100 + yr;
    l_month = mon;
    l_day   = day;
    l_hour  = hr;
    l_min   = min;
    l_sec   = sec;
    last    = a;

    /* A clock reading 47:00 or month 0 is a clock that is not there. Better a
     * refusal than a header confidently drawing nonsense. */
    if (l_month < 1 || l_month > 12 || l_day < 1 || l_day > 31 ||
        l_hour > 23 || l_min > 59 || l_sec > 60) {
        rtc_last_fail = 3;
        return 0;
    }

    rtc_ok = 1;
    return 1;
}

int rtc_present(void)
{
    /* Status A reading all-ones is an undecoded port floating high - there is
     * no RTC there at all. All-zeroes with a zero month is the other shape of
     * absent. */
    u8 sa = cmos(R_STATUS_A);
    if (sa == 0xFF) return 0;
    return 1;
}

int rtc_year(void)  { return l_year; }
int rtc_month(void) { return l_month; }
int rtc_day(void)   { return l_day; }
int rtc_hour(void)  { return l_hour; }
int rtc_min(void)   { return l_min; }
int rtc_sec(void)   { return l_sec; }
int rtc_valid(void) { return rtc_ok; }
int rtc_fail(void)  { return rtc_last_fail; }

/* ---- seconds since 1970 --------------------------------------------------
 * Needed because a file's mtime is a number, not six of them, and because
 * comparing two timestamps should not mean comparing six fields in order.
 *
 * days_from_civil, which is exact for every date in the range and has no
 * lookup table and no loop over years. It shifts the year to start in March so
 * the leap day lands at the END, which is the trick that removes the special
 * case entirely - February's length stops mattering because February is last.
 */
static long long days_from_civil(int y, int m, int d)
{
    y -= (m <= 2);
    long long era = (long long)((y >= 0 ? y : y - 399) / 400);
    long long yoe = (long long)y - era * 400;                       /* 0..399 */
    long long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; /* 0..365 */
    long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;          /* 0..146096 */
    return era * 146097 + doe - 719468;
}

/* Split into two halves for the same reason design_kernel.md gives for every
 * other 64-bit value here: a zl number is a double, and above 2^53 it stops
 * being exact. A unix timestamp is nowhere near that, but the two-halves rule
 * is what keeps that true by construction rather than by luck. */
u64 rtc_unix64(void)
{
    if (!rtc_ok) return 0;
    long long days = days_from_civil(l_year, l_month, l_day);
    long long s = days * 86400LL + l_hour * 3600LL + l_min * 60LL + l_sec;
    if (s < 0) return 0;
    return (u64)s;
}

u32 rtc_unix(void) { return (u32)rtc_unix64(); }

/* ---- printing ------------------------------------------------------------
 * Two digits, always, because a clock that reads 9:5 is a clock nobody trusts.
 */
static void two(int v)
{
    if (v < 0) v = 0;
    zl_putc_pub((char)('0' + (v / 10) % 10));
    zl_putc_pub((char)('0' + v % 10));
}

void rtc_print_time(void)
{
    if (!rtc_ok) { zl_putc_pub('-'); zl_putc_pub('-'); return; }
    two(l_hour); zl_putc_pub(':'); two(l_min); zl_putc_pub(':'); two(l_sec);
}

/* "HH:MM" one character at a time, because the zl kernel subset has no string
 * values and the header has to draw this a glyph at a time anyway - the same
 * shape as cpu_char(), which kernel.zl already uses for the CPU brand string.
 * Reads the CACHED time, so drawing costs no port I/O; rtc_read() is what
 * refreshes it. An invalid clock spells "--:--" rather than a stale time. */
int rtc_hhmm_byte(int i)
{
    if (i < 0 || i > 4) return 0;
    if (!rtc_ok) return (i == 2) ? ':' : '-';
    switch (i) {
    case 0: return '0' + (l_hour / 10) % 10;
    case 1: return '0' + l_hour % 10;
    case 2: return ':';
    case 3: return '0' + (l_min / 10) % 10;
    default:return '0' + l_min % 10;
    }
}

void rtc_print_date(void)
{
    if (!rtc_ok) { zl_putc_pub('-'); zl_putc_pub('-'); return; }
    int y = l_year;
    zl_putc_pub((char)('0' + (y / 1000) % 10));
    zl_putc_pub((char)('0' + (y / 100) % 10));
    zl_putc_pub((char)('0' + (y / 10) % 10));
    zl_putc_pub((char)('0' + y % 10));
    zl_putc_pub('-'); two(l_month); zl_putc_pub('-'); two(l_day);
}
