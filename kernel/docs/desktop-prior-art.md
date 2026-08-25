# How other hand-built OSes do graphics — and what it means for zlOS

Researched 2026-08-17, after the question *"how did TempleOS do it, and how do
other self-made OSes look so good and do 3D?"*

**The headline: nobody in this space has GPU acceleration. Not one of them.**
The best-looking hobby OS in the world runs Half-Life — on the CPU.

---

## TempleOS

Terry Davis's OS, written in his own language (HolyC) — the closest structural
parallel to zlOS, which is written in Roy's own language.

| | |
|---|---|
| Resolution | **640×480** |
| Colours | **16** |
| 3D | yes — software, built into the standard library |
| GPU driver | none. VGA only. |

Davis stated the 640×480 / 16-colour / single-voice-audio limits were deliberate
design constraints, not technical ceilings.

**What this means for zlOS: you are already well past TempleOS graphically.**
TempleOS: 640×480, 16 colours. zlOS: 1920×1200+ true colour, real TrueType
glyphs, subpixel LCD rendering, gamma-correct linear-light blending, dithered
gradients. That is not a close comparison.

TempleOS's look is **deliberate minimalism**, not a graphics achievement. If the
goal is "looks good", TempleOS is not the bar — it is a different intention.

---

## SerenityOS — the one to actually study

The most relevant prior art by a wide margin, and it answers the "how do they
look so good" question directly.

| | |
|---|---|
| Written in | C++, from scratch, custom kernel |
| Look | late-1990s productivity aesthetic, clean and cohesive |
| Compositor | **WindowServer** — a userspace service handling window placement, compositing of overlapping windows, and input routing |
| Toolkit | **LibGUI** + **LibGfx** — window themes, gradient painting, frame metrics |
| 3D | **LibGL** — OpenGL 1.5, rendered by **LibSoftGPU**, a software rasterizer using SIMD |
| GPU acceleration | **none** |

**How LibSoftGPU works:** triangles are processed in grid-aligned **16×16 pixel
blocks**, computing barycentric coordinates and edge derivatives, interpolating
bilinearly across each block. Based on Fabian Giesen's *"A trip through the
Graphics Pipeline"* and Scratchapixel's rasterization series.

**It runs Half-Life.** In software. On the CPU.

### Why SerenityOS looks good, precisely

Not the renderer. Three other things:

1. **A real toolkit** — `LibGUI` knows what a button is, where it goes, how it
   arranges. zlOS has no equivalent (`desktop-northstar-feasibility.md`).
2. **A theme system** — `WindowManager` loads a theme object into a shared
   buffer; window frame painting lives in `LibGfx` with gradient support. Colours
   and metrics are **data**, not hardcoded numbers.
3. **Many contributors over years.**

**The lesson: their advantage over zlOS is architectural, not graphical.** The
theme-as-data idea in particular is directly stealable and small.

---

## Essence OS — the closest analogue to zlOS, and the most useful one

**One developer** (nakst), from scratch, since **2017**. Recipient of the 2021
Icculus Microgrant. This is the nearest thing to Roy's situation that exists.

| | |
|---|---|
| Team | **one person** |
| Started | 2017 — so **~9 years** to what it is now |
| Renderer | **software vector renderer, with complex animation support** |
| Window manager | **runs in the kernel** |
| Toolkit | its own graphics library |
| Text | FreeType + Harfbuzz, multi-lingual layout |
| Visual reference | **Windows 7**, deliberately |
| Signature feature | windows split into **tabs** — several apps in one window |
| GPU acceleration | **none** |

Three things worth taking from it:

1. **The window manager is in the kernel.** That is the same structural choice
   `archive/superseded/desktop-plan.md` makes for zlOS (mechanism in C, in-kernel). Independent
   confirmation it is a reasonable design, not a compromise.
2. **A software *vector* renderer, and it animates.** So "software rendered" does
   not mean "static and flat". One person built animated vector UI on a CPU.
   The animation ambition in `desktop-polish-and-speed.md` is proven achievable.
3. **It picked an explicit visual reference (Windows 7) and followed it.** It
   did not invent a look from nothing. That is why it reads as coherent.

**And the honest number: nine years, one person.** That is the realistic scale
of "a good-looking desktop OS from scratch".

## The rest of the landscape

| OS | Look / status |
|---|---|
| **Haiku** | BeOS clone, R1/beta5 (Sept 2024), still beta in 2026 but very stable. Pervasively multithreaded GUI — every window on its own thread. |
| **Redox** | Rust. Big 2025 gains — 500–700% faster I/O, multithreading by default, Servo and WebKitGTK ported. But the **UI is "sluggish and unpolished"**. |
| **Managarm** | Microkernel, fully asynchronous I/O, x86-64 / Arm64 / RISC-V, SMP, ACPI, AHCI, NVMe, IPv4. Runs **both Wayland and X11 servers**. |
| **Asterinas** | Rust, Linux-ABI compatible, 230+ syscalls, 4 years in. Papers at USENIX ATC 2025 and ICSE 2026. Kernel-focused, not a desktop. |
| **Vinix** | Written in the V language — another "my own language" OS. |
| **duckOS** | UNIX-like with its own graphical window manager. |
| **MOROS** | Rust, x86, minimal. |
| **ReactOS / MenuetOS / KolibriOS** | Windows-compatible / tiny assembly. Different goals. |

Redox is the instructive counter-example: a modern language, serious engineering,
academic-grade systems work — and the UI is still the weak part. **A good-looking
UI is not downstream of good systems work.** It is its own discipline, and it
does not come for free with a better kernel.

---

## Who DOES have GPU acceleration — and how they got it

"Nobody has it" needed testing. Some do. **How they got it is the finding.**

### Nobody writes a 3D driver. Everybody ports Mesa.

| OS | GPU acceleration | How |
|---|---|---|
| **Haiku** | AMD ✅, NVIDIA ✅ (2026), **Intel ❌** | Mesa's **RADV** + **Zink** (OpenGL over Vulkan). NVIDIA via Mesa's **NVK** + the NVRM kernel driver. v0.0.2 pre-release Jan 2026 plays 3D games at reasonable FPS on Turing. |
| **Managarm** | in progress | Ported **Mesa** — *"the patches were extremely simple, mainly `#ifdef`s and minor `meson.build` changes"* — and **upstreamed** them. 3D planned on top of virtio-gpu. |
| **Genode** | Intel Gen8+ ✅ | A **GPU multiplexer under 10K lines**, by supporting only recent GPUs versus i915's ~100K carrying legacy hardware. Mesa/Gallium still does the graphics. |
| **Fuchsia** (Google) | ✅ | **Magma**: a hardware-specific client library per app (an ICD — i.e. Mesa) plus a system driver managing the hardware. |

**The split is the same every time:**

```
userspace   shader compiler, OpenGL/Vulkan state tracking, actual graphics
            -> MESA. Nobody rewrites this.
kernel      memory manager, command submission, contexts, arbitration
            -> each OS writes this. Genode did it in <10K lines.
```

So "add GPU acceleration" never means *write a graphics driver*. It means
**port Mesa, then write the kernel-side command submitter under it.**

### The most relevant single fact in this document

> **Haiku's AMD and Intel drivers are modesetting only. No hardware 2D or 3D
> acceleration.**

Haiku is 20+ years old with a real team. On Intel it has **exactly what `intel.c`
is building.** Two things follow, pointing opposite ways:

1. **`intel.c` is aimed at the right target.** Modesetting on Intel is where a
   mature, staffed hobby OS actually sits. Not a low bar.
2. **Intel 3D is the hard one even for projects that already ship GPU
   acceleration.** Haiku got AMD and NVIDIA working and still has nothing on
   Intel. zlOS's hardware is the worst case.

---

## "Can we just take a premade driver from another OS?"

The right question, and it has a real answer with a real precedent.

### FreeBSD did exactly this. Here is how.

FreeBSD runs the **actual Linux i915 source**, largely unmodified, via a
compatibility shim called **LinuxKPI**:

- LinuxKPI *fakes the Linux kernel's internal API* on top of FreeBSD's.
- The `drm-kmod` tree holds **source copied from Linux**, patched only minimally.
- Project rule: *"If a patch can be avoided by adding functionality to LinuxKPI,
  do that instead. No new code should be added to the driver module."*

**They did not port the driver. They emulated the operating system underneath
it.** That is the only approach that survives, because i915 changes constantly
and a forked copy rots immediately.

### Why that route is closed to zlOS — measured on this machine

| | Size |
|---|---|
| `i915.ko`, uncompressed, this laptop | **11.2 MB** |
| Mesa ANV, Intel's Vulkan driver | **24.3 MB** |
| **zlOS entire kernel (`kernel.elf`)** | **1.07 MB** |

**The Intel kernel driver alone is ~10× the size of all of zlOS.** Mesa's Vulkan
driver is ~23×.

By source:

| | Lines |
|---|---|
| Linux i915 | **~100,000** |
| **All hand-written zlOS kernel C** | **11,374** |
| `kernel.zl` | 2,198 |
| `intel.c` | 3,816 — **3.8% of i915** |

### What i915 actually needs underneath it

Porting it means providing all of this, because the driver assumes it:

- **GEM** — the graphics memory manager; all video memory is GEM buffer objects
- **TTM** — *"a large, complex piece of code that turned out to be hard to use"*,
  added to i915 for discrete GPU local memory
- **dma-buf / dma_fence** — with strict cross-driver locking contracts:
  `dma_resv` outermost, `ww_acquire_ctx` hoisted to the top of the call chain
- Power-domain management, ACPI, a scheduler, workqueues, ww-mutexes, and a heap

The driver is *"hundreds of .c files"*, with *"a sizeable amount of indirection"*
because one codebase spans every Intel generation.

**zlOS has no heap, no filesystem, no locking primitives and no allocator — by
design.** LinuxKPI for zlOS would mean building a Linux-shaped kernel first,
which is the opposite of what this project is.

### The honest verdict on transferring a driver

| Route | Verdict |
|---|---|
| Copy i915 into zlOS | **No.** 11.2 MB and hundreds of files against a 1.07 MB kernel with no heap. |
| LinuxKPI-style shim | **No.** You must first build the Linux kernel API it emulates. FreeBSD had one already. |
| Port Mesa for userspace 3D | **No.** Needs POSIX, a toolchain, dynamic linking, threads, a filesystem. |
| Genode-style <10K multiplexer | **Closest** — and it is only the arbitration layer, built by a funded team, still on top of Mesa. |
| **Read i915 as documentation** | **Yes, and this is already what `intel.c` does.** HANDOFF calls it *"often a good reference"*. Reading it to learn register semantics is exactly right, and it is working. |

**The distinction that matters: `intel.c` already borrows Linux's *knowledge*
without borrowing its *code*. That is the correct and only viable relationship.**

---

## The one real escape hatch: virtio-gpu

`virtio_gpu.c` exists and works, and `virtio_gpu.c:314` deliberately negotiates
**no virgl, no blob resources** — 2D only, on purpose.

But **virgl/Venus is how a guest gets real 3D**: the guest proxies commands to
the host's GPU. Managarm's 3D plan is built on precisely this.

Enabling virgl would give zlOS genuine hardware-accelerated 3D **inside QEMU**,
and nothing at all on the ThinkPad, which has no hypervisor under it.

Worth knowing it exists. Not worth doing while the laptop is the target.

## GPU acceleration on bare metal: the honest answer

The OSDev consensus, from its own forums and wiki:

- Getting even **basic 2D acceleration** on an Intel GPU requires *"absolutely
  insane effort and research."*
- Intel does publish register documentation, and Linux's `i915` is a reference —
  but the *"functional descriptions are quite sparse."*
- **The more modern the chip, the worse it gets.** For the 8xx/9xx family only
  the i82965 is properly documented.

zlOS targets **Gen9.5 / Comet Lake**, which is far newer than anything in that
"documented" set.

**And this project already has direct evidence of the cost.** Getting the
*display* driver working — not 3D, not even 2D blitting, just **turning the panel
on at the right timing** — has taken a researched 13-conflict plan, a
21-check hardware survey, an 86-defect audit, and a host-side development
harness. That is `intel.c`, and it still has never executed a cold-start modeset.

**A 3D driver is a strictly larger problem than that, by a wide margin.**

### So: do not write a GPU driver

Not "it's too hard" — **nobody has, and the ones who got furthest chose not to.**
SerenityOS has a full desktop, a browser engine, and Half-Life, and it renders
every pixel on the CPU.

---

## What zlOS should do instead

### 1. Software 3D with SIMD — the proven path

`fb3d.c` already exists: a filled, flat-shaded, back-face-culled cube with
fixed-point Bhaskara sine, integer only, no float, no GPU. It even has a clip
rectangle (`fb3d_set_clip`) — which `fb.c` still does not.

The upgrade path is exactly SerenityOS's: **16×16 tiles, barycentric
coordinates, SIMD.**

And the hardware is there — `cpu.c` already detects SSE/SSE2/SSE3/SSSE3, and
`HANDOFF.md` confirms **SSE is enabled** in the 64-bit build. Nothing is
currently using it. `fb.c`'s per-pixel blend loops are the obvious first
customer, well before 3D.

### 2. Steal the theme-as-data idea

SerenityOS keeps colours and frame metrics in a theme object rather than in the
drawing code. zlOS hardcodes both. This is small, cheap, and would fix the
"spacing is hand-picked numbers" gap in `desktop-polish-and-speed.md`.

### 3. Accept the real comparison

zlOS's **renderer** is already ahead of TempleOS's and competitive with
SerenityOS's on quality per pixel — gamma-correct subpixel text is not something
every hobby OS has.

What SerenityOS has and zlOS does not is **WindowServer and LibGUI**: a
compositor and a toolkit. That is the gap, and it is a software-architecture
gap, not a graphics one.

---

---

## Roy already wrote a graphics plan — and zlOS went the other way

`docs/archive/superseded/GRAPHICS_PLAN.md`, drafted **2026-08-03**, maps a five-layer ladder:

```
5. GPU / 3D        OpenGL / DirectX / Vulkan
4. accelerated 2D  Direct2D
3. live window     CreateWindowEx + message loop + GDI    <- "the first FFI layer"
2. raw framebuffer a block of pixels in memory
1. image file      compute pixels, write a .bmp           <- "zl IS HERE"
```

Its conclusion: *"the live window is behind FFI"*, layers 3–5 are **all gated on
calling Windows DLLs**, and the ordered path is `bytes` type → canvas → PNG →
terminal game -> **FFI** -> future stdlib window module -> OpenGL.

**Eight days later zlOS jumped to layer 3 without any of it.**

Not by adding FFI — by **deleting the operating system underneath**. On bare
metal there is no `CreateWindowEx` to call, because there is no Windows; the
framebuffer is just memory, and `poke32` reaches it. No `bytes` type was needed,
because the kernel's pixel buffers are C arrays at fixed physical addresses.

That plan is not wrong — it is a **Windows-hosted** plan, and it is still correct
for `zl` the language on Linux/Windows. It is simply not the road zlOS took.

**The part that must not be carried over:** the plan says layer 5 (GPU/3D) is
reached through `opengl32.dll` via FFI. **On zlOS that DLL does not exist and
never will.** Layer 5 on bare metal means writing the driver yourself — which
this document has just established that nobody does. So on zlOS, layer 5 is not
"FFI to OpenGL", it is **"write a software rasterizer"**: the SerenityOS /
Essence path, and `fb3d.c` is already the first step of it.

`docs/archive/superseded/GRAPHICS_PLAN.md` has been annotated to say so.

## Sources

- [TempleOS — HandWiki](https://handwiki.org/wiki/Software:TempleOS)
- [TempleOS: A Lone Programmer's Crusade in 640x480 glory](https://dev.to/sshamza/templeos-a-lone-programmers-crusade-in-640x480-glory-2964)
- [SerenityOS — Grokipedia](https://grokipedia.com/page/SerenityOS)
- [LibGL — SerenityOS docs](https://www.mintlify.com/serenityOS/serenity/api/libgl)
- [The 4th year of SerenityOS](https://serenityos.org/happy/4th/)
- [LibGfx+WindowServer window themes PR](https://github.com/SerenityOS/serenity/pull/25537)
- [The probability of hw-accelerated graphics in a hobby os — OSDev forum](https://forum.osdev.org/viewtopic.php?f=11&t=23148)
- [Accelerated Graphic Cards — OSDev Wiki](https://wiki.osdev.org/Accelerated_Graphic_Cards)
- [Native Intel graphics — OSDev Wiki](https://wiki.osdev.org/Native_Intel_graphics)
- [Indie & Hobbyist Operating Systems 2026 Deep Dive](https://www.youngju.dev/blog/culture/2026-05-16-indie-hobby-operating-systems-2026-serenityos-ladybird-haiku-reactos-redoxos-theseus-plan-9-9front-arcanos-deep-dive.en)
- [Essence OS — GitLab](https://gitlab.com/nakst/essence) · [OSnews](https://www.osnews.com/story/133812/essence-an-new-desktop-operating-system/) · [architecture writeup](https://en.linuxadictos.com/essence-an-os-with-its-own-kernel-and-desktop-built-from-scratch.html)
- [Asterinas](https://asterinas.github.io/) · [Three alternative microkernels — The Register](https://www.theregister.com/software/2025/09/12/three-alternative-microkernels-show-devs-dont-need-linux/1428275)
- [Haiku running Zink over RADV — Phoronix](https://www.phoronix.com/news/Haiku-OS-Zink-RADV)
- [Haiku gets accelerated NVIDIA driver — OSnews](https://www.osnews.com/story/144097/haiku-gets-accelerated-nvidia-graphics-driver/) · [Hackaday](https://hackaday.com/2026/07/12/porting-the-nvidia-gpu-driver-to-haiku-for-3d-acceleration/)
- [Genode is developing a GPU multiplexer for Intel — Phoronix](https://www.phoronix.com/news/Genode-GPU-Multiplexer) · [Genode 17.08 supports Intel Gen-8 GPUs — OSnews](https://www.osnews.com/story/29988/genode-1708-supports-intel-gen-8-gpus/)
- [Managarm end-of-2023 update (Mesa port)](https://managarm.org/2023/12/31/end-of-year-update.html)
- [Magma: Overview — Fuchsia](https://fuchsia.dev/fuchsia-src/development/graphics/magma)
- [FreeBSD drm-kmod](https://github.com/freebsd/drm-kmod) · [LinuxKPI-based DRM on FreeBSD — Phoronix](https://www.phoronix.com/news/DRM-Next-KMOD-On-FreeBSD-11)
- [drm/i915 Intel GFX Driver — Linux kernel docs](https://docs.kernel.org/gpu/i915.html)

Sizes in the tables above were measured on this laptop 2026-08-17:
`xz -dc i915.ko.xz | wc -c`, `ls -l libvulkan_intel.so`, `ls -l kernel.elf`,
and `wc -l` over the hand-written kernel sources.
