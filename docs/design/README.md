# Design Documents

The repository-wide per-directory orientation and agent-policy system is defined
in [`directory-documentation-system.md`](directory-documentation-system.md). It
covers every tracked directory and is enforced by `tools/directory-docs.py`.

This folder contains proposals, decisions, inventories, gap studies, and visual
references. It is not an implementation-status folder.

- Trust each document's own `Status:` banner.
- `proposal`, `plan`, and `ready to build` mean not implemented.
- `decision` fixes an intended behavior; it does not prove every engine follows it.
- Inventory and gap files are dated analysis and should be rechecked before use.
- Superseded designs belong in `../archive/superseded/`.

Current delivery status lives in [`../PROJECT-STATUS.md`](../PROJECT-STATUS.md)
and [`../program/`](../program/).

## Desktop redesign archive

**Status:** historical design exploration preserved as input and evidence. The
old branch-level claim that none of this reached the kernel is no longer current:
the selected direction evolved into [`presswork-prototype.html`](presswork-prototype.html),
and the PRESSWORK implementation now lives in the kernel and UI sources. Use
[`PRESSWORK-BACKLOG.md`](PRESSWORK-BACKLOG.md),
[`PRESSWORK-ROUND-12.md`](PRESSWORK-ROUND-12.md), and the current project status
for implementation truth. Round 12 is complete in the 2026-08-30 consolidated
tree: all 56 confirmed findings are implemented, with build/host/QEMU evidence
and explicit hardware skips recorded in that audit.

This archive contains the four original replacement design languages, each taken
far enough to be judged by eye rather than argued about. The prototypes remain
design evidence; they are not runtime proof for the current kernel.

## What is here

```
specs/<direction>.json      the frozen specification: palette with every token,
                            hex and role; structure; chrome; type scale; radius
                            system; depth model; icon language; motion
prototypes/proto-*.html     a full interactive desktop per direction, one
                            self-contained file, 2.0k-2.8k lines each
prototypes/render/          49 headless renders, every scene in every mode
prototypes/compare/         light-vs-dark pairs and per-direction contact sheets
prototypes/recover.py       replays the prototypes out of agent transcripts
../../kernel/docs/desktop-redesign-spec.md
                            the 72 KB implementation contract for PLATE, written
                            when it won the judged comparison
```

Open any `proto-*.html` in a browser. Windows focus, move and close; the overview
and command palette open; the settings toggles and sliders change real values.

**Scene deep-links.** Every prototype reads the URL hash on load and on
`hashchange`, so any state is one URL away with zero clicks:

```
#desktop  #overview  #palette  #files  #monitor  #settings  #lock
```

and, for the three that have a dark ladder, the same seven with `-dark`
appended. `shift+D` toggles, as does the button top right.

**If you like parts of several directions, read [`deciding.md`](deciding.md) first.**
It decomposes the four into ten independent axes with a position and a stated cost
each, a conflict matrix where every conflict carries a recomputed number (the
knockout-on-Raking case, the L\* ≥ 89.6 that Datum's material law actually requires,
the ≥3:1 floor free floating puts on the window boundary), the features that combine
with anything, and the order to settle the axes in. It is written for deciding, not
for reading.

**[`prototypes/decide.html`](prototypes/decide.html) is that decomposition as an
interactive picker.** Choose a position per axis, in settling order, with every
option's cost visible on the option itself; the conflict matrix evaluates live and
tells you whether the combination is buildable and why not. Free riders are
separate toggles because they combine with anything. The three blends
(`proto-blendpaper`, `proto-blendgraphite`, `proto-blendpage`) load as one-click
presets and can be edited from there.

**Round two is in [`round-two-candidates.md`](round-two-candidates.md).** Twelve new
directions across four lanes (press / page / merge / linux), culled to six and all six
built: **Apparatus, Deck, Neatline, Blueline, SPREAD, Shelf** — each with the one change
it needed before building, and one line on why each of the other six died. Summary in
[Round two](#round-two-six-more-built) below. Eleven prototypes exist in total.

## The four

| | mode | the one structural move |
|---|---|---|
| **PLATE** | light + dark | printed technical document. Warm paper, black ink, 2 px rules, one vermilion overprint. The left rail is a *numbered register*; the module grid is ruled onto the desk where you can see it. Focus = the header plate knocks out to solid ink. |
| **Datum** | light + dark | paper and ink are two different materials on screen at once, by law: paper is the user's, ink is the machine's. Full-height ink rail, full-width raster strip, per-window microsecond timing in every status band, memory-map ruler along the bottom edge. |
| **Folio** | light + dark | the desktop as a printed page. Square-cornered sheets on a 12-column grid, a left margin rail carrying the running table of contents, and every distinction made with weight, case, scale, rule-thickness and whitespace — never with colour. |
| **Raking** | dark only | a warm-graphite machine lit by one cold light from the upper left. Depth is a 1 px lit edge plus a 1 px cut groove, never a shadow. Focus means "turned toward the light". Radius encodes how much an object can move, and nesting halves it. |

All four independently deleted the top bar, the dock and the start menu and
replaced them with a single left rail. None of them needs a drawing primitive
`fb.c` does not already have.

## How they were chosen

Four directions were generated independently under different briefs, then scored
by three judges on separate lenses. Scores are out of 10 per lens:

| direction | identity | buildability | regression risk | total |
|---|---|---|---|---|
| **PLATE** | 9 | 8 | 7 | **24/30** |
| Raking | 4 | 7 | 9 | 20/30 |
| Folio | 8 | 5 | 7 | 20/30 |
| Datum | 7 | 3 | 4 | 14/30 |

Raking is the best-engineered and the least distinctive; the identity judge called
it "the best-engineered proposal and the worst one on identity, which is exactly
the trap the brief warns about." Datum has the most original single idea and the
worst type fit — six sizes against three baked bitmap atlases means real
upscaling.

## Dark mode is a second ladder, not an inversion

Each dark mode is implemented as `:root[data-theme="dark"]` overriding the light
`:root`, with **every custom-property name identical between the two**. That is
deliberate: it is exactly the shape `ui_theme_init_q8()` will need, mapping one
set of role names onto two primitive ladders. A token that exists in only one
mode is a defect, and the prototypes are checked for it.

Each direction derived its dark ground from its own grammar rather than reaching
for a stock near-black:

- **PLATE** → ortho lith film. Amber base, Dmax rail, and "the knockout clears"
  rather than inverting.
- **Datum** → both materials go dark and stay two materials. Explicitly *not* a
  swap: swapping would make the machine's chrome the brightest thing on screen
  and let it outrank the user's document.
- **Folio** → a darker stock, keeping the warm hue axis, with the
  colour-carries-no-distinction rule intact.

## What each one costs, in its own words

These are the implementers' verdicts after rendering every scene and looking at
the pixels. They are kept unedited because the costs are the useful part.

**PLATE — dark keeps the direction.** "Every contrast failure I found was
symmetric across both modes, and light was marginally the worse of the two. A
bolted-on dark mode fails in dark and passes in light; this one failed
identically in both, which means the flaw was in the shared token ladder, not in
the negative." Cost: large screened areas are held at 5.60:1 instead of the solid
9.00:1, so meter fills, sliders and plot bars read softer in dark. The settings
sliders are the clearest case.

Its light-mode structural weakness is separate and real: **there is no reflow
story at all.** An app authored to fill a 7×8 tile overflows its own frame at
7×4. Four separate defects were that one failure wearing different hats, and the
fix needed a `sheetRows()`-style gate. A real implementation must put that rule
in the shell, not sprinkle it through the app renderers.

**Folio — dark is the better-argued half.** "Light was the mode with the real
defect, not dark. The scrimmed-furniture contrast miss (4.13:1) was in paper only,
and it was there precisely because the dark block did the structural thinking
about the veil and the light block did not." Cost: the one shadow mostly stops
working, and the scrim buys 8.7 dL\* of recession against light's 18.9.

**Datum — light is primary, dark is a competent second, not an equal.** "The
paper/ink law is a luminance idea, and dark takes the luminance away and hands
back hue and a hairline." Material separation falls from 14.26:1 on paper to
**1.18:1** in dark; separation moves onto hue and onto the datum rule at 3.55:1.
The overview loses most — in light the thumbnails are sheets of paper glowing on
a black table, in dark they are outlined rectangles at 1.18:1 fill. The raster
strip is the one element that is *better* in dark, because the cyan and orange
columns become the only chroma on screen.

**Raking — the strongest premise, the weakest focus signal.** "It commits to one
mechanism and refuses to add a second." Its settings pane shows that all four
surface steps sit between 1.073:1 and 1.126:1 — below perceptual threshold — and
that the entire separation budget is spent on the 1 px runs. Weakest link, stated
by its own implementer: "the focus signal is one blue pixel on the top edge plus
a title going from `#BDB5A7` to `#F2EDE4`. At arm's length on a busy desktop it
is near the floor of obvious-at-a-glance, and it is the first thing I would test
on a real panel rather than a PNG."

That last sentence generalises. Every judgement in this directory was made
against a headless browser render, not against `fb.c` output on a 2560×1440
panel. The prototypes flatter the target in at least one known way: CSS gives
letter-spacing free, and `fb_text_role` cannot track text without a new
primitive.

## Why `recover.py` exists

The first version of these prototypes was written to the session scratchpad under
`/tmp`, and `/tmp` was cleared before any of it was committed. Every subagent
`Write` and `Edit` is recorded with full arguments and a timestamp in
`~/.claude/projects/<project>/<session>/subagents/workflows/*/agent-*.jsonl`, so
the files were not lost, only unreplayed. `recover.py` applies them in timestamp
order across all agents that touched a path, dropping any whose `tool_result`
errored — a failed edit gets retried with different text, and applying both
corrupts the file.

It recovered two of four intact. The two it could not recover were built partly
through shell heredocs rather than the Write tool, which leaves nothing to
replay. Those were rebuilt from `specs/`.

The rule this bought: **deliverables live in the repository from the first
write.** The scratchpad is for genuinely disposable intermediates only.

---

## Round two: six more, built

Round one asked "what could this be". Round two asked "more like PLATE and Folio,
merge them, and give me one that looks like a Linux". Twelve candidates were
generated across four lanes and culled to six, with the cull forced to keep at
least one from every lane so no lane could be quietly wiped out.

Every one of these ships light **and** dark from the first write, rather than
having dark bolted on afterwards.

| | lane | the move |
|---|---|---|
| **Blueline** | press | An engineering drawing, in diazo whiteprint: blue line on white, **no black anywhere**, cyanotype as the other mode. All persistent chrome collapses into a bottom-right title block carrying the revision list, view index, scale and sheet number. Window state is genuinely **line type** - present windows continuous, unfocused hidden, minimised phantom - with numbered view bubbles and grid coordinates on all four margins. |
| **Neatline** | press | One map sheet. A full-perimeter marginal apron carries grid coordinates on three edges and the KEY on the fourth. It takes the third position on the seam PLATE and Folio disagree along: not one accent, not zero, but **five inks with a written contract on screen**, each lexical (water = I/O, wood = user data, contour = level, road = routes and actions), all forbidden as text, none carrying state. Its dark ladder is sourced from the IHO S-52 night colour table real ECDIS bridges use. |
| **Apparatus** | page | Every window is a scholarly text with a critical apparatus: numbered lines in the outer margin, the machine's commentary at the foot keyed to line numbers. Focus means the apparatus **opens**. Its real contribution is the **cut edge** - the border you can see is drawn by the occluder, never the occluded - which is a direct answer to the one problem this whole family has, and it needs no new primitive. |
| **Deck** | page | A front page that has been made up. **No window frames at all**: a story is bounded only by the rules it shares with its neighbours, which deletes the gutter, the mount and the reason for a shadow in one move. Hierarchy is by rank rather than z-order, so it is the only chrome in the set that is deliberately unequal - one window is the lead story. Focus means the deck line gets set. |
| **SPREAD** | merge | Folio's 12-column page with PLATE's knockout lifted off the window entirely and hoisted into a **running head shared by the whole screen**. One head, one baseline, one page. |
| **Shelf** | linux | A workstation, from the NeXTSTEP / IRIX line rather than the GNOME one. No bar anywhere: a **mid-grey hatched desk**, a left menu column that belongs to the focused application, and a right-edge dock where every item is a **live instrument** rather than an icon. |

### Why the other six died

Each kill names a defect, not a preference.

| | why |
|---|---|
| Ledger | Its premise is a foot rule reconciling per-window CPU, memory and disk against capacity, and that instrumentation does not exist. A balance that cannot balance. |
| Bradshaw | Buys cross-window horizontal alignment by changing `theme.row_h` from `dp(28)` to `dp(32)` at `ui.c:145`, re-laying out every list in every app. |
| GALLEY | "PLATE with three of its four accent jobs deleted - that is an edit to the spec, not a direction." |
| STIPPLE | Takes PLATE's rail, desk grid, module and knockout unmodified and bolts a sixth distinction channel onto Folio's law. |
| Shingle | The whole pitch is finding a buried window by reading its tab, and the tabs stop being readable at around ten windows - exactly the case the idea exists to solve. |
| Fascia | A full-width 72dp bottom taskbar with window buttons, a tray and a clock: the exact furniture all four round-one directions independently deleted. |

### The two sharpenings worth knowing about

Both were applied before building, and both are the same class of mistake this
project keeps catching:

- **Blueline** was told to move line type off the 1px weight axis. As sketched,
  focused was continuous-2px and unfocused dashed-1px, so state was carried
  partly by a hairline - which is precisely the failure Raking was marked down
  for, and it does not survive a real panel.
- **SPREAD** was told to define its non-disjoint fallback *before* drawing a
  single head segment, because the running head is undrawable for any window
  that cannot own a disjoint horizontal span.

**Apparatus** also had its own stated blind spot deleted as simply wrong: a top
sheet lying entirely inside a lower one has its whole boundary over another
sheet, so the cut-edge rule already draws all four edges at 16.20:1. That is the
mechanism's loudest case, not its silent one. The genuinely silent case is a
fully covered lower sheet, which has nothing to disambiguate anyway.

## Round three: one blend, built per axis rather than per direction

**The Graphite Blend** (`prototypes/proto-blendgraphite.html`) is the first
prototype here assembled from `deciding.md`'s axes instead of from a single
brief. It takes Raking's warm-graphite ground and 1px light-direction depth,
PLATE's numbered register rail, and Datum's instrumentation, and it answers the
one thing Raking's own implementer called its weakest link.

Two results are worth lifting out of it, because they are arithmetic rather than
taste and they apply to any dark direction this project ships:

- **On a dark ground the >=3:1 overlap boundary cannot be met by a groove, at
  any value.** `ZD_BASE` has Y = 0.02552, so the largest ratio pure black can
  reach against it is 1.510:1; against `ZD_FLOAT` it is 1.772:1. Free floating
  therefore forces the occluding edge to be the LIGHT, not the shadow, and the
  blend adds one token for it - `ZD_EDGE_OVER #8A8279`, which clears 3:1 on
  every surface in the ladder (3.67 BASE / 3.13 FLOAT / 4.14 VOID / 3.37 RAISE /
  4.44 WELL). Apparatus's cut-edge rule then says who draws it.
- **Raking's focus signal can be fixed by spending area instead of contrast.**
  A 3dp `ZD_STEEL` bezel plus a `ZD_FOCUS_WASH` ramp across the title band moves
  17,976 px of a 600x420 plate, 7.13%, against Raking's 1,019 px / 0.404% - and
  it does it with the maximum contrast left exactly where Raking had it, 6.19:1.
  PLATE's knockout would have been 11.92:1 and would have inverted the light's
  sign on the one object it lands on.

It is **dark native**: `:root` carries the derived work-light ladder and
`:root[data-theme="dark"]` carries the home one, so `#desktop-dark` is the
direction and `#desktop` is its second ladder. The derivation is not an
inversion - the light stays upper-left in both, and what changes is which edge
has the headroom to carry depth (graphite: struck edge 1.993:1, groove 1.332:1;
work light: groove 1.831:1, struck edge 1.241:1).

Its settings pane computes every number on this page at run time from the
tokens as the browser resolved them, and two of its toggles exist to make the
argument falsifiable rather than readable: turning the occlusion edge off drops
the boundary back to 1.332:1 so the failure is visible, and switching the raster
strip's clock from composite to frame restores Datum's 215,993 idle wakeups per
hour so the standing tax is countable.

### Still unverified, for all ten

Every judgement in this directory was made against a headless browser render.
This archived comparison did not go through `fb.c` on a 2560x1440 panel, and the prototypes
flatter the target in at least one known way: CSS gives letter-spacing free,
where `fb_text_role` cannot track text without a new primitive.
