# The overnight desktop run — 2026-08-17/18

What one unattended session did against `OVERNIGHT-PROMPT.md`, what it got
wrong, and what is left. Written because chat is gone next session and the repo
is not.

**Branch:** `desktop/overnight-compositor` · **24 commits** of desktop work
(three others on the branch are the display session's — see T-10) ·
**+13,367 / −176** across 35 files.

Gates green at the end of the run, all four boot paths plus every harness:

```
verify.sh      ok    32-bit BIOS, transcript matches golden.txt
verify-efi.sh  ok    64-bit UEFI application - the laptop's path
verify-raw.sh  ok    our own bootloader
wmtest 59/59 · inputtest 12/12 · tritest 9/9 · fbbench clean
```

---

## 1. What landed, group by group

### Group A — the three resampling bugs

| | |
|---|---|
| **A1 icons** | `gen_icons.py` emits `icons24` **and** `icons48`, each rasterized from the same 96-unit geometry at its own 4× supersample. Not the 24×24 set scaled up — that was the bug. **Also 2× faster**: 20.1 → 10.0 cyc/px, because two integer divides per pixel cost more than the larger atlas costs in cache. |
| **A2 lines** | Wu's algorithm replaces Bresenham. **4.3× slower per line pixel** and kept anyway — it is a quality gate, the sparkline is 8 segments not 200, and axis-aligned lines still take the `a == 255` fast path with no read-back. |
| **A3 logo** | `fb_glyph_scaled` read the **1-bit** font and drew solid squares. Now resamples the 16×32 coverage atlas. |
| **A4** | `div255` deleted — defined, never called. |

The 24×24 atlas came out **byte-identical** to before, which is what proved the
`ScaledDraw` proxy is an identity at scale 1 rather than merely looking right.

### Group B — the enabling changes

**B1 — the resolution cliff.** Three buffers had compile-time *pixel* ceilings.
Now every ceiling is the distance to the **actual next buffer** in a high-RAM
map written out at the top of `fb.c`, with each base re-read from the file that
owns it.

```
2560x1440   back ON (14,400 KiB), drag ON     <- both were silently OFF
3840x2160   back ON via the fallback arena, drag OFF, and it says why
```

`back` and `drag` no longer fail *together*, because their ceilings are
different numbers — so the boot line reports them separately. Saying "dragging
is lost" at 4K would have been a lie printed every boot.

**B2 — `fb_clip`, the keystone.** The plan said "change **exactly two**
functions". It is **five**. Three more write the back buffer directly and call
neither: `draw_glyph`'s two fast paths, `fb_gradient`'s `back_on` branch, and
`fb_scroll`.

```
1920x1200   2,184,000 pixels escaped the scissor
2560x1440   3,566,400 escaped
3840x2160   zero escaped        <- and THAT is the instructive one
```

4K passed *while broken*, because `back_on` is 0 there so those paths fall back
to `put_pixel`, which was clipped. **A gate that only runs in the degraded
configuration tests the wrong code.**

All five fold the scissor into their **loop bounds** rather than testing per
pixel, so a clipped draw costs no more per pixel than an unclipped one — which
matters, because the compositor's hot path is many small clipped rectangles.

**B3 — the damage list.** Eight rectangles, merged on *contact*. When full,
everything collapses into one, which *is* the old single box — so the worst
case is "as slow as it was", never "wrong".

| | presented | one box would be | |
|---|---|---|---|
| two corners @1920×1200 | 33,000 px | 2,180,800 px | **66×** |
| two corners @2560×1440 | 33,000 px | 3,528,000 px | **107×** |

The **pixel accumulator is the design, not an optimisation**: `put_pixel` is
the hottest path in the file, so it still grows a single box at the same four
compares, and that box is flushed into the list when a rect-shaped primitive
reports its own damage. Interleaved A/B against the previous commit: AA text
+3.0%, one window −4.9%, whole desktop −3.3%. The sign is not consistent — that
is noise, which is the answer the design was for.

**B4 — `EV_MOUSE`.** Declared since the file was written, **never pushed**. Two
properties that are decisions:
- a position is *state*, so moves coalesce to the latest — replaying
  intermediate positions would tell a window it was dragged through places the
  hand had already left;
- a **button is not state**, and that is the seam. `idt.c` keeps only the
  current mask, so a press+release between two polls is gone before `input.c`
  runs. The fix is a latch in the ISR, not here.

**B5 — not done.** Needs `runtime_kernel.c`, mid-flight.

### Groups C / D / F — the compositor and the toolkit

`wm.c` (mechanism), `ui.c` (widgets), `ui.h` (the contract). Plus **D0**
theme-as-data on the 4/8/12/16/24 × `ui()` scale, **F1** three-level elevation,
**F2** the close box with hover/press states.

Two structural decisions worth keeping:

- **wm damage is not fb damage.** One is *intent* ("these regions need
  repainting"), the other *consequence* ("these pixels changed and need
  blitting"). Sharing one list would couple them into a bug that only shows on
  some frames.
- **A window's damage is its frame plus its shadow.** `fb_shadow` reaches
  `off + soft` beyond the footprint, so repainting `w + 16` when the shadow drew
  to `w + 28` leaves 12 px behind on every drag step — the exact smear measured
  in the 0a screenshot.

### Group H — the stretch goals, all three

- **H1 motion.** Four frames, a **scale not a fade** — a fade needs an
  offscreen buffer this kernel has nowhere to put, while a scale needs nothing
  new because apps are already size-agnostic by contract. **Hit testing ignores
  it entirely**: a click during those 40 ms lands where the window is *about to
  be*.
- **H2 tabs.** Everything downstream asks `win_app(win)` rather than reading
  `.app`, so a tabbed window and a plain one are indistinguishable to the
  repaint and the routing. One `tab_rect()` serves both drawing and
  hit-testing — two copies is how controls end up responding a few pixels from
  where they are drawn. And the tab is checked **before** the drag, or the strip
  is unclickable.
- **H3 tiled rasterizer.** Correct, and **2.8× slower** than the scanline fill
  it does not replace. Not wired in. See §3.

### Group G — SIMD

`cpu.c` had detected SSE since it was written and **nothing used it**.

| @1920×1200 | scalar | SIMD | |
|---|---|---|---|
| fill whole screen | 0.86 cyc/px | **0.39** | 2.2× |
| fill + present | 2.05 cyc/px | **1.35** | 34% |
| whole desktop | 4.35 cyc/px | **4.00** | 8% |

The desktop figure is small **on purpose**: that scene is blend-bound, not
store-bound. Fills were never most of a frame.

**Where it is allowed is the whole care of the change.** SSE is enabled only in
`boot64.S`; `boot.S` — the 32-bit entry `verify.sh` boots — never touches CR4,
so an SSE instruction there *faults*. The guard is `#ifdef __SSE2__`, which
tracks the build with nothing to keep in sync, and it is checked mechanically:
the 32-bit object contains **zero** `xmm` instructions.

`blend_rgb`/`blend_sub` are deliberately **not** vectorised — three table
lookups per pixel, and a gather is the one thing SSE2 cannot do. Vectorising
around it means giving up gamma-correct linear-light blending.

### F3 — proportional text

`desktop-look.md` ranks this as *"the one that stops the desktop reading as a
terminal"*, and **its plan does not work**. It proposes emitting per-glyph
advances from the existing atlas — but that atlas is DejaVu Sans **Mono**, so
every advance is the same number and the screen would not move a pixel. Right
about the mechanism, wrong about the data.

A proportional **face** (DejaVu Sans, same superfamily) gives **12 and 20
distinct advances** against mono's 1. That number is the feature.

---

## 2. Things I got wrong, and how they were caught

**The tablet is not a thief.** I claimed `-device usb-tablet` steals the pointer
and `try.sh` should drop it. Two measurements were consistent with that, and
removing the tablet made the test pass — which *felt* like confirmation. It was
not. Sending **absolute** events with the tablet still attached showed the
tablet path works exactly (0.75/0.50 of the screen → 1439,599 on 1920×1200).
zlOS drives *two* pointers and my harness was sending the wrong event type.
Retracted in T-5.

> Run the experiment that could **refute** you, not the one that agrees.

**All six rasterizer step constants were sign-inverted.** The cube looked
perfect, because interior tiles never step — only boundary tiles were wrong.
The scanline oracle found 13,031 differing pixels on one triangle and a sliver
drawing almost nothing. No amount of looking at a spinning cube catches that.

**`probe-mouse.py` was lying.** It asserted only that the pointer *moved*, and
passed while the pointer was being driven to 0,0 — both axes pinned at the
clamp. Any test whose predicate is weaker than the property you care about will
eventually pass while the property is false.

**Three bugs in my own new code**, found by turning the anti-idle audit on the
code I had just written rather than on the old code:

- `wm.c`'s **modal branch could never execute** — nothing set `WF_MODAL`. The
  hazard `HANDOFF.md` names for `intel.c`, in new code.
- **Two silent refusals**: `wm_open` returning −1 with no message, and
  `fb_setup` rejecting an unsupported depth without a word. The same bug class
  0a was written about, reintroduced by someone who had just fixed it.
- **`wm_damage_win` used a fixed shadow reach** after F1 made the shadow a
  variable. A modal reaches 42 px and was damaged at 28 — a 14 px ring nothing
  would ever clean up.

**`ui_toggle` drew a circle, not a pill.** Every assertion about it passed the
whole time, because *does it toggle* and *does it look like a toggle* are
different questions and only one had a test. Caught by rendering a frame and
looking at it.

---

## 3. Two changes that are slower, and still here

`DECISIONS.md` #25 records an optimisation argued from an instruction count,
shipped, and measured 25% slower afterwards. Measuring in **both** directions is
the point.

| | measured | kept? | why |
|---|---|---|---|
| Wu lines | **4.3× slower** per line pixel | yes | quality gate; real volume is 8 segments, not 200 |
| tiled rasterizer | **2.8× slower** than scanline | **not wired in** | a scanline fill is already optimal for a flat triangle |

A scanline fill emits one full-width `fb_fill_px` per row — 500 calls, each
hitting the SIMD fill, close to memory bandwidth. The tiled path's expense is
~25,600 per-pixel edge tests on boundary tiles, inherent to barycentric
rasterization, and exactly what SIMD removes by testing 4 or 8 pixels at once.
**That is the next step and also the one that reverses the table.**

`-DFB_NO_SIMD` and the scanline oracle both stay buildable, so these comparisons
are commands anyone can re-run rather than numbers in a commit message.

---

## 4. The harnesses, which are half the output

None of this was verifiable by screenshot. Five new host programs run the
shipping sources against fake hardware, in milliseconds, with no boot:

| | |
|---|---|
| `hosttest/wmtest.c` | **59 assertions** over `fb.c` + `input.c` + `ui.c` + `wm.c` |
| `hosttest/inputtest.c` | 12 — every failure mode of `EV_MOUSE` is invisible in a picture |
| `hosttest/tritest.c` | 9 — the tiled rasterizer against the scanline **oracle** |
| `hosttest/wmshot.c` | renders a frame to a PNG; found the toggle bug immediately |
| `hosttest/fbbench.c` | + FNV scene hash, clip check, damage check, drag check |
| `probe-shot.py`, `probe-drag.py`, `probe-mouse-sync.py` | headless QEMU boot, screendump, scripted pointer |

**The FNV scene hash** is what makes "this change is supposed to be invisible" a
number instead of a squint. `8473499efb49abb1` @1920×1200 and
`81c4be85c58763e7` @2560×1440 have not moved since Group A. The 4K one changed
**once**, deliberately, when 4K gained a back buffer and therefore subpixel text.

Two lessons the harnesses taught:

- **The full-screen-scissor identity check alone is worthless** — an `fb_clip()`
  that did nothing would pass it perfectly. It needs the second half: *zero*
  pixels escape a scissor that is set.
- **The scene hash could not have caught a `present()` that only blitted
  `dmg[0]`**, because that scene opens with a full-screen gradient and always
  presents as *one* rectangle. That needed its own test.

---

## 5. What is left, and why

**Blocked on `kernel.zl` and friends** — mid-flight in the display session all
night, and the brief forbids staging another session's unfinished work:

| | |
|---|---|
| **C4** delete the sticker-drag machinery | `kernel.zl` still calls those builtins |
| **C5** start menu as a modal window | policy |
| **D2, E1–E7** app conversions, the shell as app 0 | policy |
| **B5** `tsc()` builtin | `runtime_kernel.c` |

**The single highest-leverage next move: wire the compositor in.** It is built,
tested, and unreachable — `kernel.zl` still ends in `while running == 1`.
Everything that does *not* require touching that file is done: `wmglue.c` holds
every shim with **weak symbols**, so it links today and lights up the moment
`kernel.zl` grows `app_draw`. The remaining zl is written out verbatim in
[`desktop-wiring.md`](desktop-wiring.md).

**Deliberately not attempted:** F4 (fractional scale — its own gate says
"only if A–D are all green", and D2 is blocked) and **G2 SMP band rendering**,
which remains the biggest single speed lever — a real 4×, three cores parked in
`cli; hlt` — and remains the wrong risk unattended.

**Two findings in other people's files**, logged not fixed:

- **T-11.** The GOP framebuffer address is **truncated to 32 bits** in the EFI
  build — `unsigned long` is 4 bytes on that target, proven by static assert.
  Origin is `efi.c:250`. The `-Werror` guard added for this exact bug class is
  **silent** on it, verified by compiling both cast shapes with the exact build
  line: it catches pointer↔`int`, and this is a UINT64 narrowed by an *explicit*
  cast. Latent — a GOP base is normally a PCI BAR below 4 GiB.
- **T-10.** `git switch -c` in a shared checkout moved HEAD for *both* sessions,
  so three display commits sit on this branch instead of `main`.

---

## 6. Two operational lessons

**A gate that only runs in the degraded configuration tests the wrong code.**
`fb_clip` passed at 4K while leaking 2.18 M pixels at 1920×1200.

**One `.git`, two sessions, is not safe.** T-10 put commits on the wrong branch;
T-12 was **four zero-length objects** with HEAD pointing at one, almost
certainly a raced object write. One commit's object was lost — its content never
left the working tree, and `.git/logs/HEAD` is plain text and survived when the
object store did not. **Read the reflog first when git will not talk to you.**
A `git worktree` gives each session its own index and refs while sharing the
object store, and is what should have been used from the start.

---

Task list: [`desktop-TODO.md`](desktop-TODO.md) · Wiring:
[`desktop-wiring.md`](desktop-wiring.md) · Decisions:
[`DECISIONS.md`](DECISIONS.md) · Numbers:
[`desktop-polish-and-speed.md`](desktop-polish-and-speed.md) · Blocks:
`../../.ultra/TENSIONS.md` T-5 … T-12
