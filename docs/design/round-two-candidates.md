# Round two — twelve candidates, six to build

Round one produced four directions (`specs/*.json`, `prototypes/proto-*.html`) and
PLATE won the judged comparison with Folio second. Zac likes **PLATE** and **Folio**
and wants more in their family.

They agree on: warm paper, square corners, hard rules, a left rail, no shadows, a real
type-register change rather than a size bump, and killing the dock / top bar / start
menu. They disagree on **exactly one thing** — PLATE spends one saturated accent,
Folio spends none. That disagreement is the seam a merge runs along, and it is where
the merge lane below lives.

Twelve candidates were proposed across four lanes:

| lane | brief |
|---|---|
| **press** | print genres other than the technical document — ledger, map sheet, engineering drawing |
| **page** | the desktop as a set page — newspaper, critical edition, timetable |
| **merge** | PLATE × Folio, resolving the accent seam one way or another |
| **linux** | the good ideas from BeOS/Haiku, CDE/IRIX and NeXTSTEP, re-cut into this family |

This page holds all twelve as proposed, the six chosen with the one change each needs
before it is built, and the six kills with the reason.

## What already failed — do not re-propose

- near-black surfaces plus one neon accent. That is the current desktop and it is what
  a generic prompt produces.
- top bar + floating windows + bottom dock. All four round-one directions independently
  deleted it.
- a row of identical rounded tiles as a launcher.
- every corner at one radius.
- drop shadows used as separation instead of layout.
- purple or indigo gradients on dark. Gradient text. Emoji as decoration.
- Inter as a primary face.

## Constraints every proposal must respect

- Text comes from three baked bitmap atlases (8×16, 16×32, 24×48, from DejaVu Sans
  Mono) plus anti-aliased and subpixel variants. **There is no runtime glyph
  rasteriser.** A proposal needing six type sizes needs upscaling; Datum lost points
  for exactly this.
- `fb.c`'s whole vocabulary: `fb_fill_px` `fb_fill_blend` `fb_rrect` `fb_rrect_blend`
  `fb_rrect_grad_top` `fb_gradient` `fb_grad_conic` `fb_grad_radial` `fb_wedge`
  `fb_shadow` `fb_glow` `fb_shade` `fb_line` `fb_box` `fb_bar` `fb_image` `fb_icon24`
  `fb_clip` `fb_text_prop` `fb_text_aa` `fb_text_role` `fb_text_rich` `fb_text_scaled`
  `fb_glyph_aa` `fb_blur_paint` `fb_surface_begin/blit/end`. Needing something outside
  this list is a real cost.
- Spacing lives on a 4/8/12/16/24 scale.
- Preserve: subpixel LCD text, gamma-correct blending in linear light, dithered
  gradients, anti-aliased rounded corners, geometric icons.

---

# The six chosen

| # | direction | lane | the one structural move |
|---|---|---|---|
| 1 | **Apparatus** | page | every window is a scholarly text — line numbers in the outer margin, the machine's commentary at the foot, and **the cut edge**: the border you can see is drawn by the occluder, never the occluded |
| 2 | **Deck** | page | no window frames at all; stories abut across shared rules and opening a window re-makes the page |
| 3 | **Neatline** | press | a full-perimeter map apron whose **key** is the launcher, and five inks that are lexical because the key on screen defines them |
| 4 | **Blueline** | press | the only dark-primary member: saturated blue ground, no black anywhere, and focus carried by **line type** — continuous / hidden / chain-dot / phantom |
| 5 | **SPREAD** | merge | PLATE's knockout lifted off the window into one running head shared by the whole screen, plus a cross-window baseline grid |
| 6 | **Shelf** | linux | a **mid-grey** desktop, a right-edge dock of live instruments, and the single saturated ink spent on selection — the thing neither parent solved |

Coverage: 2 press, 2 page, 1 merge, 1 linux. All four lanes represented.

Spread check — no two survivors put persistent chrome in the same place:
Deck (head + foot bands) · Neatline (full perimeter apron) · Blueline (one corner
block) · Apparatus (left rail) · SPREAD (left rail + shared running head) ·
Shelf (right dock + a movable menu column).

Six different focus mechanisms: apparatus opening · a deck line materialising ·
the neatline closing plus margin ticks · line type · a hoisted knockout segment ·
a grip hatch appearing plus the menu column's contents changing.

Grounds: three warm papers, one newsprint, one saturated blue, one mid-grey. Dark
ladders: iron-gall ink, composing stone, IHO S-52 night, cyanotype, carbon paper,
NeXT textured black.

---

# Lane: press

## 1. Ledger — **KILLED**

**pitch.** The desktop is a ruled account book that has to balance — windows are
entries on a fixed rule pitch in unequal money columns, and a foot rule reconciles what
every window claims against what the machine actually has.

**lineage.** Takes from PLATE: warm stock, hard rules as the entire depth system,
tracked caps, radius 0, mono figures right-aligned on a shared decimal, one saturated
second ink. Takes from Folio: tabular figures as an editorial device, and hierarchy by
rule weight rather than fill. Breaks three things deliberately. (1) Chrome leaves the
left edge entirely — a HEAD band and a FOOT band, so nothing vertical is reserved and
windows get full screen width. (2) The grid columns are **unequal by law**: one wide
description column plus six narrow figure columns (8/2/2/2/2/2/2 units), which kills
PLATE's uniform 12×8 module and makes the equal-tiles failure unreachable at shell
level. (3) Focus is not a knockout — the ink plate is deleted outright, and a focused
entry is closed with a **double rule** the way a ledger closes a total.

**structure.** No rail. Persistent chrome is two horizontal bands. HEAD 28dp: column
headings in tracked caps, folio, date, closed by 1px. FOOT 64dp: **the balance** —
CPU / MEM / DISK / NET as four ruled money columns, figures right-aligned on a shared
decimal, above a double rule (2px ink / 2px gap / 1px ink), reconciliation on the last
line. Between them the page is ruled at a fixed 28dp line pitch across the full width,
banded 4-on/4-off, with 1px blue column rules at the seven column boundaries. Windows
snap to whole rule-lines vertically and to column boundaries horizontally; because the
columns are asymmetric, legal window widths are an explicit small set and none of them
is one-third.

**palette.** Both modes. LIGHT, *the pad*: `ZL_PAD #EFEBDF` sheet, `ZL_BAND #D3DECB`
the green bar (zebra step 1.17:1), `ZL_DESK #DFDACB`, ink `#12140F` (15.56:1 on pad,
13.31:1 on band), ink2 `#4C4A3E` (6.40:1 on band), row hairline `#7C8A79` (3.05:1), and
the **column rule in ledger blue** `#3C5A6E` (6.12:1 on pad) — the printed ruling and
the written figures are two different inks, which generalises PLATE's one-accent idea
into a rule about who wrote what. `ZL_RED #9E2B18` (6.26:1) for exactly one thing: a
negative or over-committed figure. DARK, *the slate*, made of the ledger before paper —
a school slate ruled in chalk, not an inverted pad: ground `#22262A`, band `#2E3439`
(1.21:1), chalk `#E6E2D6` (11.76:1), chalk2 `#9EA39F` (5.94:1), rules `#6E7C86`
(3.55:1), sanguine (red chalk) `#D2755B` (4.67:1). Stated cost: a slate has two chalks,
so dark **loses** the blue/grey rule-hue distinction and must carry rank on weight and
rule density — the zebra moves from a fill to a rule at every band boundary.

**focus.** The focused window is bracketed by a double rule — 2px ink, 2px gap, 1px ink
— across its full width above the header and below the footer, the accountant's mark
for "this line is a total". Its title and all its figures step ink2 → ink0 (6.40 →
13.31:1 on the band). The signal is then echoed arithmetically: in the FOOT balance only
the focused window's contribution to each account is set in full ink, every other
window's in ink2. Unfocused windows carry a single 1px hairline. Nothing inverts,
nothing changes hue, nothing gains a colour.

**not default because.** A generic prompt cannot produce a desktop whose chrome performs
arithmetic. The foot is not a status bar, it is a reconciliation, and its columns must
add up: sum of per-window claims plus free equals capacity, with the discrepancy printed
in ledger red when it does not. That makes the WM's resource model a first-class
readable object instead of a decorative meter, and Ledger is the one direction here
whose chrome would be **wrong** rather than merely ugly if the numbers were fake.
Second: unequal columns. Every generic layout system produces equal columns; a ledger's
are asymmetric by function. Third: it deletes both the left rail and the ink plate — the
two things PLATE and Folio independently agreed on — and still lands in the same family.

**weakest, in its own words.** The premise rests on a number nobody has measured. If the
shell cannot actually attribute CPU, memory and disk per window, the foot balance is a
decorative sum wearing an accountant's clothes — and this repo has a documented history
of exactly that class of failure (a guard that guarded nothing, a written rationale
nobody ran). Ledger is the only candidate here that is only as good as its
instrumentation, and the instrumentation does not exist yet. Secondary and cosmetic by
comparison: the zebra is 1.17:1 in light and 1.21:1 in dark, below perceptual threshold
on its own — it counts rows only in company with the hairlines, and Raking already lost
points for spending its budget on steps that small.

## 2. Neatline — **CHOSEN**

**pitch.** The desktop is one map sheet — a full-perimeter marginal apron carrying grid
coordinates on three edges and the KEY on the fourth, and five inks that each mean
exactly one class of thing because the key on screen says so.

**lineage.** From PLATE: warm stock, hard rules, radius 0, a drawn grid that *is* the
layout system rather than decoration, mono readouts, and chrome that states machine
state instead of implying it (PLATE's `[3,1 6x4]` becomes a real grid reference). From
Folio: the launcher as a set table rather than a tile row. Breaks two things.
(1) Colour policy: PLATE spends one accent, Folio spends none, Neatline spends **five**
and makes each one lexical, with an on-screen key that defines it — the opposite of
decorative colour, and the only position on that seam neither existing direction
occupies. (2) The margin is **brighter** than the field (`#F5F2E8` apron over `#E7E2D4`
body, 1.16:1), inverting PLATE's darker-desk-than-sheet relationship, because on a real
sheet the margin is unprinted paper and the body carries the base tint.

**structure.** Chrome is a perimeter apron, not a rail. Top 24dp: eastings ruled at
every module boundary plus the sheet title in tracked caps. Left and right 24dp:
northings and the tick channel. Bottom 96dp: **the key** — three columns of symbol /
name / shortcut, simultaneously the launcher, the tasklist and the definition of every
mark on screen; plus the **scale bar** (the measured module size at the current
resolution and ui scale) and the **north point** drawn with `fb_wedge`, whose four
quadrants are the four workspaces. Inside the neatline the field is ruled on a 100-unit
grid, 1px, every fifth line heavier. Windows are placed at a grid reference and each
header prints it in the real format — two letters, four digits — which is exactly six
mono cells wide.

**palette.** Both modes. LIGHT: apron `#F5F2E8`, paper `#F1EDE1`, field `#E7E2D4`,
black `#101211` (16.07:1 on paper), grid line `#8E897A` (2.70:1 on field). Five lexical
inks, all measured at or above 4.10:1 on the field and 4.53:1 on paper, and all
**forbidden as text**: WATER `#42718A` = I/O, net, pipes, streams (4.10 / 4.53); WOOD
`#4F6F44` = user data and files (4.40 / 4.86); CONTOUR `#95592D` = level and pressure —
CPU, memory, heat, drawn as banded fills exactly as contours are (4.33 / 4.79); ROAD
`#B23A22` = routes and actions, the one primary action and failure (4.61 / 5.09). DARK
is made of a documented real thing rather than taste: the **IHO S-52 night colour
table** that ships' ECDIS displays use so a chart can be read on a bridge without
destroying night vision. Ground `#0D1214` (cool teal-black, not neutral), sheet
`#1E262A` (1.23:1), chart white `#D6DAD6` (13.34:1), grid `#333E43` (1.72:1), and every
lexical ink re-tuned to hold its meaning at night: water `#5A93AB` (5.56 / 4.54), wood
`#6E9560` (5.50 / 4.49), contour `#B0793F` (5.08 / 4.14), road `#D2604A` (4.95 / 4.04).

**focus.** The neatline itself, plus an echo in the margin. Unfocused: a single 1px
frame. Focused: the full neatline — 2px outer rule, 3px of paper, 1px inner rule,
exactly how a map sheet is bordered. Simultaneously four 6px heavy ticks appear in the
top and left aprons marking the focused window's left, right, top and bottom edges, like
the coordinate readout on a drafting machine. That echo is the glance signal: the ticks
jump in peripheral vision when focus changes, so the cue does not require reading the
window you are not looking at. Nothing inverts and no hue moves — the five inks are
lexical and may never be spent on state.

**not default because.** Three things a prompt will not produce. (1) Chrome on all four
edges. All four existing directions independently chose one left rail; a perimeter apron
is the only other answer with real precedent, and it changes what the desktop *is* — a
bounded sheet with an outside, rather than a field with a sidebar. (2) A legend that is
the launcher. A key is the one launcher shape that gets *more* useful as apps multiply,
because a key's job is to explain a sheet you have never seen, whereas a dock's job is to
hold seven things you already know. It is the only direction here with a real answer to
53 apps: the key scrolls and stays alphabetical, where a rail of 53 numbered rows does
not. (3) Colour with a written contract on screen. Five inks is more than the current
desktop uses and is nonetheless the anti-generic move, because each is defined in the key
and none may carry state, be set as text, or be chosen for mood.

**weakest, in its own words.** The apron costs 24dp on three sides and 96dp on the
fourth, permanently — roughly 13% of a 1200-tall screen gone before a single window
opens — and it is worse than PLATE's rail in one specific way: a rail can collapse to a
strip below a width breakpoint, but the apron cannot be collapsed without deleting the
key, and the key is the entire legal justification for the five inks. Cut it and Neatline
becomes a polychrome desktop with no contract, which is precisely the thing it claims not
to be. Second, unresolved: because colour is doing referent work, it cannot also do
identity work, so the key's symbols must be distinguishable by **silhouette alone** at
24dp under the monochrome geometric-icon grammar.

## 3. Blueline — **CHOSEN**

**pitch.** The desktop is an engineering drawing — one hue and no black at all, every
piece of persistent chrome collapsed into a bottom-right title block, and focus carried
by **line type**: present windows drawn continuous, unfocused in hidden line, minimised
in phantom.

**lineage.** From PLATE: hard rules as the whole depth system, radius 0, tracked caps,
chrome that prints the machine's own state, and one second ink with a single meaning.
From Folio: the refusal to let hue carry distinction — Blueline goes further and uses
**zero black** in either mode. Breaks three. (1) The ground is a saturated blue, neither
paper nor near-black, making this the dark-primary member of the family, which nothing
else here is. (2) All persistent chrome is one 320×180dp corner block, deleting the rail,
the bands and the apron alike — the most aggressive chrome deletion of the three.
(3) Focus is not value, weight or lit edge but **line type**, the one channel none of the
four existing directions uses.

**structure.** One 2px border rule, a 16dp margin, otherwise edge-to-edge drawing area.
Bottom-right: **the title block**, ruled into cells exactly as a drawing's is — SCALE
(the measured ui scale and module), SHEET, REV, DATE — plus the **view index**, a
numbered list of open windows with their view labels, which is the tasklist and the
launcher. Stacked directly above it, and this is the move nothing else here has: **the
revision table**, five rows of REV / VIEW / DESCRIPTION / TIME recording the last five
state changes the WM made. The session's history becomes a permanent readable part of
the sheet, which is what a drawing does and what no desktop does. Windows are **views**:
each carries a 24dp view bubble with its number, and a dialog or child window is joined
to its parent by an orthogonal dogleg leader — orthogonal because `fb_line` has no
dashed-diagonal path, which is also the correct drafting convention, so the constraint
and the artifact agree.

**palette.** Both modes, one hue in each, no black anywhere in the system.
DARK-PRIMARY, *the blueprint / cyanotype*, flat with no gradient: ground `#16375E` (real
Prussian-blue cyanotype, not a tinted near-black), window sheet `#1D4470` (1.21:1 —
deliberately below threshold, because separation is line and never value), linework and
text `#EDF1F3` (10.61:1 on ground, 8.74:1 on sheet), secondary line `#9FB6CC` (5.76:1),
HIDDEN line `#7E9CB8` (4.21 / 3.47), and the view bubble as a knockout — a solid
`#EDF1F3` disc with its number in `#16375E` at 10.61:1. LIGHT, *the diazo / whiteprint*,
the historical successor to the blueprint and therefore a real second ladder rather than
an inversion: buff ground `#F0ECE0`, sheet `#F7F4EA` (1.07:1), full-density blueline
`#27427D` (8.22 / 8.83), secondary `#5C74A6` (3.95:1), hidden `#7286B4` (3.07 / 3.30).
The one second ink in both modes is **the redline**, the correction a human adds by hand
over a print: `#B23A22` light (5.05:1), `#F07A5F` dark (4.39 / 3.62), spent on exactly
three things — revision balloons, a failure mark, and the delta between a dragged
window's current and snapped geometry.

**focus.** Line type, four states, all glanceable and all surviving a monochrome print.
FOCUSED: continuous line, 2px, full-value linework, view bubble filled solid with the
number knocked out. UNFOCUSED: hidden line — 1px dashed, 8dp dash / 4dp gap, in the
hidden-line value (4.21:1 dark, 3.07:1 light), bubble drawn as outline. CHILD or DIALOG:
chain-dot, long-short-long, the drafting mark for a reference. MINIMISED: phantom — the
window does not vanish, its outline stays at its last position dashed at 25% alpha with
only its bubble, so "where did it go" is answered without a dock. Solid-2px versus
dashed-1px is a larger difference at arm's length than any hue change, and it costs no
new primitive: every window frame is axis-aligned, so a dashed rectangle is a loop of
`fb_fill_px`.

**not default because.** (1) It is the only proposal in the set that uses neither black
nor paper. A generic prompt reaches for near-black-plus-accent, or once corrected for
warm-paper-plus-ink; a saturated blue ground with white linework is neither, and its
light mode is monochrome blue rather than black-on-buff, so the family resemblance to
PLATE is structural rather than chromatic. (2) Focus by line type. Every direction judged
so far spends focus on value (PLATE's knockout), weight (Folio's rule) or lit edge
(Raking). Line type is unclaimed, free, and the only channel that additionally encodes
**relationship** (chain-dot means this belongs to that) and **absence** (phantom means
minimised but still here). (3) The revision table. A desktop that keeps a printed
revision history in its chrome is not something prompts produce, and it is directly
useful in this repo, where "what changed and when" is the recurring question.

**weakest, in its own words.** The blueprint ground is the closest anything here comes to
the banned near-black-plus-accent pattern, and the defence is structural rather than
obvious: it is a saturated blue that is the *paper*, flat, with white linework as the
body, not a dark neutral with a chromatic accent bolted on. Anyone judging from a
thumbnail will misfile it — a real risk to the direction rather than to the design.
Second and more concrete: the entire separation budget is spent on 1–2px lines, so it
stands or falls on `fb_line` and on the dashed-rectangle loop landing on exact integer
pixel positions at both ui scales. A dash phase that drifts one pixel per edge turns a
hidden-line frame into visible noise, and with surface steps at 1.21:1 dark and 1.07:1
light there is no value fallback underneath. That is the same bet Raking lost, taken with
more contrast in the lines and none at all in the surfaces.

---

# Lane: page

## 4. Deck — **CHOSEN**

**pitch.** The desktop is a front page that has been **made up**: no window frames at
all, stories abutting across shared rules, and a real newspaper hierarchy of kicker /
head / deck where focus means the deck line gets set.

**lineage.** Takes from Folio: warm paper ground, square corners, hard rules, no shadows,
three atlas sizes exactly, colour makes no distinction. Takes from PLATE: the
register/index as a numbered list that is simultaneously launcher, tasklist and Alt+N
keymap. **Breaks the one thing PLATE and Folio agree on hardest — the left rail is
deleted.** A newspaper has no left rail; it has a masthead at the head and a folio line
at the foot. That buys back the 320 device px Folio's 160dp rail costs permanently (its
own spec calls this out: 17% of a 1920 screen, 23% on a 1366 laptop). Also breaks the
free-window model: a page is composed, not stacked, so stories occupy whole columns and
whole bands and there are no gutters — adjacent stories share one hairline the way a
broadsheet does.

**structure.** Persistent chrome moves to two horizontal typographic bands and there is
nothing else. MASTHEAD, 44dp, top: wordmark R4 caps tracked +4, a 2px INK_0 rule under
it, then one line of R1 caps tracked — `SAT 25 AUG · EDITION 0.3 · INSIDE: 1 SHELL
2 FILES 3 MONITOR 4 EDITOR 5 LOG`. That INSIDE line **is** the tasklist, the launcher and
the keymap; click a name to raise, `INDEX` opens the full-page app index. No tray, no
menus, no clock widget, no click target that is not a set word. FOLIO LINE, 24dp, foot:
colophon flush left in R1 caps, live stats centred in mono, workspace numeral flush right
in R4. Between them, **the page**: 12 columns × 9 bands, stories snapped to whole
modules, abutting, shared rules, zero desk showing. Windows have no frame — a story is
bounded by the rules it shares with its neighbours. Opening or closing a window **re-makes
the page**: the new story takes the lead position, everything else demotes one rank.
There is a `HOLD` lock in the folio line that freezes make-up.

**palette.** Both modes. Light is **newsprint**, not book paper — greyer and cooler than
Folio's `#E9E4DB`, because newsprint is a groundwood buff and ink on it never reaches
black. PAGE `#D9D5C9` (L* 85.3, the only ground; there are no sheets), BOX `#CDC8B9`
(the tint block: sidebars, table heads, selected rows), PATCH `#E8E5DC` (the pasted-in
patch: input wells, code, terminal). Ink INK_0 `#19170F` (12.22:1 on page), INK_1
`#2B2820` body (10.03:1), INK_2 `#4C4739` (6.31:1), INK_3 `#585240` (5.31:1 on page and
4.66:1 on BOX — picked at that value specifically because BOX is the darkest text-bearing
surface, Folio's own rule). RULE_HAIR `#B5AF9D` (1.49:1) for column rules, RULE_CUT
`#7A7462` (3.18:1) for the cut-off rule that ends a story, 2px INK_0 for a region
boundary. **Zero chromatic ink** — Deck spends less colour than Folio, which still keeps
three state hues; here state is the *word* in R1 caps and nothing else. DARK IS THE
COMPOSING STONE, and that is what it is made of: the page is made up on a slate stone and
the marks are **type metal**, not ink — so dark Deck is cooler and slightly brighter than
light Deck is warm, the exact opposite move to a tint-inversion. STONE `#1A1C1A` (L* 10.0,
cool slate), BOX `#262825`, PATCH `#121413`. METAL_0 `#E9E7DF` (13.85:1 on stone),
METAL_1 `#D2CFC5` (11.0:1), METAL_2 `#ABA79B` (7.13:1), METAL_3 `#989486` (5.65:1 on
stone, 4.90:1 on BOX). HAIR `#494B46` (1.94:1, dL*-matched to light's 1.49 rather than
ratio-matched), CUT `#6C6E67` (3.32:1). Metal is capped below paper-white for the same
reason Folio capped ink-0: a full-white body of marks blooms.

**focus.** **The deck line gets set.** Every story reserves three head elements: KICKER
(R1 caps tracked, always set), HEAD (R3 bold), DECK (R2, 24dp of reserved air).
Unfocused: kicker + head, deck air left blank, head rule 1px RULE_HAIR. Focused, three
things move at once — (1) the head rule above the story goes 1px hair → 3px INK_0 across
the story's full measure, a 1.49:1 → 12.22:1 jump; (2) the head steps R3 → R4 and goes
caps at +4 tracking, one real atlas step, 24 → 32 device px, no resampling; (3) a whole
**line of type appears** in the reserved 24dp — the deck, a one-line statement of what
the window is currently doing (`/kernel · 22 items · fb.c selected`). At arm's length the
appearing line is the loudest of the three, because a line of text materialising is a
change of ink *area*, not a change of value. Crucially the 24dp is **reserved in every
story** whether set or not, so focus fills air and never moves geometry — no reflow, no
re-wrap, the terminal does not lose a row.

**not default because.** A generic prompt cannot produce a layout with **no window
frames**, because "window" retrieves "box with a title bar". It cannot produce hierarchy
carried by rank rather than by z-order, and it cannot produce make-up — the idea that
opening a window recomposes the whole page is the opposite of every WM ever shipped. It
also inverts the family's own consensus in a motivated way rather than a contrarian one:
the rail dies because a newspaper's identity lives in a masthead, and the 320px it frees
is measurable. And it is the only one of the six lanes where hierarchy is genuinely
**unequal** — Folio and PLATE both give every window the same chrome; Deck says out loud
that one of them is the lead story.

**weakest, in its own words.** Two, and the first is severe. (1) **The masthead is one
bad implementation away from being a top bar** — the exact thing all four existing
directions independently deleted. The only defence is discipline: it carries set type and
a rule, no tray, no menus, no icons, no popovers. If anyone adds a status icon to it, the
direction is dead and it dies quietly. (2) Page re-make on open/close moves every window
on screen. That is the whole point of the metaphor and it is also genuinely disruptive —
you open a file browser and your terminal jumps. The `HOLD` lock is a hedge, and a hedge
is an admission. Third, smaller: with no frames and no gutters, two adjacent stories are
separated by one 1.49:1 hairline, so Deck inherits Folio's low-contrast-boundary problem
in the ADJACENT case even though it dodges the OVERLAP case entirely.

## 5. Apparatus — **CHOSEN**

**pitch.** Every window is a scholarly text with a critical apparatus: numbered lines in
the outer margin, the machine's commentary set at the foot in the smallest register with
lemma references, and focus means the apparatus is open.

**lineage.** Takes from Folio: the left rail, the sheet-on-desk model, square corners,
warm monochrome, colour-carries-nothing, and the two-tone mount as a physical edge. Takes
from PLATE nothing structural — deliberately. Breaks Folio's flat, hierarchy-free window
chrome by splitting every sheet into two settings with different jobs: the **text block**
is the user's, the **apparatus** is the machine's. That is Datum's paper/ink law, which
was the most original idea in the four and got the worst score for buildability —
Apparatus does it typographically (a register change and a rule) instead of materially (a
luminance law), so it costs a rule and 28dp instead of a second surface system. It also
breaks the family's convention that keyline weight is a constant property of a sheet:
here the edge is drawn by whoever is on top.

**structure.** The rail stays and becomes the **sigla register** — 128dp, each open sheet
gets a one-letter siglum (A, B, C…) set in R4 bold in a 24dp gutter, then the title in
R2, and the register is ordered **top of stack first**. The desk is Folio's drawn column
grid, unchanged. Every SHEET is divided by two horizontal rules into three regions: (1) a
28dp running head — title R3 focused / R2 INK_3 unfocused, siglum flush right; (2) the
**text block**, with a 28dp outer margin column on its right carrying line numbers every
5 in mono R1, right-aligned, tabular; (3) the **apparatus**, a 28dp-minimum band at the
foot on AP_APP stock, opened by a 2px INK_0 rule, holding the machine's commentary in R1
with printed lemma form — `14 fb_rrect ] r<=0 → fb_fill_px · 22:41:07`, `31 open ]
ENOENT · /kernel/missing.c`. Errors, paths, PIDs, timings and warnings live there and
**nowhere else**: no toast, no status bar, no inline red. The apparatus is the only place
the machine speaks, and it always speaks in the same register.

**palette.** Both modes. Light is a **white wove** — the highest-contrast paper in the
family, because an academic press prints on good stock, not on cream. PAPER `#EFEBE1`
(L* 93.1, the sheet), DESK `#E3DED1` (the reading table), APP `#E8E3D6` (the apparatus
band, one rung down from the sheet so the setting change is visible before you read a
word), WELL `#F7F5EE`. INK_0 `#100E0A` (16.20:1 on the sheet — a true near-black, deeper
than Folio's `#17150F`), INK_1 `#25221B` (13.33:1), INK_2 `#474134` (8.51:1), INK_3
`#5B5445` (5.86:1 on APP, the darkest text-bearing surface). HAIR `#BEB6A3` (1.69:1). No
chromatic ink at all: an error is the word `err` in the apparatus, in the same R1 as
everything else, distinguished by the lemma and the position — which is exactly how a
real *apparatus criticus* distinguishes a variant from a conjecture. DARK IS IRON GALL
INK, and the ground is the ink itself: the page is what is left where the ink is not. So
dark Apparatus inverts the **hue temperature** as well as the value — a cool
blue-green-black ground carrying warm bone-coloured marks, where Folio's dark is warm
ground / warm marks. GROUND `#0F1412` (L* 5.8), SHEET `#1A211E` (L* 12.0), APP band
`#131A17`, WELL `#232B27`. PARCH_0 `#EDE6D6` (13.19:1 on the sheet), PARCH_1 `#D8CFBC`
(10.60:1), PARCH_2 `#B0A793` (6.87:1), PARCH_3 `#9B9280` (4.71:1 on WELL, the lightest
text-bearing surface — the mirror of the light-mode rule). HAIR `#3E4844` (1.73:1).

**focus.** **The apparatus opens**, and two whole regions of type appear at once.
Unfocused: the apparatus is closed to a single 1px hair with a right-flush mono count
`⟨3⟩`, and the outer margin column is blank. Focused: the hair becomes a 2px INK_0 rule
(1.69:1 → 16.20:1), three lines of apparatus are set below it, **and** the 28dp outer
margin fills with line numbers every 5 in tabular mono. Plus the running head title steps
R2 INK_3 → R3 bold INK_0, and the sigla register's letter for that sheet goes R4 bold
with a 3dp INK_0 marginal bar. Four changes, none of them a hue, and two of them are ink
appearing where there was air — the most detectable class of change at arm's length. Both
regions are **reserved at all times** (28dp foot + 28dp margin), so opening the apparatus
reflows nothing.

**And the stacking fix — THE CUT EDGE**, which is the real answer to the reviewer's
complaint that three same-value sheets separated by a 1.71:1 hairline take a beat to
rank. Sheets keep Folio's quiet two-tone mount on their *free* edges. But where sheet A
occludes sheet B, A draws a 2px INK_0 rule along exactly the segments of **its own** outer
boundary that lie over another sheet — nowhere else. **The border you can see is drawn by
the occluder, never by the occluded.** An isolated sheet stays quiet at 1.69:1; the
instant anything overlaps, the contact boundary is 16.20:1 black and it unambiguously
belongs to the sheet on top. Three stacked sheets read as three progressively inset black
L-shapes — a card stack, at a glance, with no shadow, no hue, no value step. Cost: `wm.c`
intersects each sheet's four border rects against the union of the rects below it,
integer arithmetic on a z-ordered rect list it already keeps for damage, then
`fb_fill_px`. No new primitive, no blur, no per-frame `fb_shadow`. Backstop is the sigla
register, ordered top-of-stack first, so reading order down the margin **is** the z-order:
`A B C` means A is on top.

**not default because.** No generic prompt produces a window with a line-number margin
and a footnote apparatus, because that grammar comes from a printed critical edition and
nothing else. More importantly it is the only proposal here that gives the machine its own
**register** rather than its own **colour** — every desktop ever built says "error" in
red; this one says it in the same 8dp type as everything else, at the foot, keyed to a
line number, and it is more findable that way because it is always in the same place. The
cut edge is also a genuinely new answer to a problem the family has: PLATE avoids overlap
by tiling, Folio admits its overlap case is weak, Raking spends its whole budget on 1px
runs. This spends nothing until two things actually touch.

**weakest, in its own words.** The cut edge has a real blind spot and I will not hide it:
a sheet **entirely inside** another sheet's rect draws no contact edge, so the visual
mechanism produces nothing and the whole burden falls on the sigla register in the rail.
That is an ordinal cue, not a spatial one, and it is a beat slower than a shadow would be.
Second: this direction is **expensive in pixels** — 28dp of apparatus plus 28dp of
line-number margin is ~56dp of every window handed to machine text, which on a 4-module-wide
sheet is a large fraction of the window. Third, and this is the implementation risk: the
cut edge is computed geometry drawn every frame, and a bug in `wm.c`'s occlusion segments
produces 2px black lines in the wrong places at 16:1 contrast — the loudest possible
failure mode. It needs a hosttest that renders three known-overlapping rects and asserts
the black segments against a golden, before it is trusted.

> **Reviewer's note — the stated blind spot does not exist.** Take the case as written:
> sheet A is on top and lies entirely inside sheet B's rect. The rule is "A draws 2px on
> the segments of its own boundary that lie over another sheet". A's *entire* boundary
> lies over B, so all four of A's edges are drawn at 16.20:1 — that is the loudest case
> the mechanism has, not its silent one. The genuinely silent case is the reverse (a lower
> sheet fully covered by a higher one), and a fully covered sheet has nothing to
> disambiguate. The caveat should be deleted, and the confidence it was costing spent on
> the third weakness instead, which is the real one.

## 6. Bradshaw — **KILLED**

**pitch.** The desktop is one continuous ruled timetable covering the whole screen —
windows are full-height service columns that abut and share rules, every app's rows land
on the same horizontal ruling, and focus is two full-height black bracket rules.

**lineage.** Takes from Folio: warm paper, square corners, colour-carries-nothing, the
left rail (reinterpreted as the **stub column**, which is what a timetable calls the
leftmost column that names the rows), tabular mono figures as an editorial device, and the
three-size type scale landing exactly on the atlases. Breaks the sheet entirely — there is
no sheet, no desk, no gutter, no mount, no shadow and no overlap. Folio and PLATE both keep
windows as objects laid *on* a ground; Bradshaw says the ground **is** the object and
windows are regions of it. It also breaks the family's tacit rule that each window owns its
own vertical rhythm: here one horizontal ruling crosses every column, so a row in the shell
is on the same baseline as a row in the file list.

**structure.** One table, edge to edge, no margins. STUB COLUMN 96dp at the left: the
numbered app register (R1 caps tracked, mono index) top, marginalia at the foot (clock,
CPU, MEM, CPUs — labels flush left R1, values right-flush in tabular mono). Right of it,
N SERVICE COLUMNS, full height, variable width, abutting; each column is one window. A
32dp COLUMN HEAD band across the top of the table carries each column's name in R1 caps
tracked plus its mono column number. A 24dp FOOT band carries the totals row — per-window
CPU and RSS in mono, right-aligned on a shared decimal column, because a totals row is what
a timetable is for. **The horizontal ruling is the whole claim**: a 1px hairline every 32dp
runs unbroken across the entire table, through every column. 32dp = 64 device px at ui2 =
exactly two mono cells (the mono cell is 16×32 device px, so a mono line is 16dp), which is
why the rhythm is 32 and not 28 — it is derived from the atlas, not chosen. Terminal lines
land on every half-rule; list rows land on every rule. Resize is dragging a column boundary,
which is now a real grabbable object rather than an invisible hit zone. Menus and dialogs
are **notice rows** — a full-table-width band inserted between two rulings with a 2px INK_0
rule above and below, pushing rows down, exactly as a printed timetable prints `SERVICE
ALTERED`.

**palette.** Both modes, and the dark one is the point of the direction. Light is
**timetable stock** — thin, cheap, faintly pink-buff, warmer and pinker than Folio's neutral
cream. TABLE `#EDE7DC` (L* 91.8, the ground, and there is nothing else behind it), BAND
`#E1D9CB` (the zebra, alternate 32dp rows — a timetable's own device and the reason long
tables stay scannable), STUB `#E7E0D2`, WELL `#F8F5ED`. INK_0 `#14120E` (15.20:1 on the
table), INK_1 `#29251D` (12.40:1), INK_2 `#4A4436` (7.86:1), INK_3 `#5D5646` (5.20:1 on
BAND, the darkest text-bearing surface). HAIR `#C0B7A3` (1.62:1) for the ruling; 2px INK_0
for a bracket or a notice. No chromatic ink. DARK IS A CYANOTYPE — the working timetable,
the blueprint. Engineering and operating schedules were reproduced as blueprints, so dark
Bradshaw is a prussian-blue ground with the paper showing through as white lines. TABLE
`#10243D` (L* 13.9), BAND `#16304F`, STUB `#0C1C31`, WELL `#0A1728`. LINE_0 `#EEF1F4`
(13.81:1 on the table), LINE_1 `#D3DBE3` (11.19:1), LINE_2 `#A6B4C2` (7.40:1), LINE_3
`#8DA0B2` (4.97:1 on BAND, the lightest text-bearing surface). HAIR `#3D5670` (2.06:1,
dL*-matched to light's 1.62 rather than ratio-matched). **The defence of the blue**, stated
up front because it is the obvious objection: Folio's law is that colour makes no
*distinction*, not that colour is absent. Every surface, every rule and every mark in dark
Bradshaw is one hue; nothing anywhere is distinguished from anything else by hue, exactly as
light Bradshaw's warm buff distinguishes nothing.

**focus.** **The bracket.** Two 2px INK_0 vertical rules, full screen height, floor to
ceiling, through the head band and the foot band, at the focused column's two boundaries.
Every other column boundary is a 1px HAIR. That is 1.62:1 → 15.20:1 on two lines each 1200
device px long, which is the largest single mark any of these proposals puts on screen and
the cheapest — two `fb_fill_px` calls, no clip, no blur, no state. Secondary cues, both
free: the focused column's head goes R1 caps tracked → R3 bold caps, and its stub-register
row grows a 3dp INK_0 marginal bar. On the reviewer's complaint about stacking ambiguity:
Bradshaw **dissolves** it rather than solving it — there is no overlap, so three same-value
sheets cannot stack. That is an answer by deletion and I would not claim it as a fix.

**not default because.** Nothing in the space of "desktop UI" produces a screen with no
gutters, no margins, no sheets and no visible ground — a fully set page where every pixel
belongs to the table. And no other direction here, including PLATE and Folio, delivers
cross-window horizontal alignment: in Bradshaw a row in the process list is on the same
baseline as a row in the file list, which is the one thing a timetable does that no desktop
does. The cyanotype dark mode is also the only dark ground in the family that is neither a
photographic film (PLATE), a darker paper (Folio), nor a stock near-black.

**weakest, in its own words.** The 32dp cross-table ruling **forces all eight apps onto one
row rhythm**, and it changes `theme.row_h`, which is `dp(28, scale_q8)` at `ui.c:145` today
and which Folio explicitly preserved. 28 → 32 is a one-line change in `ui.c` and a
re-layout of every list, table and toolbar in `kernel.zl`, and any app that cannot sit on
32dp breaks the direction's entire claim rather than just looking slightly off. Second:
full-height columns mean a window can never be short, so a 3-line dialog either becomes a
notice row (fine) or wastes a full-height column (not fine), and I have not solved the case
of a genuinely small utility window. Third: no free-floating windows at all — no
drag-anywhere, no picture-in-picture, no window dragged half off-screen. Fourth, unverified
and the thing I would test first: the subpixel LCD text pipeline's 5-tap (1,2,3,2,1)/9 FIR
was tuned on near-black, and light marks on a *saturated blue* ground is the case furthest
from that tuning in the whole family — the fringe could go visibly warm. That is a
wmshot-and-look-under-a-loupe item, not an argument, and I have not run it.

---

# Lane: merge (PLATE × Folio)

## 7. GALLEY — **KILLED**

**pitch.** PLATE's paper and knockout, with the vermilion cut from four jobs down to one —
it marks the live edge, the single place on screen taking your next keystroke, and nothing
else.

**lineage.** Takes PLATE's warm paper stock, 2px ink rules, numbered register rail, ruled
desk module grid and header knockout wholesale. Takes Folio's discipline about what colour
is permitted to *mean*, but not its ban. Breaks PLATE by deleting three of its four accent
jobs — no vermilion primary-action fill, no vermilion register mark terminating the slug, no
vermilion crop marks at the field corners. Breaks Folio by refusing the absolute law
outright. What it **loses** from PLATE: the field is now entirely achromatic, so PLATE's one
warm event at the desk corners is gone and the wallpaper is duller; and "the one primary
action per view" loses its cheapest emphasis and must become a second knockout plate, which
puts two ink plates on one sheet competing with the focused header. What it **loses** from
Folio: the law itself, and with it Folio's free colour-blindness argument.

**structure.** PLATE's rail/slug/field is unchanged — 148dp left register rail carrying
identity, numbered app rows and the pinned readout, replacing dock, top bar and start menu;
windows tile to the 12×8 module; square corners; 2px ink frame; no shadows. The merge's move
is to **split a signal PLATE conflates**: WINDOW FOCUS (which sheet is active) is the ink
knockout, and INPUT LOCUS (which control consumes the next keystroke) is the vermilion. The
accent's law is a **cardinality** law, not a budget: exactly one accent region exists on
screen at any instant — never zero, never two — and it is never wider than 3 design px, so
vermilion never fills an area. Forbidden to it: status, severity, selection, category,
branding, icons, primary actions, hover, and any use where a word or a value would carry the
same information. Gateable in one assertion: the shell emits exactly one accent-coloured
rect per frame and its width is `<= UI_DP(3)`.

**palette.** Both modes. Light is PLATE's stock unmodified: paper `#F2EFE6`, desk `#E4E0D5`,
well `#D6D0C0`, ink/2px-rule `#14120E`, body `#26231D`, label `#5C5646`, printed grid
`#A69E8A` (2.02:1 on desk), accent vermilion `#C4341C` (4.74:1 on paper). The dark ground is
made of a galley pulled in **opaque white ink on black newsprint** — the mark is what was
added, not what remains: field `#15130E`, sheet `#201D16`, well `#2C271F`, rail plate
`#0A0906`. Marks `#EDE7D8` (13.63:1 on the sheet, 16.14:1 knocked out on the plate), body
`#D8D1C0` (11.05:1), secondary `#A79E8C` (6.34:1 on sheet, 5.58:1 on well), hairline
`#4E4739` (1.83:1), 2px rule `#A99F8D` (6.43:1). Accent lightens to `#E0684B` — 5.01:1 on
the sheet, 5.53:1 on the field — and is a graphic mark only, never text.

**focus.** Two independent signals, on purpose. WINDOW FOCUS: the 28dp header flips ground
from `#F2EFE6` to `#14120E` with the title reversed out in tracked caps — a full value
inversion 12.95:1 apart, inherited from PLATE unchanged. INPUT LOCUS: a 3dp vermilion bar
down the left inner edge of whatever owns the keyboard, plus a 2dp caret in the same ink,
hard-blinking at 500ms/50% duty, no opacity ramp. Because colour must never be the sole
carrier of anything, the locus bar is redundantly cut: a 4dp notch is taken out of the
sheet's 2px frame at the bar's top and bottom ends, so under full achromatopsia the locus is
still readable as geometry.

**not default because.** A generic prompt spends its accent on hierarchy, brand and status
simultaneously — that is what "accent colour" means to a generator. This spends it on none
of those three. The strict form (a hard cardinality of exactly one accent region per frame,
enforced by a gate rather than a guideline) is not something a style prompt produces, and it
is a measurable improvement on the parent: PLATE claims one accent and actually spends it on
four semantically unrelated jobs.

**weakest, in its own words.** The strictness that makes the law honest also makes the mark
nearly unfindable. 3 design px is 6 device px at ui=2; on a 2560×1440 panel that is a
hairline, and this is precisely the failure the repo already recorded against Raking in its
own implementer's words. Second: focus-versus-locus is a distinction terminal users hold and
most people do not. If the two coincide in the overwhelming majority of frames, users will
read the vermilion as redundant with the knockout, and a rationed accent that is *perceived*
as redundant is decoration carrying extra rules.

## 8. STIPPLE — **KILLED**

**pitch.** PLATE's entire structure with the second ink replaced by ink **coverage** — the
run mark, the desk grid and every rank below solid black are ordered screens of the one ink,
so the desk is textured rather than tinted.

**lineage.** Takes PLATE's numbered register rail, ruled desk grid, tiling module, square
corners and header knockout **unmodified**. Takes Folio's law that colour never
distinguishes, absolutely. What it **loses** from PLATE: vermilion outright, and with it the
only warm event on a neutral screen — PLATE gives your eye one thing to jump to on entry and
STIPPLE gives it none. It also loses PLATE's clean solid-versus-paper binary at the run mark,
because the 50% screen is a third state and three states of one mark is measurably harder to
learn than two. What it **loses** from Folio: Folio's central claim that every distinction is
made with weight, case, scale, rule-thickness and whitespace — STIPPLE adds a sixth channel
Folio deliberately refused, texture, and texture is the least robust of the six. It also
deletes Folio's one gradient, because a screened mark sitting over a dithered ramp beats.

**structure.** The accent is replaced by **screen percentage** — the historically correct
answer when you cannot afford a second ink. One ink, four coverages, and coverage *is* rank:
100% solid = focused/active, 50% = running but not focused, 25% = structure of the desk, 0% =
idle. The 3dp run bar on a rail row and on a window's left inner edge is solid ink when
focused, a 50% screen when merely running, absent when not. The desk's module grid is not a
mixed grey: it is a 1px rule of **solid** ink at 50% duty, every other pixel, so the grid line
is pure `#14120E` dots and survives any panel gamma. Hard rule with a gate: no text ever sits
on a screen above 25%. Consequence worth the whole candidate — this makes `fb.c`'s ordered
dither a load-bearing shell caller, which fixes PLATE's own stated #1 rot risk
(`fb_gradient`'s dither losing its last desktop caller).

**palette.** Both modes. Light is PLATE's stock with the chromatic row **deleted** from
`design.h` rather than left unused: paper `#F2EFE6`, desk `#E4E0D5`, well `#D6D0C0`, ink
`#14120E`, body `#26231D`, label `#5C5646`. Screen ladder measured against paper, computed in
linear light through the existing gamma LUTs: 100% = 16.27:1, 50% = 1.884:1, 25% = 1.307:1,
12.5% = 1.133:1; on desk, 50% = 1.868:1 against PLATE's reference grid line at 2.021:1. The
dark ground is made of the same forme at full ink coverage — the stock only shows where a
screen opens it: field `#12100B`, sheet `#221E17` (1.15:1 off the field), marks `#EAE3D3`
(12.97:1), secondary `#B4AA97` (7.22:1). **Measured asymmetry, and it is the real cost:** in
dark a screen is stock showing through ink, and 50% coverage gives 6.99:1 against the sheet
where light's 50% gives 1.88:1 — a screen that is a whisper on paper is a lamp on ink.
Perceptual parity in dark needs about 7.5% and 2.5% coverage; a 4×4 ordered matrix quantises
at 6.25% and **cannot express the lighter step at all**. Dark therefore requires the threshold
matrix to go 4×4 → 8×8.

**focus.** Three states, all made of one ink, no hue anywhere. FOCUSED: the 28dp header knocks
out to solid `#14120E` with the title reversed in `#F2EFE6` (12.95:1 from the unfocused paper
header), plus a solid 3dp ink bar down the full left inner edge. RUNNING BUT UNFOCUSED: paper
header, `#5C574C` title, 2px ink rule beneath, and the 3dp left bar drops to a 50% screen — at
pixel scale that reads as a dotted bar of pure ink, not as a grey bar. NOT RUNNING: no bar at
all. Nothing fades, nothing pulses, nothing tints.

**not default because.** No generator reaches for tint screens, because a screen is a printing
constraint and not a screen-UI idiom. It obeys Folio's law without inventing a distinction
channel out of nothing, and it converts the repo's most-likely-to-rot primitive into the one
thing the shell cannot draw a frame without. It also structurally resists the failure that
produced the current desktop — there is nothing to "just add a hue" to later.

**weakest, in its own words.** The whole candidate rests on a 4×4 dither cell reading as
**texture** at the real pixel pitch, and I have not verified that it does — this is a
prediction, not a measurement. On a 14-inch 2560×1440 panel a 4×4 cell is roughly 0.19 mm; at
arm's length that is at or below the eye's resolving limit, at which point a 50% screen
collapses into a flat grey and STIPPLE degrades into PLATE-with-one-more-grey. **It does not
fail loudly — it fails by becoming ordinary**, which is the worst failure mode available here.

## 9. SPREAD — **CHOSEN**

**pitch.** Folio's 12-column page with PLATE's knockout lifted off the window entirely and
hoisted into a running head shared by the whole screen — one head, one baseline, one page.

**lineage.** Takes Folio's 12-column grid, square-cornered sheets, left margin rail, three-rank
keyline system and its refusal of an accent. Takes PLATE's knockout as the focus mechanism and
its willingness to tile. What it **loses** from PLATE: the per-window header plate, deleted —
a window has no title bar at all, so the focus signal is no longer adjacent to the content it
describes; and the numbered register rail loses its primacy, shrinking because the running head
now carries the tasklist (Alt+NN still works). What it **loses** from Folio, harder: Folio is
explicit that it is a stacking WM with printer's guides and not a tiler — free drag, legal
overlap. A shared running head cannot exist over overlapping windows, because two windows in
one column range would demand the same head segment. So overlap goes and SPREAD is a tiler;
Folio's freedom is the price. Folio also loses its per-window subtitle/path line, which moves
to a foot rule inside the sheet, and its arbitrary 20dp internal padding, which gets quantised
to the baseline.

**structure.** Two things neither parent has. (1) **The running head** — a 32dp band ruled
across the top margin of the type area, divided at column boundaries, where each open window
owns exactly the segment spanning its own columns and its title is set there in tracked caps.
There is **one header for the entire screen and its divisions are the column layout**. A 1px
leader rule drops from each segment boundary down the desk to the sheet's top edge, tying head
to sheet at distance. (2) **The shared baseline** — a 24dp baseline grid ruled across the whole
type area, and every window's body text sits on it, so a line in one window is level with a
line in the window beside it across the gutter. This is only affordable because text comes from
fixed-height baked atlases (16/24/32 device px): integer, no float, and gateable as "every
shell text origin `y % UI_DP(24) == 0`". **The constraint that cost Datum points is the thing
that makes it exact.** COLOUR takes the third position on the axis: the shell spends none at
all, but colour is unconstrained *inside* a content rect where the colour is the payload —
syntax highlighting, a heat map, the browser rendering a real page. It is a **boundary law**,
not a budget law; the 2px frame is the constitution, and it closes a hole Folio's absolute law
never addresses.

**palette.** Both modes, and its own ladder rather than either parent's. LIGHT: desk `#EDE8DD`,
sheet `#F7F4ED`, well `#FCFAF5`, band `#DFD9CC`, ink0 `#16140E` (16.76:1 on the sheet), body
`#2C2820` (13.36:1), tertiary `#5C5547` (6.72:1 on sheet, 6.04:1 on desk, 5.25:1 on the band —
the band is the darkest text-bearing surface and it is what the token was picked against),
hairline `#C0B8A6` (1.80:1), interactive control rule `#8A8270` (3.47:1, clears WCAG 1.4.11),
knockout `#F7F4ED` on `#16140E` at 16.76:1. **Zero chromatic tokens in the shell.** The dark
ground is made of **carbon paper** — the sheet under the top sheet, where a mark is where the
pressure went through, not where ink was laid: desk `#191510`, sheet `#232019` (1.12:1 off the
desk, so the keyline carries it, exactly as Folio's 1.123 did), marks `#E7E1D1` (12.45:1),
tertiary `#A69D8A` (6.05:1), hairline `#4C4536` (1.71:1). It stays warm deliberately — carbon
is wax on stock, not glass.

**focus.** The focused window's running-head segment knocks out to solid `#16140E` with the
title reversed in `#F7F4ED` — 16.76:1 against the unfocused segment, which is paper ground with
a `#5C5547` title and a 1px hairline beneath. Because the segment spans exactly that window's
columns, **the black bar's left edge, right edge and width ARE the window's column span**, read
off the top of the screen: the focus signal and the geometry readout are one object. That is the
third thing neither parent has — PLATE prints its coordinate readout as text in the header,
Folio does not print it at all. Second, redundant, structural cue: the focused sheet's frame
steps from a 1px `#C0B8A6` hairline to a 2px `#16140E` rule on all four sides and its leader
rule doubles with it. In dark the knockout inverts polarity as carbon does — the segment
*clears* to `#EFE9DA` with the title in `#191510`, 15.00:1 — while the geometry is invariant.

**not default because.** Every desktop, including both parents, puts a window's identity on the
window. Hoisting all identity into one shared running head whose divisions are the layout only
works if windows own disjoint column ranges, which makes it a **merge decision rather than a
style decision** — it is a consequence of taking Folio's grid seriously enough to let it own the
chrome. And a cross-window shared baseline is something no shipping desktop does, because no
shipping desktop controls its own text metrics that tightly. zlOS does, and gets it from the same
bitmap-atlas limitation that a generic proposal would treat purely as a handicap.

**weakest, in its own words.** The head must reflow on every window move, resize, open and close
— and PLATE's own post-mortem records that its single largest structural failure was **having no
reflow story at all**, with four separate defects turning out to be that one failure wearing
different hats. SPREAD takes the parent's worst-understood area and makes continuous reflow
mandatory rather than occasional. Concretely: the tiler has to *guarantee* column disjointness
before the head can be drawn, and every state where it cannot — a modal, a deliberately detached
window, a window mid-drag — needs a defined fallback that this sketch does not have. Second,
smaller but certain rather than speculative: a window at the bottom of a 1200px screen has its
own name roughly 900px away from it, a distance PLATE's per-window plate never paid.

---

# Lane: linux (BeOS / CDE / NeXT re-cut)

## 10. Shingle — **KILLED**

**pitch.** The title bar is not a bar — every window carries an index tab the width of its own
title, so a pile of overlapping windows reads like a card drawer and you find a buried window by
reading it rather than by cycling through it.

**lineage.** Takes from BeOS/Haiku: the tab title bar, the corner Deskbar (menu + running list +
tray in one top-right block rather than a full-width strip), and drag-tab-onto-tab to stack two
windows into one frame with two tabs. Takes from PLATE: warm paper ground, square corners, 2px ink
frames, no shadow, hierarchy by case and weight. Takes from Folio: the running list is set in type,
not tiles. **Deliberately breaks the family's shared axiom that persistent chrome is a full-height
left rail** — Shingle has no rail at all; nearly all chrome rides on the windows themselves, and the
only fixed furniture is a 200dp × 240dp corner block. Also breaks Haiku: no plastic bevel, no
gradient, no 3D. The tab is a flat piece of coloured card stock with a hard 2px contour, and the
contour is continuous with the window's own frame — the tab is a bump in the border, not an object
sitting on it.

**structure.** Chrome lives on the window edge, not on the screen edge. Tab: 28dp tall when focused,
22dp when not, width = title width + 24dp, radius 0, left-flush by default but draggable along the
top edge and snapping to an 8dp step so tabs in a stack fan out instead of colliding. Close is a 12dp
X at the tab's left, zoom and minimise are 12dp geometric cells at its right — BeOS's own left/right
split. Window body: square-cornered sheet, 2px ink frame, no shadow, free-floating and overlapping
(this is a stacking WM, not a tiler — that is what makes the tabs earn their keep). The **Deskbar**
is a 200dp-wide block pinned to the top-right corner only, 2px framed like a mounted card, containing
top-to-bottom: the zlOS mark as the menu button (opens a downward text column, not a grid), the
running-window list as 24dp text rows with a 3dp ink bar on the focused one, and the tray + clock
pinned to its bottom edge.

**palette.** Both modes; the tab is the one element that does not invert. LIGHT — desk `#DED9CD`,
sheet `#F3F0E8`, well `#FAF8F3`, ink `#16150F` (16.06:1 on sheet), ink2 `#4B4539` (8.35:1), ink3
`#5E5749` (6.28:1), hairline `#B9B1A0` (1.87:1). Focused tab stock `#BE7003`: 3.35:1 against the sheet
(clears the 3:1 non-text floor) with its ink title at 4.80:1 on it. Unfocused tab is bare sheet with a
`#5E5749` title and a 1px contour. DARK — the ground is made of the back of the card: kraft chipboard,
warm brown-grey, not near-black. Desk `#201E19`, sheet `#2C2924` (paper `#EDE8DC` at 11.85:1), hairline
`#4A463D`. The tab keeps its hue and keeps its dark ink title in both modes and steps exactly one value,
`#BE7003` → `#E0A62E`: 8.42:1 ink on tab, 7.66:1 tab against the dark desk. States: green `#2C6A38` /
ochre `#7E5305` / red `#A0301B`, always with the word.

**focus.** Four channels, three of them luminance or geometry. (1) The tab fills `#BE7003` card stock;
unfocused tabs are bare sheet. (2) The tab **stands proud**: focused 28dp vs unfocused 22dp, so in a
fanned stack the focused tab is physically 6dp taller and is drawn last, overlapping its neighbours.
(3) The window's whole contour steps 1px `#B9B1A0` → 2px `#16150F`. (4) The title goes `#5E5749`
regular → `#16150F` bold, and the Deskbar's matching text row does the same and grows a 3dp ink bar.

**not default because.** Nothing in the default retrieval set for "desktop" produces a title bar shorter
than its window. Every generic answer puts a full-width strip across the top of each window and then
needs a shadow to tell two overlapping strips apart. Shingle deletes the strip, which deletes the reason
for the shadow, and turns overlap from the problem into the feature. It also refuses the other default,
a full-width taskbar: the Deskbar is a corner block, so three of the four screen edges stay empty.

**weakest, in its own words.** Tab collision at scale, and I can put a number on the colour half of the
focus signal. `#BE7003` against `#F3F0E8` is 3.35:1 — that is a **chroma** signal, not a luminance one,
so on a greyscale render or to a deuteranope the tab fill contributes almost nothing. Worse, the
self-labelling claim inverts exactly when it matters: at 3 windows the tabs fan out and read beautifully;
at 10 they must either overlap (hiding the titles) or truncate to ~6 characters (destroying them), and
**Haiku has never solved this either**. Second cost, concrete: a tab-notched window outline is
non-convex, so it is not one `fb_box` — it is roughly six `fb_fill_px` runs per window, recomputed every
frame of a drag, and the tab's top corners must be radius 0 because `fb_rrect` cannot round two corners
and leave two square.

## 11. Fascia — **KILLED**

**pitch.** The desktop is a machined instrument panel with a screen set into it — one 72dp bottom fascia
of anodised plate, divided by engraved grooves into bays that never move, never reflow, and never
rearrange themselves around your window count.

**lineage.** Takes from CDE's Front Panel: one fixed bottom instrument panel with subpanels that rise
above it, and a workspace switcher as a lit matrix at its centre. Takes from IRIX Indigo Magic: the
engraved-legend, fabricated-metal reading. Takes from Datum the one idea Datum got right — two materials
on screen at once by law — but restates it in a different pair: **metal is chrome, paper is work.**
Anything the machine owns is plate; anything you own is paper. Takes from PLATE: radius 0, hierarchy by
case and weight, geometric icons, no gradients on chrome. Deliberately breaks both parents' refusal of a
panel, and breaks CDE's defining tic: there is not one raised bevel in the system. The only relief unit is
an **incised groove** — 1px dark then 1px light, cut into the plate — and it is never inverted to make
something look raised. Nothing is embossed; things are milled.

**structure.** One 72dp full-width bottom panel, fabricated as a single part, divided by grooves into
**fixed bays** in a fixed order: MENU | TASK (window buttons) | WORKSPACE (a 1×4 lit matrix) | INSTRUMENT
(load, mem, net as real drawn meters, `fb_bar`) | TRAY | CLOCK. A bay boundary is a physical edge and never
moves — when you open a twelfth window the task bay scrolls internally rather than shoving the clock
sideways, which is the single behaviour every reflowing taskbar gets wrong. Above each bay sits a 10dp
arrow that raises a **subpanel**: a square, 2px-framed sheet that rises from the panel edge, anchored to
its bay. The menu is a subpanel, so it opens in a known place instead of wherever the pointer was. Windows
are conventional: full-width 30dp title bar, min/max/close as three engraved cells at the right, 3dp plate
frame.

**palette.** Both modes, and the metal-to-paper relationship **inverts** between them because the material
law is "metal is mid-value, paper is extreme". LIGHT — bead-blasted natural anodise. Plate `#C6C3B9`, bay
face `#D6D3CA`, work deck `#E7E3D9`, window sheet `#F2EFE7`, ink `#191813` (15.47:1 on sheet, 10.08:1 on
plate), engraved legend `#4A463D` (5.33:1 on plate), ink3 `#5E594E` (6.06:1). Here the chrome is **darker**
than the work area (plate/deck 1.38:1). DARK — the ground is made of hard-anodised aluminium, a
warm-neutral mid-dark with a graphite cast, deliberately not near-black. Plate `#33342F`, bay face
`#3F403A`, deck `#171815`, window sheet `#262722`, silkscreen `#E9E6DB` (12.04:1 on sheet, 10.05:1 on
plate). Here the chrome is **lighter** than the work area (plate/deck 1.42:1) — the panel catches light, the
deck is in shadow. GROOVE: light `#E2DFD6`/`#8C887C`, dark `#5C5D55`/`#141511`. Its legibility is the
2.75:1 *between its own two lines*, not either line against the plate. LAMP: one indicator amber `#F0A21E`,
and it never sits on bare plate — it is inset in a 12dp `#2A2822` lamp well, 6.94:1 inside it, well-to-plate
8.36:1. Hard cap: no lamp fill exceeds 12dp, ever, and a lamp is never text and never a border.

**focus.** Four channels, and the loudest one is on the panel rather than the window. (1) The focused
window's **task button is physically pressed**: its groove flips from incised to sunk (2px dark on top and
left, light on bottom and right). That is a switch position, readable across a room, and it is the
affordance every flat taskbar threw away. (2) The window's title-bar plate steps one rung to the bay-face
value. (3) A 12dp amber lamp lights in the well at the title bar's left cap. (4) The 3dp frame edge goes
from groove to solid ink (light) / solid paper (dark).

**not default because.** Every generic panel is a floating rounded strip with translucency and a blur,
holding widgets that slide around as the window list changes. Fascia is a fabricated part: opaque, square,
bolted to the bottom edge, with bay boundaries as fixed as holes drilled in sheet metal. Two structural
consequences a default prompt cannot reach. First, **no reflow**: the clock is at `x = W − 96` forever, so
you build muscle memory instead of re-reading the panel. Second, the dark mode is the inverse of every dark
UI ever generated — the chrome is **brighter** than the desktop, which is what makes it read as a machine
with a display set into it rather than as a dark theme.

**weakest, in its own words.** The whole system rests on one 2px unit, which is precisely Raking's failure
reprised — and **the surface ladder is worse than Raking's, not better**. Measured: in dark mode
plate-to-deck is 1.42:1, plate-to-sheet 1.20:1, bay-to-plate 1.20:1, all below perceptual threshold. My
defence is that separation is carried by the groove's internal 2.75:1 and by 3dp edges rather than by the
steps, but that defence is exactly the argument Raking made and lost with. Second, the groove degrades
badly: at ui=1 it is 1px total, one of its two lines vanishes, and the entire machined reading collapses
into a plain hairline — so this design is materially worse at 1× than at 2×. Third, fixed bays are a real
cost on small panels: at 1366 wide the task bay holds about four window buttons before it must scroll.
Fourth, 72dp × full width is permanently spent, and unlike PLATE's rail it cannot collapse without breaking
the fixed-bay law that justifies it.

## 12. Shelf — **CHOSEN**

**pitch.** No bar anywhere: the dock is a column of live instruments on the right edge, the menu is a
persistent column that belongs to the focused application and can be torn off and left on the desk, and the
desktop itself is a **mid-grey** — a value no current desktop uses and the one that reads instantly as a
workstation.

**lineage.** Takes from NeXTSTEP and WindowMaker: the right-edge dock of square 64dp cells, the vertical
menu as an object that lives on the desktop rather than a popup, tear-off submenus, the shelf strip at the
top of a file window, close-left/miniaturise-right title bars with a hatched grip, and the bottom **resize
bar** (a 12dp full-width bar in three sections) which is a far better resize target than a 4px edge. Takes
from Folio: colour carries no chrome distinction. **Deliberately breaks the seam between the two parents by
reallocating the budget rather than splitting the difference** — PLATE spends its one saturated ink on
focus and primary action, Folio spends none, and Shelf spends exactly one on **selection** and nothing
else. Selection is the thing neither parent solved (both fall back to a paper band that is 1.1–1.3:1 and
disappears in a screenshot). Also breaks NeXT: no bevel, no 2-bit ladder, no `#AAAAAA` — the grey is warmed
and given a real four-rung ramp.

**structure.** Chrome is at two vertical edges and one movable column; no horizontal bar exists. **The
dock**, right edge, top-anchored: 64dp square cells, 2px ink frame, radius 0, and every cell is a **live
instrument** rather than an icon — the clock cell draws a clock, the monitor cell draws a live sparkline,
the terminal cell draws its last line, the mail cell draws a count. Bottom cell is always the workspace
switcher as a numbered 2×2. **The menu**: a 180dp column of 26dp rows that belongs to the focused
application, default position top-left, and it can be torn off, dropped anywhere, and left there —
including submenus torn off individually. Right-click on the desk raises the workspace menu at the pointer.
**The shelf**: a 64dp strip across the top of every file window, a per-window staging area for what you are
working with, doubling as that window's open-document switcher. Title bar 26dp: close left, title centred,
miniaturise right, with a 4-line machined grip hatch flanking the title at both ends.

**palette.** Both modes, and one ladder law holds across both: **the desktop is always the middle value and
windows are always the extreme.** LIGHT — desk `#9E9B93` (the identity: a mid-grey desktop, which nothing
shipping today does), window sheet `#E6E3DA`, chrome `#C8C5BC` (title bar, dock cell face, menu row), well
`#F1EFE8`, ink `#141310` (14.48:1 on sheet, 10.77:1 on chrome, 6.69:1 even on the bare desk), ink3
`#58534A` (5.95:1), grip hatch `#7E7A70` (2.48:1 on chrome — texture, not text). Sheet-to-desk 2.16:1,
chrome-to-desk 1.61:1, and every window additionally carries a 2px ink frame. DARK — the ground is made of
the same object in its other finish: the matte black textured paint on a NeXT magnesium case, so it is
still visibly a *grey*, not a black. Desk `#2E2C28` (middle), window sheet `#131210` (extreme), chrome
`#3A3833`, paper `#E8E5DC` (14.86:1 on sheet, 9.30:1 on chrome), ink3 `#A39D91`. Sheet-to-desk 1.34:1 plus
the 2px frame. **The one saturated ink, selection only:** `#1B4F8C` light (6.43:1 on sheet, and 6.43:1 for
knocked-out text on it), `#6FA8E0` dark (7.45:1 both directions). It appears on selected text, selected
file rows and the active menu row, and on nothing else — not focus, not primary action, not status.

**focus.** Deliberately **not** a knockout, because PLATE already owns that. (1) The grip hatch **appears**:
the focused title bar carries its 4-line `#7E7A70` hatch at both ends, the unfocused one has none — a
texture arriving, which no other candidate in this family uses as a state. (2) The frame steps 1px
grip-value → 2px ink. (3) The title goes ink3 regular → ink bold. (4) The strongest and cheapest: **the menu
column changes its contents**, with the focused application's name set at its head. The focus indicator is
a 180dp column of that app's actual commands, which means focus is legible from a part of the screen you
were already looking at. Unfocused windows also lose the section divisions in their bottom resize bar,
leaving a plain strip — geometry is not changed on focus, only what is drawn inside it, so nothing reflows.

**not default because.** Three things a generic prompt cannot produce, all structural. First, a **mid-grey
desktop**: every generated desktop for fifteen years has been near-white or near-black, and `#9E9B93` with
light windows on it is instantly not-that. Second, the menu is not a popup — it is a persistent object with
position and torn-off state that survives, so your workspace includes your menu layout, and right-click on
the desk is the only transient menu in the system. Third, the direct answer to the banned launcher pattern:
**a dock cell that draws live data cannot become a row of identical rounded tiles**, because no two cells
look alike and none of them is an icon. NeXT actually shipped this and everyone since has replaced it with
static pictograms. Finally, spending the single saturated ink on **selection** rather than focus is a
genuinely different allocation from both PLATE and Folio, and it fixes the one thing both of them left at
1.1–1.3:1.

**weakest, in its own words.** The two best ideas are the two that cost real architecture, and neither PLATE
nor Folio pays anything comparable. Folio's own spec states the property I am breaking: *"Immediate-mode
`ui.c`: widgets return whether they fired and take no action arguments… Nothing in this design needs
retained state."* Torn-off menus need exactly that — per-menu persistent position and open/closed state
owned by the shell — and it is the first thing in this family to need it. Live dock cells are worse: an
application must be able to render a 64dp tile while it is neither focused nor visible, which is a new
lifecycle contract, not a drawing primitive. Second, the usability bet: a menu column that follows focus is
the complaint people have about a global menu bar, transplanted onto a desktop where windows are *not*
full-screen, so the menu can be 1400px away from the window it controls. Third, 180dp menu + 64dp dock is
244dp of permanent horizontal budget, comparable to PLATE's rail but split across two edges, which is
harder to justify and harder to collapse on a narrow panel.

---

# The cull

## Chosen — and the one change each needs first

### 1. Apparatus (page)
**Why.** The cut edge is the single best technical contribution in the twelve: it is a real answer to the
problem the family actually has — PLATE dodges overlap by tiling, Folio admits its overlap case is weak,
Raking spent its whole budget on 1px runs and lost — and it costs nothing until two sheets touch. Giving the
machine its own *register* instead of its own *colour* is the most Folio-faithful idea here without being
Folio, and it does Datum's paper/ink law at a rule and 28dp instead of a second surface system. Focus fires
four channels, two of which are ink appearing in reserved air.

**Sharpen before building.** *Delete its stated blind spot — it is wrong, and it is costing the direction
confidence it should be spending elsewhere.* A top sheet lying entirely inside a lower one has its **entire**
boundary over another sheet, so the rule as written draws all four of its edges at 16.20:1 — the loudest case
the mechanism has, not its silent one. The genuinely silent case is a fully covered lower sheet, which has
nothing to disambiguate. Redirect that budget to the real risk it names third: a `hosttest` golden that
renders overlapping, contained and disjoint rects and asserts the black segments, **written before the rest
of the direction is drawn**, because a wm.c occlusion bug is 2px of black at 16:1 in the wrong place.

### 2. Deck (page)
**Why.** No window frames at all. Nothing else in the twelve — and nothing in round one — deletes the box
around the window, and "window" is the single hardest retrieval to break. Hierarchy by rank rather than
z-order is the only genuinely *unequal* chrome in the set. Its focus signal (a whole line of type
materialising in reserved air, plus 1.49:1 → 12.22:1 on the head rule, plus one real atlas step) is the
second loudest here and costs nothing, and the reserved 24dp means focus never moves geometry. Spends less
colour than Folio.

**Sharpen before building.** *Scope make-up to the band, not the page.* Opening a window should recompose
only the horizontal band it lands in — ranks still demote, the lead story still exists, but your terminal
three bands away does not jump. That removes the one genuinely user-hostile behaviour without touching the
metaphor, and it retires `HOLD` from a hedge to a convenience. (Keep the masthead discipline mechanical
too: the band's only permitted draw calls are `fb_text_role` and one `fb_fill_px` rule — no icon, no tray,
no fill. If that is a guideline rather than an assertion, the direction dies quietly the first time someone
adds a status glyph.)

### 3. Neatline (press)
**Why.** It takes the third position on the seam PLATE and Folio disagree along — not one accent, not zero,
but five with a **written contract on screen** — which is the only structurally new answer available there
and the exact opposite of decorative colour. The key-as-launcher is the only proposal in the twelve with a
real answer to 53 apps, because a key's job is explaining a sheet you have never seen. A full-perimeter apron
is the only chrome position with real precedent that nobody in round one chose. The IHO S-52 night table is
the best-sourced dark ladder in the set.

**Sharpen before building.** *Make the key two-state so the apron can shrink.* Collapse the 96dp bottom key
to a 24dp **live legend** showing only the inks and symbols currently on screen, with the full alphabetical
key on a held key or a chord. That cuts permanent chrome from ~13% of a 1200-tall screen to ~6% while keeping
a contract visible at all times — which is the legal justification for five inks, and the thing that must not
be cut.

### 4. Blueline (press)
**Why.** The only dark-primary member of the family, and the only one whose ground is neither paper nor
near-black. Focus by **line type** is the one channel nothing in round one used, it is free, and it is the
only channel that additionally encodes relationship (chain-dot) and absence (phantom — minimised windows stay
where they were, which answers "where did it go" without a dock). The revision table is a desktop keeping its
own changelog in its chrome, which is directly useful in this repo.

**Sharpen before building.** *Move line type off the 1px weight axis.* As sketched, focused is continuous-2px
and unfocused is dashed-1px, so the state is carried partly by a single-pixel mark with 1.21:1 (dark) and
1.07:1 (light) surfaces underneath and no value fallback — the bet Raking lost. Make **both** 2px and let the
*pattern alone* carry the state (continuous / 8-4 dash / chain-dot / phantom). A one-pixel dash-phase drift is
then invisible on a 2px mark, and the distinction survives any panel gamma.

### 5. SPREAD (merge)
**Why.** The only merge candidate that is a new direction rather than an edit to PLATE's spec. Hoisting all
window identity into one running head whose divisions **are** the column layout is a decision that only
exists if you take Folio's grid seriously enough to let it own the chrome — a merge consequence, not a style
choice. The cross-window shared baseline turns the baked-atlas limitation from Datum's handicap into an
exactness guarantee, and it is gateable in one line. And its colour position — a **boundary** law rather than
a budget law, zero in the shell, unconstrained inside a content rect — closes a hole Folio's absolute law
never addressed.

**Sharpen before building.** *Define the non-disjoint fallback before drawing a single segment.* The head is
undrawable for any window that cannot own a disjoint column range — modal, detached, mid-drag — and PLATE's
recorded single largest failure was having no reflow story at all. Specify it now: such a window gets a local
28dp plate and its head segment is struck through with a 1px rule. Without that case written down, SPREAD
inherits the parent's worst-understood area *and* makes reflow mandatory.

### 6. Shelf (linux)
**Why.** The mid-grey desktop is the single most immediately not-generic ground in the twelve, and it is the
only one that is neither paper, near-black, nor a chromatic stock. Live instrument dock cells make the banned
identical-rounded-tiles launcher structurally unreachable — no two cells can look alike. And spending the one
saturated ink on **selection** is a genuinely different allocation from both parents that fixes a named,
measured gap: both PLATE and Folio fall back to a 1.1–1.3:1 paper band that vanishes in a screenshot.

**Sharpen before building.** *Drop tear-off menus; keep the persistent menu column.* Tear-off is the only
thing in the whole family that needs retained per-widget position state in an immediate-mode `ui.c`, and it is
the least load-bearing of Shelf's three ideas — the column following focus is what carries the focus signal,
and that costs nothing. Same discipline for the dock: cells are drawn by the shell from a small per-app state
struct the app pushes, not rendered by a backgrounded app, so no new application lifecycle contract is needed.

## Killed — one line each

| direction | lane | why it dies |
|---|---|---|
| **Ledger** | press | Its whole premise is per-window CPU/mem/disk attribution that does not exist, and this repo's documented failure mode is exactly chrome that performs a calculation nobody ran — plus its head/foot bands occupy the same chrome position as Deck with a weaker structural idea, and its zebra is 1.17:1. |
| **Bradshaw** | page | It buys cross-window alignment by changing `theme.row_h` 28→32 and re-laying out every list, table and toolbar in `kernel.zl`, forbids any window shorter than the screen, kills free-floating windows outright, and has no answer at all for a 3-line dialog. |
| **GALLEY** | merge | PLATE with three of four accent uses deleted — an edit to the spec, not a direction — and the one thing it adds is a 6-device-px hairline its own author places at Raking's failure floor. |
| **STIPPLE** | merge | Takes PLATE's structure "unmodified" and bolts on a texture channel whose author says the failure mode is *silently becoming PLATE-with-one-more-grey* at real pixel pitch; it also needs the ordered-dither matrix widened 4×4 → 8×8, which is a change inside an existing primitive rather than a use of one. |
| **Shingle** | linux | The entire pitch is "find a buried window by reading its tab", and the tabs stop being readable at about ten windows — precisely when you need it — which Haiku has never solved either; the colour half of its focus signal is 3.35:1 of pure chroma that contributes nothing in greyscale. |
| **Fascia** | linux | A full-width bottom taskbar with a tray — the exact furniture all four round-one directions independently deleted — and by its own measurements its surface ladder (1.20–1.42:1) is *worse* than the one Raking was marked down for, with the groove collapsing into a plain hairline at ui=1. |

## Rules the cull was run against

- ≥1 survivor from each of the four lanes. **Met**: 2 press, 2 page, 1 merge, 1 linux.
- Kill anything that is a re-skin of PLATE or Folio rather than its own idea. **GALLEY** and **STIPPLE** both
  fell here; both describe themselves as taking PLATE's structure unmodified.
- Kill anything needing more than three type sizes. None of the twelve did — every one maps its R1–R4 *roles*
  onto the three atlases plus weight and case, which is the Folio move. Worth re-checking per direction at
  spec time; Datum's failure was six real sizes, not four roles.
- Kill anything needing a primitive `fb.c` does not have. **STIPPLE** is the only one that spends outside
  current primitive behaviour (the 8×8 threshold matrix), and it says so.
- Kill anything whose focus signal is weaker than Raking's (one blue pixel on the top edge plus a title
  stepping `#BDB5A7` → `#F2EDE4`). All six survivors clear it; **Shingle** clears it only on the geometry
  channel, which is one of several reasons it went.
- Prefer maximum spread. The six put persistent chrome in six different places, use six different focus
  mechanisms, and stand on six different grounds.

## What still has to be tested on the real panel, not in a browser

Every judgement in this directory has been made against a headless browser render. The prototypes flatter the
target in at least one known way: CSS gives letter-spacing free and `fb_text_role` cannot track text without a
new primitive. Carry these three forward from round one into round two's prototypes:

1. **Tracking.** Every survivor uses tracked caps as a register. If `fb_text_role` cannot track, four of six
   lose a hierarchy channel. Measure before committing the type scale.
2. **Subpixel FIR on non-neutral grounds.** The 5-tap (1,2,3,2,1)/9 filter was tuned on near-black. Blueline
   puts light marks on saturated Prussian blue and Shelf puts dark marks on mid-grey — both far from that
   tuning. `wmshot` plus a loupe, not an argument.
3. **1–2px marks at arm's length.** Apparatus's cut edge, Blueline's line types and SPREAD's leader rules all
   live on thin marks. Raking's recorded loss was exactly this class. Photograph the actual panel.
