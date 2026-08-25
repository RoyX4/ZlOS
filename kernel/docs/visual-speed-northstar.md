# zlOS visual and speed north star

Recorded 2026-08-19 from Roy's clarification. This is the intent behind pointing
at `~/zl OS v10.dc.html`.

## Implementation status — 2026-08-19 isolated visual pass

The concrete shared-system work in this document is implemented on the isolated
`codex/visual-northstar-v2` branch. It was deliberately built outside the shared
checkout so it cannot overwrite another agent's in-progress files.

Implemented and verified:

- one live navy/cyan theme owns semantic colours, chrome, wallpaper roles and
  design metrics; `kernel.zl` consumes roles instead of carrying a second UI
  palette;
- continuous q8 UI scale and resampled text/icons replace whole-number layout
  jumps; the final icon-atlas entry is reachable and the next index is refused;
- the wallpaper keeps the glows, sweeps and vignette and adds deterministic
  fine grain without adding per-frame work;
- zl now has bindings for the existing immediate-mode widgets, rows, lists,
  scrolling and theme metrics; Start, Pointer, System Monitor and About use the
  shared layout path, while Settings and Browser retain their existing toolkit
  paths;
- open, close, press, pulse and fade motion is elapsed-time based with bounded
  fixed-point easing, so late frames skip forward instead of slowing motion;
- the incorrect fade-backdrop origin is fixed;
- edge dragging draws a real destination preview, and drag, double-click and
  Super+Arrow now share one work-area-aware snap/restore state;
- the wallpaper cache is full-resolution dithered RGB565. 3840x2160 uses
  16,200 KiB and fits the existing 16 MiB graphics arena without changing the
  fixed memory map; an obsolete competing blur still refuses loudly;
- calibrated-TSC deadlines replace PIT cadence when available, the Intel path
  waits on the existing vblank source when it is actually supported, and the
  compositor loop executes `sti; hlt` instead of busy-spinning at rest;
- QEMU probes fall back to TCG when `/dev/kvm` is unavailable, so boot and
  screenshot evidence remains reachable in sandboxes.
- the boot workspace is now a deliberate three-app composition rather than a
  full-screen shell plus debug cards: quiet 36 px chrome, 16 px radii, a compact
  dock, centered titles and circular close controls;
- 1920x1200 is a true 1x desktop instead of crossing the old 1400 px cliff into
  a 16x32 terminal and oversized controls;
- boot diagnostics remain complete on COM1, while the visible Terminal clears
  to a concise ready state once the graphical session starts;
- System Monitor has real Resources hierarchy and owns the frame/peak/late
  diagnostics that previously polluted permanent dock chrome.

Evidence from this branch:

- kernel link: zero undefined symbols, 1,718,100 bytes;
- `fbtext`: 48 checks, 0 failed, including density-aware q8 scale and icon
  19/20 bounds;
- `wmtest` and `wmtest_feel`: 0 failures, including preview pixels, unified
  restore state, duration-based motion and work-area snapping;
- `walltest`: 0 failures at 1920x1200, 2560x1440 and 3840x2160;
- BIOS/QEMU boot reaches `ready.` at 1920x1200 and produces a visually inspected
  screenshot with the final Terminal, Browser and Resources composition;
- the native compositor benchmark measures a 4.319 ms dragged frame at
  1920x1200, inside the 16.67 ms 60 Hz budget;
- the resize probe passes resize (6,084 changed samples), move (29,159) and
  no-ghost (25 residual samples) checks.

Not honestly proven yet:

- TCG timing is not target timing. The final `late = 0` / peak-under-16.67 ms
  claim still requires the ThinkPad or KVM, and the Intel vblank path requires
  the real Gen9 pipe;
- this pass does not turn every simulated HTML application into a complete
  real application. Application completeness remains the separate track this
  document defines below;
- there is still no runtime vector-font rasteriser. Continuous intermediate
  sizes are resampled from the built-in raster atlases.

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

## Why the current desktop still feels blocky

The renderer is capable of smooth pixels, but the current composition often
reads as large blocks placed on a grid. These are the concrete contributors:

- the wallpaper's broad dark regions are visually obvious; the glows, sweeps
  and vignette exist, but their composition still needs an art-direction pass
  so the background reads as one flowing field;
- UI scale changes in whole-number steps rather than adapting continuously;
- fonts are build-time atlases at a few fixed sizes, not continuously scalable
  runtime vector text;
- several windows contain large flat areas with strong rectangular boundaries;
- much of `kernel.zl` manually positions rectangles and text instead of using
  the shared layout/widget toolkit;
- icons, spacing, colour roles and corner treatment are not yet completely
  consistent, and only 10 of the 20 generated 24 px icons are currently
  reachable because `fb.c` still declares `ICON_N` as 10;
- animation uses a few discrete frames, while compositor work is released by a
  100 Hz PIT against a 59.998 Hz panel, so motion can be uneven even when an
  individual frame renders quickly;
- terminal drawing historically dominated drag frames. `term_draw()` now skips
  rows outside the active damage clip, but that path still needs a fresh
  measurement before the old hitch is declared closed.

Do not pretend one exact cause has been proved. The earlier coarse-screenshot
investigation did not isolate a single dominant defect. Scaling, fixed font
sizes, icon resampling, spacing and composition are evidenced contributors;
which one dominates must be settled by like-for-like screenshots and
measurements.

## The intended background and visual flow

Treat the desktop as one deliberate scene rather than independent rectangles:

- a broad navy gradient establishes the overall direction;
- two or three subtle overlapping light fields create depth and guide the eye;
- a soft vignette pulls attention inward;
- a very faint structure or texture prevents large areas from feeling empty;
- restrained shadow and contrast separate windows without thick rectangular
  outlines;
- one spacing scale and smaller visual steps connect every surface;
- short, smooth motion connects one state to the next.

The gradient, light fields, vignette and cached wallpaper already exist in the
static background path. The subtle texture layer does not. The remaining task
is partly implementation and partly composition: tune the layers together at
the real display mode rather than merely proving each primitive can draw.

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

Further visual work is now iteration and real app surfaces, not missing shared
machinery. Do not describe it as blocked by the language. The recurring
distinction is:

- **the language cannot express it** — a real language/runtime constraint;
- **nobody has wired or written it yet** — ordinary engineering work.

Most remaining application work is the second kind. `ui.c` is no longer the
counterexample: its widget and layout surface is callable from zl.

## Capability map — what exists, what needs finishing, and the real limits

### Already possible with the current machinery

- flowing linear-gradient backgrounds;
- radial glows, conic sweeps and a vignette;
- rounded windows, panels and controls;
- soft shadows, alpha blending and translucent stationary surfaces;
- anti-aliased proportional and monospace text at the built-in atlas sizes;
- crisp generated icons, with a current 10-of-20 reachability bug to fix;
- hover, press, focus, selection and error states;
- opening, closing, dragging, resizing and snapping windows;
- a cached detailed wallpaper at the ThinkPad's 2560x1440 mode;
- fast interaction when damage tracking repaints only what changed.

These are not future renderer research. They are composition and finishing work
over primitives that exist now.

### Implemented here, with one remaining hardware proof

- **Smoother animation:** elapsed-time sampling and fixed-point easing.
- **Flexible UI scaling:** continuous q8 metrics, type and icon output.
- **Correct opacity fades:** immutable backdrop origin through the fade.
- **Consistent application controls:** zl bindings plus initial app migrations.
- **One coherent theme:** semantic colours and metrics in `ui_theme`.
- **Even 60 Hz intent:** calibrated-TSC deadlines plus guarded Intel vblank.

The last item is code-complete but not hardware-proven by this pass. The real
panel measurement remains a gate; a TCG screenshot cannot settle it.

### Possible, but only with the right implementation

- A stationary blurred menu can be computed once and cached. Recomputing blur
  continuously behind a moving window is too expensive on the current CPU path.
- A detailed animated background can move small cached layers. Rebuilding all
  full-screen glows and gradients every frame is over budget.
- A detailed 4K wallpaper is cached as full-resolution dithered RGB565. This
  keeps the existing memory map and avoids a 32-bit cache that cannot fit.
- GPU acceleration could make live full-screen effects cheaper, but is not
  required to reach the prototype's visible quality.

The design rule is simple: compute expensive static appearance once, cache it,
and animate or repaint only the small region that actually changes.

## Five-part working direction

This is the concrete visual-and-feel pass Roy was describing. Each item states
what exists and what remains so it cannot be misread as either "all done" or
"start from zero".

1. **Make the background flow.** The base gradient, three elliptical glows, two
   conic sweeps, vignette and wallpaper cache exist. Tune them as one scene and
   add the missing faint texture if it improves the comparison.
2. **Remove the blocky composition.** Rounded geometry, type atlases and shared
   metrics exist. The remaining work is better padding hierarchy, softer
   separation, fewer giant empty rectangles, consistent corners, flexible
   scaling and one palette source.
3. **Make movement connect states.** Open/close/press/pulse/fade mechanisms,
   hover, direct pointer dragging, resizing and snapping exist. Convert motion
   from a few frame steps to short time-based transitions, fix the fade origin,
   and add the missing snap preview. Dragging itself stays directly attached to
   the pointer rather than eased behind it.
4. **Fix what people physically feel.** Damage tracking, frame timing, miss
   counters and source-level terminal row clipping exist. Remeasure the terminal
   drag, stop idle busy-spin, and replace uneven 100 Hz release with a measured
   60 Hz deadline/vblank design.
5. **Unify the toolkit.** `ui.c` already implements layout and widgets. Expose
   those widgets to zl, migrate the raw drawing call sites, remove the duplicate
   theme and make applications share one spacing scale and component set.

The practical result uses a beautiful background rendered once, damage tracking
for moving windows, cached expensive effects, time-based 60 Hz motion, more
flexible typography/scaling and consistent shared UI components.

## Visual hierarchy and identity ideas

These are the next brainstorm, not locked implementation decisions. Preserve
them as a direction to test side by side rather than silently treating every
idea as mandatory.

1. **Four explicit depth levels:** wallpaper, ordinary windows, focused window,
   modal/menu. Give each one a defined shadow, brightness and contrast so
   inactive windows genuinely recede.
2. **Break up the giant bottom block:** test a floating dock plus a separate
   compact status island, or make the full-width surrounding bar visually
   disappear. The current bottom slab is one of the largest rectangles on the
   screen.
3. **Simplify window chrome:** near-black content, restrained navy chrome, one
   cyan focus line and a soft shadow. Use fewer outlines and reserve the
   strongest colour for the focused window.
4. **One motion language:** roughly 140–180 ms open/close, 70–90 ms press,
   gentle hover brightening and a short snap preview. Drag stays directly under
   the pointer. Errors pulse briefly; nothing bounces for decoration.
5. **Strong typography roles:** proportional UI/chrome, monospace terminal/code/
   measurements, bold only for selected or important content, and small
   uppercase text for hardware/status labels.
6. **One icon family:** shared stroke thickness, optical size, curve language
   and padding. Fix the 10-of-20 reachability bug before judging the complete
   set.
7. **Colour carries truth:** cyan means interaction/focus, green means verified
   healthy, amber means attention or unfinished, red means failure/destructive,
   everything else stays neutral. Never draw a healthy indicator without a
   live fact behind it.
8. **Applications have internal structure:** toolbars, quiet dividers, grouped
   rows, side navigation, compact charts and designed empty states. Giant empty
   panels are a major source of the blocky/unfinished feeling.
9. **A signature zlOS transition:** preserve boot output by transforming it into
   the terminal window when the compositor appears, instead of hiding startup
   history behind a generic loading screen.
10. **Make the background feel alive cheaply:** let tiny cached details respond
    to workspace, focus, activity or real warning state. Do not continuously
    rebuild the full-screen background.

Three identity directions were considered:

- **Precision instrument:** navy/cyan, crisp, dense and exact — a machine-control
  cockpit.
- **Cinematic dark desktop:** softer glows, floating surfaces and fluid motion —
  closest to the v10 HTML.
- **Neo-retro system:** black, mono, lime, scanlines and hard geometry — strong
  identity, but likely to reinforce the blockiness Roy wants removed.

Current recommendation: **precision instrument as the foundation, with the
cinematic background and motion layered over it.** It fits an OS that controls
real hardware while still allowing the softness and polish of the prototype.

## What the 4K memory limit actually means

zlOS currently reserves fixed memory regions for major graphics jobs instead
of asking a general heap for any size at runtime. Think of it as labelled
drawers rather than one open warehouse.

The wallpaper/effect drawer is 16 MiB. The cache now stores RGB565:

- 1920x1200x2 = 4,500 KiB — fits;
- 2560x1440x2 = 7,200 KiB — fits, even after the obsolete dock-blur allocation;
- 3840x2160x2 = 16,200 KiB — fits with 184 KiB left.

The renderer dithers on save and expands to RGB888 on paint. The visual cache
therefore remains full resolution without stealing another fixed region. If a
competing cached effect consumes the arena first, the existing honest gradient
fallback and refusal log remain in force.

Other valid long-term ways to handle larger future modes or more cached effects
remain:

1. reserve a larger wallpaper cache region and prove it does not overlap any
   other fixed region;
2. size the cache from the boot mode rather than using one fixed ceiling;
3. cache the wallpaper in tiles and keep only the tiles needed;
4. use a lower-resolution background and filter it when presenting;
5. retain the current honest fallback to a cheap plain gradient when caching
   is refused.

The clean long-term answer is still a boot-sized graphics cache with central
memory-map assertions. RGB565 solves the current 4K case without casually
moving a fixed boundary.

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
| dragged compositor frame, current composition | 4.319 ms | inside 16.67 ms at 1920x1200 |
| Terminal scrollback share of that frame | 1.166 ms | 27% of the dragged frame |
| System Monitor contents share | 0.111 ms | the structured app body is not the bottleneck |
| chrome and wallpaper share | 2.792 ms | 65%; the clearest remaining optimisation target |
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

The remaining speed proof should follow perception, not theoretical throughput:

1. remeasure the exact composition on the target machine; the native host
   harness is green, while QEMU TCG is intentionally not treated as target
   timing;
2. measure the new deadline/vblank path on the 59.998 Hz ThinkPad panel;
3. keep measuring `late`, not just average and peak.

Full evidence and the pacing analysis are in `desktop/look-and-speed.md`.

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
that exists now. This branch completes the shared visual pass and the software
side of the pacing pass. What remains is target-hardware timing proof and then
applications built on top, not another renderer rewrite.

Do not answer this question with one completeness percentage. Say which layer
is being judged: **look, feel, machinery, or applications**.

Related evidence: `desktop/look-and-speed.md` · `evidence/desktop-v10-plan.md` §8 ·
`archive/superseded/desktop-northstar-feasibility.md` · `DECISIONS.md` #29–#33 ·
`../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`
