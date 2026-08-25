# LOOK-AND-SPEED-PROMPT — make it look like the northstar, and make it fast

> **USER INTENT CLARIFIED 2026-08-19.** Roy is pointing at the v10 HTML mainly
> because of how polished it looks and how fast it feels. Do not turn that into
> a demand to clone every simulated application or into another completeness
> percentage. Read [`visual-speed-northstar.md`](../../visual-speed-northstar.md)
> first; it is the durable intent. This prompt is the older execution brief and
> much of it has already been answered by `DECISIONS.md` and
> `kernel/docs/desktop/look-and-speed.md`.

Written 2026-08-19, for a fresh session. Two halves. **Do the look first** — a
frame budget is meaningless until you know what has to be in the frame.

The reference is now in the repo: **`docs/design/zlOS-design-northstar.html`**
(33 KB, dated 2026-08-14). It was living loose in `~/Documents` and versioned
nowhere, which is why nothing was measured against it. Open it in a browser
beside `./try.sh` and put them side by side. That comparison IS the task.

---

## Part 1 — the look

### The northstar states its own rules. Read them first.

From its header comment, verbatim:

> A single committed visual world: a dark, CPU-rendered desktop. Every choice
> here is constrained to what an 800x600 linear framebuffer with no GPU can
> actually draw: **vertical gradients only**, a fixed palette, **tight low-spread
> shadows**, **small radii**, **alpha blends but no gaussian blur**. No light
> theme on purpose — this is a screen, not a document surface.

And its closing line:

> That constraint is a design language, not a handicap ... Lean into it and the
> result reads as a deliberate aesthetic instead of a failed imitation.

**This is the spec. It is not a mood board.** Every rule in it is a rule the
kernel can actually honour, because it was written from the kernel's own
constraints.

### The kernel has drifted from three of those rules. Measured, not guessed.

| northstar says | the kernel does | where |
|---|---|---|
| "alpha blends but **no gaussian blur**" | a cached gaussian blur, 7.37 ms cold / 0.18 ms cached | `fb.c` blur arena, `HI_BLUR` |
| "**vertical gradients only**" | radial **and conic** gradients in the wallpaper | v10 plan §8.1 item 5, "green" |
| "**small radii**" | radius raised **5 → 12** | v10 plan §8.1 item 10 |

Each of those was shipped as a v10 deliverable and gated green, so **none of
them is a bug** — they are decisions taken against a different reading of the
reference. Your job is to decide, deliberately and in writing, which one wins
for each: the northstar as written, or the code as shipped. Then make the tree
and the docs agree.

There is direct evidence they diverged by accident rather than by choice:
`evidence/desktop-v10-plan.md` §4 says *"The prototype wants `backdrop-filter:blur(10..22px)`
in 6 places and `filter:blur(30..34px)` in 2"* and calls blur "the only genuine
wall". **The prototype in this repo contains zero `backdrop-filter` and zero
`blur()`** — its only two uses of the word "blur" are the two sentences above
saying not to use it. Either an earlier prototype existed and was replaced, or
that section was written from memory. Settle it before you touch the blur code:
a cached blur that nothing is supposed to use is 16 MiB of arena and 7 ms of
first-paint for nothing.

### What already matches — do not "fix" it

The palette is faithful. Verified by converting the northstar's hex to the
kernel's `rgb()` form and grepping `kernel.zl`:

```
wall-top #1a1e32 = rgb(26, 30, 50)     found
wall-bot #0a0c16 = rgb(10, 12, 22)     found
accent   #60d2eb = rgb(96, 210, 235)   found
txt-hi   #d2e4ff = rgb(210, 228, 255)  found
crit     #e05a5a = rgb(224, 90, 90)    found
panel    #05060a, ok #5bd66e           NOT found - check these two
```

The northstar's own comment says the palette came "straight from kernel.zl's
rgb() theme", so agreement is expected and disagreement is the signal. Check
`panel` and `ok` specifically.

### The open visual defects, already diagnosed

- **The dock readout leaves digit debris** (`frame 0 us peak 0 )08 up 1`). The
  status numbers are drawn at fixed x-offsets with **no background clear**, so a
  shrinking value leaves the tail of the previous one. Full diagnosis in
  `kernel/docs/archive/prompts/POINTER-PROMPT.md` §1b. Small fix, visible every boot.
- **The desktop reads coarser than the v10 screenshots and nobody knows why.**
  `POINTER-PROMPT.md` §1c has both PNGs under `docs/evidence/visual-diffs/` and the list of what
  has already been ruled out — no visual function was lost, `fb_text_aa` and
  `fb_glyph_aa` are byte-identical to the branch that drew those shots. That
  points at a **parameter**, not deleted code: scale, cell size, padding, or the
  console geometry. Do not repeat the ruled-out work.

### How to compare, properly

`kernel/probe-shot.py` photographs the running desktop headlessly and can crop
and zoom:

```
./probe-shot.py -o now
./probe-shot.py --crop 1300,1090,620,80 --zoom 3 -o dock
```

Measure, do not eyeball. A previous attempt at §1c compared the two screenshots
at a fixed pixel row while they showed **different content** at that row, and
got an inconclusive answer from a real difference. Compare like for like.

---

## Part 2 — speed

### What paces a frame today, measured

```
idt.c:344   divisor = 1193182 / 100      the PIT ticks at 100 Hz
wm.c:1520   unsigned int now = idt_ticks();
wm.c:1521   if (now == last_tick) return;
```

So **the compositor is paced by a 100 Hz timer interrupt and nothing else.**
Consequences, none of them yet addressed:

- **No vsync anywhere.** Nothing waits for a vertical blank, so a present can
  land mid-scanout. On the ThinkPad the panel was *measured* at **59.998 Hz**
  (`kernel/HANDOFF.md`, from `PIPE_LINK_M1/N1`), so a 100 Hz compositor against
  a 60 Hz panel gives an uneven cadence by construction — some refreshes get two
  presents, some get none.
- **A slow frame is dropped silently.** If a frame takes longer than 10 ms the
  next tick simply runs; nothing counts the miss. `wm_peak_us()` records the
  worst frame but nothing records *how many* were late, which is the number that
  actually describes smoothness.
- **100 Hz is a cap as well as a floor.** Even a 1 ms frame waits for the tick.

### The numbers that already exist — start from these, do not re-measure

From `evidence/desktop-v10-plan.md` §8, all gated:

```
cached blur          7.37 ms cold, 0.18 ms cached
full-screen fill     0.71 cyc/px   (was 7.97 before the back buffer moved)
fb_fill_blend        22.2 cyc/px
back buffer          ON at 3840x2160 (31.6 MiB of 40)
```

And the on-screen readout is live: the tray shows `frame N us  peak N us`, and
`probe-frame.py` reads it over serial. **Use it.** desktop-TODO 0h's rule stands:
*"add a tsc() builtin and put frame time on screen. DO THIS BEFORE ANY
PERFORMANCE WORK. Optimising without measurement is guessing."*

### Where the time probably goes, in order

1. **`fb_shadow` was 4.3 ms of a 5.1 ms window redraw** before the scissor fix
   landed. Re-measure it now — that fix changed the picture and nobody has
   re-timed it.
2. **SMP band rendering exists and may not be on.** `fb.c` has `fb_par_hook`,
   `fb_par_run` and `fb_band_edges`, and `par_dispatch` is **NULL by default** —
   with no dispatcher installed every band runs serially on one core. Check
   whether anything actually calls `fb_par_hook` at boot. Three of four cores
   are parked (`smp.c`); this is the single largest untapped win and it may be
   one call.
3. **The wallpaper is cached, the dock is baked into it.** That was the v10 win.
   Confirm it still holds after the merge rather than assuming.

### What "fast" should mean here

Pick a target and write it down. A defensible one: **every frame under 16.6 ms
with the peak under 16.6 ms too**, because a peak over budget is what a person
actually perceives as stutter, and the average hides it. State the number, gate
it with `probe-frame.py`, and put the miss count on screen next to the peak.

---

## Rules

- **The look decision is a decision, not a bug fix.** Where the code and the
  northstar disagree, say which wins and why, in `kernel/docs/DECISIONS.md`.
  Silently changing either one is how they drifted in the first place.
- Nothing here needs new hardware capability. The northstar was written to the
  kernel's limits on purpose; if something looks impossible, re-read its header.
- `gates/land-gate.sh` runs the builds, 27 harnesses, both static checkers and
  every boot gate. Run it before landing, backgrounded, on a quiet box.
- Everything lands on `main`, the same day — `docs/WORKING-RULE.md`.
- A thing is done when a command says so and you read the output.
