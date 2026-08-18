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

It looked correct at 3840×2160, and that is the instructive part: `back_on` is 0
there, so those three fall back to `put_pixel`, which *was* clipped. A gate that
only ran at 4K would have passed a broken keystone.

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

## Open

| # | Question | Owner |
|---|---|---|
| **A** | `.ultra/METRICS.json` primary metric is blank — **modeset or desktop?** They give different numbers. Logged as T-4. | Roy |
| ~~B~~ | ~~No layout engine~~ — **now designed**, see #28 and `desktop-toolkit.md` | closed |
| **C** | Whether to add "ideas worth stealing" + sources to `os-landscape.md` (edit was declined) | Roy |
| **D** | C9 watermark encoding still unsettled — firmware's values fit both narrow and wide | Intel side |

---

## Standing rule adopted today

**Substantive answers get written into the repo as part of answering, not
offered afterwards.** Logged to `~/.claude/CLAUDE.md`, the vault's
`_meta/learnings.md`, and auto-memory. This file exists because of it.

---

Docs: `desktop-build-guide.md` (start here) · `desktop-TODO.md` (the task list) ·
`desktop-plan.md` · `desktop-look.md` · `desktop-polish-and-speed.md` ·
`desktop-northstar-feasibility.md` · `desktop-prior-art.md` ·
`os-landscape.md` · `intel-graphics-stack.md`
