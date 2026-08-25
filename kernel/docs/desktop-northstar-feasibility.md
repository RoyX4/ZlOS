> **AUDITED 2026-08-19 · PARTLY OPEN.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. Its one verified finding still holds exactly: `ui.c` is a complete widget toolkit with zero zl bindings, so every app in `kernel.zl` draws with raw primitives. Keep the gap list and the measurements; ignore the percentage — four successive numbers have each been wrong.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**

# Can zlOS run the v10 mockup?

The reference: `~/zl OS v10.dc.html`, 216 KB — a browser mockup of a zlOS
desktop. Not a literal target, a **direction**.

> **This document has now been wrong twice, in opposite directions, and both
> corrections are worth keeping because they are the same mistake with the sign
> flipped.**
>
> On 2026-08-17 it said **95%**. That counted *visual effects* — gradients,
> blur, rounded corners — asked whether `fb.c` could draw each one, and ignored
> everything underneath. Roy called it out.
>
> Later the same day it said **20%**, and that is the number this rewrite
> replaces. It was arrived at by counting two things as **permanent**: "zl has
> no lists" and "there is no layout engine". Neither was a property of the
> language. `ui.c` is a layout engine and it exists; `ui_list_row` expresses a
> list without a list *type*. Those were **"nobody has written it yet"**
> dressed up as **"the language cannot"**, which is the single most expensive
> confusion available on this project — it costs you the wrong fix.
>
> **Then it said ~65%, and that was too generous.** It counted the toolkit as
> "built, 90%" without checking whether the layer that writes apps can reach
> it. `ui.c` has **no zl bindings at all** — `ui_scale` and `ui_theme` are the
> only `ui_*` builtins and neither is a widget. Every app in zlOS lives in
> `kernel.zl`, so not one of them can call `ui_button`, `ui_slider`,
> `ui_list_row` or `ui_scroll`. The start menu had to be drawn with raw
> primitives, which is how this was found.
>
> **The number is ~35%.** See §3.

---

## 1. What the mockup actually asks for, counted from its source

Counted, not estimated. `grep -c` on the file.

| Feature | Count | Status | Where |
|---|---|---|---|
| `rgba()` transparency | **107** | **done** | `fb_fill_blend`, `fb_rrect_blend` |
| `border-radius` | 34 | **done** | `fb_rrect`, radius now 12 to match |
| `position:absolute` | 32 | **done** | everything is absolute already |
| `transform` | 29 | **mostly done** | scale is the animation timeline; translate is free |
| `font-family` (Public Sans + Roboto Mono) | 24 | **done** | proportional atlas, 3 sizes × 2 weights |
| `animation` / `@keyframes` | 22 / 7 | **done** | `wm_anim`, five kinds |
| `display:flex` + `grid` | 63 + 13 | **in C only** | `ui.c`'s flowing cursor — no zl binding |
| `box-shadow` | 11 | **done** | `fb_shadow`, three elevations |
| linear gradients | 7 | **done** | `fb_gradient`, dithered |
| radial gradients | 4 | **done** | `fb_grad_radial`, elliptical, two-alpha |
| conic gradients | 2 | **done** | `fb_grad_conic` |
| `sc-for` list loops | **106** | **in C only** | `ui_list_row` exists and zl cannot call it |
| `sc-if` | 84 | **done** | that is `if` |
| `blur` | 9 (10–34 px) | **done, with a rule** | `fb_blur_cache` — §4 |
| `onClick` / `onPointerDown` | 181 | **mechanism done** | `wm_route` → `app_event`; the handlers are per-app work |
| **13 full applications** | 13 | **~1.5** | the real remaining work, and it is most of it — §3 |

**The two claims that produced the 20% were both false.**

- *"zl has no lists"* — true, and irrelevant. A list **type** is not needed to
  draw a list. `ui_list_row(text, selected)` called in a `while` over data the
  app already holds is exactly what 106 `sc-for` loops compile down to. Snake
  keeps its state in raw memory and has done since before any of this.
- *"there is no layout engine and it is not designed"* — `ui.c` is one. It is
  immediate-mode with a flowing cursor rather than a retained tree, because a
  tree needs a heap and *is* a list of children. That is not a lesser thing; it
  is the same thing chosen to fit the constraints.

---

## 2. The honest breakdown, again

| Layer | zlOS status |
|---|---|
| **Renderer** — pixels | **~95%.** Real TrueType shapes, subpixel LCD, gamma-correct linear blending, dithered gradients, translucency, radial and conic gradients, a cached blur. Most hobby OSes have none of it. |
| **Compositor** — windows, z-order, damage, routing | **built and shipping.** `wm.c`, 69 assertions in `wmtest`. It is the boot state. |
| **Toolkit** — layout, widgets, events | **built in C, unreachable from zl.** `ui.c` has label, bar, button, toggle, slider, num, list row and scroll, 90% complete and asserted — and **zero** builtins expose it. Apps live in `kernel.zl`. From where the apps are, the toolkit does not exist. |
| **Apps** — the prototype has thirteen | **~10%.** A shell, a System Monitor of three lines, an identity card, and snake. |

**The toolkit line is the one that changed this document's answer.** `ui.c` is
real, complete and asserted — and it is C, and every app is zl, and nothing
bridges them. It was counted as done because it exists. That is the same
mistake as the 95%, one layer up: measuring what was *built* rather than what
is *reachable*.

---

## 3. What the number is, precisely

Weighted by the work each layer represents rather than by how much of the
screen it occupies:

| Layer | Weight | Done | Contribution |
|---|---|---|---|
| Renderer | 25% | 95% | 23.8 |
| Compositor | 20% | 95% | 19.0 |
| Toolkit | 20% | **10%** — built in C, no zl bindings | 2.0 |
| Apps | 35% | **10%** | 3.5 |
| | | | **~48%** |

...and even that flatters it, because the app figure is measured against "seven
demos to convert" rather than against what the mockup actually contains. Its
own template model names **thirteen applications with real state**:

| the prototype has | zlOS has |
|---|---|
| a file manager — breadcrumbs, a tree, mounts, icon/list views, properties, rename, search | — |
| a hex viewer — paged, with a status line | — |
| a code editor — tabs, a gutter, syntax modes, find/next/prev with a match count | a line editor, full-screen, ESC to save |
| a terminal — tabs, a cwd, geometry, a scan/run/reset toolbar | a shell in a window ✓ |
| a system monitor — a process list, kill, tabs, a CPU graph, meters | three lines and a fake sparkline |
| a log viewer with filters | — |
| settings — nav, toggles, sliders, accent picker | — |
| a lock screen | — |
| an activities overview — apps, windows, workspaces, with search | — |
| a command palette | — |
| a calendar | — |
| a GL view — orbit, spin, zoom, wireframe, a render-ms readout | a full-screen spinning cube |
| a framebuffer test with counters | — |
| plus: workspaces, window snapping, toasts, context menus, dialogs, a clipboard | — |

**Call it 35%.** The renderer and the compositor are genuinely most of the way
there and the two layers above them are barely started. That is the honest
shape, and it is the opposite of what a screenshot suggests.

**This is a number to be argued with, not trusted.** The only part of it that
is measured is "what does the mockup ask for and does the primitive exist" in
§1. The rest is estimation, and it is labelled as such because the last two
versions of this document were confident and wrong.

---

## 4. Blur is a budget, not a barrier — and the number changed

Re-measured on the shipping `fb.c` (`hosttest/fbbench`, 1920×1200, i7-10510U):

```
BLUR 600x460 r=20 (cold)     7.368 ms     61.50 cyc/px
...cached, painted           0.184 ms      1.54 cyc/px
```

The old figure in this document was 15.9 ms for the same rectangle. The
difference is one change: the running sum was divided by the box width three
times per pixel per pass, twelve integer divides per pixel across four passes.
The width is constant for a whole call, so it is now one reciprocal and a
multiply. **5.4× faster, identical pixels.**

| case | verdict |
|---|---|
| blur behind a panel that opens and **sits still** | **yes** — 7.4 ms once, 0.18 ms per frame after |
| blur behind a **moving** window, per frame | **no** — still 44% of a frame, every frame |
| the two big background decorations | **yes** — once at boot, into the wallpaper cache |

**The design consequence is a UI decision and it is taken:** a panel that blurs
must not be draggable, or must re-blur only on drop.

### The wallpaper had to become a bitmap, and that is a measurement

The v10 background is a gradient plus three elliptical glows, two conic sweeps
and a vignette — six translucent full-screen passes. A translucent pass costs
**22 cyc/px measured**, about 22 ms at 1920×1200, and the compositor redraws
the wallpaper inside *every* damage rectangle. Six of them per frame is 130 ms
against a 16.67 ms budget.

So the wallpaper is drawn once into a cache and blitted: **1.5 cyc/px**. There
is no version of this that is affordable per frame. Caching is not an
optimisation here, it is the only way the look exists at all.

It costs one screen-sized buffer out of the 16 MiB arena that C4 freed — 9 MiB
at 1920×1200, 14.4 MiB at 2560×1440, and **more than the arena holds at 4K**,
where it refuses, says so on the boot log, and falls back to the plain
gradient.

---

## 5. What is actually left

Not "a layout engine, a widget toolkit, an animation system and 10 apps". Those
first three exist. What is left:

1. **Converting the seven full-screen demos into apps** — `snake`, `paint`,
   `cube`, `anim`, `editor`, `mousedemo`, and the modeset viewer. Each is three
   functions and no loop of its own (`app_draw` / `app_event` / `app_tick`).
   Snake is the one to do first: its state is already in raw memory.
2. **The apps the mockup has that zlOS does not** — its own markup names a file
   manager (Places, Devices, Properties, Rename), a task manager (End Process,
   CPU, Update interval), settings (Scanlines, Accent colour) and a lock screen
   (Unlock). Four applications, each on top of a toolkit that now exists.
3. **The start menu as a `WF_MODAL` window** — the mechanism is in `wm.c` and
   has no caller.
4. **A real fade.** `wm_anim_alpha()` reports it and `wmtest` asserts it, but
   nothing composites it: a fade needs the window drawn against what is
   *behind* it at fractional opacity, which needs a copy of the rectangle taken
   before the window was drawn on it. The arena can hold one. `ANIM_PULSE` is
   composited today because a tint needs no such copy.

None of that is a research problem. It is app work, which is what this document
should have said the first time.

---

## 6. The honest verdict

| Question | Answer |
|---|---|
| Could one zlOS window look like one of those windows? | **Yes**, and does |
| Could zlOS run that whole desktop? | **Not yet** — but what is missing is applications, not machinery |
| Is any of it impossible? | **No.** Nothing here is a research problem. |
| Is the renderer the bottleneck? | **No.** It is the most finished part, and always was. |
| Is "zl has no lists" a wall? | **No.** It was never even a fence — see §1. |

The useful way to hold the mockup is unchanged: **a direction, not a
milestone.** What changed is that pulling one thing at a time out of it is now
cheap, because the layer underneath each thing exists.

---

Rendering bugs: `desktop-look.md` · Speed baseline:
`desktop-polish-and-speed.md` · Task list: `archive/superseded/desktop-TODO.md` · The v10 plan
this rewrite came from: `evidence/desktop-v10-plan.md`
