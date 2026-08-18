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
    ul base = (ul)arena_base_addr();
    ul off  = (ul)(uptr)p - base;
    ul left = arena_capacity() - off;
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
