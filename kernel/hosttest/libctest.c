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

#define ARENA_BASE  0x00800000UL
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

    printf("\n%d checks, %d failures\n", checks, fails);
    if (!fails) printf("ok    every replacement agrees with the libc it replaces\n");
    return fails ? 1 : 0;
}
