# Retained window surfaces — the next real desktop speed layer

Measured on the ThinkPad on 2026-08-21. This is an implementation contract,
not a claim that the feature already exists.

## The measured problem

The write-combining framebuffer fix removed the old present bottleneck: a full
2560x1440 copy now takes about 2.07 ms instead of 131-137 ms. A later physical
session retained 1,043 compositor phase samples with no recorder drops. Its
large damaged frames visited 12-13 windows and spent about 130 ms in C chrome,
23-28 ms in zl app drawing, and 9-13 ms restoring the cached desktop; final
presentation remained about 3 ms.

The compositor already knows *where* pixels changed. It does not retain what a
window last looked like. When a changed rectangle intersects a window,
`wm_repaint()` calls `chrome()` and `hook_draw()` again for that window. Moving
one top-level window therefore makes every exposed or intersecting window
rebuild its shadows, title bar and application contents. That is the direct
reason the pointer can feel delayed after framebuffer write-combining is fixed.

## Target behaviour

```text
app state changes        -> render that one window into its RAM surface
window moves / is raised -> copy existing surfaces into the compositor back buffer
damage is presented      -> copy changed rows to the write-combining scanout
```

An unchanged, obscured window must not execute `hook_draw()` merely because a
different window moved. A surface allocation refusal must remain correct by
falling back to the current direct redraw path; it must never leave a blank or
stale window.

## Surface ownership and budget

- Surface metadata belongs to `wm.c`, one slot per open window.
- Pixel storage comes from the existing 64 MiB bounded heap, which is already
  initialized before the desktop starts and has coalescing/free tests.
- Start with 32-bit RGB in client coordinates. It avoids colour conversion,
  permits exact blits, and matches the compositor back buffer.
- A surface is `client_width * client_height * 4`, rounded and overflow-checked
  before allocation. The metadata stores byte count, dimensions, dirty state,
  and a generation number.
- Surface policy is a fixed budget below the heap's capacity. The first version
  should reserve at most 48 MiB for retained clients, leaving at least 16 MiB
  for all other heap users. A 2560x1440 full-screen client alone needs about
  14.1 MiB.
- LRU eviction is not required for the first safe version: a surface that no
  longer fits is direct-rendered until close/minimize/resize frees capacity.
  This makes failure bounded and auditable.

## Rendering seam

`fb.c` needs a narrow offscreen-target API. It must support a client-sized RAM
buffer with a screen-coordinate origin and the same clipping/drawing primitives
used today. It must restore the compositor target and clip state on every exit.
No app may retain a raw target pointer.

For an invalid client surface:

1. clip to the client rectangle;
2. bind its surface as the draw target;
3. call `hook_draw()` once;
4. unbind, mark the surface valid, and copy the requested portion into the
   compositor back buffer.

For a valid surface, skip `hook_draw()` and copy only the damage intersection.
Chrome remains direct-rendered initially. Once client retention is proved,
chrome can receive a separate cache; mixing both in the first change would make
title hover, focus, tabs, resize and shadow invalidation hard to audit.

## Invalidation contract

The current `wm_damage_win()` means screen pixels changed. It must not be
silently reinterpreted as application-content invalidation. Add a separate
`wm_invalidate_client(win)` used only when the app's own pixels changed.

Client invalidation is required for:

- keyboard or pointer edits handled by the app;
- app ticks that return changed;
- scroll, resize, tab selection, app launch/reopen and explicit app redraw;
- mode/scale changes and any colour/font resource rebuild.

Client surfaces stay valid across move, raise, focus change and another window
covering/uncovering them. Resize frees and reallocates the surface; close and
minimize free it. Animations that scale/fade a window bypass retention for that
window until settled, then rebuild one surface, because drawing a scaled cached
client without a resampler would be visually wrong.

## Correctness gates

1. Host framebuffer test: direct draw and surface draw produce byte-identical
   client pixels for a deterministic app.
2. Host compositor test: move/raise an unchanged window and assert the covered
   window's app-draw count is zero while its on-screen pixels stay unchanged.
3. Resize, close, workspace switch, tab switch, focus change and allocation
   refusal tests: no stale pixels and every heap allocation is freed.
4. Heap audit after repeated open/move/resize/close loops: no live surface
   blocks and heap validation succeeds.
5. UEFI/QEMU gate: boot, desktop, pointer drag and clean journal remain green.
6. Physical ZLLOG rerun: compare window visits, app calls, compositor p50/p95,
   missed deadlines and input-to-present latency against the 2026-08-21 trace.

The first physical success criterion is not a marketing frame rate. It is a
trace proving an unchanged underlying app was not redrawn while another window
moved, with a visibly responsive pointer and zero recorder drops.
