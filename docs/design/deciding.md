# Deciding — the four as ten independent choices

You said you like different aspects of PLATE, Folio, Datum and Raking. That is not
indecision. Those four are **bundles**: each one took a position on the same ten
decisions and shipped the bundle as if it were one thing. Liking parts of all four
means the bundling is wrong, not the taste.

So stop picking a direction. Pick a **position per axis**, in the order at the
bottom, and check each pick against the conflict matrix before moving on.

**[`prototypes/decide.html`](prototypes/decide.html) is this page as a working
tool.** Same axes in settling order, same costs on the face of every option, and
the conflict matrix runs live against the current selection: an exclusive pair
comes back as a hard conflict naming the mechanism, a degrading pair as a warning
carrying its number, a requires pair as a statement of what it needs. It keeps a
plain-English brief you can hand to an implementer, and the three blends are
one-click presets you can then edit. State lives in the URL hash, so a combination
is one link.

Every number in this page was recomputed from the hexes in `specs/*.json` and the
`:root[data-theme="dark"]` blocks in `prototypes/proto-*.html`, not copied from the
prose. Where a number differs from what a spec claims, this page says so.

---

## The ten axes

Each row is a decision, its positions, and what that position costs. A position with
no cost has not been understood — every one here has one.

### 1. Ground polarity

*Which end of the value ladder does the user's document sit on, and does a second
ladder exist?*

| position | from | mechanism | cost |
|---|---|---|---|
| **Light paper primary, dark as a role remap** | PLATE, Folio, Datum | sheet at L\* 94–96 (`#F2EFE6` 94.45, `#F4F1EA` 95.20, `#F5F3EE` 95.87); dark mode is the *same* token names bound to a second ladder | Full-screen warm paper on an OLED panel at 2 a.m. Every direction that took this position states it as a risk and none of them solved it. |
| **Dark stock primary, no light twin** | Raking | warm graphite at L\* 18.15 (`#322B27`), six rungs inside 12 L\* points | Deletes the material law (axis 2) outright and halves the drag shadow (see C7). It is also the position the identity judge scored 4/10 — dark-warm-with-blue is the most crowded space in desktop UI. |
| **Dual ladder, both real** | PLATE, Folio, Datum all shipped one | two `:root` blocks, identical token names, values re-derived per mode rather than inverted | A token that exists in one mode only is a defect, so every future token costs two picks. PLATE pays it in screened areas: a large tonal fill is held at 5.60:1 in dark (`#A7A191` on `#2E2921`) instead of the solid 9.00:1 (`#D5CBBC`), so meter fills, sliders and plot bars read softer. |

**This axis is not "light or dark". It is "which ladder is load-bearing, and does the
other one exist."** Those are separable and the four conflate them.

### 2. Material law

*Is any large surface drawn at the opposite end of the ladder, and does that carry
meaning?*

| position | from | mechanism | cost |
|---|---|---|---|
| **Two materials, by law** — paper is the user's, ink is the machine's | Datum | rail, raster strip, drawer, menus and modals are all `#101317`; 14.26:1 against the ground `#E4E1DA`. Ink surfaces need no border, no shadow, no blur | **Requires a ground at L\* ≥ 89.6** (see C3). It is a luminance idea and dark takes the luminance away. |
| **One material, inversion as a state** | PLATE | ink appears only as the rail, a *focused* header, a terminal ground and a pressed control — 16.27:1, used as an event, not as a substance | The rail is a 296-device-px black column at ui=2 that is never explained by anything except "it is chrome". No second material means no way to say "this readout is the machine talking". |
| **One material, no inversion anywhere** | Folio | the rail is the same `#F4F1EA` sheet as a window; separated from the ground by one 1.71:1 hairline | The whole screen is one value. Two overlapping unfocused sheets are indistinguishable except by a 1.71:1 hairline — Folio names this as its own residual weakness. |
| **One material, light as the substance** | Raking | every surface within 12 L\* points; nothing inverts | See C2 — this position is what makes a knockout impossible. |

### 3. Window boundary

*What physically draws the edge of a window?*

| position | from | mechanism | cost |
|---|---|---|---|
| **2px ink frame, all four sides** | PLATE | `#14120E`, 16.27:1 on the sheet and 16.27:1 on another sheet | Two abutting windows show a 4px black bar. Survives overlap trivially, which is a strength PLATE then doesn't use because it tiles. |
| **Two-tone mount** — outer 1px hairline + inner 1px lighter | Folio | `#C2BAA8` (1.71:1) outside, `#FBFAF6` inside; a print mounted on board | 1.71:1 is below every threshold. It works because Folio also snaps to columns; the moment two unfocused sheets overlap by a few pixels, stack order is unreadable. |
| **1px outline at 3:1+, on both sides** | Datum | `#847E73` — 3.08:1 on the ground, 3.63:1 on the plate body. Clears WCAG 1.4.11 in both directions | Deliberately loud. It is the only boundary in the set that survives free overlap on its own merits, and it looks like a boundary rather than like paper. |
| **Directional 1px runs** — lit top+left, cut bottom+right | Raking | `#5F5854` top (1.99:1), `#4A433F` left, `#19120E` bottom/right (1.33:1) | **Fails silently.** The surrounding steps are 1.07–1.13:1, so a clipped or omitted edge does not look slightly wrong — the widget vanishes into its parent. Raking's own risk list says exactly this. And the bottom/right edge, the one that lands on top of another window, is the *weak* one at 1.33:1. |
| **Cut edge** — the occluder draws the border, never the occluded | Apparatus (round two) | needs no new primitive; a top sheet lying inside a lower one gets all four edges at 16.20:1 | Zero value under a tiling arrangement, because nothing overlaps. See free riders. |

### 4. Depth rank

*How do you read which of two things is on top?*

| position | from | mechanism | cost |
|---|---|---|---|
| **Rule weight, two values** | PLATE | 1px `#5C574C` between rows and cells, 2px `#14120E` between regions. Nesting a surface does **not** change its value | Only two ranks exist. A popover on a menu on a window has nowhere to go. PLATE avoids the problem by having exactly one modal in the whole system. |
| **Keyline rank, three values** | Folio | ground→sheet 1.71:1, sheet→popover 2.48:1, popover→modal 16.18:1 + a 24% flat scrim | The three ranks are not perceptually even: 1.71 → 2.48 is a whisper, 2.48 → 16.18 is a cliff. Rank 1 and rank 2 are hard to tell apart at arm's length. |
| **Material** | Datum | anything transient is ink: 14.26:1 with no border, no shadow, no blur | Dies with the ground (C3). Also: ink menus on paper is the most contestable single call in the set, by its author's own admission. |
| **Light direction** | Raking | one implied source upper-left, everywhere, no exceptions; raised = lit top + grazed left, sunken = same four with the sign flipped | The entire separation budget is 1px wide. Loudest achromatic element in the system is 1.99:1. |

### 5. Focus signal

*The one axis where the four disagree most, and the one Zac will feel every day.*

| position | from | mechanism | measured loudness | cost |
|---|---|---|---|---|
| **Knockout** — the header inverts to solid ink, title knocked out | PLATE | full value flip of the header band | 16.27:1 between states; **12.17 %** of a 445×460 window's pixels change | Requires a fillable header band (kills Folio's running head, C8). Overwhelms any low-contrast ladder (C2). |
| **A rule doubling and going black** | Folio | 1dp `#C2BAA8` → 2dp `#17150F` under the running head, plus title regular→bold and `#5E5648`→`#17150F`, plus the TOC row's 3dp marginal bar | 1.71:1 → 16.18:1, over 2 × window-width px | Three cues move together, which is good, but all three are *thin*. On a busy screen the eye has to find a 2px line. |
| **A head rule going accent** | Datum | outline top edge 1px `#847E73` → 2dp `#0B5E78`, title ink → `#14130F` | 6.56:1 accent, stated in three places at once (plate, rail tick, strip title) | Spends the one accent on focus, so the accent can never mean anything else. In dark the accent has to be re-picked (`#3E9FC6`, 4.76:1) because the deep blue is a 2.36:1 smudge. |
| **The light swings** | Raking | focused plate's top+left runs go `#5F5854` → `#7FB2E0`; title `#BDB5A7` → `#F2EDE4` | edge 1.99:1 → 6.19:1, title 6.84:1 → 11.92:1, on **0.42 %** of the window's pixels | Its own implementer's stated weakest link: "one blue pixel on the top edge plus a title going from `#BDB5A7` to `#F2EDE4`. At arm's length on a busy desktop it is near the floor of obvious-at-a-glance." |
| **Hoist it off the window** | SPREAD (round two) | the knockout becomes a screen-wide running head, one head for the whole screen | undrawable for any window that cannot own a disjoint horizontal span — SPREAD was told to define that fallback before drawing a single segment |

The 0.42 % vs 12.17 % figure is the whole argument. Both are computed on the same
window (445 × 460 device px, measured off `render/raking-desktop.png`, corner radius
10): Raking changes `(445−20) + (460−20) = 865` px, PLATE changes `445 × 56 = 24,920`.

### 6. Colour policy

| position | from | mechanism | cost |
|---|---|---|---|
| **Zero** — colour never distinguishes anything | Folio | one chromatic ink `#A3301B`, meaning attention and nothing else, 6.21:1. Hierarchy is case, weight, scale, rule-weight, whitespace | Everything has to be carried by type. When type is the only channel, a bad atlas or a wrong tracking value is not a blemish, it is a functional bug. |
| **One structural ink, four jobs** | PLATE | vermilion `#C4341C`: focus/run mark, the one primary action, the register mark, the crop marks. Never status, never text, never an icon | 4.74:1 on paper, 3.54:1 on the well, 3.43:1 on the ink plate — it can never carry text anywhere in the system, so it is a fill or a mark and nothing else. |
| **One instrument accent + a knockout twin** | Datum, Raking | Datum `#0B5E78` / `#5CD2F2`; Raking `#7FB2E0` / `#A9CFF2` | Blue is the most common accent hue in software. Raking's whole differentiation is that it is 43 % saturation on a warm ground — raise the saturation and it collapses into generic dark-UI-with-blue. Saturation is a locked value, not a taste knob. |
| **Five lexical inks with a written contract on screen** | Neatline (round two) | water = I/O, wood = user data, contour = level, road = routes and actions; all forbidden as text; none carries state | Needs the contract visible, which costs a permanent KEY panel. |

All four already spell the state word next to the state colour (`OK` / `WARN` /
`FAIL`), so **no direction uses colour as a sole carrier**. That is why this axis is
cheap to settle late.

### 7. Arrangement

| position | from | mechanism | cost |
|---|---|---|---|
| **Tile onto a drawn module grid** | PLATE | 12 × 8, integer division only; at 1920×1200/ui2 columns 1–4 are 116 px and 5–12 are 115 px, rows exactly 121. Overlap exists as an explicit `DETACHED` state | **There is no reflow story at all.** An app authored to fill 7×8 overflows its own frame at 7×4. Four separate defects in the prototype were that one failure wearing different hats. Choosing this buys a `sheetRows()`-style gate **in the shell**, not sprinkled through app renderers. |
| **Column snapping, floating underneath** | Folio | 12 columns of 48dp with 16dp gutters; exact at 1920×1200 (752dp = 12×48 + 11×16); remainder split into the outer margins | Snapping is a default, not a law, so the ambiguous-overlap case still exists — just rarely. |
| **8dp grid whose origin is the datum** | Datum | plates snap to 8dp measured from the rail's right edge, not the screen edge, plus a mandatory 1dp gutter | The gutter is the seam, so a window flush against another shows 1 device px of ground and nothing else. |
| **Free floating, unchanged wm.c** | Raking | z-ordered, draggable, resizable; per-window `fb_surface` cache retained | Every overlap lands on the weakest boundary the direction owns. |

### 8. Type and hierarchy carrier

There are three baked atlases (8×16, 16×32, 24×48) and no runtime rasteriser. Every
position below is really a statement about how many of those three you use.

| position | from | mechanism | cost |
|---|---|---|---|
| **Two sizes; hierarchy is case + weight + tracking + rule weight** | PLATE | four exhaustive styles: DATA (mono, one size, forever), LABEL (bold caps +2px tracked), BODY (prose only), PLATE (one per view) | Needs `UI_F_TRACK` / `UI_F_CAPS` on the existing flag word, and **measure and draw must share the helper** — uikit.c already documents that a measure/draw disagreement is a clipped label. |
| **Three sizes landing exactly on the three atlases** | Folio | R1 8dp/16px, R2 12dp/24px, R3 same size bold, R4 16dp/32px. Zero resampling | At `ui()==1` `prop_cell()` floors at 12px and R1/R2 merge. Hierarchy there rests on weight, case and whitespace alone — which still works, but it was read out of fb.c, not assumed. |
| **Six sizes from a 1.2 ratio + a face law** | Datum | 9.0 / 10.5 / 12.5 / 15.0 / 18.0 / 21.5. Every number the machine measured is mono; everything the user wrote is proportional | Six sizes against three atlases means real upscaling. This is why Datum scored 3/10 on buildability. |
| **Three sizes; hierarchy is the ink ramp** | Raking | SM 11 / MD 13 / LG 21, with `ZD_TEXT_0..3` doing the levelling | LG at 1.615× is deliberately off any smooth ramp so it can't be used as a heading. Loses tracking entirely — a tracking parameter on `fb_text_role` would be a new primitive and Raking judged it not worth one. |

**The prototypes flatter all four here.** CSS gives letter-spacing free; `fb_text_role`
cannot track text without a new primitive. Any position that leans on tracking is
carrying an unpriced item.

### 9. Instrumentation

| position | from | mechanism | cost |
|---|---|---|---|
| **A printer's slug** | PLATE | one mono line: `zlOS 0.3 · 1920x1200 · 4 CPU · LOAD 0.31 · 14:32:07`, plus a legend stating the grid **as measured** | 28dp of permanent vertical space for text that rarely changes. |
| **Marginalia** | Folio | four caption lines at the rail's foot, labels left, values right-flush in mono | Tabular figures in the margin, and nothing else. Cheapest position on this axis. |
| **Full instrumentation** | Datum | per-window `app_us` in every status band, a full-width raster strip with a live histogram against the 16,667 µs budget, a memory-map ruler along the bottom edge | Per-window µs **does not exist yet**: wm.c measures `app_us` in aggregate only. Datum's own fallback is the window's `repaint_pixels` in the same slot. And the strip is a perpetual animation on a damage-driven compositor (C6). |
| **Status column only** | Raking | network, volume, power icons recoloured by state | The least the machine can say about itself. |

### 10. Motion

| position | from | mechanism | cost |
|---|---|---|---|
| **Cut — 0 ms default** | PLATE | one animation survives: SLIDE 100 ms **linear**, only along the grid. Distance is always a whole number of modules + gutters, so intermediate frames land on integer pixels with no rounding drift | Nothing eases, so nothing feels like it has mass. Deliberate; some people read it as broken. |
| **Page motion, five tokens** | Folio | FADE 120 / POP 80 / PRESS 100 / SNAP 140 / GRID 100 | SNAP keeps the cubic easing, so the float-shaped thing stays in the motion path (stored as four ints ×1000). |
| **Three durations + one perpetual** | Datum | APPEAR 90 (5.4 frames), WIN 160 (9.6 frames), focus 0. Quantised against the **measured** 59.998 Hz | The raster strip advances one column per frame forever — see C6. |
| **Three durations keyed to distance** | Raking | RISE 90 / TRAVEL 160 / SETTLE 240, one curve. The focus transition is the signature: two 1px runs lerping between two colours | SETTLE 240 ms on a focus change means the focus signal — already the weakest in the set — takes a quarter of a second to arrive. |

---

## The conflict matrix

Eight conflicts, each with a mechanism and a number. Anything without a number is an
opinion and is not in this table.

### C1 · exclusive — Folio's zero-colour law × PLATE's vermilion

Both claim the same slot: the one primary action per view. Folio fills it with an ink
block `#17150F` at **16.18:1**. PLATE fills it with `#C4341C` at **4.74:1** on paper,
**3.54:1** on a well, **3.43:1** on the ink plate. They are not swappable at equal
legibility — adopting Folio's rule *raises* contrast on the control that most needs
it; adopting PLATE's *drops* it by 3.4×.

Adopting Folio's law deletes 1 of PLATE's 4 accent jobs. Deleting 3 of them was already
tried in round two and killed: *"PLATE with three of its four accent jobs deleted —
that is an edit to the spec, not a direction."* So this axis has no partial position.

### C2 · degrades — PLATE's knockout on Raking's ladder

Raking's four surface steps, from its own settings pane and reproduced here:
1.073, 1.126, 1.089, 1.077 : 1. The loudest achromatic element in the whole system is
`ZD_LIT` on `ZD_BASE` at **1.99:1**; the loudest element of any kind is `ZD_STEEL` at
6.19:1.

A knockout header — `#F2EDE4` ground on the plate — computes **11.92:1** against
`ZD_BASE` and **13.42:1** against the field. That is **6.0×** the achromatic budget and
1.9× the accent.

Two things break, both measurable:

1. **The light inverts sign on that one object.** `ZD_LIT #5F5854`, which is *the
   struck top edge, the light itself*, computes **5.98:1 dark** against a `#F2EDE4`
   header. `ZD_CUT #19120E` computes **15.88:1**. So on a knocked-out header the
   "lit" edge reads as a groove and the groove reads as the loudest line on screen.
   The recipe says raised; the pixels say cut.
2. **The steps stop being read at all.** They were already below perceptual threshold;
   next to a full inversion they are not even a whisper.

The knockout does fix Raking's stated weakest link. It also deletes Raking's premise.
Those are the same act.

### C3 · requires — Datum's two-material law requires a ground at L\* ≥ 89.6

Solve the law for the ground. Against ink `#101317` (Y = 0.00637):

| law holds at | ground needs Y ≥ | ground needs L\* ≥ |
|---|---|---|
| 14.26:1 (its light-mode value) | 0.7539 | **89.6** |
| 10:1 | 0.5138 | 76.9 |
| 7:1 | 0.3446 | 65.3 |
| 4.5:1 | 0.2037 | 52.3 |
| 3:1 | 0.1191 | 41.1 |

Now every dark ground in the set:

| ground | L\* | law would give |
|---|---|---|
| Raking `ZD_FLOAT` (its **lightest** surface) | 23.21 | 1.57:1 |
| Datum's own dark paper | 17.10 | 1.30:1 |
| PLATE dark paper | 16.88 | 1.29:1 |
| Folio dark sheet | 11.84 | 1.13:1 |

Datum's own dark mode measures **1.18:1** (`#241D14` ground / `#060B14` ink) — verified,
not quoted. Separation moves onto hue and onto the datum rule at **3.55:1**
(`#7C7360` on the ground).

**The law is not "dark mode is worse". The law is dead below L\* ≈ 65 and it is a
different design above and below that.** If you want Datum's material split, ground
polarity is already decided for you: light, primary, no negotiation.

Note the consequence Datum's dark mode pays and does not hide: the rail is still 56dp
of "the machine's material", but at 1.18:1 it is no longer a different material. Its
meaning evaporates while its screen area does not.

### C4 · exclusive — Folio's mount × Raking's directional edge

Both claim the 1px ring immediately around every raised object, and that ring can only
carry one rule.

- Folio's mount is **symmetric**: outer `#C2BAA8` (1.71:1) + inner `#FBFAF6`, two
  `fb_box` calls.
- Raking's is **asymmetric**: top `#5F5854` (1.99:1), left `#4A433F`, bottom and right
  `#19120E` (1.33:1).

Drawing both means a 2px edge on all four sides. That breaks Folio's rank contract
directly: rank 1 (ground→sheet) and rank 2 (sheet→popover) are *both* 1px and are told
apart only by value, 1.71 vs 2.48:1. Make rank 1 two pixels and the ranks collapse into
one another.

### C5 · requires — free floating requires a boundary at ≥ 3:1

Under overlap, the boundary that does the work is the **bottom and right** edge of the
upper window, sitting on the lower window's body. Measured:

| direction | overlap boundary | value |
|---|---|---|
| Datum | `#847E73` outline on plate body | **3.63:1** ✅ |
| PLATE | 2px `#14120E` frame on a sheet | **16.27:1** ✅ |
| Folio | `#C2BAA8` hairline on `#F4F1EA` | **1.71:1** ❌ |
| Raking | `#19120E` cut run on `#322B27` | **1.33:1** ❌ |

Folio and Raking both name this as their own residual weakness, and both mitigate it
with arrangement rather than with the boundary — Folio by column-snapping, PLATE by
tiling outright. So **axis 7 sets the floor on axis 3.** Pick free floating and only
two of the four boundary positions survive.

### C6 · degrades — Datum's raster strip × damage-driven repaint

Datum deletes `ZD_MS_PULSE`, `ZD_MS_PULSE_SLOW` and `ZD_MS_SWEEP` with an explicit
reason: *"an infinitely pulsing element means the damage list is never empty on a
compositor whose entire architecture is damage-driven repaint."* Then it makes exactly
one exception with exactly that property — the strip advances one 1px column per frame,
forever.

By Datum's own accounting the strip costs 3 `fb_fill_px` calls of 1px width per frame.
Small. But it is a **floor**, not a peak: at the measured 59.998 Hz it is
**215,993 composite wakeups per hour on a machine nobody is touching**, against 0 for
PLATE (motion CUT = 0 ms, nothing self-moving) and 0 for Folio and Raking at rest.
Datum's own prototype strip reports `damage 09 rects · blend 1.45 Mpx` per frame.

This is not fatal. It is a standing tax on the one thing this kernel does well, and it
should be paid knowingly.

### C7 · degrades — Folio's drag-lift shadow on a dark ground

Folio keeps exactly one shadow, on the drag lift, with a physical reason. On a light
ground the scrim buys **18.9 dL\*** of recession. On the dark stock it buys **8.7 dL\***
— 46 % of its light-mode strength — and *"the one shadow mostly stops working."*

The mechanism, in Folio's own dark block: on paper a shadow falls across desk and
printed line together and the line survives, because both terms scale. On ink the veil
is the lamp going down, and a light mark fading toward a black ground has nothing left
— the colophon measured 1.27:1 in Overview. No alpha fixes it: matching light's ratio
loss needs 0.30, which buys 1.5 dL\* and stops being a scrim.

**So: choosing a dark ground costs you the drag cue.** Folio's dark mode compensates by
strengthening the pre-drop column highlight to 6.4 dL\* (light's is 2.5). If you go
dark, budget a replacement drag cue explicitly — do not assume the shadow carries.

### C8 · exclusive — knockout focus × Folio's running head

Folio's running head is deliberately **not a filled strip**: it is the same
`#F4F1EA` as the body, *"so the window is one sheet rather than a panel with a label
nailed across it."* PLATE's knockout **requires** a fillable header band.

Same value flip, two areas: Folio's 2dp rule covers `2 × window_width` px; PLATE's
knockout covers `56 × window_width` — **28× the area for the same ~16:1 flip**. There is
no middle. SPREAD already took the third position (hoist the head off the window
entirely into one screen-wide running head) and immediately inherited the constraint
that it is undrawable for any window without a disjoint horizontal span.

---

## Free riders

Things that combine with anything, because of what they do **not** touch. Verified,
not assumed — each row says what it would have collided with and didn't.

| feature | from | why it is free |
|---|---|---|
| **Per-window µs in the status band** | Datum | It is mono text in a band every direction already draws. Touches no surface value, no boundary token, no radius, no focus signal, no colour. Costs one real thing: wm.c currently measures `app_us` in **aggregate only**, so per-window attribution is new work — with a stated fallback (`repaint_pixels`, same slot, same face, no layout change). |
| **The state word beside the state colour** | PLATE (all four do it) | `OK` / `WARN` / `FAIL` spelled next to the dot. Four characters. Works under every colour policy including Folio's zero-ink law — which is exactly why the colour axis can be settled last. |
| **Tabular mono for figures that must align** | Folio, Datum | Uses the mono atlas that already exists, at one size. Distinction is carried by **face**, not colour, so it survives Folio's law intact; it is also the cheapest half of Datum's face law, separable from the six-size ramp that made Datum unbuildable. |
| **The numbered register (`01 SHELL`, Alt+NN)** | PLATE | Text and two digits. Makes the launcher, the tasklist and the keymap one list. No surface, no colour, no radius. Combines with Folio's TOC directly — Folio already sets a folio numeral in that gutter. |
| **The measured legend** (`MODULE 12 x 8 · 116/115 x 121 px · GUTTER 16`) | PLATE | One mono line stating the grid **as computed at the current resolution**. It is instrumentation about the layout, not part of it. Free everywhere, and it is a live check on the integer division. |
| **Stacked rail clock** | Raking | A 56dp rail cannot hold `23:41` on one line at display size, so hours sit above minutes. Free with respect to every axis — but it **requires a rail of ~56dp**. PLATE's 148dp rail has no use for it. |
| **The radius system** | all four | 0/2 (PLATE), 0/2/`h/2` (Folio), 0/3dp/`h/2` (Datum), 0/10/5/3/`h/2` (Raking). Every one resolves through the existing `fb_rrect` with **no new primitive**, and `fb_rrect(r=0)` already degrades to three `fb_fill_px` calls. Near-free. One coupling: Raking's "nesting halves it" needs concentric rrects, which its depth recipe draws anyway. |
| **Apparatus's cut edge** — the border you can see is drawn by the occluder, never the occluded | Apparatus (round two) | Free with respect to palette, type, colour and focus, and it needs no new primitive. **Conditional, not unconditional:** its value is zero under PLATE's tiling, because nothing overlaps. Its loudest case is a top sheet lying entirely inside a lower one, where the rule draws all four edges at 16.20:1. Pick free floating (axis 7) and this is the best answer in the set to C5; pick tiling and it is inert. |

**Not free, despite looking it:** Datum's raster *strip* (C6 — it is a perpetual
animation), and any position that leans on letter-tracking (the prototypes get it free
from CSS; `fb_text_role` cannot do it without a new primitive).

---

## The order to decide in

Settle these in order. Each one deletes positions further down; going out of order
means re-deciding.

**1 · Ground polarity (axis 1) — first, and it is not close.**

It is the only axis that *deletes* positions elsewhere rather than constraining them:

- Datum's material law is dead below L\* ≈ 65 (C3). Not weakened — dead.
- Folio's drag-lift shadow loses 54 % of its recession on dark (C7).
- Raking's entire depth recipe is a dark-ground idea: a 1px *lighter* run only reads as
  light when the surface is dark. On a light surface the lit edge computes 5.98:1 as a
  **dark** line (C2).

Answer two questions here, not one: *which ladder is load-bearing*, and *does the
second ladder exist as a role remap*. Three of the four proved the second is cheap —
same token names, values re-derived — and PLATE proved what it costs (screened areas
held at 5.60:1 instead of 9.00:1).

**2 · Arrangement (axis 7) — because it sets the floor on the boundary.**

Free floating requires a window boundary at ≥ 3:1 (C5), which kills Folio's mount and
Raking's cut run. Tiling avoids the problem entirely and buys a hard prerequisite
instead: a shell-level reflow gate. PLATE has no reflow story at all, and four separate
prototype defects were that one failure wearing different hats.

Free floating also turns Apparatus's cut edge from inert into the best available answer
to the overlap problem.

**3 · Focus signal (axis 5).**

Settle it third because it consumes the single largest block of contrast in whichever
ladder you picked, and because it is where your four disagree most: **0.42 %** of a
window's pixels (Raking) against **12.17 %** (PLATE), on the same window, for the same
job. Everything downstream fits around whatever this takes.

If the ground is dark, the knockout is off the table (C2) unless you also accept that
the ladder stops doing any work.

**4 · Window boundary + depth rank (axes 3 and 4) — together, because they share the token.**

Whatever draws the edge is also what says "this is above that", in every one of the
four. Deciding them separately produces a system with two edge tokens and no rank.

**5 · Material law (axis 2).**

Now it is decidable, because it is the intersection of what step 1 left alive and how
much of step 3's budget is spent.

**6 · Everything else: colour (6), type (8), instrumentation (9), motion (10), radius.**

All of them late, all of them reversible, most of them free. Colour last specifically
because **no direction uses colour as a sole carrier** — every state colour already
ships the word beside it — so changing the policy repaints things without breaking any
of them.

---

## One thing this page cannot tell you

Every judgement above was made against a headless browser render, not against `fb.c`
output on a 2560×1440 panel. The prototypes flatter the target in at least one known
way: CSS gives letter-spacing free, and `fb_text_role` cannot track text without a new
primitive.

Two of the picks above are the ones most likely to move on real glass, and both were
flagged by their own authors: Raking's warm-graphite ladder ("must be judged on the
ThinkPad X1 Carbon's actual panel and not in a PPM viewer") and Raking's focus signal
("the first thing I would test on a real panel rather than a PNG"). If focus is the
axis you care most about — and the 0.42 % figure suggests it should be — that is the
one to render through `hosttest/wmshot` before committing.

---

## The final two: paper vs graphite

Zac narrowed to `proto-blendpaper.html` and `proto-blendgraphite.html`. This section
records the recommendation and, more usefully, the asymmetry it rests on.

**Recommendation: the paper blend.** The reason is risk, not taste. Both are good.

### What each implementer said about its own work

Neither claimed a win over PLATE, and both said why.

**Paper**, on itself: *"an improvement of degree, not of kind, and the honest word is
additive. It does not change a single one of PLATE's ten answers... the identity is
100% PLATE's; nobody looking at this would call it a new direction. The fair
description is 'PLATE plus the instrumentation it should have had', not 'a fifth
direction'."*

**Graphite**, on itself: *"strictly better than Raking - it keeps the premise and
repairs both defects Raking's own implementer wrote down, and neither fix cost the
premise a token. Against PLATE it is not a win, it is a different bet. It inherits
Raking's identity problem almost whole: it is still dark-warm-with-blue, which is the
most crowded space in desktop UI, and the numbered register does not change that - it
changes the rail, not the impression. Raking scored 4/10 on identity and I have not
moved that number by much."*

### The asymmetry that decides it

Both carry unverified claims. They are not the same *kind* of claim.

| | paper | graphite |
|---|---|---|
| what is unverified | the per-window microsecond figure is derived from `repaint_pixels` by an invented constant; the "0 idle wakeups" claim is read off the architecture, not measured | `ZD_EDGE_OVER` is the entire boundary story, has never been through `fb.c`, and clears the 3:1 floor by 4% (3.13:1) |
| what breaks if wrong | a number on screen is wrong. Delete the label or replace it with a real reading | the depth model stops separating windows |
| cost to fix | one line | re-derive the ladder |

Paper's failure mode is a wrong caption. Graphite's failure mode is the design.

### Why the panel makes it worse for graphite

The point of this project is the Intel driver on a 2560x1440 eDP panel, and `fb.c`
does subpixel LCD rendering through a 5-tap FIR with gamma-correct blending in linear
light. That pipeline is exactly what a browser render does not model.

- **Paper's separation is a 2px ink frame at 16.27:1.** Nothing in that pipeline
  threatens it.
- **Graphite's separation is 1px runs across surface steps of 1.073-1.126:1, judged in
  Chromium, with a 4% margin.** Its own report names the second hazard too: the wash is
  a 10%-alpha ramp across a plate width, *"precisely the case Raking's own spec warns
  will band without fb.c's 4x4 threshold"*. And the occlusion rule as prototyped
  repaints all four edges of every overlapping plate, which has not been shown cheap in
  the damage math.

### What choosing paper does not cost

It does not cost dark. Paper's dark ladder is ortho lith film and its verdict was
*"dark keeps the direction"* - the only direction in the set whose contrast failures
came out **symmetric across both modes**, which is the signature of a shared token
ladder rather than a bolted-on negative.

It costs *dark-native*. Graphite is the only design here whose dark ground is
load-bearing for its own depth model rather than tolerated by it. If the machine will
be run dark nearly always, that is a real thing to give up.

### The test to run instead of trusting this page

Both take the same scene hashes. Open them side by side at `#monitor` and at `#files`,
sit back to normal viewing distance, and see which one you can find the focused window
in without hunting for it.

That is the 12.17% versus 0.42% figure made physical, and it is the one axis where the
two genuinely disagree. Nobody's opinion beats looking at it.

---

## The graphite-plus-paper mixes, and the number that decides them

Zac preferred the graphite blend for feeling modern, and asked for paper's character
brought into it. That is not one design - it is three resolutions of a single conflict
that `deciding.md` had already measured: PLATE's knockout computes 11.92:1 against
graphite's 1.99:1 achromatic budget, and worse, it inverts the light's sign (on a
knocked-out header the lit edge reads 5.98:1 DARK and the cut reads 15.88:1, so the
raised edge becomes a groove).

| | resolution | focus signal |
|---|---|---|
| **PRESSWORK** | widen the ladder until a knockout fits | knockout, 7.133% of a plate |
| **STOCK** | keep the ladder, overprint instead of invert | vermilion fore-edge, 1.997% |
| **INKSTOCK** | two dark materials, hue and texture | material change, 1.0018:1 luminance |

### The measurement

Focus signal as a share of a window's own pixels, on the reference 600x420 dp plate:

```
PLATE (paper)     12.174 %
PRESSWORK          7.133 %
graphite blend     7.127 %
STOCK              1.997 %
Raking original    0.420 %
```

PRESSWORK's figure was re-derived here rather than quoted, because it is a number the
file asserts about itself:

```
header 600x28  = 16,800 px      claim 16,800   OK
bar    3x392   =  1,176 px      claim  1,176   OK   (--zd-focus-bar: 3px, not 2)
total          = 17,976 px      claim 17,976   OK
fraction       =  7.133 %       claim  7.133 % OK
```

**PRESSWORK changes the same area as the graphite blend - 7.133 % against 7.127 % - and
raises the contrast inside it from 1.3999:1 to 6.4796:1, a factor of 4.63.** It does not
add a new mark to the screen; it makes the mark graphite already had legible. That is
the strongest single argument in the set, and it is arithmetic rather than taste.

### What the implementers said about their own work

**STOCK**, on itself: *"the safest and the least interesting. It resolves the conflict by
routing around it: it spends in a currency the ladder does not use, which is clever and
is also an admission that the ladder cannot afford a luminance event... what it
definitely did not keep is graphite's identity gesture - the light no longer swings, it
is constant and indifferent to attention."* Its own recommendation: *"If the question is
'which will still be right in six months', build STOCK. If the question is 'which is a
design rather than a compromise', get PRESSWORK's numbers before you decide."*

**INKSTOCK**, on itself, recommending against itself: *"in `#files` and `#monitor` I know
which window is focused instantly, and the thing that tells me is the vermilion frame,
every time. The material change is the second thing I notice, after I have already been
told where to look."* Its 1.0018:1 material separation is not doing the work, and it is
weaker in light mode than in dark - the wrong way round for a mode named WORK LIGHT.

### One thing no agent flagged

In STOCK the vermilion is not exclusively the focus signal - it also fills the primary
action button and the RUN indicator. So the focus mark competes with its own accent.
PRESSWORK does not have that problem: the knockout carries focus, which leaves the
vermilion free for actions. That is PLATE's original "one ink, four jobs" dilution
inherited without being noticed.

### Standing caveat

All of it is judged from headless browser renders. The three differ most on exactly the
axis a browser models worst: 1px runs, ordered-dither texture, and hue at low luminance
separation. None has been through `fb.c` on the 2560x1440 panel.

### PRESSWORK's numbers, independently re-derived

Every figure PRESSWORK asserts about itself was recomputed here from its own token
values rather than quoted, because a file measuring itself is exactly the claim that
needs checking. All four hold to four decimal places:

```
knockout vs plate     #B6B0AB on #322B27  = 6.4796:1   file claims 6.4796  OK
lit run vs plate      #6F6864 on #322B27  = 2.5423:1   file claims 2.5423  OK
light knock vs sheet  #47433D on #EBE6DC  = 7.8968:1   file claims 7.8968  OK
focus area            600x28 + 3x392      = 7.133 %    file claims 7.133   OK
```

One figure it does not state, and which matters more than the ratio: the focused
header moves **54.0 L\* points** - plate L\* 18.15 to knockout L\* 72.19 - across 7.133 %
of the window. For comparison the raking light's own struck run moves 26.4 L\*. So the
knockout is roughly twice the lightness excursion of the mechanism it sits on top of,
which is what "6.0x the achromatic budget" means in perceptual terms rather than in
ratio terms.

**Conclusion of the mix round.** PRESSWORK spends the same screen area as the graphite
blend Zac liked (7.133 % against 7.127 %) and raises what happens inside that area by
4.63x. It does not add a mark; it makes graphite's existing mark legible. STOCK spends
a third of the area in a currency the ladder does not use, and its own implementer calls
that "an admission that the ladder cannot afford a luminance event". INKSTOCK's own
implementer recommends against it.

Ranking, on measured evidence rather than preference: **PRESSWORK, then STOCK, then
INKSTOCK.** This reverses the earlier lean toward STOCK in the section above, and the
reason is that STOCK's 1.997 % was not known when that was written.
