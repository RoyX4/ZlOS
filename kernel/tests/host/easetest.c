/* easetest.c - is ease.c actually the reference's motion, or just smooth?
 *
 * A curve is easy to get wrong in a way that still looks fine in a screenshot
 * and wrong in motion, and there is no screenshot test for "feels flicked".
 * So this checks the curve against properties that distinguish the reference's
 * curves from the smoothstep they replaced, and against values computed
 * independently in double precision from the same control points.
 *
 * Build and run:  ./build.sh && ./easetest
 */

#include <stdio.h>
#include <math.h>

#include "../../src/graphics/ui/ease.h"

static int fails;
static void ok(int cond, const char *what)
{
    printf("  %s %s\n", cond ? "ok  " : "FAIL", what);
    if (!cond) fails++;
}

/* The independent implementation: the same cubic-bezier, in double, solved by
 * a different method (many small steps rather than bisection). If ease.c's
 * fixed-point bisection agrees with this to within a thousandth, both are
 * almost certainly right - they share the formula but not the arithmetic. */
static double bez_d(double t, double p1, double p2)
{
    double u = 1 - t;
    return 3 * u * u * t * p1 + 3 * u * t * t * p2 + t * t * t;
}
static double ref_bezier(double x, double x1, double y1, double x2, double y2)
{
    double lo = 0, hi = 1;
    for (int i = 0; i < 60; i++) {
        double mid = (lo + hi) / 2;
        if (bez_d(mid, x1, x2) < x) lo = mid; else hi = mid;
    }
    return bez_d((lo + hi) / 2, y1, y2);
}

/* smoothstep, i.e. what wm.c used for everything before this. */
static int smoothstep(int p) { return p * p / 1000 * (3000 - 2 * p) / 1000; }

int main(void)
{
    printf("easetest - the reference's curves, not merely smooth ones\n\n");

    /* ---- endpoints, for every curve --------------------------------------- */
    int ends = 1;
    for (int c = EASE_LINEAR; c <= EASE_WIN; c++) {
        if (ease_apply(c, 0) != 0)       ends = 0;
        if (ease_apply(c, 1000) != 1000) ends = 0;
        if (ease_apply(c, -50) != 0)     ends = 0;   /* clamps, not wraps */
        if (ease_apply(c, 5000) != 1000) ends = 0;
    }
    ok(ends, "every curve starts at 0, ends at 1000, and clamps outside");

    /* ---- monotonic: no curve may go backwards ------------------------------ */
    int mono = 1;
    for (int c = EASE_LINEAR; c <= EASE_WIN; c++) {
        int prev = -1;
        for (int x = 0; x <= 1000; x += 1) {
            int v = ease_apply(c, x);
            if (v < prev) { mono = 0; break; }
            prev = v;
        }
    }
    ok(mono, "no curve ever moves backwards");

    /* ---- agreement with an independent double-precision solve -------------- */
    double worst = 0; int worst_x = 0;
    for (int x = 0; x <= 1000; x += 1) {
        double want = ref_bezier(x / 1000.0, 0.2, 0.85, 0.3, 1.0) * 1000.0;
        double got  = ease_apply(EASE_WIN, x);
        double d = fabs(want - got);
        if (d > worst) { worst = d; worst_x = x; }
    }
    printf("    zwin worst disagreement with the double solve: %.2f/1000 at x=%d\n",
           worst, worst_x);
    ok(worst <= 2.0, "zwin matches cubic-bezier(.2,.85,.3,1) to within 2/1000");

    worst = 0;
    for (int x = 0; x <= 1000; x += 1) {
        double want = ref_bezier(x / 1000.0, 0, 0, 0.58, 1.0) * 1000.0;
        double d = fabs(want - ease_apply(EASE_OUT, x));
        if (d > worst) worst = d;
    }
    ok(worst <= 2.0, "ease-out matches cubic-bezier(0,0,.58,1)");

    /* ---- THE POINT OF THE WHOLE FILE --------------------------------------
     * zwin must NOT be the smoothstep it replaced. They agree at both ends and
     * near the middle, so an equality test at three points would pass on the
     * wrong curve. What actually separates them is the FIRST QUARTER: the
     * reference curve is already past 60% by x=250 because its first control
     * point is high (y1=.85); smoothstep is at about 16%. That early lead is
     * the "flicked" feeling. Assert it. */
    int win_q = ease_apply(EASE_WIN, 250), ss_q = smoothstep(250);
    printf("    at x=250:  zwin %d/1000   smoothstep %d/1000\n", win_q, ss_q);
    ok(win_q > 550, "zwin is more than half done a quarter of the way through");
    ok(win_q - ss_q > 300, "zwin leads smoothstep by a wide margin early on");

    /* ease-out likewise must not be symmetric: it covers more than half the
     * distance in the first half. ease-in-out covers exactly half. */
    ok(ease_apply(EASE_OUT, 500) > 600, "ease-out is front-loaded");
    ok(ease_apply(EASE_IN_OUT, 500) >= 495 && ease_apply(EASE_IN_OUT, 500) <= 505,
       "ease-in-out is symmetric about its midpoint");
    ok(ease_apply(EASE_LINEAR, 371) == 371, "linear is the identity");

    /* ---- zpress: a dip, not a ramp ---------------------------------------- */
    int p0 = ease_press_scale(0), pm = ease_press_scale(EASE_PRESS_MID),
        p1000 = ease_press_scale(1000);
    printf("    zpress: %d -> %d at 45%% -> %d\n", p0, pm, p1000);
    ok(p0 == 1000 && p1000 == 1000, "zpress starts and ends at full scale");
    ok(pm == EASE_PRESS_MIN, "zpress reaches exactly .88 at 45%");
    int dipped = 1;
    for (int x = 1; x < 1000; x++)
        if (ease_press_scale(x) > 1000) dipped = 0;   /* never overshoots */
    ok(dipped, "zpress never scales above 1 - it is a dip, not a bounce");

    /* ---- zpulse: never reaches 0, never exceeds 1 -------------------------- */
    int lo = 100000, hi = -1;
    for (int x = 0; x <= 1000; x++) {
        int v = ease_pulse(x);
        if (v < lo) lo = v;
        if (v > hi) hi = v;
    }
    printf("    zpulse ranges %d..%d/1000\n", lo, hi);
    ok(lo == EASE_PULSE_FLOOR, "zpulse floors at .55, so nothing ever vanishes");
    ok(hi == 1000, "zpulse reaches full opacity at the midpoint");

    /* ---- NEGATIVE CONTROL --------------------------------------------------
     * Every assertion above would also pass if ease_apply returned x for
     * everything - linear is monotonic, hits both endpoints, and is its own
     * identity. These prove the curves are actually DIFFERENT from each other
     * and from the input. */
    ok(ease_apply(EASE_WIN, 250) != 250 && ease_apply(EASE_OUT, 250) != 250,
       "control: the curves are not all secretly linear");
    ok(ease_apply(EASE_WIN, 250) != ease_apply(EASE_OUT, 250),
       "control: zwin and ease-out are distinguishable");
    ok(ease_bezier(500, 0, 0, 1000, 1000) == 500,
       "control: the general solver reproduces linear from its control points");

    printf("\n%s (%d failure%s)\n", fails ? "FAILED" : "all passed",
           fails, fails == 1 ? "" : "s");
    return fails ? 1 : 0;
}
