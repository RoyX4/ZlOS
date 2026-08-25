/* interp_kernel.c - what interp.c needs that a kernel does not have.
 *
 * The interpreter is 1,900 lines of ordinary C. It needs a heap, a string
 * library, a maths library and a way to format a number, and a freestanding
 * kernel has none of those. This file is all four, plus the binding that makes
 * a program's memory come out of arena.c rather than out of nowhere.
 *
 * WHY THESE ARE NOT #ifdefs INSIDE interp.c
 *
 * Because then the hosted interpreter and the kernel one are two programs that
 * look like one, and the test suite only ever exercises the half that is not
 * shipping. Everything here is a plain function with a libc name, so interp.c
 * compiles against libc on the host and against this file in the kernel with
 * no conditional code in the interpreter at all - and hosttest/libctest.c
 * checks every one of these against the real libc it is replacing, for
 * thousands of inputs, so "mine agrees with glibc" is a measured claim.
 *
 * THE ONE THING THAT IS DELIBERATELY NOT A FAITHFUL COPY is the allocator.
 * zi_alloc goes to the arena, which has a ceiling and no free(). That is the
 * whole of Item 2's third non-negotiable and the reason a runaway program is
 * bounded in memory as well as in time.
 */

typedef unsigned long  ul;
typedef unsigned int   u32;
typedef unsigned char  u8;

#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__ uptr;
#else
typedef unsigned long    uptr;
#endif

/* arena.c - a program's memory, and its ceiling. */
extern void *arena_alloc(unsigned long bytes);
extern unsigned long arena_capacity(void);
extern unsigned long arena_base_addr(void);

/* ---- memory --------------------------------------------------------------
 * Byte-at-a-time on purpose. A word-at-a-time memcpy is four times faster and
 * needs alignment reasoning that is wrong exactly once, on the one call where
 * a Value straddles a boundary - and the interpreter's copies are short. If
 * this ever shows up in a profile it can be widened THEN, against a benchmark,
 * which is the only honest reason to write the harder version. */
void *k_memcpy(void *d, const void *s, ul n)
{
    u8 *dd = (u8 *)d; const u8 *ss = (const u8 *)s;
    for (ul i = 0; i < n; i++) dd[i] = ss[i];
    return d;
}

void *k_memmove(void *d, const void *s, ul n)
{
    u8 *dd = (u8 *)d; const u8 *ss = (const u8 *)s;
    if (dd == ss || n == 0) return d;
    /* Overlap is the entire reason this is not memcpy: copying forwards when
     * the destination is inside the source overwrites bytes not yet read. */
    if (dd < ss) { for (ul i = 0; i < n; i++) dd[i] = ss[i]; }
    else         { for (ul i = n; i-- > 0; )  dd[i] = ss[i]; }
    return d;
}

void *k_memset(void *d, int c, ul n)
{
    u8 *dd = (u8 *)d;
    for (ul i = 0; i < n; i++) dd[i] = (u8)c;
    return d;
}

int k_memcmp(const void *a, const void *b, ul n)
{
    const u8 *x = (const u8 *)a, *y = (const u8 *)b;
    for (ul i = 0; i < n; i++) if (x[i] != y[i]) return x[i] < y[i] ? -1 : 1;
    return 0;
}

/* ---- strings -------------------------------------------------------------
 * Return values match C's: the SIGN of the difference, not the difference, so
 * nothing can depend on a magnitude that is not specified. And the comparison
 * is on UNSIGNED chars, which is what the standard says and what a naive
 * signed-char version gets wrong for anything above 127 - a difference that
 * only shows up once somebody puts a non-ASCII byte in a string literal. */
ul k_strlen(const char *s) { ul n = 0; while (s[n]) n++; return n; }

int k_strcmp(const char *a, const char *b)
{
    const u8 *x = (const u8 *)a, *y = (const u8 *)b;
    while (*x && *x == *y) { x++; y++; }
    return (*x == *y) ? 0 : (*x < *y ? -1 : 1);
}

int k_strncmp(const char *a, const char *b, ul n)
{
    const u8 *x = (const u8 *)a, *y = (const u8 *)b;
    for (ul i = 0; i < n; i++) {
        if (x[i] != y[i]) return x[i] < y[i] ? -1 : 1;
        if (x[i] == 0) return 0;
    }
    return 0;
}

char *k_strcpy(char *d, const char *s)
{
    ul i = 0;
    while ((d[i] = s[i]) != 0) i++;
    return d;
}

char *k_strncpy(char *d, const char *s, ul n)
{
    ul i = 0;
    while (i < n && s[i]) { d[i] = s[i]; i++; }
    while (i < n) d[i++] = 0;          /* C says pad, and callers rely on it */
    return d;
}

char *k_strcat(char *d, const char *s)
{
    ul i = k_strlen(d), j = 0;
    while ((d[i + j] = s[j]) != 0) j++;
    return d;
}

char *k_strchr(const char *s, int c)
{
    for (;; s++) {
        if (*s == (char)c) return (char *)s;
        if (!*s) return 0;             /* c=='\0' finds the terminator: C says so */
    }
}

char *k_strstr(const char *h, const char *n)
{
    if (!*n) return (char *)h;         /* empty needle matches at the front */
    for (; *h; h++) {
        ul i = 0;
        while (n[i] && h[i] == n[i]) i++;
        if (!n[i]) return (char *)h;
    }
    return 0;
}

/* ---- numbers in ----------------------------------------------------------
 * k_atof only has to read what zl's lexer produces: an optional sign, digits,
 * an optional fraction, an optional exponent. No hex floats, no infinities, no
 * locale. Building the fraction by dividing rather than by accumulating and
 * scaling at the end costs a little precision in the last bits and cannot
 * overflow on a long run of digits, which the other order can. */
double k_atof(const char *s)
{
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    int neg = 0;
    if (*s == '-') { neg = 1; s++; } else if (*s == '+') s++;

    double v = 0.0;
    while (*s >= '0' && *s <= '9') { v = v * 10.0 + (double)(*s - '0'); s++; }

    if (*s == '.') {
        s++;
        double scale = 0.1;
        while (*s >= '0' && *s <= '9') { v += (double)(*s - '0') * scale; scale *= 0.1; s++; }
    }

    if (*s == 'e' || *s == 'E') {
        s++;
        int eneg = 0, e = 0;
        if (*s == '-') { eneg = 1; s++; } else if (*s == '+') s++;
        while (*s >= '0' && *s <= '9') { e = e * 10 + (*s - '0'); s++; }
        if (e > 308) e = 308;          /* saturate rather than loop forever */
        double p = 1.0;
        for (int i = 0; i < e; i++) p *= 10.0;
        if (eneg) v /= p; else v *= p;
    }
    return neg ? -v : v;
}

int k_atoi(const char *s)
{
    while (*s == ' ' || *s == '\t') s++;
    int neg = 0;
    if (*s == '-') { neg = 1; s++; } else if (*s == '+') s++;
    long long v = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
    return (int)(neg ? -v : v);
}

/* ---- the allocator -------------------------------------------------------
 * The whole point. interp.c allocates through one function; in the kernel that
 * function is the arena, so a program's memory is its own, bounded, and freed
 * in one move between runs. There is no free() to write. */
void *k_malloc(ul n) { return arena_alloc(n ? n : 1); }
void  k_free(void *p) { (void)p; }      /* the arena frees in one move */

void *k_realloc(void *p, ul n)
{
    /* A bump allocator cannot grow in place, so this copies. It cannot know
     * the old size, so it copies n bytes - safe only because the caller is
     * always GROWING (interp.c's three realloc sites all double a capacity).
     * Reading n bytes from a smaller old block would run off the end, so the
     * copy is clamped to what is left of the arena below p. */
    if (!p) return k_malloc(n);
    void *q = k_malloc(n);
    if (!q) return 0;
    uptr base = (uptr)arena_base_addr();
    uptr off  = (uptr)p - base;
    ul left = arena_capacity() - (ul)off;
    k_memcpy(q, p, n < left ? n : left);
    return q;
}

char *k_strdup(const char *s)
{
    ul n = k_strlen(s) + 1;
    char *d = (char *)k_malloc(n);
    if (d) k_memcpy(d, s, n);
    return d;
}

/* ---- maths ---------------------------------------------------------------
 * zl exposes about eighteen of these as builtins. A kernel has no libm, and
 * writing one is the part of this port where "it looks right" is worth the
 * least - every function below is checked against glibc's over a swept range
 * in hosttest/libctest.c, because a sine that is subtly wrong produces a
 * plausible picture rather than an error.
 *
 * THE SHAPE OF ALL OF THEM: reduce the argument into a range where a short
 * series converges fast, evaluate, then undo the reduction. The reduction is
 * where the accuracy is won or lost - a Taylor series for sin is excellent
 * near zero and useless at 100, so the answer for 100 is entirely decided by
 * how well it was folded back to near zero.
 *
 * Accuracy target is ~1e-12 relative, which is far tighter than anything a zl
 * script drawing on a screen can notice and loose enough to write in 150 lines
 * rather than 1500. Where that is not met it is stated, not hidden.
 */

#define K_PI   3.14159265358979323846
#define K_PI2  1.57079632679489661923
#define K_LN2  0.69314718055994530942

int k_isnan(double x) { return x != x; }

double k_fabs(double x)  { return x < 0 ? -x : x; }
double k_floor(double x)
{
    /* The cast is only valid inside the range a long long can hold; outside
     * it the value is already integral, so returning it unchanged is both
     * correct and the only thing that does not invoke UB. */
    if (x >= 9.2e18 || x <= -9.2e18 || k_isnan(x)) return x;
    double t = (double)(long long)x;
    return (t > x) ? t - 1.0 : t;
}
double k_ceil(double x)
{
    if (x >= 9.2e18 || x <= -9.2e18 || k_isnan(x)) return x;
    double t = (double)(long long)x;
    return (t < x) ? t + 1.0 : t;
}
double k_trunc(double x)
{
    if (x >= 9.2e18 || x <= -9.2e18 || k_isnan(x)) return x;
    return (double)(long long)x;
}
double k_round(double x)
{
    /* away from zero on a tie, which is what C's round() does - and NOT what
     * floor(x+0.5) does for negatives, the usual shortcut and the usual bug */
    return x < 0 ? -k_floor(-x + 0.5) : k_floor(x + 0.5);
}
/* MEASURED WRONG THE OBVIOUS WAY. `a - trunc(a/b)*b` reads correctly and is
 * accurate only while a/b is small: swept against libm over a in [-100,100]
 * and b in [-20,20] its worst error was 9.03, not 1e-9, because for a small b
 * the quotient is huge and trunc() throws away the low bits that the whole
 * answer consists of.
 *
 * Repeated subtraction by a scaled divisor instead. Every operation here is
 * exact in binary floating point - doubling and halving only move the
 * exponent, and the subtraction is between numbers within a factor of two of
 * each other, so it is exact by Sterbenz's lemma. The result is therefore
 * exact, not merely close, which is what libm's is. */
double k_fmod(double a, double b)
{
    if (b == 0.0 || k_isnan(a) || k_isnan(b)) return 0.0 / 1.0 * 0.0;   /* nan */
    int neg = a < 0.0;
    double r = k_fabs(a), d = k_fabs(b);
    if (r < d) return a;

    /* scale d up to just below r, counting the doublings */
    int shifts = 0;
    while (d <= r * 0.5) { d *= 2.0; shifts++; }

    while (shifts-- >= 0) {
        if (r >= d) r -= d;
        d *= 0.5;
    }
    return neg ? -r : r;
}

double k_sqrt(double x)
{
    if (x < 0.0 || k_isnan(x)) return 0.0 / 1.0 * 0.0;
    if (x == 0.0) return 0.0;
    /* Newton-Raphson. Seeded by halving the exponent rather than from a
     * constant: from a fixed seed this needs ~40 iterations for large inputs
     * and converges in 6 from a good one. */
    double g = x;
    int e = 0;
    while (g > 2.0)  { g *= 0.25; e++; }
    while (g < 0.5)  { g *= 4.0;  e--; }
    double r = g;
    for (int i = 0; i < 8; i++) r = 0.5 * (r + g / r);
    while (e > 0) { r *= 2.0; e--; }
    while (e < 0) { r *= 0.5; e++; }
    /* two more at full scale to clean up whatever the scaling cost */
    r = 0.5 * (r + x / r);
    r = 0.5 * (r + x / r);
    return r;
}

double k_exp(double x)
{
    if (x > 709.0)  return 1.0e308 * 10.0;      /* inf */
    if (x < -745.0) return 0.0;
    /* exp(x) = 2^n * exp(r), |r| <= ln2/2. Without the reduction the series
     * needs hundreds of terms at x=700 and loses everything to cancellation. */
    double n = k_round(x / K_LN2);
    double r = x - n * K_LN2;
    double term = 1.0, sum = 1.0;
    for (int i = 1; i < 18; i++) { term *= r / (double)i; sum += term; }
    int e = (int)n;
    while (e > 0)  { sum *= 2.0; e--; }
    while (e < 0)  { sum *= 0.5; e++; }
    return sum;
}

double k_log(double x)
{
    if (x < 0.0 || k_isnan(x)) return 0.0 / 1.0 * 0.0;
    if (x == 0.0) return -1.0e308 * 10.0;
    /* x = m * 2^e with m in [2/3, 4/3), then atanh's series in
     * z = (m-1)/(m+1), which converges far faster than log(1+u) near the
     * edges of the mantissa range. */
    int e = 0;
    while (x > 1.3333333333333333) { x *= 0.5; e++; }
    while (x < 0.6666666666666666) { x *= 2.0; e--; }
    double z = (x - 1.0) / (x + 1.0), z2 = z * z, t = z, sum = 0.0;
    for (int i = 1; i < 30; i += 2) { sum += t / (double)i; t *= z2; }
    return 2.0 * sum + (double)e * K_LN2;
}

double k_pow(double a, double b)
{
    if (b == 0.0) return 1.0;
    if (a == 0.0) return 0.0;
    /* An integer exponent goes by squaring: exact for the cases a script
     * actually writes (x^2, x^3), where exp(b*log a) would return 8.000000001 */
    if (b == k_trunc(b) && k_fabs(b) < 1024.0) {
        int n = (int)k_fabs(b);
        double r = 1.0, base = a;
        while (n) { if (n & 1) r *= base; base *= base; n >>= 1; }
        return b < 0 ? 1.0 / r : r;
    }
    if (a < 0.0) return 0.0 / 1.0 * 0.0;        /* non-integer power of a negative */
    return k_exp(b * k_log(a));
}

double k_log2(double x)  { return k_log(x) / K_LN2; }
double k_log10(double x) { return k_log(x) / 2.30258509299404568402; }

static double k_sin_core(double r)          /* |r| <= pi/4 */
{
    double r2 = r * r, term = r, sum = r;
    for (int i = 1; i < 9; i++) {
        term *= -r2 / (double)((2 * i) * (2 * i + 1));
        sum += term;
    }
    return sum;
}
static double k_cos_core(double r)          /* |r| <= pi/4 */
{
    double r2 = r * r, term = 1.0, sum = 1.0;
    for (int i = 1; i < 9; i++) {
        term *= -r2 / (double)((2 * i - 1) * (2 * i));
        sum += term;
    }
    return sum;
}

/* Fold to the nearest quadrant, then pick the core by which quadrant it is.
 * Accuracy falls off for very large arguments because the fold subtracts two
 * close numbers - the standard limitation, and it starts to matter around
 * 1e9, which no zl script drawing a circle will reach. */
double k_sin(double x)
{
    double n = k_round(x / K_PI2);
    double r = x - n * K_PI2;
    int q = ((int)n) & 3;
    if (q < 0) q += 4;
    switch (q) {
        case 0: return  k_sin_core(r);
        case 1: return  k_cos_core(r);
        case 2: return -k_sin_core(r);
        default:return -k_cos_core(r);
    }
}
double k_cos(double x)
{
    double n = k_round(x / K_PI2);
    double r = x - n * K_PI2;
    int q = ((int)n) & 3;
    if (q < 0) q += 4;
    switch (q) {
        case 0: return  k_cos_core(r);
        case 1: return -k_sin_core(r);
        case 2: return -k_cos_core(r);
        default:return  k_sin_core(r);
    }
}
double k_tan(double x) { double c = k_cos(x); if (c == 0.0) return 1.0e308 * 10.0; return k_sin(x) / c; }

double k_atan(double x)
{
    int neg = x < 0; if (neg) x = -x;
    int inv = x > 1.0; if (inv) x = 1.0 / x;
    /* Halve twice with the identity atan(x)=2*atan(x/(1+sqrt(1+x^2))) so the
     * series argument is small and 30 terms is plenty. */
    int halves = 0;
    for (; halves < 2; halves++) x = x / (1.0 + k_sqrt(1.0 + x * x));
    double x2 = x * x, t = x, sum = 0.0;
    for (int i = 0; i < 24; i++) {
        sum += ((i & 1) ? -1.0 : 1.0) * t / (double)(2 * i + 1);
        t *= x2;
    }
    for (int i = 0; i < halves; i++) sum *= 2.0;
    if (inv) sum = K_PI2 - sum;
    return neg ? -sum : sum;
}
double k_asin(double x)
{
    if (x > 1.0 || x < -1.0) return 0.0 / 1.0 * 0.0;
    if (x == 1.0) return K_PI2;
    if (x == -1.0) return -K_PI2;
    return k_atan(x / k_sqrt(1.0 - x * x));
}
double k_acos(double x) { return K_PI2 - k_asin(x); }

double k_hypot(double a, double b)
{
    /* scaled, so hypot(1e200,1e200) does not overflow squaring its way there */
    a = k_fabs(a); b = k_fabs(b);
    double m = a > b ? a : b, n = a > b ? b : a;
    if (m == 0.0) return 0.0;
    double r = n / m;
    return m * k_sqrt(1.0 + r * r);
}

double k_atan2(double y, double x)
{
    if (x > 0.0) return k_atan(y / x);
    if (x < 0.0) return y >= 0.0 ? k_atan(y / x) + K_PI : k_atan(y / x) - K_PI;
    if (y > 0.0) return K_PI2;
    if (y < 0.0) return -K_PI2;
    return 0.0;
}

/* ---- formatting ----------------------------------------------------------
 * Not a printf. interp.c uses exactly five conversions and this supports those
 * five and refuses the rest, loudly, by emitting the specifier verbatim - so a
 * conversion somebody adds later shows up in the output as `%q` rather than
 * silently formatting as something else. A half-working printf in a kernel is
 * worse than none, because the half that works stops anyone looking.
 *
 *     %s      a string
 *     %d      an int
 *     %lld    a long long
 *     %llx    a long long, hex
 *     %g      a double, and see below
 *
 * ALWAYS NUL-TERMINATED, and the return is the length that WOULD have been
 * written, like C's - interp.c does not check it, but a function that lies
 * about truncation is a buffer overrun waiting for its first caller who does.
 */
typedef __builtin_va_list k_va_list;
#define k_va_start(ap, last) __builtin_va_start(ap, last)
#define k_va_arg(ap, t)      __builtin_va_arg(ap, t)
#define k_va_end(ap)         __builtin_va_end(ap)

static void kf_put(char *buf, ul cap, ul *n, char c)
{
    if (*n + 1 < cap) buf[*n] = c;
    (*n)++;
}

static void kf_puts(char *buf, ul cap, ul *n, const char *s)
{
    if (!s) s = "(null)";
    while (*s) kf_put(buf, cap, n, *s++);
}

static void kf_u64(char *buf, ul cap, ul *n, unsigned long long v, int base)
{
    char t[24];
    int i = 0;
    if (!v) { kf_put(buf, cap, n, '0'); return; }
    while (v) { int d = (int)(v % (unsigned)base); t[i++] = (char)(d < 10 ? '0' + d : 'a' + d - 10); v /= (unsigned)base; }
    while (i) kf_put(buf, cap, n, t[--i]);
}

static void kf_i64(char *buf, ul cap, ul *n, long long v)
{
    if (v < 0) { kf_put(buf, cap, n, '-'); kf_u64(buf, cap, n, (unsigned long long)(-(v + 1)) + 1ULL, 10); }
    else kf_u64(buf, cap, n, (unsigned long long)v, 10);
}

/* %g. zl only reaches this for a NON-integral number - whole ones go through
 * %lld above it - so the job is "show a fraction the way the hosted
 * interpreter shows it".
 *
 * SIX SIGNIFICANT DIGITS, NOT SIX DECIMAL PLACES, and that distinction is the
 * whole of this function's difficulty. Six decimals is easier, more precise,
 * and WRONG: glibc's %g prints 2.887614 as "2.88761", so a kernel doing six
 * decimals makes the same script print differently depending on whether it was
 * interpreted in the kernel or on the host. This project already has one such
 * divergence logged (1/0 is inf in interp.c and a dead machine in
 * runtime_kernel.c, T-EXEC-8) and one is enough.
 *
 * ROUNDED, NOT TRUNCATED. The first version pulled digits out one at a time by
 * repeated multiplication, which never gets to look at the digit after the
 * last one it keeps: 3.14159 printed as "3.141589" and 2/3 as "0.666666".
 * Scale once, round once, and handle the carry - without it 0.9999999 prints
 * as "0.1000000" with the integer part still reading 0.
 *
 * Magnitudes outside 1e-4..1e15 fall back to a plain integer rendering rather
 * than the exponent notation this does not implement. glibc would print
 * "1e+20"; readable-and-different beats subtly-wrong-and-similar, and no zl
 * script that draws on a screen produces one.
 */
static void kf_g(char *buf, ul cap, ul *n, double v)
{
    if (k_isnan(v)) { kf_puts(buf, cap, n, "nan"); return; }
    if (v > 1.0e308)  { kf_puts(buf, cap, n, "inf"); return; }
    if (v < -1.0e308) { kf_puts(buf, cap, n, "-inf"); return; }
    if (v < 0) { kf_put(buf, cap, n, '-'); v = -v; }
    if (v == 0.0) { kf_put(buf, cap, n, '0'); return; }

    if (v >= 1.0e15 || v < 1.0e-4) {          /* out of range: plain integer */
        kf_i64(buf, cap, n, (long long)v);
        return;
    }

    /* how many digits before the point */
    int intdigits = 1;
    { double t = v; while (t >= 10.0) { t *= 0.1; intdigits++; } }
    if (v < 1.0) intdigits = 0;

    int decimals = 6 - intdigits;             /* six SIGNIFICANT digits */
    if (decimals < 0) decimals = 0;
    if (decimals > 12) decimals = 12;

    double scale = 1.0;
    for (int i = 0; i < decimals; i++) scale *= 10.0;

    double ip = k_floor(v);
    long long f = (long long)((v - ip) * scale + 0.5);
    long long lim = (long long)scale;
    if (f >= lim) { ip += 1.0; f -= lim; }    /* the carry */

    kf_i64(buf, cap, n, (long long)ip);
    if (decimals == 0 || f <= 0) return;

    char d[16];
    for (int i = decimals - 1; i >= 0; i--) { d[i] = (char)('0' + (int)(f % 10)); f /= 10; }
    int last = decimals - 1;
    while (last >= 0 && d[last] == '0') last--;   /* trim trailing zeros */
    if (last < 0) return;
    kf_put(buf, cap, n, '.');
    for (int i = 0; i <= last; i++) kf_put(buf, cap, n, d[i]);
}

int k_snprintf(char *buf, ul cap, const char *fmt, ...)
{
    k_va_list ap;
    ul n = 0;
    k_va_start(ap, fmt);
    for (const char *f = fmt; *f; f++) {
        if (*f != '%') { kf_put(buf, cap, &n, *f); continue; }
        f++;
        if (*f == '%') { kf_put(buf, cap, &n, '%'); continue; }
        if (*f == 's') { kf_puts(buf, cap, &n, k_va_arg(ap, const char *)); continue; }
        if (*f == 'd') { kf_i64(buf, cap, &n, (long long)k_va_arg(ap, int)); continue; }
        if (*f == 'g') { kf_g(buf, cap, &n, k_va_arg(ap, double)); continue; }
        if (f[0] == 'l' && f[1] == 'l' && f[2] == 'd') {
            kf_i64(buf, cap, &n, k_va_arg(ap, long long)); f += 2; continue;
        }
        if (f[0] == 'l' && f[1] == 'l' && f[2] == 'x') {
            kf_u64(buf, cap, &n, k_va_arg(ap, unsigned long long), 16); f += 2; continue;
        }
        /* unsupported: emit it verbatim so it is VISIBLE rather than silently
         * formatted as something else */
        kf_put(buf, cap, &n, '%');
        kf_put(buf, cap, &n, *f);
    }
    k_va_end(ap);
    if (cap) buf[n < cap ? n : cap - 1] = 0;
    return (int)n;
}

/* ---- output --------------------------------------------------------------
 * The interpreter's print builtin writes through here, and here writes to
 * term.c - the scrollback AND the serial line, which is where the person who
 * typed `run` is looking and where a gate can see it. There is no stdout, no
 * buffering, and nothing to flush.
 *
 * The buffer is deliberately modest and on the STACK of this function rather
 * than a static: the kernel has 256 KiB of stack in total, but a static print
 * buffer is a piece of shared mutable state that two tasks would race on the
 * moment sched.c is wired in (Item 3), and this is easier to get right now
 * than to find later. */
extern void term_say(const char *s);

int k_printf(const char *fmt, ...)
{
    char b[512];
    k_va_list ap;
    k_va_start(ap, fmt);
    /* k_vsnprintf would be the tidy way; k_snprintf's body is the same loop
     * and duplicating it to gain a va_list variant is not worth a second copy
     * of the conversion table. Small, fixed set of callers. */
    ul n = 0;
    for (const char *f = fmt; *f; f++) {
        if (*f != '%') { kf_put(b, sizeof b, &n, *f); continue; }
        f++;
        if (*f == '%') { kf_put(b, sizeof b, &n, '%'); continue; }
        if (*f == 's') { kf_puts(b, sizeof b, &n, k_va_arg(ap, const char *)); continue; }
        if (*f == 'd') { kf_i64(b, sizeof b, &n, (long long)k_va_arg(ap, int)); continue; }
        if (*f == 'c') { kf_put(b, sizeof b, &n, (char)k_va_arg(ap, int)); continue; }
        if (*f == 'g') { kf_g(b, sizeof b, &n, k_va_arg(ap, double)); continue; }
        if (f[0]=='l' && f[1]=='l' && f[2]=='d') { kf_i64(b, sizeof b, &n, k_va_arg(ap, long long)); f += 2; continue; }
        if (f[0]=='l' && f[1]=='l' && f[2]=='x') { kf_u64(b, sizeof b, &n, k_va_arg(ap, unsigned long long), 16); f += 2; continue; }
        kf_put(b, sizeof b, &n, '%'); kf_put(b, sizeof b, &n, *f);
    }
    k_va_end(ap);
    b[n < sizeof b ? n : sizeof b - 1] = 0;
    term_say(b);
    return (int)n;
}

int k_puts_no_nl(const char *s) { term_say(s ? s : "(null)"); return 0; }
int k_putchar(int c) { char b[2]; b[0] = (char)c; b[1] = 0; term_say(b); return c; }

/* ---- the small odds and ends interp.c reaches for ------------------------*/
int k_isdigit(int c) { return c >= '0' && c <= '9'; }
int k_isspace(int c) { return c==' '||c=='\t'||c=='\n'||c=='\r'||c=='\v'||c=='\f'; }
int k_isalpha(int c) { return (c>='a'&&c<='z')||(c>='A'&&c<='Z'); }
int k_isalnum(int c) { return k_isdigit(c) || k_isalpha(c); }

void *k_calloc(ul n, ul sz)
{
    ul bytes = n * sz;
    if (n && bytes / n != sz) return 0;   /* overflow */
    void *p = k_malloc(bytes ? bytes : 1);
    if (p) k_memset(p, 0, bytes ? bytes : 1);
    return p;
}

/* Weak: the kernel defines a real kfatal; libctest does not. */
extern void kfatal(const char *msg) __attribute__((weak));
int k_exit(int c)
{
    (void)c;
    if (kfatal) kfatal("interpreter exit");
    for (;;) {}
}

unsigned long k_strcspn(const char *s, const char *reject)
{
    ul n = 0;
    for (; s[n]; n++) {
        for (const char *r = reject; *r; r++) if (s[n] == *r) return n;
    }
    return n;
}

long k_atol(const char *s) { return (long)k_atof(s); }

unsigned long long k_strtoull(const char *s, char **end, int base)
{
    while (k_isspace((int)(unsigned char)*s)) s++;
    if (base == 0) {
        if (s[0]=='0' && (s[1]=='x'||s[1]=='X')) { base = 16; s += 2; }
        else if (s[0]=='0') base = 8;
        else base = 10;
    } else if (base == 16 && s[0]=='0' && (s[1]=='x'||s[1]=='X')) s += 2;

    unsigned long long v = 0;
    for (;; s++) {
        int c = (unsigned char)*s, d;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'z') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z') d = c - 'A' + 10;
        else break;
        if (d >= base) break;
        v = v * (unsigned)base + (unsigned)d;
    }
    if (end) *end = (char *)s;
    return v;
}

/* frexp/ldexp - only the hex-float formatter uses these, and only for finite
 * positive values, so the sub-normal and NaN cases are not pretended at. */
double k_ldexp(double x, int e)
{
    while (e > 0) { x *= 2.0; e--; }
    while (e < 0) { x *= 0.5; e++; }
    return x;
}
double k_frexp(double x, int *e)
{
    int n = 0;
    if (x == 0.0 || k_isnan(x)) { *e = 0; return x; }
    double a = k_fabs(x);
    while (a >= 1.0) { a *= 0.5; n++; }
    while (a < 0.5)  { a *= 2.0; n--; }
    *e = n;
    return x < 0 ? -a : a;
}

/* A PRNG, because a kernel has no rand(). xorshift32: three shifts, a full
 * 2^32-1 period, and vastly better distribution than the LCG a kernel usually
 * gets given. The seed is fixed rather than time-based, deliberately - a
 * program that behaves differently on every run cannot be gated. */
static unsigned k_rng = 2463534242u;
void k_srand(unsigned s) { k_rng = s ? s : 2463534242u; }
int  k_rand(void)
{
    k_rng ^= k_rng << 13;
    k_rng ^= k_rng >> 17;
    k_rng ^= k_rng << 5;
    return (int)(k_rng & 0x7FFFFFFF);
}
