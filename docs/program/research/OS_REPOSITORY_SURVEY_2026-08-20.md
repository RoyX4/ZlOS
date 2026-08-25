# OS repository survey — what matters to zlOS

**Snapshot:** 2026-08-20. The default branch, repository tree, recent commits,
build files, architecture documents and representative source were inspected for
all eight projects. README claims are treated as claims unless the tree, tests or
build structure backs them. The Lunaris compiler was also built locally; its test
suite passed and its machine backend produced a valid relocatable x86-64 ELF object
from the kernel's Lua source.

## Bottom line

There is no hidden project here that invalidates zlOS's design or makes its current
work redundant.

- **Brook is the best complete-system implementation reference.** It is the one to
  study for Linux ABI compatibility, processes, virtual memory, host-side kernel
  tests, profiling, loadable drivers and a serious userspace.
- **Astral is the best real-hardware and POSIX reference.** Its driver coverage and
  ported software make it the most useful source when zlOS reaches AHCI, HDA, xHCI
  depth, real NICs and a distribution layer.
- **Lunaris is the most relevant idea to the zl thesis.** Its kernel is written in a
  Lua dialect whose compiler directly emits relocatable x86-64 ELF objects. It is
  tiny and immature, but it proves the shape of the eventual zl-to-kernel object
  path.
- **Zinnia is the cleanest modular kernel/driver reference.** Its Rust crates split
  the kernel from NVMe, virtio, ext2, xHCI, NIC and GPU drivers cleanly.
- **Skift and RustOS are architecture references, not roadmaps to adopt.** Their
  capability, IPC and service-boundary ideas become relevant after zlOS has real
  64-bit processes.
- **EquinoxOS is a discontinued feature showcase. EquantOS is its much earlier,
  cleaner restart.** Both contain useful local ideas, but neither outranks Brook or
  Astral as an engineering reference.

The current zlOS order remains right: physical boot evidence, measured latency,
durable storage, 64-bit Ring 3, real networking, then broader drivers and apps.
These repositories mostly demonstrate how expensive it is to skip those joins.

## Which projects have their own language?

Only **Lunaris** has its own OS source language among the eight external
repositories. Even that needs a precise label: it is a custom, statically typed
Lua dialect with a compiler written in C++, not a fully original or self-hosting
language. zl remains much further ahead as a language project.

| Project | Kernel source language | Own language? | Own compiler? | Native output? | Self-hosting? | Exact gap |
|---|---|---|---|---|---|---|
| Brook | C++ and assembly | No | No | Clang/LLVM builds it | No | Complete OS work, but no language layer of its own |
| Astral | C and assembly | No | No | GCC/assembler toolchain | Partial ecosystem only | GCC is ported and full self-hosting is a goal, but C and GCC are not Astral inventions |
| Zinnia | Rust | No | No | Rust/LLVM toolchain | No | Custom kernel and drivers, standard external language/compiler |
| Skift | C++20 modules and assembly | No | No | External C++ toolchain | No | Karm is a library and CuteKit is a build/package tool, not a programming language |
| RustOS | Rust | No | No | Rust/LLVM toolchain | No | Extensive OS architecture, no language/compiler project |
| EquinoxOS | C and assembly | No | No | GCC/binutils | No | QuickJS runs JavaScript in an app; JavaScript and QuickJS are external |
| EquantOS | C and assembly | No | No | GCC/binutils | No | `Eon` is only a possible future graphical DSL in the roadmap |
| **Lunaris** | **Lunaris Lua dialect** | **Yes, a custom dialect** | **Yes, written in C++** | **Yes: direct x86-64 relocatable ELF** | **No** | Compiler works, but the language and OS are tiny, not self-hosting, and have no declared license |
| **zlOS** | **zl plus freestanding C/assembly support** | **Yes, original language** | **Yes** | **Yes: LLVM and own x86-64 paths** | **Yes at the language fixpoint** | The OS build is not yet an end-to-end self-hosted zl-only toolchain; important kernel support still comes from C/assembly/linker tooling |

## Full gap matrix

This matrix uses a serious general-purpose, self-hosting desktop OS as the target,
not merely “boots and draws a window.” `✅` means substantial current evidence,
`🟡` means partial, early, VM-only or split into another repository, `❌` means
absent, and `⏸` means the project has stopped.

| Capability | Brook | Astral | Zinnia | Skift | RustOS | EquinoxOS | EquantOS | Lunaris | zlOS |
|---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| Own programming language | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | 🟡 | ✅ |
| Language/compiler self-hosting | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| Direct own machine-code backend | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ | ✅ |
| x86-64 boot path | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Own UEFI loader/application | ✅ | 🟡 | 🟡 | ✅ | 🟡 | ❌ | ❌ | ❌ | ✅ |
| Proved physical-machine boot | 🟡 | ✅ | 🟡 | 🟡 | ❌ | 🟡 | 🟡 | ❌ | ❌ |
| SMP/multicore is load-bearing | ✅ | ✅ | ✅ | 🟡 | 🟡 | ❌ | ❌ | ❌ | 🟡 |
| Virtual memory and page protection | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | 🟡 |
| Isolated Ring-3 processes | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ on 64-bit path |
| Process-aware scheduler | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | 🟡 |
| Safe user-copy and fault containment | ✅ | ✅ | ✅ | ✅ | ✅ | 🟡 | 🟡 | ❌ | ❌ |
| ELF userspace loader | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | 🟡 32-bit proof only |
| Dynamic linker/libc environment | ✅ | ✅ | 🟡 | ❌ | 🟡 | 🟡 | 🟡 broken fresh clone | ❌ | ❌ |
| Broad POSIX/Linux ABI | ✅ | ✅ | ✅ | ❌ | 🟡 | ❌ custom ABI | 🟡 | ❌ | ❌ |
| Stable native application ABI | ✅ | ✅ | 🟡 | ✅ | 🟡 | 🟡 | 🟡 | ❌ | ❌ |
| IPC/capability handles | ✅ | ✅ | ✅ | ✅ | ✅ | 🟡 | ❌ | ❌ | ❌ |
| Durable VFS/filesystem | ✅ | ✅ | ✅ | 🟡 | 🟡 | ✅ | 🟡 | ❌ | 🟡 zlfs not yet universal |
| NVMe/AHCI breadth | ❌ | ✅ | 🟡 NVMe only | ❌ | 🟡 through DVM | ❌ ATA PIO | 🟡 NVMe | ❌ | 🟡 NVMe, block seam open |
| xHCI/USB HID/storage | ✅ | ✅ | ✅ | ❌ | 🟡 DVM path incomplete | 🟡 | ❌ | ❌ | 🟡 physical write unproved |
| Real physical NIC driver | ❌ virtio | ✅ rtl8169 | ✅ Intel igc | ❌ | ❌ usable physical path | ✅ RTL8139 | ❌ | ❌ | ❌ virtio only |
| TCP/IP and sockets | ✅ | ✅ | ✅ | ❌ | 🟡 | 🟡 incomplete TCP | ❌ | ❌ | 🟡 QEMU path works |
| TLS/web stack | ✅ via ports | 🟡 not clearly evidenced | ❌ kernel only | 🟡 external Vaev | ❌ | ✅ | ❌ | ❌ | ✅ bounded QEMU browser |
| Audio | ✅ HDA | ✅ HDA | ❌ | ❌ | ❌ | ✅ AC97 | ❌ | ❌ | ❌ |
| Modern GPU/DRM acceleration | ❌ framebuffer/BGA | 🟡 framebuffer/X.org | 🟡 virtio GPU/DRM | 🟡 | 🟡 failed physical gate | ❌ VESA | ❌ | ❌ | 🟡 Intel primitives not load-bearing |
| Desktop compositor/GUI | ✅ | ✅ | ❌ kernel repo only | ✅ external Hideo stack | 🟡 | ✅ | ❌ | ❌ terminal only | ✅ |
| Browser | ✅ NetSurf, partial Ladybird | ❓ not evidenced in inspected tree | ❌ | 🟡 external Vaev | ❌ | ✅ custom | ❌ | ❌ | 🟡 bounded custom browser |
| Package manager/software ecosystem | ✅ Nix | ✅ xbps/ports | ❌ kernel repo only | 🟡 CuteKit/project packages | 🟡 manifests, not ecosystem | ❌ | ❌ | ❌ | ❌ |
| Can rebuild the OS from inside itself | ❌ | 🟡 stated goal | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| Host unit tests | ✅ | ❌ visible suite | ❌ visible runtime suite | ❌ | ✅ | 🟡 | ❌ | ✅ compiler only | ✅ |
| QEMU integration gates | ✅ | 🟡 manual | 🟡 build CI | 🟡 | ✅ | 🟡 | 🟡 manual | ❌ | ✅ |
| CI on default branch | ❌ | ❌ | ✅ | ✅ | ✅ | ✅ | ❌ | ❌ | ✅ in zl-linux |
| Active maintenance | ✅ | ✅ | ✅ | ✅ | ✅ | ⏸ | ✅ | 🟡 only 3 commits | ✅ |

## Detailed missing work by project

| Project | Language gap | Kernel/process gap | Hardware/storage gap | Network/media gap | Desktop/ecosystem gap | Verification/project gap |
|---|---|---|---|---|---|---|
| Brook | No own language or compiler | Linux compatibility is still incomplete; large apps remain partial and hardening is ongoing | No visible AHCI/NVMe or serious physical GPU path; many drivers are QEMU/virtio-oriented | No physical NIC driver was found; Wayland and large-app compatibility still have edge cases | Depends deliberately on musl/Nix/Linux binaries rather than a native language ecosystem | Good tests, but no GitHub Actions workflow and physical-hardware evidence is weaker than VM evidence |
| Astral | Uses C/GCC; “fully self hosting” is still a goal | Needs the installer and final self-host loop; exact POSIX conformance is not demonstrated | Broadest hardware tree here, but modern accelerated GPU coverage is still limited | No browser was evidenced in the inspected tree | Ports and xbps exist, but completing installation and native rebuild are still open | No visible automated test suite or CI is the main engineering gap |
| Zinnia | Uses Rust/LLVM | Kernel interfaces are broad, but this repository contains no complete userspace/distribution | AHCI and audio are absent; GPU support is virtio-focused; real-hardware proof is unclear | TCP/IP exists, but end-to-end desktop/network product proof lives outside this repo | No desktop, browser or package ecosystem in this repository | CI builds/lints, but almost no in-tree runtime tests |
| Skift | Uses C++; Karm and CuteKit are not languages | No POSIX layer; current scheduler/SMP and process maturity are below Brook/Astral | Little storage, USB, NIC or audio depth in this mirror | No complete network stack here; browser/UI are external packages | System is split across Codeberg dependencies, so this mirror cannot prove the complete product | No in-tree tests; a spot-check found the capability-slot free-space bug described below |
| RustOS | Uses Rust/LLVM | Compatibility and capability design are ahead of fully proven runtime paths | DVM device data plane and physical GPU/block/NIC gates remain incomplete or failed | Network/display depend on those incomplete transports; audio and browser are absent | Manifests and services exist, but not a mature application/package ecosystem | Strong tests/formal work, but one contributor and much source/model evidence still lacks target-hardware proof |
| EquinoxOS | Uses C; embedded JavaScript is external QuickJS | No SMP; custom ABI is small and tightly coupled; isolation/hardening is incomplete | ATA PIO and older virtual/legacy devices dominate; storage and USB depth lag | TCP state/retransmission was still roadmap work; browser can panic the OS | Visually broad but closely coupled kernel, SDK and desktop | Explicitly discontinued; thin tests make regression risk high |
| EquantOS | Uses C; proposed Eon DSL does not exist | Partial Linux ABI includes placeholders; fresh clone lacks musl CRT objects | USB, audio, graphics, installer and broader drivers are missing; filesystems are experimental | No networking stack or browser | No GUI, package manager or general userspace | No CI or project-owned regression suite; early alpha |
| Lunaris | Dialect is not self-hosting and is still a small Lua subset | No PMM/VMM, interrupts, scheduler, processes, syscalls, isolation or userspace | No filesystem, storage, PCI, USB or real drivers beyond framebuffer handoff | No network, audio, GPU acceleration or browser | Only a framebuffer terminal; no applications, packages or self-host environment | Three commits, compiler-only tests, no OS integration gate and no license |
| zlOS | zl is self-hosting, but the OS toolchain still relies on C/assembly/linker stages | Needs load-bearing 64-bit Ring 3, PML4 per process, safe user-copy, process fault containment, stable app ABI, handles and IPC | Must unify NVMe/USB behind the block seam, prove the physical USB write path, and make Intel modeset/GPU work load-bearing without losing GOP fallback | Needs a real NIC, DHCP/reliable TCP on hardware and audio; QEMU virtio/TLS/browser proof is not physical-network proof | Needs user applications outside the kernel, package/app distribution, permissions/directories and eventually in-OS rebuilding | Highest immediate gap is still the first ThinkPad boot/journal transcript; several paths have strong host/QEMU proof but no physical proof |

## Evidence table

Repository metadata below came from the GitHub API on the snapshot date. Commit
counts describe the current default branch, not total historical work across every
branch.

| Project | Shape | Default-branch evidence | Honest maturity call | License |
|---|---|---:|---|---|
| [Brook](https://github.com/IanNorris/Brook) | C++ monolithic kernel, UEFI, SMP, Linux ABI, Wayland, Nix userspace | 1,260 commits, 7 contributors; last commit 2026-07-21 | Most complete and best-tested direct reference here | MIT |
| [Astral](https://github.com/Mathewnd/Astral) | C kernel, SMP, POSIX, broad real hardware, packaged userspace | 1,663 commits, 12 contributors; last commit 2026-08-18 | Mature hobby OS with exceptional hardware and ports breadth; weak visible automated-test story | MIT |
| [Zinnia](https://github.com/zinnia-os/zinnia) | Modular Rust Unix-like kernel and driver modules | 620 commits, 5 contributors; last commit 2026-08-07 | Serious kernel tree with CI; boot distribution lives elsewhere and the repo exposes little runtime-test evidence | GPL-2.0 |
| [Skift](https://github.com/skift-org/skift) | C++ capability microkernel plus userspace services; UI/browser packages externalised | 2,000 commits, 16 contributors; last commit 2026-07-08 | Long-running and architecturally interesting, but this GitHub mirror is only part of the system and has no in-tree test files | LGPL-3.0 |
| [RustOS](https://github.com/ierwarf/rustos) | Rust hybrid microkernel, userspace services, Linux driver VM, ELF/PE compatibility | 285 commits, 1 contributor; last commit 2026-08-19 | Substantial source, tests and formal contracts, but its own docs still mark key DVM/device paths and hardware gates incomplete | MIT |
| [EquinoxOS](https://github.com/Equinox-Collective/EquinoxOS) | C monolithic kernel, custom ABI, desktop, network, audio, browser | 654 commits, 4 contributors; last commit 2026-07-10 | Broad working demo surface, but README explicitly says development is discontinued | GPL-2.0 |
| [EquantOS](https://github.com/Equinox-Collective/EquantOS) | C monolithic kernel, Linux-like ABI, musl bring-up, NVMe/VFS | 162 commits, 2 contributors; last commit 2026-08-19 | Active early alpha; honest docs list placeholder syscalls and a fresh-clone CRT packaging failure | GPL-2.0 |
| [Lunaris](https://github.com/Diamantino-Op/Lunaris) | Lua-derived systems language compiler plus kernel written in that language | 3 commits, 1 contributor; last commit 2026-04-24 | Compiler works; OS is a framebuffer-terminal starter, not yet a general OS | No license declared |

## 1. Brook — the primary implementation reference

Brook is far ahead of the others as a complete machine. It has a custom UEFI
bootloader, SMP with per-CPU queues, virtual memory and copy-on-write work,
processes/signals/futexes, ext2 and FAT, xHCI, HDA, networking, a compositor and
Wayland server. Its Linux x86-64 syscall compatibility is deep enough to run musl,
bash, BusyBox, TCC, Go programs, GTK software, NetSurf, Quake 2 and partial large
desktop applications. Nix supplies existing userspace rather than forcing the
author to port every program manually.

The most valuable parts for zlOS are less flashy than the screenshots:

1. Kernel subsystems are testable on the host. Brook has host-native tests for
   TCP, locks, file descriptors, loopback, scheduling policy and ext2, plus
   freestanding/QEMU tests for memory, VFS and devices. This is the natural next
   step for zlOS's existing host-tested Intel and xHCI work.
2. Diagnostics are first-class: sampling profiler, watchdog, symbolized panic
   path, QR crash payload, debug socket and explicit boot self-tests.
3. Driver modules have a small registration surface and are separately loadable.
4. Linux compatibility is treated as an ecosystem multiplier, not as the kernel's
   identity.

Do not imitate Brook's full scope now. The Linux ABI, Wayland and Nix path only pay
after processes, memory protection, files and synchronization are load-bearing.
Brook's source tree also shows the real price: roughly nine hundred non-vendored
C/C++/assembly source files, before imported userspace.

## 2. Astral — hardware, POSIX and distribution depth

Astral is the other major reference. It is a C x86-64 OS with SMP, networking,
multiple users, package management and ports of X.org, Wine, GCC, bash, Quake,
vim and more. Its kernel tree contains separate implementations for NVMe, AHCI,
virtio-block, xHCI, USB HID/hubs, HDA, rtl8169, virtio-net, TCP/UDP, ext2, FAT,
signals, futexes, sockets, process control and a large POSIX-style syscall set.

For zlOS, Astral should be the first source opened when implementing:

- Intel HDA or a software audio service;
- a real Ethernet driver and the boundary between NIC and network stack;
- deeper xHCI hub/HID behavior;
- AHCI beside the existing NVMe path;
- package/image construction and self-hosting userspace.

The weakness is verification visibility. The default branch has no GitHub Actions
workflow and no obvious in-tree test suite. It has strong outcome evidence through
ports and hardware support, but Brook is the better model for preventing regressions.

## 3. Lunaris — the zl-to-kernel compiler idea in miniature

Lunaris is not a competitor in OS maturity. It has three commits and its kernel
currently initializes a Limine framebuffer, prints to a terminal and halts.

It is still unusually relevant. Its language adds exactly the freestanding forms
a source language needs to describe a kernel:

- `data ... section ".limine_requests"` for linker-visible objects;
- `packed struct` for hardware and protocol layouts;
- `asm function` for symbols implemented in assembly;
- pointer types and typed field/index access;
- `require` for source modules.

Its default backend directly writes x86-64 instructions, ELF sections, symbols and
relocations. Local verification on this snapshot produced a valid `ET_REL` object
with `.text`, `.limine_requests*`, `.rodata`, `.rela.text`, `.symtab` and string
tables, and the compiler's CTest target passed.

That is the important lesson: zl does not need a second language or a new compiler
architecture. It eventually needs its existing native backend to grow an
**ELF-relocatable kernel mode** with named sections, external/assembly symbols,
relocations, packed layout types and an explicit freestanding ABI. Do this after
the current 64-bit process/storage spine, not as a detour now.

Lunaris declares no license. Read it for concepts; do not copy its source.

## 4. Zinnia — modular kernel and drivers

Zinnia is a Rust Unix-like kernel split into a kernel crate and separate driver
crates for NVMe, virtio block/network/GPU, ext2, Intel `igc` networking and xHCI.
The kernel itself has clear boundaries for VFS, sockets, epoll/eventfd/timerfd,
DRM, input, USB, process/signals, memory and architecture-specific code. It also
has x86-64 and partial RISC-V structure, and CI runs formatting, lint and builds.

What zlOS should borrow conceptually is the dependency direction:

```text
driver -> narrow block/net/input/display interface -> kernel service
```

That is directly applicable when zlOS unifies NVMe and USB storage behind the
planned block seam and makes virtio-net one implementation of `net_link`.

Do not copy the code casually: Zinnia is GPL-2.0, uses Rust-specific ownership and
async patterns, and the bootable distribution is a separate repository. Its value
here is module shape and interface placement.

## 5. Skift — capability and service-boundary reference

Skift's `Hjert` kernel uses capability domains and kernel objects for tasks,
address spaces, VM objects, port I/O, IRQs, channels, pipes, jobs and listeners.
Capabilities can be transferred through IPC; device, filesystem, input, power and
shell policy live in userspace `strata-*` services. This is a good model for a
future zlOS handle table because authority is an object reference, not a magic
integer accepted everywhere.

Use the idea, not the current tree as unquestioned authority. Most of Skift's
Karm UI, desktop and browser stack is fetched from separate Codeberg repositories,
the GitHub mirror has no in-tree tests, and a source spot-check found a concrete
capacity bug: `Domain::_availableUnlocked()` counts occupied slots, while channel
receive uses it as the number of free capability slots. That can reject transfers
into an empty domain and admit them with the wrong capacity assumption.

This becomes relevant only in zlOS Phase 4, when process handles and IPC exist.
Before that, a capability rewrite would create architecture with no consumer.

## 6. RustOS — contracts, isolation and compatibility experiments

RustOS is the most ambitious design: a Rust hybrid microkernel, policy services,
Linux ELF and Windows PE loaders, a Wayland UI server, and an isolated Linux driver
VM connected through validated shared-memory transports. It includes many kernel
tests, fault injection, benchmarks and formal models.

The best thing to take is its definition-of-done discipline:

- bind authority to exact owner and lifetime;
- bound every queue, mapping and wait;
- validate cross-domain records before use;
- test cancellation, crash, restart and stale-message paths;
- keep formal models separate from runtime proof.

Its own documentation is appropriately blunt that major paths remain incomplete:
the DVM lacks a general usable device data plane in the basic smoke path, physical
GPU/device admission gates remain open, and some display work is source/model proof
rather than hardware proof. One contributor and 285 commits have produced a great
deal of architecture and assurance machinery very quickly. Treat it as a source of
contracts and adversarial tests, not as proof that this topology is the shortest
route to a finished OS.

For zlOS this is Phase-4-and-later material. The current persistent journal and
three-layer driver evidence rule already point in the same direction with much
less machinery.

## 7. EquinoxOS — discontinued, broad, useful in pieces

EquinoxOS demonstrates a traditional short route from a C monolithic kernel to a
visible desktop: Limine, Ring 3 ELF apps, a custom `int 0x80` ABI, VFS, FAT/ext2,
RTL8139 networking, AC97, shared memory, a compositing GUI, an immediate-mode UI
library, DOOM and a custom browser using BearSSL and QuickJS.

Useful pieces to study are its immediate-mode application SDK, small custom syscall
surface and bounded browser decomposition. The project README now explicitly says
it is discontinued for an unknown period, despite an old “active development” badge.
Its kernel and UI remain fairly tightly coupled and its test surface is thin. Use
it as a feature prototype catalogue, not as zlOS's foundation.

## 8. EquantOS — honest clean restart

EquantOS returns to a smaller base: Limine, PMM/VMM/heap, Ring 3, an ELF loader,
round-robin scheduling, a VFS, RAMFS, experimental FAT/ext2, NVMe and a partial
Linux x86-64 syscall ABI for musl. Its README is unusually honest about what is
placeholder behavior and what does not build from a fresh clone: ignored musl CRT
objects are missing, `fstat` and `munmap` are stubs, and networking, USB, graphics,
audio, a package manager and an installer are absent.

That makes it a useful small reference for the exact transition zlOS is approaching:
one protected process, one ELF loader, a minimal libc-facing ABI, then general
userspace. It is not mature enough to displace Brook's process implementation or
Astral's hardware work.

## What this changes for zlOS

### Keep the current execution roadmap

Nothing found justifies moving work ahead of the current joins:

1. Boot the ThinkPad and recover the persistent `ZLLOG` transcript.
2. Measure end-to-end input-to-present latency and fix the measured owner.
3. Put NVMe and USB storage behind one block interface and make zlfs the ordinary
   named-file service.
4. Build real 64-bit Ring 3, per-process page tables, validated user copies and
   failure containment.
5. Prove a real wired/USB network path, then deepen TCP and the browser.
6. Add broader driver and application depth.

Brook, Astral and Zinnia all show that processes, files and stable subsystem seams
are what let later breadth compound. EquinoxOS shows what happens when breadth is
added on a small custom surface and the architecture later needs a restart.

### Add one explicit decision after the first native zl user process

Once Phase 4 can run, fault and kill a tiny zl program without taking down the
desktop, decide between:

1. **Native zl ABI first, Linux compatibility later** — recommended. Keep the
   kernel syscall surface small and purpose-built; add a userspace compatibility
   service or translation layer only when a real port needs it.
2. **Linux syscall ABI as the native ABI** — faster access to musl and existing
   binaries, but imports Linux's large semantic contract into the kernel before
   zl's own process model has settled.

Brook and Astral prove the ecosystem payoff of Linux/POSIX compatibility. They
also prove its scale. zlOS should first make its own language a first-class user
program, then buy compatibility deliberately rather than accidentally.

## Reference map by upcoming subsystem

| zlOS work | First reference | Second reference | Why |
|---|---|---|---|
| Host-testable kernel components and diagnostics | Brook | RustOS | Brook has practical host/QEMU tests; RustOS has adversarial lifecycle contracts |
| Block interface, NVMe/AHCI | Astral | Zinnia | Mature C hardware code, then clean driver boundary |
| xHCI, hubs, HID, USB storage | Astral | Brook | Broad hardware structure plus tested loadable module |
| Real Ethernet and TCP | Astral | Brook | rtl8169/virtio plus complete sockets/TCP behavior |
| Processes, signals, user copies | Brook | Zinnia | Proven Linux userspace surface plus cleaner module split |
| Capability handles and IPC | Skift | RustOS | Small object-capability core plus hardened cross-domain contracts |
| zl-native kernel object output | Lunaris | zl's own `nativegen` | Concrete ELF object/relocation shape without changing the language thesis |
| UI/app SDK | EquinoxOS | Skift | Small immediate-mode ABI versus service-based cohesive desktop |
| Distribution and self-hosting | Astral | Brook | Packaged ports versus Linux ABI plus Nix binary reuse |

## Licensing boundary

Architecture, interfaces and algorithms can be studied across all eight. Source
reuse is a separate decision:

- Brook, Astral and RustOS are MIT: reuse is practical with their copyright and
  license notice preserved.
- EquinoxOS, EquantOS and Zinnia are GPL-2.0: copying code can impose GPL terms on
  the resulting combined work.
- Skift is LGPL-3.0: obligations depend on how code is incorporated, but direct
  kernel source copying is not a no-strings shortcut.
- Lunaris has no declared license: default copyright applies. Do not copy its
  implementation without permission.

zl and zlOS currently have no top-level license file, so the project's intended
distribution terms should be settled before importing any external source.
