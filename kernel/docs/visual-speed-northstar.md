# zlOS visual and speed north star

Recorded 2026-08-19 from Roy's clarification. This is the intent behind pointing
at `~/zl OS v10.dc.html`.

## The point

The prototype is primarily a reference for **how good the desktop looks and how
fast it feels**. It is not primarily a demand to clone every simulated app in
the HTML before the comparison counts.

Keep these as two separate tracks:

1. **Visual quality and feel** — the desktop should look deliberate, modern,
   coherent and finished.
2. **Application completeness** — file manager, editor, settings, process
   manager, browser and the rest become real applications over time.

Missing applications do not mean the visual target is unreachable. Conversely,
a matching screenshot does not mean the applications work. Report both truths
without collapsing one into the other or inventing a percentage.

## What "look like this" means

The target is the prototype's level of finish, not necessarily every literal
colour or effect:

- a single coherent visual system across wallpaper, windows, panels and dock;
- strong typography hierarchy, clean spacing and useful information density;
- smooth curves, restrained shadows, clear elevation and crisp icons;
- windows and controls that feel like parts of one OS rather than separate
  drawing demos;
- immediate visual feedback for focus, hover, press, open, close, drag, resize,
  snap and errors;
- motion that is short and purposeful, never decorative waiting;
- no coarse scaling, digit debris, clipped text, duplicated palette roles or
  status indicators that claim something the machine has not proved.

The existing navy/cyan zlOS identity stays unless Roy explicitly chooses a
repaint. The useful thing to take from the v10 HTML is its polish, hierarchy,
spacing, motion and responsiveness, not automatically its lime/black palette.

## Can the current system draw it?

**Yes.** This is no longer speculative. The shipping renderer and compositor
already have the important machinery:

- damage-tracked windows, z-order, focus and input routing;
- dragging, resizing, snapping, modal windows and a dock;
- proportional and monospace text atlases;
- rounded rectangles, alpha blending, gradients, shadows and cached effects;
- an animation timeline;
- a cached wallpaper and a back buffer through 3840x2160;
- the desktop as the normal framebuffer boot state.

The remaining visual work is refinement, consistency, reachability and real app
surfaces. Do not describe it as blocked by the language. The recurring
distinction is:

- **the language cannot express it** — a real language/runtime constraint;
- **nobody has wired or written it yet** — ordinary engineering work.

Most of the remaining desktop gap is the second kind. The clearest example is
`ui.c`: the widget toolkit exists in C, but zl applications cannot call its
widgets yet.

## Capability map — what exists, what needs finishing, and the real limits

### Already possible with the current machinery

- flowing linear-gradient backgrounds;
- radial glows, conic sweeps, a vignette and subtle generated texture;
- rounded windows, panels and controls;
- soft shadows, alpha blending and translucent stationary surfaces;
- anti-aliased proportional and monospace text at the built-in atlas sizes;
- crisp generated icons;
- hover, press, focus, selection and error states;
- opening, closing, dragging, resizing and snapping windows;
- a cached detailed wallpaper at the ThinkPad's 2560x1440 mode;
- fast interaction when damage tracking repaints only what changed.

These are not future renderer research. They are composition and finishing work
over primitives that exist now.

### Possible, with unfinished machinery

- **Smoother animation.** The timeline exists, but motion is still a small set
  of discrete frames and pacing is not synchronised to the 59.998 Hz panel.
- **More flexible UI scaling.** Layout currently changes in whole-number scale
  steps, which contributes to the coarse/blocky feeling between modes.
- **Arbitrary text sizes.** Font shapes are rasterised at build time into a few
  fixed atlases; there is no runtime vector font rasteriser.
- **Correct opacity fades.** The fade path exists, but the current compositor
  restores its saved backdrop at the wrong origin in one path.
- **Consistent application controls.** `ui.c` has the widgets; zl applications
  do not yet have the widget builtins that expose them.
- **One coherent theme.** `kernel.zl` and `ui.c` currently carry overlapping
  palettes and roles that must become one source of truth.
- **Even 60 Hz motion.** A TSC deadline or real vblank-informed pacer must
  replace the current uneven 100 Hz PIT cadence.

All of these are possible. "Unfinished" means code and integration work, not a
known hardware or language impossibility.

### Possible, but only with the right implementation

- A stationary blurred menu can be computed once and cached. Recomputing blur
  continuously behind a moving window is too expensive on the current CPU path.
- A detailed animated background can move small cached layers. Rebuilding all
  full-screen glows and gradients every frame is over budget.
- A detailed 4K wallpaper can be cached after changing its memory budget. The
  current 16 MiB wallpaper/effect arena cannot hold a 3840x2160x4 image.
- GPU acceleration could make live full-screen effects cheaper, but is not
  required to reach the prototype's visible quality.

The design rule is simple: compute expensive static appearance once, cache it,
and animate or repaint only the small region that actually changes.

## What the 4K memory limit actually means

zlOS currently reserves fixed memory regions for major graphics jobs instead
of asking a general heap for any size at runtime. Think of it as labelled
drawers rather than one open warehouse.

The wallpaper/effect drawer is 16 MiB:

- 1920x1200x4 = about 8.8 MiB — fits;
- 2560x1440x4 = about 14.1 MiB — fits after the unused dock blur stopped taking
  space first;
- 3840x2160x4 = about 31.6 MiB — does not fit in that 16 MiB drawer.

This does **not** mean the machine lacks RAM and it does not make a detailed 4K
background impossible. It means the current fixed memory map assigned too
small a compartment to that cache. The back buffer already has a separate
large region and supports 4K.

Valid ways to handle the wallpaper are:

1. reserve a larger wallpaper cache region and prove it does not overlap any
   other fixed region;
2. size the cache from the boot mode rather than using one fixed ceiling;
3. cache the wallpaper in tiles and keep only the tiles needed;
4. use a lower-resolution background and filter it when presenting;
5. retain the current honest fallback to a cheap plain gradient when caching
   is refused.

The clean long-term answer is a boot-sized graphics cache with central memory
map assertions. This repo has already suffered silent fixed-region collisions,
so changing the map must be gated rather than treated as a casual constant
increase.

## What "fast like this" means

Fast is perceived latency and smoothness, not a high average frame rate printed
after the fact:

- a click or keypress responds on the next visible frame;
- dragging and resizing stay attached to the pointer;
- opening, closing, switching and snapping do not hitch;
- terminal output and app updates do not stall the compositor;
- no painted frame exceeds the real panel budget;
- missed frames are counted instead of hidden by an average.

The ThinkPad panel is measured at 59.998 Hz, so the visible frame budget is
**16.67 ms**. The standing target is:

> Every painted frame under 16.67 ms, peak under 16.67 ms, and `late = 0` for
> ordinary desktop interaction.

`wm_us`, `wm_peak`, `wm_late`, `wm_lost` and `wm_painted` exist because a fast
average can still hide a desktop that visibly stutters once a second.

## What the current measurements say

Measured on the i7-10510U target-class CPU at 1920x1200, using the shipping
renderer:

| work | measured cost | meaning |
|---|---:|---|
| one complete window | about 0.8 ms | window chrome is not the problem |
| whole desktop redraw | about 4.7 ms | comfortably inside 16.67 ms at 1920x1200 |
| 40 lines of AA monospace text | about 5.1 ms | terminal redraw is the largest ordinary visible cost |
| full-screen present/blit | about 2 ms | unavoidable floor; larger at 4K |
| menu-sized blur, cold | about 7.5 ms | too expensive to recompute while moving |
| the same cached blur | about 0.19 ms | cached effects are affordable |
| one 900x700 radial glow | about 12.2 ms | the wallpaper cannot be rebuilt per frame |

That explains the current design:

- the wallpaper is rendered once and cached;
- damage rectangles repaint only what changed;
- expensive effects are cached or removed from moving paths;
- a 4K full-screen redraw is over budget, so damage tracking is load-bearing;
- enabling all four cores is not the automatic answer: measured SMP band
  rendering reached 1.76x, two bands were slower than serial, and the current
  implementation keeps the extra cores spinning.

The next speed work should follow perception, not theoretical throughput:

1. remeasure terminal drag cost now that `term_draw()` skips rows outside the
   active clip; the source fix exists, but this note does not claim a fresh boot
   measurement for it;
2. fix cache lifecycle on mode changes;
3. stop the compositor's idle busy-spin with the intended `hlt` path;
4. replace uneven 100 Hz PIT pacing with a measured deadline/vblank design;
5. keep measuring `late`, not just average and peak.

Full evidence and the pacing analysis are in `look-and-speed.md`.

## How future sessions should use the prototype

1. Compare the prototype and a fresh zlOS screenshot side by side at the same
   viewport and content.
2. Name the visible difference precisely: type scale, spacing, colour role,
   icon filtering, motion, alignment, clipping or latency.
3. Fix the shared primitive or theme role, not each caller independently.
4. Gate appearance with pixel evidence and interaction with pointer/keyboard
   probes. A still screenshot cannot prove a dock works.
5. Re-run frame measurements after the visual change. A prettier frame that
   misses the panel deadline is not finished.

## The durable verdict

zlOS can reach this level of visual quality and responsiveness with the system
that exists now. The hard renderer/compositor foundation is already present.
What remains is a focused visual pass, performance work on the paths people
actually feel, toolkit-to-zl wiring, and then applications built on top.

Do not answer this question with one completeness percentage. Say which layer
is being judged: **look, feel, machinery, or applications**.

Related evidence: `look-and-speed.md` · `desktop-v10-plan.md` §8 ·
`desktop-northstar-feasibility.md` · `DECISIONS.md` #29–#33 ·
`../../docs/STATE-OF-THE-PROJECT.md`
