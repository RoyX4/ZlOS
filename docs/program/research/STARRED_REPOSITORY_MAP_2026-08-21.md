# Starred repository map

The complete 33-repository audit suite, normalized matrix, independent refutation, and clean-room integration plan start at [`README.md`](README.md).

**Snapshot:** 2026-08-21. This is a map of all 33 repositories starred by
`RoyX4` at the snapshot. For every repository I inspected its current default
branch metadata, complete Git tree, root README, declared license and source
layout. The earlier source-level comparison of Brook, Astral, Zinnia, Skift and
RustOS remains in `OS_REPOSITORY_SURVEY_2026-08-20.md`; this document expands
the map to the whole starred set.

This is a **reference map**, not a claim that each project boots today or that
every README feature claim has been runtime-tested here. Source/build layouts
support the classification below; runtime maturity remains a separate question.

## Bottom line

The stars do not contain a project that makes zl or zlOS redundant. They do
contain a much better reference shelf than a random OSDEV search:

1. **Brook, Astral, banan-os, LemonOS, MollenOS and duckOS** are the useful
   broad-system references. They cover processes, ELF, VFS, drivers, desktops,
   ports and real application surfaces at different depths.
2. **Hyper** is the most immediately reusable *conceptual* boot reference: a
   small MIT C bootloader supporting BIOS, UEFI, multiple filesystems and
   deterministic boot-device resolution. Use it to audit boot contracts; do
   not add a bootloader detour while zlOS's boot evidence is still open.
3. **NexIOS** is unusually strong on deterministic test discipline: its tree
   has dedicated test classes, QEMU execution targets, recorded test history and
   an explicit microkernel migration. Its claimed numbers still need an
   independent run before they are treated as proof.
4. **ChittiOS** is not a conventional OS roadmap, but its central boundary is
   exactly right for agent systems: stochastic model output becomes a parsed,
   grammar-constrained plan and a deterministic, capability-checked executor
   performs effects. That reinforces the `intent -> contract -> bounded
   execution -> check -> receipt` design already chosen for the agent work.
5. **Sapphire GPU**, **asm.fm**, **Unix history**, and GNU freestanding headers
   are focused learning/reference projects, not zlOS substitutes. Sapphire is
   relevant only when the future CPU/GPU/hardware floor is real.

The near-term zlOS order does not move: prove physical boot/journal, measure
latency, make persistent storage ordinary, establish protected Ring 3, then
real networking and broader drivers/apps. Big desktops and ambitious designs
are evidence of what comes *after* those joins, not a reason to skip them.

## All starred repositories

| Repository | What the checked tree actually is | Best use | Boundary / caution |
|---|---|---|---|
| `robotman2412/sapphire-gpu` | Scala/Chisel hardware design for a simple 2D/possible-3D GPU; docs cover command and interface shape. CERN-OHL-P. | Future GPU pipeline, scan-out and rasterization study. | HDL project, not an OS or a display driver to drop into zlOS. |
| `UltraOS/Hyper` | MIT C bootloader; BIOS + UEFI, x86/i686/aarch64 ELF, FAT/ISO, GPT/MBR/PXE; has C/Python test tree and CI. | Boot protocol, image/layout and deterministic origin-partition design. | Replace nothing until zlOS needs a concrete loader change. |
| `zinnia-os/zinnia` | GPL-2.0 modular Rust kernel plus driver crates; the bootable distribution is a separate `bootstrap` repo. | Narrow driver-to-service boundaries, NVMe/xHCI/net module layout. | No license-compatible code copying; use the prior survey for source findings. |
| `rayanmargham/NyauxKC` | Small Rust monolithic Unix-like kernel rewrite: PMM/slab/VMM/RAMFS are present; syscalls, scheduler and ELF loader remain goals. 0BSD. | Tiny Rust PMM/VMM progression. | Starter-state only; no userspace proof. |
| `ProvableHQ/snarkOS` | Production-shaped Rust decentralized/ZK application node: CLI, accounts, BFT, ledger, router, sync, REST/CDN, metrics and tests. Apache-2.0. | Service decomposition, restart/peering/e2e test patterns—if zl ever builds a distributed system. | Not an operating system kernel; do not confuse its marketing name with OS engineering. |
| `dspinellis/unix-history-repo` | Historical early-Unix source archive, chiefly PDP-11 assembly plus provenance/license documents. | Learn the original small-system design and tool lineage. | Archaeology, not a current architecture or hardware reference. |
| `felipenlunkes/osdev-projects` | Curated OSDEV project index, not an operating system implementation. BSD-3-Clause. | Discovery list only. | README links are leads, not evidence. |
| `skift-org/skift` | LGPL-3.0 C++ system: Karm libraries/UI, Hjert capability microkernel, Hideo desktop, Vaev web; much of the broader system is external. | Later handles/capability/IPC service boundaries. | Earlier survey found no in-tree test surface and a capacity bug; not near-term zlOS work. |
| `SerenityOS/serenity` | Very large BSD-2 C++ OS, desktop, libraries and browser stack with an extensive test tree. | UI/application/library design, browser decomposition, project quality bar. | Far too broad to be a short roadmap; use a subsystem only when zlOS reaches it. |
| `marceldobehere/MaslOS-2` | GPL-3 C/C++ 64-bit BIOS hobby desktop with scheduler, ELF, IPC, GUI, apps and experimental audio; claims real-hardware boot. | Small desktop/app SDK and IPC examples. | Thin visible test structure and PS/2 dependence; GPL source boundary. |
| `LemonOSProject/LemonOS` | BSD-2 C++ desktop OS: kernel, GUI libraries, apps, services, ports and documentation; in-tree application/kernel tests plus CI. | C++ GUI/app split, IPC documentation, system image composition. | Broad fork/ports burden; architecture needs current runtime verification. |
| `hhuOS/hhuOS` | GPL-3 C++ teaching/hobby OS with kernel, user applications, documentation and a wide utility/app tree. | Small userspace/tool examples and readable component division. | License boundary; docs/apps do not prove the whole OS under real load. |
| `jezze/fudge` | MIT C message-based concurrent OS for x86, ARM and RISC-V, with architecture modules and low-level tests/demos. | Minimal message-passing/concurrency and multi-arch examples. | Much smaller surface than a protected desktop OS. |
| `alexdboxall/Banana-Operating-System` | 32-bit-focused C/C++ graphical OS, drivers/modules, installer, apps and substantial bundled assets/docs. | Legacy PC hardware, installer and visible desktop feature catalogue. | License is non-commercial rather than a normal source-reuse license; huge tree is asset-heavy. |
| `KingVentrix007/AthenX-2.0` | GPL-3 C OS with kernel, drivers, ELF, FS, network, UI, installer and userspace directories. | Broad subsystem inventory. | README makes scope claims; sparse automated-test evidence in the checked tree. |
| `ktauchathuranga/privilegeos` | A Linux/BusyBox image-builder with custom shell scripts, not an independently written kernel. Includes offline Windows login-bypass tooling. MIT repository wrapper plus upstream licenses. | Authorized recovery/pentest image construction only. | Not a zlOS reference; destructive/offline-access functions require explicit authorization and care. |
| `yashvi-soni-30/mYOS` | Tiny C/assembly i386 kernel with VGA shell, ATA PIO and a fixed-table custom filesystem. | Small persistent-storage and shell teaching example. | No paging, userspace, ELF, multitasking, networking or GUI yet. |
| `staycool1374-Ger/nexios` | GPL-3 freestanding C++20 hard-RT OS: O(1) scheduler, capability transition, Ring 3, VMM/PMM, VFS and structured QEMU test harness/docs. | Deterministic kernel test classes, capability migration contracts. | Tree includes generated Doxygen assets; verify claimed 881 tests independently before relying on it. |
| `chittios/chitti` | Apache-2 Rust bare-metal x86_64/aarch64 research OS whose native unit is an agent; includes model/runtime, capability ABI, apps, browser/media and tests. | Safe agent executor, authority reduction, plan/effect separation. | Research system, deliberately not POSIX; hardware README explicitly limits real-device support. |
| `Meulengracht/MollenOS` | GPL-3 large C/C++ OS source tree: boot, kernel, runtime, modules, services, apps and test infrastructure; much source volume comes from compiler/runtime imports. | System-service layout and toolchain/runtime integration. | Separate project-owned code from imported compiler/runtime corpus before judging scope; GPL boundary. |
| `CorruptedByCPU/Cyjon` | Small AMD64 assembly multitasking kernel with its companion Fern-Night project as a submodule. | Readable pure-assembly scheduling/kernel mechanics. | Educational/readability-oriented; very small system surface. |
| `cppsever/MineBios` | Tiny 16-bit assembly Minecraft-like experiment that runs in BIOS. Apache-2. | BIOS graphics/input constraints and tiny-program fun. | Not an OS architecture reference. |
| `viralcode/vib-OS` | C/assembly multi-architecture OS with kernel, libc, drivers, GUI/apps and embedded media/DOOM assets; ARM64 is the stated primary working path. | ARM64/x86 build separation, app/GUI showcase. | README claims must be separated from real-hardware evidence; large assets inflate the repository. |
| `whispem/asm.fm` | MIT x86_64 Linux NASM programs that synthesize valid WAV through syscalls only. | Freestanding output, fixed-point/audio math, syscall-only discipline. | Linux user-mode program, not bare metal or a sound driver. |
| `brainboxdotcc/mission-control` | Laravel app that leases isolated QEMU overlay sessions, proxies noVNC/WebSocket and has a meaningful PHP test suite. | Safely publishing a browser-accessible zlOS demo later. | Requires a real Linux/QEMU host and secure lifecycle policy; not a static site or OS. |
| `Bananymous/banan-os` | BSD-2 C++ x86_64/i686 OS with Ring 3, SMP, COW, ELF dynamic linking, VFS/ext2/FAT, NVMe/ATA, xHCI, NICs, TCP and GUI; has user test targets. | One of the strongest new direct zlOS references: storage/USB/NIC/process/GUI breadth. | TCP is labelled partial/buggy; source use requires its attribution/license terms. |
| `UnmappedStack/TacOS` | MPL-2 C/assembly Unix-like OS: VM, VFS, scheduler, frame allocator, libc/userspace and DOOM; README claims laptop boot. | Compact protected-process/VFS/ELF path. | Explicitly hobby/incomplete with known bugs; MPL obligations matter for source reuse. |
| `byteduck/duckOS` | GPL-3 C++ Unix-like desktop with WM/toolkit, TCP/UDP, E1000, AC97, dynamic linker, ptrace/profiler, apps and ports. | Desktop services, debugging/profiling, app libraries. | Development is on hold and SMP remains a future item. |
| `IanNorris/Brook` | MIT C++ x86_64 OS with UEFI, SMP, VM, Linux-ABI userspace, networking, loadable drivers, Wayland, Nix userspace and host/QEMU tests. | Primary full-system reference for process/VMM/VFS/diagnostic and test discipline. | Broad system: use a narrow subsystem at a time; source/runtime findings are detailed in the prior survey. |
| `Mathewnd/Astral` | MIT C/assembly x86_64 OS with strong hardware/POSIX/ports depth. | First C reference for AHCI/HDA/xHCI/NIC/package work. | Prior survey: broad source outcome, weak visible test/CI proof. |
| `ierwarf/rustos` | MIT Rust microkernel/hybrid research OS with services, drivers, formal models, contracts, tests and compatibility experiments. | Lifecycle contracts, fault/restart/adversarial tests. | Source/model/CI evidence does not close physical device/runtime gates. |
| `osdev0/freestanding-c-hdrs-gnu` | GNU freestanding headers across architectures. | Bootstrap/header reference for freestanding C support. | Headers only; keep GCC runtime/license obligations clear. |
| `Ellicode/protOS-kernel` | MIT x86-64 Limine C teaching kernel with processes, ELF, VFS/TAR/devfs, IPC/shared memory, PTYs, a user-space WM, terminal and shell. | Compact vertical-subsystem and desktop-seam map. | Documented build failed; broken ELF validation, raw user pointers, null syscall, UAFs and ambient sharing make it a negative correctness reference. |

## Direct reference assignments

| zlOS need | First source to open | Why |
|---|---|---|
| BIOS/UEFI boot image or boot-device semantics | Hyper | Small, focused boot contract with an actual test tree. |
| Real NVMe/ATA, xHCI and Ethernet breadth | banan-os, then Astral | Banan has all three in one C++ tree; Astral is the more mature hardware/POSIX reference. |
| Protected 64-bit userspace, ELF and VFS | Brook, then TacOS/banan-os | Brook has the deepest proof discipline; the other two are more compact reading. |
| App framework / Files / desktop | LemonOS, duckOS, SerenityOS | Different useful scales: practical local C++ stack, small OS services, then high-quality large-system design. |
| Kernel test harnesses | Brook, NexIOS, RustOS | Host tests, QEMU class execution, then contract/fault thinking. |
| Future capability/IPC design | Skift, NexIOS, RustOS | Read only after Ring 3 and stable handles exist. |
| Agent-native operating-system ideas | Chitti | Keep the deterministic executor and capability boundary; do not adopt its non-POSIX scope blindly. |
| Future hardware/GPU/CPU education | Sapphire GPU, asm.fm, Unix history | Focused, bounded projects with clear lessons. |
| Browser-hosted demo deployment | Mission Control | Per-session overlay and lease/reaper shape is directly useful. |

## License rule

Studying interfaces, tests and architecture is safe. Copying is a different
decision. The starred set mixes permissive (MIT/BSD/Apache/0BSD), weak/strong
copyleft (MPL/LGPL/GPL/AGPL), CERN-OHL hardware terms, non-commercial terms and
repositories with no asserted SPDX license. zl/zlOS has no top-level license in
this checkout, so do not copy external implementation code until its own
distribution terms and the source project's exact obligations are settled.

## What this does not prove

Repository metadata, trees and documentation cannot prove a hardware driver,
desktop or browser works today. For a concrete subsystem, the required next
step is narrow: checkout that project at a recorded commit, read its relevant
source/tests, build it, and—where realistic—run its existing QEMU or hardware
gate. Do not promote a README checkbox into zlOS planning evidence without that
step.
