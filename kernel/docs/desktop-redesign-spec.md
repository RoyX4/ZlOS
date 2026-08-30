# PLATE — the frozen desktop specification

**Status: FROZEN CONTRACT.** Four implementation agents build from this document
without talking to each other or to the author. Where this document and any
other document in the tree disagree, **this document wins** — including
`docs/design/desktop-look.md`, whose ranked bug list is partially stale (see
§0.3), and including `design.h`'s own header comment, which becomes false the
moment this lands and is rewritten in the same commit (§10.1).

---

## 0. THE DECISION

### 0.1 Winner

**PLATE (Proposal 1)** — *zlOS is a printed technical document: warm paper,
black ink, hard rules, one vermilion overprint, and the modular grid the windows
are set on is ruled onto the desk where you can see it.*

| Proposal | identity | buildable | regression | verdict |
|---|---|---|---|---|
| **PLATE** | **9/10 (best)** | **8/10 (best)** | 7/10 | **WINNER** |
| Folio | 8/10 | 5/10 | 7/10 | runner-up, grafted from |
| Datum | 7/10 | 3/10 (worst) | 4/10 (worst) | rejected |
| Raking | 4/10 (worst) | 7/10 | **9/10 (best)** | rejected, grafted from |

**Why PLATE and not Raking**, which won the regression lens outright and was the
best-researched document of the four. The brief's first gate is the default
test, and Raking fails it in its own risk list: *"STEEL IS A BLUE ACCENT, THE
MOST COMMON ACCENT HUE IN SOFTWARE"*, defended on a saturation value. Its second
failure is macrostructure — it swaps top-bar+dock for a left icon rail, which is
the Windows 11 / Unity / VS Code arrangement, i.e. one of the two most common
desktop shapes for the other. Its third is fatal on this brief: its identity
lives in 1px lit/cut edge runs against surface steps of 1.07:1 to 1.13:1, so a
cropped screenshot is anonymous. A design whose identity can be deleted by one
missing `fb_fill_px` call does not have an identity. Craft is not the brief.

**Why PLATE and not Folio**, which is close and is the most honest document in
the set. Folio deletes the top bar and the dock but keeps floating stacking
windows with snap guides — it passes the macrostructure gate by *deletion*, not
by *replacement*. It also carries two code-contract errors that would ship
silently (a `-1` radius sentinel that `fb_rrect` resolves to 0, not h/2; and
"R1 mono" figures that come out 32px tall because mono has one size).

**Why not Datum.** Its central idea — *every number the machine measured is set
in monospace* — requires sized monospace, which this toolkit does not have and
this spec does not add. `uikit.c:143` and `uikit.c:151` ignore the size argument
for `UI_F_MONO` and route to `fb_text_aa`, which draws the console cell. Six
type sizes against three atlases is the current eight-step failure with two
rungs removed.

### 0.2 What was grafted, and from where

1. **The functional grid — from Folio.** PLATE draws the module grid on the
   desk. Folio's move is better: while a window is dragged, the modules it will
   land on *fill*, so the "decoration" is the layout system made visible and
   operative in one stroke. Adopted verbatim as §6.4. This is the single
   grafted idea that changes behaviour rather than paint.
2. **Hairlines are one DEVICE pixel, never scaled — from Datum.** PLATE's
   two-weight rule system is kept, but the 1px rule is pinned to one physical
   pixel at every UI scale (`fb_fill_px`, not `UI_DP`). A 2px "hairline" is a
   border, and at ui=2 an unscaled rule is what makes the page read as printed
   rather than drawn. Structural rules stay in design px. See §2.4.
3. **No sentinel radii, ever — from Raking (and from Folio's defect).** Any
   radius that is a function of height is computed *in the widget* from that
   widget's own height and passed to `fb_rrect` as a positive integer.
   `fb_rrect` clamps `r < 0` to `0` (`fb.c:1988`, verified), so a `-1` sentinel
   renders a square with no compile error and no assert.
4. **The factual corrections — from Raking.** Both of PLATE's "hard
   prerequisites" are already done in this tree and are struck from the plan
   (§0.3). Raking is the only proposal that checked.
5. **A judgeable first increment — from Raking.** §11 defines an increment that
   changes every window in `hosttest/wmshot` with `kernel.zl` untouched, so the
   direction can be looked at before the rail is built.

Everything else is PLATE. The result reads as one direction: a printed page.

### 0.3 Two phantom prerequisites, struck

PLATE's plan makes two items hard blockers. **Both are already fixed in this
tree. Do not do either. Do not cite `docs/design/desktop-look.md` for them.**

- **Icon atlas.** `fb_icon24` (`fb.c:3635-3656`) selects the native atlas per
  scale and bilinear-resamples only for scales neither atlas covers.
  `icons.c:107` is `icons24[89][24][24]`, `icons.c:2424` is
  `icons48[89][48][48]`, and `gen_icons.py`'s `render()` builds a separate
  4x-supersampled master per output size. There is no nearest-neighbour upscale.
  The icon work in §7 is a **grammar redraw only**, not a resolution fix.
- **`fb_line`.** `fb.c:2453` is Wu's algorithm with a 16.16 accumulator and two
  `blend_px` calls per step, and the comment above it narrates the migration
  from Bresenham. Diagonals are already anti-aliased.

### 0.4 One capability PLATE asked for that is NOT in scope

PLATE proposes routing `fb_text_aa` through `blend_sub` so subpixel LCD
rendering reaches every table and readout. **Frozen as a non-goal.** Verified:
`draw_glyph`'s subpixel branch is gated on `subpixel_on && back_on &&
!surface_on` (`fb.c:1394-1395`), and `wm.c` renders both chrome and client
content into a retained surface, so `surface_on` is true for essentially all
desktop text. `surface_alpha` is `unsigned char *` — **one** byte per pixel
(`fb.c:357`) — so three alphas cannot be retained without changing the surface
format. This is a real subsystem change, not a routing tweak. Subpixel stays
exactly as it is and keeps serving the boot console. Nobody spends a day on it
under this spec.

---

## 1. THE PALETTE — full `design.h` colour replacement

Every value is `0xRRGGBB`. A colour literal may appear in `design.h` and in **no
other file**; the surviving violations in `kernel.zl` (`island_fill()` at line
427 returning `rgb(12,14,16)`, `dock_fill()` at line 590 returning
`rgb(18,21,24)`, and `wall_glow_a/b/c()` at 4607-4609) disappear because the
island, the dock and the nine-layer wallpaper are all deleted (§6).

Contrast ratios below were **computed**, not asserted, with an independent sRGB
relative-luminance implementation (WCAG 2.x, `(L1+0.05)/(L2+0.05)`). Every
number in this section is machine output.

### 1.1 Surfaces — four, and a rule that replaces the eight-step ladder

```c
#define ZD_PAPER    0xF2EFE6  /* THE SHEET. Window interiors, panels, cards,
                                 menus, modals. The only surface text is set on
                                 by default. Warm (R>G>B) so it reads as stock. */
#define ZD_DESK     0xE4E0D5  /* THE FIELD. Behind the windows. Flat. One value
                                 darker than the sheet. Replaces the wallpaper. */
#define ZD_WELL     0xD6D0C0  /* SUNKEN. Inputs, meter and progress tracks, code
                                 and log wells, segmented grounds. Carries no
                                 rule of its own; its 1px ZD_RULE_1 border is
                                 what defines it. */
#define ZD_FILL     0xBFB8A4  /* TINT. Zebra stripe on alternate table rows and
                                 unselected segment faces. NO TEXT EVER. */
#define ZD_PLATE_0  0x14120E  /* THE INK PLATE. The rail, and a FOCUSED window
                                 header. Text on it is ZD_PAPER knocked out.
                                 This is how focus is signalled. */
#define ZD_PLATE_1  0x2A2721  /* SECOND PLATE. The terminal / mono-panel ground,
                                 and the pressed state of a control that sits on
                                 ZD_PLATE_0. */
```

**THE RULE THAT REPLACES THE LADDER.** Nesting a surface inside another does
**not** change its value. A panel inside a window is the same `ZD_PAPER` as the
window; what marks it is a rule. Value changes only when the surface changes
**kind** — sheet, desk, well, plate. This is why brief item 3 (a fifteen-step
radius scale collapsed to one value) disappears rather than being patched:
there is nothing left for a fifteen-step radius scale or an eight-step grey
ladder to encode.

Surface separation, measured (all non-text, all deliberate):

| pair | ratio |
|---|---|
| ZD_DESK on ZD_PAPER | 1.15 |
| ZD_WELL on ZD_PAPER | 1.34 |
| ZD_FILL on ZD_PAPER | 1.72 |
| ZD_WELL on ZD_DESK | 1.17 |
| ZD_PLATE_0 on ZD_PAPER | **16.27** |
| ZD_PLATE_0 on ZD_DESK | **14.18** |
| ZD_PLATE_1 on ZD_PLATE_0 | 1.26 |

### 1.2 Rules and the printed grid

```c
#define ZD_RULE_1   0x5C574C  /* THE 1px RULE. Row dividers, cell dividers, well
                                 borders, the line under a column head. Same
                                 value as ZD_TEXT_4 deliberately — a rule is
                                 ink, not a grey. */
#define ZD_RULE_2   0x14120E  /* THE 2dp RULE. Window frames, region boundaries,
                                 the line under the slug. Same value as
                                 ZD_TEXT_0. Structure is the darkest thing on
                                 screen. */
#define ZD_RULE_KO  0x4A443B  /* the 1px rule ON the ink plate: register row
                                 dividers, readout separators. Non-text. */
#define ZD_GRID_LINE 0xA69E8A /* THE PRINTED GRID on the desk. 1px at every
                                 module boundary. Non-text, non-interactive. */
```

| rule | on | ratio | note |
|---|---|---|---|
| ZD_RULE_1 | ZD_PAPER | 6.25 | legible as ink |
| ZD_RULE_1 | ZD_DESK | 5.45 | |
| ZD_RULE_1 | ZD_WELL | 4.67 | clears 4.5 even as a border on a well |
| ZD_RULE_1 | ZD_FILL | 3.63 | still clears WCAG 1.4.11's 3:1 |
| ZD_RULE_2 | ZD_PAPER | 16.27 | |
| ZD_RULE_2 | ZD_DESK | 14.18 | |
| ZD_RULE_2 | ZD_WELL | 12.15 | |
| ZD_RULE_KO | ZD_PLATE_0 | 1.94 | 1px divider on ink, deliberately quiet |
| ZD_GRID_LINE | ZD_DESK | 2.02 | visible as structure, quiet under a window edge |
| ZD_GRID_LINE | ZD_PAPER | 2.32 | (never drawn there; recorded for completeness) |

### 1.3 The ink ramp — seven steps, names unchanged

Every `ZD_TEXT_n` name survives with its ordinal meaning intact, so the ~40
existing references in `ui.c`, `uikit.c` and `wm.c` compile unchanged and simply
invert.

```c
#define ZD_TEXT_0   0x14120E  /* EMPHASIS INK. Window titles, the one PLATE-size
                                 mark per view, the value that is the answer to
                                 the question the window asks. */
#define ZD_TEXT_1   0x26231D  /* BODY INK. The root colour. All prose, all mono
                                 data, all list rows. */
#define ZD_TEXT_2   0x3A362E  /* strong secondary: a selected row's label, a
                                 table header set in ink rather than in caps. */
#define ZD_TEXT_3   0x4A4539  /* control ink at rest: window-control glyphs on
                                 an unfocused header, icon ink on a sheet. */
#define ZD_TEXT_4   0x5C574C  /* SECONDARY. Units, the slug line, inactive rail
                                 entries, disabled-but-legible. */
#define ZD_TEXT_5   0x5C5646  /* LABEL. Tracked uppercase column heads, section
                                 heads, field captions. Darkened from the
                                 obvious #6E6858 specifically so it clears 4.5:1
                                 on ZD_WELL, because column heads sit above
                                 wells. */
#define ZD_TEXT_6   0x655F4E  /* HINT. Placeholders, timestamps, keybinding
                                 hints. Permitted on ZD_PAPER and ZD_DESK ONLY —
                                 4.13:1 on ZD_WELL. */
```

Measured, ink on every paper surface:

| ink | on PAPER | on DESK | on WELL | on FILL |
|---|---|---|---|---|
| ZD_TEXT_0 `#14120E` | **16.27** | **14.18** | **12.15** | 9.45 |
| ZD_TEXT_1 `#26231D` | **13.62** | **11.88** | **10.18** | 7.91 |
| ZD_TEXT_2 `#3A362E` | **10.45** | **9.11** | **7.81** | 6.07 |
| ZD_TEXT_3 `#4A4539` | **8.30** | **7.23** | **6.20** | 4.82 |
| ZD_TEXT_4 `#5C574C` | **6.25** | **5.45** | **4.67** | 3.63 ✗ |
| ZD_TEXT_5 `#5C5646` | **6.35** | **5.54** | **4.75** | 3.69 ✗ |
| ZD_TEXT_6 `#655F4E` | **5.53** | **4.82** | 4.13 ✗ | 3.21 ✗ |

✗ = below 4.5:1. **`ZD_FILL` carries no text, at all, ever** — it is the zebra
stripe and the unselected segment face, and every ink fails on it or nearly so.
**`ZD_TEXT_6` is banned on `ZD_WELL`.** These two restrictions are gated in
§10.2, not left as comments.

### 1.4 Knockouts — ink-plate text

```c
#define ZD_KO_0     0xF2EFE6  /* == ZD_PAPER. Knocked-out emphasis on the plate:
                                 focused window title, focused rail row. */
#define ZD_KO_1     0xE4E0D5  /* == ZD_DESK. Knocked-out body on the plate:
                                 window subtitle, rail values. */
#define ZD_KO_2     0x9C958A  /* knocked-out secondary: build string, inactive
                                 rail names, the coordinate readout. */
```

| knockout | on ZD_PLATE_0 | on ZD_PLATE_1 |
|---|---|---|
| ZD_KO_0 | **16.27** | **12.95** |
| ZD_KO_1 | **14.18** | **11.29** |
| ZD_KO_2 | **6.31** | **5.02** |

The 12.95:1 figure is the load-bearing one: it is the value distance between a
focused header (plate, knocked out) and an unfocused header (paper, ink). Focus
is a full material inversion, which is the most legible focus signal available
and costs no ring, no hue shift and no second signal.

### 1.5 The accent — the second ink

```c
#define ZD_ACCENT      0xC4341C /* THE SECOND INK. Vermilion. Appears in exactly
                                   four places: the 3dp run/focus mark on a rail
                                   row and on the focused window's left inner
                                   edge; the fill of the one primary action per
                                   view; the register mark terminating the slug;
                                   and the crop marks at the field corners.
                                   NEVER for status, NEVER for text, NEVER for
                                   an icon. */
#define ZD_ACCENT_KO   0xD14A2E /* ACCENT ON PLATE. The only lightened accent,
                                   used SOLELY as a graphic mark on
                                   ZD_PLATE_0/1 where #C4341C falls to 3.43:1.
                                   Never carries text. */
#define ZD_ACCENT_BR   0xC4341C /* == ZD_ACCENT. The concept "a brighter accent
                                   for live values" is deleted; the name is kept
                                   so existing call sites compile. Live values
                                   are ZD_TEXT_0, not a hue. */
#define ZD_ACCENT_LINK 0x9E2814 /* link HOVER. On paper a hover goes DARKER, not
                                   brighter. 6.60:1 on ZD_PAPER. */
#define ZD_ACCENT_PALE 0xC4341C /* == ZD_ACCENT. Kept for call-site compatibility
                                   only; a pale accent has no role on paper. */
```

| pair | ratio | permitted use |
|---|---|---|
| ZD_ACCENT on ZD_PAPER | **4.74** | text-capable, but reserved for marks |
| ZD_ACCENT on ZD_DESK | 4.13 | marks only (crop marks, register mark) |
| ZD_ACCENT on ZD_WELL | 3.54 | marks only |
| ZD_PAPER on ZD_ACCENT | **4.74** | the primary button's caption |
| ZD_ACCENT on ZD_PLATE_0 | 3.43 | **banned** — use ZD_ACCENT_KO |
| ZD_ACCENT_KO on ZD_PLATE_0 | 4.21 | graphic marks only, no text |
| ZD_ACCENT_KO on ZD_PLATE_1 | 3.35 | graphic marks only, no text |
| ZD_ACCENT_LINK on ZD_PAPER | **6.60** | text |

### 1.6 State — colour is never the only channel

```c
#define ZD_OK       0x1F6B3A  /* PASS. Press green. Always accompanied by the
                                 tracked-caps word OK. */
#define ZD_WARN     0x7D5200  /* WARN. Ochre. Always accompanied by WARN. */
#define ZD_BAD      0xA31D11  /* FAIL. Oxblood — deliberately darker and less
                                 orange than the accent. Always with FAIL. */
#define ZD_OK_KO    0x5FC07E  /* PASS, knocked out on the ink plate. */
#define ZD_WARN_KO  0xE5B14A  /* WARN, knocked out on the ink plate. */
#define ZD_BAD_KO   0xFF8E76  /* FAIL, knocked out on the ink plate. */
#define ZD_BAD_SOFT ZD_BAD_KO /* alias: the existing name for "red text on a
                                 dark row", which is now the knockout. */
```

| state | on PAPER | on DESK | on WELL | knockout on PLATE_0 |
|---|---|---|---|---|
| ZD_OK | **5.67** | **4.94** | 4.23 ✗ | ZD_OK_KO **8.31** |
| ZD_WARN | **5.93** | **5.17** | 4.43 ✗ | ZD_WARN_KO **9.55** |
| ZD_BAD | **6.66** | **5.81** | **4.98** | ZD_BAD_KO **8.36** |

**Hue is a redundant channel here, not the signal.** Every state occurrence
carries the tracked-caps word as well as the colour. A greyscale screenshot of
this desktop loses nothing.

### 1.7 Ink on a fill — and why `ui_ink_on()` needs no change

```c
#define ZD_INK_DARK   0x14120E /* == ZD_TEXT_0 */
#define ZD_INK_LIGHT  0xF2EFE6 /* == ZD_PAPER */
```

`ui.c:361-365` picks between these by comparing `ui_luminance_q16(bg)` against
`12242` (L = 0.18680). **Verified against the new palette** — the function is
correct as written and must not be touched:

| fill | measured L | q16 | `ui_ink_on()` returns | resulting ratio |
|---|---|---|---|---|
| ZD_ACCENT | 0.14276 | 9356 | ZD_INK_LIGHT | **4.74** |
| ZD_OK | 0.11112 | 7282 | ZD_INK_LIGHT | **5.67** |
| ZD_WARN | 0.10395 | 6812 | ZD_INK_LIGHT | **5.93** |
| ZD_BAD | 0.08706 | 5705 | ZD_INK_LIGHT | **6.66** |
| ZD_WELL | 0.63214 | 41428 | ZD_INK_DARK | **12.15** |
| ZD_FILL | 0.48038 | 31482 | ZD_INK_DARK | 9.45 |
| ZD_DESK | 0.74609 | 48896 | ZD_INK_DARK | **14.18** |
| ZD_PAPER | 0.86323 | 56573 | ZD_INK_DARK | **16.27** |

Every one resolves correctly on the first try. That is not luck — it is what a
palette with a real value gap looks like.

### 1.8 Accent alternates — the Settings control

The four neon alternates are replaced by four second inks that survive on paper.
`ZD_ACCENT` remains the default and the one everything is drawn against.

```c
#define ZD_ACCENT_ALT_1 0x2F3E9E  /* indigo   7.93:1 on ZD_PAPER, both ways */
#define ZD_ACCENT_ALT_2 0x0F6B6B  /* teal     5.48:1 */
#define ZD_ACCENT_ALT_3 0x8A5A00  /* ochre    5.15:1 */
#define ZD_ACCENT_ALT_4 0x7A2A6B  /* plum     7.70:1 */
```

All four fall below the `ui_ink_on()` threshold, so a caption on any of them is
`ZD_INK_LIGHT` and clears 4.5:1. **`ZD_OK` / `ZD_WARN` / `ZD_BAD` remain wired
to state and are never re-pointed by this control** — the existing rule, kept.

### 1.9 Compatibility block — how 126 `ZD_*` references in `uikit.c` keep compiling

`uikit.c` holds 126 `ZD_*` references, `ui.c` 47, `hosttest/palette.c` 43,
`hosttest/uitest.c` 112. A flag-day rename would serialise four agents onto one
file. Instead `design.h` ships a **compatibility block**, clearly fenced, with a
stated deletion phase:

```c
/* ---- COMPATIBILITY, PHASE 2 ONLY -------------------------------------------
 * The eight-step surface ladder is gone (§1.1). These names map the old ladder
 * onto the four surfaces that survive so existing call sites compile and
 * INVERT CORRECTLY on day one. Every one of these is deleted in phase 4, after
 * the call sites have been rewritten to the four real names. Do not add a new
 * use of any name below. */
#define ZD_SURF_0       ZD_DESK    /* was: the canvas behind everything   */
#define ZD_SURF_1       ZD_WELL    /* was: sunken                         */
#define ZD_SURF_2       ZD_RULE_1  /* was: the 47-use hairline            */
#define ZD_SURF_3       ZD_PAPER   /* was: window interior                */
#define ZD_SURF_4       ZD_PAPER   /* was: raised — NESTING NO LONGER     */
#define ZD_SURF_5       ZD_PAPER   /* was: menus, popovers, modal bodies  */
#define ZD_SURF_6       ZD_RULE_1  /* was: borders, chip faces            */
#define ZD_SURF_7       ZD_TEXT_4  /* was: the dimmest ink                */
#define ZD_SURF_TABS    ZD_WELL
#define ZD_SURF_WELL    ZD_WELL
#define ZD_SURF_CARD    ZD_PAPER
#define ZD_SURF_HEAD    ZD_PAPER
#define ZD_SURF_GAME    ZD_PLATE_1 /* a game canvas is a mono panel       */
#define ZD_SURF_BODY    ZD_DESK
#define ZD_SURF_BAR_OFF ZD_PAPER   /* the UNFOCUSED title bar IS the sheet */
```

The deliberate consequence: on day one, several widgets draw one surface where
they used to draw two adjacent ladder steps. That is not a bug — it *is* the
new rule (§1.1), and the rules added in phase 3 are what re-separate them.

### 1.10 Deleted outright

`ZD_WALL_0`, `ZD_WALL_42`, `ZD_WALL_100`, `ZD_WALL_GLOW_A/B/C`,
`ZD_WALL_GLOW_A_A/B_A/C_A`, `ZD_WALL_WEDGE_A_A/B_A`, `ZD_WALL_GRID`,
`ZD_WALL_GRID_MAJOR`, `ZD_WALL_GRID_V_A/H_A`, `ZD_WALL_MAJOR_V_A/H_A`,
`ZD_WALL_VIGNETTE_A` — 18 tokens, all of them describing a nine-layer wallpaper
that no longer exists.

`ZD_RING_FOCUS`, `ZD_RING_FOCUS_A`, `ZD_RING_BLUR_A`, `ZD_RING_OFF_A` — 4
tokens. Focus is inversion plus one bar; there is no ring.

`ZD_BLUR_PANEL`, `ZD_BLUR_ISLAND`, `ZD_BLUR_DOCK`, `ZD_BLUR_GLOW_A`,
`ZD_BLUR_GLOW_B`, `ZD_SHADOW_DY`, `ZD_SHADOW_BLUR`, `ZD_SHADOW_ALPHA` — 8
tokens. See §8.

`ZD_FILL_A`, `ZD_FILL_SOFT_A`, `ZD_FILL_ICON_A`, `ZD_SEL_TINT_A`, `ZD_ZEBRA_A`,
`ZD_BADGE_TINT_A`, `ZD_SPARK_AREA_A` — 7 tokens. The rgba-over-white idiom does
not survive on paper: white at 7% over `#F2EFE6` is invisible. Replaced by flat
surfaces, §3.4.

Plus **15** radius tokens (§2.2), **8** type tokens (§4), and **23** motion
tokens (§8: 10 durations + 9 transforms + 4 easing control values).

**Ledger, so the arithmetic is auditable rather than asserted:**

| deleted | n | added | n |
|---|---|---|---|
| wallpaper | 18 | surfaces | 6 |
| focus rings | 4 | rules + grid line | 4 |
| blur + shadow | 8 | knockouts | 3 |
| alpha fills | 7 | accent-on-plate | 1 |
| radius scale | 15 | state knockouts | 3 |
| type ramp | 8 | radius | 2 |
| motion | 23 | metrics (§3.2) | 10 |
| | | registers (§4.2) | 3 |
| | | motion | 2 |
| | | `ZD_SW_KNOB_R` | 1 |
| **total** | **83** | **total** | **35** |

**Net −48 tokens.** Generic designs add; this one subtracts, and it subtracts
because nothing calls the tokens any more — which is the honest reason.

---

## 2. THE RADIUS SYSTEM

**Two values, and the second is derived from the rasteriser rather than chosen.**
The fifteen-step `ZD_R_XS`…`ZD_R_20` scale is deleted outright. Brief item 3 is
not that `ui.c` collapses the scale to one value — it is that a fifteen-value
radius scale measured out of an external artifact was never a system.

### 2.1 The two values

```c
#define ZD_R_0   0   /* everything that CONTAINS something */
#define ZD_R_2   2   /* only objects that ARE a token, and only below 24dp */
```

**`ZD_R_0` — the default, ~95% of drawn rectangles.** Window frames, headers,
the rail, plates, wells, toolbars, status bars, list rows, table cells, tab
faces, buttons, meter tracks and fills, cards, menus, modals, toasts, popovers,
input fields, segmented controls and their items. *A printed rule has no corner
and neither does a container.*

**`ZD_R_2` — four objects, and only four.** The status dot, the chip, the toggle
knob, the slider thumb. **The rule is size-derived and stated as such:** 2 is the
smallest radius `fb_rrect`'s 4x4 supersampled corner (`fb.c:1997-2031`) can
express as a *curve* rather than as a single softened pixel, so below 24dp a
radius-0 square with AA edges is already reading as a lozenge and 2 makes that
honest. Above 24dp the same object goes to 0. **The two values are not two
tastes — they are one threshold.**

### 2.2 The resolution table

Every surviving radius token becomes an alias. This is mechanical and complete;
an implementer never has to decide.

| resolves to `ZD_R_0` | resolves to `ZD_R_2` |
|---|---|
| `ZD_PILL_SM_R` `ZD_PILL_MD_R` `ZD_PILL_LG_R` | `ZD_DOT_R` |
| `ZD_SEG_R` `ZD_SEG_ITEM_R` | `ZD_CHIP_R` |
| `ZD_TAB_R` `ZD_NAV_R` `ZD_LISTROW_R` | `ZD_SW_KNOB_R` *(new name; see §2.3)* |
| `ZD_CARD_R` `ZD_MENU_R` `ZD_MENU_ITEM_R` | `ZD_SLIDER_THUMB_R` |
| `ZD_MODAL_R` `ZD_TOAST_R` `ZD_TOAST_ICON_R` | |
| `ZD_INPUT_R` `ZD_SEARCH_R` `ZD_SW_R` | |
| `ZD_SLIDER_R` `ZD_METER_R` `ZD_PROG_R` | |
| `ZD_SEGBAR_R` `ZD_SEGBAR_SEG_R` `ZD_MINIBAR_R` | |
| `ZD_BADGE_R` | |

`ZD_R_XS`, `ZD_R_SM`, `ZD_R_6`, `ZD_R_7`, `ZD_R_8`, `ZD_R_9`, `ZD_R_10`,
`ZD_R_11`, `ZD_R_12`, `ZD_R_13`, `ZD_R_14`, `ZD_R_15`, `ZD_R_16`, `ZD_R_17`,
`ZD_R_20` — **all fifteen deleted.**

### 2.3 No sentinels — grafted from Raking, and the reason

`ZD_SW_R` was `ZD_R_14` on a 22dp switch, i.e. an accidental capsule.
`ZD_SLIDER_THUMB_R` was `ZD_R_12` on a 15dp thumb. Under this spec both go to
their table value above, and **no widget passes a negative or sentinel radius to
`fb_rrect`.** Verified in the code: `fb.c:1986-1988` reads

```c
if (2 * r > w) r = w / 2;
if (2 * r > h) r = h / 2;
if (r < 0) r = 0;
```

`2 * -1` is not greater than `w` or `h`, so a `-1` "capsule sentinel" falls
through both clamps and becomes **0**, rendering a square with no compile error
and no assert. Any height-derived radius is computed in the widget from that
widget's own height and passed as a positive integer. This is written down
because it is exactly how Folio's toggles would have shipped square.

### 2.4 Rule weights are part of the shape system

Rules replace the radii that were deleted. Two weights, and the weight tells you
the rank of the boundary:

- **HAIRLINE** — `ZD_RULE_1` (or `ZD_RULE_KO` on ink), **exactly 1 device
  pixel, never through `UI_DP()`**, drawn with `fb_fill_px`. Row dividers, cell
  dividers, control-group separators, well borders, the line under a column
  head. *(Grafted from Datum: a 2px "hairline" is a border.)*
- **STRUCTURE** — `ZD_RULE_2`, `UI_DP(t, 2)` device pixels (4 at ui=2), drawn
  with `fb_fill_px`. Window frames, the rail's boundary, the line under the
  slug, the line under a toolbar, the line under an unfocused window header.

There is no third weight. If a boundary needs to be louder than STRUCTURE, it
is not a boundary — it is an inversion (§3.2).

---

## 3. THE METRICS

### 3.1 `ui_theme_init_q8()` — exact assignments

```c
theme.scale_q8 = scale_q8;
theme.scale    = (scale_q8 + 128) >> 8;  if (theme.scale < 1) theme.scale = 1;
theme.pad      = dp(12, scale_q8);   /* unchanged */
theme.gap      = dp( 8, scale_q8);   /* unchanged */
theme.row_h    = dp(24, scale_q8);   /* was 28 */
theme.radius   = dp( 0, scale_q8);   /* was 16 — resolves to 0 at every scale */
theme.title_h  = dp(28, scale_q8);   /* was 36 */
```

All five stay on the 4/8/12/16/24 scale. `row_h` drops to 24 because a mono DATA
line is `fb_cell_h()` = 32 device px at ui=2 = 16 design px, and 24dp gives 4dp
of air above and below — a set line, not a padded button. `title_h` drops to 28
for the same reason: a header is a line of type with a rule under it, not a bar.

**`theme.radius == 0` is safe.** Widgets that draw a nested inner rect at
`radius - 1` produce `-1`, which `fb_rrect` clamps to 0 (`fb.c:1988`). Verified,
not assumed. Nonetheless the phase-3 sweep changes those call sites to pass
`theme.radius` directly, because relying on a clamp to hide an arithmetic bug is
how the sentinel defect in §2.3 happens.

### 3.2 New metric tokens

```c
#define ZD_RAIL_W      148  /* design px. The rail. 296 device px at ui=2.      */
#define ZD_RAIL_W_MIN   56  /* the collapsed rail below ZD_RAIL_COLLAPSE_W      */
#define ZD_RAIL_COLLAPSE_W 1100 /* device px of screen width; below this the
                                   rail shows numbers only, no names (§6.2.6)   */
#define ZD_SLUG_H       28  /* design px. The printer's slug across the field.  */
#define ZD_GUTTER        8  /* design px between modules; margin is 2*gutter.   */
#define ZD_MOD_MIN      48  /* design px. A module narrower/shorter than this
                               is illegible; reduce the grid instead (§6.3).    */
#define ZD_TRACK_LABEL   2  /* design px of extra advance per glyph, LABEL      */
#define ZD_TRACK_PLATE   4  /* design px of extra advance per glyph, PLATE      */
#define ZD_MARK_W        3  /* design px. THE MARK: the accent bar that means
                               "this one". One width, one colour, two places.   */
#define ZD_CTL_W        20  /* design px. A window-control cell.                */
```

### 3.3 The spacing scale, unchanged

`UI_S1`=4, `UI_S2`=8, `UI_S3`=12, `UI_S4`=16, `UI_S6`=24, all through `UI_DP()`
and the q8 scale. Every number in this document is a **design** pixel and passes
through `UI_DP()` before it reaches `fb.c`, with the single stated exception of
the 1-device-pixel hairline (§2.4).

### 3.4 What replaces `neutral_fill()`

`uikit.c:168-171` draws `fb_rrect_blend(..., ZD_INK_LIGHT, alpha)` — white at
5-9% — in ~6 call sites. On paper this is invisible. It is replaced by two flat
helpers with no alpha at all:

```c
static void ui_hover_fill(int x,int y,int w,int h,int r) /* fb_rrect(..., ZD_WELL)  */
static void ui_sel_fill  (int x,int y,int w,int h,int r) /* fb_rrect(..., ZD_FILL)  */
```

and a **press** is not a fill at all — it is an inversion (§8.3). The tokens
`ZD_FILL_A`, `ZD_FILL_SOFT_A`, `ZD_FILL_ICON_A` are deleted, so a stale caller
fails to compile rather than drawing nothing.

---

## 4. THE TYPE SCALE

**There is no ratio, because there are only two sizes in ordinary use.**
Hierarchy is carried by CASE, WEIGHT, TRACKING and RULE WEIGHT. That is the
Swiss-print position and it is the direct opposite of the current eight-step
`ZD_T_*` ramp — 9 / 9.5 / 10 / 10.5 / 11 / 11.5 / 12 / 12.5 px, which is eight
sizes inside 3.5 pixels and therefore not a hierarchy at all. **All eight
`ZD_T_*` tokens are deleted.**

### 4.1 The atlas facts this is built on

Read out of the code, not assumed:

- `fb.c:3360` — `role_base[3] = { 8, 12, 16 }` design px.
- `fb.c:3362-3370` — `prop_cell(role)` = `(role_base[role] * ui_scale_q8 + 128)
  >> 8`, floored at 12.
- `fb.c:3372-3377` — `prop_atlas_cell(want)`: `<=20 → 16`, `<=28 → 24`,
  else `32`.
- `fb.c:3477` vs `fb.c:3484` — only `want == cell` takes the fast
  `blend_cov_s` branch; anything else goes through the bilinear resampler.
- `ui.h:372-374` — `UI_SM 0`, `UI_MD 1`, `UI_LG 2`, identical to `fb.c`'s
  `TEXT_CAPTION` / `TEXT_BODY` / `TEXT_TITLE`.
- `uikit.c:143` and `uikit.c:151` — **`UI_F_MONO` ignores the size argument** and
  routes to `fb_text_aa`, which draws `fb_cell_w() x fb_cell_h()` = 16x32 device
  px at ui=2. **Mono has exactly one size and it is the console cell.**

At ui=2 the three roles land on **16 / 24 / 32 device px**, which are exactly the
three generated atlases. **Every register in this spec hits the fast path and
nothing resamples.** That is the reason this type system is free.

### 4.2 Four registers, exhaustive

Anything on screen is one of these. There is no fifth.

```c
#define ZD_REG_LABEL  0   /* == UI_SM == TEXT_CAPTION, 8dp  -> 16 device @ui2 */
#define ZD_REG_BODY   1   /* == UI_MD == TEXT_BODY,   12dp  -> 24 device @ui2 */
#define ZD_REG_PLATE  2   /* == UI_LG == TEXT_TITLE,  16dp  -> 32 device @ui2 */
/* DATA has no role constant: it is the mono cell, fb_text_aa, UI_F_MONO. */
```

**1. DATA** — DejaVu Sans Mono via `fb_text_aa` / `UI_F_MONO`, **one size
forever** (the console cell: 16x32 device px at ui=2), ink `ZD_TEXT_1` on paper,
`ZD_KO_1` on plate. Every number, size, path, address, percentage, time,
filename, hex byte, keybinding, and every table cell that is not prose. One size
is not a limitation here, it is the rule: *a table where two numbers are
different sizes is a table you cannot scan.*

**2. LABEL** — proportional **BOLD** at `ZD_REG_LABEL`, **UPPERCASE**, tracked
`ZD_TRACK_LABEL` (2dp per glyph), ink `ZD_TEXT_5` on paper / `ZD_KO_2` on plate.
Column heads, section heads, button captions, window titles, rail entries, tab
captions, field captions, status words. **This is the workhorse and carries
roughly 60% of the screen's text.** It is what makes the system read as print
rather than as an app.

**3. BODY** — proportional regular at `ZD_REG_BODY`, ink `ZD_TEXT_1`. **PROSE
ONLY**: dialog copy, help text, descriptions. Deliberately rare. A technical
desktop is mostly not prose, and the current design uses body-weight
proportional text for things that are actually data.

**4. PLATE** — proportional **BOLD** at `ZD_REG_PLATE`, **UPPERCASE**, tracked
`ZD_TRACK_PLATE` (4dp per glyph), ink `ZD_TEXT_0` or `ZD_KO_0` knocked out.
**EXACTLY ONE PER VIEW.** The rail's `ZLOS`, or an app's name in a hero header.
Never twice on one sheet. Size is a scarce resource, spent once.

### 4.3 Tracking and caps — the one toolkit addition, and it is not an `fb.c` change

`uikit.c:733-734` already records the gap: *"Letter-spacing is not available in
fb.c's text path"*. Two new flags on the existing `ui_text` / `ui_text_w` flag
word:

```c
#define UI_F_TRACK (1 << 2)   /* add ZD_TRACK_LABEL per glyph (ZD_TRACK_PLATE
                                 when size == ZD_REG_PLATE) */
#define UI_F_CAPS  (1 << 3)   /* map a-z to A-Z on the way out */
```

**One helper, used by both the measure and the draw.** `uikit.c:131-134` states
the exact failure this guards against — *"consistently between the measure and
the draw"*, *"a measure and a draw that disagree is a clipped label"*. So:

```c
static int track_dp(const struct ui_theme *t, int size)
{
    return UI_DP(t, size == ZD_REG_PLATE ? ZD_TRACK_PLATE : ZD_TRACK_LABEL);
}
static char caps1(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }
```

- **draw**: for each byte, apply `caps1` if `UI_F_CAPS`, call `fb_text_role` on
  the single character, advance by `fb_text_role_w(one_char, size, weight) +
  track_dp(t, size)` if `UI_F_TRACK`.
- **measure**: `ui_text_w` returns the sum of the same per-glyph widths plus
  `(n - 1) * track_dp(t, size)`. **Not a separate expression — the same
  function.** If you find yourself writing the arithmetic twice, stop.

**The uppercasing is ASCII-only by construction.** Gate it on
`c >= 'a' && c <= 'z'` and leave every other byte alone — a blanket `-32` mangles
any non-ASCII byte in the string.

`UI_F_TRACK` and `UI_F_CAPS` are appended to the flag word; `UI_F_MONO (1<<0)`
and `UI_F_BOLD (1<<1)` do not move.

### 4.4 Where mono's one size is load-bearing, and where it is not

**Load-bearing (good):** the DATA register *is* the console cell, so tables,
readouts, the slug and the terminal all sit on one grid with tabular figures for
free. This is the constraint being built on rather than fought.

**Where it forces a decision:** a sub-line under a title cannot be mono, because
mono is 16 design px tall — as tall as the PLATE title it sits under. So:

- The rail's build string under `ZLOS` is **LABEL**, not mono.
- A window's subtitle beside the title is **mono** (they are on one baseline,
  not stacked) — see §5.2.
- Any two-level stack where the lower level must be smaller uses **LABEL** for
  the lower level.

*This is the constraint that killed Proposal 4. It is written down so that no
implementation agent rediscovers it at 2 a.m.*

---

## 5. WINDOW CHROME

A window is a **SHEET**: `ZD_PAPER` interior, a STRUCTURE frame on all four
sides, radius 0, **no shadow, no glow, no blur**. The gutter between sheets is
`ZD_GUTTER` of `ZD_DESK`; that gutter plus the frame is the entire separation
mechanism.

### 5.1 The frame

- `fb_rrect(x, y, w, h, 0, ZD_RULE_2)` — the frame, drawn as a filled rect.
- `fb_rrect(x+f, y+f, w-2f, h-2f, 0, ZD_PAPER)` — the interior, where
  `f = UI_DP(t, 2)`.
- Nothing else. No outer ring, no inner ring, no bevel, no highlight edge.

**A two-tone border is a bug in this design.** If a review sees a light edge and
a dark edge on the same object, the direction has been broken.

### 5.2 The header — 28dp, and it is two completely different objects

`theme.title_h = UI_DP(t, 28)`. Let `f = UI_DP(t, 2)` (frame), `p = UI_DP(t, 12)`
(pad), `g = UI_DP(t, 8)` (gap), `c = UI_DP(t, ZD_CTL_W)` (control cell).

**FOCUSED** — ground `ZD_PLATE_0`, filling the full inner width, from
`(x+f, y+f)` to `(x+w-f, y+f+title_h)`:

| element | x | ink | register |
|---|---|---|---|
| title | `x+f+p`, vertically centred | `ZD_KO_0` (16.27:1) | LABEL + CAPS + TRACK |
| subtitle | title_right + `g`, same baseline | `ZD_KO_1` (14.18:1) | DATA (mono) |
| coordinate readout | right-aligned at `x+w-f-3c-p` | `ZD_KO_2` (6.31:1) | DATA (mono) |
| control cells | the last `3c` before `x+w-f` | `ZD_KO_0` | geometry, §5.4 |

**UNFOCUSED** — ground `ZD_PAPER` (the header is *not a separate surface*; it is
the top 28dp of the sheet):

| element | ink | register |
|---|---|---|
| title | `ZD_TEXT_4` (6.25:1) | LABEL + CAPS + TRACK |
| subtitle | `ZD_TEXT_6` (5.53:1) | DATA (mono) |
| coordinate readout | `ZD_TEXT_6` | DATA (mono) |
| control glyphs | `ZD_TEXT_3` (8.30:1) | geometry, §5.4 |
| **plus** a STRUCTURE rule (`UI_DP(t,2)` of `ZD_RULE_2`) under the header | | |

The focused header has **no** rule under it — the plate's own bottom edge is the
boundary, at 16.27:1.

### 5.3 Focus, stated exactly

Focus is signalled by **two** things and nothing else:

1. **INVERSION.** The header flips between `ZD_PLATE_0`-ground-with-knockout and
   `ZD_PAPER`-ground-with-ink. That is a **12.95:1** value flip.
2. **THE MARK.** A focused window carries a `UI_DP(t, ZD_MARK_W)` (3dp) bar of
   `ZD_ACCENT` down its entire **left inner edge**, from the inside of the top
   frame to the inside of the bottom frame — header included, full height.

**It is the same mark, the same width and the same colour as the run mark on a
rail row (§6.2.3): one mark, one meaning, two places.** This replaces
`ZD_RING_FOCUS`, `ZD_RING_FOCUS_A`, `ZD_RING_BLUR_A` and `ZD_RING_OFF_A`, all
four deleted.

There is **no** focus ring, **no** glow, **no** coloured title bar, **no**
shadow change, **no** pulse.

`wm.c`'s `shell_state_key` already carries `focused`, so the cached shell
surface re-keys correctly with no change to the caching layer.

### 5.4 The controls — three glyph cells, pure geometry, radius 0

Not 26x26 rounded squares. Three cells of `ZD_CTL_W` (20dp) x `title_h`, flush
to the header's right, separated by 1-device-pixel verticals
(`ZD_RULE_KO` on the focused plate, `ZD_RULE_1` on the unfocused sheet). Each
glyph is drawn as geometry, centred in its cell, with `s = UI_DP(t, 2)` stroke:

| control | geometry (design px, centred in the cell) |
|---|---|
| **minimise** | a `12 x 2` bar, its bottom edge on the cell's vertical centre + 5 |
| **maximise** | a `12 x 12` square outline, stroke 2, four `fb_fill_px` calls |
| **close** | a `12 x 12` X — two 45-degree strokes, `fb_line` (already Wu-AA, §0.3), stroke 2 via three parallel passes offset by one device px |

States:

| state | focused header (plate) | unfocused header (sheet) |
|---|---|---|
| rest | glyph `ZD_KO_0` | glyph `ZD_TEXT_3` |
| hover (min/max) | cell fills `ZD_PAPER`, glyph `ZD_TEXT_0` | cell fills `ZD_PLATE_0`, glyph `ZD_KO_0` |
| hover (close) | cell fills `ZD_ACCENT`, glyph `ZD_PAPER` (4.74:1) | same |
| press | cell fills `ZD_PLATE_1`, glyph `ZD_KO_0` | same |

**Hover is an inversion of the individual cell, not a tint.** No hover alpha, no
rounded plate behind the glyph.

### 5.5 The coordinate readout — the chrome states its own state

Every window header carries, at its right before the controls, a mono readout of
its own grid placement:

```
[3,1 6x4]
```

column, row, colspan, rowspan. A detached window (§6.5) reads `[DETACHED]`
instead. This is the aircraft-checklist move: the chrome **tells** you the state
rather than implying it, and it makes the tiling grid legible without any hover
affordance. It is the single most identity-carrying detail in the window chrome
and it is not optional.

### 5.6 Menus, modals, toasts

All are sheets: `ZD_PAPER` ground, STRUCTURE frame, radius 0, **no shadow**.
Separation from the window beneath is the frame at 16.27:1, which is more
separation than any shadow gives. A modal darkens nothing — the sheet behind it
simply loses focus and inverts its header back to paper, which is the same
"not focused" language used everywhere else.

`ZD_TOAST_MS` 4200 and `ZD_TOAST_MAX` 3 survive unchanged: they are dwell and
count, not motion.

---

## 6. THE DESKTOP SHELL

Three regions, separated by STRUCTURE rules. Nothing floats and nothing overlaps
by default. **The top bar, the top island, the bottom dock, the seven dock tiles,
the start menu, the wallpaper vignette, the two conic wedges, the three coloured
glows, the two-scale technical grid and the 7-second sweep are all deleted.**

```
+-----------+--------------------------------------------------------------+
|           | zlOS 0.3 · 1920x1200 · 4 CPU · LOAD 0.31 · 14:32:07      (+) | slug 28dp
|  ZLOS     +==============================================================+
|  0.3 RAW  | ┌ ─ ─ ─ ┬ ─ ─ ─ ┬ ─ ─ ─ ┐   the module grid, ruled           |
|  =======  |                                                              |
| |01 SHELL |    ▐███████████████████▌      windows tile onto it           |
| |02 FILES |    ▐  a sheet         ▌                                      |
|  03 MON   |    ▐███████████████████▌                                     |
|  04 EDIT  |                                                              |
|    ...    | └ ─ ─ ─ ┴ ─ ─ ─ ┴ ─ ─ ─ ┘                                    |
|  -------  |  MODULE 12 x 8 · 116/115 x 121 px · GUTTER 16                |
|  CLOCK    |                                                              |
|  MEM      |                                                              |
+-----------+--------------------------------------------------------------+
   148dp                            the field
```

### 6.1 The wallpaper — four layers, in this order

The desk is not decoration. It is the layout grid, drawn.

1. `fb_fill_px(0, 0, W, H, ZD_DESK)` — flat. **No gradient, no vignette, no
   glow, no wedge.** Alpha 255.
2. **The printed grid.** One `fb_fill_px` of 1 device px `ZD_GRID_LINE` at every
   module boundary inside the field: 13 verticals (both edges of every column)
   and 9 horizontals. Alpha 255 — the colour is already quiet at 2.02:1.
3. **The crop marks.** At each of the four corners of the content box, two
   `fb_fill_px` calls forming an L: `UI_DP(t,16)` long, `UI_DP(t,2)` thick,
   `ZD_ACCENT`, offset outward from the content box by `ZD_GUTTER`.
4. **The legend.** Bottom-left of the field, inside the margin, DATA (mono),
   `ZD_TEXT_4`: the grid **as measured at the current resolution** —
   `MODULE 12 x 8 · 116/115 x 121 px · GUTTER 16`. If the grid was reduced
   (§6.3), the legend states the reduced grid, because a legend that lies is
   worse than no legend.

All four layers are static for a given resolution, so all four bake into the
existing `fb_wall_save` / `fb_wall_paint` cache and cost a blit per frame.

### 6.2 The rail — this one element replaces the top bar, the dock and the start menu

Left edge, flush, full height, `ZD_RAIL_W` (148dp = 296 device at ui=2), ground
`ZD_PLATE_0`, radius 0 on all four corners. **It needs no edge**: the plate is
`ZD_RULE_2`'s value and separates from `ZD_DESK` at 14.18:1 on its own.

Layout top to bottom, all coordinates design px from the rail's origin, text
column inset `UI_DP(t,16)` on both sides:

**6.2.1 IDENTITY — y 0..56.**
`ZLOS` in **PLATE** register (bold, caps, tracked 4dp), `ZD_KO_0`, at
`(16, 12)`. Build string `0.3 · RAW` in **LABEL** register, `ZD_KO_2`, at
`(16, 34)`. *(LABEL, not mono — §4.4.)*

**6.2.2 y 56..58 — a `UI_DP(t,2)` rule of `ZD_ACCENT_KO`** spanning the text
column. This is the only accent rule in the system, and it is what makes the
identity block read as a masthead.

**6.2.3 THE REGISTER — y 58 to (H − readout_h).**
Every app as a **numbered row**, `theme.row_h` (24dp) tall, separated by a
1-device-pixel `ZD_RULE_KO`:

| element | x | width | ink (idle) | ink (focused row) |
|---|---|---|---|---|
| run mark | 0 | `ZD_MARK_W` (3dp) | `ZD_ACCENT_KO` if running, else nothing | `ZD_ACCENT` |
| `NN` | 16 | 2 mono cells (16dp) | `ZD_KO_1` | `ZD_TEXT_0` |
| name | 40 | rest | `ZD_KO_2` | `ZD_TEXT_0` |

Registers are LABEL (bold caps tracked). `NN` is DATA (mono) so the digits are
tabular and the column cannot jitter.

**The focused app's row is inverted back to `ZD_PAPER` ground with ink text** —
the same inversion as a window header, at the same 16.27:1, so the rail and the
window agree about which one is focused without any second signal.

**Two digits means `Alt+NN` launches or raises it, so the launcher, the tasklist
and the keymap are one list.** This is the macrostructural invention and it is
not negotiable.

*Capacity, stated because 53 apps do not fit.* At 1200 device px / ui=2 the rail
is 600 design px tall; minus 58 identity and 112 readout leaves 430, i.e. **17
rows**. The register lists **running windows first, then pinned apps**, and
overflows by scrolling (`ZD_MS_SLIDE`, §8.2). `Alt+NN` addresses the first 99
regardless of what is scrolled into view. The full 53-app catalogue is the one
modal in the system (§6.6).

**6.2.4 THE READOUT — pinned to the bottom, 112dp tall.**
A 1-device-pixel `ZD_RULE_KO` above it, then 8dp of pad, then four rows of 24dp:
`CLOCK`, `MEM`, `LOAD`, `NET`. Label in LABEL caps, `ZD_KO_2`, flush left at
x=16. Value in DATA (mono), `ZD_KO_0`, **right-aligned to a shared right edge at
`ZD_RAIL_W - 16`**, so all four values share a decimal column. 8dp of bottom pad.

**6.2.5** State in the readout uses `ZD_OK_KO` / `ZD_WARN_KO` / `ZD_BAD_KO`
(8.31 / 9.55 / 8.36:1) and always carries the word as well as the colour.

**6.2.6 THE COLLAPSE.** Below `ZD_RAIL_COLLAPSE_W` (1100 device px of screen
width) the rail collapses to `ZD_RAIL_W_MIN` (56dp): the register shows `NN`
only, the identity block shows `ZL` in PLATE, and the readout collapses to the
clock alone. *This breakpoint is defined here because no proposal defined one
and a 148dp rail is 23% of a 1366-wide panel.*

### 6.3 The grid — integer arithmetic only, and it degrades

```
cols   = 12, rows = 8                    (nominal)
gutter = UI_DP(t, ZD_GUTTER)
margin = 2 * gutter
field_x = rail_w                 field_w = screen_w - rail_w
field_y = slug_h                 field_h = screen_h - slug_h
content_w = field_w - 2*margin   content_h = field_h - 2*margin
col_w = (content_w - (cols-1)*gutter) / cols        remainder R_w
row_h = (content_h - (rows-1)*gutter) / rows        remainder R_h
```

**The remainder is distributed one pixel each to the first R columns / rows.**
Exact, deterministic, no float, no drift.

Verified by arithmetic at **1920x1200, ui=2**: `field_x` 296, `field_w` 1624,
margin 32, `content_w` 1560; `(1560 - 11*16)/12 = 1384/12 = 115 r 4`, so columns
1-4 are 116px and 5-12 are 115px. Vertically: `content_h` 1080;
`(1080 - 7*16)/8 = 968/8 = 121` exactly. **This is where the legend's
`116/115 x 121 px` comes from.**

Verified at **1366x768, ui=1**: `field_w` 1218, margin 16, `content_w` 1186,
`(1186 - 88)/12 = 1098/12 = 91 r 6`. Vertically `content_h` 708,
`(708 - 56)/8 = 652/8 = 81 r 4`. Both legible.

**DEGRADATION, mandatory.** While `col_w < UI_DP(t, ZD_MOD_MIN)` and `cols > 4`,
decrement `cols` and recompute. Same for `rows` against a floor of 3. Clamp
`content_w`/`content_h` to `> 0` before the division — **a column that comes out
0 or negative at a small resolution is a divide-by-zero away from a fault.** The
legend prints the reduced grid.

### 6.4 THE GRID IS FUNCTIONAL — grafted from Folio

While a window is being dragged, the modules it will land on **fill** with
`ZD_FILL` at 60% alpha (`fb_fill_blend`), fading in over `ZD_MS_SLIDE`
(§8.2). On drop the window snaps to that module rect.

This is what turns the wallpaper from decoration into the layout system made
visible. `wm.c` already computes a drag preview through one function so that
*"the preview cannot promise a landing spot the snap then disagrees with"*
(`wm.c:162-165`) — the module highlight uses that same function and no other.

### 6.5 Windows tile

A window's geometry is stored as `(col, row, colspan, rowspan)` in module units
and resolved to device pixels by **one** function, `grid_rect()`. A window
occupies a whole number of modules and snaps on open.

`snap_rect()` (`wm.c:158-167`) currently takes `reserve_top` and `reserve_bot`
and **has no left reserve** — it gains `reserve_left`, which is the rail. This
is the one genuinely new piece of `wm.c` arithmetic and it has a foothold
already.

**Overlap exists, but as an explicit state.** A window dragged with the existing
no-snap modifier becomes **DETACHED**: it keeps its STRUCTURE frame, it is
excluded from tiling, and its coordinate readout reads `[DETACHED]` on a
`ZD_ACCENT` block with `ZD_PAPER` ink. There is no free-floating default, so
there is no z-order to reason about in the common case and no shadow needed to
explain one.

### 6.6 The slug — what used to be the top bar

The top `ZD_SLUG_H` (28dp) of the field, ground `ZD_DESK`, terminated below by a
STRUCTURE rule of `ZD_RULE_2`. One DATA (mono) line, `ZD_TEXT_4`, left at
`field_x + margin`, middot-separated, exactly as a printer's slug:

```
zlOS 0.3 · 1920x1200 · 4 CPU · LOAD 0.31 · 14:32:07
```

No menus, no tray icons, no click targets — **except one**. Right-terminated at
`field_x + field_w - margin` by the **register mark**: a 12dp cross-in-circle in
`ZD_ACCENT`, drawn as geometry (a full circle plus two axis-aligned strokes). It
opens **the one modal in the system**: the full catalogue of ~53 apps, a
`ZD_PAPER` sheet inset by the margin with a STRUCTURE frame, radius 0, all apps
in four columns separated by 1-device-pixel `ZD_RULE_1`, rows 24dp, each row
`NN` in DATA plus name in LABEL. Type filters, Esc closes. **A book index, not a
tile grid** — which is why the "three or seven equal tiles" failure mode cannot
recur.

### 6.7 What moved where, explicitly

| deleted from `kernel.zl` | where it went |
|---|---|
| the top island (clock, status chips, separators), `island_fill()` | the rail's readout (§6.2.4) |
| the dock (bar, 7 tiles, tray text, running dots), `dock_fill()` | the rail's register (§6.2.3) |
| the start menu | the slug's register mark → the one modal (§6.6) |
| `draw_wallpaper()`'s nine layers, `wall_glow_a/b/c()` | four layers (§6.1) |
| `paint_col()`'s seven `rgb()` literals, the snake and starfield literals | `ui_color()` roles |

**Every `rgb()` colour literal in `kernel.zl` disappears** — not because they
were recoloured, but because their call sites were deleted or routed through
`theme()`. The token-architecture violation the brief names is fixed by
subtraction.

---

## 7. THE ICON LANGUAGE

`gen_icons.py` keeps its exact pipeline: geometry drawn at a 4x-supersampled
master, box-filtered, emitted at **both** 24 and 48 native. **No resolution work
is required (§0.3).** What changes is the drawing grammar — a review pass over
the coordinate lists that turns 89 loose drawings into one family.

### 7.1 Four constraints

**1. THE 12-UNIT MODULE.** The 96x96 field is 8x8 modules of 12 units. **Every
vertex lands on a module boundary or a module midpoint — i.e. on a multiple of
6.** This is the icon-scale echo of the desktop's own 12x8 grid, and it is
checkable: a generator assertion that every emitted coordinate is a multiple of
6 is one line and it is the gate (§10.2).

**2. ONE STROKE WEIGHT: 8 units at 96**, which is 2px at 24 and 4px at 48. No
thin/thick contrast, no tapering, no optical correction. Solid shapes are
permitted where the shape **is** a block (a disk drive is a filled rectangle);
**outlines and fills never mix inside one icon.**

**3. FOUR CONSTRUCTIONS ONLY:** the axis-aligned rectangle, the 45-degree
diagonal, the full circle, and the quarter-round. No other angle, no bezier, no
free curve. **This single rule is what makes 89 icons drawn over months read as
one set.**

**4. BLEED TO THE FIELD.** Icons are drawn edge to edge in the 96x96 field with
no built-in optical padding, so a row of them is flush and the surrounding
layout supplies the spacing. Padding inside the asset is what makes an icon row
look ragged at two sizes.

Terminals are **square-cut**, joins are **mitred**. No rounded caps — a rounded
cap is a fifth construction wearing a disguise.

### 7.2 Colour

Monochrome, and the colour is the ink of the context — `ZD_TEXT_3` on a sheet,
`ZD_KO_1` knocked out on a plate, `ZD_TEXT_4` when inactive. `fb_icon24`'s `fg`
argument already works this way. **Colour appears on an icon for exactly one
reason**: an alert or active overlay, drawn as a solid 24x24-at-96 `ZD_ACCENT`
square occupying the lower-right module. One shape, one place, one meaning.

Banned: any icon that is a letterform; any icon whose meaning depends on colour;
any two-tone icon; any pictorial detail below 2px at 24.

### 7.3 Icon load is deliberately reduced

**In the rail the identifier is the two-digit index plus the tracked-caps name;
there is no icon there at all.** Icons appear at 24dp only in window headers and
the file grid. Fifty-three apps do not need fifty-three memorable pictograms if
they have fifty-three numbers, and pretending otherwise is how icon sets become
mush.

*This is also the designed hedge for the four-construction grammar: when an app
wants a shape the grammar cannot draw, the register still identifies it. **That
hedge is only real if the register is built before the icons are redrawn** — see
the phase order in §11.*

### 7.4 The list `gen_icons.py` must emit — 89, unchanged in count and order

**Order is load-bearing: `icons.c` indices are wired into `kernel.zl` and
`apps_registry.zl`. Do not reorder. Do not insert. Redraw in place.**

*Tier 1 — the app set (10), redraw first:*
`terminal` `snake` `paint` `editor` `monitor` `about` `cube3d` `files`
`settings` `power`

*Tier 2 — the control set (34), redraw second:*
`search` `lock` `drive` `close` `check` `chevron` `clock` `network` `volume`
`grid` `browser` `home` `minimize` `maximize` `restore` `menu` `refresh` `plus`
`folder_add` `trash` `download` `upload` `copy` `paste` `warning` `bell` `back`
`forward` `external` `user` `calendar` `camera` `play` `pause`

*Tier 3 — the reference apps (45), redraw last:*
`log` `render` `hex` `snap` `tty` `calcApp` `font` `disk` `svc` `binary` `imgv`
`chip` `uClip` `uColor` `uRegex` `uBase` `uDiff` `uHash` `uUnit` `uNote`
`uKeys` `uBench` `gTetris` `gPong` `gBrick` `gMine` `gNum` `gLife` `gRock`
`gAlien` `gCross` `gTiles` `gDrop` `gDisc` `gBulb` `gWave` `gMaze` `gCrate`
`gBird` `gTarget` `gTower` `gSticks` `gCard` `gFrog` `gWord`

The window controls (`minimize`, `maximize`, `restore`, `close`) stay in the
atlas for app use, but **the window chrome does not draw them from the atlas** —
it draws pure geometry (§5.4), because a 12x12 bar and a 12x12 X are cheaper and
sharper as three `fb_fill_px` calls than as a resampled coverage blit.

---

## 8. MOTION

**Mechanical, not organic. Nothing scales, nothing fades, nothing bounces,
nothing pulses.** A sheet is *placed*; it does not inflate into existence.

### 8.1 CUT — 0 ms

The default. Window open, window close, menu open and close, tab switch, modal
open, sidebar toggle, and every state change that is not a position change.

Deleted: `ZD_MS_WIN`, `ZD_MS_POP`, `ZD_MS_POP_FAST`, `ZD_MS_POP_SLOW`,
`ZD_MS_OV`, `ZD_MS_TOAST`, and with them `ZD_WIN_FROM_SCALE`, `ZD_WIN_FROM_DY`,
`ZD_POP_FROM_SCALE`, `ZD_POP_FROM_DY`, `ZD_OV_FROM_SCALE`, `ZD_TOAST_FROM_DY`.

### 8.2 SLIDE — 100 ms, LINEAR

```c
#define ZD_MS_SLIDE 100
```

**The only animation in the system.** It applies only to things that travel
along the grid: a window re-tiling to a new module, a window resizing by a whole
number of modules, the register scrolling, and the drag-target module highlight
fading in and out (§6.4).

**Linear, never eased** — the motion should read as a plotter head under stepper
control rather than as something with mass. Distance is always a whole number of
modules plus gutters, so intermediate frames land on integer pixel positions
with no rounding drift.

`ZD_EASE_WIN_X1/Y1/X2/Y2` are **deleted**. Those four values existed only
because the kernel has no floating point and a bezier had to be stored scaled by
1000; linear needs no curve, which removes the last float-shaped thing from the
motion path. `ease.c` / `ease.h` stay in the tree and keep `hosttest/easetest.c`
— see §10.3.

### 8.3 STEP — instantaneous, held

A pressed control **inverts**: a paper button becomes a `ZD_PLATE_0` plate with
a knocked-out caption for as long as the button is down, then inverts back on
release. **No 0.88 scale dip.**

Deleted: `ZD_MS_PRESS`, `ZD_PRESS_MIN_SCALE`, `ZD_PRESS_MID_PCT`.

### 8.4 BLINK — 500 ms period, 50% duty, hard on/off

```c
#define ZD_MS_BLINK 500
```

The text caret, and the single busy/recording mark. No opacity ramp.

Deleted: `ZD_MS_PULSE`, `ZD_MS_PULSE_SLOW`, `ZD_PULSE_FLOOR`. An infinitely
pulsing element means the damage list is never empty on a compositor whose whole
architecture is damage-driven repaint; those two tokens are a standing tax on the
one thing this kernel does well.

`ZD_MS_SWEEP` is deleted with the wallpaper it swept.

### 8.5 Summary

| event | duration | curve |
|---|---|---|
| window open | **0 ms** | cut |
| window close | **0 ms** | cut |
| menu pop | **0 ms** | cut |
| modal open | **0 ms** | cut |
| tab switch | **0 ms** | cut |
| **press** | **0 ms**, held inverted until release | step |
| **focus change** | **0 ms** | cut — *a focus change that animates is a focus change you have to wait to believe* |
| **window re-tile / resize by modules** | **100 ms** | **linear** |
| register scroll | 100 ms | linear |
| drag-target module highlight | 100 ms | linear |
| caret / busy mark | 500 ms period, 50% duty | hard |
| toast dwell | 4200 ms (`ZD_TOAST_MS`, unchanged) | — |

**Two motion tokens survive, from fifteen.**

---

## 9. WHAT IS EXPLICITLY PRESERVED — file by file

Any change that discards one of these is a regression, not a redesign.

**`kernel/font_sub.c`, `kernel/fb.c:1394-1417` (`draw_glyph`'s subpixel
branch), `blend_sub`** — subpixel LCD text rendering, three alphas per pixel.
**Preserved byte-for-byte, and its reach is NOT extended (§0.4).** Note for the
record: `gen_subfont.py:39-43` explicitly rejects the (1,2,3,2,1)/9 FIR as
reading as blur and ships FreeType's default `(0x08,0x4D,0x56,0x4D,0x08)/256`,
which is symmetric and sums to 256 — so the filter introduces **no net chroma
bias** and a light substrate flips fringe polarity without adding a cast. Any
risk item claiming "the light ground may show a warm/cool cast from the FIR" is
naming the wrong mechanism.

**`kernel/fb.c:195-273`** — gamma-correct blending in **linear light**, the two
boot-built LUTs, no floating point. Preserved and **load-bearing**: 1px black
rules on warm paper and 16:1 text edges are precisely where a non-linear blend
shows as a coloured fringe. This direction stresses the LUTs *harder* than the
dark palette did, and in a better part of the ramp — `srgb_to_lin[0..7]` all
collapse to linear 0, so the dark end is degenerate and the light end is not.

**`kernel/font_aa.c`** (DejaVu Sans Mono, FreeType-hinted) and
**`kernel/gen_prop_font.py`** (DejaVu Sans at 16/24/32, regular and bold, with
per-glyph advances) — both faces kept, both used more deliberately: **mono owns
data, proportional bold owns labels.**

**`kernel/fb.c:1984-2031` (`fb_rrect`)** — anti-aliased rounded corners, 4x4
supersampled per-corner coverage. **Kept and still called**: its radius-0 path
draws every frame in the system, and its corner supersampling is what makes
`ZD_R_2` a defensible threshold rather than an arbitrary number (§2.1).

**`kernel/fb.c:2453` (`fb_line`)** — Wu's algorithm, 16.16 accumulator. Already
anti-aliased; the sparkline diagonals and the close glyph both depend on it.

**`kernel/fb.c:3635-3656` (`fb_icon24`) and `kernel/icons.c`** — the native
24 and 48 atlases and the per-scale selection. **Must not be reintroduced as a
nearest-neighbour upscale.**

**`kernel/gen_icons.py`** — icons as geometry drawn at a supersampled master and
box-filtered, with `render()` building a separate master per output size. Kept,
and given the module/stroke/construction grammar of §7 rather than replaced.

**`kernel/ui.h` `UI_DP()` and the q8 UI scale** — the metric mechanism, kept
exactly. Four values are retuned (`row_h` 28→24, `radius` 16→0, `title_h` 36→28;
`pad` 12 and `gap` 8 unchanged) and ten are added (§3.2).

**`kernel/ui.h`'s 29-role `UI_COLOR` enum and `struct ui_theme`'s field order.**
**This is the highest-probability way to break the port and it must be read
before touching `ui.h`.** `ui_color()` indexes `&theme.bg` by an integer role
(`ui.c:154-159`), and `kernel.zl` passes role **numbers** (`TH_BG = 0` …
`TH_ACCENT_BR = 28`, `kernel.zl:33-64`), not names. Inserting a role anywhere but
the end repaints the whole desktop with off-by-one colours **and nothing fails to
compile.** `ui.h:60` says so itself: *"These are appended, so no existing
ui_color() index moves."*

> **RULE: new roles are APPENDED before `UI_COLOR_COUNT`, and `struct ui_theme`'s
> field order is changed in lockstep. `title_off_bot` stays after the
> zl-visible contiguous array. No exceptions.**

**`kernel/ui.c`'s immediate-mode layout cursor and the hit-test-by-redraw
contract** — no widget takes an action as an argument. Nothing in this design
needs retained state.

**`kernel/ui.c:353-365` (`ui_luminance_q16`, `ui_ink_on`)** — verified correct
against the new palette (§1.7). **Do not touch.**

**`kernel/wm.c`'s window manager** — z-order, drag, resize, tabs, and its
per-window `fb_surface` cache. `shell_state_key` already carries `focused`, so
the inversion-based focus signal re-keys correctly. Tiling reduces how often the
drag path runs; it does not change what it does.

**The three-layer token architecture** — `design.h` holds every literal, `ui.c`
maps primitives to semantic roles, `kernel.zl` names only roles. Not merely
preserved but **tightened**: `dock_fill()` and `island_fill()` disappear with
their call sites, and every surviving literal routes through `ui_color()`.

**`kernel/fb.c:459-512` (`fb_gradient`'s ordered 4x4 dither),
`fb.c:574` (`fb_shadow`), `fb_glow`, `fb_blur_paint`, `fb_grad_conic`,
`fb_grad_radial`, `fb_rrect_grad_top`, `kernel/ease.c`** — **code preserved
untouched. The desktop shell calls none of them.** Apps and the browser may
still call them. **This is the single largest rot risk in the spec and §10.3 is
its mitigation, not a comment.**

---

## 10. GATES — what must be mechanically checked, not asserted

### 10.1 The provenance rewrite, same commit

`design.h`'s header comment currently asserts that every constant was MEASURED
out of `docs/design/ds-reference.html` with a re-runnable command. **That
provenance claim becomes false the moment this lands.** Rewrite it in the same
commit to state: which values are measured (none of the new ones), which are
derived from the rasteriser (`ZD_R_2`), which are computed contrast decisions
(`ZD_TEXT_5`, `ZD_ACCENT_KO`, `ZD_KO_2`), and which are chosen. *A file whose
stated method no longer matches its contents is worse than no comment, and this
repo has been bitten by exactly that.*

### 10.2 New assertions

Add to `hosttest/palette.c` and `hosttest/uitest.c`:

1. **Contrast.** Every (ink, surface) pair in §1.3 / §1.4 / §1.6 recomputed at
   test time and asserted against the value in this document. The gate carries
   the check; a comment claiming it was done does not.
2. **`ZD_FILL` carries no text.** Fail if any `fb_text_*` call resolves a
   background of `ZD_FILL`.
3. **`ZD_TEXT_6` is never drawn on `ZD_WELL`** (4.13:1).
4. **`ZD_ACCENT` is never drawn on `ZD_PLATE_0`/`ZD_PLATE_1`** (3.43 / 2.73:1) —
   `ZD_ACCENT_KO` only, and never carrying text.
5. **Radius.** Fail on any radius reaching `fb_rrect` that is not 0 or
   `UI_DP(t, 2)`. Fail on any **negative** radius (§2.3).
6. **Hairlines.** Fail on any 1px rule drawn at a `UI_DP()`-scaled thickness
   (§2.4).
7. **Measure/draw agreement.** For every tracked, capitalised string, assert
   `ui_text_w(s, size, flags)` equals the sum of the advances the draw loop
   actually used. This is the clipped-label class `uikit.c:131-134` warns about.
8. **Icon grid.** In `gen_icons.py`, assert every emitted coordinate is a
   multiple of 6 (§7.1). One line, and it is the gate for the whole grammar.
9. **The `kernel.zl` literal gate stays and gets stricter.** `hosttest/palette.c`
   already asserts `kernel.zl` holds no second copy of a `design.h` token; the
   `ZD_WALL_*` carve-out that made it toothless is deleted along with those
   tokens, closing the recorded hole.
10. **Grid arithmetic.** Assert `col_w > 0` and `row_h > 0` at 1024x768,
    1366x768, 1920x1200 and 2560x1440, at ui 1 and 2, and that the legend string
    matches the computed values.

### 10.3 The dead-primitive gates — write these BEFORE changing the wallpaper

`fb_gradient`'s ordered 4x4 dither loses its only desktop caller. **That is the
single thing in this spec most likely to rot into a broken primitive nobody
notices.** Same class, lower stakes: `fb_shadow`, `fb_glow`, `fb_blur_paint`,
`fb_grad_conic`, `fb_grad_radial`, `fb_rrect_grad_top`, `ease.c`.

**Requirement, in order:** `hosttest/walltest.c` (which exists) renders a
full-height ramp through `fb_gradient` and asserts no banding — **and this test
lands before `draw_wallpaper()` is touched.** Each of the other primitives gets
a hosttest that calls it and asserts a non-trivial property. *A capability with
no caller and no test is preserved in name only.*

### 10.4 Damage-rectangle hazard — read before deleting shadows

`wm.c:2025` sizes the retained shell surface from `shadow_reach(win)`, and
`wm.c:2231-2241` documents `fb_shadow`'s reach as the source of a drag-smear
bug class. **Deleting the shadow shrinks that reach to zero in at least two
places that compute it independently.** Find both before changing either.
*(This hazard was found by Raking, not by PLATE, and is carried across.)*

### 10.5 Look at it

`hosttest/wmshot` renders a frame in milliseconds. **Render at 1920x1200 and
2560x1440 and LOOK at the result before accepting any other number in this
spec.** The two things that cannot be settled by reading code:

- Whether the gamma LUTs, built and tuned against `ZD_SURF_0`-class near-black,
  behave on dark-ink-on-bright-paper — the opposite half of the ramp, where
  over-correction shows as text that looks too **thin** rather than too fat.
- Whether a 1px `ZD_GRID_LINE` at 2.02:1 survives the wallpaper cache's RGB565
  quantisation with its ordered ±8 dither (`fb.c:2884-2905`). **No proposal
  raised this and it is the most likely place the desk's identity silently
  disappears.** Check it in the first render, not the tenth.

### 10.6 Stated non-goals

- **No dark mode in v1.** A light desktop on an OLED panel at 2 a.m. is a real
  ergonomic cost and this spec refuses to hedge on it. The mitigation is cheap
  *only because* PLATE never uses value steps for depth: every colour is
  role-mapped in `ui_theme_init_q8`, so an inverted variant is one alternate
  assignment block in `ui.c` — swap the paper ladder for an ink ladder, keep
  every rule weight, every radius, every metric and the entire structure. It is
  not a second design language; it is the same one on black stock. **Build the
  light one first. Do not build both at once.**
- **No subpixel extension** (§0.4).
- **No icon atlas regeneration for resolution** (§0.3).
- **No `fb.c` primitive additions of any kind.** If a widget seems to need one,
  the widget is wrong.

---

## 11. THE FOUR WORK PACKAGES

Sized so four agents can work in parallel without a flag day. Phase 1 alone
changes every window in `hosttest/wmshot` with `kernel.zl` untouched — **that is
the increment on which the direction is judged.** *(Discipline grafted from
Raking.)*

**PHASE 1 — `design.h` + `ui.c`. One agent. Nothing else may start until this
lands.**
The full palette (§1) including the compatibility block (§1.9), the radius
resolution table (§2.2), the metrics (§3.1, §3.2), the register constants
(§4.2), the motion tokens (§8), the header rewrite (§10.1), and the theme role
mapping in `ui_theme_init_q8()`. New `UI_COLOR` roles **appended only** (§9).
Gate: `hosttest/wmshot` renders; §10.2 items 1, 5, 9 pass; §10.5 has been looked
at.

**PHASE 2A — `wm.c` chrome.** The frame (§5.1), the 28dp two-state header
(§5.2), focus by inversion plus the mark (§5.3), the control cells (§5.4), the
coordinate readout (§5.5), shadow deletion with the damage-rectangle audit
(§10.4). Gate: `hosttest/wmtest`, `hosttest/wmshot`, §10.4 resolved.

**PHASE 2B — `uikit.c`.** `UI_F_TRACK` / `UI_F_CAPS` with the shared helper
(§4.3), `neutral_fill` → `ui_hover_fill` / `ui_sel_fill` (§3.4), radius sweep to
the table (§2.2), rules replacing surface steps (§2.4). Gate:
`hosttest/uitest`, §10.2 items 5, 6, 7.

**PHASE 2C — the dead-primitive hosttests (§10.3).** Small, independent, and it
**must land before phase 3 touches the wallpaper.**

**PHASE 3 — `kernel.zl` shell.** Delete the island, the dock and the start menu;
build the rail (§6.2), the slug (§6.6), the wallpaper (§6.1), the grid (§6.3),
the module highlight (§6.4), the one modal (§6.6). `snap_rect` gains
`reserve_left` (§6.5). Gate: §10.2 item 10, the `kernel.zl` literal gate at its
new strictness, and a `wmshot` at four resolutions.

**PHASE 4 — `gen_icons.py` grammar redraw (§7), tiers 1→2→3**, plus deletion of
the §1.9 compatibility block once every call site names a real surface.
**The register (phase 3) must exist before the icons are redrawn**, because the
register is the hedge that lets the four-construction grammar stay strict
(§7.3).

---

## 12. THE WEAKEST LINKS, NAMED UNPROMPTED

1. **Warm paper + vermilion + a Swiss grid is itself becoming a reflexive
   anti-slop answer.** This spec escapes the near-black attractor and lands near
   a second one. What keeps it out is structural, not chromatic: the numbered
   register that fuses launcher + tasklist + keymap into one list, the tiling
   grid drawn on the desk with a legend stating the measured module size, and the
   per-window `[3,1 6x4]` readout. **If any of those three is dropped for
   schedule, the direction is gone and only the palette is left.**
2. **Nothing here has been rendered.** Every performance and appearance
   statement in this document is either read out of the source or computed. The
   contrast numbers, the atlas mapping, the `ui_ink_on` resolutions, the grid
   arithmetic and the `fb_rrect` clamp behaviour are measured or derived; **how
   it looks is not.** §10.5 is the first thing to do after phase 1, not the last.
3. **`ZD_GRID_LINE` at 2.02:1 through an RGB565 wallpaper cache is the most
   likely silent failure.** If the grid does not survive quantisation, the desk
   is a flat beige field and the identity halves.
4. **The four-construction icon grammar may not survive contact with 89 icons.**
   Some will want a shape that is not a rectangle, a 45-degree diagonal, a circle
   or a quarter-round, and the temptation will be to add a fifth construction —
   which is how the family dissolves. Phase order (§11) is the hedge; ignore it
   and the pressure arrives with nothing holding it back.
