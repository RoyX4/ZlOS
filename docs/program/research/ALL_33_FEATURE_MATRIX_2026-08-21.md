# All 33 starred repositories: normalized feature and donor matrix

Date: 2026-08-21

Inputs: the mature-OS, architecture-OS, focused-repository, ProtOS, refutation, and master-taxonomy audits in this directory.

Purpose: compare unlike repositories without promoting documentation, filenames, build recipes, or checked-in artifacts into runtime proof.

## How to read this artifact

This is a synthesis of the source-grounded dossiers, not a new README survey. The repository ledger contains every audited repository exactly once. The subsystem matrices then rank the strongest clean-room idea donors; they do not authorize code reuse and they do not imply that every named mechanism worked locally.

Evidence is normalized across the three source reports:

- **`C` — claimed:** documentation, screenshots, release notes, roadmaps, or checked-in logs say it exists.
- **`S` — source:** substantive implementation source was inspected.
- **`R` — reachable:** source is connected to the normal build, boot, route, module, or service graph. This corresponds to `B`/build-reachable in the mature report.
- **`T` — covered:** a focused test, model, CI scenario, or harness targets the behavior; it was not necessarily run during this audit.
- **`CI` — gated in CI:** CI actually invokes the relevant build or test path. `CI-build` means image/build only, not a runtime assertion.
- **`V` — locally verified:** this audit successfully built or executed the stated path.
- **`F` — falsified or failed:** a local check failed, a claimed feature was disconnected/stubbed, or a gate was shown to be false-green.
- **`—` — absent/not applicable:** no meaningful implementation was established for this repository's role.

Evidence is per statement, not a repository-wide certification. `S/R/T` never means “works on hardware.” A fresh ISO proves the build path, not a healthy desktop. A formal model proves properties of the model unless conformance is separately established.

## Repository ledger — exactly 33 audited snapshots

| # | Repository | Kind | Strongest connected surface | Evidence ceiling in this audit | Best clean-room contribution | Dominant limit or claim boundary |
|---:|---|---|---|---|---|---|
| 1 | `IanNorris/Brook` | x86-64 UEFI desktop OS | SMP/process/VM, VFS/ext2, TCP, modular VirtIO/xHCI/HDA, Wayland-like desktop | `C/S/R/T`; host CTest configured, freestanding fixtures build-only; no local run | Host harnesses around real kernel code; candid driver hazard ledger | Roughly 60 unsafe user dereferences, permissionless signals and unauthenticated compositor control; hardware seams unproved |
| 2 | `Mathewnd/Astral` | x86-64 POSIX-like OS/distribution | Declarative init, SMP/VM/VFS, NVMe/AHCI/xHCI/HDA, TCP/UDP, broad ports | `C/S/R`; no native tests/CI/local run | Ordering metadata and centralized deny-first authorization listeners | Init has no cycle/failure/timeout/rollback/teardown semantics; hardware recovery untested |
| 3 | `Bananymous/banan-os` | x86-64/i686 Unix-like desktop OS | Processes/VM, ext2/FAT, storage/USB/net/audio, SHM GUI, dynamic loader, ports | `C/S/R/T`; no automated runner/CI/local run | Pinned/checksummed port recipes; honest maturity list; damage GUI | Critical `setreuid(-1,0)` privilege escalation plus username-only login; no unload; partial TCP/USB |
| 4 | `LemonOSProject/LemonOS` | x86-64 Unix-like desktop OS | VM/process, ext2/FAT, AHCI/NVMe, network, IDL IPC, SHM damage compositor | `C/S/R/T/CI-build`; test failures cannot gate | Generated IPC schema and service-registry ideas only | Deterministic endpoint-credit mismatch, fork credential corruption, fake-success `setgid`, permissionless kill, false-green tests |
| 5 | `SerenityOS/serenity` | multiarchitecture full desktop OS | Deep kernel, services, libraries, browser, apps, ports, host/target test architecture | `C/S/R/T/CI`; multiarch build/QEMU boot, x86-64 target tests only; no current run | Monotonic pledge/unveil, parser isolation, service boundaries, post-test filesystem check | Non-x86 runner skips target tests and reports zero failures; scale remains expensive |
| 6 | `Meulengracht/MollenOS` | microkernel-shaped service OS | Kernel mechanisms plus filed/deviced/processd/usbd/netd, generated contracts, VaFS apps | `C/S/R/T/CI`; no local run; service tests unreachable/runner false-green | Kernel/service boundary, provider schemas, immutable app images | Invalid `ModuleDelete(moduleBuffer)` duplicate-cache path; ownership/revocation/lifecycle incomplete |
| 7 | `hhuOS/hhuOS` | educational i386 OS | Readable boot composition, VFS/FAT/ISO, UDP, framebuffer apps/games/emulators | `C/S/R/T/CI-build`; no local run | Explicit, teachable system composition root | Ring 3 lacks authority: arbitrary user MMIO mapping, mount, kill, shutdown and raw syscall pointers |
| 8 | `byteduck/duckOS` | i686 GUI Unix-like OS | VM, ext2, TCP/UDP/UNIX, dynamic linking, Pond GUI, Quack audio, River RPC | `C/S/R/T/CI-build`; tests not CI-run | Small service bus, SHM audio queue, separate loader | SHM ownership bypass, SafePointer continuation, one-past VMO fault, signal-0/group defects, weak TCP timers |
| 9 | `chittios/chitti` | agent-native dual-architecture kernel | Grammar-constrained tools, caps/taint/skills/audit, broad media/net/UI services | `C/S/R/T`; no local run | Typed plans and grant-attenuation questions | Built-in MAC mislabeled Ed25519, foreign ingestion unfinished, partial install/uninstall, no-op kill, manifest authority resurrection risk |
| 10 | `staycool1374-Ger/nexios` | x86-64 monolithic RTOS evolving toward caps | Fixed pools, deadline schedulers, CSpace/revoke, IPC, VFS, drivers, large kernel tests | `C/S/R/T/V-build/F-run`; kernel/ISO built, no local test summary | Boundedness budgets and deterministic snapshot/rewind tests | Not a derivation-tree exemplar: source COPY unchecked, broad revoke truncates; RT claims and runtime remain unproved |
| 11 | `ierwarf/rustos` | contract-heavy microkernel/service OS | Transactional VM, bounded IPC, service brokers, formal/fuzz/conformance evidence | `C/S/R/T/CI`; no local run | Intent-to-contract-to-receipt discipline; failed evidence retained | Very high complexity; formal models are not implementation/physical proof |
| 12 | `zinnia-os/zinnia` | compact Rust Unix kernel and drivers | Processes/signals, VFS/epoll family, TCP/UDP, narrow NVMe/virtio/xHCI/ext2 crates | `C/S/R/T/CI-compile/V-build`; local release workspace built, no runtime gate | Narrow driver modules and explicit kernel-to-driver dependency | uACPI hooks stub/fake success; malformed ELF panic and nontransactional VM ranges; no boot proof |
| 13 | `skift-org/skift` | object-capability kernel with service strata | Domains/tasks/VMOs/channels/jobs plus device/FS/input/power/compositor strata | `C/S/R/T`; much framework source fetched externally | Typed object API and send-side rollback design | Receive transfer broken by inverted capacity; VM map not failure-atomic; incomplete local product surface |
| 14 | `jezze/fudge` | mailbox/actor-style small OS | Fixed pools, typed mailbox events, ring-3 per-task page tables, userspace services | `C/S/R/T-weak`; build-only Travis | Reactive typed services plus real CR3/ring-3 mechanism | Ambient IDs/shared kernel regions, destructive oversize receive, unsafe ELF, static limits, no verification |
| 15 | `UnmappedStack/TacOS` | small x86-64 Unix desktop | SMP, fork/exec, local sockets, PTY, TempFS/USTAR, WM/terminal/Doom | `C/S/R`; manual probes only | Traceable end-to-end smoke image and simple compositor seam | Negative syscall indexes, ELF write inversion and `filesz > memsz` overflow; weak usercopy and false-green lint |
| 16 | `rayanmargham/NyauxKC` | rewrite-stage Rust kernel | x86-64/RISC-V kernel threads, PMM/slab/VMM, RAMFS/USTAR, partial virtio-GPU | `C/S/R/T-weak`; no local run | Honest scope and small VFS/per-CPU queue decomposition | Reversed PCI tuple; USTAR imports header as payload; IOMMU passthrough is not isolation |
| 17 | `CorruptedByCPU/Cyjon` | pure-assembly x86-64 SMP kernel + external userspace | Paging/tasks/syscalls/IPC/ELF plus Fern-Night VFS image and framebuffer UI | `C/S/R/F`; wrapper masks build failures, no tests | Readable instruction-level boot/state layouts | Raw output pointers plus global halt fault path; magic-only ELF, false-green build, restrictive license |
| 18 | `robotman2412/sapphire-gpu` | SpinalHDL hardware protocol laboratory | Serial command/payload engines, DMA, SPI, framebuffer/texture/ILI9341 blocks | `S/T`; simulations not locally run; incomplete top level `F` | Capability descriptors and bounded ready/valid device streams | No connected rasterizer/shader/compute/top-level GPU; empty/incomplete blocks |
| 19 | `UltraOS/Hyper` | compact cross-firmware bootloader | BIOS/UEFI, i686/amd64/AArch64, FAT/ISO, GPT/MBR, PXE/TFTP, Ultra handover | `S/R/T/V`; 33 BIOS tests passed, 75 cases collected | Assertion-kernel boot matrix and exact-origin semantics | Local execution is BIOS-focused; AArch64 handover exists but was not locally run; chainload/APM gaps remain |
| 20 | `ProvableHQ/snarkOS` | distributed blockchain node/control plane | Typed roles, bounded routing, tiered sync, BFT/ledger, REST, plugins, checkpoints | `S/R/T`; no local build/runtime | Role-specific exposure, lifecycle, telemetry, checkpoint and plugin operations | Not an OS; large dependency graph; reload/sync/specialized consensus gaps |
| 21 | `dspinellis/unix-history-repo` | synthesized software-history dataset | Provenance-preserving commit graph, branches/tags/snapshots and blame continuity | `C/S/V`; fully unshallowed, 849,161 commits, 8,037 merges and refs/objects measured | Preserve architecture evolution and evidence provenance as history | Historical rights vary; snapshot/line/contributor totals remain upstream claims |
| 22 | `felipenlunkes/osdev-projects` | curated repository index | Family/language/architecture/activity/license discovery tables | `S`; no schema/test/CI | Normalize discovery leads into validated, timestamped data | Manual stale/malformed/duplicate rows; links are not feature evidence |
| 23 | `marceldobehere/MaslOS-2` | x86-64 graphical hobby OS | Tasks/ELF/IPC, stdio conversations, AHCI/custom FS/SAF, desktop/apps/games/audio | `C/S/R/V-build`; fresh ISO, no boot assertion | Conversation IDs, stdio-over-IPC, restartable user-facing modules | Unsafe/raw-pointer C++, no robust automated suite, custom FS integrity gaps |
| 24 | `alexdboxall/Banana-Operating-System` | large legacy i386 graphical OS | Processes/ELF drivers, VFS/storage, GUI/apps/packages/installer, partial net/audio | `C/S/R`; checked artifacts only, bespoke toolchain unavailable | Broad desktop packaging/installer and modular device organization | Unsafe kernel-module/user-pointer boundary, no current CI/runtime, non-commercial license |
| 25 | `KingVentrix007/AthenX-2.0` | 32-bit GRUB hobby OS | Shell/TUI, VFS/FAT/custom FS, IDE/AHCI, partial ELF/syscalls/UDP/TFTP | `S/R/V-weak`; dry graph only, checked ELF | Approachable vertical source organization and manuals | Credential validator accepts everyone; multitasking stub; raw pointers/format-string issue; no tests |
| 26 | `ktauchathuranga/privilegeos` | Linux/BusyBox rescue distribution | Hybrid BIOS/UEFI rescue image recipe, broad Linux config, recovery scripts | `S/R/V-syntax/F`; claimed image absent | Signed conservative rescue media with paired backup/restore and audit log | Passwordless root/bypass tooling, unverified downloads, no artifact/runtime gate |
| 27 | `yashvi-soni-30/mYOS` | tiny 32-bit persistence demo | VGA shell, PS/2, ATA PIO, one serialized fixed filesystem structure | `S/R`; no tests | Minimal cold-storage concept demonstrator | No process/VM protection; unsafe disk format and polling; no license |
| 28 | `cppsever/MineBios` | 16-bit bootable tile game | FAT12 chain loader, BIOS retry loop, mode-13h fixed-window game | `S/R`; no tests | Tiny staged boot and bounded render window | Hardcoded FAT geometry; game wrapper, not protected OS architecture |
| 29 | `viralcode/vib-OS` | ARM64 graphical demo kernel | Linked ARM64 kernel with GUI/media and many nominal subsystem sources | `S/R/V-build/F`; kernel compiled, runtime unavailable | Evidence tuples that expose disconnected, simulated, or stubbed features | Major claim drift: fake runtimes/network UI, no real exec/clone/SMP/sandbox/ext4/APFS/net |
| 30 | `whispem/asm.fm` | standalone x86-64 assembly DSP lessons | 25 integer/fixed-point WAV generators covering synthesis, filters and effects | `S/R/V`; all 25 build/run/WAV checks passed | Deterministic DSP/test-tone behaviors | Offline duplicated examples; no real-time device/mixer/streaming tests |
| 31 | `brainboxdotcc/mission-control` | web/QEMU demo control plane | Slot/lease allocation, COW overlays, deadlines, noVNC, logs, admin/reaper | `S/R/T/V-syntax`; dependency tests unavailable | Ephemeral browser demo lifecycle | Nontransactional allocation, weak lease/VNC auth, unsafe termination/resource/egress controls |
| 32 | `osdev0/freestanding-c-hdrs-gnu` | generated toolchain-header snapshot | Six architectures, pinned GCC/binutils, digest verification, CI regeneration | `S/R/T/V/F`; core smoke passed, clang atomic failed | Generated ABI inputs with exact provenance and compiler matrix | Compiler headers are not libc; clang compatibility patch is incomplete |
| 33 | `Ellicode/protOS-kernel` | compact x86-64 graphical teaching kernel | Ring 3/ELF/syscalls, VFS/TAR/devfs/PTY/AIO, IPC/SHM, user WM/terminal/shell | `C/S/R/V-kernel/F`; documented/full builds failed, kernel target linked only with audit workarounds | End-to-end vertical slices and a user-space WM/shared-surface seam | Broken ELF validation; raw user pointers; null syscall; UAFs; ambient/unbounded IPC and sharing |

## Normalized all-repository capability grid

Cells show the strongest *meaningful* evidence for that area, not the maximum evidence anywhere in the repository. `partial` means the dossier established an important disconnect, stub, or narrow subset.

| # | Boot/platform | Memory/execution | IPC/authority | Storage/FS | Network | Drivers/media | UI/apps | Proof character |
|---:|---|---|---|---|---|---|---|---|
| 1 | UEFI x86-64 `S/R` | SMP, VM/COW, ELF `S/R/T` | protocol/syscall policy partial | ext2/FAT/VFS `S/R/T` | TCP/IP `S/R/T` | VirtIO/xHCI/HDA `S/R` | compositor/apps `S/R` | strong host tests, no local/CI proof |
| 2 | x86-64 image/order graph `S/R` | SMP/VM/ELF `S/R` | POSIX credentials + auth listeners `S/R` | ext2/FAT/tmp/dev `S/R` | TCP/UDP/DHCP `S/R` | NVMe/AHCI/xHCI/HDA `S/R` | ports/X surface `C/R` | broad source; init lacks cycles/failure semantics; no native tests |
| 3 | x86-64/i686 `S/R` | SMP/process/VM `S/R` | Unix permissions with critical `setreuid` bypass | ext2/FAT/VFS `S/R` | partial TCP `S/R/T` | NVMe/AHCI/xHCI/audio `S/R` | SHM GUI/apps `S/R` | test sources, no runner |
| 4 | x86-64 `S/R/CI-build` | process/VM; fork corrupts credentials | IDL endpoints `S/R`, broken credit ownership, permissionless kill | ext2/FAT/VFS `S/R` | TCP/local `S/R` | NVMe/AHCI; xHCI/HDA stub | damage compositor `S/R` | false-green tests |
| 5 | x86-64/AArch64/RISC-V `S/R/CI` | SMP/process/VM `S/R/T` | IPC + pledge/unveil `S/R/T` | many FS/storage `S/R/T` | TCP/IP `S/R/T/CI` | broad USB/storage/audio/display `S/R` | full service/app stack `S/R` | multiarch build/boot; x86-64 target tests only; post-test FS gate |
| 6 | x86 families/service boot `S/R` | kernel handles/VM/scheduling `S/R/T` | generated providers `S/R` | filed/VaFS `S/R` | unsupported/limited | modular devices, reach gaps | apps external/packaged | invalid PE-cache destructor; service tests unreachable/runner false-green |
| 7 | i386 `S/R/CI-build` | Ring-3 address spaces `S/R` | arbitrary MMIO/mount/kill/power and raw pointers | FAT/ISO/VFS `S/R` | UDP only `S/R` | IDE/AHCI/PS2/SB `S/R` | framebuffer apps/games `S/R` | build-only; privilege transition is not authority |
| 8 | i686, AArch64 WIP `S/R` | VM/ELF/dynamic link; one-past fault bug | River RPC; SHM ownership bypass; signal branch defects | ext2/VFS `S/R` | incomplete TCP `S/R` | E1000/AC97 `S/R` | Pond/Quack/apps `S/R` | kernel registry, CI build only; SafePointer continues after failure |
| 9 | x86-64/AArch64 `S/R/T` | task spaces, special tenants `S/R` | caps/taint/grants/audit `S/R/T` | store/FAT/ext2 `S/R/T` | TCP/TLS/HTTP etc. `S/R/T` | virtio/USB/media/audio `S/R/T` | compositor/browser/agents `S/R/T` | broad scenarios, huge TCB |
| 10 | x86-64, other scaffolds | fixed pools/VMM/deadlines `S/R/T` | CSpace/IPC `S/R/T`; source COPY/revoke defects | VFS/FAT/tmp/dev/proc `S/R/T` | UDP subset `S/R/T` | AHCI/virtio/framebuffer `S/R/T` | console only | local kernel/ISO build; corrected runner emitted no test summary |
| 11 | x86-64 contracts/AP boot `S/R/T` | atomic VM/W^X/scheduler `S/R/T` | bounded endpoints/handles `S/R/T` | provider services `S/R/T` | AF_UNIX/AF_INET `S/R/T` | Linux-VM/provider architecture | bounded Wayland server `S/R/T` | formal/fuzz/mutation plus candid failed gates |
| 12 | x86-64, partial RISC-V `S/R` | VM/tasks/signals/ELF `S/R` with loader/VM gaps | conventional Unix | VFS/ext2/tmp/dev `S/R` | full conventional stack `S/R` | narrow NVMe/virtio/xHCI crates | DRM/evdev, no desktop | local release compile; no boot; uACPI fake-success hooks |
| 13 | EFI x86-64 `S/R/T` | VMO/task/address-space with non-atomic map | object caps/channels `S/R`; receive transfer broken | bootfs/userspace FS `S/R` | stubs `F` | device strata `S/R` | external UI framework `C/R` | fetched-dependency tests; local product boundary incomplete |
| 14 | x86; partial ARM/RISC-V | fixed pools plus real per-task CR3/ring 3 `S/R` | universal mailboxes, ambient IDs | message FS, ext2 read-only | simple/net modules partial | modular storage/video/audio | userspace WM/apps | build-only; oversized receive destroys message; unsafe ELF |
| 15 | Limine x86-64 BIOS/UEFI `S/R` | SMP/fork/exec with loader overflows | local ring-buffer sockets; negative syscall index | USTAR/TempFS `S/R` | local only | framebuffer/basic PCI, broken NVMe | WM/PTY/shell/Doom `S/R` | no automated gate |
| 16 | Limine x86-64/RISC-V partial | kernel threads/VMM `S/R` | none | RAMFS; USTAR writes header bytes as contents | none | reversed PCI BAR; passthrough IOMMU; GPU query only | console only | boot assertions only |
| 17 | Limine x86-64 `S/R` | SMP/page maps; magic-only ELF | fixed TTL IPC; raw output pointers | memory VFS image | none | PS2/serial/framebuffer | external Fern-Night WM/apps | no tests; masked build; user fault can globally halt |
| 18 | hardware block, not OS | DMA/stream state `S` | serial control protocol `S` | byte DMA only | — | SPI/display/pixel blocks `S` | — | simulations `T`, no local toolchain |
| 19 | BIOS/UEFI, 3 architectures `S/R/V` | handover only | Ultra boot contract | FAT/ISO/GPT/MBR `S/R/V` | PXE/TFTP `S/R/T` | firmware/storage interfaces | — | best verified boot matrix |
| 20 | host process | typed node roles `S/R` | role/exposure/plugin policy `S/R` | RocksDB/checkpoints `S/R` | router/CDN/P2P/BFT `S/R/T` | — | CLI/REST/metrics | large test/CI corpus, no local build |
| 21 | historical corpus | evolution data | provenance/authorship graph | historical source snapshots | historical | historical | — | full graph measured; secondary scale claims remain upstream |
| 22 | metadata only | — | — | — | — | architecture/language metadata | discovery tables | no schema/link validation |
| 23 | x86-64 Limine `S/R/V-build` | tasks/ELF/scheduler `S/R` | queues/conversations `S/R` | AHCI/MRAPS/MRAFS/SAF | limited | AC97/audio/storage | full desktop/apps/games | fresh ISO, no boot assertion |
| 24 | i386 checked image `C/S/R` | process/VM/ELF modules `S/R` | weak syscall/privilege boundary | FAT/exFAT/ISO/storage `S/R` | partial | broad legacy hardware/audio | desktop/packages/installer | old artifacts, unavailable toolchain |
| 25 | GRUB i386 `S/R` | ELF/syscall/process partial | raw syscall pointers | FAT/custom FS/VFS | UDP/TFTP subset | IDE/AHCI/PS2/SB/e1000 source | shell/TUI/apps | dry graph only |
| 26 | Linux GRUB hybrid recipe `S/R` | Linux/BusyBox | passwordless root | Linux/NTFS rescue | Linux config | Linux driver configuration | console tools | syntax only; image absent `F` |
| 27 | multiboot i386 `S/R` | kernel shell, no VM/process | — | fixed raw FS + ATA PIO | — | PS2/VGA/ATA | VGA shell | no tests |
| 28 | BIOS FAT12 `S/R` | 16-bit game loop | — | loader chain read only | — | BIOS video/disk | tile game | no tests |
| 29 | ARM64 kernel `V-build` | process/syscalls partial `F` | sandbox disconnected `F` | RamFS; ext4/APFS false claims `F` | incomplete/simulated `F` | many linked stubs/partials | coherent GUI/demo surface | compile-only CI; no boot probe |
| 30 | Linux process ABI | standalone DSP programs | — | WAV stdout | — | offline audio algorithms | — | all 25 outputs locally validated `V` |
| 31 | host QEMU launcher | lease/VM process state | session token model incomplete | COW overlays | guest user NAT | QEMU/KVM/OVMF/noVNC | browser demo | syntax `V`; feature tests unavailable |
| 32 | six compiler targets | compiler ABI inputs | — | — | — | generated intrinsic headers | — | cross-target smoke `V`; atomics compatibility `F` |
| 33 | Limine x86-64 `S/R` | RR processes/ELF `S/R/F` | named IPC/SHM `S/R/F` | VFS/TAR/devfs/PTY `S/R` | none | PIC/PIT/PS2/PCI enumerate | user WM/terminal/shell `S/R` | normal/full build `F`; workaround kernel target linked |

## Subsystem donor matrices

“Best donor” means the strongest pattern to specify clean-room behavior for zl/zlOS. A runner-up can be better for a narrower case. The tradeoff is part of the recommendation, not a footnote.

### Boot and handover

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **Hyper** | `S/R/T/V` | Tiny assertion kernel; firmware × architecture × medium × filesystem × partition/path × fragmentation matrix; exact-origin boot identity | BIOS was locally proven, not the full UEFI/AArch64 matrix; it is a loader, not an OS boot-service architecture |
| 2 | Astral | `S/R` | Registered init nodes with explicit dependency tokens | Add timeouts, critical/optional policy, rollback and teardown; source breadth has no native test proof |
| 3 | hhuOS / Cyjon | `S/R` | Human-readable complete composition root and explicit state layout | Do not retain one giant failure domain or Cyjon's failure-masking wrapper |
| 4 | SerenityOS | `S/R/T/CI` | Multi-firmware/multiarchitecture image and boot-test infrastructure | Large toolchain/build surface; architecture parity is incomplete |

**zlOS decision:** define a generated boot DAG, then validate its handover with a Hyper-style assertion image. Each node emits a versioned receipt and has bounded start/stop semantics.

### Kernel/service architecture

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **MollenOS** | `S/R/T/CI` | Minimal kernel mechanisms plus typed filed/deviced/processd/usbd/netd-style providers | Current ownership, isolation, revocation and process lifecycle are incomplete; close those gaps before moving policy |
| 2 | RustOS | `S/R/T/CI` | Explicit authority/ownership maps and fail-closed provider contracts | Too complex to transplant whole; use contracts selectively |
| 3 | Zinnia | `S/R` | Narrow driver crates and a small conventional Unix kernel-to-driver ABI | Mostly kernel/drivers; no full userspace distribution or capability system |
| 4 | Fudge | `S/R` | Reactive services assembled from a small message mechanism | Fixed pools/weak isolation/polling require redesign |

**zlOS decision:** kernel owns address spaces, threads, handles, IPC/wait, interrupts and DMA resource authority. Filesystem, network, audio, compositor, package, session, parser and agent policy become supervised services as their contracts mature.

### Memory and virtual-address semantics

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **RustOS** | `S/R/T` | Preflight entire mapping span, checked arithmetic, W^X, failure-atomic commit, exact ownership | Models/contracts do not prove physical paths; architecture complexity is high |
| 2 | NexiOS | `S/R/T` | Owner-tagged fixed pools, deterministic exhaustion, cap-authorized frame mappings | Fixed pools trade flexibility for predictability; SMP/admission maturity is incomplete |
| 3 | SerenityOS | `S/R/T/CI` | VM fault results explicitly become recovery, signal, coredump or kernel failure | Bigger implementation surface; not all architectures have equal MMU semantics |
| 4 | Brook | `S/R/T` | Host-test real allocators/VMM/COW logic with sanitizers | Host shims cannot reproduce privileged page-table/TLB behavior |

**zlOS decision:** every VM operation uses `validate full span -> reserve resources -> mutate -> commit/rollback`; executable pages default non-writable, and exhaustion/error paths are host- and kernel-image tested.

### Scheduler and process model

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **NexiOS** | `S/R/T` | Explicit queue bounds, deadlines/WCET budgets, replenishment state, deterministic snapshots | Do not call zlOS real-time until admission, IRQ latency, I/O waits and SMP are bounded and measured |
| 2 | RustOS | `S/R/T` | Per-CPU continuation ownership, bounded priority donation, separated scheduling classes | More state/invariants to verify |
| 3 | Brook | `S/R/T` | Scheduler policies cross a real dynamic-module boundary and run in host tests | Pluggability adds ABI/lifecycle hazards; debug/teardown incomplete |
| 4 | TacOS | `S/R` | Small traceable fork/exec/PTY path for end-to-end smoke | Global round-robin, polling and weak validation are not production patterns |

**zlOS decision:** begin with fair per-CPU queues and exact task ownership. Every scheduler path records maximum traversal, allocation source, donation depth, IRQ-off time and wakeup deadline before adding policy classes.

### IPC, handles and capabilities

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **NexiOS + Skift** | `S/R/T` + `S/R` | Opaque per-process slots, rights subsets, generations/derivation, bounded revoke; transactional `preflight -> reserve -> transfer -> commit/rollback` | Skift's apparent free-slot inversion shows this path needs empty/full/boundary property tests; NexiOS retyping is incomplete |
| 2 | LemonOS / MollenOS | `S/R` | Generate client/server/validator/trace/conformance code from one IDL | Define queue-capacity owner, deadline/cancel, peer death and handle-transfer rules; current implementations leave gaps |
| 3 | Fudge | `S/R` | Typed/versioned event envelopes and generated dispatch | Unknown events must return explicit errors; services need separate address spaces |
| 4 | MaslOS-2 | `S/R` | Conversation IDs and stdio-over-IPC as an application composition primitive | Hand-rolled queues/raw pointers are unsafe; preserve behavior, not implementation |

**zlOS decision:** build `zlIDL` around bounded bytes plus bounded transferred handles, protocol/version, request/reply ID, rights and explicit cancellation/backpressure. Transfer is atomic or has no effect.

### Storage and filesystems

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **Brook** | `S/R/T` | Compile real VFS/ext2/storage-independent logic into host image tests against disk images | Ext2 still has feature gaps; host tests do not prove flush/power-loss/device behavior |
| 2 | MollenOS | `S/R` | Read-only application images separated from writable data/state | Immutable bytes are packaging, not isolation; provider ownership must be enforced |
| 3 | banan-os | `S/R` | VFS/storage layering plus reproducible application/port manifests | Crash consistency and recovery proof remain weak |
| 4 | mYOS / privilegeOS | `S/R` | Minimal persistence smoke; separately bootable conservative rescue flow | Never inherit monolithic unversioned disk state, password bypasses or passwordless general operation |

**zlOS decision:** zlfs remains the native spine. Add checksummed/versioned metadata, explicit flush/barrier semantics, power-cut and corrupt-image scenarios, disk-full behavior, concurrent rename/write and cold-reboot receipts. Packages are content-addressed read-only images with capability-rooted writable mounts.

### Networking

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **Brook** | `S/R/T` | Run real TCP/lock/futex/descriptor logic in deterministic host harnesses | Fixed pools and hardware seam remain; add packet loss/reorder/timer/property tests |
| 2 | SerenityOS | `S/R/T/CI` | Network/DHCP/lookup/request services and restricted client processes | Large custom library/service scope; bare-metal NIC support remains narrow |
| 3 | Zinnia | `S/R` | Compact complete conventional Ethernet/ARP/IPv4/ICMP/TCP/UDP/raw/local object map | No runtime CI or network service/capability boundary |
| 4 | snarkOS | `S/R/T` | Separate bootstrap CDN, ordinary peer sync and authoritative control/BFT paths; role caps and rate limits | Blockchain semantics do not belong in the kernel; adopt operations, not consensus |

**zlOS decision:** NIC services expose socket handles; stage protocols independently. A loaded driver, exchanged packet, completed TCP flow, DNS result and TLS policy check are separate evidence states. Host simulation injects loss, duplication, reordering, delay, reset and exhaustion.

### Drivers and hardware lifecycle

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **Zinnia + Brook** | `S/R` + `S/R/T` | One narrow module/registration descriptor per driver plus real-logic host tests and a local hazard ledger | Separately compiled source still needs shipped-image inventory, QEMU/hardware and teardown proof |
| 2 | Sapphire GPU | `S/T` | Required/optional capability descriptors; separate command/data streams; ready/valid protocol simulation | It is incomplete hardware, so use its protocol shape only |
| 3 | Astral | `S/R` | Modern NVMe/AHCI/xHCI/HDA implementations and declarative initialization | Recovery, timeout, hot-unplug and integrity paths are the weak area |
| 4 | banan-os / SerenityOS | `S/R` | Broad connected PCI/storage/USB/audio/display composition | Breadth and initial enumeration must not define completion |

**zlOS decision:** every driver declares IDs, dependencies, claimed resources, DMA/IOMMU needs and `probe/start/online/quiesce/stop/recover` transitions. Promotion requires forced timeout/reset/error, repeated init/teardown and matching QEMU/hardware receipts.

### Graphics, desktop, audio and applications

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **SerenityOS** | `S/R/T/CI` | Server-owned window/audio state, damage/occlusion, per-client audio queues, restricted disposable decoders | Large ecosystem cannot be copied as one milestone; browser docs contain aspirational parts |
| 2 | LemonOS / banan-os | `S/R` | SHM double buffering, damage rectangles, client libraries and explicit compositor boundary | Validate client dimensions/stride/format/handles; avoid final full-screen copies where possible |
| 3 | duckOS | `S/R/T` | Small Pond/Quack/River service seams and shared circular audio buffers | RPC length/user-pointer/permission gaps must be fixed at the common boundary |
| 4 | MaslOS-2 / Banana OS | `S/R` | Cohesive desktop, packaging, installer and rich reference-app experience | They are breadth/UX donors, not isolation/security donors |
| 5 | asm.fm | `S/R/V` | Deterministic integer DSP/test tones for PCM/mixer verification | Offline WAV generators are not a real-time audio architecture |
| 6 | ProtOS | `S/R/F` | Small vertical slice from named events and per-window surfaces to a user-space WM and PTY terminal | Use only the seam: direct framebuffer access, ambient IPC, unchecked geometry/pointers and unowned shared mappings are negative patterns |

**zlOS decision:** compositor owns scanout/input and server metadata; clients receive bounded shared surfaces and event channels. AudioServer owns hardware and bounded stream rings. Image/font/archive/media parsers run in restartable restricted processes. Existing zlOS apps migrate behind these protocols rather than being visually rewritten first.

### Security, trust boundaries and recovery

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **SerenityOS** | `S/R/T` | Monotonic authority reduction inherited across exec/fork and enforced at shared VFS/process boundaries | String promises/paths should evolve into typed zlOS handles; retain kernel-wide enforcement |
| 2 | ChittiOS | `S/R/T` | Grammar-limited execution, consent and grant-attenuation design questions | Built-ins use a mislabeled baked-key MAC, foreign signed ingestion is unfinished, install/uninstall is non-atomic, kill is a no-op, and bounded local logs are not permanent attestation |
| 3 | RustOS | `S/R/T/CI` | Authority maps, authenticated providers, fail-closed behavior, receipts preserving failed gates | Process overhead is high; model evidence needs implementation conformance |
| 4 | privilegeOS | `S/R` | Separately signed rescue image, explicit target confirmation, backup/restore pairing and repair log | Reject credential bypass/passwordless general operation and unverified supply chain |

**zlOS decision:** capabilities are monotonic, typed, revocable handles. Validate whole user spans before mutation. Driver/tool/plugin install is signed and capability-authorized. Recovery is separate, conservative and audited; no username-only elevation or ambient global resource lookup.

### Build, tests and evidence

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **RustOS** | `T/CI` | Owner/authority/state contracts, source-model links, mutation/fuzz/formal checks, preserved failing receipts and explicit untested scope | Expensive; models do not certify implementation or hardware |
| 2 | SerenityOS | `T/CI` | Host suites, sanitizer/fuzzer variants, x86-64 target tests, multiarch boot CI and post-test filesystem gate | AArch64/RISC-V runner skips target tests; scale/toolchain burden is high |
| 3 | Brook | `T` | Fast registered host-native execution of real kernel logic | Freestanding fixtures are build targets, not registered executions; add CI/current/QEMU/hardware evidence |
| 4 | Hyper | `T/V` | Small assertion payload and parameterized black-box matrix | Extend local proof beyond BIOS |
| 5 | freestanding-c-hdrs-gnu | `T/V/F` | Regenerate from pinned upstream digests and test every supported compiler/target combination | Generated headers are compiler inputs, not language/libc completion |

**zlOS decision:** one machine-readable feature ledger records claim, source, reachability, build, boot, scenario, failure injection, QEMU, hardware, model/conformance and exact artifact receipt. Gates propagate exit status; the booted guest prints a commit/artifact identity so stale images cannot pass.

### Operations, deployment and public demos

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **snarkOS** | `S/R/T` | Typed roles, least exposure, health/telemetry, ordered startup/reverse shutdown, checkpoints, bounded peers, reversible plugins | Adopt the operational control plane, not blockchain consensus/proof semantics |
| 2 | mission-control | `S/R/T/V-syntax` | Prewarmed QEMU slots, opaque short leases, COW disks, hard/idle deadlines, logs and reaping | Needs transactional state, per-lease proxy auth, cgroups, egress policy, PID identity and safe termination |
| 3 | banan-os | `S/R` | Pinned dependency graphs and source/patch/build hashes for ports | Add install-and-launch runtime receipts; recipes alone are not availability |
| 4 | unix-history-repo | `C/S/V` | Preserve architecture epochs, migrations, provenance and blame continuity | Imported rights still require review; only commits/refs/objects/disk size were locally measured |

**zlOS decision:** production/demo roles get distinct surfaces and resource budgets. Public demos use transactional lease state machines, strict QEMU allowlists, per-lease proxy secrets, COW rollback, graceful-then-force termination, cgroups and restricted guest egress.

### Agent execution and tool/plugin patterns

| Rank | Donor | Evidence | Pattern to adopt | Tradeoff / rejection boundary |
|---:|---|---|---|---|
| **1 — best** | **ChittiOS, corrected by RustOS** | `S/R/T` | Intent becomes a typed plan; grammar constrains decoding; broker checks capabilities/provenance/consent; deterministic execution emits a receipt; grants attenuate for subagents | Chitti's placement is wrong for zlOS: the root orchestrator must be an ordinary process; RustOS-style independent evidence must challenge success |
| 2 | snarkOS | `S/R/T` | Role-specific workers, authenticated plugin management, bounded callbacks, explicit lifecycle and telemetry | Plugin reload is incomplete; blockchain roles are analogies, not direct agents |
| 3 | NexiOS | `S/R/T` | Every agent queue, traversal, wait and resource pool has a declared bound and tested exhaustion result | Static bounds require explicit backpressure/admission UX |
| 4 | Fudge | `S/R` | Versioned event messages and reactive callback dispatch | Silent drops, weak process isolation and global pools are rejected |

**zlOS decision:** the model and orchestrator run as isolated processes. One small deterministic broker owns tool handles and validates a compiled plan. Subagents receive strict subsets with deadlines and revocation. Every result records inputs, authority used, artifacts, failures, skipped checks and independent review.

## Donor selection summary

| Need | Primary donor | Secondary corrective donor | Resulting zl/zlOS rule |
|---|---|---|---|
| Boot truth | Hyper | Astral | Assertion-kernel matrix over a dependency-checked boot DAG |
| Kernel/service split | MollenOS | RustOS | Small mechanism kernel; policy in fail-closed supervised services |
| Memory correctness | RustOS | NexiOS | Atomic admission plus explicit bounded allocation/exhaustion |
| Scheduling | NexiOS | RustOS | Per-CPU ownership, bounded behavior, measured deadlines before RT claims |
| IPC/capabilities | NexiOS + Skift | LemonOS | Transactional handles and generated bounded protocols |
| Files/storage | Brook | MollenOS | Host-tested FS logic plus immutable packages and crash-consistency proof |
| Network | Brook | SerenityOS/snarkOS | Deterministic protocol tests plus service/operations separation |
| Driver framework | Zinnia | Brook/Sapphire | Narrow descriptors, host protocol tests and complete lifecycle receipts |
| Desktop/audio | SerenityOS | LemonOS/duckOS | Server-owned state, SHM/damage, bounded audio, isolated parsers |
| Security | SerenityOS | ChittiOS/RustOS | Monotonic handles, constrained tools, fail-closed providers |
| Evidence | RustOS | SerenityOS/Hyper | Claims advance only through deterministic gates and receipts |
| Public demo | mission-control | snarkOS | Transactional leased QEMU service with role/resource policy |
| Agent system | ChittiOS | RustOS/NexiOS | Typed plan, attenuated execution, bounded resources, independent receipt review |

## Anti-donor findings: mechanisms to reject

- **Masked success:** Cyjon's `|| true`, vib-OS's timeout-success CI, LemonOS tests that always return zero, and MollenOS runner status hazards demonstrate why the outermost gate must preserve every inner exit status.
- **File equals feature:** vib-OS ext4/APFS/network/sandbox, TacOS NVMe, LemonOS xHCI/HDA, AthenX multitasking, and privilegeOS's missing image show that source presence and artifacts need reachability and behavior probes.
- **Ambient/ring-0 power:** Chitti's privileged orchestrator, Banana OS's unguarded kernel modules, AthenX raw user pointers, and weak banan/duckOS credential checks are direct negative references.
- **Pointers and handles without provenance:** ProtOS directly dereferences user pointers, exposes a null in-range syscall, lets callers share arbitrary ranges to any PID, and accepts a user-visible kernel message pointer for freeing. zlOS must use checked slices, opaque handles and capability-scoped grants.
- **Unbounded lifecycle:** busy waits, missing timeouts, no cancellation, no detach/unload, orphan QEMU processes, and global queues recur across otherwise impressive systems.
- **Manual catalogs as truth:** osdev-projects and stale README/test-count mismatches require generated inventories with timestamps, immutable commits and validators.
- **Immutable bytes mistaken for isolation:** VaFS/package images reduce mutation, but only capabilities, namespaces, process ownership, revocation and validated IPC provide authority isolation.

## What this matrix proves—and does not

It proves that all 33 downloaded snapshots were classified through the completed source audits and that their meaningful mechanisms, evidence ceilings, strongest ideas, and dominant gaps were normalized into one comparison. It does not prove every repository builds or boots. Local successful execution exists only where marked `V`; several important systems remained source/test/CI audits. The Unix history graph is now fully unshallowed and its commit/ref/object/disk measurements are recorded, while other README scale totals remain claims. The clean-room architecture should therefore depend on behavior and contracts described here, never on unverified feature counts or copied source.
