> **AUDITED 2026-08-19 · PARTLY OPEN.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. The survey of ~15 hand-built OSes is still good and its citations still hold. The zlOS have/partial/none column is stale in one direction: eleven tracks landed nine of the things it marks absent, including three of the five it tells you not to build.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**


# Feature catalogue — everything the other OSes have

Compiled 2026-08-17 from research across ~15 hand-built operating systems.
Organised **by subsystem**, not by OS, because the useful question is *"what
features exist in this space, who has them, and does zlOS?"*

zlOS status: ✅ have · 🟡 partial · ❌ none · 📋 designed, unbuilt

---

## 1. Kernel and scheduling

| Feature | Who has it | zlOS |
|---|---|---|
| Preemptive multitasking | SerenityOS, Haiku, Redox, KolibriOS, TempleOS | 🟡 `sched.c` works, **desktop does not use it** |
| SMP / multi-core | most | 🟡 4 cores woken, **3 parked in `cli;hlt`** |
| Microkernel (drivers in userspace) | Redox, Managarm, Genode, Fuchsia | ❌ monolithic, ring 0 |
| Framekernel / Linux-ABI | Asterinas (230+ syscalls) | ❌ |
| Single address space, ring-0 only | TempleOS | ✅ same model |
| Isolation without an MMU (type system) | Theseus | ❌ |
| Runtime-swappable kernel modules | Theseus, Essence (on-demand loading) | ❌ |
| Async I/O throughout | Managarm | ❌ |
| Priority levels | Essence | ❌ |
| Per-window thread | Haiku (a window *creates* a thread) | ❌ |
| JIT compilation of system code | TempleOS (HolyC, JIT + AOT) | ❌ zl compiles ahead of time |

## 2. Memory

| Feature | Who | zlOS |
|---|---|---|
| Heap / dynamic allocation | everyone except zlOS | ❌ **by design** |
| Shared memory | Essence, SerenityOS, Genode | ❌ |
| Memory-mapped files | Essence | ❌ |
| Copy-on-write | Redox, Haiku | ❌ |
| Working-set balancing | Essence | ❌ |
| Multithreaded page zeroing | Essence | ❌ |
| Filesystem-independent cache manager | Essence | ❌ |
| 4-level paging | most 64-bit | ✅ |
| W^X memory | SerenityOS | ❌ |
| (K)ASLR | SerenityOS | ❌ |

## 3. Security

| Feature | Who | zlOS |
|---|---|---|
| `pledge` / `unveil` (OpenBSD-style) | SerenityOS | ❌ |
| Capability-based security | Genode, Fuchsia (unforgeable handles) | ❌ |
| Principle of least authority, per-component sandbox | Genode | ❌ |
| Hierarchical sub-sandboxes | Genode | ❌ |
| Process isolation for web content | SerenityOS | ❌ |
| Userspace driver isolation | Redox, Genode, Fuchsia | ❌ |
| OOM resistance | SerenityOS | ❌ |
| Modern TLS | SerenityOS (`LibTLS`) | ❌ |

**zlOS has no security model at all.** Everything is ring 0, one address space.
That is the same choice TempleOS made and it is defensible for a single-user
hobby OS — but it should be a *stated* choice, not an accident.

## 4. Filesystems

| Feature | Who | zlOS |
|---|---|---|
| A real on-disk filesystem | nearly all | ❌ RAM disk, 10 fixed slots |
| Journaling | Haiku (BFS) | ❌ |
| **Extended attributes** | Haiku (BFS) | ❌ |
| **Attribute indexing + fast queries** | Haiku — search by filetype-specific attributes | ❌ |
| Copy-on-write / transactional | Redox (RedoxFS, "ZFS features, more modular") | ❌ |
| **Packages mounted, never unarchived** | Haiku (PackageFS, read-only overlay) | ❌ |
| Virtual filesystems `/proc` `/dev` `/sys` | SerenityOS, Redox | ❌ |
| Read foreign filesystems | Essence (ext2/FAT/NTFS/ISO9660), KolibriOS (FAT12/16/32, ISO9660, NTFS read) | ❌ |
| File compression in the FS | TempleOS (RedSea) | ❌ |
| Content-addressed archival | Plan 9 (venti + fossil) | ❌ |
| NVMe / AHCI | Managarm, 9front, zlOS | ✅ NVMe |

## 5. IPC and system structure

| Feature | Who | zlOS |
|---|---|---|
| One uniform IPC for everything | Redox ("schemes"), Plan 9 (9P) | ❌ |
| **Everything is a file — truly** | Plan 9 / 9front | ❌ |
| **Per-process namespaces** | Plan 9 | ❌ |
| Network-transparent resources | Plan 9 (9P over the wire), `cpu` + `drawterm` | ❌ |
| Message-passing with automatic locking | Haiku (handler/looper) | ❌ |
| Handle/capability passing over channels | Fuchsia (Zircon) | ❌ |
| Out-of-process services | SerenityOS (`WindowServer`, `AudioServer`, `RequestServer`, `CrashServer`…) | ❌ |
| Component tree with delegated authority | Genode | ❌ |
| **Message plumbing between apps** | Plan 9 (`plumber`) | ❌ |

## 6. Graphics and display

| Feature | Who | zlOS |
|---|---|---|
| Framebuffer drawing | all | ✅ |
| **Native Intel modesetting** | Haiku (`intel_extreme`, solid ≤SandyBridge), **Redox (in progress)** | ✅ **Gen9.5, further than both** |
| VESA fallback | Haiku, many | 🟡 uses firmware mode |
| virtio-gpu | SerenityOS, Redox, zlOS | ✅ 2D only, virgl off |
| GPU 3D acceleration | Haiku (AMD/NVIDIA via Mesa), Genode (Intel Gen8+), Fuchsia | ❌ **and correctly ruled out** |
| **Software 3D / OpenGL** | SerenityOS (`LibGL` + `LibSoftGPU`, **Quake III**) | 🟡 `fb3d.c` — one shaded cube |
| **Software vector renderer** | Essence — resolution-independent, animated | ❌ |
| Anti-aliased text | SerenityOS, Haiku, Essence | ✅ |
| **Subpixel LCD text** | rare | ✅ **5-tap FIR** |
| **Gamma-correct linear-light blending** | rare | ✅ |
| Dithered gradients | — | ✅ |
| Alpha blending / translucency | Essence, Haiku | 🟡 blend exists, no per-window alpha |
| **Clipping / scissor** | all of them | ❌ **`fb.c` has none** — the keystone gap |
| Damage tracking | all of them | ❌ one dirty box |
| Multi-monitor / hotplug | Haiku, Managarm | ❌ |
| Screen rotation | Gen9 hardware supports it | ❌ |

## 7. Windowing and compositing

| Feature | Who | zlOS |
|---|---|---|
| Window server / compositor | SerenityOS (`WindowServer`), Haiku (`app_server`), Essence (**in kernel**), Redox (Orbital) | 📋 designed |
| Z-order, raise, focus | all | 🟡 hardcoded for 2 windows |
| Drag, resize, maximise, snap | all | 🟡 drag only, via sprite, ≤640×480 |
| **Tabbed windows** | Essence — several apps in one window | ❌ |
| **Docking windows by their title tabs** | Haiku — the yellow offset tab | ❌ |
| **Replicants — app fragments on the desktop** | Haiku, working without their parent app | ❌ |
| Window themes as **data** | SerenityOS (`LibGfx` theme object) | ❌ hardcoded |
| Virtual desktops / workspaces | Haiku, SerenityOS | ❌ |
| Activities overview | the v10 mockup | ❌ |
| Animation / transitions | Essence | ❌ |
| Per-window scaling | Essence (fully vector) | ❌ 1× or 2× only |

## 8. Toolkit and widgets

| Feature | Who | zlOS |
|---|---|---|
| GUI toolkit | SerenityOS (`LibGUI`), Haiku (BeAPI), Essence | 📋 `desktop-toolkit.md` |
| **Layout engine** | all of them | 📋 immediate-mode cursor |
| Buttons, sliders, toggles, scrollbars | all | ❌ |
| Text fields with selection | all | ❌ (shell owns text input) |
| Menus, context menus | all | 🟡 one hardcoded start menu |
| Dialogs / modals | all | ❌ |
| Drag and drop | Haiku, SerenityOS | ❌ |
| Clipboard | all | ❌ |

## 9. Text and fonts

| Feature | Who | zlOS |
|---|---|---|
| Real TrueType glyph rendering | SerenityOS, Haiku, Essence (FreeType) | ✅ via `gen_hd_font.py` |
| Complex text shaping | Essence (**Harfbuzz**), Haiku | ❌ |
| Multi-lingual / RTL | Essence, Haiku | ❌ |
| **Proportional text layout** | all of them | ❌ **monospace cell grid** |
| Multiple sizes and weights | all | ❌ two sizes, one weight |
| Text wrapping / measurement | all | ❌ |
| **UTF-8** | Plan 9 *invented* it | ❌ ASCII 32–126 |
| **Hypertext embedded in plain files** | TempleOS (**DolDoc** — links, images, 3D meshes in ASCII) | ❌ |

## 10. Input

| Feature | Who | zlOS |
|---|---|---|
| Keyboard with modifiers, repeat, events | all | ✅ **good** — `input.c` |
| USB HID keyboard | most | ✅ |
| PS/2 mouse | most | ✅ |
| **Laptop touchpad (I2C-HID)** | Haiku, Managarm | 🟡 **transport only, no decoder** |
| Multi-touch / gestures | Haiku | ❌ |
| Mouse events in the same queue as keys | all | ❌ `EV_MOUSE` declared, never pushed |
| Hotkeys / shortcuts | all | 🟡 single-letter commands |

## 11. Audio

| Feature | Who | zlOS |
|---|---|---|
| Audio stack with mixing | SerenityOS (`AudioServer`), Haiku, Essence | ❌ |
| **Multimedia as a first-class OS citizen** | Haiku — PCM, MIDI, video decode | ❌ |
| Synthesizer / DSP | SerenityOS (`LibDSP`, Piano app) | ❌ |
| AC'97 / HDA driver | KolibriOS | ❌ |
| PC speaker | TempleOS (single voice), zlOS | ✅ `beep()` |

## 12. Networking

**This whole table was ❌ until the `desktop/browser` track landed on `main` on
2026-08-19.** It is corrected in full rather than one row at a time, because a
`TCP/IP stack ❌` sitting directly above a browser that fetches off the real
internet is not a stale row, it is a contradiction.

| Feature | Who | zlOS |
|---|---|---|
| Ethernet driver | everyone | ✅ `virtio_net.c` (763 lines) |
| TCP/IP stack | SerenityOS, Haiku, Essence, KolibriOS, Managarm | ✅ `net.c` + `tcp.c` (1,352) — ARP, IPv4, ICMP, TCP with retransmit and backoff |
| DNS resolver | SerenityOS, KolibriOS | ✅ `dns.c` (433) — resolves by name; most of its gate is malicious input, not malformed |
| HTTP/TLS client | SerenityOS (`LibHTTP`, `LibTLS`) | 🟡 HTTP/1.0 in `http.c` (290). **No TLS**, so `https://` does not load |
| **A web browser** | SerenityOS (Ladybird), Haiku, KolibriOS, MenuetOS | ⚠️ **Bounded, and built.** Fetches `http://example.com/` **by name, off the real internet**, and renders it. ~4,657 lines across eight files. No JavaScript, no HTTPS — see §"Why a browser is in a category of its own" |
| Email / IMAP | SerenityOS | ❌ |
| Servers (ftp/http) | KolibriOS | ❌ |
| Network-transparent filesystem | Plan 9 (9P), `drawterm` | ❌ |
| WiFi | 9front, Haiku | ❌ — `../../plans/wireless-plan.md` is a plan, not a driver |
| Feature | Who | zlOS |
|---|---|---|
| TCP/IP stack | SerenityOS, Haiku, Essence, KolibriOS, Managarm | ❌ |
| HTTP/TLS client | SerenityOS (`LibHTTP`, `LibTLS`) | ❌ |
| **A web browser** | SerenityOS (Ladybird), Haiku, KolibriOS, MenuetOS | ❌ |
| Email / IMAP | SerenityOS | ❌ |
| Servers (ftp/http) | KolibriOS | ❌ |
| Network-transparent filesystem | Plan 9 (9P), `drawterm` | ❌ |
| WiFi | 9front, Haiku | ❌ |

## 13. Boot and platform

| Feature | Who | zlOS |
|---|---|---|
| **Boots three ways, no GRUB** | rare | ✅ BIOS multiboot + own 512-byte bootloader + native UEFI |
| UEFI | 9front, ArcaOS, Haiku | ✅ |
| ACPI | Managarm, ArcaOS | 🟡 APIC via ACPI |
| Multi-architecture | Managarm (x86-64/ARM64/RISC-V), Redox (+RISC-V), SerenityOS (in progress) | ❌ x86-64 only |
| **Fits on a floppy** | MenuetOS, KolibriOS (1.44 MB, *with* a GUI and browser) | 🟡 kernel is 1.07 MB |
| Live USB in one command | Redox | 🟡 `mkusb.sh` |
| Runs in a browser | ReactOS (v86) | ❌ |

## 14. Applications

| | Count / notable |
|---|---|
| **SerenityOS** | dozens built in + **300+ ports**. Browser, HackStudio (C++ IDE), Piano (synth), Mail, Spreadsheet, PixelPaint, Terminal, Solitaire, Minesweeper, 2048, chess, Game of Life, **Quake III, Half-Life** |
| **KolibriOS** | **150+ programs** — word processor, image viewer, graphic editor, web browser, media players, games, ftp/http/mp3 servers |
| **Haiku** | full native suite; Qt and GTK ported; legacy BeOS R5 binaries run |
| **Essence** | File Manager, Text Editor, IRC client, System Monitor; POSIX subsystem runs GCC and Busybox |
| **ReactOS** | Office 2007, Photoshop CS6, AutoCAD — badly |
| **ArcaOS** | 1990s OS/2 business apps; **deployed in real ATMs and industrial controllers** |
| **zlOS** | shell, System Monitor, About, and **7 demos that each take over the screen** |

## 15. Development tooling

| Feature | Who | zlOS |
|---|---|---|
| Self-hosting compiler | Redox, TempleOS, Haiku | 🟡 zl compiles zl on Linux, not on zlOS |
| On-system IDE | SerenityOS (HackStudio), TempleOS | ❌ |
| POSIX subsystem / GCC | Essence, Haiku | ❌ |
| **Host-side driver harness** | **rare — zlOS's `hosttest/`** | ✅ **and it is a real advantage** |
| Golden-transcript boot gate | — | ✅ `verify.sh` |
| Benchmark harness in-tree | — | ✅ `fbbench`, `gpu_fillrate` |
| Crash reporting service | SerenityOS (`CrashServer`) | ❌ |

## 16. Debugging, diagnostics and tooling on the OS itself

| Feature | Who | zlOS |
|---|---|---|
| **Kernel tracing buffer** — record events into reserved memory, filter and print them later in the kernel debugger | Haiku | ❌ |
| Userspace debugger with a debug API | Haiku (non-stop mode), SerenityOS | ❌ |
| Stack traces with symbols | SerenityOS, Haiku | ❌ |
| Crash reporting as a **system service** | SerenityOS (`CrashServer`) | ❌ |
| Kernel debugger you can drop into | Haiku, TempleOS | ❌ |
| Profiler | Haiku, SerenityOS | ❌ |
| Kernel memory dump on panic | Haiku | ❌ |
| Panic-and-reboot to a known state | embedded practice | 🟡 halts |
| **Serial console for debugging** | most | ✅ COM1, 115200 8N1 — and it is how everything gets debugged |

**zlOS's answer to all of this is different and worth naming:** it does not debug
*on* the machine, it debugs *from the host* — `hosttest/` runs kernel driver code
as a Linux program, `verify.sh` gates against a golden transcript, `fbbench`
times the renderer, `gpu_fillrate` measures the real GPU. **That is a genuinely
unusual and effective substitute** for an on-system debugger, and it exists
because a display driver gives no feedback when it is wrong.

## 17. Power, hardware and housekeeping

| Feature | Who | zlOS |
|---|---|---|
| Suspend / resume | Haiku (partial), ArcaOS | ❌ |
| CPU frequency / power states | Haiku | ❌ |
| Display power management (DC states, PSR) | Intel driver territory | 🟡 PSR read, DC states handled in the sequence |
| Battery status | Haiku, ArcaOS | ❌ |
| **Printing** | Haiku (*"vastly improved printing"* still on the wishlist), ArcaOS | ❌ |
| **Localisation** | Haiku — *"probably the single most requested feature"* on their wishlist | ❌ |
| Accessibility | weak everywhere in this space | ❌ |
| Time zones / RTC / NTP | most | 🟡 PIT ticks only |
| Package management | Haiku (PackageFS), Redox, SerenityOS ports | ❌ |
| Init / service supervision | SerenityOS (`LoginServer` et al), Redox | ❌ |
| Virtualisation / hypervisor | Managarm (Intel VT), Genode (microhypervisors, VMs in Sculpt) | ❌ |
| Containers / sandboxed apps | Genode (hierarchical sandboxes), Fuchsia | ❌ |

**Note the two Haiku wishlist items.** After 25 years, the most-requested feature
is **localisation** and printing is still called out as needing work. Those are
the things everyone defers, forever. Worth knowing before promising either.

## 18. The distinctive ideas — one per project

The single thing each project did that nobody copied:

| OS | The idea |
|---|---|
| **Plan 9** | **Everything is a file, truly** — plus per-process namespaces, so *your* `/dev` can be someone else's hardware over the network |
| **Haiku** | **Filesystem attributes you can index and query** — the FS is a database |
| **Haiku** | **Replicants** — drag a piece of an app onto the desktop, it keeps working alone |
| **TempleOS** | **DolDoc** — hypertext, images and 3D meshes embedded in ordinary text files, used for the shell *and* the editor |
| **Essence** | **Tabbed windows** — group apps by task inside one frame |
| **Redox** | **One IPC ("schemes") for literally everything** |
| **Genode** | **Hierarchical sandboxes** — a component sub-allocates its own authority |
| **Fuchsia** | **Unforgeable handles** — capabilities passed over channels |
| **Theseus** | **Isolation with no MMU**, enforced by the type system |
| **KolibriOS** | **95% assembly**, GUI + browser + TCP/IP, on a floppy |
| **SerenityOS** | **One repo, whole stack** — kernel to browser, no third-party code |

---

## What zlOS should actually take — ranked

Filtered by: cheap, fits no-heap/no-lists, and pays off more than once.

| # | Feature | From | Why |
|---|---|---|---|
| **1** | **Clip rectangle** | everyone | Not from anyone — it is the gap. **Three customers**: compositor, toolkit, 3D rasterizer. Two functions. |
| **2** | **Theme as data** | SerenityOS | One struct kills every hand-picked number. Tiny. |
| **3** | **Immediate-mode toolkit** | (forced by constraints) | The layer nothing else can be built on. `desktop-toolkit.md`. |
| **4** | **Tiled SIMD software rasterizer** | SerenityOS `LibSoftGPU` | The 3D goal, and SSE work speeds up `fb.c` first |
| **5** | **Mouse into the event queue** | everyone | `EV_MOUSE` already declared. Half a day. |
| **6** | **Tabbed windows** | Essence | Genuinely distinctive, and cheap once `wm.c` exists |
| **7** | **Vector/fractional UI scaling** | Essence | Fixes the 1×/2× cliff on the 1440p panel |
| **8** | **Per-window thread** | Haiku | `sched.c` already exists — but see the data-race warning in `desktop-plan.md` |
| **9** | **Proportional text** | everyone | Needs per-glyph advances from `gen_hd_font.py` |
| **10** | **Audio mixing** | Haiku, SerenityOS | Whole new subsystem. Later. |

**Explicitly not worth taking:** microkernel structure, capability security,
attribute-indexed filesystems.

~~network stacks, a browser~~ — **both were on this list and both were built,
on 2026-08-19.** The stated reason was that each "needs a heap, a filesystem, or
processes — all of which zlOS refuses by design." That reason was never checked
against an attempt. `net.c`, `tcp.c`, `dns.c` and `http.c` use the same static
arenas as the rest of the kernel and introduce no heap, no filesystem and no
process. The premise was wrong, and it was wrong in a way that only building the
thing could show.
attribute-indexed filesystems, network stacks, a browser. Each needs a heap, a
filesystem, or processes — all of which zlOS refuses by design.

---

## Why a browser is in a category of its own

> **CORRECTED 2026-08-19. zlOS has a browser.** It fetches
> `http://example.com/` by name, off the real internet, and renders it.
>
> Every number below is correct and **the conclusion did not follow from
> them.** The section measured the *maximal* version of the capability —
> Chromium, Ladybird, "the web works" — and reported that measurement as the
> capability. Chromium is unbounded. Ladybird is unbounded *for this project*.
> A browser is not, and the counter-example was four rows up in this same
> document the whole time: KolibriOS ships an OS, a GUI **and a browser** in
> 1.44 MB.
>
> The original text is kept below rather than deleted, because the reasoning in
> it is still correct about the thing it actually measured, and because this
> project has already had to publicly correct a "95% achievable" that was really
> 20% (`DECISIONS.md` #26). **The same standard applies in the optimistic
> direction.** A capability called impossible and then built has to have its row
> corrected, or the catalogue teaches the wrong lesson about what "unbounded"
> means — and the wrong lesson is expensive, because it is an argument for not
> starting.
>
> What it cost, measured: **~4,657 lines across eight files**, against a ~3,050
> estimate. What it does and refuses is in `kernel/docs/browser-status.md`.


It comes up because SerenityOS has one, so it looks reachable. It is not, and
the reason is worth understanding because **it is a different kind of hard from
everything else in this document.**

### The size

| | Lines of C++ |
|---|---|
| **Chromium** | **6,382,614** |
| **Ladybird** (from scratch, zero borrowed code) | **313,947** |
| **All hand-written zlOS kernel** | **11,374** |

**Ladybird alone is 27× the entire zlOS kernel. Chromium is 560×.**

### The team

Ladybird is the browser that grew out of SerenityOS and spun off in 2024–25:

- **50+ contributors**, and **7 full-time paid engineers**
- funded by Shopify and GitHub's co-founder
- alpha **mid-2026**, beta targeted **2027**
- **Andreas Kling worked professionally on WebKit before starting it**

So: a person who had already shipped a production browser engine, plus seven
funded engineers and fifty volunteers, is taking years to get to *alpha*.

### The part that actually makes it hard

Not the size. **There is no fixed target.**

W3C publishes specifications, but **none of them are binding**, and engines can
interpret them differently. Chromium has ~70% market share — so real websites are
built and tested against **Chromium's behaviour**, not against the spec.

**You are not implementing a standard. You are reverse-engineering someone
else's implementation, which changes continuously, on millions of sites that
each depend on a different corner of it.**

Kling's own words on it: *"I don't have a full picture of everything needed, but
it's fairly complicated."*

### Why that is different from everything else here

Every other feature in this catalogue has a **finish line**. A compositor is done
when windows composite. A rasterizer is done when triangles come out right. Link
training is done when the panel syncs. Hard, but bounded.

A browser is done when **the web works**, and the web is defined by whatever
Chrome did last Tuesday. There is no state in which you are finished.

**Compare it to the two other "big" things in these docs:**

| | Hard because | Bounded? |
|---|---|---|
| Intel display driver | subtle, no error feedback | **yes** — the panel lights or it does not |
| Mesa / 3D acceleration | enormous volume | **yes** — a spec exists and is stable |
| **A browser** | **the target moves and is defined by a competitor** | **no** — *for the maximal version. See the correction below.* |

**Original verdict: not "too hard". Unbounded.** If zlOS ever needs to show a
web page, the answer is to render a *subset* — a documentation viewer, a
Markdown or DolDoc-style hypertext reader like TempleOS's — and call it that
honestly, rather than aiming at "a browser" and never arriving.

### The corrected verdict

**The prescription in that paragraph was right, and it was followed.** What it
got wrong was filing the result under ❌ before anyone tried — because "render a
subset and call it that honestly" *is* a browser row, and it belongs in the
table as one.

The bounded version has a finish line, and it was named in advance rather than
discovered afterwards: **fetch one real URL over HTTP and render the HTML.**
That is done. It is 4,657 lines, every one of them gated by a host test that
needs no kernel and no boot.

| | |
|---|---|
| Fetches by name off the real internet | ✅ DNS → TCP → HTTP/1.0 |
| Renders HTML | ✅ parser, box model, inline flow, links, lists, `<pre>` |
| URL bar, Back, history | ✅ |
| HTTPS | ❌ no TLS |
| JavaScript | ❌ and not planned |
| CSS | ❌ beyond the built-in stylesheet |

**The general lesson, which is why this correction is written at length rather
than as an edit to one cell:** a capability whose *maximal* version is unbounded
is not thereby an unbounded capability. Chromium being unbounded says nothing
about whether one page can be fetched and drawn. Measuring the ceiling and
reporting it as the floor is how a catalogue talks a project out of work that
was three days away — and the cost of that error is invisible, because nothing
ever appears in a bug list saying "this was never attempted."

Where this reasoning still holds exactly as written: **do not aim at "a
browser"** in the sense of "the web works." That target does move, it is defined
by a competitor, and there is no state in which you are finished. Aim at a
sentence you can put a ✅ next to.
| **A browser** | **the target moves and is defined by a competitor** | **no** |

**Verdict: not "too hard". Unbounded.** If zlOS ever needs to show a web page,
the answer is to render a *subset* — a documentation viewer, a Markdown or
DolDoc-style hypertext reader like TempleOS's — and call it that honestly,
rather than aiming at "a browser" and never arriving.

*(TempleOS's DolDoc is the interesting precedent here: hypertext with links,
images and 3D meshes embedded in plain ASCII, used for both the shell and the
editor. A whole hypertext system with a finish line, because Terry Davis defined
the format himself.)*

---

## Sources

[SerenityOS](https://github.com/SerenityOS/serenity) · [apps](https://man.serenityos.org/man1/Applications.html) · [ports](https://ports.serenityos.net/) ·
[Haiku Quick Tour](https://www.haiku-os.org/docs/welcome/en/quicktour.html) · [Haiku packages](https://www.markround.com/blog/2023/02/13/haiku-package-management/) ·
[Essence](https://en.linuxadictos.com/essence-an-os-with-its-own-kernel-and-desktop-built-from-scratch.html) ·
[Redox goals](https://doc.redox-os.org/book/our-goals.html) · [Redox microkernel](https://deepwiki.com/redox-os/book/3.1-microkernel-design) ·
[TempleOS features](https://tinkeros.github.io/WbTempleOS/Doc/Features.html) · [TempleOS](https://en.wikipedia.org/wiki/TempleOS) ·
[KolibriOS](https://kolibrios.org/en) · [KolibriOS README](https://github.com/KolibriOS/kolibrios/blob/main/README.md) ·
[Genode](https://genode.org/about/index) · [Sculpt](https://genode.org/download/sculpt) ·
[Zircon handles](https://fuchsia.dev/fuchsia-src/concepts/kernel/handles) ·
[9front FQA](https://fqa.9front.org/fqa8.html) · [Plan 9 desktop guide](https://pspodcasting.net/dan/blog/2019/plan9_desktop.html) ·
[Managarm](https://managarm.org/2023/12/31/end-of-year-update.html) · [Asterinas](https://asterinas.github.io/) ·
[Ladybird](https://en.wikipedia.org/wiki/Ladybird_(web_browser)) · [Kling on starting Ladybird](https://awesomekling.github.io/Ladybird-a-new-cross-platform-browser-project/) ·
[Haiku kernel debugging](https://www.haiku-os.org/documents/dev/welcome_to_kernel_debugging_land/) · [Haiku feature wishlist](https://www.haiku-os.org/glass_elevator/rfc/feature_wishlist/) ·
[Haiku R1/beta5 release notes](https://www.haiku-os.org/get-haiku/r1beta5/release-notes/)
