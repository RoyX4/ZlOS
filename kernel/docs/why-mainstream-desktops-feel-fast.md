# Why mainstream desktops feel fast, where RAM matters, and where zlOS differs

**Measured 2026-08-20 against `main` at `b8a00ec`.** This document compares
mechanisms, not brands. An operating system has no single "speed": input
latency, frame time, scheduling, memory access, file access and application
startup are different paths with different limits.

## The useful mental model

RAM capacity is the size of a workshop. RAM bandwidth is how quickly material
can move through it. CPU caches are the tools already in your hands. The
compositor is the assembly line, and the display refresh is the truck that
leaves on a fixed schedule.

A larger workshop helps only if the system knows how to fill it with useful
work-in-progress. It does not make the assembly line move faster, and missing
the truck by one millisecond still means waiting for the next truck.

Mainstream systems feel fast because they coordinate all five:

```text
input interrupt -> scheduled app -> retained window surface -> GPU composition
                -> frame selected for a known vblank -> scanout
```

zlOS currently does this:

```text
input poll/queue -> one desktop loop -> re-run zl app drawing through boxed calls
                 -> CPU repaint into one RAM back buffer -> copy damage to scanout
```

The second design can be fast, but it has less overlap, less isolation and more
CPU work on every changed region.

## What Windows, Linux desktops and macOS use

### Retained window surfaces

Modern compositors keep the latest image for each window. Moving or uncovering
a window usually means recomposing an existing surface rather than asking the
application to redraw its entire contents. Microsoft explicitly documents that
obscured DWM applications do not receive repaint requests because their content
is already available to the compositor.

zlOS tracks damage, which is necessary, but when damage intersects a window it
calls that application's drawing function again. Damage limits *where* pixels
are written; it does not automatically avoid recomputing the window.

### GPU composition and asynchronous pipelines

Windows DWM/DirectComposition, Linux DRM/KMS compositors and macOS Core
Animation all hand composition work to graphics hardware. The CPU prepares
state while the GPU renders, and the display scans a different completed
buffer. The work is pipelined instead of one synchronous CPU chain.

Windows can bypass ordinary DWM composition with direct scanout or Independent
Flip; Microsoft's current documentation states that Independent Flip can reach
one frame of presentation latency. Linux DRM exposes atomic plane updates,
DMA-buffer sharing, fences and page flips targeted at a vertical blank. Apple
describes Core Animation as a retained layer system that hands most per-frame
work to dedicated graphics hardware.

zlOS has Intel Gen9 ring, blit and modesetting work, but the normal compositor
still paints on the CPU and copies to a linear framebuffer. The GPU path is a
proof and an opt-in diagnostic, not the load-bearing desktop backend.

### Native hot paths

Mainstream UI and driver hot paths are compiled machine code with direct calls.
The shipping zlOS kernel is not using zl's active LLVM speed backend. It still
builds `kernel.zl` through the archived boxed-C backend.

Current-tree facts:

- one zl `Value` is now 16 bytes on both the 32-bit kernel and 64-bit host;
- generated `_gen.c` contains thousands of `zl_calln` sites and temporary
  `Value` declarations (3,855 literal-name call sites in this snapshot);
- the freestanding runtime contains 644 name comparisons in its builtin
  dispatcher;
- common pixel calls sit late in that chain: `fill_rgb` at position 509,
  `rrect` at 515 and `text_aa` at 516.

The compositor's C benchmark bypasses most of that policy-layer overhead. This
is a likely owner of part of the measured 3.454 ms native compositor versus
13 ms live KVM/QEMU gap, but it needs per-phase timing before an exact share is
claimed.

Historical project benchmarks show the size of this class of cost:

| same zl workload | measured time |
|---|---:|
| boxed-C backend used by the kernel, `fib(28)` | 65 ms |
| unboxed C backend | 2 ms |
| LLVM speed backend | 3 ms |

For a 10-million-iteration bitwise loop, the typed LLVM path falling back to
boxed name dispatch measured 999 ms versus 7 ms for native C. That is about
140 times slower on the exact operation shape common in drivers. Both tables
predate Value16 and are evidence of the backend's shape, not current timings;
they must be rerun before being used as a present-day speed claim.

A fresh post-Value16 measurement on 2026-08-20 used `bench/b2_arith.zl`, which
performs seven arithmetic/comparison operations per iteration for three million
iterations. Best of five:

| current host path | time | relative |
|---|---:|---:|
| boxed-C backend used by `kernel.zl` | 113 ms | 1.0x |
| LLVM `compilel` backend | 13 ms | 8.7x faster |

Both produced `8999988000010`. This measures an arithmetic-heavy language
kernel on Linux, not a zlOS desktop frame. It proves the backend can own a
large share of ZL-heavy work; it does not say what share of a frame is ZL.

This is not evidence that the zl language is inherently slow. It is evidence
that the **backend currently used to build the kernel is slow**. `kernel.zl` is
ahead-of-time compiled to C; it is not interpreted at boot. The generated C,
however, still implements dynamic-language machinery for ordinary operations.

For example, this short zl code:

```zl
fn ui() { return ui_scale() }
draw_window(sx, sy, MON_W * u, MON_H * u, "System Monitor", sfoc)
```

does not become a direct machine-code call plus two integer multiplies. The
generated C has the following shapes:

```c
return zl_calln("ui_scale", 0);
zl_binop("*", v_MON_W, v_u);
zl_fn_draw_window(temporary_Value_1, ..., temporary_Value_6);
```

Each number is wrapped with a type tag. Each `*`, `<`, `==` or `+` calls a
generic operator helper which checks the tags. User-defined zl functions such
as `draw_window` do get direct C calls, but their parameters and results remain
boxed `Value` objects. Builtins such as `ui_scale`, `glow`, `wm_open` and pixel
primitives package arguments, search the name-dispatch chain, then unwrap the
values again in C. The eight-argument wallpaper `glow(...)` constructs eight
temporaries before entering the pixel routine. Value16 made those boxes much
smaller; it did not turn the operations into native integer instructions or
the builtin names into direct calls.

That can absolutely make zl application policy and driver loops slow. It cannot
be the *only* explanation for the desktop, because the pacing error, redundant
redraw, CPU composition and likely uncacheable final framebuffer exist even if
all of `kernel.zl` were rewritten in C. Phase timing must separate these owners.

The useful bound is therefore algebra, not a guessed percentage. Let `Tzl` be
the time the new recorder measures inside ZL app/tick/draw work. If that exact
work received the arithmetic benchmark's 8.7x improvement, the possible saving
would be `Tzl * (1 - 1/8.7)`, about 88.5% of **that phase only**. If ZL owns 1 ms,
that saves at most about 0.9 ms; if it owns 8 ms, about 7.1 ms. Those are worked
examples, not claims about the ThinkPad. `fb_present`, vblank wait and pacing do
not become faster merely because ZL arithmetic does.

## Three different meanings of "changing the bits"

These are independent projects and must not be mixed together.

### 1. Running the CPU and kernel as 32-bit or 64-bit

The 32-bit build uses four-byte pointers and can directly name at most a 4 GiB
virtual address space. The x86-64 build uses eight-byte pointers, four-level
page tables, more general-purpose registers and native 64-bit address
arithmetic. It can safely represent device BARs and DMA addresses above 4 GiB
in C. x86-64 also makes SSE2 baseline, which is useful for pixel loops.

64-bit mode is not an automatic speed switch. Eight-byte pointers make some
structures larger and consume more cache/bandwidth. Code becomes faster only
where more registers, the calling convention, SIMD or native 64-bit arithmetic
remove actual work. It primarily buys zlOS correctness, address space and the
foundation for isolated processes.

Current zlOS has three relevant paths:

| path | current state |
|---|---|
| 32-bit Multiboot kernel | boots; paging off; contains the Ring-3 proof |
| 64-bit Multiboot kernel | builds as ELF64 and shares the drivers |
| 64-bit UEFI/PE32+ path | the ThinkPad's actual boot path |

The laptop is therefore already running the 64-bit kernel path. "Moving 32 to
64" now mainly means porting the **user-mode/process proof**: a 64-bit syscall
entry, per-process PML4/CR3, user/supervisor page permissions, guarded stacks,
pointer validation and an ELF64/ZL program loader. That work prevents one app
from corrupting the kernel; it does not fix the current frame pacing by itself.

### 2. The width and exactness of a ZL number

The boxed `Value` is 16 bytes, but its numeric payload is still a `double`.
Those are different widths: a 16-byte container holding a 64-bit floating-point
payload. A double represents every integer only through 2^53. A 32-bit register
value survives exactly; an arbitrary 64-bit BAR, pointer or DMA descriptor does
not.

The fix is not to inflate every `Value` back to 64 bytes. The compiler/runtime
needs an exact integer representation: ideally statically inferred native `i64`
in `compilel`, plus an exact boxed integer variant only where dynamic values
cross a runtime boundary. At hardware seams we keep addresses as `u64`/pointer-
sized C values, or explicitly use two 32-bit halves until ZL can carry them.

### 3. Bitwise driver operations

`band`, `bor`, `bxor`, `bnot`, `shl` and `shr` manipulate individual bits in
device registers. For example, `band(status, 4)` asks whether bit 2 is set;
`bor(control, 1)` enables bit 0. Drivers perform these operations constantly.

Today the kernel's boxed backend turns each operation into a function call,
converts doubles to integers, searches for the builtin name late in a 644-entry
chain, performs the CPU operation, then converts the result back to a double.
`compilel` also falls through its boxed bridge for these builtins instead of
emitting LLVM `and`, `or`, `xor`, `shl` and `lshr` instructions directly.

"Fixing the bits" therefore means:

1. emit direct native `i64` bitwise instructions in `compilel`;
2. preserve exact `i64` types through functions instead of degrading to double;
3. add explicit address/integer conversions with overflow checks;
4. make `compilel` support the complete freestanding kernel subset;
5. move `kernel.zl` from archived `compile` to that backend only after every
   32-bit, 64-bit and UEFI gate agrees.

It does **not** mean converting every variable to 64-bit. Pixel colours and most
MMIO registers are correctly `u32`; widening those wastes cache and can hide
hardware mistakes.

## What BoredOS contributes to this comparison

The referenced project is [BoredOS](https://boredos.dev/), not a measured
Windows/Linux replacement. Its public architecture is useful because it shows
another from-scratch desktop crossing boundaries zlOS has only partly crossed:

| BoredOS architecture | lesson for zlOS |
|---|---|
| x86-64 C and assembly | native exact addresses and direct hot-path code |
| physical/virtual memory plus slab allocation | use RAM dynamically instead of fixed regions |
| preemptive scheduler and ELF64 processes | isolate slow apps from input/composition |
| shared memory, Unix sockets and wait/work queues | pass surfaces/events without redrawing inside the kernel |
| Nova user-space Wayland-style compositor | retain client surfaces and compose them separately |
| lwIP with several NIC drivers | a mature network core plus replaceable hardware links |

BoredOS's own project history says its previous kernel UI was unstable and slow
and was replaced by Nova. That supports the architectural direction, but the
project publishes no comparable p95 frame-time or input-to-photon benchmark.
Claiming that it is numerically faster than zlOS would therefore be fiction.

### Scheduling and isolation

Mature systems separate the input stack, applications, compositor and GPU
driver. A blocked file read or slow application does not have to stall pointer
feedback or every other window. They use priorities, preemption, asynchronous
I/O and deadlines to keep interactive work ahead of background work.

zlOS has scheduler primitives, APIC interrupts and an event queue, but the
desktop and its app callbacks still share one compositor loop. There are no
production user processes with separate address spaces. A slow `app_tick` or
`app_draw` consumes the same frame budget as input routing and presentation.

### Caching and deliberate prefetch

Windows, Linux and macOS use spare RAM for file/page caches, executable pages,
font and glyph caches, decoded images, application working sets and compositor
surfaces. Frequently reused data returns from RAM instead of storage, and cold
data can be reclaimed when pressure rises.

zlOS has isolated caches and arenas, including a cached wallpaper, browser
image memory and a Terminal scrollback ring. It does not have a general page
cache, demand-paged application working sets, swap/compression, a unified
buffer cache or retained per-window surfaces.

## What the speed numbers mean

### Display deadlines

These are hardware deadlines shared by every OS:

| refresh | complete-frame budget | one queued frame |
|---:|---:|---:|
| 60 Hz | 16.67 ms | 16.67 ms |
| 90 Hz | 11.11 ms | 11.11 ms |
| 120 Hz | 8.33 ms | 8.33 ms |
| 144 Hz | 6.94 ms | 6.94 ms |

**16.67 ms is the departure interval, not a good CPU-work target.** The panel
can only begin a new 60 Hz scanout every 16.67 ms, but input routing and a small
UI update should normally take microseconds to comfortably below 1 ms. A larger
repaint may take a few milliseconds. Using all 16.67 ms leaves no safety margin
and almost guarantees an occasional missed departure.

Sub-millisecond work also does not mean a 60 Hz panel can visibly replace the
whole screen every sub-millisecond. If input arrives just after scanout starts,
the next tear-free complete-frame opportunity is still the next vblank. A
hardware cursor can update independently, and tearing or variable refresh can
change the trade-off, but they do not make a fixed 60 Hz panel a 1,000 Hz panel.

At 60 Hz, a compositor finishing in 17 ms does not look 0.33 ms slower than one
finishing in 16.67 ms. It misses the departure and can display a frame roughly
16.67 ms later. Consistency matters as much as the average.

Microsoft's "one frame" Independent Flip figure therefore means roughly
16.67 ms of presentation latency at 60 Hz or 8.33 ms at 120 Hz. It is not a
claim that total mouse-to-photon latency is only that number; input sampling,
application work, scanout position and display electronics are additional.

### What good frame pacing does on Windows and Linux

The mature design does not start a frame immediately after the previous one and
hope it finishes. It predicts the next display departure and works backwards:

```text
next vblank known
      -> wake shortly before it
      -> sample the newest input
      -> render into a non-visible buffer
      -> queue exactly that completed buffer
      -> hardware flips it at vblank
```

On Windows, a DXGI frame-latency waitable object wakes an application when the
previous queued frame has been presented. The default waitable-swap-chain
latency is one frame, specifically to keep the queue shallow and the input data
fresh; two frames can be chosen when CPU/GPU overlap matters more than latency.
Flip-model presentation and Independent Flip let a completed surface be scanned
out without an extra full copy in the favorable case.

On Wayland, `wl_surface.frame` tells a client when it is useful to produce the
next surface instead of drawing continuously. The presentation-time protocol
reports when that surface was actually shown. Underneath, DRM/KMS page flips and
atomic commits arrange for a completed buffer to become scanout at a chosen
vblank. Different Linux compositors make different scheduling choices, but the
important mechanism is the same: retain buffers, avoid an uncontrolled queue,
and measure actual presentation rather than only the end of CPU drawing.

### What zlOS currently does instead

The current TSC gate is more precise than using the PIT as a stopwatch, but it
is not a precise wakeup system:

1. `wm_frame()` checks its 16.667 ms TSC deadline **before** `input_poll()`. If
   called early, it returns without routing newly arrived input.
2. Its caller then executes `sti; hlt`. The ordinary periodic wake source is
   the 100 Hz PIT, one interrupt every 10 ms. There is no one-shot APIC/HPET
   wake scheduled for the exact frame deadline.
3. A 60 Hz deadline sampled by a 100 Hz wake can therefore be released in an
   uneven 10 ms, 20 ms, 20 ms pattern. The TSC knows that the time has arrived;
   it cannot wake the sleeping CPU by itself.
4. On the Intel path, zlOS draws first, spins in `intel_wait_vblank()`, then
   calls `fb_present()` to copy into the active GOP framebuffer. It waits for
   the boundary and copies *after* it instead of flipping a previously completed
   buffer *at* the boundary. `intel_flip()` exists, but the normal compositor
   does not use it.

This means a frame can report under 16.67 ms of measured drawing and still feel
bad. Input may already have waited for the pacing gate; the release itself can
jitter; then a 13 ms draw can miss the next scanout and wait another refresh.
The existing `late` counter measures CPU work over budget and `lost` counts
whole skipped software deadlines. Neither records a three-millisecond late
wakeup, input-queue residence time or actual photon/presentation time.

The correction is: interrupt on input, use a one-shot high-resolution wake for
`predicted_vblank - estimated_render_time`, sample input late, keep normal
interaction work below 1 ms where practical, render into a completed scanout
buffer, flip at vblank, and record actual presentation feedback.

### Comparable Windows, Linux and zlOS latency facts

There is no honest single "Windows speed" or "Linux speed". Hardware, driver,
compositor, refresh rate and workload all change it. The comparable facts are:

| layer | Windows | Linux desktop | zlOS now |
|---|---|---|---|
| high-rate mouse input | Raw Input explicitly supports 1000 Hz devices: one hardware report each 1 ms | evdev forwards timestamped kernel events; device rates commonly range from 125 to 1000 Hz | USB/PS2 events enter a queue, but routing can wait behind the frame gate |
| precision wakeup | waitable presentation objects/fences; not tied to a 10 ms desktop poll | hrtimers are tick-independent and represented in 64-bit nanoseconds | calibrated TSC can read precise time, but ordinary release still depends on interrupts including the 10 ms PIT |
| best documented presentation queue | Independent Flip can reach one queued frame | compositor policy varies; page flip and presentation feedback expose the actual event | no true compositor page-flip path in normal use |
| 60 Hz physical departure | 16.67 ms | 16.67 ms | 16.67 ms |
| 120 Hz physical departure | 8.33 ms | 8.33 ms | 8.33 ms if the panel/mode supports it |

A 1000 Hz mouse does not guarantee one-millisecond pixels. It means a new input
sample can arrive every millisecond. Windows or Linux can then preserve its
timestamp, wake the correct work promptly and use the freshest sample for the
next flip. zlOS currently discards much of that advantage by checking the frame
deadline before it polls/routes the queue.

### Measured on this Linux development machine

These were measured in this session on the i7-10510U host, not copied from a
cross-machine benchmark:

| operation | result |
|---|---:|
| `getppid()` syscall, 10 million calls | 0.0966 us/call |
| two-process pipe handoff, 100,000 operations | 2.107 us/op |
| 256 MiB `memcpy`, glibc default | 6.68 GB/s |
| GNOME/Wayland display refresh | 59.94 Hz, 16.68 ms/frame |

These are not direct Windows/macOS comparisons. They show the scale separation:
basic kernel operations take fractions or low single microseconds, while a UI
frame has thousands of microseconds. Desktop smoothness is normally dominated
by rendering, presentation and queueing, not by one syscall.

### zlOS display work

Current measurements at 1920x1200:

| path | result |
|---|---:|
| native C compositor benchmark, dragged frame | 3.454 ms |
| live KVM/QEMU desktop, one captured painted frame | 13 ms |
| 60 Hz budget | 16.667 ms |

The KVM sample is not a percentile distribution. It proves that the full guest
path can get close to the budget; it does not yet assign the cost.

The raw byte volume explains why framebuffer policy matters:

| mode, 32 bpp | one full buffer | copying 60 full frames/s |
|---|---:|---:|
| 1280x800 | 3.91 MiB | 0.246 GB/s |
| 1920x1200 | 8.79 MiB | 0.553 GB/s |
| 2560x1440 | 14.06 MiB | 0.885 GB/s |
| 3840x2160 | 31.64 MiB | 1.991 GB/s |

Those rates fit inside normal RAM bandwidth. The problem is redundant work and
the final destination: on the ThinkPad, current evidence says the GOP front
buffer is mapped uncacheable rather than write-combining. More RAM capacity
does not fix an uncacheable framebuffer mapping.

### RAM bandwidth and cache mode, physically

The current pixel path has two different kinds of memory:

```text
CPU draws -> ordinary write-back DRAM back buffer
          -> fb_present copies changed rows
          -> firmware/GOP front buffer -> display engine scans it repeatedly
```

The table above counts only the final arrow. It excludes reading and blending
old pixels, drawing glyphs and shadows, recomputing the wallpaper, and any
extra passes. A 1920x1200 full frame is 9,216,000 bytes; sixty final copies are
0.553 GB/s. The ThinkPad's 2560x1440 panel raises that final-copy stream to
0.885 GB/s. 4K is 1.991 GB/s.

Those are easy rates for normal cached system DRAM. The destination's cache
type changes the operation:

- **WB, write-back:** normal RAM. Stores first land in fast CPU cache and are
  evicted in large cache lines. Reads, repeated writes and read-modify-write
  drawing are efficient.
- **WC, write-combining:** normally suitable for a framebuffer. The CPU groups
  adjacent stores into burst transactions before sending them to the device.
  Sequential writes are good; reads from the same mapping remain bad.
- **UC, uncacheable:** every pixel store behaves much more like a device access.
  Ordinary caching and combining are disabled, so millions of small stores can
  be serialized into expensive bus traffic. The 0.885 GB/s byte count may be
  modest while the transaction pattern is disastrous.

Current ThinkPad evidence places the GOP framebuffer at `0xC0000000`, inside an
MTRR range covering `0x80000000-0xFFFFFFFF` as UC. zlOS's page tables do not
currently select a PAT write-combining type, despite an old `fb.c` comment
calling the mapping WC. The effective type is therefore likely UC. That is a
hardware-specific hypothesis backed by the mapping evidence, not yet a measured
bare-metal throughput number.

Adding RAM cannot repair this. It gives more storage behind the CPU; it does not
change how writes reach the display aperture. The safe next proof is a dedicated
bare-metal timing split around `fb_present`, followed by a controlled UC-versus-
WC test. A PAT change must be made consistently on every logical CPU with the
required cache/TLB synchronization; blindly toggling a bit risks corruption.

### Why bare metal can be far worse than the VM

The 13 ms KVM/QEMU value is one captured painted frame, not proof that the
ThinkPad is fast. QEMU's emulated framebuffer is backed by host memory with a
very different cache and display path. The laptop uses the real GOP/Intel
aperture, the native 2560x1440 mode, real vblank waits and real firmware memory
attributes. The user's observation that bare metal is extremely slow is real
evidence, and it makes the final framebuffer mapping more suspicious.

The laptop does not simply need a larger RAM allocation. The QEMU gates supply
1 GiB, while the bare-metal build mostly uses fixed physical regions below that
ceiling. At 2560x1440, the 14.06 MiB back buffer fits its 40 MiB region and the
7.03 MiB RGB565 wallpaper cache fits its 16 MiB region. The 64 MiB heap is
initialized, but production code does not currently allocate desktop objects
from it. More installed RAM remains invisible and unused until zlOS has a real
physical-page allocator and consumers such as retained surfaces and a page
cache.

## Exactly where RAM enters zlOS today

Every QEMU gate now gives zlOS 1 GiB because `HI_TOP` promises addresses up to
that point. That is a minimum physical-address map, not a dynamic RAM budget.

The current main regions include:

| memory | purpose |
|---:|---|
| 16 MiB | one zl program arena, reset as a unit |
| 8.79 MiB at 1920x1200 | CPU back buffer |
| 4.39 MiB at 1920x1200 | RGB565 wallpaper cache |
| 16 MiB fixed region | cached blur arena |
| 64 MiB | general heap |

The important fact is that the production tree currently has no caller of
`heap_alloc` outside the allocator itself. The 64 MiB heap exists and is tested,
but the desktop, browser and drivers have not migrated to it. Most large users
still live in fixed-address regions.

Therefore:

- raising QEMU from 1 GiB to 2 or 4 GiB will not make the desktop faster;
- enlarging the 64 MiB heap will not help until real subsystems allocate from it;
- RAM *bandwidth and cache type* affect framebuffer copies today;
- RAM *capacity* becomes useful when zlOS adds retained window surfaces, a page
  cache, decoded-image/glyph caches and real process working sets.

A retained copy of the three boot-window client areas would cost only a few
megabytes. That is a better use of existing RAM than increasing the guest
ceiling: spend memory to stop executing drawing code repeatedly.

## Current zlOS bottlenecks, ranked

### Tier 0: measure and remove self-inflicted work

1. **No end-to-end latency trace.** Current timing begins inside `wm_frame` and
   cannot see how long input waited in the queue.
2. **Always-on wallpaper sweep.** It causes a roughly 34%-height full-width
   damage band around fourteen times per second.
3. **Boxed zl backend and string builtin dispatch.** Common draw calls may walk
   more than 500 names before entering the C primitive.

### Tier 1: presentation architecture

4. **No retained window surfaces.** Damage crossing a window re-executes its
   drawing code instead of normally recomposing cached content.
5. **Synchronous CPU composition/present.** There is no normal asynchronous GPU
   composition pipeline or direct page flip.
6. **Likely uncacheable real framebuffer mapping.** The ThinkPad path must
   measure UC versus WC before safely changing PAT on every CPU.
7. **One desktop loop.** App work, input and presentation share one deadline.

### Tier 2: expensive drawing and perceived quality

8. **Terminal glyph work.** It remains the largest measured single application
   drawing share in the native benchmark.
9. **Integer layout scale.** This mainly damages readability and perceived
   quality, but inconsistent scale also creates oversized or badly targeted
   damage at some modes.
10. **Placeholder and empty app state.** This does not consume many cycles, but
    it makes every delay feel worse because the result appears unfinished.

### Not first

- more guest RAM;
- more CPU cores without a measured parallel phase;
- moving the whole compositor to the Intel ring immediately;
- adding more applications before the shared hot paths are fixed.

## Correct execution order

1. Add timestamps for event enqueue, routing, app work, compositor work, vblank
   wait and present. Report p50/p95/p99 plus queue depth.
2. Default the wallpaper sweep off and repeat the same interaction trace.
3. Measure hot builtin dispatch directly. Prove one direct-call A/B for the
   common drawing primitives before choosing the migration mechanism.
4. Make `compilel` kernel-capable: native bitwise operations, exact 64-bit
   integers, memory/port intrinsics and function values; then move the kernel
   off the archived boxed backend.
5. Allocate retained app surfaces from the existing heap and invalidate them
   only when app state changes.
6. Measure UC versus WC framebuffer writes on the ThinkPad, then implement the
   mapping correctly on every logical CPU if the result justifies the risk.
7. Make proven Intel composition/page-flip pieces load-bearing behind the CPU
   oracle and fallback.

## Sources for the mainstream mechanisms

- Microsoft, [For best performance, use DXGI flip model](https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/for-best-performance--use-dxgi-flip-model)
- Microsoft, [Reduce latency with DXGI 1.3 swap chains](https://learn.microsoft.com/en-us/windows/uwp/gaming/reduce-latency-with-dxgi-1-3-swap-chains)
- Microsoft, [`GetFrameLatencyWaitableObject`](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_3/nf-dxgi1_3-idxgiswapchain2-getframelatencywaitableobject)
- Microsoft, [Raw Input overview](https://learn.microsoft.com/en-us/windows/win32/inputdev/about-raw-input)
- Microsoft, [Desktop Window Manager](https://learn.microsoft.com/en-us/windows/win32/learnwin32/the-desktop-window-manager)
- Microsoft, [Cache and Memory Manager performance](https://learn.microsoft.com/en-us/windows-server/administration/performance-tuning/subsystem/cache-memory-management/)
- Linux kernel, [DMA-buffer sharing and synchronization](https://docs.kernel.org/driver-api/dma-buf.html)
- Linux kernel, [DRM/KMS](https://docs.kernel.org/6.18/gpu/drm-kms.html)
- Linux kernel, [PAT memory types](https://docs.kernel.org/6.2/x86/pat.html)
- Linux kernel, [device I/O mappings](https://www.kernel.org/doc/html/latest/driver-api/device-io.html)
- Linux kernel, [Memory-management concepts](https://docs.kernel.org/5.19/admin-guide/mm/concepts.html)
- Linux kernel, [input event interface](https://kernel.org/doc/html/latest/input/input.html)
- Linux kernel, [high-resolution timers](https://docs.kernel.org/timers/hrtimers.html)
- Wayland, [`wl_surface.frame`](https://wayland.app/protocols/wayland)
- Wayland, [presentation time](https://wayland.app/protocols/presentation-time)
- BoredOS, [project and architecture](https://github.com/BoredOS/BoredOS)
- Apple, [Core Animation](https://developer.apple.com/documentation/quartzcore)
