/* ease.c - the reference's motion curves, in fixed point, with no FPU.
 *
 * WHY THIS FILE EXISTS. wm.c animated everything with one smoothstep:
 *
 *     p * p / 1000 * (3000 - 2 * p) / 1000
 *
 * which is a symmetric ease-in-out. The reference uses five DIFFERENT curves
 * and the difference is the whole character of the motion - a window that
 * eases in as well as out feels sluggish where the reference feels flicked.
 * docs/design/ds-reference.html lines 14-20 name seven animations:
 *
 *   zwin    .2s  cubic-bezier(.2,.85,.3,1)   window open
 *   zpop    .08/.1/.11s ease-out             menus, chips, tiles
 *   zpress  .25s ease                        button press
 *   zpulse  1s / 2.6s ease-in-out infinite   activating / unfocused
 *   zov     .16s ease-out                    overlays, modals
 *   ztoast  .16s ease-out                    toasts
 *   zsweep  7s linear infinite               the wallpaper sweep
 *
 * ONE EVALUATOR COVERS ALL OF THEM, because CSS's named timing functions are
 * themselves cubic-beziers:
 *
 *   linear        (identity - handled without the solver)
 *   ease          cubic-bezier(.25, .1,  .25, 1)
 *   ease-out      cubic-bezier(0,   0,   .58, 1)
 *   ease-in-out   cubic-bezier(.42, 0,   .58, 1)
 *
 * so "implement the reference's motion" is one function and five constants,
 * not five hand-tuned polynomials that drift apart.
 *
 * THE MATH. A CSS cubic-bezier is a curve through (0,0) and (1,1) with two
 * control points (x1,y1) and (x2,y2). It is NOT y = f(x): both coordinates are
 * cubic functions of a parameter t. So evaluating it at a time x means first
 * solving bezier_x(t) = x for t, then returning bezier_y(t).
 *
 * bezier_x is monotonic on [0,1] whenever x1 and x2 are in [0,1] - which CSS
 * requires - so BISECTION always converges and cannot diverge the way Newton
 * can near a flat spot. 18 halvings take the interval below 1/262144, far under
 * the 1/1000 the caller can express. It costs ~54 multiplies per call and runs
 * a handful of times per frame; that is not worth a lookup table, and a table
 * would have to be regenerated whenever a curve changed.
 *
 * FIXED POINT. Everything is in thousandths: 0 == 0.0, 1000 == 1.0. Products
 * of two such values are divided by 1000 to stay in range. All intermediates
 * fit in 32 bits: the largest is 3 * 1000 * 1000 = 3e9 before division, which
 * would overflow a signed 32-bit int, so the cubic is evaluated with the
 * divisions interleaved rather than at the end. That is the one place in this
 * file where the obvious rewrite is wrong.
 */

#include "ease.h"

/* B(t) = 3(1-t)^2 t P1 + 3(1-t) t^2 P2 + t^3, with P0 = 0 and P3 = 1.
 *
 * THE PARAMETER IS IN HUNDRED-THOUSANDTHS and the result in MILLIONTHS, both
 * finer than the thousandths the API speaks. That is not gold-plating, it is
 * the fix for a real defect this file shipped first time round: with t in
 * thousandths and the divisions interleaved to stay inside 32 bits, the
 * truncation error reached 6/1000 and made the curve non-monotonic - it went
 * BACKWARDS in places, which is a visible stutter. easetest.c caught it.
 *
 * The bounds, so the next person can check them rather than trust them:
 *   u, t <= 1e5      p1, p2 <= 1e3
 *   3*u*u*t*p1       <= 3 * 1e10 * 1e5 * 1e3 = 3e18
 *   two such terms   <= 6e18                  < 9.22e18 = INT64_MAX
 * so a single 64-bit division at the end is safe, and no intermediate needs
 * to be scaled down first. The 32-bit kernel gets __divdi3 from divmod.c.
 */
#define BEZ_T   100000      /* t domain: 0 .. BEZ_T */
#define BEZ_OUT 1000000     /* result domain: 0 .. BEZ_OUT */

static long long bez(int t5, int p1, int p2)
{
    long long u = BEZ_T - t5, t = t5;
    long long a = 3 * u * u * t * (long long)p1;   /* <= 3e18 */
    long long b = 3 * u * t * t * (long long)p2;   /* <= 3e18 */
    long long c = t * t * t;                       /* <= 1e15 */
    return (a + b) / 1000000000000LL + c / 1000000000LL;
}

int ease_bezier(int x, int x1, int y1, int x2, int y2)
{
    if (x <= 0) return 0;
    if (x >= 1000) return 1000;

    /* Solve bez_x(t) == x for t. bez_x is monotonic on [0,1] for any control
     * points CSS allows, so bisection always converges; Newton can stall on a
     * flat spot, which cubic-bezier(0,0,.58,1) has at the origin. 17 halvings
     * take the interval below 1 in BEZ_T. */
    long long target = (long long)x * (BEZ_OUT / 1000);
    int lo = 0, hi = BEZ_T;
    for (int i = 0; i < 17; i++) {
        int mid = (lo + hi) / 2;
        if (bez(mid, x1, x2) < target) lo = mid; else hi = mid;
    }
    long long y = bez((lo + hi) / 2, y1, y2);
    return (int)((y + 500) / 1000);            /* millionths -> thousandths */
}

int ease_apply(int curve, int x)
{
    if (x <= 0) return 0;
    if (x >= 1000) return 1000;
    switch (curve) {
    case EASE_LINEAR:  return x;
    case EASE_IN_OUT:  return ease_bezier(x, 420,   0, 580, 1000);
    case EASE_OUT:     return ease_bezier(x,   0,   0, 580, 1000);
    case EASE_STD:     return ease_bezier(x, 250, 100, 250, 1000);
    case EASE_WIN:     return ease_bezier(x, EASE_WIN_X1, EASE_WIN_Y1,
                                             EASE_WIN_X2, EASE_WIN_Y2);
    default:           return x;
    }
}

/* zpress is not a progress curve, it is a SHAPE: scale dips to .88 at 45% of
 * the duration and returns to 1. Reproduced as two eased halves rather than
 * one curve, because that is what a keyframe list with a stop in the middle
 * actually is. Returns a scale in thousandths. */
int ease_press_scale(int x)
{
    if (x <= 0 || x >= 1000) return 1000;
    if (x < EASE_PRESS_MID) {
        int t = ease_apply(EASE_STD, x * 1000 / EASE_PRESS_MID);
        return 1000 - (1000 - EASE_PRESS_MIN) * t / 1000;
    }
    int t = ease_apply(EASE_STD, (x - EASE_PRESS_MID) * 1000
                                 / (1000 - EASE_PRESS_MID));
    return EASE_PRESS_MIN + (1000 - EASE_PRESS_MIN) * t / 1000;
}

/* zpulse: opacity oscillates between a floor and 1 and back, forever.
 * `0%,100%{opacity:.55} 50%{opacity:1}` - so the triangle is eased, not the
 * raw ramp, or the turn at each end reads as a corner. Returns 0..1000. */
int ease_pulse(int x)
{
    int tri = x <= 500 ? x * 2 : (1000 - x) * 2;
    int e = ease_apply(EASE_IN_OUT, tri);
    return EASE_PULSE_FLOOR + (1000 - EASE_PULSE_FLOOR) * e / 1000;
}
