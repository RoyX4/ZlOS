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

/* ---- the two inks that go ON the accent ------------------------------------
 * ds-reference.html 3039-3045 COMPUTES this: the WCAG relative luminance of
 * the accent is compared against black-ish 0.0034 and white 1, and INK becomes
 * whichever wins the contrast ratio. On the default #b8e838 it resolves dark.
 *
 * docs/reference/ui/widgets.md S20.1 is the reason both are named here rather than
 * spelled at a call site: the reference writes ink-on-accent FOUR ways -
 * #0c1005 (INK, computed), #0d1005 (utility button, drag ghost), #0c0f05
 * (close hover) and a hard-coded #fff in three places that is a contrast
 * failure on the light lime. ui_ink_on() computes it; these are the only two
 * answers it can give.
 */
#define ZD_INK_DARK   0x0C1005 /*  2x  the reference's own INK  */
#define ZD_INK_LIGHT  0xFFFFFF /*  1x  INK's light alternate    */

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

/* ---- widget geometry, in REFERENCE pixels ---------------------------------
 * Every number below is a design-space pixel out of docs/reference/ui/widgets.md, and
 * every one of them must go through UI_DP() before it reaches fb.c. They live
 * here rather than in uikit.c for the same reason the colours do: the moment a
 * second widget spells `13` for the card radius, the two copies drift.
 *
 * WHERE THE REFERENCE DISAGREES WITH ITSELF, ONE VALUE IS PICKED AND THE
 * DISAGREEMENT IS RECORDED. docs/reference/ui/widgets.md S13.1 catalogues 26 pill
 * buttons that agree on nothing; S20 indexes eighteen more such splits. The
 * collapse below is that document's own recommendation, and a port built from
 * it differs from the reference by 1-4 px in places. That is deliberate: 26
 * one-off pills is not a design system, it is a file.
 */

/* the pill button, three sizes. S13.1's recommended collapse. */
#define ZD_PILL_SM_PY    3
#define ZD_PILL_SM_PX    9
#define ZD_PILL_SM_R     ZD_R_9
#define ZD_PILL_MD_PY    6      /* the reference says 5-6; 6 is the mode  */
#define ZD_PILL_MD_PX   13      /* the reference says 12-13               */
#define ZD_PILL_MD_R     ZD_R_11
#define ZD_PILL_LG_PY    7
#define ZD_PILL_LG_PX   15      /* the reference says 13 and 17; midpoint */
#define ZD_PILL_LG_R     ZD_R_12

/* the segmented control - S3. Container 2px pad, 2px gap, r11; item r9. */
#define ZD_SEG_PAD       2
#define ZD_SEG_GAP       2
#define ZD_SEG_R         ZD_R_11
#define ZD_SEG_ITEM_R    ZD_R_9
#define ZD_SEG_ITEM_PY   4
#define ZD_SEG_ITEM_PX  13

/* tab strips - S4.1 closeable, S4.2 underline */
#define ZD_TAB_H        30
#define ZD_TAB_PAD_T     4
#define ZD_TAB_PAD_X     5
#define ZD_TAB_GAP       3
#define ZD_TAB_PL       12      /* asymmetric: 12 left, 9 right */
#define ZD_TAB_PR        9
#define ZD_TAB_MAXW    170
#define ZD_TAB_R         7
#define ZD_TAB_X         9      /* the close glyph box */
#define ZD_UTAB_H       32
#define ZD_UTAB_PX      15
#define ZD_UTAB_RULE     2      /* inset 0 -2px 0 ACC */

/* bars - S5 toolbar, S6 status bar. The reference has five toolbar heights
 * and three status-bar heights (S20.16); 34 and 26 are the modes. */
#define ZD_TOOLBAR_H    34
#define ZD_TOOLBAR_PX   10
#define ZD_TOOLBAR_GAP   7
#define ZD_STATUS_H     26
#define ZD_STATUS_PX    11
#define ZD_STATUS_GAP   12

/* column header + list row - S7 */
#define ZD_COLHEAD_H    24
#define ZD_COLHEAD_PR   15      /* the scrollbar gutter pcolGrid forgets */
#define ZD_COLHEAD_PL    4
#define ZD_LISTROW_H    26
#define ZD_LISTROW_R    ZD_R_9
#define ZD_SEL_TINT_A   15      /* rgba(184,232,56,.15) - percent */
#define ZD_SEL_BAR_W     2      /* inset 2px 0 0 ACC */
#define ZD_ZEBRA_A       1      /* rgba(255,255,255,.014) rounded to 1% */

/* stat card strip - S8. Four instances use minmax 84/88/88/96 (S20.4). */
#define ZD_STAT_MIN     88
#define ZD_STAT_PY       7
#define ZD_STAT_PX       9
#define ZD_STAT_GAP      1      /* the 1px gap over SURF_2 IS the hairline */

/* sidebar - S9 */
#define ZD_SIDEBAR_W   158
#define ZD_SIDEBAR_PY    7
#define ZD_SIDEBAR_PX    6
#define ZD_NAV_H        28
#define ZD_NAV_PX        9
#define ZD_NAV_R        ZD_R_11
#define ZD_HEADING_PT    4
#define ZD_HEADING_PB    6

/* bars - S10. Radius and track colour differ per instance (S20.11). */
#define ZD_METER_H       5
#define ZD_METER_R      ZD_R_7
#define ZD_PROG_H        5
#define ZD_PROG_R       ZD_R_XS
#define ZD_SEGBAR_H     12
#define ZD_SEGBAR_R     ZD_R_7
#define ZD_SEGBAR_SEG_R ZD_R_SM
#define ZD_MINIBAR_H     6
#define ZD_MINIBAR_R    ZD_R_XS

/* toggle switch - S11. 3 + 16 + 21 == 40; the travel is asymmetric by 3. */
#define ZD_SW_W         40
#define ZD_SW_H         22
#define ZD_SW_R         ZD_R_14
#define ZD_SW_KNOB      16
#define ZD_SW_INSET      3

/* slider - S12 */
#define ZD_SLIDER_H      4
#define ZD_SLIDER_R     ZD_R_7
#define ZD_SLIDER_THUMB 15
#define ZD_SLIDER_THUMB_R ZD_R_12

/* cards and key/value - S15 */
#define ZD_CARD_R       ZD_R_13
#define ZD_CARD_PY      11
#define ZD_CARD_PX      13
#define ZD_CARD_HEAD_H  32
#define ZD_KV_H         22
#define ZD_KV_GAP       16

/* overlays - S16. Window menu 224 / gap 16 vs context menu 218 / gap 14
 * (S20.8); collapsed to one widget at the midpoint. */
#define ZD_MENU_W      220
#define ZD_MENU_PAD      5
#define ZD_MENU_R       ZD_R_13
#define ZD_MENU_ITEM_H  26
#define ZD_MENU_ITEM_PY  7
#define ZD_MENU_ITEM_PX 10
#define ZD_MENU_ITEM_R  ZD_R_9
#define ZD_MENU_GAP     14
#define ZD_MODAL_W     334
#define ZD_MODAL_R      ZD_R_14
#define ZD_MODAL_HEAD_H 42
#define ZD_MODAL_FOOT_H 40
#define ZD_TOAST_W     300
#define ZD_TOAST_R      ZD_R_14
#define ZD_TOAST_PY     12
#define ZD_TOAST_PX     13
#define ZD_TOAST_GAP    11
#define ZD_TOAST_ICON   22
#define ZD_TOAST_ICON_R ZD_R_14
#define ZD_TOAST_MS   4200      /* auto-dismiss, ds-reference.html 2362 */
#define ZD_TOAST_MAX     3      /* slice(-3), 2358                     */

/* chart - S17. The GAIN IS DELIBERATELY ABSENT; see S20.18. */
#define ZD_SPARK_H      66
#define ZD_SPARK_GRID    4      /* quarters: 3 lines at h/4, h/2, 3h/4 */
#define ZD_SPARK_AREA_A 16      /* rgba(184,232,56,.16) */
#define ZD_SPARK_MAX    64      /* samples the toolkit will buffer     */

/* indicators and inputs - S18 */
#define ZD_DOT           7
#define ZD_DOT_R        ZD_R_SM
#define ZD_BADGE_PY      3
#define ZD_BADGE_PX      8
#define ZD_BADGE_R      ZD_R_8
#define ZD_BADGE_TINT_A 14      /* rgba(<col>,.14) */
#define ZD_INPUT_H      26
#define ZD_INPUT_PX      9
#define ZD_INPUT_R      ZD_R_10
#define ZD_SEARCH_H     21
#define ZD_SEARCH_R     ZD_R_14
#define ZD_CHIP_PY       4
#define ZD_CHIP_PX       9
#define ZD_CHIP_R       ZD_R_9

/* Neutral fills, as alpha over white. The reference writes these as
 * rgba(255,255,255,.05 .06 .07 .08 .09); .07 is the button fill, 14 uses. */
#define ZD_FILL_A        7
#define ZD_FILL_SOFT_A   6
#define ZD_FILL_ICON_A   8

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

/* ---- the wallpaper --------------------------------------------------------
 * ds-reference.html lines 37-45. Nine stacked layers, and zlOS already draws
 * all nine in the right places - draw_wallpaper() in kernel.zl was written
 * against this exact structure. What it did NOT take was the colour: its
 * comment said so outright, "its structure is taken exactly; its palette is
 * NOT... this is navy and cyan". That decision is reversed with the rest of
 * the palette, and these are the values it should have had.
 *
 * THIS IS THE LARGEST SINGLE HUE ERROR ON THE SCREEN. The oracle reports most
 * regions as hue-WRONG at 176-192 degrees against the reference's 75, and in
 * nearly every one of those regions zlOS has no window - what is being
 * measured is the wallpaper showing through. One teal gradient is responsible
 * for most of the colour error in the whole comparison.
 *
 * Alphas are given as the reference's fraction and as 0..255, because the
 * drawing primitives take the latter and the reference states the former.
 */
#define ZD_WALL_0    0x0A1005  /* linear-gradient(168deg, this 0%,      */
#define ZD_WALL_42   0x080A0B  /*                 this 42%,             */
#define ZD_WALL_100  0x07080A  /*                 this 100%)            */

#define ZD_WALL_GLOW_A   0x7EA62C  /* rgba(126,166,44,.3)  at 16% 8%   */
#define ZD_WALL_GLOW_A_A 77        /* .3  * 255                        */
#define ZD_WALL_GLOW_B   0x2E3A14  /* rgba(46,58,20,.62)   at 88% 92%  */
#define ZD_WALL_GLOW_B_A 158       /* .62 * 255                        */
#define ZD_WALL_GLOW_C   0x182026  /* rgba(24,32,38,.7)    at 76% 18%  */
#define ZD_WALL_GLOW_C_A 179       /* .7  * 255                        */

/* The two conic wedges, blurred 30px and 34px. Both are the accent. */
#define ZD_WALL_WEDGE_A_A 19       /* .075 * 255 */
#define ZD_WALL_WEDGE_B_A 11       /* .045 * 255 */

/* The two-scale technical grid. Both are the accent; the vertical lines are a
 * shade stronger than the horizontal ones in both scales, which is the
 * reference's own asymmetry and not a transcription slip. */
#define ZD_WALL_GRID       64      /* px */
#define ZD_WALL_GRID_MAJOR 252     /* px - NOT 256; the reference's own value */
#define ZD_WALL_GRID_V_A    7      /* .028 * 255 */
#define ZD_WALL_GRID_H_A    5      /* .02  * 255 */
#define ZD_WALL_MAJOR_V_A  13      /* .05  * 255 */
#define ZD_WALL_MAJOR_H_A  10      /* .04  * 255 */

/* radial-gradient(120% 96% at 50% 40%, transparent 26%, rgba(0,0,0,.82) 100%) */
#define ZD_WALL_VIGNETTE_A 209     /* .82 * 255 */

#endif /* ZL_DESIGN_H */
