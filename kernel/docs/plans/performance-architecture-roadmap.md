# zlOS performance architecture roadmap

**Updated 2026-08-22.** This is the decision document for making the physical
desktop feel fast. It reconciles the current ThinkPad journal, the active
source tree, earlier plans, the local OS-reference collection, and primary
Windows, Apple and Linux documentation.

The locally executable implementation batch has now landed. The exact source
changes, final BIOS/UEFI/network/SMP evidence and the physical-only acceptance
gates are recorded in
[`performance-architecture-implementation-2026-08-22.md`](../evidence/performance-architecture-implementation-2026-08-22.md).
This page remains the architecture and test contract; it must not be read as a
claim that the ThinkPad gates have run.

Read the measured evidence first:

- [`current-speed-and-quality-diagnosis.md`](../current-speed-and-quality-diagnosis.md)
  is the current physical measurement record;
- [`retained-window-surfaces.md`](retained-window-surfaces.md) is the first
  implementation contract;
- [`always-on-telemetry.md`](../architecture/system/always-on-telemetry.md) is the recorder contract;
- [`kernel/HANDOFF.md`](../../HANDOFF.md) is the current hardware handoff.

## The answer in one page

The first physical bottleneck was the final copy to the ThinkPad's scanout
framebuffer. It was effectively uncacheable, measured about 111.59 MB/s, and
needed roughly 131--137 ms for a full 2560x1440 frame. That is fixed in the
current tree: the existing mapping is safely retyped to write-combining. The
physical rerun measured about 7,089 MB/s and a 2.070 ms median for seven
full-screen presents.

The remaining slow path is not USB, RAM capacity, privilege rings, or simply
"the zl language." Large damage currently makes `wm_repaint()` execute the
desktop restore, C window shell/chrome, and zl app drawing for each intersected
window. The retained ThinkPad phase trace contains 5,967 valid records and
1,043 paint samples with no recorder drops. Its biased slow-frame sample names
real owners:

| work in a large damaged frame | physical evidence | conclusion |
|---|---:|---|
| C window chrome | p95 117.285 ms; max 132.819 ms | must be cached, not rebuilt for an unrelated move |
| zl app drawing | p95 23.501 ms; max 37.705 ms | client content needs a retained surface |
| desktop restoration | p95 9.169 ms; max 143.744 ms | split and cache/optimise the desktop restore separately |
| present after WC | about 3 ms in the phase trace | no longer the dominant owner |

At `kernel/src/graphics/windowing/wm.c:1735`, damage is walked against visible windows; at
`kernel/src/graphics/windowing/wm.c:1810` and `:1820`, it invokes `chrome()` and `hook_draw()`.
That explains the trace directly. The next architecture is therefore:

```text
app state change     -> redraw that app's retained client surface
shell/focus change   -> redraw that window's retained shell surface
move/raise/exposure  -> compose existing surfaces by visible damaged regions
present              -> write only changed rows to WC scanout near vblank
```

It is the same broad shape as Windows DirectComposition and Apple Core
Animation: retain layer contents; change position/opacity/stacking without
asking the app to repaint. GPU composition is valuable later, but it is not the
first fix because a GPU still receives a needlessly rebuilt scene when the
retention contract is missing.

## What "fast" means on a real panel

One millisecond is a useful CPU-response goal, not a possible end-to-end
60 Hz photon goal. A 60 Hz display has a 16.667 ms refresh period; a completed
frame can still wait until the next refresh and the panel then scans it out.
zlOS should target:

| edge | target | why |
|---|---:|---|
| HID completion -> queue / route | p95 below 1 ms | makes input processing immediate |
| simple visual response CPU work | normally below 2 ms | leaves a vblank deadline margin |
| ordinary composed frame | below the next predicted refresh deadline | avoids adding one whole refresh of latency |
| queued visual frames | at most one, always newest | prevents a pointer from following old work |
| full desktop redraw | measured separately, never confused with pointer feel | makes a deliberate heavy operation diagnosable |

The required causal trace is:

```text
HID completion -> queue entry -> route -> app mutation -> WM commit
               -> compose start/end -> WC write complete -> chosen vblank
```

Current records reach CPU present, not photon output. That is still useful, but
the distinction must stay visible in every report.

## Decisions locked now

### 1. Retain both client content and stable shell/chrome

The existing client-surface design is the safest first seam, but it is not the
complete optimisation: the latest physical trace names C chrome as the largest
repeated cost. The finished design has independent validity for:

- client content: invalidated by app state, scroll exposure, resize or scale;
- shell/chrome: invalidated by geometry, focus, hover, title, tabs, controls or
  theme; never by another window moving;
- shadow: invalidated by geometry, scale or shadow style;
- desktop/dock: invalidated only when their own state changes.

Each window starts with a heap-backed RGB32 client surface and a separate shell
surface. Surface allocation is overflow-checked and bounded. The initial client
budget remains 48 MiB of the existing 64 MiB heap, leaving 16 MiB for other
kernel allocations. A 2560x1440 RGB32 surface is about 14.1 MiB, so this is a
real policy rather than a claim that memory is infinite.

If allocation fails, use today's direct rendering path for that window. Never
present a blank or stale surface. On resize/close/minimise, stop compositor use
before freeing the old allocation; a later multi-core compositor needs an
explicit retire/fence rule rather than a free while another CPU reads it.

### 2. Use a real region representation before elaborate GPU work

Damage is a correctness contract first and a speed hint second. The current
rectangle merger in `kernel/src/graphics/framebuffer/fb.c:472-510` treats touching rectangles as one
bounding box. That is not area-preserving: two thin L-shaped/touching regions
can turn into a large rectangle containing pixels neither region requested.
This is a plausible amplifier, not yet a measured explanation for the physical
trace.

Use a bounded disjoint-region list:

1. always merge true overlap when it reduces region count;
2. merge merely touching rectangles only when the bounding-box area passes a
   measured waste threshold;
3. retain separate opaque and transparent coverage where occlusion needs it;
4. fall back to full damage only after a bounded fragmentation limit, and count
   the reason.

Tests must cover an L shape, a touching chain, partially obscured windows and
the full-list fallback. Linux DRM's damage clips are a useful warning: clips
are optimisation hints but must include every changed pixel; under-reporting is
corruption, not a performance win.

### 3. Keep framebuffer and retained surfaces in the correct cache type

Normal retained surfaces, object state, font caches and file cache belong in
ordinary write-back RAM. The linear scanout mapping is write-combining for
sequential writes and should not be read back in normal rendering. This is why
the WC correction changed the real result by roughly 63.5x without adding RAM.

More RAM capacity helps only when it keeps useful retained surfaces, a page
cache or a browser document resident. It does not make an uncacheable MMIO
mapping, redundant chrome rebuild, or missed vblank faster. A larger/faster USB
only extends journal history or shortens delayed recorder checkpoints; it is
not part of the frame path.

### 4. Preserve a software renderer as oracle and fallback

The CPU renderer remains the correctness path. GPU work should submit the same
scene as software, compare a bounded reference hash in development, record
fences/reset state, and fall back safely. The Gen9 blitter is not a speed
shortcut: the measured CPU path already wins for plain copy/present. The render
engine becomes worthwhile for blend-heavy shadows, gradients and transforms
only after surface retention, damage and lifecycle rules are stable.

### 5. Treat the language/backend as a measured second-order owner

The UEFI build goes through `kernel/buildefi.sh`, which invokes zl compilation
to generated C and then Clang. Dynamic/boxed zl callbacks and runtime builtin
dispatch can cost real CPU time, particularly in app drawing and driver glue.
They do **not** explain the measured 130 ms C-chrome component, and moving the
whole kernel to a new backend cannot substitute for retained surfaces.

After redundant repaint is removed, profile `hook_draw`, callback boxing and
top builtin dispatches. Then either direct-bind the hot primitives or extend
the native `compilel` path in a freestanding relocatable-kernel mode. Native
integer lowering, direct bitwise operations, packed hardware layouts and
ELF64-relocatable output matter for drivers, processes and the long-term zl
thesis; they should not be sold as a magic current-frame-rate switch.

## Execution order and proof gates

Software status on 2026-08-22: orders 1--7 are implemented and host/QEMU
gated; order 8 has cooperative and PIT-preemptive two-process CR3/kernel-stack
switching, bounded IPC, and an owner-bound text/window/input ABI plus the
hostile/file ABI and a bounded persistent kernel-owned spawn/observe/reap desktop
service, but not a userspace process-management ABI or pixel/configure window depth;
order 9 has generic NIC selection, DHCP, virtio, e1000/e1000e and xHCI
CDC-ECM, browser persistence, triple-duplicate-ACK fast retransmit, and guarded
HTTP/1.0 keep-alive reuse, but not physical CDC-ECM/tether or I219 carrier;
order 10 has opt-in
blit/copy hash agreement and fallback
but not a physical Gen9 render-engine timing win. Order 0 and every physical
acceptance column remain open.

| order | implementation | success evidence | do not do first |
|---:|---|---|---|
| 0 | Script a physical stacked-window drag/raise/scroll probe; retain the raw journal and decoded report. | Same interaction can be compared before/after; no recorder drops. | Visual judgement without a trace. |
| 1 | Client retained surfaces, explicit `wm_invalidate_client`, direct fallback. | Moving an unchanged covered window causes zero app draws; output remains byte-correct. | A GPU rewrite. |
| 2 | Retained shell/chrome and shadow surfaces with separate invalidation. | Focus/move does not rerun client content; drag removes the 117 ms chrome owner. | Calling client-only caching “finished.” |
| 3 | Split desktop restore telemetry; cache/pre-expand wallpaper if measurements justify its RAM. | Desktop p95/max fall without stale pixels; budget is reported. | Blind cache allocation. |
| 4 | Area-aware region/occlusion handling and one atomic WM commit. | Region pixels/rect count, visible-window visits and compose bytes fall; L-shape tests pass. | Bounding every nearby rect together. |
| 5 | Input-driven wake, drain newest input immediately before commit, one-shot deadline pacing, one pending visual frame. | HID->route and input->CPU-present p50/p95/p99 improve; no deep visual queue. | A blanket realtime policy. |
| 6 | Async block/page-cache/writeback worker. | Save/journal I/O never executes in input/frame/IRQ context; bounded queue/flush telemetry. | Synchronous multi-block UI writes. |
| 7 | Surface/tile pools, then measured SMP composition jobs. | No frame-time allocations; APs wake for bounded work and do not spin; serial fallback matches. | Parallel zl app drawing with shared mutable state. |
| 8 | 64-bit process isolation and a stable file/window/input ABI. | A hostile user process faults alone; desktop remains alive. | Rings 1/2 or a cosmetic process demo. |
| 9 | Physical wired network, then browser reliability/storage. | Real link, DHCP/DNS/TCP traces and bounded browser load pass. | AX201 Wi-Fi as the first real network driver. |
| 10 | Intel render composition behind the software oracle. | Scene agreement, fence/hang recovery, physical timing benefit. | Making GPU output mandatory before recovery works. |

## Storage, drivers and the broader system

### Storage and files

The current durable `zlfs` path and ZLLOG journal are valuable, but foreground
disk operations must not stall the desktop. Build one block/page-cache seam:

```text
filesystem / journal -> bounded request queue -> NVMe or USB block provider
                         |                         |
                         -> completion + writeback <- IRQ/poll service
```

Reads should hit a bounded page/block cache when possible. Normal writes become
dirty cached pages and are flushed by a worker; explicit Save, metadata commit,
safe removal and crash markers retain synchronous durability semantics. Record
cache hit/miss, dirty bytes, request queue depth, completion latency, forced
sync count and writeback duration.

### Input, USB and drivers

HID/USB input must keep its interrupt path small: acknowledge, timestamp,
append a bounded event, and defer parsing/painting. MollenOS is a useful later
reference for a generic HID report-descriptor parser, but current pointer
latency work should first fix frame scheduling and retained composition.

The real-network order remains USB CDC-ECM/Ethernet or an external supported
adapter, then the ThinkPad I219 path, DHCP, DNS and TCP reliability. AX201 Wi-Fi
is a separate firmware/transport project and not a shortcut.

### Processes and scheduling

The existing scheduler and 32-bit Ring 3 proof are foundations, not current
desktop isolation. The actual 64-bit UEFI system needs per-process PML4/CR3,
user/supervisor mappings, guarded stacks, validated copy-to/from-user, a
64-bit syscall path, process lifetime and kill-on-fault. That unlocks trusted
network/file services and third-party apps. It does not by itself repair a
slow repaint.

Foreground/display work should receive a bounded latency class and background
work should be throttled. The useful mainstream lesson is not “make everything
realtime”; it is admission, runtime budgets, clear wake sources and queue-depth
telemetry.

## Telemetry: full visibility without making the OS slow

"Log everything" cannot mean storing every instruction, pixel write and packet
payload on a USB stick: doing so changes the timing, exhausts storage and
collects private content. The useful whole-system recorder is a five-tier
system, documented in [`always-on-telemetry.md`](../architecture/system/always-on-telemetry.md):

| tier | keep | examples |
|---|---|---|
| exact | every important boundary | faults, panics, driver state, command submit/complete/timeout, syscalls, filesystem mutation, drops |
| counters | cheap high-rate totals/maxima | interrupts, packets, allocations, scheduler activity, MMIO polls, dirty/cache bytes |
| sampling | representative healthy operation | ordinary pointer movement, healthy frames, periodic queue depth |
| triggered burst | local raw state around an anomaly | pre/post timeout registers, TD/event/context, late-frame phase history |
| laboratory | deliberately narrow investigation | bounded QEMU trace; later Intel PT with a trigger and offline decoder |

Rules that protect the result:

- no USB/filesystem I/O from IRQ, input or paint context;
- every drop is counted and high-priority faults reserve capacity;
- journal records contain IDs, sizes, states, timings and hashes, not typed
  content, passwords, web bodies, file contents or framebuffer captures;
- the flusher has a bounded chunk/time budget and backoff, so a bad stick cannot
  freeze an otherwise healthy desktop;
- a fatal event is exact in RAM, but a broken kernel cannot promise a successful
  final USB write. A reserved crash page and next-boot importer are the next
  durable-fault improvement.

Add correlation IDs through HID, input routing, window commit, compositor and
present. For a slow frame retain its damage list/pixel area, visible windows,
surface cache hit/miss, draw categories, compose bytes, vblank decision and
outcome. That identifies whether a stutter came from input backlog, app code,
chrome, wallpaper, present, storage or an IRQ gap.

## What the references say to borrow

| reference | take | avoid |
|---|---|---|
| Windows DirectComposition | retained visual tree, atomic commit, occlusion avoidance and partial surface update | treating a proprietary API as an implementation plan |
| Apple Core Animation | layer backing stores: move/transform cached content instead of rerendering it | assuming GPU acceleration replaces cache lifetime rules |
| Linux DRM/KMS | damage correctness, atomic state and vblank/deadline thinking | assuming broad/full damage is always incorrect; it is a permitted fallback |
| Brook | persistent per-window framebuffer ownership, compositor-safe surface retirement, bounded asynchronous profiling | importing a much larger Linux-compatible system wholesale |
| SerenityOS | disjoint regions, separate opaque/transparent flush and recomputed occlusion | premature complexity before the basic retained surface gates work |
| RustOS | scene commands: moving a window changes destination state, not raster content | relying on unproven performance claims in its incomplete paths |
| Astral | page cache/writeback, slab allocation and a separate DHCP service | copying its large C kernel rather than its boundaries |
| MollenOS | generic HID descriptor/parser separation | treating its older tree as a current speed benchmark |

The local collection also demonstrates a trap: duckOS and LemonOS merge
colliding rectangles into a bounding box. That is exactly the region-inflation
class zlOS must test rather than inherit.

Primary mainstream references:

- [Windows DirectComposition architecture](https://learn.microsoft.com/en-us/windows/win32/directcomp/architecture-and-components)
- [Windows composition surfaces](https://learn.microsoft.com/en-us/windows/win32/directcomp/composition-surface)
- [Apple Core Animation basics](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/CoreAnimation_guide/CoreAnimationBasics/CoreAnimationBasics.html)
- [Apple animation performance guidance](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/CoreAnimation_guide/ImprovingAnimationPerformance/ImprovingAnimationPerformance.html)
- [Linux DRM damage tracking](https://docs.kernel.org/gpu/drm-kms.html)
- [Linux NAPI](https://docs.kernel.org/networking/napi.html)
- [Linux ftrace latency tracing](https://docs.kernel.org/6.15/trace/ftrace.html)

## Documentation status

The newest physical journal and this document override performance priorities
in older historical plans. Do not delete those plans: they retain why earlier
work happened. In particular:

- `current-speed-and-quality-diagnosis.md` is the source of truth for measured
  display performance;
- `retained-window-surfaces.md` is the immediate implementation contract, now
  understood as client **and** shell retention in sequence;
- `../research/why-mainstream-desktops-feel-fast.md` remains a useful conceptual document,
  but its older framebuffer hypothesis is closed by the physical WC rerun;
- `STATE-OF-THE-PROJECT.md` and older numbered plans are audit/history inputs,
  not the current performance queue;
- `EXECUTION-ROADMAP.md` remains the system-wide dependency spine; this page
  refines its speed phase with the newest physical evidence.

## Definition of success

Do not call the desktop fast because one benchmark or one screenshot looks
good. Call this stage complete only when the same physical interaction probe
proves all of the following:

1. unchanged covered windows receive zero app and shell redraws during another
   window's move/raise;
2. no stale/incorrect pixels across move, resize, focus, occlusion, minimise,
   allocation refusal and surface eviction;
3. the trace attributes a large reduction in chrome/app work and materially
   improves input-to-CPU-present p95 without recorder drops;
4. all frame pacing, region, allocator and UEFI/QEMU gates remain green;
5. software direct rendering remains selectable and byte-correct;
6. the user sees a responsive pointer/drag on the physical ThinkPad, not merely
   a favourable average.

The weakest link is now physical proof, not missing retained-compositor source.
All locally executable gates above are green, but the same retained-surface,
input, pacing and fallback source still needs the physical before/after journal
to earn the claim that the target desktop is fast.
