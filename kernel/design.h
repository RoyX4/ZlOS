/* design.h - THE design tokens, taken from the reference and from nowhere else.
 *
 * The reference is `docs/design/ds-reference.html`, vendored into this repo at
 * sha256 1302282882...e62a4b3a so a gate can read it without depending on a
 * path in ~/Downloads. Every constant below was MEASURED out of that file, and
 * each carries the evidence that put it here - a frequency count, or the line
 * that defines it. Nothing in this file was picked by eye.
 *
 * WHY ONE FILE. zlOS has shipped two simultaneous palettes before: kernel.zl's
 * rgb() constants painted the header bar and the dock, ui.c's ui_theme painted
 * every window frame, and they agreed on 2 of 10 roles - two different cyans
 * and two different panel colours on screen at the same time. hosttest/palette.c
 * exists because of that. The rule that prevents the third occurrence is that
 * a colour literal may appear HERE and in no other file.
 *
 * HOW THE NUMBERS WERE OBTAINED - re-run these, do not trust this comment:
 *
 *   colours    grep -oE '#[0-9a-fA-F]{6}' docs/design/ds-reference.html \
 *                | tr A-F a-f | sort | uniq -c | sort -rn
 *   radii      grep -oE 'border-?[Rr]adius:[^;",]*' docs/design/ds-reference.html \
 *                | grep -oE '[0-9]+(\.[0-9]+)?px' | sort | uniq -c | sort -rn
 *   semantics  grep -n "const OK = " docs/design/ds-reference.html
 *
 * These are 0xRRGGBB, matching ui_theme. The reference writes them #rrggbb.
 */
#ifndef ZL_DESIGN_H
#define ZL_DESIGN_H

/* ---- surfaces -------------------------------------------------------------
 * Eight steps, darkest first. This is a DEPTH LADDER, not a set of greys: a
 * surface drawn on top of another must be exactly one step lighter, and the
 * hairline between them is SURF_2. Skipping a step is what makes a panel look
 * pasted on rather than raised.
 *
 * The frequency counts are the whole argument for which step is which - the
 * one used 47 times is the hairline, the one used 32 times is the raised
 * surface every toolbar and sidebar and status bar is made of.
 */
#define ZD_SURF_0   0x07080A   /* 12x  the canvas behind everything          */
#define ZD_SURF_1   0x090A0C   /* 11x  sunken: input wells, segmented tracks */
#define ZD_SURF_2   0x0B0D0F   /* 47x  hairline/divider, and terminal ground */
#define ZD_SURF_3   0x101215   /* 18x  window interior - the content ground  */
#define ZD_SURF_4   0x14171A   /* 32x  raised: toolbars, sidebars, cards     */
#define ZD_SURF_5   0x1C2024   /* 30x  menus, popovers, modal bodies         */
#define ZD_SURF_6   0x22262B   /* 26x  borders, scrollbar thumb, chip faces  */
#define ZD_SURF_7   0x474B50   /* 16x  scrollbar hover, the dimmest ink      */

/* Three surfaces that sit BETWEEN the ladder steps. They are rare and each has
 * exactly one job, so they are named for the job rather than given a number -
 * a ladder with half-steps in it is not a ladder. */
#define ZD_SURF_TABS   0x171A1E /* 5x  the tab strip behind unselected tabs  */
#define ZD_SURF_WELL   0x0D0F12 /* 5x  calculator body, ping output well     */
#define ZD_SURF_CARD   0x1A1D21 /* 3x  System Info card border               */
#define ZD_SURF_HEAD   0x0F1114 /* 2x  the Clocks / System Info hero header  */
#define ZD_SURF_GAME   0x0B0D10 /* 2x  the game canvas ground                */
#define ZD_SURF_BODY   0x070809 /* 2x  <body> - a hair off SURF_0 in the ref */
#define ZD_SURF_BAR_OFF 0x131518 /* the UNFOCUSED title bar. ds-reference    */
                                 /* line 3110: focused '#171a1e' else       */
                                 /* '#131518'. One step down, not a tint.   */

/* ---- the window, exactly ---------------------------------------------------
 * ds-reference.html lines 3101-3132. These are quoted rather than summarised
 * because the window is the single most repeated object on the screen and
 * every one of these numbers is visible 20 times over.
 *
 *   radius        16px             ZD_R_16
 *   background    #101215          ZD_SURF_3
 *   title bar     36px tall, gap 5px, padding 0 8px
 *   title text    12px / weight 600 / letter-spacing .005em
 *                 focused #eef0f2, unfocused #8b9096
 *   subtitle      Roboto Mono 9px, focused #74797f, unfocused #5c6167
 *   controls      26x26, radius 16, rgba(255,255,255,.08), ink #b9bec4
 *   close hover   background #ff6a50, ink #0c0f05
 *
 * THE FOCUS RING IS THE SIGNATURE. A focused window carries a 1px lime ring
 * at 16% and a lime outer glow; an unfocused one carries a 1px white ring at
 * 3.5% and no glow. Focus is communicated by that ring, NOT by a coloured
 * title bar - which is why title and title_off are only one surface step
 * apart. Getting the ring right matters more than the bar colour.
 */
#define ZD_TITLE_H         36
#define ZD_TITLE_INK       ZD_TEXT_0   /* focused   */
#define ZD_TITLE_INK_OFF   0x8B9096    /* unfocused */
#define ZD_WINCTL          26          /* the square control button */
#define ZD_WINCTL_INK      ZD_TEXT_3   /* #b9bec4 */
#define ZD_CLOSE_HOVER_BG  ZD_BAD
#define ZD_CLOSE_HOVER_INK 0x0C0F05
#define ZD_RING_FOCUS      ZD_ACCENT   /* at 16% alpha */
#define ZD_RING_FOCUS_A    16          /* percent */
#define ZD_RING_BLUR_A     18          /* the outer lime glow, percent */
#define ZD_RING_OFF_A       4          /* white at 3.5%, rounded */

/* ---- the text ramp --------------------------------------------------------
 * Seven steps, brightest first. The reference sets #dfe2e5 as the ROOT colour
 * and reaches for #eef0f2 only to lift something above body text, so TEXT_1 is
 * the default and TEXT_0 is emphasis - not the other way round.
 */
#define ZD_TEXT_0   0xEEF0F2   /* 43x  emphasis: titles, values, bright rows */
#define ZD_TEXT_1   0xDFE2E5   /* 60x  BODY - the root colour of the desktop */
#define ZD_TEXT_2   0xC4C9CF   /*  5x                                        */
#define ZD_TEXT_3   0xB9BEC4   /* 10x                                        */
#define ZD_TEXT_4   0x9BA0A6   /* 54x  secondary: captions, status bars      */
#define ZD_TEXT_5   0x74797F   /* 40x  tertiary: labels, column heads        */
#define ZD_TEXT_6   0x5C6167   /* 37x  quaternary: hints, units, timestamps  */

/* ---- the accent -----------------------------------------------------------
 * `ACC = st.accent` with `accent:'#b8e838'` in the initial state, so the lime
 * IS the accent and everything else here is a fixed relation to it.
 * ds-reference.html line 1212 offers four alternates; they are a Settings
 * control, not part of the palette, and live at the bottom of this file.
 */
#define ZD_ACCENT      0xB8E838 /* 15x  the one saturated colour            */
#define ZD_ACCENT_BR   0xCDF25A /* 10x  brighter: links, live values        */
#define ZD_ACCENT_LINK 0xD8F588 /*  3x  link hover, hex bytes               */
#define ZD_ACCENT_PALE 0xDFF29A /*  3x  palest lime ink                     */

/* ---- semantics ------------------------------------------------------------
 * ds-reference.html line 3046, verbatim:
 *     const OK = '#a9e34b', BAD = '#ff6a50', WARN = '#f5b93c';
 * These are wired to STATE, not to the accent setting. The reference says so
 * itself in the Settings pane: "Amber means warning and red means failure.
 * Those two are wired to state, not to this setting."
 */
#define ZD_OK       0xA9E34B   /* healthy, running, pass                     */
#define ZD_BAD      0xFF6A50   /* failure only - and the close-box hover     */
#define ZD_WARN     0xF5B93C   /* warning - read-only mounts, drift          */
#define ZD_BAD_SOFT 0xFF9C88   /* the softer red, for text on a dark row     */

/* ---- the radius scale -----------------------------------------------------
 * Measured, with counts. The brief that commissioned this work summarised the
 * scale as 4/7/10/11/12/13/14/16; the file also uses 5, 6, 8, 9, 15, 17 and 20.
 * The measurement wins. 11px is the single most common radius in the reference
 * (26 uses) and 12px is second (18) - those two are the house shape.
 */
#define ZD_R_XS     4   /*  5x  the tiniest chips                            */
#define ZD_R_SM     5   /*  5x                                               */
#define ZD_R_6      6   /*  2x                                               */
#define ZD_R_7      7   /* 11x  meter bars, segmented tracks                 */
#define ZD_R_8      8   /*  9x                                               */
#define ZD_R_9      9   /* 15x  small buttons                                */
#define ZD_R_10    10   /* 12x                                               */
#define ZD_R_11    11   /* 26x  THE house radius - chips, pills, segments    */
#define ZD_R_12    12   /* 18x  cards                                        */
#define ZD_R_13    13   /* 13x  stat cards, popovers                         */
#define ZD_R_14    14   /* 14x  modals, search fields                        */
#define ZD_R_15    15   /*  1x  the command palette                          */
#define ZD_R_16    16   /* 10x  windows, the dock, the top island            */
#define ZD_R_17    17   /*  2x  the overview search field                    */
#define ZD_R_20    20   /*  1x  the lock-screen avatar                       */

/* ---- depth ----------------------------------------------------------------
 * There is no GPU here and no gaussian blur. These record what the reference
 * ASKS for; what zlOS actually does about it is fb.c's cached blur arena and
 * is documented as an approximation, not as parity.
 */
#define ZD_BLUR_PANEL   18   /* backdrop-filter: blur(18px)  notifications   */
#define ZD_BLUR_ISLAND  20   /* blur(20px) saturate(1.3)     the top island  */
#define ZD_BLUR_DOCK    22   /* blur(22px)                   dock, overview  */
#define ZD_BLUR_GLOW_A  30   /* filter: blur(30px)           wallpaper glow  */
#define ZD_BLUR_GLOW_B  34   /* filter: blur(34px)           wallpaper glow  */

/* Shadow: the reference's two window shadows, as (dy, blur, alpha_pct).
 * 0 20px 50px rgba(0,0,0,.72) and 0 20px 46px rgba(0,0,0,.7). */
#define ZD_SHADOW_DY     20
#define ZD_SHADOW_BLUR   50
#define ZD_SHADOW_ALPHA  72   /* percent */

/* ---- motion ---------------------------------------------------------------
 * The seven named keyframes, ds-reference.html lines 14-20. Durations are in
 * MILLISECONDS. The cubic-bezier is stored as its four control values times
 * 1000, because the kernel has no floating point in the drawing path.
 *
 *   zwin    .2s  cubic-bezier(.2,.85,.3,1)   window open
 *   zpop    .08/.1/.11s ease-out             menus, chips, tiles
 *   zpress  .25s ease                        button press
 *   zpulse  1s and 2.6s ease-in-out infinite activating / unfocused
 *   zov     .16s ease-out                    overlays, modals
 *   ztoast  .16s ease-out                    toasts
 *   zsweep  7s linear infinite               the wallpaper sweep
 */
#define ZD_MS_WIN      200
#define ZD_MS_POP       100  /* .08 and .11 also occur; .1 is the mode */
#define ZD_MS_POP_FAST   80
#define ZD_MS_POP_SLOW  110
#define ZD_MS_PRESS     250
#define ZD_MS_PULSE    1000
#define ZD_MS_PULSE_SLOW 2600
#define ZD_MS_OV        160
#define ZD_MS_TOAST     160
#define ZD_MS_SWEEP    7000

/* zwin's curve, x1 y1 x2 y2 scaled by 1000. */
#define ZD_EASE_WIN_X1  200
#define ZD_EASE_WIN_Y1  850
#define ZD_EASE_WIN_X2  300
#define ZD_EASE_WIN_Y2 1000

/* The transforms each keyframe starts from. Translations are in pixels,
 * scales are per-mille (965 == scale(.965)), opacity is percent. */
#define ZD_WIN_FROM_SCALE   965
#define ZD_WIN_FROM_DY       10
#define ZD_POP_FROM_SCALE   980
#define ZD_POP_FROM_DY       -6
#define ZD_PRESS_MIN_SCALE  880   /* zpress dips to .88 at 45% */
#define ZD_PRESS_MID_PCT     45
#define ZD_PULSE_FLOOR       55   /* opacity .55 */
#define ZD_OV_FROM_SCALE   1030   /* zov starts ABOVE 1 and settles down */
#define ZD_TOAST_FROM_DY     10

/* ---- type -----------------------------------------------------------------
 * Eight sizes, four of them half-pixel. zlOS draws from bitmap atlases, so
 * these are the sizes a runtime glyph scaler must hit - stored x2 to stay in
 * integers. See kernel/docs/type-scaling.md for what actually happens.
 */
#define ZD_T_9      18   /*  9.0px  stat-card keys, uppercase micro labels */
#define ZD_T_9_5    19   /*  9.5px  mono hints, timestamps                 */
#define ZD_T_10     20   /* 10.0px  small labels                           */
#define ZD_T_10_5   21   /* 10.5px  captions, column heads                 */
#define ZD_T_11     22   /* 11.0px  secondary body, list rows              */
#define ZD_T_11_5   23   /* 11.5px  mono list rows                         */
#define ZD_T_12     24   /* 12.0px  terminal and editor body               */
#define ZD_T_12_5   25   /* 12.5px  THE ROOT SIZE of the desktop           */

/* ---- the accent alternates ------------------------------------------------
 * ds-reference.html line 1212:
 *   const ACCENTS = ['#b8e838','#4ce0b3','#57b6ff','#8f7bff','#e86ec4'];
 * A Settings control, offered so the palette can be re-pointed at runtime.
 * ZD_ACCENT above is the default and the one everything is drawn against.
 */
#define ZD_ACCENT_ALT_1 0x4CE0B3
#define ZD_ACCENT_ALT_2 0x57B6FF
#define ZD_ACCENT_ALT_3 0x8F7BFF
#define ZD_ACCENT_ALT_4 0xE86EC4

#endif /* ZL_DESIGN_H */
