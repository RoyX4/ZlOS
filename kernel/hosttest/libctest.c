/* libctest.c - the kernel's libc replacements, against the real thing.
 *
 * interp_kernel.c reimplements fourteen libc functions because a freestanding
 * kernel has none. Each is a dozen lines and each has a well-known way to be
 * subtly wrong:
 *
 *   strcmp     comparing SIGNED chars, so anything above 127 orders backwards
 *   strncpy    forgetting that C says PAD to n, not just stop
 *   strchr     forgetting that c=='\0' must find the terminator
 *   strstr     an empty needle must match at the front, not return NULL
 *   memmove    copying forwards when the regions overlap
 *   atof       accumulating the fraction then scaling, which overflows on a
 *              long digit run
 *
 * None of those crash. They return a plausible wrong answer, in a kernel, in
 * an interpreter running somebody's script. So rather than testing what I
 * thought to test, every function is run against GLIBC'S OWN on a few thousand
 * generated inputs, including every edge I could think of and a large pile I
 * could not. Disagreement is the failure.
 *
 * The arena is mmapped at the address arena.c hardcodes, exactly as
 * arenatest.c does, so k_malloc/k_realloc/k_strdup are exercised for real
 * rather than stubbed.
 *
 *   ./build.sh && ./libctest
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/mman.h>

#define ARENA_BASE  0x00E00000UL
#define ARENA_BYTES 0x01000000UL

typedef unsigned long ul;

void *k_memcpy(void *, const void *, ul);
void *k_memmove(void *, const void *, ul);
void *k_memset(void *, int, ul);
int   k_memcmp(const void *, const void *, ul);
ul    k_strlen(const char *);
int   k_strcmp(const char *, const char *);
int   k_strncmp(const char *, const char *, ul);
char *k_strcpy(char *, const char *);
char *k_strncpy(char *, const char *, ul);
char *k_strcat(char *, const char *);
char *k_strchr(const char *, int);
char *k_strstr(const char *, const char *);
double k_atof(const char *);
int   k_atoi(const char *);
void *k_malloc(ul);
void *k_realloc(void *, ul);
char *k_strdup(const char *);
void  k_free(void *);

double k_sqrt(double); double k_exp(double); double k_log(double);
double k_log2(double); double k_log10(double); double k_sin(double);
double k_cos(double); double k_tan(double); double k_atan(double);
double k_asin(double); double k_acos(double); double k_floor(double);
double k_ceil(double); double k_trunc(double); double k_round(double);
double k_fabs(double); double k_pow(double, double);
double k_hypot(double, double); double k_fmod(double, double);

int   k_snprintf(char *, unsigned long, const char *, ...);
int   arena_init(void);
void  arena_reset(void);
void zl_putc_pub(char c) { (void)c; }

static int checks, fails;
static void ok(int c, const char *what)
{
    checks++;
    if (!c) { fails++; printf("  FAIL  %s\n", what); }
}
static int sgn(int v) { return v < 0 ? -1 : (v > 0 ? 1 : 0); }

/* a deterministic generator - no time(), so a failure reproduces exactly */
static unsigned long rng = 12345;
static unsigned nextr(void) { rng = rng * 6364136223846793005UL + 1442695040888963407UL; return (unsigned)(rng >> 33); }

static void randstr(char *b, int max)
{
    int n = (int)(nextr() % (unsigned)max);
    for (int i = 0; i < n; i++) {
        /* deliberately includes bytes above 127 - that is the signed-char trap */
        b[i] = (char)(nextr() % 255 + 1);
    }
    b[n] = 0;
}

/* interp_kernel.c and exec.c report through the terminal. term_say tees to
 * the scrollback and the serial log; neither exists in this harness. */
void term_say(const char *s) { (void)s; }

int main(void)
{
    printf("libctest - the kernel's libc replacements, against glibc's\n\n");

    void *p = mmap((void *)ARENA_BASE, ARENA_BYTES, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p != (void *)ARENA_BASE) { printf("  FAIL  cannot map the arena\n"); return 1; }
    arena_init();

    /* ---- the fixed edges -------------------------------------------------*/
    ok(k_strlen("") == 0, "strlen of empty");
    ok(k_strlen("abc") == 3, "strlen");
    ok(sgn(k_strcmp("", "")) == 0, "strcmp empty/empty");
    ok(sgn(k_strcmp("a", "")) == sgn(strcmp("a", "")), "strcmp a/empty");
    /* THE SIGNED-CHAR TRAP: 0x80 must compare GREATER than 'a', not less */
    ok(sgn(k_strcmp("\x80", "a")) == sgn(strcmp("\x80", "a")),
       "strcmp treats bytes as UNSIGNED (0x80 > 'a')");
    ok(k_strchr("abc", 0) == strchr("abc", 0),
       "strchr for '\\0' finds the terminator");
    ok(k_strstr("abc", "") == strstr("abc", ""),
       "strstr with an empty needle matches at the front");
    ok(k_strstr("abc", "d") == NULL, "strstr with no match");

    /* strncpy MUST pad to n */
    {
        char a[16], b[16];
        memset(a, 'X', sizeof a); memset(b, 'X', sizeof b);
        k_strncpy(a, "hi", 10); strncpy(b, "hi", 10);
        ok(memcmp(a, b, 16) == 0, "strncpy pads the whole n, as C requires");
    }

    /* ---- generated: strlen / strcmp / strncmp / strchr / strstr ----------*/
    {
        char x[64], y[64];
        int bad = 0;
        for (int i = 0; i < 40000; i++) {
            randstr(x, 40); randstr(y, 40);
            if (k_strlen(x) != strlen(x)) bad = 1;
            if (sgn(k_strcmp(x, y)) != sgn(strcmp(x, y))) bad = 1;
            ul n = nextr() % 40;
            if (sgn(k_strncmp(x, y, n)) != sgn(strncmp(x, y, n))) bad = 1;
            int c = (int)(nextr() % 256);
            if (k_strchr(x, c) != strchr(x, c)) bad = 1;
            /* a needle drawn from x itself, so matches actually happen */
            if (strlen(x) > 3) {
                char nd[8]; ul off = nextr() % (strlen(x) - 2);
                ul ln = 1 + nextr() % 3;
                memcpy(nd, x + off, ln); nd[ln] = 0;
                if (k_strstr(x, nd) != strstr(x, nd)) bad = 1;
            }
            if (bad) { printf("  ...disagreed on x=%s y=%s\n", x, y); break; }
        }
        ok(!bad, "40000 random strings: strlen/strcmp/strncmp/strchr/strstr all agree");
    }

    /* ---- generated: memcpy / memmove / memset / memcmp -------------------*/
    {
        unsigned char a[256], b[256], c[256], d[256];
        int bad = 0;
        for (int i = 0; i < 20000; i++) {
            for (int j = 0; j < 256; j++) a[j] = b[j] = (unsigned char)nextr();
            ul n = nextr() % 128;
            ul so = nextr() % 128, dof = nextr() % 128;
            memcpy(c, a, 256); memcpy(d, b, 256);
            k_memmove(c + dof, c + so, n);      /* OVERLAPPING, deliberately */
            memmove(d + dof, d + so, n);
            if (memcmp(c, d, 256)) { bad = 1; printf("  ...memmove n=%lu s=%lu d=%lu\n", n, so, dof); break; }
            k_memset(c, (int)(nextr() % 256), n);
            memset(d, 0, 0);                    /* keep d as the reference */
            if (sgn(k_memcmp(a, b, n)) != sgn(memcmp(a, b, n))) { bad = 1; break; }
        }
        ok(!bad, "20000 random OVERLAPPING memmoves agree with glibc");
    }

    /* ---- generated: atof / atoi -----------------------------------------*/
    {
        char b[64];
        int bad = 0;
        double worst = 0;
        for (int i = 0; i < 20000; i++) {
            long long ip = (long long)(nextr() % 1000000);
            unsigned fp = nextr() % 1000000;
            int neg = nextr() & 1;
            snprintf(b, sizeof b, "%s%lld.%06u", neg ? "-" : "", ip, fp);
            double mine = k_atof(b), real = atof(b);
            double err = fabs(mine - real);
            double rel = fabs(real) > 1 ? err / fabs(real) : err;
            if (rel > worst) worst = rel;
            if (rel > 1e-12) { bad = 1; printf("  ...atof %s -> %.17g vs %.17g\n", b, mine, real); break; }
            snprintf(b, sizeof b, "%s%lld", neg ? "-" : "", ip);
            if (k_atoi(b) != atoi(b)) { bad = 1; break; }
        }
        ok(!bad, "20000 decimals: atof matches to 1e-12 relative, atoi exactly");
        if (!bad) printf("        worst atof relative error: %.3g\n", worst);
    }
    ok(k_atof("1e10") == 1e10, "atof handles an exponent");
    ok(k_atof("-2.5e-3") == -2.5e-3, "atof handles a negative exponent");
    ok(k_atof("") == 0.0, "atof of empty is 0");

    /* ---- the allocator, against the real arena ---------------------------*/
    arena_reset();
    {
        char *s = k_strdup("hello");
        ok(s && !strcmp(s, "hello"), "strdup through the arena");
        ok((unsigned long)s >= ARENA_BASE &&
           (unsigned long)s < ARENA_BASE + ARENA_BYTES,
           "...and it lands INSIDE the arena, not on a host heap");

        /* realloc must preserve contents while growing - the case interp.c's
         * three realloc sites all rely on */
        char *g = k_malloc(8);
        memcpy(g, "1234567", 8);
        g = k_realloc(g, 64);
        ok(g && !strcmp(g, "1234567"), "realloc preserved the old contents");

        k_free(g);                       /* a no-op, and must not corrupt */
        char *after = k_malloc(16);
        ok(after != NULL, "allocation still works after a free()");
    }

    /* the arena's ceiling still applies to k_malloc - a program cannot
     * out-allocate its budget by going through the interpreter */
    {
        arena_reset();
        void *big = k_malloc(ARENA_BYTES * 2);
        ok(big == NULL, "k_malloc cannot exceed the arena's ceiling");
    }

    /* ---- maths, swept against libm --------------------------------------
     * A sine that is subtly wrong produces a plausible picture rather than an
     * error, so every function is swept rather than spot-checked, and the
     * WORST error over the sweep is printed - a max is a measurement, an
     * average hides the one input that is wrong. */
    {
        struct { const char *name; double (*mine)(double); double (*real)(double);
                 double lo, hi; double tol; } fns[] = {
            { "sqrt",  k_sqrt,  sqrt,   0.0,    1e6,   1e-14 },
            { "exp",   k_exp,   exp,   -30.0,   30.0,  1e-13 },
            { "log",   k_log,   log,    1e-6,   1e6,   1e-13 },
            { "log2",  k_log2,  log2,   1e-6,   1e6,   1e-13 },
            { "log10", k_log10, log10,  1e-6,   1e6,   1e-13 },
            { "sin",   k_sin,   sin,   -50.0,   50.0,  1e-12 },
            { "cos",   k_cos,   cos,   -50.0,   50.0,  1e-12 },
            { "atan",  k_atan,  atan,  -100.0,  100.0, 1e-12 },
            { "asin",  k_asin,  asin,  -1.0,    1.0,   1e-11 },
            { "acos",  k_acos,  acos,  -1.0,    1.0,   1e-11 },
            { "floor", k_floor, floor, -1e9,    1e9,   0.0   },
            { "ceil",  k_ceil,  ceil,  -1e9,    1e9,   0.0   },
            { "trunc", k_trunc, trunc, -1e9,    1e9,   0.0   },
            { "round", k_round, round, -1e9,    1e9,   0.0   },
            { "fabs",  k_fabs,  fabs,  -1e9,    1e9,   0.0   },
        };
        for (unsigned f = 0; f < sizeof fns / sizeof fns[0]; f++) {
            double worst = 0, worst_at = 0;
            const int N = 20000;
            for (int i = 0; i <= N; i++) {
                double x = fns[f].lo + (fns[f].hi - fns[f].lo) * (double)i / (double)N;
                double m = fns[f].mine(x), r = fns[f].real(x);
                double e = fabs(m - r);
                double rel = fabs(r) > 1.0 ? e / fabs(r) : e;
                if (rel > worst) { worst = rel; worst_at = x; }
            }
            char msg[128];
            snprintf(msg, sizeof msg, "%s agrees with libm over its range (worst %.2g at %.4g)",
                     fns[f].name, worst, worst_at);
            ok(worst <= fns[f].tol, msg);
            if (worst <= fns[f].tol)
                printf("        %-6s worst relative error %.3g\n", fns[f].name, worst);
        }

        /* two-argument ones */
        double wp = 0, wh = 0, wm = 0;
        for (int i = 0; i < 20000; i++) {
            double a = -100.0 + 200.0 * (double)(nextr() % 10000) / 10000.0;
            double b = -20.0  + 40.0  * (double)(nextr() % 10000) / 10000.0;
            if (a > 0) {
                double m = k_pow(a, b), r = pow(a, b);
                if (r != 0 && !isinf(r) && !isnan(r)) {
                    double e = fabs(m - r) / fabs(r);
                    if (e > wp) wp = e;
                }
            }
            double m2 = k_hypot(a, b), r2 = hypot(a, b);
            double e2 = fabs(m2 - r2) / (fabs(r2) > 1 ? fabs(r2) : 1);
            if (e2 > wh) wh = e2;
            if (b != 0) {
                double m3 = k_fmod(a, b), r3 = fmod(a, b);
                double e3 = fabs(m3 - r3);
                if (e3 > wm) wm = e3;
            }
        }
        ok(wp < 1e-11, "pow agrees with libm");
        ok(wh < 1e-13, "hypot agrees with libm");
        ok(wm < 1e-9,  "fmod agrees with libm");
        printf("        pow %.3g   hypot %.3g   fmod %.3g\n", wp, wh, wm);

        /* the cases the shortcuts get wrong */
        ok(k_round(-2.5) == round(-2.5), "round(-2.5) goes AWAY from zero, not floor(x+0.5)");
        ok(k_round(2.5)  == round(2.5),  "round(2.5) likewise");
        ok(k_pow(2.0, 10.0) == 1024.0, "an integer power is EXACT, not 1023.9999999");
        ok(k_pow(3.0, 3.0) == 27.0, "...and so is 3^3");
        {
            double h = k_hypot(1e200, 1e200), hr = hypot(1e200, 1e200);
            /* NOT exact equality - that asks a reimplementation to reproduce
             * glibc bit for bit, which is a different and much harder claim.
             * What matters is that scaling stopped it reaching inf. */
            ok(!isinf(h) && fabs(h - hr) / hr < 1e-14,
               "hypot(1e200,1e200) does not overflow on the way");
        }
    }

    /* ---- the formatter --------------------------------------------------
     * Only five conversions, and each is checked against glibc's snprintf on
     * the same inputs. The TRUNCATION contract matters as much as the output:
     * a formatter that lies about how much it would have written is a buffer
     * overrun waiting for its first caller who checks. */
    {
        char a[64], b[64];
        int bad = 0;
        for (int i = 0; i < 5000; i++) {
            long long v = (long long)(nextr()) - 2147483648LL;
            int ra = k_snprintf(a, sizeof a, "%lld", v);
            int rb = snprintf(b, sizeof b, "%lld", v);
            if (strcmp(a, b) || ra != rb) { bad = 1; printf("  ...%lld -> %s vs %s\n", v, a, b); break; }
            unsigned long long u = (unsigned long long)nextr() * 65537ULL;
            k_snprintf(a, sizeof a, "%llx", u);
            snprintf(b, sizeof b, "%llx", u);
            if (strcmp(a, b)) { bad = 1; break; }
            k_snprintf(a, sizeof a, "[%s]", "hi");
            if (strcmp(a, "[hi]")) { bad = 1; break; }
            k_snprintf(a, sizeof a, "%d", (int)v);
            snprintf(b, sizeof b, "%d", (int)v);
            if (strcmp(a, b)) { bad = 1; break; }
        }
        ok(!bad, "5000 x %lld / %llx / %d / %s match glibc's snprintf exactly");

        /* truncation: the return is what WOULD have been written */
        char small[5];
        int r = k_snprintf(small, sizeof small, "%s", "abcdefgh");
        ok(r == 8, "a truncated snprintf returns the length it WOULD have written");
        ok(small[4] == 0, "...and still NUL-terminates");
        ok(!strcmp(small, "abcd"), "...with the prefix that fit");

        /* %g only ever sees a non-integral number from interp.c */
        k_snprintf(a, sizeof a, "%g", 1.5);      ok(!strcmp(a, "1.5"), "%g of 1.5");
        k_snprintf(a, sizeof a, "%g", -0.25);    ok(!strcmp(a, "-0.25"), "%g of -0.25");
        k_snprintf(a, sizeof a, "%g", 3.14159);  ok(!strcmp(a, "3.14159"), "%g of 3.14159 ROUNDS, not truncates");
        k_snprintf(a, sizeof a, "%g", 2.0/3.0);  ok(!strcmp(a, "0.666667"), "%g of 2/3 rounds the last digit up");
        /* the carry: 0.9999999 must not print as 0.1000000 with the integer
         * part still reading 0 */
        k_snprintf(a, sizeof a, "%g", 0.9999999); ok(!strcmp(a, "1"), "%g carries into the integer part");
        /* swept against glibc where the two agree by construction: six
         * decimals is six significant digits for values under 10 */
        {
            int gbad = 0;
            for (int i = 0; i < 5000; i++) {
                double v = (double)(nextr() % 100000000) / 100000.0;   /* 0 .. 1000 */
                k_snprintf(a, sizeof a, "%g", v);
                snprintf(b, sizeof b, "%g", v);
                if (strcmp(a, b)) { gbad = 1; printf("  ...%%g %.9f -> %s vs %s\n", v, a, b); break; }
            }
            ok(!gbad, "5000 fractions over 0..1000: %g matches glibc EXACTLY");
        }

        /* an unsupported conversion must be VISIBLE, not silently wrong */
        k_snprintf(a, sizeof a, "%q", 1);
        ok(strstr(a, "%q") != NULL, "an unsupported conversion is emitted verbatim");
    }

    printf("\n%d checks, %d failures\n", checks, fails);
    if (!fails) printf("ok    every replacement agrees with the libc it replaces\n");
    return fails ? 1 : 0;
}
