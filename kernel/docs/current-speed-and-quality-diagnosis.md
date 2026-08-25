# Current speed and visual-quality diagnosis

**Updated 2026-08-21 from a complete physical ThinkPad ZLLOG boot, followed by
the write-combining and cursor-copy fix in the current uncommitted tree.** This
is the current diagnosis for the desktop that actually boots. It separates
renderer cost, end-to-end feel and visual quality because treating all three as
"the OS is slow" had been sending work to the wrong layer.

For the deeper comparison with Windows, Linux and macOS—including RAM, retained
surfaces, GPU presentation and the boxed zl backend—read
[`research/why-mainstream-desktops-feel-fast.md`](research/why-mainstream-desktops-feel-fast.md).

## Blunt result

The physical ThinkPad trace found the main lag source. zlOS was copying pixels
into the 2560x1440 scanout at about **111.59 MB/s** because the live firmware
mapping was effectively uncacheable. Full-screen copies therefore took roughly
131-137 ms. Input routing and the zl app tick were tiny by comparison.

The current tree now retypes that existing identity mapping to
write-combining after zlOS installs its own IDT and before it starts other CPUs
or paints the desktop. It does not create a mixed-cache alias. The QEMU UEFI
gate exercised the firmware 1 GiB-page split, and the following physical rerun
proved the effective type changed from uncacheable to write-combining on the
ThinkPad itself.

A second definite lag bug was fixed at the same time. Moving the software cursor
far across the screen used to merge its old and new 16/32-pixel patches into one
enormous bounding rectangle; even an unchanged cursor was restored and redrawn.
Old and new cursor patches now stay separate, unchanged cursors copy nothing,
and cursor-only presents are included in input-to-present telemetry.

The next passes are reducing genuinely large window damage, splitting
compositor attribution further, replacing integer layout
scaling with q8 geometry, and establishing a quieter visual direction. More RAM,
more applications or more privilege rings would not repair this measured copy
bottleneck.

## Evidence from the current build

A fresh native `wmbench` run at 1920x1200, best of seven, reports:

| workload | time |
|---|---:|
| dragged compositor frame, everything | 3.454 ms |
| without Terminal scrollback | 2.583 ms |
| chrome and wallpaper | 2.493 ms |
| Terminal scrollback attribution | 0.871 ms, 25% |

That is comfortably below the 16.667 ms frame budget on this host. It proves
the tested compositor path is capable; it does not prove the booted OS feels
fast.

A fresh QEMU boot screenshot at 1920x1200 reports `frame ms 13`, `peak ms 13`,
`late 0`, and `ui 1x`. This is one sample rather than a distribution, but it is
already about 3.8 times the native benchmark result and leaves only about
3.7 ms before a missed 60 Hz frame.

The two runs are not equivalent:

- `wmbench` drives a controlled window drag and does not enable `zsweep`;
- `wm_boot_start()` enables `zsweep` for every real desktop session;
- QEMU includes the guest runtime and emulated display path;
- that screenshot's on-screen counter had no per-phase attribution or event
  timestamp.

## Persistent QEMU flight-recorder proof

The later ZLLOG proof replaces that one screenshot with persistent per-phase
records. The exact retained evidence is:

- source image: `/tmp/zllog-qemu-proof-postfix.9f43Tm/boot.img`, SHA-256
  `01f1626acf2c6873b572ab6ddc5ff0ba11e026308a1bd53b1950eb253edb21d2`;
- exported report: `/tmp/zllog-qemu-proof-postfix.9f43Tm/final.json`, SHA-256
  `0a9fd5d2798ecdc493d379e21e1a74ce8410c57e4916500ecc775a1567316857`;
- build identity stored in both boot slots:
  `c76a73b4f34a7a11cdd9b7905d25d08070460650c68755a25bc177ef781b125e`;
- `python3 tools/zllog.py inspect <image>` selects valid superblock A,
  generation 5; superblock B is also valid at generation 4;
- `python3 tools/zllog.py extract <image> --all --json` reproduces `final.json`
  byte-for-byte;
- boot 2, slot 0 is complete with records 1..183; boot 4, slot 1 is complete
  with records 184..407. Both slots report zero recorder drops and the reader
  reports no warnings.

The serial evidence shows a 1920x1200 UEFI QEMU boot from an xHCI USB disk. It
also says there is no Intel GPU on the emulated bus, so vblank pacing was
unavailable. These results therefore describe QEMU's BGA/framebuffer path only.

### HID-interaction boot: measured frame distribution

Boot 4 retained 38 painted-frame records: every one of the 35 late frames plus
three periodic healthy samples. Values are microseconds. Percentiles use the
nearest-rank method over those 38 retained records.

| phase | min | p50 | p95 | p99 | max | mean |
|---|---:|---:|---:|---:|---:|---:|
| input poll + route | 29 | 40 | 126 | 135 | 135 | 47.13 |
| app/animation tick | 18 | 29 | 62 | 62 | 62 | 33.37 |
| compositor repaint | 9,060 | 19,969 | 21,665 | 21,833 | 21,833 | 19,080.55 |
| vblank wait | 0 | 0 | 0 | 0 | 0 | 0 |
| framebuffer present | 532 | 852 | 2,337 | 5,511 | 5,511 | 1,053.53 |
| total painted frame | 11,010 | 20,945 | 22,747 | 23,027 | 23,027 | 20,216.21 |

Across the retained records, phase totals were 1,791 us input, 1,268 us tick,
725,061 us compositor, 0 us vblank and 40,034 us present. Against 768,216 us
of total frame time, that is 0.23%, 0.17%, **94.38%**, 0% and 5.21%
respectively; the remaining 62 us is timer/rounding overhead. The dominant QEMU
phase is therefore not ambiguous: it is compositor repaint.

Thirty-five painted frames exceeded the 16,667 us deadline. That count is
exact for this boot because every late frame is retained and the slot dropped
no records. **It is not valid to call this 35/38 of all painted frames.** Healthy
frames are deliberately sampled only once per 60 paints, so the journal does
not contain the denominator needed for a late-frame percentage or an unbiased
all-frame percentile distribution. The three periodic healthy samples were
11.010, 11.119 and 14.706 ms total; the 35 late records ranged from 18.763 to
23.027 ms.

Boot 2 provides one earlier periodic sample, not a distribution: 315 us input,
27 us tick, 5,141 us compositor, 0 us vblank, 3,267 us present and 8,751 us
total. It proves the QEMU path can produce a healthy painted frame, while boot
4 proves the tested interaction can drive repeated misses.

### Input evidence from the same boots

Boot 4 retained three logical pointer records: an ordinary no-button sample,
a button-down edge and a button-up edge (`code` 0, 1, 0). Ordinary motion is
sampled one in sixteen, while button edges are exact, so three is a retained
record count rather than the number of physical HID reports. It also retained
five exact character records for `q`, `u`, `i`, `t` and newline. Boot 2 contains
the same five character records and no retained pointer record.

There is no `INPUT_DROP` record in either boot, so measured logical input-queue
drops were zero. Boot 4's observed enqueue depth was one for all three retained
pointer records; its single sampled active-batch record processed one event and
ended at depth one. The five shutdown characters reached depths one through
five because they were enqueued together. Thus the observed maximum queue depth
was five overall and one for retained pointer events. Pointer and ordinary
batch sampling mean an unrecorded transient pointer depth is still possible;
zero drops is the stronger exact result.

This rejects one hypothesis in QEMU: the measured cost is not time spent
polling/routing input, and this trace shows neither queue overflow nor an input
backlog. It does **not** measure device-completion-to-enqueue delay or associate
one physical input with the frame that presented its result, so it does not yet
give input-to-photon latency.

### What this says about zl language speed

Nothing conclusive. `tick_us` includes zl app ticks and is tiny in this run, but
`compositor_us` includes `wm_repaint()`, zl `hook_draw` calls, window chrome,
wallpaper/damage work and framebuffer primitives in one bucket. A 19.969 ms
compositor p50 therefore does not say whether boxed zl execution, the amount of
damaged content, QEMU memory behaviour or a C renderer primitive is responsible.
The native 3.454 ms `wmbench` result makes QEMU/emulated-memory and live-workload
differences serious candidates, but it does not exonerate zl app drawing until
that compositor bucket is split.

## The first concrete speed problem: the permanent sweep (fixed)

The old boot path called `wm_sweep(1)`. The effect is a full-width band 34% of the
screen height. At 1920x1200 that is roughly 1920x408 pixels before the union of
the old and new positions is considered. Whenever its quantised position
changes, the compositor damages that band and redraws every visible window it
intersects.

The effect moves across 816 vertical pixels in seven seconds in six-pixel
steps: about 136 heavy repaint steps, or roughly 19% of the 700 timer ticks in
one loop. That is not a continuous low cost. It is a recurring expensive frame
about fourteen times per second for a tint whose peak alpha is only 11/255.
That cadence is exactly the shape people perceive as unevenness.

The reference keeps the sweep behind an optional CRT overlay. zlOS previously
turned it on at every boot. The boot default is now off, while the compositor
effect and its explicit enable API remain. That removes roughly fourteen large
decorative repaint steps per second without removing any desktop function.

## What the flight recorder still cannot answer

The recorder now separates input polling/routing, app tick, compositor, vblank,
present and total wall time, and it records logical queue depth/drop evidence.
Every sampled or late frame also carries the exact framebuffer damage rectangle
count and pixel area. That separates a slow small update from a frame which was
slow because it redrew most of the panel.
It still does not timestamp raw device completion separately from logical queue
entry, associate an input event with its first presented frame, or split the
large compositor bucket into zl app draw, wallpaper, chrome and framebuffer
primitives. A frame can therefore be under budget while a physical event waited
before queue entry, and the current QEMU bottleneck can still hide several
different causes inside `wm_repaint()`.

Extend the current calibrated timestamps and counters across the missing links:

```text
device event
  -> queue entry
  -> route start/end
  -> app tick/draw
  -> compositor repaint
  -> vblank wait
  -> framebuffer present
```

The next instrumentation split should publish current, peak and p50/p95/p99
values for:

- input queue wait;
- app work;
- compositor work;
- present work;
- input-to-present latency;
- maximum queue depth;
- painted, late and lost frames.

The Intel path currently waits for vblank inside the timed section while the
QEMU path does not. Keep both `cpu_work_us` and `wall_frame_us`; otherwise a
correct vblank wait can be mistaken for slow drawing.

## Physical ThinkPad measurement — completed 2026-08-21

The latest automatic ZLLOG run produced one clean COMPLETE boot with 1,168 of
1,168 CRC-valid records, globally contiguous sequence numbers, no recorder
drops and a clean `quit`. Storage was initially refused, then recovered and
mounted automatically. The firmware mode was 2560x1440x32 with a 10,240-byte
pitch and scanout at physical `0xC0000000`; the recorded initial effective cache
type was uncacheable.

The desktop counted 506 painted frames, 378 late frames and 1,386 lost frame
deadlines. The recorder retained 210 frames, deliberately biased toward late
ones:

| phase (us) | p50 | p95 | max |
|---|---:|---:|---:|
| input | 5 | 22 | 501 |
| app tick | 35 | 70 | 92 |
| compositor | 7,838 | 67,741 | 117,836 |
| vblank wait | 7,845 | 15,985 | 17,215 |
| framebuffer present | 67,241 | 135,895 | 136,876 |
| total | 78,589 | 206,628 | 262,137 |

For the 210 paired present-size/time records, least-squares regression was
`present_us = 3130.48 + 0.008961511 * bytes`, correlation 0.9977. The slope is
about 111.59 MB/s and a full 14,745,600-byte frame lands around 131-137 ms. This
near-linear size relationship is much stronger evidence than simply observing
that the desktop feels slow: the final CPU copy into the scanout mapping is the
primary physical bottleneck.

There were 159 retained input-to-CPU-present correlations: p50 114.895 ms and
p95 236.142 ms. These stop at the CPU present boundary, not panel photon output.
The old recorder omitted cursor-only presents, so those numbers undercounted the
worst pointer path; the current tree records cursor-only timing explicitly.

## Write-combining fix — physical proof complete

The UEFI loader first asks the firmware CPU architecture protocol to mark the
framebuffer write-combining and records the exact result. If firmware refuses,
the kernel safely retypes the existing identity leaf in place. It can split a
firmware 1 GiB leaf into permanent 2 MiB leaves, preserves unrelated mapping
flags, selects a live or unused high PAT slot, synchronizes that PAT value on
application processors, and fences every CPU's write-combining blit before it
publishes completion.

Host tests cover the 30 stable PAT/MTRR combinations, PAT-index selection and
leaf-bit placement. OVMF exercises the actual 1 GiB split. Cursor regression
tests prove that a far movement creates two tiny patches and that an unchanged
cursor creates zero damage.

The new physical boot completed with 6,503 of 6,503 CRC-valid records, contiguous
sequences and zero drops. Its display-state records prove the live transition
from uncacheable to write-combining. Across 1,291 paired present-size/time
records, regression was approximately `present_us = 6.00 + 0.000141059 * bytes`
with correlation 0.9799, or **7,089 MB/s**. That is about **63.5 times** the old
111.59 MB/s slope. Seven full-screen 14,745,600-byte presents took 2.036-2.155
ms, median 2.070 ms, versus roughly 131-137 ms before.

The complete-session counters were 9,563 painted frames, 947 late and 1,504
lost deadlines. The retained distribution is still selection-biased, but
present is no longer the owner: its p50/p95/max were 59/1,325/3,117 us, while
compositor work was 223/53,655/104,628 us. Of 366 retained late frames, the
largest phase was compositor work in 247 and vblank waiting in 119; present was
largest in none. The next speed target is therefore the repeated 3.2-3.5
million-pixel compositor/draw path, not framebuffer cache policy.

That physical phase rerun is now complete: it retained 5,967 CRC-valid records,
1,043 phase samples and zero recorder drops. It eliminates the ambiguous
combined-frame explanation. The 462 retained late frames had compositor
median/p95/max **55.698/163.192/220.773 ms**. The longest frame was 220.773 ms:
106.800 ms desktop restoration, 110.106 ms C chrome, 3.840 ms zl app drawing
and 3.101 ms present. In the repeated many-window path, a 3.2-3.5 million-pixel
damage region visited and redrew 12-13 windows; C chrome alone cost about
130 ms, app drawing 23-28 ms and cached desktop restoration 9-13 ms. The final
write-combining present remained about 3 ms.

This is the current real bottleneck: a changed region causes the compositor to
re-run every intersecting visible window's shadow, frame, title controls and
app draw routine. It is not USB throughput, pointer queueing, final framebuffer
cache policy, or a generic claim that the zl language is slow. The immediate
small-loop experiment (removing repeated shadow divisions while preserving
pixels) measured no meaningful gain because the compiler already strength
reduced those divisions, so it was intentionally not retained. The next real
speed project is retained window surfaces: draw each window into a RAM surface
when its content changes, then compose/copy that surface while moving,
uncovering or stacking windows. That is the mechanism Windows, Wayland and
macOS use to avoid re-running application and chrome drawing for unrelated
damage. It needs a bounded 64 MiB heap-backed surface budget, explicit
invalidation, a safe fallback when a surface cannot fit, and tests for overlap,
resize, close and workspace switching; it is the right substantial fix rather
than another cosmetic micro-optimization.

The new cursor-only records also close the old input blind spot. Across 608
cursor-only correlations, hardware-boundary-to-CPU-present latency was p50
7.827 ms and p95 16.262 ms. Non-cursor interactions remain much worse at p50
29.331 ms and p95 101.963 ms because they can trigger the expensive compositor
work. These are CPU-present timings, not panel-photon measurements.

## Why it looks low quality

### 1. The layout is too small for the screen

`fb.c` calculates `ui_scale_q8`, but `kernel.zl` defines `ui()` as the rounded
integer `ui_scale()`. Most window sizes, positions, padding and radii multiply
by that integer. At 1920 pixels it is exactly 1, so the desktop places a
1280-wide reference composition in the middle of a 1920-wide panel. The result
is tiny controls surrounded by unused space. At intermediate modes the q8 text
and integer geometry can also scale at different rates.

Use q8 design-unit helpers for all geometry. Scale against both width and
height, clamp for readability, then lay out within a centred work area. Do not
keep multiplying hundreds of literals by a rounded integer.

### 2. Typography reads as firmware, not a desktop

The boot reports an 8x16 terminal cell. Terminal text can remain monospaced,
but the surrounding labels are small, tightly packed and visually close to the
raster console. Window titles, navigation, metadata and controls need a larger
type ramp, more line height and stronger distinction between UI text and
terminal text.

### 3. The boot composition looks accidental

Terminal, Files and System Monitor deliberately copy the reference's overlapping
coordinates. On the current desktop, Terminal obscures much of Files and the
Files body reports that no zlfs volume is mounted. A large, mostly empty black
surface hidden behind another window reads as broken, not layered.

The first boot should show useful state. Open Terminal plus System Monitor, and
only open Files automatically when a volume is mounted; or move the windows so
each application has an obvious role and visible content.

### 4. The background competes with the work

The olive/lime glow, two technical grids, conic wedges and moving sweep all use
the same accent family. The grid remains legible across the large empty desktop
and fights the window hierarchy. The palette reads as a game or security-tool
theme, not a general-purpose operating system.

Choose a new direction before doing pixel polish: near-neutral surfaces, one
controlled accent, much lower wallpaper contrast, and no permanent motion.
This is a product decision, not a renderer limitation.

### 5. Some polished-looking surfaces are still placeholders

System Monitor shows a hard-coded 27% memory value, a fixed sparkline and tabs
whose pages are not implemented. Empty or simulated content damages perceived
quality more than missing decoration. A smaller honest surface is better than
a fuller fake one.

## Execution order

### Pass A: remove the self-inflicted stutter

- default `zsweep` off;
- keep normal open/close/press motion enabled;
- rerun the identical QEMU interaction probe and `wmbench`;
- record frame distribution, not one screenshot value.

### Pass B: make latency observable

- retain the phase timers and queue/drop metrics now proven by ZLLOG;
- split compositor time into zl app draw, wallpaper, chrome and primitives;
- timestamp raw device completion and associate it with first presentation;
- script pointer drag, typing, Terminal scroll, app open and Files scroll;
- run in native host tests, KVM/QEMU and on the ThinkPad;
- fix the largest measured phase first.

### Pass C: repair scale and hierarchy

- convert layout geometry to q8 helpers;
- set minimum readable text/control sizes;
- stop opening an unusable Files window;
- reduce wallpaper contrast and remove permanent ambient motion;
- capture 1280x800, 1920x1200 and 2560x1440 screenshots as gates.

### Pass D: establish the real visual north star

Do not spend another long pass cloning the current lime prototype unless that
is still the desired product. Produce one representative desktop state with
the intended typography, palette, density, window hierarchy and controls, then
make the native desktop match it. Application completeness stays a separate
track.

## Definition of fixed

- ordinary interaction has zero late and zero lost frames on the ThinkPad;
- p95 input-to-present latency is measured and stays within one refresh;
- no decorative effect causes a recurring large repaint at idle;
- 1920x1200 and 2560x1440 use readable, proportionate geometry;
- the default desktop contains no obscured empty application;
- every visible monitor value is live or explicitly labelled unavailable;
- screenshots at all three gate modes show the same deliberate hierarchy.
