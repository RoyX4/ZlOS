# Looking good, and being fast

Two questions, answered 2026-08-17: *what makes Windows and Linux desktops look
so good, and how do we get there* — and *what about speed*.

They are the same document because they trade against each other. Everything that
makes a desktop look good costs pixels, and zlOS draws every pixel on one CPU
core with no GPU.

Companion to `desktop-look.md`, which covers the three specific rendering bugs.
This one is the general answer.

---

# Part 1 — what actually makes a UI look good

Not one thing. Six, and zlOS already has two of them.

## 1. Type — a scale, not sizes

Modern UI picks maybe five sizes and never uses anything else. 12 / 14 / 16 /
20 / 28. Two or three weights. Proportional spacing. Line height around 1.4–1.6.

Hierarchy comes from *size and weight*, not from colour or boxes.

**zlOS today:** two sizes (13px body, 26px title), one weight, monospace,
locked to an 8 or 16 pixel cell. Every label on screen — window titles, dock
labels, the System Monitor readouts — advances by exactly one cell per character.

Uniform advance is the single strongest visual signal of "terminal", and it is
currently applied to things that are not terminals.

**To fix:** emit per-glyph advance widths from `gen_hd_font.py` (FreeType already
knows them) and give UI labels a proportional draw path. The console keeps its
grid — only titles and labels change.

## 2. Space — a scale, not numbers

Modern UI has a spacing scale (4 / 8 / 12 / 16 / 24 / 32) and every gap, pad and
margin is one of those. Nothing is 14. Nothing is 13.

And the amounts are **generous**. Modern panels have 16–24px of internal padding.

**zlOS today:** hand-picked per window. `sx + 14 * u` here, `sy + 58 * u` there,
`214 * u` for a bar width. Nothing snaps to anything.

**To fix:** define the scale as zl constants and use them everywhere. This is
cheap and it is most of what "designed" means.

## 3. Colour — roles, not a palette

Modern UI has maybe six colour roles: surface, raised surface, primary text,
secondary text, border, accent. Every colour on screen is one of them. Surfaces
are low saturation; saturation is spent only on the accent.

**zlOS already does this** — `WALL_TOP`/`WALL_BOT`, `PANEL`, `TXT_HI`,
`TXT_DIM`, `ACCENT`, `BAR_TOP`/`BAR_BOT`. That is a real role-based palette.
Nothing to fix. Keep it.

## 4. Depth — elevation as a system

Shadows should encode a *hierarchy*: a menu is above a window, a window is above
the desktop. Different elevations get different shadow sizes.

**zlOS today:** every window gets the identical shadow — `shadow(wx, wy, ww, wh,
8*u, 6*u)`, focused or not, menu or window.

**To fix:** three elevation levels. `off` and `soft` are already parameters of
`fb_shadow`, so this costs nothing but deciding.

## 5. Motion — the biggest thing missing

This is the one. Modern desktops animate: windows fade in, menus slide, focus
transitions, buttons respond. Even 120ms of movement changes how a UI *feels*
more than any static detail.

**zlOS today: nothing animates at all.** Windows appear instantly. Menus pop.
Focus snaps.

**This is probably the single largest "feels modern" lever available**, and it is
also the one most constrained by speed — see Part 2. A fade needs the window
composited at several opacities, which means drawing it several times.

Cheapest version that still reads as motion: **4 frames.** A menu that appears
over 4 frames instead of 1 already feels different. Start there, not at 60fps
easing curves.

## 6. Consistency — decide once

Same radius everywhere. Same padding. Same transition length. The reason
commercial UI looks coherent is that a design system decided once and everything
obeys.

**zlOS is already fairly good here** — nested 5px/4px `rrect` and `TITLE_H 28`
are used consistently. The gaps are spacing (see #2) and elevation (see #4).

## Where that leaves it

| | zlOS | verdict |
|---|---|---|
| Type | 2 sizes, 1 weight, monospace, cell-locked | **gap** |
| Space | hand-picked numbers | **gap** |
| Colour | role-based palette | **already good** |
| Depth | one shadow for everything | small gap |
| Motion | none | **biggest gap** |
| Consistency | mostly good | small gap |

**Do not replace the look.** Deep navy, nested rounded panels, focused-blue
title gradients with an accent underline — that is a coherent aesthetic, closest
to BeOS/NeXT. It needs its own rules applied consistently, plus motion.

---

# Part 2 — speed

## The situation

Every pixel is drawn by **one CPU core**, in C, with no GPU acceleration of any
kind. At the ThinkPad's 2560×1440 that is **3,686,400 pixels** per full-screen
redraw.

## MEASURED — `hosttest/fbbench.c`

The dev box **is** the target laptop: `i7-10510U`, Comet Lake-U, the same chip
family as the `8086:9B41` graphics in `HANDOFF.md`. So `fb.c` benchmarked here
runs on the real target CPU.

`hosttest/fbbench.c` compiles the **shipping `fb.c`, unmodified**, at the same
`-O2` the kernel uses, and `mmap`s the three fixed physical addresses fb.c
hardcodes so nothing has to be patched. Same method as `intel_probe`.

```
cd kernel/tests/host && ./build.sh && ./fbbench
```

**Baseline, before the shadow fix below** (min of 7 runs; this CPU turbos 1.8–4.9
GHz so ms drifts between runs — cycles per pixel and the *ratios* are the real
numbers):

| | 1920×1200 (back buffer ON) | 2560×1440 (back buffer OFF) |
|---|---|---|
| fill whole screen | 1.80 ms · 1.80 cyc/px | 6.37 ms · **3.98 cyc/px** |
| gradient whole screen | 2.93 ms · 2.93 cyc/px | 12.75 ms · **7.97 cyc/px** |
| bg_snapshot | 6.89 ms | **0.00 ms — bg_ok=0, dragging dead** |
| shadow 600×460 soft=12 | 4.34 ms · 33 cyc/px | 3.28 ms · 25 cyc/px |
| rrect 600×460 | 0.25 ms · 2.1 cyc/px | 0.33 ms · 2.7 cyc/px |
| 10 dock icons | 0.30 ms · **29.5 cyc/px** | 0.21 ms · 20.9 cyc/px |
| 40 lines of AA text | 4.98 ms | 4.50 ms |
| one window, full chrome | **5.12 ms** | 3.82 ms |
| **whole desktop, 3 windows** | **19.98 ms** | **26.80 ms** |

### What that table proves

**1. The resolution cliff is real, and now measured.** The per-pixel cost of a
gradient goes from 2.93 to **7.97 cyc/px** — 2.7× worse *per pixel* — purely
because `back_on = 0` forces the slow `put_pixel` path instead of the row fill.
And `bg_snapshot` returns in **0.00 ms** at 2560×1440 because `bg_ok = 0`:
that is window dragging, silently doing nothing, on a stopwatch.

**2. It is worse than this on real hardware.** The harness uses ordinary RAM for
"VRAM". On the real machine, with `back_on = 0`, every `fb_get_px` in every
shadow and every anti-aliased glyph reads actual write-combining VRAM at the
30–50× penalty `fb.c:76` documents. **Treat the 2560×1440 column as a floor,
not an estimate.**

**3. Shadows dominated a window.** 4.34 ms of a 5.12 ms window — **85%.**

**4. A full desktop redraw was 20 ms at 1920×1200 and 27 ms at 2560×1440.**
That is a 50 fps and a 37 fps ceiling with *nothing else running* — no input, no
apps, no present. Not enough headroom to animate anything.

## How this compares to a real GPU compositor — measured on the same laptop

The honest comparison. `gnome-shell` runs on this box on the **same Intel UHD
(CML GT2)** the modeset driver targets, so the GPU number is measured on the
exact hardware zlOS runs on, not quoted from a spec sheet.

Test: 300 alpha-blended full-screen quads to an **offscreen** GLX pixmap (never
appears on the desktop, disturbs nothing).

```
GL renderer: Mesa Intel(R) UHD Graphics (CML GT2)
300 blended full-screen quads at 1920x1200 in 0.133 s
  -> 5.21 Gpixel/s blended
  -> one 1920x1200 blended layer costs 0.4426 ms
```

Against zlOS on the same machine, from `fbbench`:

| Operation | zlOS (CPU) | Intel UHD (GPU) | ratio |
|---|---|---|---|
| plain fill | **1.28 Gpx/s** (1.80 cyc/px) | 5.21 Gpx/s *blended* — plain is higher | **~4×** |
| subpixel-blended text | **0.22 Gpx/s** | 5.21 Gpx/s | **~24×** |

### The surprise: only ~4× on plain fills

Because the GPU is **integrated**. It shares the same DDR4 as the CPU, so for
straightforward pixel writes both hit the same memory-bandwidth wall. This is not
a discrete card with its own 500 GB/s VRAM. A hand-written CPU renderer is
genuinely competitive here in a way it would not be against a dGPU.

Blending is where it diverges — ~24× — because the GPU blends in fixed-function
ROPs at full rate while the CPU runs a gamma-correct table lookup per channel.

### But the ratio is not the point. The budget is.

Everyone gets **16.67 ms per frame** at 60 Hz — and this panel is measured at
59.998 Hz, so that is the real deadline.

| | per frame | % of budget |
|---|---|---|
| GNOME/Windows, typical frame | ~0.44 ms or less | ~3% |
| **zlOS @1920×1200, FULL redraw** | **4.88 ms** | **29%** |
| **zlOS @2560×1440, FULL redraw** | **15.06 ms** | **90%** |

**zlOS is already inside the frame budget at 1920×1200.** That is the real
finding. The renderer is not too slow.

The difference is *what is being redrawn*. GNOME redraws the damaged region —
typically a few percent of the screen — and does it on the GPU while the CPU
stays free for applications. zlOS redraws **everything, every time**, on the only
core it uses.

So the gap is not 4× of hardware. It is 100% versus ~2% of the screen, which is
roughly **50×** of avoidable work — and it is entirely a software problem.

Which is, again, damage tracking. Third document, same conclusion.

## What is still not measured

**Nothing measures a frame inside the running kernel.** That is still true and
still the first problem for anything the renderer does not cover.

`cpu_tsc()` and `cpu_tsc_khz()` exist and work (`cpu.c:212`, `cpu.c:220`, TSC
calibrated against the PIT). But they are **not exposed to zl** — there is no
`tsc` builtin, so no desktop code can time anything.

`idt_ticks()` is 100 Hz — 10 ms resolution. Too coarse to measure a frame that
should take 16 ms.

**Action: add a `tsc()` builtin and put a frame time on screen.** Everything
below is arithmetic and reading; none of it is a measured frame time, because
there is no such measurement yet. Do not optimise before this exists.

## What was measured

Compiled `fb.c` at the real `-O2` and counted divide instructions:

```
gcc -m32 -O2 -ffreestanding ... -c fb.c
objdump -d | grep -c 'idiv|div '   ->  17
```

Where they are:

| Function | Divides | Runs |
|---|---|---|
| `fb_cube` | 6 | per vertex — the 3D demo, irrelevant |
| `fb_shade` | 3 | **per pixel** |
| `fb_gradient` | 3 | per **row**, not per pixel — negligible |
| `fb_setup` | 2 | once |
| `fb_icon24` | 2 | **per pixel** |
| `fb_shadow` | 1 | **per pixel** |

**The blend path has zero divides.** `blend_rgb` and `blend_sub` write `/ 255`,
and GCC strength-reduced it to a multiply-and-shift as expected. The hand-written
`div255()` helper at `fb.c:198` is **dead code — defined and never called.**
Harmless, but delete it or use it; right now it implies a problem that the
compiler already solved.

### The one that matters: `fb_shadow` — and the wrong answer first

`fb.c:598` — `int amount = 62 - (62 * d) / (soft + 1);`

`soft` is a runtime value, so GCC cannot strength-reduce it. That is a real
divide **per shadow pixel**, and this document originally recommended tabling it.

**That recommendation was wrong, and measuring it is what proved it.** Building
a variant with the divide replaced by a lookup table made the shadow **~25%
slower**, three runs out of three:

```
run 1:  baseline 21.28   tabled 27.81   cyc/px
run 2:  baseline 22.08   tabled 23.24
run 3:  baseline 19.92   tabled 24.19
```

The loop is not arithmetic-bound. It does roughly twenty cycles of scalar work
per pixel — bounds tests, a chebyshev distance, a read-modify-write, and
`mark()` inside `put_pixel` — and the divide hides inside that. Adding a table
adds a memory access and wins nothing.

**The real problem was algorithmic.** `fb_shadow` darkened the window's *entire
footprint*, and the caller drew the window on top of it immediately afterwards.
About **90% of the work was painted over before anyone saw it.**

**Fixed and shipped.** The loop now skips the covered rectangle, inset by
`SHADOW_SKIP_INSET` (16) so the rounded corners still get their shadow.
Verified pixel-identical: FNV hash over the whole 1920×1200 back buffer, after
shadow + rrect + rrect + gradient + text, is unchanged.

| | before | after | |
|---|---|---|---|
| shadow 600×460 soft=12 | 4.34 ms | **0.61 ms** | 7.1× |
| one window, full chrome | 5.12 ms | **0.90 ms** | 5.7× |
| whole desktop, 3 windows @1920×1200 | 19.98 ms | **4.88 ms** | 4.1× |
| whole desktop @2560×1440 | 26.80 ms | **15.06 ms** | 1.8× |

**The lesson is the reason `hosttest/fbbench.c` now exists.** An instruction
count is not a measurement. The static analysis pointed at the divide; the
measurement pointed at the algorithm; only one of them was right.

### `fb_icon24`

`fb.c:929` — `int a = ic[y / sc][x / sc];` — **two divides per pixel**, on top of
the nearest-neighbour quality problem already documented in `desktop-look.md`.

Fixing the quality problem (regenerate the atlas at 48×48) removes the divides
as a side effect. Two bugs, one fix.

## The structural costs, in order

### 1. Redrawing what did not change — by far the biggest

There is one dirty rectangle (`fb.c:89`), and `mark()` grows it to cover
everything. A clock ticking in one corner plus a monitor updating in the other
unions to the **whole screen, every second**.

Dragging a window should touch perhaps 5% of the screen. Today it touches 100%.

**The damage system in `archive/superseded/desktop-TODO.md` step 0c is not an optimisation. It is
the requirement.** At 1440p on one core, a full-screen redraw per frame is
inherently marginal — no amount of per-pixel tuning fixes drawing 3.7M pixels
when 200k changed.

### 2. Reading back from video memory

Every shadow pixel, every anti-aliased glyph edge and every rounded corner calls
`fb_get_px`. With the back buffer on, that is a RAM read — fine. With it **off**,
it is a VRAM read, which `fb.c:76` measures at **30–50× slower**.

And the back buffer is off at 2560×1440. See `archive/superseded/desktop-TODO.md` step 0a. This is
the same bug appearing for the third time in these docs, which is a sign of how
central it is.

### 3. The present blit

`fb_present()` copies the dirty box from RAM to VRAM. Full screen at 1440p is
**14.1 MB per frame**. Write-combining makes writes cheap, but it is not free,
and it is another reason the dirty box must be small.

### 4. Builtin dispatch

`zl_calln` walks up to 301 string compares per call (`runtime_kernel.c:483`).
The graphics vocabulary sits at 255–287, and `mouse_x`/`mouse_y`/`mouse_btn`
at 285/286/287 are called every frame.

This scales with **call count, not pixel count**, so it never dominates a
full-screen redraw — but it is a reason to keep the per-window repaint loop in
C, and to prefer few coarse builtins over many fine ones.

## The unused 4×: three cores are parked

`smp.c:79`, `smp_ap_main` — the three application processors boot, register
themselves, and then:

```c
for (;;) __asm__ volatile("cli; hlt");
```

They halt forever. **All drawing happens on one of four cores.**

The comment explains why, and it is correct: nothing there is protected by a
lock, so an AP touching the console or a driver would race.

**But drawing into disjoint horizontal bands of the back buffer needs no lock at
all** — no shared mutable state, no overlap, each core writes only its own rows.
Split a damage rectangle into 4 bands, wake the APs, join, present.

That is a real 4× available on the most expensive operation in the system, using
SMP code that already works. It is not free — it needs a barrier primitive and
careful thought about what an AP may call — but it is the largest single lever in
the project after damage tracking.

**Do it after damage tracking, not before.** Parallelising a redraw that should
not be happening is the wrong order.

## Ranked

1. **Add a `tsc()` builtin and show frame time.** Optimising without measurement
   is guessing. Everything below is unproven until this exists.
2. **Damage tracking** (`archive/superseded/desktop-TODO.md` 0c). Structural. Nothing else compares.
3. **Back buffer at native resolution** (0a). Turns 30–50× VRAM reads back into
   RAM reads.
4. **Table the shadow falloff.** Removes ~302k divides per window redraw. Small,
   contained, obvious.
5. **Fix `fb_icon24`.** Quality bug and 2 divides per pixel, one fix.
6. **SMP band rendering.** Biggest remaining lever, most work, do it last.
7. Delete or use `div255`.

## The trade, stated plainly

Motion is the biggest visual win available and the most expensive thing to add.
A menu fading in over 4 frames means compositing it 4 times.

**That is affordable only once damage tracking exists** — a fading menu damages
the menu's rectangle, not the screen. Right now it would damage everything, four
times, and feel worse than no animation.

So the order is forced: **speed first, then motion.** Items 1–3 above are what
buys the right to animate anything.

---

Rendering bugs: `desktop-look.md` · Task list: `archive/superseded/desktop-TODO.md` ·
Architecture: `archive/superseded/desktop-plan.md` · Intro: `desktop-build-guide.md`

---

# Measured again, 2026-08-18 — after the overnight run

Replacing the table in Part 2. Same machine, `hosttest/fbbench`, min of 7 runs
internally and best of 6 invocations, interleaved A/B so host load cancels.

## G1 — SIMD in the fill and the blit

`cpu.c` had detected SSE/SSE2/SSE3/SSSE3 since it was written and **nothing
used it**. Now `fb_fill_px`'s row fill and `fb_present`'s 32-bpp blit do.

| @1920×1200 | scalar | SIMD | |
|---|---|---|---|
| fill whole screen | 0.86 cyc/px | **0.39** | 2.2× |
| fill + present (blit) | 2.05 cyc/px | **1.35** | 34% faster |
| whole desktop redraw | 4.35 cyc/px | **4.00** | 8% faster |

The desktop figure is the honest one and it is small on purpose: that scene is
dominated by shadows, rounded corners and antialiased text, all of which are
*blend*-bound, not store-bound. Fills are 2.2× and the whole frame is 8%,
because fills were never most of the frame.

**Correctness:** the FNV scene hash is byte-identical between the two paths at
all three resolutions. A vector store that produced *nearly* the same pixels
would be worse than none.

**Two things worth knowing before extending this.**

- **SSE is only enabled on the 64-bit path.** `boot64.S` sets `CR4.OSFXSR`;
  `boot.S`, the 32-bit multiboot entry `verify.sh` boots, never touches CR4, so
  an SSE instruction there *faults*. The guard is `#ifdef __SSE2__`, which gcc
  defines for x86-64 and not for `-m32` without `-msse2` — so it needs no
  build-system change and cannot drift out of step with the build. Checked
  mechanically: the 32-bit object contains **zero** `xmm` instructions.
- **GCC will not do this for you at `-O2`.** The "very-cheap" cost model
  refuses any loop with a runtime trip count, because it would need a scalar
  epilogue. `objdump` on the `-O2` object shows vector code in `fb_rrect`,
  `gamma_init` and `dmg_add` — and none at all in `fb_fill_px`, `fb_present`,
  `fb_gradient` or `fb_shadow`, which are the hot ones.

**What is deliberately NOT vectorised: `blend_rgb` and `blend_sub.`** They are
three lookups into `srgb_to_lin` per pixel, and a gather is the one thing SSE2
cannot do. Vectorising around it means giving up the gamma-correct
linear-light blend, which is the best thing about this renderer. `-DFB_NO_SIMD`
rebuilds the scalar path so this comparison stays a command rather than an
opinion — see DECISIONS.md #25 for why that matters here specifically.

## What the damage list did to the same picture

Redrawing two corners of the screen (a clock and a monitor) rather than
unioning them:

| | presented | as one box | |
|---|---|---|---|
| 1920×1200 | 33,000 px | 2,180,800 px | **66× less** |
| 2560×1440 | 33,000 px | 3,528,000 px | **107× less** |

That is a far larger lever than SIMD, and it is the one this document's Part 2
predicted: *"GNOME redraws ~2% of the screen per frame; zlOS redraws 100%. That
is ~50× of avoidable work."* Measured at 66–107× for the two-corner case.

## Still the biggest remaining lever: G2, three parked cores

Unchanged and still **excluded** from unattended work. `smp.c:79` parks three
of four cores in `cli; hlt`, and splitting the back buffer into disjoint
horizontal bands needs no lock. A real 4×. Concurrency bugs in a run with
nobody watching are the wrong risk; this wants a supervised session.

---

## H3 — the tiled rasterizer, and why it is NOT wired in

`fb3d.c` now has `fb3d_tri` / `fb3d_poly`: a 16×16 tiled barycentric
rasterizer, the shape SerenityOS used to run Quake III with no GPU. It is
**correct and it is slower**, and both halves of that need saying.

### Correct, against an oracle rather than against my eye

`hosttest/tritest.c` rasterizes the same polygons with the existing scanline
`fill_poly` and with the new tiled path, into two buffers, and compares them
pixel for pixel — over the shapes that actually break rasterizers: thin
vertical and horizontal slivers, exact axis-aligned edges, a degenerate
zero-area triangle, both windings, and triangles hanging off every corner of
the clip rectangle.

That oracle earned itself immediately. The first version had **all six**
incremental step constants sign-inverted. The cube still looked perfect,
because interior tiles never step — only the partial tiles at the edges were
wrong. The comparison found 13,031 differing pixels on one large triangle, and
a thin sliver drawing almost nothing, because a sliver is nearly *all* partial
tiles. Looking at a spinning cube would never have caught it.

Residual differences are boundary-only — about 300 pixels on a triangle with a
~1,600 pixel perimeter, all on one edge. The two use different fill rules on
the exact boundary (half-open rows versus `>= 0` edge functions), which is a
legitimate design difference rather than a bug. Interior disagreement would be
enormous, not marginal.

### Slower, by a factor of 2.8, and here is exactly why

| 400 large triangles | best of 6 | range |
|---|---|---|
| scanline `fill_poly` | **30.1 µs** | 30.1–37.5 |
| tiled barycentric | 84.7 µs | 84.7–90.1 |
| | **2.81× slower** | |

*(A single run of this reads anywhere from 2.4× to 2.8× depending on host load,
which is why the figure above is best-of-6 and the range is printed next to it.
The ratio is stable; the absolute microseconds are not.)*

Two rounds of optimisation are already in it — adjacent interior tiles merge
into one `fb_fill_px` per tile row, and the four corner values come from one
edge evaluation plus the step constants rather than twelve. Together they took
about 20% off and did not change the verdict, because neither addresses the
real cost.

**A scanline fill is already optimal for a flat triangle.** It emits one
full-width `fb_fill_px` per row — 500 calls, each hitting the SIMD fill — and
that is close to memory bandwidth. The tiled path's expense is the ~25,600
per-pixel edge tests on boundary tiles, which is inherent to barycentric
rasterization and is precisely what SIMD removes by testing 4 or 8 pixels at
once.

So **`fb_cube_filled` still uses `fill_poly`.** Shipping the slower path
because it is more sophisticated would be DECISIONS #25 again with the sign
flipped — that entry records an optimisation argued from an instruction count,
shipped, and measured 25% slower afterwards. The measurement is what decides.

### What the structure buys, which is why it exists anyway

Nothing about a flat fill. Everything about what comes next: a depth test, a
texture lookup and a vector unit all need somewhere to hang per-pixel work, and
a tile is that place. The order in `desktop-TODO` is `fb_clip` → tiled
rasterization → SIMD → depth buffer → textures. Two are done. **The next step
is SIMD across the partial tiles**, and it is also the step that would make
this table read the other way round.

---

# The full baseline, after the whole overnight run

`hosttest/fbbench`, best of 5 invocations, host load ~1.6. Replaces every
earlier table in this document. Milliseconds track host load; the *ratios* are
what to read.

| | 1920×1200 | 2560×1440 | 3840×2160 |
|---|---|---|---|
| fill whole screen | 0.396 | 0.847 | **30.613** |
| gradient whole screen | 1.480 | 2.433 | **33.424** |
| fill + present (blit) | 1.358 | 2.503 | **31.647** |
| bg_snapshot | 3.442 | 5.663 | 15.119 |
| shadow 600×460 | 0.609 | 0.629 | 0.668 |
| rrect 600×460 | 0.042 | 0.053 | 1.056 |
| 10 dock icons | 0.079 | 0.081 | 0.079 |
| 40 lines AA text (mono) | 3.430 | 3.552 | 3.428 |
| 40 lines proportional | 3.738 | 3.859 | 3.766 |
| 40 rows console text | 2.596 | 2.899 | 6.863 |
| 200 diagonal lines | 3.341 | 3.452 | 3.617 |
| **one window, full chrome** | **0.735** | **0.778** | 2.824 |
| **whole desktop redraw** | **3.953** | **5.254** | ~~44.370~~ → **9.094** |

**The ThinkPad's 2560×1440 draws a whole desktop in 5.25 ms of a 16.67 ms
budget** — and before this run it could not use the back buffer at that
resolution at all.

## Read the 4K column, because it is an argument

Nothing is wrong with 4K. `back_on` is 0 there, so every primitive goes
straight to VRAM through `put_pixel` instead of through the fast row path, and
a full-screen fill costs **77× what it costs at 1920×1200** for 3.6× the
pixels. The operations that do *not* collapse — shadow, icons, text — are the
ones already dominated by per-pixel blending rather than by stores.

That single column was the case for **C4** — and it turned out not to need C4
at all. **Done 2026-08-18, and the prediction was almost exact:**

| 3840×2160 | before | after |
|---|---|---|
| whole desktop redraw | 44.370 ms | **9.094 ms** |
| | | **4.9× faster**, 2.53 cyc/px |

`fb_setup` now computes the back buffer's **base**, not just its limit. If the
mode does not fit in `back`'s own 16 MiB span it falls back to the drag arena
at 128 MiB, which is 48 MiB — the only span in the map large enough for 4K's
31.6 MiB.

**The trade is real and the boot log states it:** dragging turns off, because
the buffer it would snapshot into is now the back buffer. That is not much of a
loss, because at that mode the drag buffers were unusable anyway — `bg_buf`
needs the same 31.6 MiB and has 32, leaving nothing for a back buffer beside
it. The choice was never "back buffer or dragging"; it was "back buffer or
neither".

`fb_bg_snapshot` and `fb_grab` **refuse** while the arena is taken, and the
harness asserts the refusal — a snapshot that "worked" there would be
overwriting the very thing it was snapshotting, silently.

**The 4K scene hash changed**, `2275f08098c8291e` → `e735d8737eeff842`, and
that is intended rather than a regression: subpixel text is gated on `back_on`
(`fb.c`'s `draw_glyph`), so 4K went from grayscale AA to subpixel. The pixels
are different because they are better. Recording it here because an unexplained
hash change is exactly what that hash exists to catch.

C4 still supersedes this properly: deleting the sticker-drag machinery frees
the arena outright, and dragging comes back through damage-based repaint, which
needs no snapshot at all. Blocked on `kernel.zl` (T-9).

## What each change contributed, all measured

| | |
|---|---|
| damage list (B3) | two-corner update **66–107× less** presented area |
| SIMD fill and blit (G1) | fill **2.2×**, blit **34%**, desktop 8% |
| icon atlas (A1) | **2×** on the icon path, 20.1 → 10.0 cyc/px |
| shadow skip (0i, pre-run) | whole desktop 19.98 → 4.88 ms |
| ink-width glyph blit (F3) | proportional text 35% → **9%** over mono |
| Wu lines (A2) | **4.3× slower** per line pixel — a quality change, and the cost is stated rather than hidden |
| tiled rasterizer (H3) | **2.8× slower** than scanline — so *not* wired in |

Two of those seven are slower and both are still here for stated reasons. That
is the point of measuring in both directions.
