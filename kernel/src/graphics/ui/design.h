/* design.h - THE design tokens for PRESSWORK, taken from the prototype and
 * from nowhere else.
 *
 * THE SOURCE IS docs/design/presswork-prototype.html, in this worktree. It is
 * a self-contained, running, pixel-verified prototype: its `:root` block is the
 * token ladder, its CSS is the geometry, and its settings pane recomputes the
 * contrast figures against the live tokens rather than quoting prose. Where
 * this header and the prototype disagree, THE PROTOTYPE WINS.
 *
 * WHAT PRESSWORK IS, in one paragraph. A warm-graphite machine lit by ONE
 * raking light entering from off-screen upper left, carrying the apparatus of
 * a printed technical document - a ruled module grid, crop marks, a register
 * mark, a memory ruler. Depth is a 1px lit top run plus a 1px cut bottom
 * groove. It is NEVER a drop shadow at rest: ZD_LIFT exists, and it is drawn
 * on a dragged plate and under the three things genuinely off the plane (menu,
 * palette sheet, toast), and nowhere else. A plate sitting still is separated
 * by the ladder and the two 1px runs alone.
 *
 * THE ONE STRUCTURAL DECISION - THE LADDER WAS WIDENED. Its parent ("graphite")
 * ran four surface steps at 1.073 / 1.126 / 1.089 / 1.077 : 1, every one of them
 * below perceptual threshold, and spent the entire separation budget on 1px
 * runs. PRESSWORK keeps the ground (ZD_BASE #322B27, L* 18.15, unchanged),
 * keeps the single lamp, keeps the lit/cut grammar and keeps "the occluder
 * draws the border" - and then widens the steps until a KNOCKOUT has a rung to
 * live on. The smallest surface step is now 1.1682:1 (ZD_RAISE -> ZD_FLOAT).
 *
 * THE WIDENING IS ASYMMETRIC BECAUSE THE ARITHMETIC IS. ZD_BASE has relative
 * luminance Y = 0.02552. Downward the total room to pure black is only
 * (0.02552 + 0.05) / 0.05 = 1.5105:1, and three rungs share it; upward the room
 * is 1.05 / 0.07552 = 13.9030:1. So PRESSWORK spends downward the little that
 * exists - ZD_CUT lands at 1.4723:1 on ZD_BASE, 97.5% of everything there is -
 * and takes the rest upward.
 *
 * THE FINDING THAT MADE THE KNOCKOUT SAFE. ZD_KNOCK is chosen so the edge run
 * computes the SAME ratio against the knockout as against the ground:
 *
 *     ZD_LIT on ZD_BASE     2.5423:1    lighter term, top edge
 *     ZD_KO_EDGE on ZD_KNOCK 2.5487:1   darker  term, foot edge
 *     skew                   0.25 %
 *
 * One value, two geometries, one loudness. The light does not invert its
 * loudness when the header inverts, because the ladder was widened until it
 * could not.
 *
 * WHAT THE WIDENING COSTS, and these are the honest prices, not caveats:
 *   1  Subtlety is gone. The 1px runs CONFIRM a plate; they no longer
 *      CONSTITUTE it. The smallest step went 1.0728 -> 1.1682:1.
 *   2  The ink ramp lost a rung. The lightest text-bearing surface moved from
 *      L* 23.21 to ZD_FLOAT's L* 29.41, the 4.5:1 floor moved up with it, and
 *      the ramp went from five rungs to FOUR. ZD_TEXT_INERT survives as a token
 *      but is STRUCTURE ONLY and must never carry a glyph.
 *   3  There is no room left below the ground. Ever. 1.4723 of 1.5105.
 *
 * LIGHT MODE IS DELIBERATELY OUT OF SCOPE. The prototype carries a second
 * ladder and its own verifier reports that ladder unfinished: on paper
 * ZD_LIT computes 1.244:1 on ZD_BASE, so the upward headroom the whole
 * widening thesis depends on does not exist there. zlOS ships the DARK ladder
 * only. Nothing in this file has a light counterpart, and a comment that would
 * naturally mention the other mode says instead that it is out of scope.
 *
 * WHY ONE FILE, and this rule has not changed. zlOS has shipped two
 * simultaneous palettes before: kernel.zl's rgb() constants painted the header
 * bar and the dock, ui.c's ui_theme painted every window frame, and they agreed
 * on 2 of 10 roles - two different cyans and two different panel colours on
 * screen at the same time. The rule that prevents the third occurrence is that
 * A COLOUR LITERAL MAY APPEAR HERE AND IN NO OTHER FILE. wm.c, uikit.c,
 * settings.c and kernel.zl reach colour through ui_color() / theme(TH_*).
 *
 * HOW TO RE-DERIVE, and do not trust this comment - run it:
 *
 *   tokens    sed -n '/^:root {/,/^}/p' docs/design/presswork-prototype.html
 *   contrast  the settings pane in that file, "MEASURED" tab, recomputes every
 *             ratio quoted below from the live custom properties
 *
 * Every ratio in this file was computed from these hex values with the WCAG
 * relative-luminance formula, not copied out of prose. sRGB, D65, 0..1 linear.
 *
 * These are 0xRRGGBB, matching ui_theme. The prototype writes them #RRGGBB.
 */
#ifndef ZL_DESIGN_H
#define ZL_DESIGN_H

/* ---- the ends of the axis, and they are NOT design colours -----------------
 * WCAG contrast is defined lighter-over-darker against pure black and pure
 * white, so "how much room does this surface have below it / above it" is the
 * ratio to one endpoint or the other. ui_ceil_dn_q4 / ui_ceil_up_q4 need those
 * two values, and the settings pane's edge table is nothing but the comparison
 * between them.
 *
 * THEY ARE HERE BECAUSE OF THE RULE, NOT BECAUSE THEY ARE TOKENS. "A colour
 * literal may appear in this file and in no other" has no clause for "unless
 * it is only maths", and hosttest/uitest.c enforces it by SCANNING for six hex
 * digits - which is the right way to enforce it, because a scanner cannot be
 * argued with about intent. Written as 0x000000 in ui.c they were two literals
 * in a file the gate says must have none, and the gate caught them.
 *
 * NOTHING PAINTS WITH THEM and nothing may. They are arguments to ui_ratio_q4
 * and no role in struct ui_theme is ever assigned either one: on this ladder
 * pure black is 1.5105:1 below the plate - louder than the groove, which is
 * the whole downward budget - and pure white is 13.9030:1 above it, twice the
 * knockout. Either as a surface would be off the top of the design. */
#define ZD_AXIS_BLACK 0x000000
#define ZD_AXIS_WHITE 0xFFFFFF

/* ---- the surface ladder ---------------------------------------------------
 * EIGHT rungs, darkest first, and this is a DEPTH LADDER, not a set of greys.
 * A surface drawn on top of another is one rung lighter; the boundary between
 * them is ZD_CUT below and ZD_LIT above. Skipping a rung is what makes a panel
 * read as pasted on rather than raised.
 *
 * Measured step ratios, adjacent pairs:
 *   CUT ->WELL   1.0877     WELL ->VOID  1.1335     VOID->BASE   1.1942
 *   BASE->RAISE  1.2468     RAISE->FLOAT 1.1682     FLOAT->LITSOFT 1.3060
 *   LITSOFT->LIT 1.3365
 * The smallest is 1.1682:1 and that number is the whole widening. The three
 * rungs below the ground are still tight, because 1.5105:1 is all there is.
 */
#define ZD_CUT      0x0A0300   /* L*  1.17  the groove, and the strip ground */
#define ZD_WELL     0x17100C   /* L*  5.23  sunken: input wells, output pits */
#define ZD_VOID     0x241D19   /* L* 11.44  the field / the desk             */
#define ZD_BASE     0x322B27   /* L* 18.15  THE PLATE. the window body       */
#define ZD_RAISE    0x413A36   /* L* 25.00  raised on a plate, and the rail  */
#define ZD_FLOAT    0x4B4440   /* L* 29.41  off the plane: menus, toasts     */
#define ZD_LITSOFT  0x5C5551   /* L* 36.68  the grazed LEFT run              */
#define ZD_LIT      0x6F6864   /* L* 44.52  the struck TOP run, 2.5423:1     */

/* ZD_EDGE_OVER - the one invented token, inherited and re-solved on the wider
 * ladder. THE OCCLUDER DRAWS THE BORDER, never the occluded: when two plates
 * overlap, the only thing saying which is on top is the upper plate's bottom
 * and right edge lying on the lower plate's body, and that edge must clear
 * 3:1. A darker edge cannot: the ceiling on a darker term is fixed by the
 * ground alone and is 1.4723:1 on ZD_BASE, so the direction is arithmetically
 * shut and the occluding edge has to be the LIGHT.
 *
 * Measured, worst case first - a plate lying on ZD_FLOAT:
 *   on ZD_FLOAT 3.4322:1   on ZD_RAISE 4.0095:1
 *   on ZD_BASE  4.9991:1   on ZD_VOID  5.9701:1
 * The floor is cleared by 14.4% at worst. On the un-widened ladder the same
 * token cleared it by 4%. */
#define ZD_EDGE_OVER 0xA09A95  /* L* 63.96 */

/* ---- the knockout ---------------------------------------------------------
 * THE FOCUSED WINDOW'S HEADER INVERTS TO A SOLID LIGHT PLATE AND THE TITLE IS
 * REVERSED OUT OF IT. This is the departure the widening was for, and it is
 * the entire focus signal - not a tinted bar, not a glow.
 *
 * ZD_KNOCK is 6.4796:1 on ZD_BASE, which is 46.6% of the 13.9030:1 upward
 * ceiling. The loudest chromatic element in the system is ZD_STEEL at
 * 6.1881:1, so the knockout is 4.7% louder than something already drawn -
 * deliberately, because a knockout on the UN-widened ladder would have come
 * out at 11.9231:1, 92% louder, and that is the failure this replaces.
 *
 * ZD_KO_EDGE is the knockout's own edge run and in this ladder it resolves to
 * EXACTLY ZD_LIT's value. It lies on the BOTTOM of the header, because on the
 * knockout the downward direction is the one with headroom (9.79:1 available
 * down, 2.15:1 up). Same lamp, same rule, other side. (In the light ladder the
 * two runs mirror; that ladder is out of scope - see the file header.)
 *
 * COST, measured on the prototype's 600 x 420 dp reference plate: the knockout
 * changes 16,800 px, the vermilion focus bar adds 1,176, total 17,976 px =
 * 7.133% of the plate. Its predecessor's bezel-plus-wash changed 17,976 px on
 * the same plate - THE IDENTICAL AREA. What moved is the ratio inside it:
 * 1.3999:1 becomes 6.4796:1, a 4.63x increase, for no extra painted pixels.
 */
#define ZD_KNOCK      0xB6B0AB /* L* 72.19   6.4796:1 on ZD_BASE            */
#define ZD_KO_EDGE    0x6F6864 /* == ZD_LIT. 2.5487:1 on ZD_KNOCK          */
#define ZD_KNOCK_INK  0x181411 /* the title, reversed out. 8.5329:1        */
#define ZD_KNOCK_INK2 0x46413D /* secondary on the knockout. 4.6965:1      */

/* ---- the fallback the knockout is argued AGAINST --------------------------
 * THE SETTINGS PANE'S "the knockout" TOGGLE IS A LIVE CONTROL, NOT A
 * PARAGRAPH, and this is the surface it falls back to. The prototype states
 * the requirement exactly: "turn the knockout off and the header returns to
 * graphite's ZD_FOCUS_WASH ramp: same band, same area". Same band and same
 * area is the whole comparison - if the fallback painted fewer pixels the
 * ratio figures beside it would not be comparing anything.
 *
 * WHERE THE VALUE COMES FROM, and it is derived rather than picked. The
 * prototype's fallback header is `background: var(--zd-base)` with one
 * gradient over it whose loudest stop is
 *
 *     color-mix(in srgb, var(--zd-lit) 34%, transparent)
 *
 * i.e. ZD_LIT at alpha 0.34 composited on ZD_BASE. Per channel:
 *     R  0x32 + 0.34*(0x6F-0x32) = 70.74 -> 0x47
 *     G  0x2B + 0.34*(0x68-0x2B) = 63.74 -> 0x40
 *     B  0x27 + 0.34*(0x64-0x27) = 59.74 -> 0x3C
 *
 * IT COMPUTES 1.3681:1 ON ZD_BASE, NOT 1.3999:1, and the difference is
 * declared rather than smoothed over. 1.3999 is graphite's own ZD_FOCUS_WASH
 * token; that token is not in this ladder and was never available to compute
 * from - wm.c makes the same disclosure at chrome_header. What IS in this
 * ladder is the ramp the prototype actually draws, and 1.3681:1 is what that
 * ramp measures at its loudest stop. The settings pane prints the live figure
 * and quotes 1.3999 separately, in the comparison rows, as the foreign number
 * it is.
 *
 * IT IS FLAT WHERE THE PROTOTYPE RAMPS. wm.c's chrome_header paints the band
 * with fb_rrect_grad_top(..., t->knock, t->knock) - one colour passed twice -
 * so swapping theme.knock for this token gives a flat wash across the band
 * rather than a horizontal ramp across it. Flat at the ramp's loudest stop is
 * the conservative direction (it overstates the fallback, which is the side
 * that makes the knockout look WORSE by comparison), and turning it into a
 * real ramp is a wm.c edit, not a token. */
#define ZD_FOCUS_WASH 0x47403C /* ZD_LIT at 34% on ZD_BASE. 1.3681:1       */

/* ---- THE COMPARISON LADDER, and it is a CONTROL rather than a footnote ------
 * The settings pane's central claim - "the knockout is safe on THIS ladder and
 * was not safe on the one before it" - is a comparison, and a comparison needs
 * the other side present as values rather than as prose. These four are the
 * prototype's `--zd-ref-*` block, verbatim, and they are the ONLY colours in
 * this file that are not PRESSWORK's: they are the parent designs' tokens,
 * kept so the pane can compute against them instead of quoting a number.
 *
 * They are in this file for the same reason everything else is - a colour
 * literal lives here and nowhere else - and they are segregated by name so no
 * call site can reach for one by accident. NOTHING PAINTS WITH THEM. They are
 * arguments to ui_ratio() and to nothing else, and ui_theme_init_q8 does not
 * assign any of them to a role.
 *
 * What they buy, all of it computed at render time rather than quoted:
 *   ZD_LIT on graphite's ground      1.9931:1  a LIGHTER term
 *   ZD_LIT on PLATE's knockout       5.9822:1  a DARKER  term - 3.00x, and
 *                                              the opposite sign. That pair
 *                                              IS the sign inversion.
 *   PLATE's knockout on that ground 11.9231:1  the figure this header quotes
 *                                              at line 128, now derived
 */
#define ZD_REF_LIT_RAKING   0x5F5854 /* the parent's struck run             */
#define ZD_REF_BASE_RAKING  0x322B27 /* == ZD_BASE. the ground never moved  */
#define ZD_REF_KNOCK_PLATE  0xF2EDE4 /* PLATE's knockout, imported untouched*/
#define ZD_REF_WASH_RAKING  0x404348 /* the parent's focus wash             */

/* ...and the three that CANNOT be derived, named so that the settings pane
 * never has to invent one. Everything else the pane prints comes out of
 * ui_ratio() against live tokens; these measure things this ladder has no
 * token for at all, so they are quoted, and the pane prints them in the dim
 * ink it reserves for exactly that distinction.
 *
 * The plate's own pixel counts are NOT here on purpose - 16,800 is
 * ZD_REF_PLATE_W x ZD_TITLE_H and 1,176 is the focus bar's width x the plate
 * below the header, so the pane derives both from geometry. Only the raking
 * design's signal is a quoted count, because its focus signal is not a
 * rectangle this ladder knows the shape of.
 *
 * Contrast constants are x10^4, the same fixed point ui_ratio() returns, so a
 * quoted figure and a computed one go through exactly one print helper and
 * cannot drift into two different roundings. */
#define ZD_REF_PLATE_W     600   /* the prototype's reference plate, dp      */
#define ZD_REF_PLATE_H     420
#define ZD_REF_RAKING_PX  1019   /* raking's focus signal on that plate      */
#define ZD_REF_GRAPHITE_PX 17976 /* graphite's bezel + wash, same plate.   */
                                 /* THE IDENTICAL AREA - that equality is  */
                                 /* the comparison, and it is why the pane */
                                 /* prints this next to a derived total.   */
#define ZD_REF_WASH_Q4   13999   /* graphite's own ZD_FOCUS_WASH on ZD_BASE. */
                                 /* the token is not in this ladder - see    */
                                 /* ZD_FOCUS_WASH above for what IS.         */
#define ZD_REF_RAKING_Q4 61900   /* raking's loudest element on a plate      */

/* the ruled module grid, printed onto the desk. 12 x 8 modules, margin 16dp,
 * gutter 12dp, and it is visible only where no plate covers the field. Its job
 * is to EXPLAIN where a window sits, not to decide it - zlOS's windows float.
 * 1.6470:1 on ZD_VOID: loud enough to be read as ruling, quiet enough to stay
 * furniture rather than decoration. */
#define ZD_GRID     0x474139

/* ---- the ink ramp ---------------------------------------------------------
 * FOUR rungs, not five, and the fifth was eaten by the widening rather than
 * dropped by choice: at ZD_FLOAT (L* 29.41) the 4.5:1 floor sits at L* 72.4,
 * and the old fifth rung no longer cleared it. ZD_TEXT_INERT survives as a
 * token and is STRUCTURE ONLY - rules, gutters, disabled marks, tick strokes.
 * IT MUST NEVER BE A GLYPH. It is 2.5213:1 on ZD_BASE and 1.7310:1 on ZD_FLOAT.
 *
 * Measured, on ZD_BASE and on ZD_FLOAT (the worst text-bearing surface):
 *   TEXT_0  11.2080 / 7.6950     TEXT_1  9.4413 / 6.4820
 *   TEXT_2   7.8606 / 5.3968     TEXT_3  6.6809 / 4.5869
 * TEXT_3 on ZD_FLOAT is the tightest thing in the system that is still text,
 * and it clears 4.5:1 by 1.9%. Nothing dimmer than TEXT_3 may sit on FLOAT.
 */
#define ZD_TEXT_0    0xECE6DD  /* L* 91.53  emphasis: titles, live values   */
#define ZD_TEXT_1    0xDAD4CB  /* L* 85.15  BODY - the desktop's root ink   */
#define ZD_TEXT_2    0xC8C2B9  /* L* 78.67  secondary                       */
#define ZD_TEXT_3    0xB9B3AA  /* L* 73.19  labels, column heads, captions  */
#define ZD_TEXT_INERT 0x6E685F /* L* 44.29  STRUCTURE ONLY. never a glyph.  */

/* ---- the two inks ---------------------------------------------------------
 * PRESSWORK carries TWO chromatic inks, which is one more than either parent
 * allowed, and the licence comes with a contract:
 *
 *   ZD_STEEL  is THE MACHINE'S OWN READING. It appears inside an instrument
 *             and nowhere else: raster columns, meter fills, spark bars, plot
 *             lines, ruler regions. Never a control, never focus, never a
 *             border. Saturation is LOCKED - do not re-tune it per widget.
 *   ZD_VERM   is THE OVERPRINT. An ink laid ON the surface, never the surface
 *             itself. EXACTLY FOUR JOBS: the focus bar and register mark; the
 *             ONE primary action per view; the crop marks; the datum mark on
 *             the memory ruler. Never status, never body text, never an icon.
 *
 * The risk is that two inks on warm graphite starts to look decorative. The
 * mitigation is the WIDTH RULE: vermilion never fills an area wider than the
 * focus bar, except for one button per view.
 *
 * Measured on ZD_BASE:  STEEL 6.1881   STEEL_BR 8.5366
 *                       VERM  4.6319   VERM_BR  6.8281
 */
#define ZD_STEEL     0x7FB2E0  /* instruments only                          */
#define ZD_STEEL_BR  0xA9CFF2  /* the instrument's own highlight            */
#define ZD_VERM      0xE8734F  /* THE OVERPRINT. 4.6319:1 on ZD_BASE        */
#define ZD_VERM_BR   0xF5A184  /* the overprint at full strength            */

/* ---- semantics ------------------------------------------------------------
 * Wired to STATE, never to the accent setting, and never to either ink. A
 * palette with no name for "warning" spends the overprint on it, and then the
 * overprint stops meaning "this is the one thing to act on".
 * Measured on ZD_BASE: OK 6.3125   WARN 6.4830   BAD 4.2591   BAD_INK 5.9093.
 * ZD_BAD is 4.2591:1, under the 4.5 floor, so it is a FILL and a mark - the
 * red that carries text is ZD_BAD_INK. */
#define ZD_OK        0x7FBF63  /* healthy, running, pass                    */
#define ZD_WARN      0xE3A63F  /* warning - read-only mounts, drift         */
#define ZD_BAD       0xE8664B  /* failure fills and marks, close-box hover  */
#define ZD_BAD_INK   0xEF9077  /* failure TEXT on a dark row. 5.9093:1      */

/* the ink that goes ON vermilion. 6.1400:1 on ZD_VERM, computed, not chosen.
 * ui_ink_on() picks between the two; on ZD_VERM (Y 0.29982) it resolves dark. */
#define ZD_INK_ON    0x181310

/* ZD_LIFT - THE ONLY SHADOW IN THE SYSTEM, and it never appears at rest. It is
 * drawn on a dragged plate and under the menu, the palette sheet and the toast.
 * Split into colour and alpha because fb.c's blend takes them apart: the
 * prototype's token is #0000008C, i.e. black at 140/255 = 55%. A dark ground
 * costs a shadow roughly 54% of its recession, which is why the value is this
 * strong rather than the 35% a paper ground would want. */
#define ZD_LIFT      0x000000
#define ZD_LIFT_A    140      /* 0..255, == 55%                             */
#define ZD_LIFT_DY     5      /* design px, the dragged-plate offset        */
#define ZD_LIFT_BLUR  13      /* design px                                  */

/* ---- the window, exactly --------------------------------------------------
 * From the prototype's .win / .hdr / .trun / .lrun / .ctl rules. These are
 * quoted rather than summarised because the window is the most repeated object
 * on the screen and every number here is visible twenty times over.
 *
 *   plate        ZD_BASE, radius ZD_R_PLATE, 1px ZD_CUT ring on all four sides
 *   under overlap the ring becomes ZD_EDGE_OVER (the occluder draws it)
 *   top run      1px ZD_LIT, inset by the corner radius at both ends
 *   left run     1px ZD_LITSOFT, inset by the corner radius top and bottom
 *   header       ZD_TITLE_H tall, ZD_BASE, 1px ZD_CUT groove along its foot
 *   FOCUSED      header becomes ZD_KNOCK; its groove becomes ZD_KO_EDGE; the
 *                top run becomes ZD_KNOCK (it MERGES - same value as the
 *                surface it lies on, which is what "the light cannot lighten
 *                this further" looks like drawn honestly); the left run starts
 *                below the header instead of at the corner radius.
 *   title        ZD_TEXT_2, uppercase, tracked ZD_TR_LAB; on the knockout it
 *                is ZD_KNOCK_INK and everything secondary is ZD_KNOCK_INK2.
 *   controls     NO FACE AT REST. Three bare glyphs, each behind a 1px ZD_CUT
 *                rule on its left. Hover fills ZD_RAISE; close hover fills
 *                ZD_VERM with ZD_INK_ON.
 */
#define ZD_TITLE_H         28      /* design px */
#define ZD_TITLE_INK       ZD_TEXT_2    /* at rest                          */
#define ZD_TITLE_INK_KO    ZD_KNOCK_INK /* reversed out of the knockout     */
#define ZD_TITLE_INK_OFF   ZD_TEXT_3    /* unfocused                        */
#define ZD_WINCTL          22      /* the control's width; it is full-height */
/* THE HEADER'S TWO MARGINS, from .hdr's own padding in the prototype:
 *   .hdr { padding: 0 calc(6px * var(--ui)) 0 calc(11px * var(--ui)) }
 * The left one was already being drawn (as an uncited UI_S3, the same number by
 * coincidence); the right one was never spelled at all, so the control cluster
 * sat flush against the inside face of the ring while the title kept its
 * margin - an asymmetry the header is not supposed to have. */
/* .wbody's OWN box, from `padding: 6px 9px 6px; gap: 5px` (proto:711-712).
 * The foot band and the client area both inherit their horizontal extent from
 * it, and both were reaching for ZD_PAD - the generic 10dp spacing step, which
 * belongs to no .wbody or .sband rule. One dp per side, everywhere the body
 * meets the plate. */
#define ZD_BODY_PX          9      /* .wbody padding-left / padding-right */
#define ZD_BODY_PY          6      /* .wbody padding-top / padding-bottom */
#define ZD_BODY_GAP         5      /* .wbody row gap                      */
#define ZD_HDR_PL          11      /* .hdr padding-left  */
#define ZD_HDR_PR           6      /* .hdr padding-right */
#define ZD_WINCTL_INK      ZD_TEXT_3
#define ZD_WINCTL_RULE     ZD_CUT  /* the 1px rule to its left               */
#define ZD_CLOSE_HOVER_BG  ZD_VERM
#define ZD_CLOSE_HOVER_INK ZD_INK_ON
/* Focus is the KNOCKOUT plus the vermilion focus bar. There is no ring and no
 * glow - both were tried on the parent ladder and both are what the knockout
 * replaces. These two names survive because call sites still reach for them;
 * ZD_RING_FOCUS is the overprint and its alpha is 0, i.e. off. */
#define ZD_RING_FOCUS      ZD_VERM
#define ZD_RING_FOCUS_A    0       /* percent - the ring is OFF in PRESSWORK */
#define ZD_RING_BLUR_A     0       /* no glow                                */
#define ZD_RING_OFF_A      0

/* ---- the radius scale -----------------------------------------------------
 * FOUR rungs. Radius encodes how much the object can move, and nesting halves
 * it. The predecessor measured fifteen distinct radii out of an artifact and
 * shipped all fifteen; fifteen radii is not a scale, it is a file. PRESSWORK
 * collapses them onto four, and the collapse is monotone by size band so no
 * call site changes meaning:
 *      0..3  -> ZD_R_CHIP     4..7  -> ZD_R_INSET     8+ -> ZD_R_PLATE
 * ZD_R_BOLT is 0 and means "this thing is bolted down" - a maximised window,
 * the rail, the strip, the foot.
 */
#define ZD_R_BOLT    0
#define ZD_R_CHIP    2
#define ZD_R_INSET   4
#define ZD_R_PLATE   9

/* the vermilion focus bar. 3dp is the shipped value and it is a slider. */
#define ZD_FOCUS_BAR 3

/* THE OLD FIFTEEN, kept as names so no call site breaks, collapsed onto the
 * four above. Prefer the four in new code; these exist to be migrated off. */
#define ZD_R_XS     ZD_R_INSET
#define ZD_R_SM     ZD_R_INSET
#define ZD_R_6      ZD_R_INSET
#define ZD_R_7      ZD_R_INSET
#define ZD_R_8      ZD_R_PLATE
#define ZD_R_9      ZD_R_PLATE
#define ZD_R_10     ZD_R_PLATE
#define ZD_R_11     ZD_R_PLATE
#define ZD_R_12     ZD_R_PLATE
#define ZD_R_13     ZD_R_PLATE
#define ZD_R_14     ZD_R_PLATE
#define ZD_R_15     ZD_R_PLATE
#define ZD_R_16     ZD_R_PLATE
#define ZD_R_17     ZD_R_PLATE
#define ZD_R_20     ZD_R_PLATE

/* ---- the shell's fixed bands, in design px --------------------------------
 * The apparatus around the field. Each is a fixed band because a printed sheet
 * has fixed furniture: it does not reflow, it is ruled once.
 *   rail    the left column of tools
 *   strip   the raster readout across the top, ground ZD_CUT, 2px ZD_LIT rule
 *   foot    the status/ruler band along the bottom, 2px ZD_LIT rule on top
 *   band    a section band inside a plate
 *   row     one line of controls
 */
#define ZD_RAIL_W    170
#define ZD_STRIP_H    30
#define ZD_FOOT_H     46
#define ZD_BAND_H     20
#define ZD_ROW_H      26
#define ZD_PAD        10
#define ZD_GAP         8
#define ZD_RULE_H      2      /* the 2px ZD_LIT rule under strip / over foot */

/* ---- the desk -------------------------------------------------------------
 * THE WALLPAPER IS ONE CALL. Not nine layers, not a conic wedge, not a
 * vignette - a single raking light entering from off-screen upper left and
 * falling to ZD_VOID at the lower right:
 *
 *   fb_glow(-w/8, -h/6, w*11/8, h*11/8, ZD_LITSOFT, 26, 0, 100)
 *
 * ZD_WALL_0 / ZD_WALL_42 / ZD_WALL_100 are that glow FLATTENED to three
 * gradient stops, for the kernel.zl path that draws it as a gradient rather
 * than a radial. They are computed, not measured: the prototype's stops are
 * ZD_LITSOFT at 46% and 16% over ZD_VOID, mixed in sRGB.
 *   46%: 0.46*5C5551 + 0.54*241D19 = 3E3733
 *   16%: 0.16*5C5551 + 0.84*241D19 = 2D2622
 */
#define ZD_WALL_0    0x3E3733  /* upper left, where the light lands          */
#define ZD_WALL_42   0x2D2622  /* the 34% stop                               */
#define ZD_WALL_100  ZD_VOID   /* lower right - the desk itself              */
#define ZD_WALL_GLOW   ZD_LITSOFT
#define ZD_WALL_GLOW_A 26      /* fb_glow a_in, percent                      */
#define ZD_WALL_GLOW_OUT 0     /* fb_glow a_out                              */

/* the ruled module grid on the desk. 12 x 8 modules; every module is ruled on
 * all four sides, so a rule is 1px ZD_GRID. Integer division remainder is
 * spread one pixel at a time across the first (inW - gut*(cols-1)) % cols
 * columns - never dumped on the last one, which is what makes the right margin
 * stay a margin. */
#define ZD_GRID_COLS  12
#define ZD_GRID_ROWS   8
#define ZD_GRID_MARGIN 16      /* design px, all four sides                  */
#define ZD_GRID_GUTTER 12      /* design px, between modules                 */

/* CROP MARKS. Overprint job 3: vermilion, at the four margin corners, an 18dp
 * arm offset 7dp outside the margin. They are the only vermilion on the desk. */
#define ZD_CROP_LEN    18
#define ZD_CROP_OFF     7

/* ---- motion ---------------------------------------------------------------
 * THREE durations and ONE curve, and the curve is unchanged from the parent
 * because it was never the problem: cubic-bezier(0.200, 0.850, 0.300, 1.000).
 *
 *   RISE    90ms   the load-bearing half of the focus signal. A CUT (0ms) was
 *                  offered and REFUSED: the knockout is a full value flip and
 *                  cutting it makes the whole screen twitch.
 *   TRAVEL 160ms   something moving across the field
 *   SETTLE 240ms   something arriving and coming to rest
 *
 * Stored in MILLISECONDS. The bezier is stored as its four control values
 * times 1000, because the kernel has no floating point in the drawing path.
 */
#define ZD_MS_RISE     90
#define ZD_MS_TRAVEL  160
#define ZD_MS_SETTLE  240

/* the named animation slots, mapped onto the three durations above. They keep
 * their names so wm.c's timeline keeps compiling; there are three values here,
 * not ten, and that is the point. */
#define ZD_MS_WIN       ZD_MS_TRAVEL   /* window open / close                */
#define ZD_MS_POP       ZD_MS_RISE     /* menus, chips, tiles                */
#define ZD_MS_POP_FAST  ZD_MS_RISE
#define ZD_MS_POP_SLOW  ZD_MS_RISE
#define ZD_MS_PRESS     ZD_MS_RISE     /* a button taking a press            */
#define ZD_MS_OV        ZD_MS_TRAVEL   /* overlays, modals                   */
#define ZD_MS_TOAST     ZD_MS_TRAVEL
#define ZD_MS_PULSE     ZD_MS_SETTLE   /* "activating" - one settle, not a   */
#define ZD_MS_PULSE_SLOW ZD_MS_SETTLE  /* forever loop. see below.           */

/* THE INFINITE PULSE IS DELETED. A machine that breathes at rest is decoration
 * and it costs a repaint every frame forever. ZD_MS_SWEEP, the 7s wallpaper
 * sweep, goes with it: the raking light does not move, because a lamp in a
 * room does not move. Both names survive at their old values ONLY so a call
 * site that still references them compiles; the alpha floor is 255, i.e. the
 * pulse does not dip, i.e. it does nothing. */
#define ZD_MS_SWEEP   7000
#define ZD_PULSE_FLOOR 255

/* the curve, x1 y1 x2 y2 scaled by 1000. Unchanged. */
#define ZD_EASE_WIN_X1  200
#define ZD_EASE_WIN_Y1  850
#define ZD_EASE_WIN_X2  300
#define ZD_EASE_WIN_Y2 1000

/* The transforms each animation starts from. Translations are design px,
 * scales are per-mille (965 == scale(.965)), opacity is percent. A plate is a
 * physical object on a desk, so it does not balloon: nothing scales from below
 * 0.96 and nothing overshoots past 1.0. */
#define ZD_WIN_FROM_SCALE   965
#define ZD_WIN_FROM_DY       10
#define ZD_POP_FROM_SCALE   980
#define ZD_POP_FROM_DY       -6
#define ZD_PRESS_MIN_SCALE  970   /* a press dips 3%, not 12%               */
#define ZD_PRESS_MID_PCT     45
#define ZD_OV_FROM_SCALE   1000   /* no overshoot. it was 1030.             */
#define ZD_TOAST_FROM_DY     10

/* ---- type -----------------------------------------------------------------
 * THREE sizes, because there are THREE baked bitmap atlases (8x16, 16x32,
 * 24x48) and NO runtime glyph rasteriser. This is a hard platform fact, not a
 * design preference: a fourth size does not exist to be asked for.
 *
 *   SM 11 / MD 13 / LG 21, with line heights 15 / 18 / 24.
 *
 * What PRESSWORK adds is a LABEL STYLE rather than a fourth size: SM,
 * uppercase, bold, tracked by ZD_TR_LAB. The tracking is the one item in this
 * file that costs work elsewhere - fb_text_role cannot track without a track
 * flag on its existing flag word, and MEASURE AND DRAW MUST SHARE THE HELPER
 * or a tracked label clips at its right edge.
 *
 * Sizes are stored x2 to stay in integers, as they always were here.
 */
#define ZD_T_SM     22   /* 11.0px  labels, mono readouts, captions          */
#define ZD_T_MD     26   /* 13.0px  BODY - titles, rows, running text        */
#define ZD_T_LG     42   /* 21.0px  the one large reading per view           */
#define ZD_LH_SM    15   /* line heights are whole design px                 */
#define ZD_LH_MD    18
#define ZD_LH_LG    24
#define ZD_TR_LAB   14   /* tracking x10: 1.4 design px, the label style     */
#define ZD_TR_BIG   26   /* tracking x10: 2.6 design px, the large reading   */

/* ---- widget geometry, in DESIGN pixels ------------------------------------
 * Every number below is a design-space pixel and every one must go through
 * UI_DP() before it reaches fb.c. They live here rather than in uikit.c for
 * the same reason the colours do: the moment a second widget spells `9` for a
 * radius, the two copies drift.
 *
 * These are inherited geometry, re-pointed at PRESSWORK's radius scale. Sizes
 * that PRESSWORK fixes itself - the header, the row, the pad, the gap, the
 * rail, the strip, the foot - are above, and where a widget number below
 * disagrees with one of those, the one above wins.
 */

/* the button. PRESSWORK has no pills: a control is a rectangle with a 1px
 * ZD_CUT ring and an inset 1px ZD_LIT top run, radius ZD_R_CHIP. The three
 * size names survive; the radii all resolve to the chip. */
#define ZD_PILL_SM_PY    3
#define ZD_PILL_SM_PX    9
#define ZD_PILL_SM_R     ZD_R_CHIP
#define ZD_PILL_MD_PY    6
#define ZD_PILL_MD_PX   13
#define ZD_PILL_MD_R     ZD_R_CHIP
#define ZD_PILL_LG_PY    7
#define ZD_PILL_LG_PX   15
#define ZD_PILL_LG_R     ZD_R_CHIP

/* the segmented control - one 1px ZD_CUT ring around the set, 1px ZD_CUT
 * between items, no gap and no inner radius. A segmented control is a switch
 * plate, and a switch plate has no gaps in it. */
#define ZD_SEG_PAD       0
#define ZD_SEG_GAP       0
#define ZD_SEG_R         ZD_R_CHIP
#define ZD_SEG_ITEM_R    ZD_R_BOLT
#define ZD_SEG_ITEM_PY   4
#define ZD_SEG_ITEM_PX  13

/* tab strips */
#define ZD_TAB_H        26
#define ZD_TAB_PAD_T     0
#define ZD_TAB_PAD_X     0
#define ZD_TAB_GAP       0
#define ZD_TAB_PL       11      /* asymmetric: 11 left, 9 right */
#define ZD_TAB_PR        9
#define ZD_TAB_MAXW    170
#define ZD_TAB_R        ZD_R_BOLT
#define ZD_TAB_X         9      /* the close glyph box */
#define ZD_UTAB_H       26
#define ZD_UTAB_PX      11
#define ZD_UTAB_RULE     2      /* the 2px ZD_LIT rule under the active tab */

/* bars */
#define ZD_TOOLBAR_H    30      /* == ZD_STRIP_H */
#define ZD_TOOLBAR_PX   10      /* == ZD_PAD     */
#define ZD_TOOLBAR_GAP   8      /* == ZD_GAP     */
#define ZD_STATUS_H     20      /* == ZD_BAND_H  */
#define ZD_STATUS_PX    10
#define ZD_STATUS_GAP   12

/* column header + list row.
 *
 * A DATA ROW IS NOT A ROW OF FURNITURE, AND THE PROTOTYPE SPENDS TWO
 * DIFFERENT NUMBERS ON THEM. This said 26, annotated "== ZD_ROW_H", and the
 * equality was the mistake: ZD_ROW_H is `--zd-row-h`, which the prototype
 * applies to `.slot` (the rail's register slot) and `.wsb` (the workspace
 * button) - objects you point at with a mouse. A table cell is `td`, which
 * carries no height at all and is sized by its own contents:
 *
 *     td { font-size: var(--fs-sm); line-height: var(--lh-sm);
 *          padding: calc(2px * var(--ui)) calc(6px * var(--ui));
 *          border-bottom: 1px solid var(--zd-cut); }
 *
 * 15 of leading + 2 + 2 of padding + 1 of groove = 20, and the prototype
 * rendered at 1920x1200 measures exactly that: consecutive ZD_CUT grooves in
 * the Files table at y = 173, 193, 213, 233, 253, 273, 293. Twenty, seven
 * times, no drift.
 *
 * So the data row is 20 and it is 23% shorter than it was. A 420px Files
 * window shows 21 rows where it showed 16. That is the whole of what "the
 * prototype's tables carry more rows" costs: one number, measured off the
 * render rather than inherited from the furniture.
 *
 * ZD_ROW_H itself is NOT touched - the rail slot and the workspace button are
 * still 26, which is what the prototype says they are. Two metrics, because
 * there are two things. */
#define ZD_COLHEAD_H    20
#define ZD_COLHEAD_PR   15      /* the scrollbar gutter */
#define ZD_COLHEAD_PL    4
#define ZD_LISTROW_H    20      /* `td`: 15 leading + 2+2 padding + 1 groove */
#define ZD_CELL_PX       6      /* `td { padding: 0 6px }` - both sides      */
#define ZD_LISTROW_R    ZD_R_BOLT
#define ZD_SEL_TINT_A   15      /* percent - the selected row's overprint    */
#define ZD_SEL_BAR_W     3      /* == ZD_FOCUS_BAR. vermilion, on the left.  */
#define ZD_ZEBRA_A       1      /* percent                                   */

/* stat card strip - the 1px gap over ZD_CUT IS the hairline between cards */
#define ZD_STAT_MIN     88
#define ZD_STAT_PY       7
#define ZD_STAT_PX       9
#define ZD_STAT_GAP      1

/* sidebar / rail */
#define ZD_SIDEBAR_W   ZD_RAIL_W
#define ZD_SIDEBAR_PY    7
#define ZD_SIDEBAR_PX    6
#define ZD_NAV_H        26      /* == ZD_ROW_H */
#define ZD_NAV_PX        9
#define ZD_NAV_R        ZD_R_BOLT
#define ZD_HEADING_PT    4
#define ZD_HEADING_PB    6

/* meters and bars. These are INSTRUMENTS, so their fill is ZD_STEEL and never
 * the overprint. Square ends: an instrument reads a value, and a rounded end
 * lies about where the value stops.
 *
 * A BAR IS A PIT WITH WALLS, and the height is what makes that legible. These
 * were 5 and 6, carried over from the predecessor, where a bar was a rounded
 * capsule with no ring at all. PRESSWORK's `.mtrack` is 9dp of ZD_WELL inside
 * a 1px ZD_CUT ring, so 2 of the 5 were spent on the ring and the reading was
 * 3dp of fill in a 5dp box - the wall ate 40% of the instrument. 9 leaves 7,
 * which is the prototype's own number and the reason it is that number. */
#define ZD_METER_H       9      /* .mtrack */
#define ZD_METER_R      ZD_R_BOLT
#define ZD_PROG_H        9
#define ZD_PROG_R       ZD_R_BOLT
#define ZD_SEGBAR_H     12
#define ZD_SEGBAR_R     ZD_R_BOLT
#define ZD_SEGBAR_SEG_R ZD_R_BOLT
#define ZD_MINIBAR_H     6      /* the inline reading inside a row; no ring */
#define ZD_MINIBAR_R    ZD_R_BOLT

/* TOGGLE SWITCH - `.sw2`. 34 x 17 with a 13 knob at 1px inset, so the travel
 * is 34 - 13 - 2 == 19 and it is SYMMETRIC. The predecessor's 40 x 20 / knob
 * 14 / inset 3 was a capsule with a white puck in it and an asymmetric 3px
 * travel; PRESSWORK's is a bolted rectangle - ZD_R_BOLT, not ZD_R_CHIP -
 * because a switch does not move on the plate, it only changes state. Off is
 * a ZD_WELL pit with a ZD_TEXT_INERT knob (structure, not a glyph); on is the
 * KNOCKOUT with the knob reversed out in ZD_KNOCK_INK. The same value flip the
 * focused header makes, on a 34dp control. */
#define ZD_SW_W         34
#define ZD_SW_H         17
#define ZD_SW_R         ZD_R_BOLT
#define ZD_SW_KNOB      13
#define ZD_SW_INSET      1

/* SLIDER. The prototype has no thumb at all - it draws a slider as an
 * `.mtrack` two design px taller than a meter and lets the fill edge be the
 * readout. That is not a simplification: a thumb is a grabbable object and
 * therefore has to be raised, and a raised object on a 9dp pit needs a run,
 * a ring and a radius the pit does not have room for. The fill edge is a
 * hard 1px boundary between ZD_STEEL and ZD_WELL - 8.3765:1 - which is a more
 * precise readout than a 13dp puck centred on the same coordinate. */
#define ZD_SLIDER_H     11
#define ZD_SLIDER_R     ZD_R_BOLT

/* cards and key/value */
#define ZD_CARD_R       ZD_R_INSET
#define ZD_CARD_PY      10      /* == ZD_PAD */
#define ZD_CARD_PX      10
#define ZD_CARD_HEAD_H  20      /* == ZD_BAND_H */
#define ZD_KV_H         20
#define ZD_KV_GAP       16

/* overlays. The menu, the modal and the toast are the three things genuinely
 * off the plane, so these are the only widgets that carry ZD_LIFT. */
#define ZD_MENU_W      220
#define ZD_MENU_PAD      4
#define ZD_MENU_R       ZD_R_INSET
#define ZD_MENU_ITEM_H  26      /* == ZD_ROW_H */
#define ZD_MENU_ITEM_PY  7
#define ZD_MENU_ITEM_PX 10
#define ZD_MENU_ITEM_R  ZD_R_BOLT
#define ZD_MENU_GAP     14
#define ZD_MODAL_W     334
#define ZD_MODAL_R      ZD_R_PLATE
#define ZD_MODAL_HEAD_H 28      /* == ZD_TITLE_H - a modal has a header too */
#define ZD_MODAL_FOOT_H 40
#define ZD_TOAST_W     340      /* .toast { width: calc(340px * var(--ui)) } */
#define ZD_TOAST_R      ZD_R_INSET
#define ZD_TOAST_PY     10
#define ZD_TOAST_PX     10
#define ZD_TOAST_GAP     8      /* #toasts { gap: var(--zd-gap) } */
#define ZD_TOAST_ICON   20
#define ZD_TOAST_ICON_R ZD_R_CHIP
#define ZD_TOAST_MS   8000      /* auto-dismiss - toast() setTimeout 8000 */
#define ZD_TOAST_MAX     3

/* chart. An instrument: ZD_STEEL, square ends, ruled grid in ZD_TEXT_INERT. */
#define ZD_SPARK_H      66
#define ZD_SPARK_GRID    4      /* quarters: 3 rules at h/4, h/2, 3h/4 */
#define ZD_SPARK_AREA_A 16      /* percent - the fill under the line   */
#define ZD_SPARK_MAX    64      /* samples the toolkit will buffer     */

/* indicators and inputs */
#define ZD_DOT           6
#define ZD_DOT_R        ZD_R_CHIP
#define ZD_BADGE_PY      3
#define ZD_BADGE_PX      8
#define ZD_BADGE_R      ZD_R_CHIP
#define ZD_BADGE_TINT_A 14      /* percent */
#define ZD_INPUT_H      26      /* == ZD_ROW_H */
#define ZD_INPUT_PX      9
#define ZD_INPUT_R      ZD_R_CHIP
#define ZD_SEARCH_H     20
#define ZD_SEARCH_R     ZD_R_CHIP
#define ZD_CHIP_PY       4
#define ZD_CHIP_PX       9
#define ZD_CHIP_R       ZD_R_CHIP

/* Neutral fills, as alpha percent over ZD_TEXT_0. A raised face on a plate is
 * ZD_RAISE, not a translucent wash - these are for the few places that must
 * tint something whose ground is unknown at the call site. */
#define ZD_FILL_A        7
#define ZD_FILL_SOFT_A   6
#define ZD_FILL_ICON_A   8

/* ---- names kept so call sites keep compiling ------------------------------
 * PRESSWORK's own names are above and are what new code should use. Everything
 * below is a name inherited from the previous design, re-pointed at the rung
 * of this ladder that does its job. They are aliases, not values: there is
 * still exactly one place a colour literal is written.
 *
 * WHERE TWO OLD NAMES COLLAPSE ONTO ONE RUNG, THAT IS THE WIDENING'S BILL AND
 * IT IS PAID HERE RATHER THAN HIDDEN. ZD_TEXT_4/5/6 were three ink rungs; the
 * ladder has room for one more below ZD_TEXT_2, so all three become ZD_TEXT_3.
 * Dimmer than ZD_TEXT_3 is not text on this ladder - see the ink ramp above.
 */
#define ZD_SURF_0      ZD_VOID     /* the canvas behind everything          */
#define ZD_SURF_1      ZD_WELL     /* sunken: input wells, segmented tracks */
#define ZD_SURF_2      ZD_CUT      /* the hairline. it is the groove now.   */
#define ZD_SURF_3      ZD_BASE     /* window interior - the plate           */
#define ZD_SURF_4      ZD_RAISE    /* raised: toolbars, sidebars, cards     */
#define ZD_SURF_5      ZD_FLOAT    /* menus, popovers, modal bodies         */
#define ZD_SURF_6      ZD_LITSOFT  /* chip faces, scrollbar thumb           */
#define ZD_SURF_7      ZD_LIT      /* scrollbar hover, the dimmest mark     */
#define ZD_SURF_TABS   ZD_BASE     /* the header ground AT REST             */
#define ZD_SURF_WELL   ZD_WELL
#define ZD_SURF_CARD   ZD_FLOAT    /* a card/toast body, off the plane      */
#define ZD_SURF_HEAD   ZD_RAISE
#define ZD_SURF_GAME   ZD_CUT      /* a canvas is a pit, like the strip     */
#define ZD_SURF_BODY   ZD_VOID
/* The unfocused header is THE SAME COLOUR as the focused one at rest, and
 * that is the design rather than an oversight: focus is the knockout. */
#define ZD_SURF_BAR_OFF ZD_BASE

#define ZD_TEXT_4      ZD_TEXT_3
#define ZD_TEXT_5      ZD_TEXT_3
#define ZD_TEXT_6      ZD_TEXT_3

/* ZD_ACCENT was one lime doing focus, status, links and instruments at once.
 * PRESSWORK splits that job in two - see "the two inks" - and the ACCENT name
 * resolves to the OVERPRINT, because the accent's job was always "the one
 * thing to act on". Anything reading a value wants ZD_STEEL instead. */
#define ZD_ACCENT      ZD_VERM
#define ZD_ACCENT_BR   ZD_VERM_BR
#define ZD_ACCENT_LINK ZD_VERM_BR
#define ZD_ACCENT_PALE ZD_TEXT_0   /* a selected row's ink, not a tint      */

#define ZD_BAD_SOFT    ZD_BAD_INK
#define ZD_INK_DARK    ZD_INK_ON   /* ui_ink_on()'s dark answer             */
#define ZD_INK_LIGHT   ZD_TEXT_0   /* its light answer. #FFF is not in this */
                                   /* palette; nothing here is pure white.  */

/* The four alternate accents were a Settings control on the previous design.
 * PRESSWORK's overprint is not re-pointable - the width rule and the four-jobs
 * contract are what keep two inks from reading as decoration, and a user-chosen
 * fifth hue breaks both. The names survive pointing at the values PRESSWORK
 * does have, so the Settings pane keeps compiling while it is rebuilt. */
/* THE FOUR ALTERNATES MUST BE FOUR COLOURS, AND NONE OF THEM MAY BE THE
 * DEFAULT. ALT_3 was ZD_VERM, which IS ZD_ACCENT, so the Settings picker drew
 * five chips carrying four colours and two of them were byte-identical. Found
 * by review, and the palette gate did not catch it because it asserted that
 * settings.c mentions the token ZD_ACCENT_ALT_4 rather than that the five
 * values differ - see palette.c, which now checks the values.
 *
 * This ladder has exactly four saturated inks besides the default: the two
 * steels and the pale vermilion. A fifth would have to come from ZD_OK or
 * ZD_WARN, and those are wired to STATE rather than to taste - an accent
 * setting that changes what "warning" looks like is a bug, not a preference.
 * So the fourth alternate is the accent's own bright, which is a different
 * colour from the accent and legitimately pickable. */
#define ZD_ACCENT_ALT_1 ZD_STEEL      /* #7FB2E0 */
#define ZD_ACCENT_ALT_2 ZD_STEEL_BR   /* #A9CFF2 */
#define ZD_ACCENT_ALT_3 ZD_VERM_BR    /* #F5A184 - was ZD_VERM == the default */
#define ZD_ACCENT_ALT_4 ZD_BAD_INK    /* #EF9077 */

/* Blur is not drawn. There is no GPU here, PRESSWORK asks for no backdrop
 * blur anywhere, and the three off-plane objects are separated by ZD_LIFT and
 * the ladder instead. These four survive as names only, all at 0, so a call
 * site that still asks for blur gets none rather than failing to compile. */
#define ZD_BLUR_PANEL   0
#define ZD_BLUR_ISLAND  0
#define ZD_BLUR_DOCK    0
#define ZD_BLUR_GLOW_A  0
#define ZD_BLUR_GLOW_B  0

/* The one shadow, in the shape wm.c already asks for. */
#define ZD_SHADOW_DY    ZD_LIFT_DY
#define ZD_SHADOW_BLUR  ZD_LIFT_BLUR
#define ZD_SHADOW_ALPHA 55      /* percent - ZD_LIFT_A as a percentage */

#endif /* ZL_DESIGN_H */
