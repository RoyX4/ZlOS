/* rtctest.c - the clock, against a CMOS chip that can be made to misbehave.
 *
 * The bug this driver exists to avoid happens for a few hundred microseconds a
 * second and produces a time that is wrong by an hour. You cannot reproduce it
 * by running the machine; you can barely reproduce it by trying. So the chip
 * is faked here, and the fake can:
 *
 *   - hold UIP high for a scripted number of reads
 *   - hand out a DIFFERENT time on the second read, which is exactly what a
 *     torn read is, on demand rather than once in a few thousand boots
 *   - claim any of the three encodings: BCD or binary, 12- or 24-hour
 *   - be absent entirely (an undecoded port floats high and reads 0xFF)
 *
 * Build and run:  ./build.sh && ./rtctest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

typedef unsigned int       u32;
typedef unsigned long long u64;

int  rtc_read(void);
int  rtc_present(void);
int  rtc_year(void);
int  rtc_month(void);
int  rtc_day(void);
int  rtc_hour(void);
int  rtc_min(void);
int  rtc_sec(void);
int  rtc_valid(void);
int  rtc_fail(void);
u32  rtc_unix(void);
u64  rtc_unix64(void);
void rtc_print_time(void);
void rtc_print_date(void);

/* ---- the character sink -------------------------------------------------- */
static char saidbuf[4096];
static int  saidlen;
void zl_putc_pub(char c)
{
    if (saidlen < (int)sizeof saidbuf - 1) saidbuf[saidlen++] = c;
    saidbuf[saidlen] = 0;
}
static void said_reset(void) { saidlen = 0; saidbuf[0] = 0; }

/* ---- the fake MC146818 --------------------------------------------------- */
static unsigned char reg[128];
static int  sel;
static int  absent;
static int  uip_reads_remaining;   /* hold UIP high for this many status reads */
static int  tear_after;            /* after N data reads, switch to the alt set */
static int  alternate_every;       /* flip source every N reads: never settles  */
static int  data_reads;
static unsigned char alt[128];
static int  have_alt;

unsigned char rtc_port_in(unsigned short port)
{
    if (absent) return 0xFF;
    if (port != 0x71) return 0xFF;

    if (sel == 0x0A) {
        unsigned char v = reg[0x0A];
        if (uip_reads_remaining > 0) { uip_reads_remaining--; return v | 0x80; }
        return (unsigned char)(v & 0x7F);
    }
    data_reads++;
    /* One full sweep is seven data reads. Flipping the source every seven
     * means sweep N and sweep N+1 can never agree, however many times the
     * driver retries - a chip that is genuinely never consistent. */
    if (have_alt && alternate_every > 0)
        return ((data_reads - 1) / alternate_every) % 2 ? alt[sel] : reg[sel];
    if (have_alt && tear_after > 0 && data_reads > tear_after) return alt[sel];
    return reg[sel];
}
void rtc_port_out(unsigned short port, unsigned char val)
{
    if (port == 0x70) sel = val & 0x7F;
}

static void cmos_reset(void)
{
    memset(reg, 0, sizeof reg);
    memset(alt, 0, sizeof alt);
    absent = 0; uip_reads_remaining = 0; tear_after = 0; alternate_every = 0;
    data_reads = 0; have_alt = 0;
    reg[0x0B] = 0x02;                 /* 24-hour, BCD - what QEMU serves      */
    reg[0x32] = 0x20;                 /* century 20, BCD                      */
}

static unsigned char tobcd(int v) { return (unsigned char)(((v / 10) << 4) | (v % 10)); }

static void set_bcd(int y, int mo, int d, int h, int mi, int s)
{
    reg[0x00] = tobcd(s);  reg[0x02] = tobcd(mi); reg[0x04] = tobcd(h);
    reg[0x07] = tobcd(d);  reg[0x08] = tobcd(mo); reg[0x09] = tobcd(y % 100);
    reg[0x32] = tobcd(y / 100);
    reg[0x0B] = 0x02;
}
static void set_alt_bcd(int y, int mo, int d, int h, int mi, int s)
{
    memcpy(alt, reg, sizeof alt);
    alt[0x00] = tobcd(s);  alt[0x02] = tobcd(mi); alt[0x04] = tobcd(h);
    alt[0x07] = tobcd(d);  alt[0x08] = tobcd(mo); alt[0x09] = tobcd(y % 100);
    alt[0x32] = tobcd(y / 100);
    have_alt = 1;
}

static int fails;
static void ok(const char *what, int cond)
{
    printf("  %-62s %s\n", what, cond ? "ok" : "FAIL");
    if (!cond) fails++;
}
static int is(int y, int mo, int d, int h, int mi, int s)
{
    return rtc_year() == y && rtc_month() == mo && rtc_day() == d &&
           rtc_hour() == h && rtc_min() == mi && rtc_sec() == s;
}
static void show(void)
{
    printf("      read back: %04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_year(), rtc_month(), rtc_day(), rtc_hour(), rtc_min(), rtc_sec());
}

int main(void)
{
    printf("the clock - CMOS, BCD, and the torn read\n\n");

    /* ---- the ordinary case ---------------------------------------------- */
    printf("  -- BCD, 24-hour: what QEMU and most firmware serve --\n");
    cmos_reset();
    set_bcd(2026, 8, 18, 14, 37, 5);
    ok("a clean read succeeds", rtc_read() == 1);
    ok("...and every field is right", is(2026, 8, 18, 14, 37, 5));
    show();
    ok("...and it reports itself valid", rtc_valid() == 1);

    /* BCD really is BCD: 0x59 is 59, not 89 */
    set_bcd(2026, 12, 31, 23, 59, 59);
    rtc_read();
    ok("0x59 decodes as 59, not 89", is(2026, 12, 31, 23, 59, 59));

    /* ---- binary mode ----------------------------------------------------- */
    printf("\n  -- binary mode (status B bit 2) --\n");
    cmos_reset();
    reg[0x0B] = 0x02 | 0x04;
    reg[0x00] = 5; reg[0x02] = 37; reg[0x04] = 14;
    reg[0x07] = 18; reg[0x08] = 8; reg[0x09] = 26; reg[0x32] = 20;
    ok("a binary-mode chip reads", rtc_read() == 1);
    ok("...with the same answer as the BCD one", is(2026, 8, 18, 14, 37, 5));

    /* ---- 12-hour mode, the one that is wrong by twelve hours -------------- */
    printf("\n  -- 12-hour mode, where the PM bit sits ON TOP of the digits --\n");
    cmos_reset();
    reg[0x0B] = 0x00;                       /* BCD, 12-hour */
    reg[0x07] = tobcd(18); reg[0x08] = tobcd(8); reg[0x09] = tobcd(26);
    reg[0x02] = tobcd(37); reg[0x00] = tobcd(5);

    reg[0x04] = (unsigned char)(0x80 | tobcd(1));      /* 1 PM */
    rtc_read();
    ok("1 PM is 13:00, not 81:00 (the PM bit comes off BEFORE the BCD decode)",
       rtc_hour() == 13);

    reg[0x04] = tobcd(1);                              /* 1 AM */
    rtc_read();
    ok("1 AM is 01:00", rtc_hour() == 1);

    reg[0x04] = (unsigned char)(0x80 | tobcd(12));     /* 12 PM = noon */
    rtc_read();
    ok("12 PM is NOON, 12:00 - not 24:00", rtc_hour() == 12);

    reg[0x04] = tobcd(12);                             /* 12 AM = midnight */
    rtc_read();
    ok("12 AM is MIDNIGHT, 00:00 - not 12:00", rtc_hour() == 0);

    reg[0x04] = (unsigned char)(0x80 | tobcd(11));     /* 11 PM */
    rtc_read();
    ok("11 PM is 23:00", rtc_hour() == 23);

    /* ---- THE TRAP: update in progress ------------------------------------ */
    printf("\n  -- update in progress --\n");
    cmos_reset();
    set_bcd(2026, 8, 18, 10, 0, 0);
    uip_reads_remaining = 12;
    ok("UIP held high for a while still yields a read", rtc_read() == 1);
    ok("...and the right time", is(2026, 8, 18, 10, 0, 0));

    cmos_reset();
    set_bcd(2026, 8, 18, 10, 0, 0);
    uip_reads_remaining = 1000000;          /* never clears */
    ok("a chip whose UIP never clears is REFUSED, not waited on forever",
       rtc_read() == 0);
    ok("...reporting that as the reason", rtc_fail() == 1);
    ok("...and the clock reports itself invalid", rtc_valid() == 0);

    /* ---- THE TRAP PROPER: a torn read ------------------------------------ */
    printf("\n  -- the torn read, which is the whole reason for reading twice --\n");
    cmos_reset();
    /* 10:59:59 rolling over to 11:00:00 midway through the register sweep.
     * A single-read driver returns 10:00:00 here - the hour not yet carried,
     * the minutes and seconds already reset. An hour wrong, once in a while,
     * and unreproducible when you go looking for it. */
    set_bcd(2026, 8, 18, 10, 59, 59);
    set_alt_bcd(2026, 8, 18, 11, 0, 0);
    tear_after = 3;                         /* switch part-way through */
    int r = rtc_read();
    ok("a read torn across the rollover still succeeds", r == 1);
    show();
    ok("...and NEVER returns the mixture 10:00:00",
       !(rtc_hour() == 10 && rtc_min() == 0 && rtc_sec() == 0));
    ok("...it returns one of the two CONSISTENT times",
       is(2026, 8, 18, 10, 59, 59) || is(2026, 8, 18, 11, 0, 0));

    /* A chip that NEVER settles must refuse rather than guess. The source
     * flips every seven reads - one whole register sweep - so no two
     * consecutive sweeps can ever agree no matter how many times it retries. */
    cmos_reset();
    set_bcd(2026, 8, 18, 10, 59, 59);
    set_alt_bcd(2026, 8, 18, 11, 0, 0);
    alternate_every = 7;
    ok("a chip whose two reads NEVER agree is refused", rtc_read() == 0);
    ok("...naming that as the reason, not the UIP timeout", rtc_fail() == 2);
    ok("...and the clock reports itself invalid rather than guessing",
       rtc_valid() == 0);
    said_reset(); rtc_print_time();
    ok("...and prints '--' rather than a half-carried time",
       strcmp(saidbuf, "--") == 0);

    /* ---- an absent chip -------------------------------------------------- */
    printf("\n  -- no RTC at all --\n");
    cmos_reset();
    absent = 1;
    ok("an undecoded port reading 0xFF is reported as absent", rtc_present() == 0);

    cmos_reset();
    set_bcd(2026, 8, 18, 12, 0, 0);
    ok("a present one says so", rtc_present() == 1);

    /* ---- nonsense must be refused, not drawn ----------------------------- */
    printf("\n  -- values a clock cannot hold --\n");
    cmos_reset();
    set_bcd(2026, 0, 18, 12, 0, 0);          /* month zero */
    ok("month 0 is refused", rtc_read() == 0);
    cmos_reset();
    set_bcd(2026, 13, 18, 12, 0, 0);
    ok("month 13 is refused", rtc_read() == 0);
    cmos_reset();
    set_bcd(2026, 8, 18, 47, 0, 0);          /* 47 o'clock */
    ok("hour 47 is refused", rtc_read() == 0);
    cmos_reset();
    set_bcd(2026, 8, 0, 12, 0, 0);
    ok("day 0 is refused", rtc_read() == 0);

    cmos_reset();
    set_bcd(2026, 2, 30, 12, 0, 0);
    ok("February 30 is refused", rtc_read() == 0);
    cmos_reset();
    set_bcd(2025, 2, 29, 12, 0, 0);
    ok("February 29 in a non-leap year is refused", rtc_read() == 0);
    cmos_reset();
    set_bcd(2024, 2, 29, 12, 0, 0);
    ok("February 29 in a leap year is accepted", rtc_read() == 1);
    cmos_reset();
    set_bcd(2026, 4, 31, 12, 0, 0);
    ok("April 31 is refused", rtc_read() == 0);

    cmos_reset();
    set_bcd(2026, 8, 18, 12, 0, 0);
    reg[0x0B] = 0x00;
    reg[0x04] = 0x00;
    ok("hour zero in 12-hour mode is refused", rtc_read() == 0);

    cmos_reset();
    set_bcd(2026, 8, 18, 12, 0, 0);
    reg[0x00] = 0x1A;
    ok("a malformed BCD digit is refused", rtc_read() == 0);

    said_reset();
    rtc_print_time();
    ok("...and an invalid clock PRINTS '--', it does not draw a stale time",
       strcmp(saidbuf, "--") == 0);

    cmos_reset();
    set_bcd(2026, 8, 18, 12, 0, 0);
    ok("a valid read recovers after a refusal", rtc_read() == 1);
    ok("a successful read clears the previous failure reason", rtc_fail() == 0);

    /* ---- the century register -------------------------------------------- */
    printf("\n  -- the century register, which is not on every machine --\n");
    cmos_reset();
    set_bcd(2026, 8, 18, 12, 0, 0);
    reg[0x32] = 0x00;                        /* absent: reads zero */
    rtc_read();
    ok("a missing century register assumes 20xx rather than year 26",
       rtc_year() == 2026);
    reg[0x32] = 0xFF;
    rtc_read();
    ok("...and 0xFF is treated the same way", rtc_year() == 2026);
    reg[0x32] = 0x19;
    rtc_read();
    ok("a real century 19 is believed", rtc_year() == 1926);

    /* ---- seconds since 1970 ---------------------------------------------- */
    printf("\n  -- seconds since 1970 --\n");
    cmos_reset();
    set_bcd(1970, 1, 1, 0, 0, 0); reg[0x32] = 0x19;
    rtc_read();
    ok("the epoch itself is 0", rtc_unix() == 0);

    cmos_reset();
    set_bcd(2000, 1, 1, 0, 0, 0);
    rtc_read();
    ok("2000-01-01 is 946684800", rtc_unix() == 946684800u);

    cmos_reset();
    set_bcd(2024, 2, 29, 12, 0, 0);          /* a leap day */
    rtc_read();
    ok("2024-02-29 12:00:00 is 1709208000", rtc_unix() == 1709208000u);

    cmos_reset();
    set_bcd(2000, 2, 29, 0, 0, 0);           /* the 400-year leap rule */
    rtc_read();
    ok("2000-02-29 exists (divisible by 400) and is 951782400",
       rtc_unix() == 951782400u);

    cmos_reset();
    set_bcd(2026, 8, 18, 14, 37, 5);
    rtc_read();
    /* every expected value in this section came from `date -u -d ... +%s`,
     * not from arithmetic done in my head - the first draft of this line was
     * wrong by 7200 for exactly that reason */
    ok("2026-08-18 14:37:05 is 1787063825", rtc_unix() == 1787063825u);

    cmos_reset();
    set_bcd(2038, 1, 19, 3, 14, 7);          /* the 32-bit signed cliff */
    rtc_read();
    ok("2038-01-19 03:14:07 is 2147483647 and still fits unsigned",
       rtc_unix() == 2147483647u);
    cmos_reset();
    set_bcd(2038, 1, 19, 3, 14, 8);
    rtc_read();
    ok("...and one second later does NOT go negative", rtc_unix() == 2147483648u);

    /* ---- printing -------------------------------------------------------- */
    printf("\n  -- printing --\n");
    cmos_reset();
    set_bcd(2026, 8, 18, 9, 5, 3);
    rtc_read();
    said_reset(); rtc_print_time();
    ok("a single-digit time is zero-padded, not '9:5:3'",
       strcmp(saidbuf, "09:05:03") == 0);
    said_reset(); rtc_print_date();
    ok("the date prints ISO order", strcmp(saidbuf, "2026-08-18") == 0);

    printf("\n%s: %d failure(s)\n", fails ? "FAILED" : "all good", fails);
    return fails ? 1 : 0;
}
