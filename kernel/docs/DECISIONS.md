# Decisions log — the desktop/graphics work

Every decision taken on 2026-08-17, in one page, newest thinking last.
The detail lives in the other docs; this is the index of *what was decided and
why*, including **the things that turned out to be wrong.**

---

## Architecture

**1. Mechanism in C, policy in zl.**
The window table, z-order, hit-test, focus, damage and the repaint loop go in a
new `wm.c`. The theme, layout, what each app draws and what the dock launches
stay in `kernel.zl`.
*Why:* the zl kernel subset **has no lists** (`kernel/README.md:154`,
`zl_list_n` is a hard fault). A window table is a list. This is not a style
choice — the language cannot express it. Same line X11 and Wayland draw.
→ `desktop-plan.md`

**2. z-order is an index array. Nothing else.**
Iteration order *is* paint order. Raise = remove + append. Hit-test walks it
backwards. Replaces the current hand-unrolled if-ladder over two windows.

**3. Event routing has exactly three modes, checked in order:**
pointer grab (a drag owns everything until button-up) → modal (menu open) →
normal (pointer to topmost containing window, keys to focus).
Focus is separate from pointer-over. Click-to-focus.

**4. An app is an integer ID plus three callbacks, and no loop of its own.**
`app_draw` / `app_event` / `app_tick`, dispatched by an if-ladder — the idiom
`run_command` already uses.
*Why:* no heap, no processes. And it is what removes "press any key to exit" —
that phrase exists because every demo owns a `while` loop today.

**5. Do NOT wire `sched.c` into the desktop.**
It works, but preemptive tasks sharing one framebuffer with no memory protection
and no locks is a data race with extra steps.

**6. The clip rectangle is the keystone, and it is ~~two functions~~ FIVE.**
*Corrected 2026-08-18, by measurement.* The claim was that `fb_fill_px` and
`put_pixel` are the only places in `fb.c` that clamp, so changing those two
makes the whole library clippable. **Three more functions write the back buffer
directly and call neither:** `draw_glyph`'s subpixel and AA fast paths,
`fb_gradient`'s `back_on` branch, and `fb_scroll`. With only the two changed,
**2,184,000 pixels escaped the scissor at 1920×1200** — the wallpaper and every
glyph, i.e. most of a desktop.

It looked correct at 3840×2160, and that is the instructive part: `back_on` was 0
there, so those three fell back to `put_pixel`, which *was* clipped. A gate that
only ran at 4K would have passed a broken keystone.

> **The 4K half of that paragraph is now HISTORICAL — 2026-08-19.** `back_on` is
> **1 at every mode the desktop can be handed**, including 3840×2160: the rule is
> `w*h*4 <= 40 MiB`, i.e. ≤ 10,485,760 px, and 4K is 8,294,400. Verified by
> running `fbbench`, which prints `back ON` at all three of its modes. The
> lesson stands and the sentence no longer describes the tree — do not re-derive
> "a 4K gate exercises the fallback paths" from it, because today it exercises
> the same five back-buffer paths as every other mode.

All five now fold the scissor into their **loop bounds** rather than testing per
pixel, so clipped drawing stays as fast as unclipped — which matters, because
the compositor's hot path is drawing many small clipped rectangles.
*Gate:* both halves, in `hosttest/fbbench.c`. (a) with the scissor at full
screen the scene hash is unchanged at all three resolutions — it broke nothing;
(b) with a scissor set, **zero** pixels escape it across fill, gradient, rrect,
shadow, AA text, icons and lines, tested at each of the four edges — it does
something. (a) alone would be passed perfectly by an `fb_clip()` that did
nothing at all.

**7. Delete the snapshot-and-sticker drag machinery** once damage-based repaint
exists. It takes the 640×480 window ceiling, the shadow halo artifact and 10 MB
of fixed buffers with it. **Keep** the 11×17 cursor save-under — that technique
is correct.

**8. The shell becomes app 0.** No special path. `fb_set_text_box` already
confines it, and `LINE_BUF`/history are already external, so `read_line`
becoming a state machine is smaller than it looks.

**9. Build order is forced:**
resolution cliff → clipping → damage → mouse events → *then* touchpad → *then*
compositor. The first four need neither the laptop nor a pointer.

---

## Graphics and speed

**10. The renderer is not the problem.** Real TrueType via FreeType, subpixel
LCD with a 5-tap FIR, gamma-correct linear-light blending, dithered gradients.
~80% of the way to modern. It is also the *smallest* layer.
→ `desktop-look.md`

**11. Three resampling bugs, not a bad renderer.** Worst: `fb_icon24`
nearest-neighbour upscaling every icon at 2× (`fb.c:929`) — icons are drawn as
geometry at 96×96, filtered to 24×24, then blown back up by pixel-copying.

**12. `fb_shadow` fixed and SHIPPED.** It darkened the window's whole footprint,
then the caller drew the window over ~90% of it.

| | before | after |
|---|---|---|
| shadow 600×460 | 4.34 ms | **0.61 ms** |
| one window | 5.12 ms | **0.90 ms** |
| whole desktop @1920×1200 | 19.98 ms | **4.88 ms** |

Verified pixel-identical (FNV hash of the whole back buffer) and `verify.sh`
passes.

**Re-measured 2026-08-19, after the five-function scissor fix (#6) changed the
picture and nobody had re-timed it.** `fbbench` at 1920×1200 on this box:
`shadow 600x460 soft=12` = **0.626 ms** (4.78 cyc/px), `ONE WINDOW (full
chrome)` = **0.787 ms**. Same harness, same `REPS 7`. So the numbers above are
**unchanged, not improved** — the scissor fix cost the shadow nothing and gained
it nothing, which is the honest reading and not the one a single fast run
suggests. The claim "fb_shadow is 4.3 ms of a 5.1 ms window redraw" is dead
either way: it is now ~4% of a 16.67 ms budget and **is not where frame time
goes**. See `look-and-speed.md` for where it does go.

**13. Measurement is a build artifact now.** `hosttest/fbbench.c` compiles the
shipping `fb.c` at the kernel's own `-O2`; `hosttest/gpu_fillrate.c` measures the
real GPU offscreen. Both in `build.sh`.

**14. Speed is not the problem either.** 4.88 ms of a 16.67 ms budget at
1920×1200. GNOME redraws ~2% of the screen per frame; zlOS redraws 100%. That
is ~50× of avoidable work, and it is software, not hardware — the CPU is only
~4× off the integrated GPU on plain fills, because they share the same memory.

**15. Blur once when a panel opens, never per frame.** Measured 8.7 ms for one
menu-sized backdrop blur — over half a frame. The two big *background* blurs are
free: 304 ms once at boot.

**16. Motion is the biggest visual gap, and it is gated on damage tracking.**
A fade over 4 frames means compositing 4 times. Affordable only when that
damages a rectangle instead of the screen. **Speed first, then motion.**

---

## GPU and 3D

**17. Do not write a GPU driver.** Measured: `i915.ko` is **11.2 MB**, Mesa's
Intel Vulkan driver **24.3 MB**, the entire zlOS kernel **1.07 MB**. i915 is
~100K lines against 11,374 hand-written in all of zlOS.

**18. Porting one is closed too.** FreeBSD runs i915 only via **LinuxKPI** — it
emulates the Linux kernel API rather than porting the driver. Doing that here
means building a Linux-shaped kernel first. i915 assumes GEM, TTM,
dma-buf/dma_fence locking, workqueues and a heap; zlOS has none by design.

**19. Everyone with GPU acceleration got it by porting Mesa** — Haiku (RADV +
Zink, NVK), Managarm, Fuchsia (Magma), Genode. Mesa needs POSIX. Closed.

**20. `intel.c` already has the correct relationship: borrow Linux's knowledge,
not its code.** Keep it.

**21. 3D goes through a software rasterizer.** SerenityOS runs **Quake III** on
LibSoftGPU — 16×16 tiles, barycentric coordinates, SIMD. `fb3d.c` is step one.
Order: `fb_clip` → tiled rasterization → SIMD → depth buffer → textures.

**22. SSE is on and nothing uses it.** `cpu.c` detects it, the 64-bit build
enables it. `fb.c`'s blend loops are the first customer, before any 3D.

**23. Three of four cores are parked** in `cli; hlt` (`smp.c:79`). Drawing into
disjoint horizontal bands of the back buffer needs no lock. A real 4×, after
damage tracking, not before.

**24. `virtio_gpu.c:314` disables virgl on purpose.** Enabling it gives real 3D
**in QEMU only** — nothing on the laptop, which has no hypervisor. Worth
knowing; not worth doing while the laptop is the target.

---

**28. The toolkit is immediate mode, and the constraints chose it.**
Not a retained widget tree (Qt / GTK / SerenityOS LibGUI) — those allocate an
object per widget and hold parent/child pointers. **A tree needs a heap, and a
tree of children is a list.** zlOS has neither. `ui_button("OK")` returns whether
it was clicked; nothing is allocated; state stays in the app where it already is.
→ `desktop-toolkit.md`

**Three sub-decisions that follow:**

- **A theme struct holds every colour and metric.** Stolen from SerenityOS.
  Kills the hand-picked spacing numbers. Scale is 4/8/12/16/24 × `ui()` and
  nothing may use a literal.
- **A flowing cursor is the entire layout algorithm.** Widget asks for a size,
  is placed, cursor advances by size + gap, rows wrap at the content width. No
  tree walk, no constraint solver.
- **Hit testing re-runs `app_draw` with drawing switched off** — the same trick
  `intel_modeset_dry()` uses for the 35-step sequence. **Consequence: widgets
  must *return* whether they fired and never take an action as an argument**,
  because C evaluates arguments eagerly. That is the same language behaviour
  that forced `MS_STEP` to be a macro rather than a flag.

**Known tension, flagged not hidden:** immediate mode normally redraws every
frame, which fights damage tracking. `app_draw` must only run when the window is
damaged. Workable, and the first thing likely to be got wrong.

## Things I got wrong, and the correction

**25. "Table the shadow divide" — WRONG.** Recommended from a static instruction
count. Measured it: **25% slower**, three runs out of three. The loop is not
arithmetic-bound. The real problem was algorithmic (see #12).
*Lesson: an instruction count is not a measurement.*

**26. "The v10 mockup is ~95% achievable" — WRONG.** Counted visual effects and
ignored the toolkit underneath. Real figure ~20%. The mockup has **106 `sc-for`
list loops** (zl has no lists) and **105 flex declarations** (zlOS positions
everything by hand). Corrected in `desktop-northstar-feasibility.md`.
*Lesson: judging a stack by its most finished layer.*

**27. `GRAPHICS_PLAN.md` is stale for zlOS** — it is the **Windows-hosted** plan
and says the GPU is reached via `opengl32.dll` FFI. Annotated in place. zlOS
reached its layer 3 by *deleting* the OS underneath, not by calling into one.

---

## The exec track — running code the kernel was not built with

### #E1 | The execution level: **ASSUMED Level 1** (zlOS runs zl)

`EXEC-PROMPT.md` §1 puts three incompatible operating systems on the table and
says Roy chooses. No answer was given, so the brief's own escape hatch applies:
proceed under a stated assumption. **Level 1 is assumed** — load a `.zl` source
file and interpret it, ring 0, with the interpreter's memory confined to the
arena. Not Level 2 (flat binaries, which the brief rejects as an end state), not
Level 3 (ring 3, TSS, per-process page tables, and every driver becoming a
syscall).

**Reversal cost, stated up front so it can be spent knowingly:** Items 0 and 1 —
the arena and the `run` command with its error paths — are level-independent.
Level 3 needs a memory budget just as much and needs the same failure modes. So
**this is free to override until Item 2 begins**, and after that costs whatever
`interp_kernel.c` contains.

### #E2 | The program arena goes at 8 MiB, below the high-RAM map, not above it

Measured, not reasoned: **no gate in this project passes `-m` to QEMU**, and
QEMU's i386 default is exactly 128 MiB (`query-memory-size-summary` →
`base-memory: 134217728`). So the entire high-RAM map is unbacked on every gate,
and a new fixed buffer placed above it would link, boot, pass review, and never
execute. `kernel/docs/memory-map.md` has the full re-grepped map, the arithmetic,
and the two collisions found while doing it.

### #E3 | An arena with a reset is not a heap, and the boot log still says so

`kernel.zl`'s `[ INFO ] no heap, no filesystem, no scheduler` was left standing.
A bump allocator with no `free()` and no reuse inside a run has nothing to
fragment, nothing to double-free, and nothing to leak that a reset does not
reclaim. **The line stops being honest the day the interpreter boxes lists and
strings** (`EXEC-PROMPT.md` §8), and that is the change that has to carry it —
not this one. The arena instead prints its own line with an address in it, the
way `fb.c` does, because "the arena is up" is a claim and "16 MiB at 8 MiB, ends
at 24 MiB, ceiling 128 MiB" is a fact somebody can check.

### #E4 | Refusals print, but the volume is bounded

Item 0 requires that exceeding the ceiling is "a refusal that prints" — this
project has shipped a silent fallback twice and paid for it twice. But an
unbounded print from a program looping on a refused allocation pins the machine
writing to a 115200 baud serial line, which is itself a way to wedge it, and
Item 2 forbids exactly that. So the first eight refusals print in full, then one
suppression notice, then silence — while the **count stays exact** and stays
reachable through `arena_refused()`. Suppression hides volume, never the fact.

---

---

## The look, measured against the northstar — 2026-08-19

`LOOK-AND-SPEED-PROMPT.md` named three places where the kernel and
`docs/design/zlOS-design-northstar.html` disagree, and asked for a decision per
item rather than a fix. All three were shipped as v10 deliverables and gated
green, so none of them is a bug. Here is which one wins, and why.

**The tie-break that decides all three:** the northstar's rules are not taste,
they are a **cost argument** — its own header says every choice is "constrained
to what an 800x600 linear framebuffer with no GPU can actually draw". So for
each rule the question is whether the cost it protects against is real *in this
tree*. Where it is, the northstar wins. Where measurement shows it is not, the
code wins and the reason is recorded here so the next person does not re-open it.

### #29 | Blur — **the northstar wins.** Removed, and the reason is not aesthetic

> "alpha blends but **no gaussian blur**"

The kernel ran a cached two-pass box blur ("two boxes ~ a Gaussian") behind the
dock at boot and behind the start menu on every open. Removing it was not a
close call once the memory was measured:

**The blur and the wallpaper cache come out of the same 16 MiB arena, and the
blur took its slot FIRST.** `fb_cache_reset()` has no callers, so the bump
pointer never rewinds — whoever asks first wins, and `wm_boot_start` asked for
the blur before `wall_save()`. Measured against the shipping `fb.c` by
`hosttest/walltest.c`:

| mode | blur first | wallpaper cache |
|---|---|---|
| 1920x1200 | 1920 KiB | **cached**, 5464 KiB spare |
| 1920x1200 | none | **cached**, 7384 KiB spare |
| **2560x1440** | **3840 KiB** | **REFUSED** — wants 14400, 12544 left |
| **2560x1440** | none | **cached**, 1984 KiB spare |

2560x1440 is the ThinkPad X1 Carbon Gen 8's panel (`gen9-modeset-plan.txt`:
"2560x1440 active / 2720x1481 total"). So **on the only real machine this OS
targets, the blur was disabling the wallpaper cache** — and `fb.c`'s own comment
on that cache is: *"There is no version of this that is affordable per frame;
caching is not an optimisation here, it is the only way the look exists."*
fbbench agrees about the stakes: one radial glow at 900x700 is **12.2 ms** and
the wallpaper has three of them plus two conic wedges.

**QEMU could never have shown this.** Both 1920x1200 rows above pass. The gate
that would have caught it did not exist because the mode cannot be booted here —
GRUB falls back to 800x600 on the emulated card and `kernel.zl`'s own `set_res()`
ladder tops out at 1920x1200. Two boots with `ZLOS_GFXMODE=2560x1440,auto` both
came up 1920x1200. `hosttest/walltest.c` exists because of that: it is the only
thing in the tree that exercises the panel's real mode.

What replaced it, in both places, was already written:

- **dock** — the `else` branch that was sitting under the blur is
  `grad_rgb(0, dy, w, px_h() - dy, BAR_TOP, BAR_BOT)` plus a `BAR_HI` top line,
  which is byte-for-byte the northstar's `.dock` rule
  (`linear-gradient(180deg,var(--bar-top),var(--bar-bot))`, `border-top:1px
  solid var(--bar-hi)`). No new drawing code.
- **menu** — an **opaque** panel, `--panel #05060a` inside a `--line #26304a`
  hairline, which is the northstar's `.win`. Keeping the 205/255 tint over an
  *unblurred* desktop would have been the worst of the three options: an alpha
  wash over sharp text reads as a rendering fault, not as translucency. The
  hover wash moved to the reference's own number, `rgba(96,210,235,.16)`.

`fb_blur_cache` and its slots stay in `fb.c`. They work, they are gated by
`fbbench`, they allocate nothing unless called, and if this decision ever
reverses the code should not have to be rewritten. They now have no caller.

> **TENSION, flagged rather than resolved — Roy's call.** This decision was
> taken against the northstar as the spec, which is how
> `LOOK-AND-SPEED-PROMPT.md` framed it. `visual-speed-northstar.md`, written the
> same day from Roy's own clarification, says the reference is the **v10
> prototype's level of finish** and that the useful thing to take from it is
> "polish, hierarchy, spacing, motion and responsiveness". The v10 prototype
> wants nine blurs. So the two briefs pull opposite ways on this one item, and I
> removed an effect.
>
> **The engineering half is not in tension and does not depend on which brief
> wins:** blur-then-wallpaper does not fit 16 MiB at 2560x1440. Something has to
> give at that mode. There are three ways to give, and only the first is
> implemented:
>
> 1. **No blur** (today). One look on every machine. Costs the effect.
> 2. **Reserve the wallpaper first**, then let the blur be refused when it does
>    not fit. Needs an `fb_wall_reserve()` in `fb.c` that allocates without
>    copying, because the *drawing* order is forced — the blur must be captured
>    before `draw_dock_bg` paints over it, and `wall_save` must run after. Gets
>    blur at 1920x1200 and no blur at 2560x1440, i.e. **the desktop looks
>    different on different machines**, which is the thing
>    `visual-speed-northstar.md` calls "one OS rather than separate drawing
>    demos".
> 3. **Grow the arena** — move `HI_NVME` up. 16 -> 20 MiB buys both at 2560x1440
>    with room. `memmap.h` has the `_Static_assert`s to make that safe, and
>    `walltest.c` is where the new numbers would be asserted.
>
> If the blur is wanted back, **(3) is the one to take**, not (2). Say so and it
> is a memmap change plus reverting two hunks in `kernel.zl`.

### #30 | Radial and conic gradients in the wallpaper — **the code wins**

> "**vertical gradients only**"

The rule's premise is cost, and here it does not bind: the glows and wedges are
drawn **once** into the wallpaper cache and never again. Per frame they cost a
blit at ~1.5 cyc/px, the same as a flat colour would.

Three further facts, each of which alone would settle it:

1. **The northstar breaks its own rule** — line 132, `.clockwin .win-body`, is a
   `radial-gradient`. A rule its author did not keep is a preference, not a
   constraint.
2. **The fallback already IS the northstar's wallpaper.** When the cache is
   refused, `desk_draw` paints `grad_rgb(0, 0, px_w(), px_h(), WALL_TOP,
   WALL_BOT)` — a plain vertical gradient. So the northstar's version is exactly
   what a machine that cannot afford the glows gets, automatically.
3. It is 6 non-vertical gradients in **one 17-line function**, `draw_wallpaper`.
   Reversing this is a deletion, any day, if the look is judged wrong.

**But #29 is what makes this true**, and it was not true before today: with the
blur taking the arena first, the ThinkPad hit case (2) permanently. The
expensive wallpaper was affordable only on the machines that did not need it.

### #31 | Radius 5 → 12 — **the code wins**

> "**small radii**"

`ui.c:60-82` is not drift; it is a counted decision, snapped onto the
4/8/12/16/24 scale the file already enforces, against the desktop-v10 prototype
(`~/zl OS v10.dc.html`, which **does exist on this box** — see #32). Radius
costs nothing either way: `fb_rrect` computes a span per row, so 12 is the same
price as 5, and the constraint the northstar's rule protects does not bind.

There is also a **drawing** consequence recorded at `kernel.zl:762` that points
the same way: the title-bar band is now rounded to match the frame, because "a
square band inside a 12-unit corner is two shapes that do not fit; at radius 5
it was invisible, which is why it survived this long." Going back to 5 does not
just change a curve, it re-hides a defect.

The one place the northstar's 5 **did** win is the new menu panel (#29), where
the reference has an explicit `.win{border-radius:5px}` and the kernel had no
opinion. The menu keeps the kernel's 10 for consistency with every other panel;
this is flagged rather than hidden.

### #32 | `desktop-v10-plan.md` §4 was **not** written from memory

`LOOK-AND-SPEED-PROMPT.md` suspected it: the plan says the prototype wants
`backdrop-filter:blur(10..22px)` in 6 places and `filter:blur(30..34px)` in 2,
and the northstar in this repo contains zero of either.

**Both documents are right; they describe different files.** No blur-using
prototype has ever been committed here — the only `.html` in the entire history,
on any branch, tag, remote or `refs/wip/*` ref, is the northstar itself, added at
`cfed3b6`. The plan names its subject explicitly and it is not in the repo:
`~/zl OS v10.dc.html`, 216 KB, a design-tool export the plan says "cannot render
on this box".

Counted in that file: **7** backdrop blurs (10, 16, 18, 18, 20, 22, 22 px) and
**2** standalone (30, 34) = **9**. The plan's own §3 table says 9 and is
correct; only §4's prose says 6, and the seventh is a JS inline style at
line 2685 that a `backdrop-filter:` grep cannot see. §4 corrected in place.

So the two references are a day apart and the newer one is the northstar. Where
they conflict, #29–#31 above say which wins, per item.

### #33 | The two missing palette entries, and why they are missing differently

The brief asked specifically about `panel` and `ok`. Both were genuinely absent,
for two different reasons, and only one of them was drift:

- **`--panel #05060a`** — `PANEL` is `rgb(0,0,0)`, and that stays. An app's
  content panel has to match the VGA text-cell background **exactly**, or the
  shell's own scrollback sits on a subtly different black. The reference's
  near-black now has its own name, `PANEL_NS`, for chrome that is not a text
  surface — which is what the new menu panel uses.
- **`--ok #5bd66e`** — `OK_GRN` was `rgb(120,220,140)`, a lighter, greyer green
  picked before the reference existed. Drift, not a decision. **The reference
  wins**; `OK_GRN` now holds it.

Also found and closed while checking: `--bar-bot` was `rgb(18,20,32)` against the
reference's `#12141f` = `rgb(18,20,31)`, off by one on blue; and `--crit
#e05a5a` was on screen as a bare literal in the window close button and named
nowhere. Both fixed. `--line #26304a` added as `LINE_SOFT` because #29 needed it.

**Not fixed, and the larger finding: there are TWO palettes.** `kernel.zl`'s
`rgb()` constants — which the northstar's header says it was transcribed from —
paint the header bar, the dock and two legacy app bodies. **Every window frame
on screen comes from `ui.c`'s `ui_theme` struct instead**, and that one agrees
with the reference on 2 of 10 roles: its panel is `rgb(30,42,68)` against
`rgb(5,6,10)`, and it ships a second cyan (`rgb(85,214,255)` for window focus
against `ACCENT`'s `rgb(96,210,235)`). Two accents and two panel colours are on
screen simultaneously. That is a bigger divergence than any of #29–#31 and it is
**open** — see the table below.

---

## Open

| # | Question | Owner |
|---|---|---|
| **A** | `.ultra/METRICS.json` primary metric is blank — **modeset or desktop?** They give different numbers. Logged as T-4. | Roy |
| **E** | **Two palettes ship at once** (#33): `kernel.zl`'s `rgb()` constants vs `ui.c`'s `ui_theme`. Two accents, two panel colours, on screen together. Which is the source of truth? | Roy |
| **F** | **The northstar wants all-mono chrome** ("Everything inside the screen is mono"); the kernel deliberately moved every dock/menu/tray/title label to proportional DejaVu Sans, citing `desktop-look.md` item 4. A straight contradiction, not drift — one of the two documents has to lose. | Roy |
| **G** | The shell's longest lines are **clipped** at 1920x1200 (§1c, `POINTER-PROMPT.md`). Cause named and measured; the terminal staying monospace is right, so the fix is width or wrapping. | open |
| ~~B~~ | ~~No layout engine~~ — **now designed**, see #28 and `desktop-toolkit.md` | closed |
| **C** | Whether to add "ideas worth stealing" + sources to `os-landscape.md` (edit was declined) | Roy |
| **D** | C9 watermark encoding still unsettled — firmware's values fit both narrow and wide | Intel side |

---

## Standing rule adopted today

**Substantive answers get written into the repo as part of answering, not
offered afterwards.** Logged to `~/.claude/CLAUDE.md`, the vault's
`_meta/learnings.md`, and auto-memory. This file exists because of it.

---

Docs: `look-and-speed.md` (the frame budget, the vsync survey, what is next) ·
`desktop-build-guide.md` (start here) · `desktop-TODO.md` (the task list) ·
`desktop-plan.md` · `desktop-look.md` · `desktop-polish-and-speed.md` ·
`desktop-northstar-feasibility.md` · `desktop-prior-art.md` ·
`os-landscape.md` · `intel-graphics-stack.md`
