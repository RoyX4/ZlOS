/* retest.c - the Regex Tester's matcher cannot hang the machine.
 *
 * WHAT THIS IS FOR. apps_utils.zl's pt_end_here is a backtracking matcher with
 * no memoisation. Its `*` arm tries every split of the subject between adjacent
 * stars, so a pattern with several stars against a non-matching subject is
 * EXPONENTIAL. It used to run inside app_draw - the compositor's synchronous
 * repaint path - where nothing can interrupt it: no timer, no way to close the
 * window, no way back. Thirteen characters in the pattern field froze the
 * machine until it was reset.
 *
 * WHY THE TEST IS IN C. The matcher is zl and this box cannot step a zl program
 * under a counter. This file is a FAITHFUL TRANSCRIPTION of pt_end_here - same
 * recursion, same lazy `*` loop, same `$` and `.` handling, same order of tests
 * - so the call counts it measures are the counts the kernel would spend. It
 * was checked against an independent transcription that produced identical
 * figures for six patterns.
 *
 * THE TRANSCRIPTION IS THE WEAK LINK and it is worth saying so plainly: if
 * pt_end_here is edited and this file is not, the test goes on passing while
 * measuring a matcher the kernel no longer has. There is no automatic tie
 * between them. Read both when changing either.
 *
 * WHAT IS ASSERTED
 *   1. the pathological patterns are genuinely pathological unbudgeted - the
 *      test proves its own premise rather than assuming it
 *   2. the budget bounds every one of them
 *   3. legitimate patterns cost the SAME with the budget as without, so the
 *      budget is not silently truncating real work
 *
 * Measured on this branch, subject = 48 'a' (PT_MAX, which the field allows):
 *
 *     pattern            unbudgeted    budgeted
 *     a*a*a*b               292,824      20,032
 *     a*a*a*a*b           3,162,509      20,081
 *     a*a*a*a*a*b        28,989,674      20,130
 *     a*a*a*a*a*a*b     231,917,399      20,179
 *     a.*b (legitimate)       1,273       1,273
 *     a*b  (legitimate)       1,274       1,274
 */
#include <stdio.h>
#include <string.h>

/* Must match PT_BUDGET in kernel/apps/apps_utils.zl. */
#define PT_BUDGET 20000
/* Must match PT_MAX in the same file - the longest subject the field accepts. */
#define PT_MAX 48

static const char *P, *T;
static int PL, TL;
static long steps;
static int budgeted;

static int end_here(int pi, int ti)
{
    steps++;
    if (budgeted && steps > PT_BUDGET) return -1;
    if (pi >= PL) return ti;
    int pc = (unsigned char)P[pi];
    if (pi + 1 < PL && P[pi + 1] == '*') {
        int n = ti;
        for (;;) {
            int e = end_here(pi + 2, n);
            if (e >= 0) return e;
            if (n >= TL) return -1;
            int tc = (unsigned char)T[n];
            if (pc != '.' && pc != tc) return -1;
            n++;
        }
    }
    if (pc == '$' && pi + 1 == PL) return (ti == TL) ? ti : -1;
    if (ti >= TL) return -1;
    int tc = (unsigned char)T[ti];
    if (pc != '.' && pc != tc) return -1;
    return end_here(pi + 1, ti + 1);
}

static long scan(const char *p, const char *t, int with_budget)
{
    P = p; T = t; PL = (int)strlen(p); TL = (int)strlen(t);
    steps = 0; budgeted = with_budget;
    for (int t0 = 0; t0 <= TL; t0++) {
        int e = end_here(0, t0);
        if (e >= 0) return steps;
        if (budgeted && steps > PT_BUDGET) return steps;
    }
    return steps;
}

static int failures;
static void ok(const char *what, int cond, long a, long b)
{
    printf(cond ? "  ok    %s   [%ld / %ld]\n" : "  FAIL  %s   [%ld / %ld]\n",
           what, a, b);
    if (!cond) failures++;
}

int main(void)
{
    char subj[PT_MAX + 1];
    memset(subj, 'a', PT_MAX);
    subj[PT_MAX] = 0;

    /* 1. the premise: these really are exponential without a budget. If this
     *    ever stops holding, the budget is guarding nothing and the rest of
     *    this file is measuring a problem that went away. */
    long bare7  = scan("a*a*a*b", subj, 0);
    long bare13 = scan("a*a*a*a*a*a*b", subj, 0);
    ok("unbudgeted, a 7-char pattern is already pathological",
       bare7 > 100000, bare7, 100000);
    ok("unbudgeted, 13 chars is three orders worse again",
       bare13 > 100000000L, bare13, 100000000L);

    /* 2. the budget bounds every one of them. The overshoot past PT_BUDGET is
     *    the recursion unwinding - bounded by depth, not by the pattern. */
    static const char *bad[] = {
        "a*a*a*b", "a*a*a*a*b", "a*a*a*a*a*b", "a*a*a*a*a*a*b",
        "a*a*a*a*a*a*a*a*b",
    };
    for (unsigned i = 0; i < sizeof bad / sizeof *bad; i++) {
        long n = scan(bad[i], subj, 1);
        ok("budgeted, the search stops", n < PT_BUDGET + 1000, n, PT_BUDGET + 1000);
    }

    /* 3. ...and honest patterns are untouched. A budget that clipped real work
     *    would trade a hang for a wrong answer, which is not a trade. */
    static const char *good[] = { "a.*b", "a*b", "aaa", "a$", ".*", "aab" };
    for (unsigned i = 0; i < sizeof good / sizeof *good; i++) {
        long u = scan(good[i], subj, 0);
        long b = scan(good[i], subj, 1);
        ok("legitimate pattern costs the same either way", u == b, u, b);
    }

    if (failures) printf("FAILED: %d failure(s)\n", failures);
    else          printf("all good: 0 failure(s)\n");
    return failures ? 1 : 0;
}
