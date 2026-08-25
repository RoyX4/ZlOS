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

**23. Three of four cores are parked** in ~~`cli; hlt` (`smp.c:79`)~~. Drawing into
disjoint horizontal bands of the back buffer needs no lock. ~~A real 4×, after
damage tracking, not before.~~

> **BOTH HALVES OF THAT ARE NOW WRONG — corrected 2026-08-19 from the source,
> see #39.** The park loop is a **spin**, not `cli; hlt`; `smp.c` says why in as
> many words ("a core halted with interrupts off can only be restarted by
> NMI/INIT/SIPI, so there is no way to hand it work"). And it is not a 4×: 1.78×
> measured, with 4 bands slower than 2 on the worst run. This entry matters
> because it is the one somebody reads when judging whether SMP bands are cheap,
> and read as written it says they are free. They cost three cores.

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

**27. `docs/archive/superseded/GRAPHICS_PLAN.md` is stale for zlOS** — it is the **Windows-hosted** plan
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
~~**open**~~ — **closed by #34 below.**

---

## Four taken 2026-08-19, second session — the ranked items from `NEXT-PROMPT.md`

### #34 | The two palettes — **`kernel.zl` wins, and the reference says so itself.** Closes E

Open item E was owned by Roy as a taste call. It turned out not to be one, which
is why it could be closed rather than escalated. `docs/design/zlOS-design-northstar.html:13`
says what it is, in its own words:

```
/* -- the zlOS palette, straight from kernel.zl's rgb() theme -- */
```

**The reference was transcribed FROM `kernel.zl`.** So "agree with the reference"
and "agree with `kernel.zl`" are one instruction, not two competing ones, and
`ui.c` was the only one of the three files that had drifted. #33 counted it —
11 of 21 roles for `kernel.zl`, 2 of 10 for `ui.c` — and
`hosttest/palette` now reproduces that count mechanically: run against the
pre-fix `ui_theme_init` it prints eight `<-- ui.c differs` rows and leaves
exactly `--hdr-top` and `--hdr-bot` agreeing. Two of ten, independently.

Every value in `ui_theme_init` is now a named reference token, with the token in
the comment beside it. The visible changes:

| role | was | now | token |
|---|---|---|---|
| window body | `#1E2A44` mid-navy | **`#05060A`** | `--panel` |
| the accent | `#55D6FF` | **`#60D2EB`** | `--accent` — the second cyan is gone |
| primary text | `#E4EDFF` | `#D2E4FF` | `--txt-hi` |
| secondary text | `#8FA0C0` | `#96A5C3` | `--txt-dim` |
| hairline | `#141A2A` | `#26304A` | `--line` |
| destructive | `#E05561` | `#E05A5A` | `--crit` |
| desktop bg | `#141A2E` | `#1A1E32` | `--wall-top` |
| unfocused title | `#243350`, flat | **`#2A3550 → #182238`** | the reference's own two stops |

The window body is the big one and it is not only the reference asking for it:
`visual-speed-northstar.md`'s identity list item 3 says "**near-black content**,
restrained navy chrome, one cyan focus line and a soft shadow" independently.

**Looked at, not asserted** — `visual-speed-northstar.md`'s rule 4 is that
appearance is gated with pixel evidence. `hosttest/wmshot` at 1920×1200, same
scene, the only difference being which `ui_theme_init` is linked:

| | |
|---|---|
| [`docs/evidence/visual-diffs/palette-before-two-palettes.png`](../../docs/evidence/visual-diffs/palette-before-two-palettes.png) | mid-navy bodies |
| [`docs/evidence/visual-diffs/palette-after-one-palette.png`](../../docs/evidence/visual-diffs/palette-after-one-palette.png) | `--panel` bodies |

Three things the pictures show that the table does not. The chrome and the
content stop being one blue wash and become separate layers, which is the
"windows recede" effect the northstar's depth-level idea asks for. **The resize
grips become visible** — they are drawn in `title_off` ink and were nearly the
same value as the old panel. And the buttons still read, which is the check that
mattered for `panel_hi`: at the reference's own `--panel-2` they would have
been all but invisible.

**Two things that are not the reference's, flagged rather than hidden:**

1. **`panel_hi` — the reference does not define a control face.** It is a static
   mockup; its own step list has "Widget toolkit — clickable buttons,
   scrollbars" as **queued**. Its nearest raised surface, `--panel-2 #0b0e18`,
   is a ~3% luminance step off `--panel` and would make every button, slider
   track and toggle in `ui.c` effectively invisible. `panel_hi` takes
   `--line-soft #1a2136` instead: a reference token, clearly above the panel,
   still below `--line` so a hairline reads on top of a control.
2. **`title_off_bot` is a new struct field.** The unfocused title bar is a
   gradient in the reference and in `kernel.zl:794`, and was a flat slab here
   because `ui_theme` had one field for it — `wm.c:828` passed the same colour
   twice.

**And the way it would have come back.** `settings.c`'s accent table had
`{ "Ice", 0x55D6FF }` marked "the default — unchanged from `ui_theme_init`".
`settings_apply()` rebuilds the theme and then writes `ACCENTS[S.accent]` over
the accent, so leaving that entry behind meant opening Settings and choosing the
entry labelled *the default* would silently repaint the desktop in the old cyan
— the divergence returning through the panel rather than through the palette.
Now `0x60D2EB`, and `palette` asserts the two are equal.

*Gate:* `hosttest/palette` **reads all three files** rather than restating them —
it parses the CSS variables out of the reference, parses `rgb()` out of
`kernel.zl`, and links `ui.c` for real. A test that hardcoded the numbers would
be a fourth copy of the palette, and the fourth copy is how you get a fifth.
Watched going red on the pre-fix values (4 failures, every diverged role named);
five of its checks are controls on its own two parsers, because every assertion
in it is an equality between two numbers it read, and the silent failure mode is
a parser that returns `-1` for everything.

### #35 | The clipped shell — **it wraps.** Closes G

`term_draw` now wraps a stored line across as many display rows as it needs, and
walks the scrollback backwards counting **display** rows rather than stored ones
so the newest line still lands against the prompt.

**The measured numbers, and one correction to the record.** §1c and item G both
say 77 columns, from the client rect: 1236 / 16. But `kernel.zl:2934` insets the
client by the toolkit's padding before `term_draw` sees it —
`term_draw(ax + 8*u, ay + 6*u, aw - 16*u, ah - 12*u, ...)` at `u = 2` — so the
terminal gets **1204 px = 75 columns**, against a longest `help` line of 82
(`kernel.zl:627`, the i2c row). Seven characters past the edge, not five.

**Width would not have fixed it,** and that is why wrapping was the choice out of
the two the item allowed. A wider boot window fixes 1920×1200 and nothing else:
the window has a resize grip, `mode` changes the screen under it, and `cols` is
`w / cell_w` at whatever size it currently is. The font stays monospace — that
half was settled and measured in §1c, three space-aligned tables depend on a
uniform advance, and mono costs 5.147 ms against 3.584 for forty lines.

**The typed line got the same defect and the opposite fix.** `input` holds 198
characters against 75 columns, so typing past the edge ran off it too — but the
prompt owns exactly one row, and a prompt that wrapped downward would walk up
over the scrollback. It scrolls sideways, anchored on the cursor.

*Gate:* `hosttest/termwrap`, `term.c` alone against recording stubs. **Not a
pixel test, deliberately:** the scissor guarantees no ink escapes the client rect
either way, so "nothing drew outside the window" is green before and after and
proves nothing — the defect is that *characters are lost*. It asserts on what
`term_draw` asks to be drawn: no segment wider than the window, and the segments
of a line concatenating back to the line. It carries its own negative control,
and the control found something worth keeping: **the reassembly check alone is
green against the bug**, because one segment equal to the whole line does
reassemble to it. The width check is the one that catches it. Both are asserted
so nobody later deletes the one that works.

### #36 | `fb_cache_reset()` had no caller because it was **incomplete**, not because nobody thought of it

`NEXT-PROMPT.md` §4 has this as "the arena never rewinds, so one mode switch
leaks the previous wallpaper". Giving it a caller as it stood would have been
worse than the leak: it rewound `arena_next` alone while three sets of live
pointers still referred to the bytes it just freed — `wall_buf`,
`blur_slot[i].px/.cap`, and `blur_tmp/blur_tmp_cap`.

`wall_buf` is the dangerous one. `fb_wall_ok()` answers from the **size**
(`wall_w == fb_w && wall_h == fb_h`), not from the allocation, so a rewind
without a clear gives a confident yes for a buffer the next `arena_take()` is
about to reissue — a use-after-free that *paints*, because the memory is still
mapped and still holds a plausible picture. It now clears every pointer into the
arena in the same breath as the pointer that allocates from it, and is defined
below the blur slots so it can see them.

**And it is not a leak, it is a refusal.** The arena is 16 MiB and only moved
forward, so `mode` at 1920×1200 asked for a *second* 8.8 MiB wallpaper with
7.2 MiB left. A refused wallpaper cache is not a slower desktop, it is a
different one — `desk_draw` falls back to the plain vertical gradient — and the
boot log's `wallpaper cached` line was printed one mode ago and still on screen
saying otherwise. One runtime resolution change turned the cache off for the
rest of the session.

`fb_setup` calls it, next to `fb_clip_none()` and `fb_pointer_forget()` and for
the same reason. **Only when the geometry actually moves:** `fb_setup` is also
the re-init path for a framebuffer that changed address at the same size, the
cache is a copy in RAM that does not care where VRAM went, and re-rendering the
wallpaper costs ~12 ms per glow.

*Gate:* `hosttest/walltest` case C, and it is **twice round the `n` command's
toggle, not once**, because one hop fits in what is left and would have been
green against the bug:

```
  1280x800    4000 KiB taken, 12384 left
  1920x1200   9000 KiB taken,  3384 left
  1280x800    wants 4000, 3384 left  ->  REFUSED
```

Watched going red on a control build with the `fb_setup` call removed.

### #37 | `intel_find()` runs at boot, and `gpu_frames` exists so the result is visible

Its only two callers were interactive shell commands — `kernel.zl:1496` (`P`)
and `:1539` (`k`) — so on a desktop nobody typed a diagnostic into, `mmio` was 0
for the whole session. Every consequence of that is **silent rather than loud**:
`mmio_r()` returns 0 when `mmio` is 0, so `intel_frame_count()` answered 0
forever, and `intel_wait_vblank()` tested `intel_pipe_enabled()`, read 0 out of a
register it had never mapped, and reported "no vblank" immediately. No error
anywhere — just a driver nobody asked to look.

That is what blocks `look-and-speed.md` §4: replacing the 100 Hz PIT release with
a 59.998 Hz panel deadline starts by reading the frame counter, and nothing
could — **`intel_frame_count` had no zl binding at all**, declared at
`runtime_kernel.c:636` and bound nowhere, which is also why "it returns 0" was
invisible from the desktop. `gpu_frames` is that binding, next to `gpu_vbl` and
`gpu_flips`.

**What it touches, precisely, because `intel.c` is the one file here that can
damage hardware:** a PCI scan, `pci_enable()` setting memory-decode and
bus-master on a device the firmware is already scanning out of, and two BAR
reads. No panel register, no AUX, no power sequencing. Nothing on §4.1's hazard
list is reachable from it and nothing arms `lt_armed`.

**It goes inside `wm_boot_start()`, not in the boot log**, and that placement is
load-bearing: `wm_avail()` is 0 on `verify.sh` — `-kernel -display none`, no
multiboot framebuffer tag — and that gate diffs its whole serial transcript
against `golden.txt` byte for byte. A line in the boot log proper would have
turned it red for a change that cannot affect the path it tests. Confirmed:
`verify.sh` passes unchanged.

**The boot line prints the PIXEL CLOCK, and that is a correction made during this
session rather than a design.** It first printed the frame counter as the
liveness fact, reasoning that 0 means the pipe is not scanning out. Running
`hosttest/intel_probe` against this laptop's own GPU — read-only, i915 left
running — refuted it:

```
  device id       0x9B41
  supported       yes - Gen9/9.5
  pipe A enabled  1
  frame counter   0 -> 0 in 0.5 s  =  0.0 Hz   <- frozen: PSR is on
  pixel clock     241690 kHz   (from PIPE_LINK_M1/N1 - exact, PSR-immune)
```

**On this panel `PIPE_FRMCNT` does not advance while Panel Self Refresh is
active**, and two documents in this repo had already said so — `intel.c`'s own
comment above `intel_pixel_clock_khz()` (firmware leaves PSR on here,
`EDP_PSR_CTL = 0x81F00406`), and `HANDOFF.md`'s pixel-clock section, which puts
it more sharply than the probe run did:

> **Do not trust the frame counter for this.** It is not reliably zero, it is
> *intermittently* zero — 0.0 Hz idle, a correct 60.0 Hz with a terminal
> scrolling. It passes in testing and returns 0 in the field.

**Intermittent is worse than frozen**, and that is the sentence that decides
item 2 in `NEXT-PROMPT.md`: a pacer built on the frame counter works on the
desk, with something on screen moving, and stalls exactly when the desktop goes
quiet. So a zeroed counter means PSR, not a dead pipe, and printing it as a
health indicator would have been precisely what the northstar forbids: a status
claiming something the machine has not proved. It prints `gpu_clk` instead —
M/N-derived, read-only, exact, correct while PSR is on — and adds one `[ INFO ]`
line naming PSR when the counter reads 0.

**Which qualifies the item itself.** `NEXT-PROMPT.md` §4 calls this "what
unblocks any vblank work at all". It is necessary and it is *not sufficient*: a
frame-counter-based pacer will read a frozen counter on the only machine this OS
targets. The pacing design in `look-and-speed.md` §4 has to start from the pixel
clock, or from turning PSR off, and that is a decision nobody has taken.

**Only the negative branch is reachable under QEMU** — it has no Intel display
controller, so a boot gate prints `no Intel GPU on the bus` and the `OK` branch
has never executed on a booted zlOS. That is the same "write paths that have
never run" caveat `CLAUDE.md` attaches to all of `intel.c`. What *is* verified is
that the driver code the line depends on works on this silicon, via
`intel_probe`, which runs the same `intel.c`.

### #38 | What a different model family found in #34–#37, and one of the four was a P1

`CLAUDE.md`'s rule — anything memory-, data- or irreversibility-adjacent gets
graded by a different model family, because the context that wrote it is biased
toward it and a fresh subagent is the *same weights inheriting the same blind
spots*. `mcp__codex__codex_review` over the uncommitted diff, pointed at memory
safety and silent wrong behaviour. **Four findings, all four reproduced against
the tree before being acted on, all four real.** Recorded because three of them
are a class, not an incident.

**P1 — `intel_find()` truncates a 64-bit BAR, and #37 promoted that to every
boot.** `pci_bar()` returns the low dword of a memory BAR. GTTMMADR (BAR0) and
GMADR (BAR2) are 64-bit BARs on Gen9. **`pci_bar_is64()` and `pci_bar_hi()`
already exist in `pci.c`, written for exactly this**, with a comment saying "this
is not a theoretical case" — and `intel_find()` never called them. Below 4 GiB
the low dword is the whole address, which is why this laptop at `0xE9000000`
never showed it and why it survived: until #37 it ran only when somebody typed
`k`. The failure is not "reads nothing": the low dword of a high BAR is a
different, unrelated physical address, so every register the driver then checks
answers with someone else's memory.

Fixed by reading the high half, and by **refusing rather than truncating** on a
32-bit build, which cannot address it at all. The high half is combined as two
16-bit shifts and never one 32-bit shift, because `CLAUDE.md` records what this
toolchain does with `x << 32` on a 32-bit type: **clang compiled it to a bare
`ret`**. `intel_bar_too_high()` / zl `intel_hibar()` exist so the boot log can
tell "there is no Intel GPU" from "there is one and we cannot reach it" —
`intel_find()` returns -1 for both, and printing the wrong one sends the next
reader hunting a missing device.

**P2 — the wrap clamp shrank the prompt.** `seg[]` in `term_draw` is one stored
line wide, so the wrap column count is clamped to `TERM_COLS - 1`. #35 then used
that same clamped number for the *typed line*, which is not a stored line: it is
198 characters and it scrolls rather than wraps. On a window of 203+ cells a
full-length line that fitted was scrolled anyway and lost its first four
characters. Two counts now. Gated, and the check was watched failing on the
pre-review code: **194 of 198 shown**, exactly the four predicted.

**P2 — `gpu_frames` reported a negative frame count.** `intel_frame_count()`
returns `int` from a free-running unsigned 32-bit register, so for half of every
cycle the top bit is set. Cast through `unsigned` in the binding rather than
changing the driver signature, which `intel.c`'s own `f1 - f0` deltas rely on.
**Corroborated by accident on the next `intel_probe` run**, which printed
`frame counter 7 -> 0 in 0.5 s = -14.0 Hz` — the counter going backwards through
a signed subtraction, the same defect one layer up.

**P2 — the PSR line blamed PSR without checking.** `if gpu_frames() == 0` printed
"PSR freezes it" unconditionally. A supported Gen9 part whose pipe A is disabled
— another adapter supplied the boot framebuffer — also reads 0. Now gated on
`intel_pipe() == 1`. This is `feedback_diagnosis_is_a_claim` committed by the
same session that had just written that rule into #37.

**The pattern worth keeping: three of the four were cases where the repo already
held the answer** — `pci_bar_hi` existed, the `<< 32` hazard was in `CLAUDE.md`,
the PSR intermittency was in `HANDOFF.md`. None of them needed new information,
only somebody who had not just written the code looking at it.

**One finding NOT from the review, found while running the land gate**, and it
belongs to the GPU-track session rather than to these four: `hosttest/gpu_ring`
exits **2** when run without root, so `gates/land-gate.sh` counts it a FAIL and
the whole gate goes red on a box that simply is not root. Commit `e1eb44a`
established `exit 77 = SKIP` five commits earlier and `gpu-blitter.md` says "the
convention now exists — use it for the next hardware harness". `gpu_ring` is the
next hardware harness. Not fixed here — it is another session's file and it was
in flight.

### #39 | SMP bands: two docs written the same day disagree, and the code settles it — **do not turn them on**

`gpu-driver.md`'s "the order that follows from all of it" ranks SMP bands **#1**:
*"1.78x on the desktop redraw, already written, switched off, no hardware
risk."* `NEXT-PROMPT.md` and `look-and-speed.md` §2 say the opposite: *"it is one
call and it should not be made."* Same measurement, same day, opposite advice,
and it sits at the top of the driver's own recommended order — so it gets
settled here rather than picked by whoever reads which file first.

**Three facts, all read out of the source rather than out of a doc.**

1. **The park loop is a spin.** `smp.c`'s own comment: *"The park loop is a
   SPIN, not `cli; hlt`. It has to be: a core halted with interrupts off can only
   be restarted by NMI/INIT/SIPI, so there is no way to hand it work without an
   interrupt path this kernel does not have."* Turning bands on burns three cores
   from `smp_go()` until reboot. **`DECISIONS.md` #23 said `cli; hlt` and is now
   corrected** — that entry is the one somebody reads when judging the cost, and
   as written it said the cost was nothing.

2. **The expensive thing is not on the band path.** Exactly four primitives
   route through `fb_par_run`: `fill_band` (:1242), `grad_band` (:1369),
   `shadow_band` (:1581), `blit_band` (:554, and only for spans ≥ 64 rows).
   `fb_grad_radial` (:1824) and the conic wedge (:1862) **do not.** Those are the
   three radial glows and two conic wedges of the wallpaper — the single most
   expensive full-screen work this system does, ~12.2 ms for one 900×700 glow.
   Band rendering does not touch it.

3. **What it does buy, from `evidence/desktop-smp-bands.md`'s own table:** whole desktop
   4.833 ms → 2.949 ms at four bands. **1.88 ms**, on a full-screen redraw that
   damage tracking already made rare, and the worst run has 4 bands slower than
   2. `fb.c:229` also rules out the reading that would make it worth more: the
   draw list is `app_draw`, which is zl, and the zl runtime is not reentrant —
   *"Four cores inside `zl_fn_app_draw` would corrupt the interpreter, not the
   framebuffer."*

**So: 1.88 ms on a rare path, nothing on the expensive path, three cores
permanently.** `NEXT-PROMPT.md` is right and `gpu-driver.md`'s #1 should not be
taken. Corrected there in place.

**The proposal neither doc makes, stated but NOT built.** The wallpaper bake is
the one place the trade inverts: it is full-screen by definition, it is the
expensive path, it is pure C with no zl anywhere near it, and it happens **once
at boot** — so the cores could be woken, used, and left parked, and the
permanent-spin objection does not apply to a bounded window. It needs
`fb_grad_radial`/`fb_grad_conic` put on `fb_par_run`, which is the same shape as
the four that already are. **Not done here:** it is new scope outside this
session's brief, it is in `fb.c` which another session also holds, and the
measurement that would justify it — bake time serial vs banded — has not been
taken. Take that measurement before writing it.

---

## Three taken 2026-08-19, the v10 look-and-speed rebuild — LOOK-AND-SPEED-PROMPT.md, resumed

`STATE-OF-THE-PROJECT.md`'s audit named several already-diagnosed visual
defects; these three were re-derived from the tree (not taken on the audit's
word — its own §3.6 shows a diagnosis can go stale between the writing and the
reading) and are small enough to fix and gate in one pass. The audit's §3.2
(dock digit debris) was checked the same way and turned out to be **already
fixed** on this branch (`kernel.zl:3247-3265`, closed by unifying the tray's
damage rect on `tray_x()`) — not re-touched here, and not re-claimed here.

### #40 | `icons24`/`icons48` externs said 10, `icons.c` has always had 20

`fb.c` declared both atlases at `[10]`; `icons.c` defines `[20]` for each, with
its own index comment naming all twenty. It linked because the element type
matched, and `fb_icon24()` refuses `n >= ICON_N` before touching the array, so
the ten icons the v10 pass generated (search, lock, drive, close, check,
chevron, clock, network, volume, grid) were unreachable from any caller.

`ICON_N` raised 10 → 20. **This has no visible effect by itself** — grepped
`dock_icon()` in `kernel.zl`, every slot returns 0–9, so nothing today asks for
icon 10+. Wiring any of the ten to a dock slot, a menu row or a title-bar icon
is a separate, undecided design question (which icon means what is not this
document's call) and is not done here.

*Gate:* `build.sh`/`build64.sh`/`buildefi.sh` clean, `hosttest/build.sh`
rebuilds all harnesses with no new warnings, `fbbench`/`wmtest`/`tritest`/
`toasttest`/`walltest`/`palette` (everything that links `icons.c`) still green.

### #41 | The resize grip was drawn twice; deleted the earlier, dimmer, wrongly-scaled one

`chrome()` drew the bottom-right grip **twice**, from two merge parents: a
`UI_S1`-scaled set of 3 diagonals in `t->border`, before the title bar was
composited, and a `UI_S3`-scaled set in `t->text_dim`/`t->title_off` after the
close box, which is the one whose own comment already documents a "looking at
it" fix for an L-bracket merge bug. Only the second matches `RESIZE_EDGE`'s
`UI_S2` hit region — the first drew an affordance *smaller* than its own hit
target, the second draws one slightly larger, which is the right direction for
a visual cue.

Deleted the first block. `wmshot` before/after at the same corner
(`docs/evidence/visual-diffs/grip-before-two-renderers.png`,
`docs/evidence/visual-diffs/grip-after-one-renderer.png`, 80×80 crop at 6×, point-filtered)
shows the extra, longer diagonal strokes gone.

*Gate:* the three kernel builds clean; `hosttest/wmtest` green including the
grip's own hit-test assertions ("dragging the grip resizes the window", "...and
does NOT move it") — unaffected, because only the drawing half changed, which
is the point of having both a hit-test suite and a screenshot tool.

### #42 | The window fade blended its saved backdrop at the wrong origin, and I widened its scissor too

`STATE-OF-THE-PROJECT.md` §4.14 named this; re-derived independently by
reading `wm_repaint()` rather than trusting the citation, because the doc's own
method (§3.6) is "re-check, don't inherit." Confirmed exactly as described:
`cx, cy, cw, ch` are declared once per window, set first to the frame+shadow
box, then `fb_stash(cx, cy, cw, ch)` captures that box for the fade — but a
later `isect()` against the *client* rect (narrower, inset by the border and
title bar) overwrites the same four variables before `fb_stash_blend(stash,
cx, cy, ...)` runs, so the saved backdrop was painted back at the client
origin instead of where it was taken from.

Fixed by capturing `sx, sy, sw, sh` at the point of the stash call, before the
client `isect` can touch them, and blending against those. **A second,
deliberate change beyond the minimal one:** the blend's scissor also moved from
the (clobbered) client rect to the full `sx, sy, sw, sh` frame+shadow box. The
surrounding comment's own algebra is `window * a + behind * (1 - a)` for *the
window*, not for its client area alone — a title bar that pops in at full
opacity while only the content fades is a second, smaller defect the minimal
fix would have left in place. Consequence worth knowing: the window's frame,
border and shadow now fade in step with its content, which changes the visible
top few pixels of any window using `ANIM_FADE` (today, only the start menu).

**Found while checking that, not part of this fix:** `ui.h:107`'s own comment
names the start menu as the example of `WF_NOCHROME` ("draws its own frame"),
but nothing in the tree ever sets it — `wm_open()` takes no flags argument and
always assigns exactly `WF_OPEN`, and a repo-wide grep for
`flags |= WF_NOCHROME` or an equivalent finds zero writers, only the six
readers. So the menu has ordinary full chrome (title bar, close box, resize
grip) today, same as any other window, and the widened scissor above genuinely
applies to it. This is the same "checked but never set" pattern
`HANDOFF.md` already names for `WF_MODAL` before it got a caller — logged here
rather than fixed, because deciding whether the menu SHOULD go chromeless is a
design call this document does not make unilaterally.

**Measured, not just read.** A standalone host probe (one window alone over
`fb_gradient`'s vertical wallpaper, so "what's behind it" varies by row and the
origin shift has somewhere to show up) sampled a column of pixels near the
frame's top edge at a fixed mid-fade frame, built once against pre-fix `wm.c`
and once against post-fix, same probe, same scene:

```
  y     before   after   delta (R,G,B)
  306   161c2e   181e32   (+2,  +2,  +4)
  312   2c539a   233e72   (-9, -21, -40)
  316   2a4f94   223b6e   (-8, -20, -38)
  320   284b8f   21396b   (-7, -18, -36)
```

19 of 20 sampled rows changed; the largest single-row channel delta is 40/255.
The probe was not kept as a committed harness — it needed a second, undocumented
mmap (`0x0C000000`, the blur/wallpaper arena `slot_capture` reads, which
`wmshot.c`'s stub set never maps because nothing there calls `fb_stash`) before
it stopped segfaulting, which is exactly the kind of fragile one-off this repo
prefers not to check in without more scrutiny than one session had time for.
**Gap, stated rather than hidden:** `hosttest/wmtest`'s existing `ANIM_FADE`
assertions (`px_mid != px_over`, `px_mid != px_under`, alpha genuinely partial)
stayed green before AND after this fix — they assert a fade composites
*something*, not that it composites the *right* rectangle, so they could not
have caught this bug and would not catch its return. A precision regression
assertion for the origin specifically does not exist yet.

*Gate:* the three kernel builds clean; `check-memmap.sh`/`check-zl-calls.sh`
pass; `hosttest/wmtest` green (0 failures) both before and after, which is
this fix's regression floor, not its proof — see the gap above.

---

## Open

| # | Question | Owner |
|---|---|---|
| **A** | `.ultra/METRICS.json` primary metric is blank — **modeset or desktop?** They give different numbers. Logged as T-4. | Roy |
| ~~E~~ | ~~Two palettes ship at once~~ — **closed by #34.** Not a taste call in the end: the reference's own header says it was transcribed *from* `kernel.zl`, so `ui.c` was the only file that had drifted. Gated by `hosttest/palette`. **The window body going near-black is the visible half — if that reads wrong, #34's table is the one block to revert.** | closed |
| **F** | **The northstar wants all-mono chrome** ("Everything inside the screen is mono"); the kernel deliberately moved every dock/menu/tray/title label to proportional DejaVu Sans, citing `desktop-look.md` item 4. A straight contradiction, not drift — one of the two documents has to lose. | Roy |
| ~~G~~ | ~~The shell's longest lines are clipped~~ — **closed by #35**, by wrapping. Note the correction: it is **75** columns, not the 77 §1c computed, because `kernel.zl:2934` insets the client before `term_draw` sees it. Gated by `hosttest/termwrap`. | closed |
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

### #43 | The snap preview: snapping worked, but you could not see where it would land

`snap.c` has had the whole geometry for a long time — `snap_zone_for_point`,
`snap_rect`, `snap_apply`, `snap_release` — and `route_mouse`'s drop path has
called it since the exec-track merge. Dragging a window to an edge **did** snap
it. What was missing was any indication *before* letting go, so the feature was
invisible until it had already happened.

**What was added.** A cached rectangle (`sp_zone`, `sp_x/y/w/h`) declared above
`wm_repaint`, filled by `snap_preview_set()` on every pointer motion during a
`GRAB_MOVE`, and drawn with `fb_rrect_blend()` at alpha 64 in the theme accent —
above the windows, below the toast.

**Four decisions in it that were not arbitrary:**

1. **The preview and the drop share one geometry function.** `snap_rect()` is
   the same arithmetic `snap_apply()` uses, minus the commit. It is declared in
   `wm.c` for this. A preview computed independently is a preview that can
   promise a landing spot the snap then disagrees with.
2. **It follows the POINTER, not the window.** The drop asks
   `snap_zone_for_point` about the pointer, so the preview must too. Using the
   window rectangle would light up a different zone than the drop takes.
3. **It damages the rectangle it LEAVES as well as the one it enters.** The
   preview is not a window; nothing else in the compositor knows those pixels
   changed. This is the same mistake `snap_to_rect` documents having made with
   `wm_move`, and it is why `snap_preview_set` returns early when the zone has
   not changed — otherwise every drag frame in the middle of the screen would
   repaint the desktop.
4. **`wm_drop_grab()` clears it.** A window CLOSED mid-drag clears `pgrab`, and
   the button-up that would normally clear the preview never arrives. Without
   this the hint stays painted for the rest of the session.

All seven zones work for free, corners included: `snap_rect`'s `default:` arm
fills the work area, so no zone can produce an uninitialised rectangle.

**Evidence — `kernel/tools/probes/probe-snap.py`, committed, real QEMU and real pointer.**
A green build proves nothing about paint, so this drives an actual drag and
asks whether the screen changed, in which half, at which moment:

```
booted 1920x1200
  1 left edge shows a preview        985575 px  ok
  2 leaving the edge clears it        14523 px  ok
  3 right edge previews the RIGHT    975365 px right,   31811 px left  ok
snap preview gate green
```

Step 3 is the one with teeth: a constant rectangle, or one derived from the
window, passes 1 and 2 and fails 3.

**The probe's first two runs reported RED, and both were the probe.** A drag
moves the window with the pointer, so comparing an edge shot against a
mid-screen shot measures the window travelling across the display as much as it
measures the preview — the first version saw the window *vacate* the left half
and called it a stray preview. Fixed by taking each reference a few pixels the
other side of the same threshold, and by setting the thresholds from the
measured confound (~30,000 px of window repaint) rather than tuning until green:
the signal is ~985,000 px, so 100,000 separates them by an order of magnitude
in both directions. Screenshots in `kernel/shots/snap-preview-{left,right}.png`
were what settled it — the numbers alone would have had me "fixing" working code.

**Not done:** the preview is a flat blend with no border and no transition. It
appears and disappears instantly rather than easing, which is the same
frame-step-versus-time-based-motion gap the north star raises for the rest of
the compositor's animation. Left alone deliberately rather than half-done.
