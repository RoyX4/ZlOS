/* zl_freestanding.h - what interp.c gets instead of libc, in the kernel.
 *
 * interp.c's body is unchanged between the two builds. On the host it links
 * glibc; here every name it uses is redirected to interp_kernel.c's, each of
 * which is checked against the real libc's over tens of thousands of generated
 * inputs (kernel/tests/host/libctest.c, 43 checks).
 *
 * The redirection is macros rather than #ifdefs through the interpreter,
 * because the alternative is a second copy of it that the hosted test suite
 * never runs - and the two drift the first time either is fixed.
 */
#ifndef ZL_FREESTANDING_H
#define ZL_FREESTANDING_H

typedef unsigned long size_t;
#ifndef NULL
#define NULL ((void *)0)
#endif

/* interp.c takes addresses through a double and casts them; it needs the same
 * integer type the rest of the kernel uses for that. */
#if defined(__UINTPTR_TYPE__)
typedef __UINTPTR_TYPE__ uintptr_t;
#else
typedef unsigned long    uintptr_t;
#endif

/* ---- memory and strings (interp_kernel.c) --------------------------------*/
void *k_memcpy(void *, const void *, unsigned long);
void *k_memmove(void *, const void *, unsigned long);
void *k_memset(void *, int, unsigned long);
int   k_memcmp(const void *, const void *, unsigned long);
unsigned long k_strlen(const char *);
int   k_strcmp(const char *, const char *);
int   k_strncmp(const char *, const char *, unsigned long);
char *k_strcpy(char *, const char *);
char *k_strncpy(char *, const char *, unsigned long);
char *k_strcat(char *, const char *);
char *k_strchr(const char *, int);
char *k_strstr(const char *, const char *);
double k_atof(const char *);
int   k_atoi(const char *);
void *k_malloc(unsigned long);
void *k_realloc(void *, unsigned long);
void  k_free(void *);
char *k_strdup(const char *);

#define memcpy   k_memcpy
#define memmove  k_memmove
#define memset   k_memset
#define memcmp   k_memcmp
#define strlen   k_strlen
#define strcmp   k_strcmp
#define strncmp  k_strncmp
#define strcpy   k_strcpy
#define strncpy  k_strncpy
#define strcat   k_strcat
#define strchr   k_strchr
#define strstr   k_strstr
#define atof     k_atof
#define atoi     k_atoi
#define atoll(s) ((long long)k_atof(s))
#define malloc   k_malloc
#define realloc  k_realloc
#define free     k_free
#define _strdup  k_strdup
#define strdup   k_strdup

/* ---- maths (interp_kernel.c) ---------------------------------------------*/
double k_sqrt(double);  double k_exp(double);   double k_log(double);
double k_log2(double);  double k_log10(double); double k_sin(double);
double k_cos(double);   double k_tan(double);   double k_atan(double);
double k_asin(double);  double k_acos(double);  double k_floor(double);
double k_ceil(double);  double k_trunc(double); double k_round(double);
double k_fabs(double);  double k_pow(double, double);
double k_hypot(double, double); double k_fmod(double, double);
int    k_isnan(double);

#define sqrt  k_sqrt
#define exp   k_exp
#define log   k_log
#define log2  k_log2
#define log10 k_log10
#define sin   k_sin
#define cos   k_cos
#define tan   k_tan
#define atan  k_atan
#define asin  k_asin
#define acos  k_acos
#define floor k_floor
#define ceil  k_ceil
#define trunc k_trunc
#define round k_round
#define fabs  k_fabs
#define pow   k_pow
#define hypot k_hypot
#define fmod  k_fmod
#define isnan k_isnan
/* atan2 is only used for angles; the quadrant fix-up is the whole of it */
double k_atan2(double, double);
#define atan2 k_atan2

/* ---- the error boundary (ksetjmp.S) --------------------------------------*/
typedef unsigned long zi_jmp_buf[8];
int  ksetjmp(zi_jmp_buf);
void klongjmp(zi_jmp_buf, int) __attribute__((noreturn));
#define zi_setjmp  ksetjmp
#define zi_longjmp klongjmp

/* ---- output --------------------------------------------------------------
 * The interpreter prints through the terminal, which is where the person who
 * typed `run` is looking. There is no stdout, no stderr and no buffering, so
 * fflush is a no-op and printf-family calls are compiled out at their sites
 * rather than emulated - a half-working printf in a kernel is worse than none.
 */
void term_say(const char *s);
int  k_snprintf(char *buf, unsigned long cap, const char *fmt, ...);
int  k_printf(const char *fmt, ...);
int  k_puts_no_nl(const char *);
int  k_putchar(int);
#define snprintf  k_snprintf
#define printf    k_printf
#define fflush(x) ((void)0)
/* stderr/stdout are the same place - the terminal. The stream argument is
 * discarded rather than emulated, because a kernel that pretended to have two
 * output streams and had one would be lying in the one place it matters. */
#define stdout    ((void *)0)
#define stderr    ((void *)0)
#define fprintf(stream, ...)  k_printf(__VA_ARGS__)
#define fputs(s, stream)      k_puts_no_nl(s)
#define fputc(c, stream)      k_putchar(c)
#define putchar(c)            k_putchar(c)

/* ---- the small odds and ends (interp_kernel.c) ---------------------------*/
int k_isdigit(int); int k_isspace(int); int k_isalpha(int);
unsigned long k_strcspn(const char *, const char *);
long k_atol(const char *);
unsigned long long k_strtoull(const char *, char **, int);
double k_ldexp(double, int);
double k_frexp(double, int *);
void k_srand(unsigned);
int  k_rand(void);

#define isdigit  k_isdigit
#define isspace  k_isspace
#define isalpha  k_isalpha
#define strcspn  k_strcspn
#define atol     k_atol
#define strtoull k_strtoull
#define ldexp    k_ldexp
#define frexp    k_frexp
#define srand    k_srand
#define rand     k_rand
#define RAND_MAX 0x7FFFFFFF
#define HUGE_VAL (1.0e308 * 10.0)

#endif /* ZL_FREESTANDING_H */
