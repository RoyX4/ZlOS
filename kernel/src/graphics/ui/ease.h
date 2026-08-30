/* ease.h - motion curves. See ease.c for why there is one evaluator.
 *
 * Everything here is in THOUSANDTHS: 0 == 0.0, 1000 == 1.0. That includes
 * time, progress, scale and opacity, so a caller never has to remember which
 * unit a particular number is in.
 *
 * This is pure math with no dependencies, so both wm.c (windows) and ui.c
 * (widgets) may call it without either one calling the other - which the
 * layering rule in ui.h would otherwise forbid.
 */
#ifndef ZL_EASE_H
#define ZL_EASE_H

/* The named curves. CSS's own timing functions, plus the reference's one
 * bespoke curve. Values in ease.c. */
/* PRESSWORK HAS ONE CURVE AND THESE FIVE ARE ITS PREDECESSOR'S.
 *
 * presswork-prototype.html:203-209 - "Graphite's three durations, one curve" -
 * declares `--ease: cubic-bezier(0.200, 0.850, 0.300, 1.000)` and every
 * transition and animation in it uses that one. EASE_WIN is that curve, so it
 * is the only one wm.c's table now selects.
 *
 * THE OTHER FOUR ARE KEPT, AND TWO OF THEM ARE STILL USED - which is not the
 * same statement as "wm.c selects them", and the first draft of this comment
 * said the wrong one. Checked rather than assumed:
 *
 *   EASE_LINEAR   anim_curve's NONE entry, which has no duration
 *   EASE_STD      ease_press_scale, INSIDE ease.c - it shapes the two halves
 *                 of zpress's keyframe dip, which is a shape and not a
 *                 timeline curve
 *   EASE_IN_OUT   ease_pulse, the same way - it eases the triangle so the turn
 *                 at each end is not a corner
 *   EASE_OUT      nothing selects it
 *
 * What proto:209 governs is which curve a TIMELINE runs on, and that is now
 * EASE_WIN everywhere. A keyframe list with a stop in the middle is a
 * different object and the authority states it separately. If a second
 * timeline curve is ever wanted, the argument has to be made against proto:209
 * rather than won by a table nobody reread. */
#define EASE_LINEAR  0   /* anim_curve's NONE only                    */
#define EASE_OUT     1   /* unused - the predecessor's common one     */
#define EASE_STD     2   /* CSS `ease` - shapes zpress inside ease.c  */
#define EASE_IN_OUT  3   /* shapes zpulse inside ease.c               */
#define EASE_WIN     4   /* THE curve - cubic-bezier(.2,.85,.3,1)     */

/* zwin's control points, x1 y1 x2 y2, thousandths.
 * presswork-prototype.html:209 - cubic-bezier(0.200, 0.850, 0.300, 1.000).
 * The figures are identical to ds-reference.html:15, which is why this line
 * survived the migration uncorrected; the CITATION was the stale half. */
#define EASE_WIN_X1  200
#define EASE_WIN_Y1  850
#define EASE_WIN_X2  300
#define EASE_WIN_Y2 1000

/* zpress: `45%{transform:scale(.88)}` - ds-reference.html line 16. */
#define EASE_PRESS_MID  450
#define EASE_PRESS_MIN  880

/* zpulse: `0%,100%{opacity:.55}` - ds-reference.html line 17. */
#define EASE_PULSE_FLOOR 550

/* The durations, in MILLISECONDS, exactly as the reference states them.
 * wm.c converts to timer ticks at the point of use; keeping these in ms means
 * a change to the PIT rate cannot silently change how the desktop feels. */
#define EASE_MS_WIN        200
#define EASE_MS_POP        100   /* .08 and .11 also occur; .1 is the mode */
#define EASE_MS_PRESS      250
#define EASE_MS_PULSE     1000
#define EASE_MS_PULSE_SLOW 2600
#define EASE_MS_OV         160
#define EASE_MS_SWEEP     7000

/* The transforms each animation starts from, thousandths.
 * zwin  from opacity 0, scale(.965) translateY(10px)
 * zpop  from opacity 0, translateY(-6px) scale(.98)
 * zov   from opacity 0, scale(1.03)   - note it starts ABOVE 1 */
#define EASE_POP_FROM_SCALE  980
#define EASE_POP_FROM_DY      -6   /* pixels, at 1x - upward */
#define EASE_OV_FROM_SCALE  1030
#define EASE_TOAST_FROM_DY     4   /* proto:966 translateY(4px) */

/* Evaluate a named curve at time x (0..1000). Returns progress 0..1000. */
int ease_apply(int curve, int x);

/* The general form, if a curve is needed that has no name here. */
int ease_bezier(int x, int x1, int y1, int x2, int y2);

/* zpress as a scale, and zpulse as an opacity. Both 0..1000. */
int ease_press_scale(int x);
int ease_pulse(int x);

#endif /* ZL_EASE_H */
