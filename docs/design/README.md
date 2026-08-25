# Desktop redesign — four directions, and how to look at them

The current zlOS desktop is a measured clone of `docs/design/ds-reference.html`:
near-black surfaces, one lime accent, 16 px radius on everything, top bar plus
floating windows plus bottom dock. It is competent and it has no identity — it is
what a generic "dark desktop UI" prompt produces, and its dock is seven identical
rounded tiles in a row.

This directory holds four replacement design languages, each taken far enough to
be judged by eye rather than argued about. **Nothing here has been implemented in
the kernel.** `design.h`, `ui.c`, `wm.c`, `uikit.c` and `kernel.zl` are untouched
on this branch.

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

**Round two is in [`round-two-candidates.md`](round-two-candidates.md).** Twelve new
directions across four lanes (press / page / merge / linux), culled to six to build:
**Apparatus, Deck, Neatline, Blueline, SPREAD, Shelf** — each with the one change it
needs before it is built, and one line on why each of the other six died.

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
