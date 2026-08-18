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
