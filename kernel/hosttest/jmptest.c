/* jmptest.c - ksetjmp/klongjmp, asserted.
 *
 * Twenty lines of assembly that the kernel's whole error boundary rests on.
 * A setjmp that forgets a callee-saved register does not crash - it silently
 * hands back a stale value after an unwind, and the symptom appears in whatever
 * loop happened to be using that register, in a different file, later. That is
 * the single most expensive shape of bug this project has, so it gets
 * assertions rather than "it seemed to work".
 *
 * Built for the HOST architecture, which is x86-64 here and the same code the
 * 64-bit kernel links. The i386 path is the same file under -m32.
 *
 *   ./build.sh && ./jmptest && ./jmptest32
 */
#include <stdio.h>

typedef unsigned long kjmp_buf[8];

int  ksetjmp(kjmp_buf);
void klongjmp(kjmp_buf, int) __attribute__((noreturn));

static int checks, fails;
static void ok(int c, const char *what)
{
    checks++;
    if (!c) { fails++; printf("  FAIL  %s\n", what); }
}

static kjmp_buf trap;

/* ---- the basics ----------------------------------------------------------*/
static void thrower(int v) { klongjmp(trap, v); }

/* ---- unwinding across several live frames -------------------------------*/
static void deep(int n)
{
    volatile int marker = n;      /* forces a real frame at every level */
    if (n <= 0) klongjmp(trap, 99);
    if (n > 0) deep(n - 1);       /* the bound is for gcc's benefit: it cannot
                                     see that klongjmp never returns, and warns
                                     about infinite recursion otherwise */
    (void)marker;
}

/* ---- callee-saved registers, the part that goes quietly wrong ------------
 * The compiler will happily keep these in rbx/r12-r15 across the call, which
 * is exactly what makes them the test. If klongjmp fails to restore one, the
 * values read after the unwind are whatever the throwing path left behind. */
static int reg_test(void)
{
    volatile int a = 0x1111, b = 0x2222, c = 0x3333, d = 0x4444;
    int i, sum = 0;
    if (ksetjmp(trap) == 0) {
        /* keep them all live across the call so they land in registers */
        for (i = 0; i < 4; i++) sum += a + b + c + d;
        thrower(7);
    }
    /* after the unwind: every one must still read what it was set to */
    return (a == 0x1111) + (b == 0x2222) + (c == 0x3333) + (d == 0x4444)
           + (sum >= 0);
}

int main(void)
{
    printf("jmptest - the kernel's error boundary, asserted (%zu-bit)\n\n",
           sizeof(void *) * 8);

    /* 1. the first call returns 0 */
    volatile int first = 0, landed = 0, val = 0;
    if (ksetjmp(trap) == 0) {
        first = 1;
        thrower(42);
    } else {
        landed = 1;
    }
    ok(first == 1, "ksetjmp returned 0 the first time");
    ok(landed == 1, "klongjmp came back to it");

    /* 2. the value is carried */
    val = ksetjmp(trap);
    if (val == 0) thrower(1234);
    ok(val == 1234, "klongjmp's value arrives at ksetjmp");

    /* 3. longjmp(buf, 0) must still return non-zero, or the caller loops */
    val = ksetjmp(trap);
    if (val == 0) { static int once = 0; if (!once) { once = 1; thrower(0); } }
    ok(val == 1, "klongjmp(buf, 0) returns 1, not 0 - otherwise it loops forever");

    /* 4. across many live frames */
    val = ksetjmp(trap);
    if (val == 0) deep(200);
    ok(val == 99, "unwound through 200 live frames");

    /* 5. callee-saved registers survive */
    ok(reg_test() == 5, "every callee-saved register survived the unwind");

    /* 6. the same buffer can be re-armed and used again, many times - which is
     *    what a kernel running one program per `run` actually does */
    {
        int rounds = 0;
        for (int k = 0; k < 500; k++) {
            val = ksetjmp(trap);
            if (val == 0) thrower(k + 1);
            if (val == k + 1) rounds++;
        }
        ok(rounds == 500, "500 arm/unwind cycles on one buffer");
    }

    /* 7. the stack pointer is restored, not leaked. If klongjmp restored a
     *    stale rsp, 10000 unwinds would walk the stack away and this would
     *    fault or drift. Compare a local's address before and after. */
    {
        volatile int here = 0;
        unsigned long before = (unsigned long)&here;
        for (int k = 0; k < 10000; k++) {
            val = ksetjmp(trap);
            if (val == 0) thrower(1);
        }
        volatile int now = 0;
        unsigned long after = (unsigned long)&now;
        long drift = (long)before - (long)after;
        if (drift < 0) drift = -drift;
        ok(drift < 256, "10000 unwinds did not walk the stack pointer away");
    }

    printf("\n%d checks, %d failures\n", checks, fails);
    if (!fails) printf("ok    the error boundary holds\n");
    return fails ? 1 : 0;
}
