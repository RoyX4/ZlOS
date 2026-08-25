# The Intel graphics stack, in depth — and where zlOS sits in it

Researched 2026-08-17. Everything zlOS's target GPU actually is, what the full
Linux stack does with it, which parts `intel.c` already implements, and what the
layers above would cost.

**The target hardware:** `8086:9B41` — Comet Lake-U **GT2**, **Gen9.5**
microarchitecture, in the ThinkPad X1 Carbon Gen 8. Panel: LG LP140QH2-SPD,
2560×1440 eDP.

Facts marked **[verified]** were measured on this machine by `hosttest/`.
Everything else is from Intel's PRMs, the kernel docs, or published research.

---

# Part 0a — "the Intel driver" is not one thing

The single most confusing point, so it goes first.

**The GPU is two separate halves of silicon:**

| Half | Job | zlOS |
|---|---|---|
| **Render engine** — 24 EUs, 168 threads | **makes** the picture | not used. The CPU draws instead. |
| **Display engine** — clock, timing, cable, panel power | **sends** the picture to the panel | **`intel.c`. This is the driver.** |

They are different hardware blocks. That is why Haiku can have working
modesetting and *no* acceleration — it built one half and not the other.

**So where the four things land:**

| Thing | Which half | Status |
|---|---|---|
| **Modesetting** | display engine | **This IS `intel.c`.** Needed for any picture at all. |
| **2D acceleration** | render side (blitter) | Would live in `intel.c` too, but it is a *new subsystem* — needs a memory manager, a ring buffer, fences. Not an extension of what exists. |
| **3D acceleration** | render engine, 24 EUs | Needs a shader compiler targeting Gen ISA. Closed — see Part 4. |
| **Software 3D** | **neither — pure CPU** | **Touches `intel.c` zero.** It is `fb3d.c`, and its output goes out through the display engine like everything else. |

**The unlock:** *software 3D and the Intel driver are unrelated projects.* You
could write a full software rasterizer today with no Intel driver at all — it
would render into the framebuffer firmware already handed you. The Intel driver
only changes **what resolution and refresh rate** that framebuffer runs at.

They can be worked on independently, in either order.

# Part 0 — what the driver actually does, in plain words

Before any of the detail below: a display driver is **setting up a projector in
a room.** Ten jobs, in this order. Get one wrong and the screen is black with no
error message.

| # | Job | In the code |
|---|---|---|
| 1 | **Find the chip** — walk the PCI bus | `pci.c` ✅ |
| 2 | **Switch the power on** — parts of the chip are off until asked | power wells ✅ |
| 3 | **Make a clock** — decides how fast pixels come out | DPLL ✅ **proven, locks** |
| 4 | **Ask the screen what it is** — a side-channel to the panel | AUX / DPCD ✅ |
| 5 | **Tune the cable** — turn the signal up until the panel can read it | link training ⚠️ **never run for real** |
| 6 | **Set the picture shape** — width, height, where each line ends | transcoder ✅ verified vs firmware |
| 7 | **Point at the pixels** — "image is here, this wide, this format" | plane ✅ |
| 8 | **Set the fetch timing** — pull pixels from RAM early enough to never run dry | watermarks ✅ verified |
| 9 | **Power the panel, in order, with waits** | ⚠️ **can damage hardware** — 500 ms T12 |
| 10 | **Turn the backlight on** and set brightness | ✅ (register layout bug fixed) |

**Nine of the ten are individually verified against what firmware programmed.**
What has never happened is **running them in order, for real** — that is
`intel_modeset_run()`, 35 steps, dry-run clean but never armed.

Two things stand out and both are true:

- **Step 5 (link training) is the untested one.** Everything under it checks out;
  the sequence has not executed.
- **Step 9 is the dangerous one.** Violating the 500 ms panel power-cycle delay,
  or driving AUX into an unpowered panel, can damage the hardware rather than
  just fail.

# Part 0b — what is actually hardest, ranked

"Big" and "hard" are different. **Acceleration is big** — 24 MB of Mesa, a
shader compiler, a memory manager. Enormous volume, but each individual piece is
well-trodden. **The display driver is hard** — small, but subtle, interactive,
and it gives you nothing when you get it wrong.

## The real difficulty: there is no error message

Every other kind of programming gives feedback. A wrong register in a display
driver gives you **a black screen**. No log line, no exception, no status bit.

And you cannot `printf` your way out, because the output device is the thing you
just broke.

`kernel/HANDOFF.md` says it directly about the pipe path: *"each one wrong is a black
screen with no error bit."*

**That absence of feedback is why this project has `hosttest/`, why it verifies
against what firmware programmed, and why the dry run exists.** Those are not
nice-to-haves; they are the substitute for a debugger.

## Ranked, hardest first

| | Why it is hard | State |
|---|---|---|
| **1. Link training** | An interactive negotiation with the panel over AUX. Voltage swing and pre-emphasis levels, clock-recovery phase, then channel-equalisation phase, with retries. The panel talks back and you adapt. **The only genuinely interactive part of the whole driver.** | ⚠️ never executed. Step 40 makes one attempt and fails loudly by design. |
| **2. The 35-step order** | No individual step is hard. The *sequence* is, because a dry run proves the writes are right and proves nothing about the order. | ⚠️ dry-verified, never run |
| **3. Watermarks** | Wrong = underrun = flicker or black, no error bit. And **C9 is still unsettled** — firmware's largest values fit both the narrow and wide encodings, so measurement cannot decide it. | ⚠️ narrow, on the plan's authority alone |
| **4. Panel power sequencing** | Not intellectually hard. **Highest stakes** — violating T12 or driving AUX into an unpowered panel can damage hardware, not just fail. And the real T9 (260 ms) lives in **VBT, not in any register**. | ✅ delays read from firmware |
| **5. The DPLL** | Computing dividers to hit an exact pixel clock, then waiting for lock — measured at ~80 ms, not the PRM's 5 ms. | ✅ **proven**, locks at 720p/1080p/1440p/4K60 |
| **6. VBT parsing** | The Video BIOS Table holds things no register exposes. Poorly documented. | not started |
| **7. EDID over I2C-over-AUX** | GMBUS does not serve eDP on DDI A, so the normal path is unavailable. | missing |

## Not started, and would be hard

- **Hotplug** — noticing a monitor was plugged in
- **External displays** — a second pipe, a second port, different timings
- **DP MST** — several monitors down one cable
- **Runtime power management** — DC states beyond what is already handled

## The summary

**Acceleration is a volume problem. The display driver is a subtlety problem.**

Link training is harder *per line of code* than anything in a blitter. It is also
about 200 lines. Mesa is 24 MB of mostly-mechanical compiler work.

**The hardest thing left in `intel.c` is link training, and the hardest thing
about the project is that nothing tells you when you are wrong.**

# Part 1 — the hardware

## Gen9.5, and what it physically is

Gen9.5 is the GPU microarchitecture in Kaby Lake, Coffee Lake, **Comet Lake**
and Goldmont Plus. It is Gen9 (Skylake) with light enhancements.

### The compute hierarchy

```
GPU
└── Slice                    (Gen9.5 scales 1-3 slices, 12-72 EUs)
    └── Subslice   x3        each with its own Local Thread Dispatcher,
        │                    Instruction Cache, Data Port, Sampler
        └── EU      x8       Execution Unit = a programmable shader core
            └── Thread x7    each with 128 SIMD-8 32-bit registers (GRF)
```

**This machine is GT2: 1 slice, 3 subslices, 24 EUs.**

So: **24 EUs × 7 threads = 168 hardware threads**, each with 128 SIMD-8
registers. Against **4 CPU cores** — three of which zlOS currently parks in
`cli; hlt` forever (`smp.c:79`).

That parallelism is the whole reason a GPU wins, and it is also why zlOS will
never match it by tuning C loops.

**[verified]** Measured blended fill rate of this exact GPU, offscreen GLX:
**5.21 Gpixel/s**, one 1920×1200 blended layer in **0.44 ms**
(`hosttest/gpu_fillrate.c`). zlOS's CPU renderer: **1.28 Gpx/s** plain,
**0.22 Gpx/s** subpixel-blended.

### The engines

Each engine has its own **Command Streamer** — the unit that fetches and parses
GPU commands from a ring buffer and dispatches work.

| Engine | Does |
|---|---|
| **Render / GPE** | 3D and GPGPU. The big one. Has its own command streamer. |
| **Blitter** | 2D copies |
| **Video (VCS)** | HEVC/H.264 decode and encode |
| **Video Enhancement (VECS)** | post-processing |

### The three firmware microcontrollers

Skylake and later ship three separate embedded controllers, each needing signed
firmware blobs:

| | What it is | Relevance to zlOS |
|---|---|---|
| **GuC** | Graphics Microcontroller — **an embedded i486 core**. Does GPU scheduling, power management, firmware attestation. | Only needed for acceleration. Not for modesetting. |
| **HuC** | HEVC/H.265 microcontroller — video encode offload | Irrelevant |
| **DMC** | **Display Microcontroller** | **Relevant.** Handles display power states. Its absence produces the `DMC firmware` warnings seen on FreeBSD/Linux. |

**`intel.c` needs none of these to do a modeset** — which is precisely why
modesetting is the reachable target and acceleration is not.

---

# Part 2 — the display engine (what `intel.c` actually drives)

This is the half of the GPU that zlOS is targeting, and it is **completely
separate from the compute/render half.**

## The pipeline

```
memory surface
   -> PLANE          (primary / sprite / cursor; stride, tiling, format)
   -> PIPE           multi-plane composition, colour conversion, scaling
   -> TRANSCODER     timing generation (hsync/vsync/porches)
   -> DDI            Digital Display Interface + PHY
   -> physical port  eDP / DP / HDMI
```

Per Intel's PRMs: *"display pipes perform input format conversion, multi-plane
composition, colour conversion, and scaling"*, and *"a pipe can only drive one
DDI"* (except DP Multi-Stream, which can fan several pipes onto one PHY).

Gen9 added: consuming **lossless compression directly**, **render-compressed
surfaces**, **Y-tiled surfaces**, and **on-the-fly 90/270 rotation**.

## Feeding it: the clock chain

```
reference clock -> DPLL -> CDCLK / link clock -> transcoder -> pixels
```

**[verified] on this panel:**

| | Value |
|---|---|
| Pixel clock | **241,690 kHz** — measured four independent ways, all agreeing |
| Refresh | **59.998 Hz** |
| Link | 4 lanes @ HBR (2.7 Gbps), the only working point |
| CDCLK | 337,500 kHz |
| DPLL0 | locked at rate_idx 1 — **feeds CDCLK, never disable it** |
| Total timing | 2720×1481 |
| Surface | X-tiled, stride 10240 |
| PSR | **ON** (`EDP_PSR_CTL = 81F00406`) |

The pixel clock was derived from `PIPE_LINK_M1/N1` — a DP link runs at a fixed
symbol rate, so the transcoder holds a ratio reconciling it with the pixel
clock. Read-only, exact, works while PSR is on.

## Watermarks — the part everyone forgets

`WM_PIPE` registers tell the display controller **when to fetch pixels from
memory**. Get them wrong and you get underruns — flicker or a black screen — with
no error bit. `WM_LP` covers low-power mode; `WM_LINETIME` sets line timing from
the horizontal resolution.

**[verified]** `WM_LINETIME` firmware `0000005B`, ours `0000005B` — **match**.

## Panel power sequencing — the part that can damage hardware

**[verified] from firmware on this panel:**

| Delay | Value |
|---|---|
| T1+T3 (power on → backlight) | 200 ms |
| T10 | 50 ms |
| T11+T12 (power off → next power on) | **500 ms** |
| T9 (real) | 260 ms — lives in **VBT**, not in any register |

Violating T12, or driving AUX into an unpowered panel, can **damage the panel**.
This is the one area of the project where "try it and see" is not acceptable.

---

# Part 3 — the software stack above the hardware

```
   application
       |  OpenGL / Vulkan
   ┌───▼──────────────────────────────────────────┐
   │  MESA (userspace)                            │
   │    iris  - OpenGL driver                     │   translates API calls into
   │    anv   - Vulkan driver                     │   GPU Command Buffers, and
   │    shader compiler (NIR -> Gen ISA)          │   compiles shaders to Gen ISA
   └───┬──────────────────────────────────────────┘
       |  ioctl, via libdrm
   ┌───▼──────────────────────────────────────────┐
   │  i915 (kernel)                               │
   │    GEM  - graphics memory manager            │
   │    TTM  - buffer placement / migration       │
   │    GT   - scheduling, command submission     │
   │    KMS / Display  - modesetting  <-- zlOS    │
   │    dma-buf / dma_fence - cross-driver sync   │
   └───┬──────────────────────────────────────────┘
       |
     hardware
```

The kernel docs put it plainly: **Mesa handles the "drawing"; i915 handles the
"computation" and "display".**

## Where `intel.c` sits

**Only in the KMS/Display box.** Nothing above it, and only part of that box.

That is not a small thing. It is also **exactly one subsystem of five**, and the
smallest one.

### zlOS versus i915, measured **[verified]**

| | Size |
|---|---|
| `i915.ko` uncompressed, this machine | **11.2 MB** |
| Mesa ANV (Intel Vulkan) | **24.3 MB** |
| **zlOS entire kernel** | **1.07 MB** |
| Linux i915 source | ~100,000 lines |
| **All hand-written zlOS kernel C** | **11,374 lines** |
| `intel.c` | **3,816 lines — 3.8% of i915** |

---

# Part 4 — what would be needed to go further

> **This table said "No" twice, and that was the wrong word both times.** Rewritten
> 2026-08-19 after the blitter ring ran on this silicon. Nothing here is
> impossible; each row has a *bounded* version and an *unbounded* one, and the
> old table costed the unbounded one and then wrote off the whole row. Naming the
> bounded version and its price is the useful answer. See the note under the
> table for what changed.

| Goal | Needs | Where zlOS is |
|---|---|---|
| **Modesetting** (current) | DPLL, transcoder, pipe, plane, DDI, AUX/DPCD, link training, watermarks, panel power | **Done for this hardware.** Haiku, at 25 years old, has exactly this and no more. Never yet run *on* the ThinkPad. |
| **2D blit acceleration** | Blitter engine, a ring buffer, buffer placement, fences | **The ring is proven on silicon** — 16384/16384 pixels, our own GGTT entries, no i915 (`kernel/docs/gpu-driver.md`). No heap was needed: zlOS uses fixed regions (`HI_GPU`). What is left is choosing what to accelerate, and the measurements say a plain fill is not it. |
| **3D acceleration, for a COMPOSITOR** | Render engine ring, the 3D pipeline packets, **three fixed shaders** | **Weeks, and now unblocked.** A compositor needs a handful of fixed operations - solid fill, blended fill, textured blit - not arbitrary programs. One shader is already captured out of Mesa (80 bytes, `gpu_shader.inc`) and the 77-packet pipeline with it (`kernel/docs/gen9-blend-pipeline.md`). The one outstanding piece is `RENDER_SURFACE_STATE`'s bit layout. |
| **3D acceleration, for ARBITRARY programs** | all of the above **plus a shader compiler targeting Gen ISA** | **Years, and the compiler is the reason** — Mesa is 24 MB of exactly that. This is the row that makes people say "impossible", and it is the only row where the cost is genuinely that shape. |
| **Video decode** | HuC firmware, VCS engine | **Not attempted, not ruled out.** The engine is present and enumerated (`VCS` in `i915_engine_info`). The firmware and the bitstream work are the cost, and nobody has priced them here. |

### What changed, and why the old verdict was wrong

The old table collapsed two very different goals into one row called "3D
acceleration" and then priced the harder one. That is what produced "No".

**A compositor does not need a shader compiler.** It needs the same three or four
shaders forever, and each can be lifted out of Mesa as bytes -
`INTEL_DEBUG=fs,hex` prints the machine code, and the constant-colour case is
**five instructions, 80 bytes, with the colour patchable in place**. A compiler
exists to turn *arbitrary* source into ISA. Nothing in a desktop needs that.

So the honest split is:

- **arbitrary GPU programs, like Windows and Linux run** — the compiler, years.
- **a GPU-accelerated desktop** — three captured shaders, a ring that already
  works, and one missing struct. Weeks.

The second is what a fast zlOS desktop actually requires, and the old table hid
it inside the first.

## The one honest datapoint about difficulty

**Genode** built a working Intel GPU multiplexer in **under 10K lines** — by
supporting *only* recent GPUs, versus i915's ~100K carrying every generation
since 2008. But that multiplexer is only the **arbitration layer**: Mesa still
does all the graphics, and it was built by a funded team.

And **Haiku** — 25 years, a real team, AMD and NVIDIA 3D acceleration working —
has **modesetting only on Intel.**

---

# Part 4b — what every other hobby OS does about Intel graphics

The most useful comparison in this document, because it is like-for-like.

## Redox — a direct peer, doing this right now

**Jeremy Soller**, Redox's founder and lead developer, has merged the first
changes for **a modesetting driver for Intel HD graphics**. He got there by
*"a few nights of reading through thousands of pages of PRMs"* — the identical
method that produced `kernel/docs/evidence/display/gen9-modeset-plan.json`.

His words: *"There is much more to do, but there is now a clear path to native
hardware accelerated graphics!"*

State: explicitly **"first baby steps"**, **"first changes"**. Target generation
not specified beyond "Intel HD graphics".

**This is the closest living comparison to `intel.c`, and zlOS is ahead of it.**
zlOS has a 21/21 hardware survey, M/N verified bit-for-bit against firmware, the
full pipe path verified, and a 35-step sequence that has been dry-run and
register-diffed. Redox has merged its first patches.

## Haiku — `intel_extreme`, 25 years in

| | |
|---|---|
| Driver | `intel_extreme` (kernel driver + accelerant) |
| **SandyBridge and older** | **well supported** |
| Newer generations | *"a bit hit or miss"* |
| Recently added | Gemini Lake, Ice Lake, more Tiger Lake |
| Acceleration | **modesetting only — no hardware 2D or 3D** |
| Fallback | VESA, when no native driver matches |

And their note on why the fallback hurts: **VESA cannot configure an arbitrary
resolution.** It picks from a fixed list baked into the card and cannot program
the chip. Exactly the limitation `intel.c` exists to remove.

**SandyBridge is 2011. zlOS targets Comet Lake — Gen9.5, 2019.** That is eight
years and several architectures newer than where Haiku is solid.

## The others

| OS | Intel graphics |
|---|---|
| **SerenityOS** | virtio-gpu and plain framebuffer. No native Intel modesetting. |
| **Genode** | Intel **Gen8+** GPU multiplexer, <10K lines — but built on Mesa, and it is the arbitration layer, not modesetting from scratch |
| **Managarm** | Mesa ported; 3D planned on virtio-gpu |
| **Essence** | own graphics stack, software rendered, no native Intel driver |
| **ReactOS** | can load real Windows drivers in principle |
| **TempleOS** | VGA only, 640×480×16 |
| **zlOS** | **Gen9.5 Comet Lake, modesetting, from scratch, verified against firmware** |

## What this comparison actually says

1. **`intel.c` is not behind. It is at or ahead of the field**, on newer silicon
   than anyone else handles well.
2. **Modesetting is where everyone stops.** Haiku at 25 years, Redox at 10 —
   both modesetting, neither accelerated on Intel.
3. **The method is the same everywhere: read the PRMs.** Soller did exactly what
   this project did. The difference is that zlOS then *verified every value
   against what firmware programmed*, which is a step further.
4. **The remaining gap is not knowledge, it is console access** — `lt_armed` is
   never armed from the kernel, and the 35-step order has never executed.

# Part 5 — documentation

Intel publishes real PRMs, which is why this project is possible at all.

| Document | Covers |
|---|---|
| *The Compute Architecture of Intel Processor Graphics Gen9* | EUs, slices, subslices, the memory hierarchy |
| PRM **Volume 12: Display Engine** | pipes, planes, transcoders, DDI, watermarks |
| PRM **Volume 9: Render Engine** | command streamer, 3D pipeline |
| PRM **Volume 11: Display** (BDW) | older but often clearer |

Caveat from the OSDev community, and consistent with this project's experience:
Intel's *"functional descriptions are quite sparse"*. The registers are
documented; the **sequences** often are not. That is why
`kernel/docs/evidence/display/gen9-modeset-plan.json` had to resolve **13 conflicts between
sources**, and why the survey settling them by measurement on real silicon was
worth building.

**And this project has now gone one better than the docs in places** —
`intel.c`'s M/N computation was verified **bit-for-bit against what firmware
programmed**, which is a stronger check than the PRM provides.

---

# Part 6 — what this means for zlOS, concretely

1. **`intel.c` targets the right layer.** Display, not render. It is where a
   25-year-old staffed project sits on this vendor.
2. **The GPU's 168 hardware threads are unreachable.** Not by effort — by the
   size of the shader compiler between you and them.
3. **But 3 of 4 CPU cores are parked** (`smp.c:79`). That is a real, available
   4× that needs no driver, and nothing currently uses it.
4. **SSE is on and unused.** `cpu.c` detects SSE/SSE2/SSE3/SSSE3; `kernel/HANDOFF.md`
   confirms SSE is enabled in the 64-bit build. `fb.c`'s per-pixel blend loops
   are the obvious first customer.
5. **3D is reachable in software.** SerenityOS runs Quake III on a SIMD
   rasterizer. See `kernel/docs/research/os-landscape.md`.
6. **Read i915 as documentation, never as source.** That is already what
   `intel.c` does, and it is the only viable relationship — see
   `kernel/docs/research/desktop-prior-art.md` on why LinuxKPI-style porting is closed here.

---

## Sources

- [The Compute Architecture of Intel Processor Graphics Gen9 (PDF)](https://cdrdv2-public.intel.com/774710/the-compute-architecture-of-intel-processor-graphics-gen9-v1d0-166010.pdf)
- [Intel PRM Vol 12: Display Engine (PDF)](https://cdrdv2-public.intel.com/705833/intel-gfx-prm-osrc-tgl-vol-12-display-engine.pdf) · [Vol 9: Render Engine (PDF)](https://cdrdv2-public.intel.com/682648/intel-gfx-prm-osrc-lkf-vol09-renderengine.pdf) · [Vol 11: Display, BDW (PDF)](https://cdrdv2-public.intel.com/690828/intel-gfx-prm-osrc-bdw-vol-11-display.pdf)
- [Gen9.5 — WikiChip](https://en.wikichip.org/wiki/intel/microarchitectures/gen9.5) · [Gen9 — WikiChip](https://en.wikichip.org/wiki/intel/microarchitectures/gen9)
- [drm/i915 Intel GFX Driver — kernel docs](https://docs.kernel.org/gpu/i915.html)
- [The i915 Kernel Driver Part 1: the Linux graphics stack panorama](https://dev.to/deleon_karen_2216eb5888b3/the-i915-kernel-driver-part-1-the-linux-graphics-stack-panorama-and-i915-overview-2jh7)
- [Enabling the GuC/HuC firmware for Linux on new Intel GPU platforms (PDF)](https://cdrdv2-public.intel.com/609249/609249-final-enabling-intel-guc-huc-advanced-gpu-features-v1-1-1.pdf)
- [Intel HD Graphics — OSDev Wiki](https://wiki.osdev.org/Intel_HD_Graphics) · [Native Intel graphics — OSDev Wiki](https://wiki.osdev.org/Native_Intel_graphics)
- [Security of the Intel Graphics Stack — Igor's Blog](https://igor-blue.github.io/2021/02/10/graphics-part1.html)
- [Genode GPU multiplexer — Phoronix](https://www.phoronix.com/news/Genode-GPU-Multiplexer)

Verified values come from `kernel/HANDOFF.md`, `hosttest/modeset_test.c --survey`
(21/21 passing), `hosttest/fbbench.c` and `hosttest/gpu_fillrate.c`.
