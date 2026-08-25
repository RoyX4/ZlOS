> **AUDITED 2026-08-19 · REFERENCE, STILL VALID.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. ~230 of its 306 lines survey 13 other hobby OSes and cannot decay; its one measured number (i915.ko = 11.2 MB) re-measures correctly today. The parts that ARE a task list have moved on — `fb_clip` and SIMD are both done, and all four bullets of its "where zlOS sits" self-assessment are false on main. Read it as reference, never as status.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**


# The hobby OS landscape — and where zlOS sits in it

Researched 2026-08-17. A survey of the OSes worth knowing about, written for
someone building one, with **3D as the stated goal**.

Companion to `desktop-prior-art.md`, which covers the graphics/GPU question
specifically. This one is "what are these projects actually like".

---

## The headline for the 3D goal

> **SerenityOS runs Quake III. On a software rasterizer. No GPU.**

Its ports list includes Chocolate Doom, DOOM, **Quake, Quake II, Quake III**,
Half-Life and Half-Life: Opposing Force — all running on **LibGL**, its OpenGL
1.5 implementation, backed by **LibSoftGPU**, a SIMD software rasterizer that
works in 16×16 pixel tiles with barycentric coordinates.

**So "3D without a GPU" is not a compromise or a stunt. It is a solved problem
with a known technique, and someone has shipped Quake III on it.**

That is the path. `fb3d.c` — the filled, back-face-culled, integer-maths cube
that already exists — is the first step of exactly this pipeline.

---

## Feature inventories — what each one actually has

Written out in full, because the useful thing is not "SerenityOS is good", it is
*which specific pieces exist and what they are called.*

### SerenityOS — the most complete feature list of any of them

**Kernel:** 64-bit, **preemptive multitasking**, multi-threaded. ARM and RISC-V
ports in progress.

**Security** (unusually strong for a hobby OS): hardware protections, limited
userland capabilities, **W^X memory**, **`pledge` and `unveil`** (from OpenBSD),
**(K)ASLR**, OOM-resistance, web-content process isolation, modern TLS.

**System services:** `WindowServer`, `LoginServer`, `AudioServer`, `WebServer`,
`RequestServer`, `CrashServer` — each a separate process, modern IPC between
them.

**Libraries — this is the part worth studying, it is a whole toolkit:**

| Library | Does |
|---|---|
| `LibGUI` | the GUI toolkit — widgets, layout |
| `LibGfx` | drawing, gradients, window themes, frame metrics |
| `LibIPC` | cross-process communication |
| `LibWeb` | HTML/CSS engine |
| `LibJS` | JavaScript engine |
| `LibGL` / `LibSoftGPU` | OpenGL 1.5 + software rasterizer |
| `LibAudio` | audio |
| `LibDSP` | signal processing / synthesizer chains |
| `LibVT` | terminal emulation |
| `LibProtocol` | out-of-process network I/O |
| `LibTLS` / `LibHTTP` / `LibIMAP` | TLS, HTTP(S), IMAP |
| `LibELF` | ELF loading |
| `LibPthread` / `LibThreading` | POSIX and higher-level threading |
| `LibMarkdown`, `LibM`, `LibPCIDB` | markdown, maths, PCI ID database |

**Filesystems:** ext2, plus POSIX-style virtual filesystems — `/proc`, `/dev`,
`/sys`, `/tmp`.

**POSIX compatibility:** standard commands, shell, syscalls, signals, file paths.

**Applications:** web browser, **HackStudio** (a C++ IDE), **Piano** (a desktop
synthesizer), Mail client, messenger, text editor, image viewer, terminal,
karaoke app, games, colour themes.

**Ports: over 300**, including Chocolate Doom, DOOM, Quake, Quake II,
**Quake III**, Half-Life, Half-Life: Opposing Force, compilers and Unix tools.

### Haiku — the ideas worth stealing

**BFS filesystem:** journaling, 64-bit design, **extended attributes**,
**attribute indexing**, and **fast queries over them**. You search files by
filetype-specific attributes from the Deskbar, not just by name. This is the BeOS
idea nobody else copied and it is genuinely good.

**PackageFS:** packages are **never unarchived**. They are mounted as a read-only
virtual filesystem overlay, assembled on the fly. Installing is mounting.

**Replicants:** self-contained fragments of applications you can **drag onto the
desktop**, where they keep working **without their parent app running**.

**BeAPI handler/looper model:** message passing with locking done automatically;
creating a window **creates a thread automatically**. Hence the "pervasively
multithreaded GUI" — every window on its own thread, and the responsiveness that
comes with it.

**Multimedia as a first-class OS citizen:** PCM audio, MIDI, video decode built
into the system rather than bolted on.

**Ported:** Mesa3D, Qt, GTK. Legacy BeOS R5 binaries still run.

### Essence — one person, and a coherent feature set

**Kernel:** task scheduler with **multiple priority levels**; memory management
with **shared memory, `mmap`, memory-mapped files, multithreaded page zeroing,
working-set balancing**; a filesystem-independent **cache manager**;
**on-demand module loading**; a **TCP/IP stack**; an **audio subsystem with
mixing**; VFS.

**Filesystems:** **EssenceFS** read-write, plus reading ext2, FAT, NTFS,
ISO9660.

**Graphics:** window manager **in the kernel**; its own graphics library; a
**vector software engine with complex animated effects**; the interface is
**fully vector and scales automatically to any resolution**.

**UI:** windows split into **tabs** — several apps in one window, grouped by
task.

**Apps:** File Manager, Text Editor, IRC client, System Monitor.

**Also:** optional POSIX subsystem running GCC and some Busybox tools.

### Redox — the microkernel discipline

**Kernel: ~20,000 lines.** (Linux: 20+ million.) Drivers, filesystems, network
stacks and system services all run **in userspace as isolated daemons** — a
driver that panics can be restarted without touching the rest of the system.

**"Schemes":** one IPC mechanism through which *everything* talks — kernel,
drivers, services, programs. The URL-like namespace is the whole system model.

**RedoxFS:** *"many of the features of ZFS, in a more modular design"* —
transactional, copy-on-write.

**relibc:** an almost-POSIX C standard library **written in Rust**. Runs C, C++
and Rust programs.

**Orbital:** the desktop environment.

**2025–26:** self-hosted compilation, RISC-V port, 500–700% I/O gains,
multithreading on by default, Servo and WebKitGTK ported. Builds a live USB with
one command.

**And the UI is still "sluggish and unpolished."** Keep both halves of that.

### The rest, briefly

- **ReactOS** — reimplements the NT kernel, HAL, drivers, `Win32k`, `USER32`,
  `GDI32` **in-house**. Not Wine. NTFS, FAT32, ext. Office 2007, Photoshop CS6,
  AutoCAD run, badly.
- **Managarm** — microkernel, **fully asynchronous I/O throughout**, x86-64 /
  Arm64 / RISC-V, SMP, ACPI, AHCI, NVMe, IPv4, Intel virtualisation. Runs
  **both Wayland and X11**. Mesa ported and upstreamed.
- **Asterinas** — Rust, **230+ Linux syscalls**, Linux-ABI compatible, framekernel
  design. USENIX ATC 2025 and ICSE 2026 papers. No desktop.
- **9front** — Plan 9's *"everything is a file — truly"*, **9P** protocol,
  **per-process namespaces**, `rio` window manager, `acme` editor, `plumb`
  messaging. Modern USB, WiFi, NVMe, UEFI. UTF-8 was invented for Plan 9.
- **Theseus** — single address space, **isolation without an MMU** via the Rust
  type system, **runtime-swappable kernel modules**. Pure research.
- **ArcaOS** — commercial OS/2 successor, UEFI/ACPI/USB/NVMe on an OS/2 base.
  **Runs in real ATMs and industrial controllers.**
- **MenuetOS** — pure assembly, fits on a **1.44 MB floppy**, and still has a
  GUI, a text editor and a web browser.
- **TempleOS** — 640×480, 16 colours, software 3D, own language (HolyC),
  ring-0-only, no networking by design.

## The full table

| OS | Since | Team | Language | GUI | 3D | Runs real software? |
|---|---|---|---|---|---|---|
| **SerenityOS** | 2018 | hundreds of contributors | C++ | own desktop, '90s aesthetic | **software OpenGL 1.5, Quake III** | **300+ ports** |
| **Haiku** | ~2001 (25 yr) | real team | C++ | BeOS-style, *"crisp responsiveness no other desktop quite matches"* | Mesa3D; AMD+NVIDIA accel, **Intel modesetting only** | Qt, GTK, legacy BeOS apps |
| **Essence** | 2017 | **one person** | C/C++ | tabbed windows, Windows-7-inspired | software **vector** renderer with animation | GCC, some Busybox |
| **ReactOS** | 1998 | community | C | reimplements Win32k/USER32/GDI32 | via Windows drivers | Office 2007, Photoshop CS6, AutoCAD — *limited* |
| **Redox** | 2015 | Jeremy Soller + community | Rust | Orbital | — | POSIX shell, package manager; **UI still "sluggish and unpolished"** |
| **Managarm** | — | small | C++ | Wayland **and** X11 servers | Mesa ported, 3D planned on virtio-gpu | Linux software compat |
| **Asterinas** | ~2022 | academic + industry | Rust | none — kernel only | — | 230+ Linux syscalls; USENIX ATC 2025, ICSE 2026 papers |
| **9front** | 2011 fork | community | C | rio window manager | — | Plan 9 tools; modern USB/WiFi/NVMe/UEFI |
| **Theseus** | PhD project | research | Rust | minimal | — | no — pure research |
| **ArcaOS** | 2017 | **commercial** | C | OS/2 desktop | — | 1990s OS/2 business apps; **runs in ATMs and industrial controllers** |
| **MenuetOS** | ~2000 | small | **pure assembly** | own GUI | — | fits on a **1.44 MB floppy**, has a web browser |
| **Vinix** | — | small | **V language** | — | — | another "my own language" OS |
| **duckOS / MOROS / banan-os** | — | solo/small | C++/Rust | duckOS has a window manager | — | early |

---

## The four that actually teach zlOS something

### 1. SerenityOS — the 3D proof

Started 2018 by Andreas Kling; *"hundreds of contributors joined"*. Full vertical
integration: bootloader, kernel, libc, drivers, shell, terminal, text editor,
image viewer, **web browser**, email client, messenger.

**What to take:** the software 3D pipeline. 16×16 tiles, barycentric
coordinates, SIMD. It is documented, it is proven, and it runs Quake III.

**What not to conclude:** hundreds of contributors did that. The technique
transfers; the timeline does not.

**Footnote worth knowing:** its browser, **Ladybird**, spun out in 2024–25, got
funded by Shopify and GitHub's co-founder, hit alpha in summer 2026 and targets
beta 2027. **The browser outgrew the OS.** A sub-component of a hobby OS became
a serious independent project. That happens.

### 2. Essence — the solo-developer benchmark

**One person, since 2017.** Software vector renderer *with animation*. Window
manager in the kernel — the same structural choice `archive/superseded/desktop-plan.md` makes.
Tabbed windows. Windows 7 as a deliberate visual reference.

**What to take:** this is the realistic ceiling for one person, and it is a good
ceiling. Also: he **picked a visual reference and followed it** rather than
inventing a look. That is why it reads as coherent.

**The number: nine years, one person.**

### 3. Haiku — the "mature project" reality check

25 years. A real team. Genuinely excellent desktop responsiveness. Mesa3D, Qt
and GTK ported. AMD and NVIDIA 3D acceleration working.

**And its Intel driver is modesetting only.**

That is exactly what `intel.c` is building. Twenty-five years and a team, and on
Intel they are where zlOS is aiming. **`intel.c` is not a small target.**

### 4. Redox — the counter-example

Rust, since 2015, microkernel, self-hosting compilation, RedoxFS, huge 2025
performance gains, Servo and WebKitGTK ported. Serious systems engineering.

**And the UI is still described as "sluggish and unpolished."**

**A good-looking desktop is not downstream of a good kernel.** It is a separate
discipline that has to be worked at directly. This is the single most useful
warning in the whole survey.

---

## Where zlOS actually sits

**Ahead of most of these on the thing it has built:**

- The renderer — real TrueType glyphs, subpixel LCD rendering, gamma-correct
  linear-light blending, dithered gradients. TempleOS was 640×480/16 colours.
  MenuetOS is assembly with a basic GUI. zlOS is doing things most of this list
  does not.
- Boots **three ways** with no GRUB — BIOS multiboot, its own 512-byte
  bootloader, and native UEFI. That is unusual.
- Written in **its own language**, which only Vinix and TempleOS also claim.

**Behind on the things it has not started:**

- No compositor (designed, unbuilt)
- No toolkit, no layout engine (not designed)
- 7 blocking demos, not applications
- No 3D pipeline beyond one cube

**The unusual bet:** zlOS's stated point is the **Intel display driver** — the
DPLL and a cold-start modeset. Nobody else on this list is doing that from
scratch on modern silicon. Haiku, with 25 years, has modesetting on Intel and
stops there. That makes `intel.c` genuinely novel work, and also explains why it
is hard.

---

## The realistic path to 3D

Not a GPU driver. `desktop-prior-art.md` covers why in detail — `i915.ko` is
11.2 MB against zlOS's 1.07 MB whole kernel, and everyone with GPU acceleration
got it by porting Mesa, which needs POSIX.

**The path is SerenityOS's:**

1. **`fb_clip`** — a scissor rectangle in `fb.c` (`archive/superseded/desktop-TODO.md` 0b). A
   rasterizer needs clipping before anything else. `fb3d.c` already has its own
   private clip; `fb.c` does not.
2. **Tiled rasterization** — 16×16 blocks, barycentric coordinates, edge
   derivatives. The documented approach, from Fabian Giesen's *"A trip through
   the Graphics Pipeline"* and Scratchapixel's rasterization series.
3. **SIMD** — `cpu.c` already detects SSE/SSE2/SSE3/SSSE3 and `HANDOFF.md`
   confirms SSE is **on** in the 64-bit build. **Nothing uses it yet.** The
   per-pixel blend loops in `fb.c` are the first customer, well before 3D.
4. **A depth buffer** — `fb3d.c` deliberately avoids one by culling back faces
   on a convex cube. Real scenes need it.
5. **Texture mapping**, then perspective-correct interpolation.

Each step is a real, bounded piece of work with published references. None is a
research problem.

**The order matters, though:** steps 1 and 3 are also what the *desktop* needs.
Clipping and SIMD blending pay off twice. Do those first regardless.

---

## Sources

- [Indie & Hobbyist Operating Systems 2026 Deep Dive](https://www.youngju.dev/blog/culture/2026-05-16-indie-hobby-operating-systems-2026-serenityos-ladybird-haiku-reactos-redoxos-theseus-plan-9-9front-arcanos-deep-dive.en)
- [SerenityOS ports list](https://ports.serenityos.net/) · [Half-Life port](https://ports.serenityos.net/halflife/dependencies) · [LibGL docs](https://www.mintlify.com/serenityOS/serenity/api/libgl)
- [SerenityOS — Wikipedia](https://en.wikipedia.org/wiki/SerenityOS) · [Grokipedia](https://grokipedia.com/page/SerenityOS)
- [Ladybird](https://ladybird.org/)
- [Essence OS — GitLab](https://gitlab.com/nakst/essence) · [architecture writeup](https://en.linuxadictos.com/essence-an-os-with-its-own-kernel-and-desktop-built-from-scratch.html)
- [Haiku Zink over RADV — Phoronix](https://www.phoronix.com/news/Haiku-OS-Zink-RADV) · [Haiku NVIDIA driver — OSnews](https://www.osnews.com/story/144097/haiku-gets-accelerated-nvidia-graphics-driver/)
- [Asterinas](https://asterinas.github.io/) · [Three alternative microkernels — The Register](https://www.theregister.com/software/2025/09/12/three-alternative-microkernels-show-devs-dont-need-linux/1428275)
- [Managarm end-of-2023 update](https://managarm.org/2023/12/31/end-of-year-update.html)
