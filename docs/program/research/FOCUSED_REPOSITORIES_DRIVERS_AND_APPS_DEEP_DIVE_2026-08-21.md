# Focused repositories: drivers, applications, services, and operations deep dive

Date: 2026-08-21

Source shelf: `/home/roy/Documents/repos/zl-starred-sources`

Scope: the remaining sixteen focused/supporting repositories, read as device, application, service, media, utility, game, tooling, and operations references rather than falsely ranking all of them as operating systems.

## Exact snapshots

| Repository | Audited commit | License at snapshot | Correct classification |
|---|---|---|---|
| `robotman2412/sapphire-gpu` | `575efe030d2ea69d1448bd43b95f6d7815249b47` | CERN-OHL-P-2.0 | incomplete GPU RTL/protocol laboratory |
| `UltraOS/Hyper` | `d4f446550ebb5df460e6c5ef8e5759b2d180d0a4` | MIT | multi-firmware boot service |
| `ProvableHQ/snarkOS` | `a8e2e33f6e769386e09c2dd6a32fafd3dc126424` | Apache-2.0 | distributed-node service/control-plane reference |
| `dspinellis/unix-history-repo` | `d0d404ce50489bf40a96165ff2fa7291194301b1` | mixed, lineage-specific historical permissions | provenance/history dataset |
| `felipenlunkes/osdev-projects` | `0736c963d92aebc3c9f6810fb6213372ff87398a` | BSD-3-Clause | manually curated discovery catalogue |
| `marceldobehere/MaslOS-2` | `154d42e53fc0320b38d120acb3bdfd8677fae60f` | AGPL-3.0 | broad graphical hobby OS |
| `alexdboxall/Banana-Operating-System` | `f463540c60727237ea4fba49137d8c9c4ab83b98` | CC BY-NC 4.0 plus third-party notices | broad legacy graphical hobby OS |
| `KingVentrix007/AthenX-2.0` | `e6734419e24f70b97c4e5ad7d9bff97d17799764` | GPL-3.0 in `LICENSE.txt`; README badge says MIT | 32-bit teaching OS with metadata conflict |
| `ktauchathuranga/privilegeos` | `1563a393fe6d6702bcacb4ccded786f4f30b3abe` | MIT wrapper; Linux/BusyBox retain their licenses | Linux/BusyBox rescue-image builder |
| `yashvi-soni-30/mYOS` | `20bf154d8da58473922c61796875d93f6e4fb900` | no license found | tiny persistent shell demo |
| `cppsever/MineBios` | `ca9831fa763c637e69b85e244dabe545343d84a8` | Apache-2.0 | bootable real-mode game |
| `viralcode/vib-OS` | `73b64299bf6e591e738c58c9d1845f0babdd6f58` | README claims MIT; root license absent | ARM64 graphical kernel/demo with major claim drift |
| `whispem/asm.fm` | `a5572911ba9c21a0782076d2e343ed301c9dc455` | MIT | offline integer-DSP example corpus |
| `brainboxdotcc/mission-control` | `a6b7ec4838eff0a51768ea68e08745f17429ada4` | Composer metadata says MIT; root license absent | QEMU/noVNC browser-demo control plane |
| `osdev0/freestanding-c-hdrs-gnu` | `cdd9b3cc11808d3259c082a77c33dfeb0f040adc` | GPL-3.0 with GCC Runtime Library Exception as applicable | generated compiler-header input |
| `Ellicode/protOS-kernel` | `daa97b48e7a213c659d34eed9f3c225568243b56` | MIT | x86-64 learning kernel and user desktop vertical slice |

License consequence: this is a clean-room ideas-and-tests study. Do not copy implementation code. That boundary is mandatory for the non-commercial, copyleft, mixed-rights, missing-license, and conflicting-license snapshots.

## Evidence vocabulary and verification boundary

- **`[C]`**: documentation or UI claim.
- **`[S]`**: substantive implementation exists at the cited path/identifier.
- **`[R]`**: the normal build, boot, route, package, or initialization graph reaches it.
- **`[T]`**: a test or simulation targets it.
- **`[CI]`**: automation invokes a build/test path; this does not imply the oracle is sound.
- **`[V-prior]`**: a local build/run was previously performed and recorded in `FOCUSED_REPOSITORIES_AUDIT_2026-08-21.md` or `FOCUSED_REFUTATION_AND_COVERAGE_2026-08-21.md`; it was not rerun for this read-only deep dive.
- **`[stub]`**: the callable surface returns a fixed error, prints simulated output, omits the device transaction, or contains only skeletal state.
- **`[external]`**: behavior comes from an upstream kernel/tool/runtime, not this repository's implementation.
- **`[F]`**: source or prior executable evidence contradicts the broad claim.

No source checkout was edited. No new builds or boots were run. Pre-existing untracked `build/` in MineBios and `build-audit*` directories in protOS were preserved. All other focused source checkouts were clean at the final audit.

## Executive feature truth

| Area | Strongest evidence-backed reference | What it really contributes | Do not infer |
|---|---|---|---|
| boot media/firmware | Hyper | BIOS/UEFI, x86/AArch64, FAT/ISO, GPT/MBR/raw/PXE matrix plus assertion kernels | that every collected UEFI/AArch64 scenario was run locally |
| device protocol design | Sapphire | capability descriptors, explicit status/IRQ, split control/bulk streams, DMA/SPI simulations | complete rasterizer, shaders, compute, or deployable GPU |
| service operations | snarkOS | role-specific service composition, bounded peers/rates, tiered sync, metrics, checkpoints, authenticated management | that blockchain consensus belongs in zlOS |
| app/process composition | MaslOS-2 | conversation IDs, stdio over IPC, SAF application bundles, restartable desktop modules | safe ELF/filesystem/authority boundaries |
| broad legacy devices/apps | Banana OS | modular driver packages, VFS/media/UI breadth, CAB installer/distribution flow | x86-64 support, safe ring boundary, reproducible build |
| small vertical teaching tree | AthenX | display/storage/audio/network/UI/user-program organization | multitasking, authentication, complete syscalls, robust networking |
| rescue/repair operations | privilegeOS | removable-target checks, offline Linux rescue image, logs and paired recovery scripts | independently implemented drivers or secure general-purpose defaults |
| minimal persistence | mYOS | ATA PIO + fixed table + shell operation reaches disk | crash safety, directory-correct lookup, robust interrupt/device handling |
| tiny bootable game | MineBios | FAT12 chain load, bounded tile viewport, direct BIOS/game loop | OS services architecture |
| graphical prototype | vib-OS | build-connected ARM64 graphics/media/RamFS demo | protected userspace, real Python, full TCP/IP, ext4/APFS, NVMe/USB MSD, SMP |
| audio behavior | asm.fm | 25 deterministic integer DSP/WAV generators | device streaming, mixer, scheduling, portable library |
| public demo control plane | mission-control | slots, leases, overlays, deadlines, reaper/admin surface | authenticated VNC data plane or process containment |
| toolchain inputs | freestanding headers | pinned regeneration of GCC target headers for six architectures | libc, POSIX, startup ABI, linker/runtime completeness |
| user-space desktop seam | protOS | VFS/TAR/devfs/PTY/process/IPC/shared surfaces/WM/terminal relationship | safe parsers, user pointers, IPC, shared memory, or clean build |
| history/discovery | unix-history + osdev-projects | provenance-preserving evolution and a candidate taxonomy | production feature evidence from list membership |

## 1. Sapphire GPU — device protocol and display RTL

### Inventory and implementation

- `[C][F]` The README claims hardware rasterization, scanout, many formats, float coordinates, future shader cores and compute (`README.md:1-4`). Drawing and compute command documents contain only headings, and no connected rasterizer/shader/compute implementation exists.
- `[S][R]` `hdl/sapphire/interface/cmd/CmdEngine.scala` decodes management and serial transfer commands. The documented surface is NOP, status, description, IRQ clear/enable, DMA read/write setup, and read/write payload (`docs/cmd/README.md:13-24`). Control and payload are separate phases rather than one unbounded transaction.
- `[S]` `docs/structures.md:5-43` defines status, implemented interrupts, RAM size, semantic version, required/optional feature bits, coordinate width, scanout count, 32/64-bit pointer compatibility, optional 3D, and vertex color. A driver is explicitly required to reject an unsupported required capability (`:31-36`).
- `[S]` DMA/memory blocks are `dma/DmaBus.scala`, `dma/DmaMux.scala`, `mem/Ahb3ToDma.scala`, and `mem/SpiMemCtrl.scala`; SPI physical blocks are `phy/spi/SpiMaster.scala` and `SimpleSpiSlave.scala`. `DmaMux` has stream wiring, but its selection register is initialized to zero and has no assignment/control path (`DmaMux.scala:20-37`), so it cannot actually select a controller.
- `[S]` Display/media blocks include pixel formats and conversions (`color/PixelFormat.scala`, `ColorMath.scala`), framebuffer scan (`scanout/FbScanner.scala`), ILI9341 output (`scanout/Ili9341Scanout.scala`), video timing, and texture read/write. `scanout/CrtScanout.scala:15-23` declares only APB/DMA ports and contains no scanout logic.
- `[T]` Eight simulations target AHB-to-DMA, command decode, framebuffer scan, SPI master/slave/memory, and texture read (`hdl/sapphire/sim/*Test.scala`). The Makefile discovers simulations dynamically and invokes SBT (`Makefile:8-25`). `[V-prior]` They were not run because SBT/Verilator were unavailable.

### zlOS lesson

Adopt the contract shape: versioned capability discovery, required-versus-optional negotiation, explicit IRQ/status state, split control and bulk payload, bounded valid/ready streams, and device-level simulations before a complete driver exists. Reject the marketing pattern: no “GPU,” “3D,” or “compute” badge until a top-level build connects command intake to memory, raster/compute execution, scanout, error reporting, and a behavioral oracle.

## 2. Hyper — boot device and firmware service

### Inventory and implementation

- `[C][S][R]` BIOS and UEFI boot, i386/amd64/AArch64 ELF, MBR/EBR/GPT/raw/PXE, FAT12/16/32, ISO9660, HDD/El Torito/hybrid ISO, and Ultra handover are documented at `README.md:7-27` and wired by `loader/CMakeLists.txt` into platform, architecture, filesystem, and protocol subgraphs.
- `[S][R]` `loader/loader.c` locates/parses configuration and selects entries; `loader/config.c` implements nested typed values; `loader/filesystem/path.c` resolves origin, disk index/GUID, partition index/GUID, raw media, optical media, and PXE/TFTP selectors. Exact-origin semantics are documented at `README.md:91-177`.
- `[S][R]` `loader/filesystem/gpt.c`, `mbr.c`, `filesystem.c`, and `block_cache.c` implement partition discovery and cached block I/O. `loader/filesystem/fat/fat.c` and `iso9660/iso9660.c` implement FAT and ISO/Rock Ridge traversal; `loader/filesystem/pxe.c` provides the TFTP-backed file surface.
- `[S][R]` `loader/elf.c::{elf_get_arch,elf_load}` validates architecture/header information and performs bounded segment loading (`:290-468`). `loader/boot_protocol/ultra.c` packages kernel, modules, command line, memory map, framebuffer, and firmware/platform metadata. `README.md:179-209` demonstrates module and video-mode configuration.
- `[S][R]` Installation/release tooling covers MBR stage gaps, GPT BIOS boot partitions, EFI removable paths, and hybrid ISO construction (`README.md:34-75,211-248`). Chainloading remains TODO (`loader/loader.c:load_entry`). AArch64 handover has a remaining cache-maintenance TODO in `loader/arch/aarch64/handover_impl.c`.
- `[T][CI][V-prior]` `tests/test_loader.py` supplies 75 collected scenarios and six assertion-kernel variants. Prior local evidence built i686 BIOS plus all assertion kernels and passed 33 BIOS scenarios; UEFI/AArch64 were collected but not locally run.

### zlOS lesson

Build a tiny zl boot-contract kernel and test every cross-product that matters: firmware, architecture, partitioning, filesystem, fragmented files, boot-origin selection, module metadata, oversized command line, optical/hybrid, and network boot. A loader feature is complete only when the kernel asserts what it received—not when the loader merely links.

## 3. snarkOS — services, operations, and lifecycle donor

This is not an OS or device-driver donor. Its useful surface is how a large stateful node composes services and exposes operations.

### Service inventory and implementation

- `[C][S][R]` Validator, client, prover, and bootstrap-client roles have different authority and resource responsibilities (`README.md:48-52`; `node/src/{validator,client,prover,bootstrap_client}`). Validators participate in BFT; clients keep/validate a ledger without consensus; provers solve work without maintaining the ledger.
- `[S][R]` Twenty-one Cargo workspace packages separate accounts/CLI/display, role nodes, TCP/network/router/messages, sync/CDN, BFT/consensus/ledger/storage, REST, metrics, and utilities (`Cargo.toml:[workspace].members`). This keeps role constructors from automatically starting every service.
- `[S][R]` Network services implement typed challenge/block/peer/ping/pong/puzzle/transaction/solution/disconnect messages, connection policy, peer limits, version/SHA-aware handshakes, throttling, and metrics (`node/router/src/messages`, `node/network/src/peering.rs`, `node/tcp/src`). Bootstrap clients use a deliberately narrower resolver/handshake path.
- `[S][R]` Synchronization has distinct bulk CDN, P2P block/locator, and BFT modes (`node/cdn/src/blocks.rs:CdnBlockSync`, `node/sync/src`, `node/bft/src`). CDN work has bounded concurrency, pending-block limits, retries, stoppability, and an await-once handle (`node/cdn/src/blocks.rs:48-121`).
- `[S][R]` The REST service exposes height/blocks/transactions/programs/mappings/state paths/committees/delegators/peers, sync mode/peers/requests/speed, broadcasts, prover limits, history, view evaluation, rewards, and validator participation (`node/rest/src/routes.rs:300-1409`). It gates operations when the node is too far out of sync.
- `[S][R]` Management operations include RocksDB checkpoints/database backup (`node/rest/src/routes.rs:1130-1153`) and Slipstream plugin list/load/unload endpoints (`:1411+`). Plugins have startup/shutdown hooks and canonical-mapping/staking-reward hooks; reload and some specialist BFT/prover paths remain TODO/unimplemented.
- `[S]` JWT construction/middleware lives in `node/rest/src/helpers/auth.rs:35-100`. README deployment guidance says validator REST should always be disabled and metrics restricted to a VPC/VPN (`README.md:126-135`). That role-level disable is stronger than endpoint-only authentication.
- `[S][F]` `--nojwt` intentionally removes authentication (`README.md:375-378`), and library construction without initialized global JWT state can leave sensitive routes unprotected. REST RPS accepts zero but later computes a nanosecond interval by dividing by RPS, so zero can panic. Both require configuration validation at construction.
- `[T][CI]` The tree contains 318 Rust test/Tokio-test declarations plus CLI/node integration tests, devnet/restart/upgrade scripts, CircleCI, benchmark, and release workflows. `[V-prior]` Cargo metadata confirmed 21 packages; the very large git dependency graph was not built locally.

### zlOS/Jarvis lesson

Use role-specific supervisors, not one omnipotent daemon. Each service role needs a declared authority set, exposure profile, connection/resource budget, sync phase, health/telemetry schema, startup order, reverse shutdown with deadline, checkpoint/restore operation, and authenticated management route. Loadable tools must be isolated processes or Wasm-like sandboxes, not untrusted native libraries in the supervisor.

## 4. Unix history repository — provenance as an operations feature

- `[C][S]` The repository is a synthesized Git history, not a driver/app codebase. It connects 22 historical snapshots and modern FreeBSD into a continuous timeline with author attribution (`README.md:4-17,33-46`).
- `[S]` Tags and branches encode release, snapshot-development, imported-VCS, and merge provenance (`README.md:53-73`). Git log/follow/blame are the user-facing “applications” for studying subsystem evolution (`README.md:75-203`).
- `[V-prior]` The initially shallow checkout was fully unshallowed. Local measurements are 849,161 commits, 8,037 merges, 195 remote refs, 39 tags, and 5,455,351 packed objects. The working tree stays on the small PDP-7 research branch while other lineages live in refs/objects.
- `[external]` Reconstruction tooling is in the separate `unix-history-make` repository (`README.md:22-31`), so this checkout is the generated dataset, not its build pipeline.
- Rights are lineage-specific: Caldera/Novell/Compaq/VenturCom and author rights coexist with BSD and FreeBSD terms; Research Unix 8-10 non-assertion is non-commercial (`LICENSE:1-54`).

### zlOS lesson

Make evolution queryable: architecture-epoch tags, parent-connected migrations, imported-source boundaries, generated-artifact provenance, feature-evidence receipts, and blame continuity. Do not mine historical code without a per-lineage rights review. The practical “feature” is being able to answer when a driver contract changed and which test justified it.

## 5. osdev-projects — discovery catalogue, not implementation evidence

- `[C][S]` One large README classifies projects by Unix/direct, Unix-like, DOS, Windows-like, BeOS-like, original, and mainframe families, then by implementation language with architecture/activity/GitHub/license columns (`README.md:19-50` and later tables).
- `[F]` It contains no project source, normalized data file, schema, generator, link checker, tests, or CI. The checkout is `README.md`, license, contributor metadata, and Git metadata.
- Manual tables contain stale, duplicated, malformed, and inconsistent fields; membership proves only that a link was curated, not that the target builds or has a named feature.

### zlOS lesson

Turn discovery into a versioned registry: canonical repository ID, commit, role, architecture, language, license evidence, last validation, feature-evidence links, and automated URL/schema/duplicate checks. Use the catalogue to find candidates; only source and executable probes graduate a candidate into an architecture decision.

## 6. MaslOS-2 — desktop applications, IPC, audio, and storage

### Driver/service inventory

- `[S][R]` Limine x86-64 boot reaches paging, scheduler, per-task page-table context, priorities, kernel/user stacks, yielding, message-wait states, ELF processes, syscalls, and experimental threads. Central code is in `kernel/scheduler`, `kernel/interrupts/interrupts.cpp`, and `kernel/elf`.
- `[S][R]` IPC message queues, conversation IDs, process start/kill/restart, and stdio connections are used by `libm/stdio/stdio.cpp`, logger, shell, dialogs, and desktop. This turns standard streams into an explicit service relationship instead of one kernel-global console.
- `[S][R]` PCI dispatch creates AC97 and AHCI devices (`kernel/devices/pci/pci.cpp:187-203`). AHCI probes ports and wraps SATA as a generic disk interface (`kernel/devices/ahci/ahci.cpp:361-543`; `kernel/diskStuff/Disk_Interfaces/sata`). AC97 uses DMA buffer descriptors, IRQ refill, sample-rate setup, and an audio-output abstraction (`kernel/devices/ac97/ac97.cpp:17-559`). PC-speaker support is also present.
- `[S][R]` Storage layers are disk interface -> MRAPS partitions -> MRAFS filesystem, plus SAF read-only bundle/initrd (`kernel/diskStuff`, `kernel/saf`). Signatures are versioned only by strings such as `MRAPS01` and `MRAFS01`; they lack the integrity/transaction model needed for user data.
- `[S][R]` The build creates `programs.saf`; kernel startup mounts SAF, copies modules/programs/assets into the runtime tree, and starts desktop modules (`kernel/kernel.cpp:21-207`). This is a concrete app-bundle/distribution path.

### Applications and user-facing features

- `[S][R]` System UI: desktop/window manager, taskbar, launcher/start menu, debug terminal, shell/terminal, explorer, open/save file explorers, dialogs, image viewer, notepad, MicroEdit, paint, calculator, and logger. Start-menu launch wiring is explicit in `modules/startMenu/main.cpp:315-403`; dialog processes are started by `libm/dialogStuff/dialogStuff.cpp:20-53`.
- `[S][R]` Media: music player, audio test, image assets/viewer, PC speaker and AC97 playback. AC97 is real source/reachable, but comments and panic/fallback paths show it is still fragile rather than production audio.
- `[S][R]` Games/demos: Tic-Tac-Toe, Flappy Rocc, Doom, Minesweeper, Connect Four, Goofy Tetris, 3DKub, MAAB Pong/Mandelbrot/alert, window demos, and thread/IPC/stdio/filesystem test programs. Roughly thirty program directories are bundled; README marks several as partial/broken rather than finished.
- `[V-prior]` After the repository-specific clean-object prerequisite, the full build produced a fresh 33,695-sector ISO and SAF and deployed Limine. No healthy graphical boot oracle or automated runtime suite was obtained.

### Gaps and lessons

- `[F]` No SMP, deterministic fairness, collision-free PID allocator, hostile ELF/SAF/MRAFS parser validation, crash consistency, or security-grade user pointer boundary.
- The ELF loader receives no file length and trusts header offsets/counts; it does not enforce `filesz <= memsz`, and a relocation entry size of zero can create a non-advancing loop. This architecture must not be copied.
- Adopt conversation IDs, typed request/reply messages, stdio-over-IPC, supervised/restartable desktop services, and declarative application bundles. Add signed manifests, hashes, format versions, quotas, transactional installation, capability-scoped endpoints, and parser fuzz tests.

## 7. Banana Operating System — broad modular devices and packaged desktop

### Drivers and media

- `[S][R]` The active build is 32-bit i386 despite x86-64 README language. `kernel/Makefile:1-16,75-99` uses hard-coded Windows/toolchain paths, recursively includes kernel source, and produces 32-bit outputs.
- `[S][R]` Core hardware areas include PCI, ISA/ISA PnP, 8237 DMA, PIC/APIC/PIT, ACPI/ACPICA, UART 16550, PS/2, floppy, VM86 BIOS services, VGA/VESA/EGA, and storage controllers/devices for IDE/AHCI and ATA/ATAPI/SATA/SATAPI (`kernel/hw`; loadable packages under `drivers/`).
- `[S][R]` Loadable driver packages include Floppy, VESA, VGA, AC97, ACPICA, audio system, BIOS, clipped drawing, EGA/VGA, legacy PS/2/SB16, RTL8139, system initialization, WAV codec, and window-system backend (`drivers/*`). This division is a useful device packaging map.
- `[S][R]` Storage/UI integration includes VFS, FAT, exFAT, ISO9660, symlinks, file cabinet, ZIP, registry, audio codec/device abstraction, compositor/windowing, and installer/CAB distribution (`kernel/fs`, `applications`, `packages`, `installer`).
- `[F]` Network source/packages exist, but project TODOs retain TCP/IP and network-driver work; RTL8139 presence is not a verified network service. USB, fork/exec, shared memory, copy-on-write, ext2, pthreads, and robust synchronization remain incomplete.

### Applications and distribution

- `[S][R]` Applications are Regedit, cabinet/file manager, clipboard test, command shell, console host, desktop, Minesweeper, GUI library, photo viewer, RAM viewer, Solitaire, text editor/typewriter, and ZIP tooling (`applications/*`).
- `[S][R]` `packages/{banana,devel,morekrnl,pci,system,wallpapers}` plus CAB/installer artifacts form a real installable distribution layer; a checked-in installer ISO is artifact evidence, not a fresh build/runtime proof.
- `[V-prior]` No useful CI/runtime suite exists and the bespoke `i386-banana-*` toolchain was unavailable, so no fresh build was produced.

### Critical boundary correction and lesson

- `[F]` `SysLoadDLL` can load a user-selected ELF driver into kernel authority without an evident privilege gate. `kernel/thr/elf2.cpp` has class/data/machine validation commented out and lacks complete size/overflow validation. Raw user pointers cross many syscalls.
- Device paths retain hangs/panics/TODOs: VM86 edge cases, symlink nesting, ISA PnP checks, audio conversion, page freeing/COW, and SATAPI reads over four sectors. “Full ACPI” or “networking” should not be inferred.
- Study its driver packaging, device naming, desktop breadth, CAB manifests, installer, and shared media abstractions. For zlOS, drivers must be signed, capability-authorized services with bounded DMA/MMIO grants; kernel module parsing must validate the entire byte slice before any mapping or relocation.

## 8. AthenX-2.0 — vertical teaching apps and partial devices

### Driver and app inventory

- `[C][S][R]` The README correctly describes a simple 32-bit command-line, monotasking hobby OS. Boot/kernel sources cover GRUB, GDT/IDT/PIC, exceptions, paging/FPU/syscall scaffolding, serial, timer, PS/2 keyboard/mouse, VGA/VESA, PCI, ACPI, IDE/AHCI, ISA DMA, AC97, SB16, PC speaker, and e1000 (`drivers/*`; `Makefile:15-28`).
- `[S][R]` Filesystems/media include a custom filesystem, SSFS, FAT32/filelib, VFS, console file type, and WAV parsing (`fs/*`).
- `[S]` Network source implements Ethernet, a one-entry-style ARP cache, IPv4, UDP, DHCP, and TFTP (`network/*`). There is no TCP/HTTP. Fixed IDs, missing checksum/timeout/error paths, and single-interface assumptions make it a learning stack, not robust networking.
- `[S][R]` Kernel UI includes graphical/text console, terminal, TTY/TUI, environment, shell commands, manuals, RLE/bitmap/fonts/images, and an editor (`ui/*`, `utils/man.c`).
- `[S][R]` Separate user programs include `amix`, `bin`, `byte`, `cat`, `edit`, editor, Kilo, shell, simulator, tar, TCC, test, init, and a partial libc (`userspace/*`). The Makefile builds libc, kernel, and userspace (`Makefile:61-67,93-96`). Presence of TCC/tar source does not prove complete runtime behavior.
- `[V-prior]` A dry run expanded 210 commands and a tracked 32-bit ELF exists, but the required i686 cross compiler was absent; there was no fresh build/boot. No CI/tests exist.

### Refutations and lesson

- `[stub]` `kernel/scheduler/multitasking.c::init_multitasking` is empty. `arch/i386/sys_handler.c` implements only a small subset of its long comment; exit/fork announce “coming soon,” screen control is stubbed, and `userspace/libc/syscall.c::syscall_write` has an empty non-void body.
- `[F]` `security/user/user.c::validate_credentials` returns success before lookup/hash comparison; unreachable code prints hashes. `handle_print_system_call` treats an unchecked user string as the format string. Authentication and user isolation are false.
- Storage/audio/TFTP busy waits lack reliable timeouts. ISA DMA 16-bit paths and PCI details remain incomplete.
- Use the approachable vertical directory structure, manuals, shell packaging, and small protocol layers as a teaching checklist. zlOS must add a generated syscall ABI, checked user slices, real credential tests, per-device timeout/reset states, packet fixtures, and a booted application oracle before claiming the same surfaces.

## 9. privilegeOS — rescue-image device coverage and operational utilities

### What is implemented here versus external

- `[S][R][external]` `build.sh` downloads/configures Linux 6.15.3 and BusyBox 1.36.1, builds an initramfs, makes a partitioned image, installs BIOS and x86-64 EFI GRUB, and can run QEMU (`build.sh:241-330,748-979,1096-1129`). The drivers are upstream Linux, not implementations in this repo.
- `[external]` Selected device coverage includes EFI/framebuffer/DRM (i915, amdgpu, nouveau, virtio GPU), ATA/AHCI, SCSI, NVMe variants, xHCI/EHCI/OHCI/UHCI, USB storage/HID, keyboards/mice/touch, networking, ext4, NTFS3, virtio console, and SMP (`build.sh:765-868`). Sound is explicitly disabled (`:871`).
- `[S][R]` Init mounts proc/sys/dev/tmp/devpts, brings up loopback, modprobes console/storage/GPU/NTFS modules, prepares root environment, then hands off to BusyBox init (`build/initramfs/init:12-64`).
- `[S][R]` User operations include drive enumeration, NTFS read-only/read-write mounting, logs, offline target backup/restore scripts, hybrid image creation, USB target size/mount checks, confirmation, write/verification, and QEMU testing (`boot.sh:130-251`; generated scripts in `build.sh`).
- `[V-prior]` Shell syntax passed. No kernel/image build was run because it requires downloads, a large Linux build, loop devices, and privilege. The claimed final image is absent from the checkout.

### Security/supply-chain boundary and lesson

- `[F]` Versioned Linux/BusyBox tarballs are downloaded without digest/signature verification. Passwordless root shells respawn on multiple TTYs/serial. That is acceptable only for explicitly offline physical rescue media.
- The Windows login-screen replacement utility is a credential-bypass tool, not an zlOS feature. The transferable operation is a separately signed rescue environment with conservative mounts, explicit target identity, dry-run/confirmation, an audit log, and verified backup/restore pairing.
- For zlOS, publish immutable rescue-image manifests, hashes/signatures for every input, read-only default mounts, typed repair operations, failure-injection tests, and a physical-presence/recovery-key policy.

## 10. mYOS — smallest disk-backed user utility surface

- `[C][S][R]` VGA text, PS/2 keyboard IRQ, command history, shell, ATA PIO, and a persistent four-entry-style fixed filesystem are the entire practical surface (`README.md:67-113`; `src/kernel.c`; `src/keyboard.c`; `src/disk.c`; `src/fs.c`).
- `[S][R]` Commands are help, clear, about, color, echo, reboot, halt, ls, touch, write, cat, rm, mkdir, cd, and pwd (`src/kernel.c:106-282`). This is a good minimal UI-to-storage vertical slice.
- `[S][R]` `fs_save/fs_load` serialize the in-memory `FileSystem` struct sector-by-sector from LBA 1; magic initializes a root record (`src/fs.c:17-105`). Directories are parent indexes and files have inline data (`:107-251`).
- `[F]` `strcpy` is unbounded (`src/fs.c:76-83`). Write/cat/remove ignore current directory and type (`:209-249`), duplicate names are allowed, and `pwd` follows untrusted parents into a fixed 16-entry stack (`:169-189`). The disk format has no version, checksum, journal, atomic commit, or independent superblock.
- `[F]` ATA waits forever for BSY to clear, never requires DRQ or checks ERR/DF, and never flushes writes (`src/disk.c:88-143`). Most IDT gates point to zero; only the keyboard path is meaningful. Checked-in 32-bit and 64-bit artifacts do not form one coherent release.
- `[C]` Paging, memory management, ELF, userspace, multitasking, networking, and GUI are explicitly future goals (`README.md:195-206`), not features. No tests/CI/build script or license exists.

### zlOS lesson

Keep the visible proof—shell mutation persists across a cold boot—but implement it with zlfs versioned structures, checksummed/transactional metadata, bounded names/data, directory-qualified lookup, loop/depth validation, ATA/NVMe timeout and error recovery, flush/barrier semantics, and power-failure probes.

## 11. MineBios — bootable game/application appliance

- `[S][R]` `src/bootloader/boot.asm` implements a 512-byte FAT12 BIOS loader: drive selection, root-directory search for `KERNEL.BIN`, disk retry/reset, FAT entry decoding, and cluster-chain loading.
- `[S][R]` `src/kernel/main.asm` is a single real-mode mode-13h game: BIOS keyboard/RTC, tile map/chunks, movement, collision/basic physics, camera, textures, pixel/text drawing, and interaction. It is one application with direct hardware/BIOS calls, not a service architecture.
- `[V-prior]` NASM created a 512-byte boot sector, 61,326-byte kernel, and valid 1.44 MiB FAT12 image. The default Make target immediately launched non-headless QEMU, which was terminated; gameplay was not observed. The existing untracked `build/` is this prior evidence.
- `[F]` Data LBA uses a hard-coded `cluster + 31` adjustment instead of deriving reserved/FAT/root/data layout and sectors per cluster. It only works for the fixed BPB. There are many NASM warnings and no tests/CI.

### zlOS lesson

For a small game/demo, use fixed logical world data plus a tiny camera viewport to cap rendering. For boot tests, keep the stage minimal, retry reads with a strict bound, and follow allocation chains—but calculate every filesystem geometry field and make `build` stop before interactive launch. Provide separate `image`, `test-headless`, and `run` targets.

## 12. vib-OS — graphical/media breadth with disconnected or simulated systems

### Build-connected driver and UI surface

- `[S][R]` The root build is ARM64-only (`Makefile:8-10,54-72`). It recursively compiles all non-x86 kernel C/assembly and all driver C into one kernel (`:142-180`). A separate x86_64 tree is a smaller UEFI demo, not feature parity.
- `[S][R]` Build-connected drivers are UART, Raspberry Pi platform, framebuffer/RAMFB/Bochs video, AGX and virtio GPU, virtio input/network, xHCI, USB HID/MSD, NVMe, Bluetooth HCI, plus HDA/PCI/platform sources elsewhere (`drivers/*`). Recursive inclusion proves linking, not initialization or behavior.
- `[S][R]` Real visual/demo assets include RamFS/VFS, desktop, windows, terminal, fonts, launcher, embedded apps, PNG/JPEG decoding, wallpaper/seed images, and readable staged boot initialization (`kernel/{fs,gui,apps,media}`, `kernel/core/main.c`). This is the genuine strength.
- `[V-prior]` `make kernel` produced a 7,813,904-byte static AArch64 ELF. AArch64 QEMU was unavailable, so no boot/application/device runtime evidence exists.

### Feature-by-feature correction

- `[F][stub]` The booted process model runs embedded programs in kernel space/EL1 and hands them raw kernel API pointers (`kernel/core/process.c`; `kernel/core/main.c::start_init_process`). The separate syscall/fork/task address-space world is not called by the production start path; `sys_clone` returns `-ENOSYS`, and `sys_execve` loads then admits userspace execution is unimplemented (`kernel/syscall/syscall.c`).
- `[F][stub]` `drivers/nvme/ans.c` read/write/flush/reinit paths are stubs; `drivers/usb/usb_msd.c` transport is stubbed and xHCI does not initialize it; Bluetooth send is TODO. Main prints device-loading messages without proving calls.
- `[F]` Only RamFS is mounted. FAT32 block reads are TODO (`kernel/fs/fat32.c`), APFS file lookup/read is unfinished (`apfs.c`), and ext4 is disconnected, lacks triple-indirect blocks/full unlink, journaling/extents/checksums/feature negotiation (`ext4.c`).
- `[F][stub]` DNS constructs but does not send a query (`kernel/net/dns.c`); socket connect can mark state without transport; TCP uses broadcast MAC pending ARP; ICMP reply is TODO (`kernel/net/socket.c`, `tcp_ip.c`). Terminal `ping` prints simulated responses.
- `[F][stub]` “Python” and “NanoLang” terminal commands pattern-match source and hard-code print/add/Fibonacci behavior; the referenced MicroPython directory is absent. Runtime targets silently say “Source not yet configured” if no Makefile exists (`Makefile:201-231`). Host download scripts are not in-OS runtimes.
- `[F]` ARM SMP counts/starts only CPU0 and defers secondaries; x86 MADT parsing is TODO. The sandbox allocates but never switches to an isolated stack, has global fault state, and has no callers. ASLR uses a timer-seeded LCG in shared kernel-space execution.
- `[F][CI]` CI builds ARM64, then runs `timeout 10s make run || true` and prints success without checking output (`.github/workflows/ci.yml:65-72`). This is a false-green boot gate.

### zlOS lesson

Use a machine-readable feature receipt: claim, implementation path, build edge, initialization caller, positive runtime probe, negative/failure probe, architecture/hardware matrix, and verified commit. Visually impressive media can remain a valid `demo` tier, but simulated commands must be named `demo-*`; they must never carry the production subsystem's badge.

## 13. asm.fm — offline DSP applications and golden vectors

- `[S][R]` Twenty-five independent Linux x86-64 NASM programs write 44.1 kHz, 16-bit mono RIFF/WAVE to stdout. Categories are oscillators/noise/supersaw/PWM; scale/melody/tempo/vibrato; polyphony/ADSR/tremolo; FM/ring modulation/bitcrush/distortion; low-pass/filter sweep/auto-wah; delay/chorus/reverb (`*.asm`, enumerated by `Makefile`).
- `[S]` The examples use integer/fixed-point arithmetic, static buffers, explicit saturation, direct syscall I/O, and hand-built WAV headers. Common setup is duplicated instead of shared as a library.
- `[V-prior]` All 25 assembled, linked, exited successfully, and produced files recognized by `file` and `soxi` as 44.1 kHz 16-bit mono PCM, 1-4 seconds. NASM emitted deprecated implicit-absolute-address warnings.
- `[F]` There is no real-time sound device, DMA/ring buffer, mixer, resampler, clock/drift policy, latency/backpressure, shared API, spectral/golden test suite, or portability beyond Linux x86-64.

### zlOS lesson

Reimplement each effect as a deterministic zl DSP golden vector behind one audio-buffer API. Test header structure, sample bounds/saturation, hashes for exact integer paths, spectral properties where exact hashes are too rigid, chunk-size invariance, silence/maximum inputs, and mixer clipping. Then separately build the driver/mixer streaming contract.

## 14. mission-control — ephemeral demo application/service

### Implemented control plane

- `[S][R]` Laravel models preallocated `VmSlot` rows and expiring `VmLease` rows. `LeaseAllocator::allocate` serializes allocation with a cache lock, selects the first free slot, creates a random 64-character token, stores only SHA-256, and sets hard/idle deadlines (`app/Services/LeaseAllocator.php:18-55`).
- `[S][R]` `VmLauncher` makes per-lease qcow2 overlays, structured QEMU argument arrays, Q35/KVM/OVMF, disk/CD-ROM/USB modes, AHCI/xHCI attachments, optional user networking/packet dump, debug/internal logs, daemon PID files, VNC/WebSocket ports, and admin-supplied extra args (`app/Services/VmLauncher.php:15-238`). Symfony Process avoids shell parsing.
- `[S][R]` Web start/session and touch/release APIs, idle/hard reaping, manual kill/stat/lease/slot commands, logs, and database relations form a real operations surface (`app/Http/Controllers`, `app/Console/Commands`, `routes/*`). Overlay deletion and slot release are centralized in `TerminatesSessions` (`app/Traits/TerminatesSessions.php:12-35`).
- `[T][CI]` Feature tests cover allocator, contention/busy slots, lifecycle/touch/release, requests, relations, routes, reaping, kill/stat, and termination. CI installs dependencies, builds assets, runs tests/coverage and static analysis. `[V-prior]` All 95 PHP files syntax-checked; dependencies were absent, so PHPUnit was not rerun.

### Critical service gaps

- `[F]` QEMU VNC/WebSocket has no password/SASL and Apache maps enumerable `/vnc/<slot>` paths directly. Lifecycle tokens do not authenticate the data plane. The session page also checks no lease token (`TryController.php:75-85`).
- `[F]` Start allocates and persists lease/slot before overlay/QEMU launch; `finally` releases only the request lock (`TryController.php:46-69`). Exceptions leak occupied slots, overlays, and possibly VMs.
- `[F]` Lease and slot saves are not one database transaction/row lock (`LeaseAllocator.php:40-52`). Cache-lock failure or multiple misconfigured instances can split state.
- `[F]` Termination suppresses kill/unlink errors and frees the slot without verifying process identity or death (`TerminatesSessions.php:12-35`). Other command paths reverse graceful/force signal ordering. PID reuse can target an unrelated process and orphan VMs can collide with recycled ports.
- Configuration always adds `--enable-kvm`, hard-codes backing format raw, can leave the base disk unattached for non-AHCI modes, and lacks cgroup CPU/RAM/process limits, seccomp/AppArmor, network egress policy, per-VM identity, and a real supervisor.

### zlOS lesson

Keep the lease/overlay/deadline/reaper/admin model. Add transactional allocation and launch rollback, capability-bearing WebSocket URLs or an authenticated proxy, per-VM credentials, PIDfd/cgroup/systemd identity, graceful-stop then bounded force, verified process death before slot reuse, overlay ownership checks, CPU/RAM/PID/egress limits, audit events, and a hostile public-session integration test.

## 15. freestanding-c-hdrs-gnu — compiler/device ABI input generation

- `[S][R]` Generated GCC 16.2.0 headers cover i686, x86_64, AArch64, RISC-V64, LoongArch64, and m68k. Common freestanding/compiler headers include stddef/stdint/stdarg/limits/stdatomic/float/unwind plus architecture intrinsics (`<target>/include`).
- `[S][R]` `make_toolchain.sh` downloads/builds target binutils/GCC, installs generated headers, applies targeted Clang compatibility patches, and uses BLAKE2 pins for the main archives. `.github/workflows/build.yml` runs the six-target matrix and pushes generated results.
- `[S]` `patches/clang-compat.patch` adjusts selected `stddef`, `stdint`, and limits behavior. It does not make GCC's `stdatomic.h` compatible with Clang.
- `[V-prior]` Cross-target Clang smoke compilation for stddef/stdint/stdarg/limits passed on all six; native GCC atomics passed; Clang atomics failed on all six because GCC `stdatomic.h` passes `_Atomic` pointers to builtins incompatibly.
- `[F]` CI proves generation, not consumption by GCC+Clang smoke matrices, and directly commits generated output from matrix jobs. GCC `contrib/download_prerequisites` adds transitive downloads not covered by the primary pins.
- `[F]` This is not libc/POSIX, startup objects, syscalls, ABI glue, linker scripts, libgcc completeness, or C++ support.

### zl lesson

Generate compiler-owned headers from an immutable upstream manifest, capture every transitive digest, sign the result, diff generated output, and compile a target/language/compiler matrix. Keep compiler headers, zl runtime/ABI headers, libc, syscalls, crt/linker, and C++ layers as separately versioned contracts.

## 16. protOS-kernel — user applications and desktop seam with unsafe boundaries

### Connected device and service surface

- `[S][R]` The active CMake target includes Limine boot metadata, serial/panic/backtrace, GDT/TSS, IDT/PIC/PIT, framebuffer graphics/console, PCI enumeration, PS/2 keyboard/mouse, PMM/VMM/heap/PAT, scheduler/process/ELF/syscalls/user framebuffer/IPC, VFS/TAR/devfs/AIO, and TTY (`CMakeLists.txt:13-149`).
- `[F]` Drivers stop at PS/2 and generic PCI enumeration. `src/acpi.c` exists but is omitted from CMake. There is no active APIC/SMP/MSI, PCI function 1-7 handling, storage, network, audio, or USB driver. PS/2 waits can hang without timeout.
- `[S][R]` VFS mounts a USTAR initramfs, devfs provides character devices/PTYs, ELF starts programs, named IPC topics carry keyboard/mouse/window events, and shared pages carry per-window pixel surfaces. This is a compact kernel-to-application composition path.
- `[S][R]` `libproto` wraps allocation, file/process, event, graphics/font/BMP, framebuffer, and window-manager calls. The compositor is user-space rather than kernel policy.

### Applications

- `[S][R]` `corgi`: shell prompt, builtins, command splitting, and external process launch (`programs/corgi`).
- `[S][R]` `protofetch`: system/version/memory display; `ls` and `read`: file utilities; `panic`: deliberate panic utility (`programs/{protofetch,ls,read,panic}`).
- `[S][R]` `protowm`: root desktop, window stack, title bars/shadows, cursor, focus, drag-to-move, keyboard forwarding, terminal shortcut, shared window buffers (`programs/protowm`).
- `[S][R]` `terminal`: graphical window, font rendering, PTY, and shell child (`programs/terminal`). `edit` is built but acknowledged unfinished.
- `[R]` `programs/CMakeLists.txt:1-8` adds all eight program targets. Source presence is therefore build-reachable even though the complete build fails.

### Critical refutations

- `[V-prior][F]` The documented GCC 15 build fails because `-no-pie` is used as a compile option without `-fno-pie`; adding that exposes a missing `aio_remove` declaration. Audit-only forced includes let `kernel.elf` link but break the whole build. No boot was reached and no tests/CI exist.
- `[F]` ELF validation rejects matching magic, uses a malformed hex literal, takes no file size, and trusts header offsets/counts/sizes. Both process loading and symbol loading use it; symbol lookup also retains a pointer to a loop-local copy.
- `[F]` Syscalls directly dereference raw user pointers. An in-range `SYS_UNSHARE` table slot is null; every process can panic, map the physical framebuffer, clear console, and share memory to arbitrary PIDs. No capability boundary exists.
- `[F]` IPC queues/payloads are unbounded and unlocked; receive has no capacity; consumption has a direct use-after-free; ambient named topics allow impersonation. Shared memory has no reference counts, receiver consent, ownership, collision, or revocation.
- `[F]` VFS/TAR/path/TTY/readdir inputs lack explicit lengths/capacities and checked parsing. Thread exit has a use-after-free. Window IPC trusts dimensions/names/IDs and the physical framebuffer is globally writable.

### zlOS lesson

Preserve the architectural seam—VFS + initramfs + devfs + PTY + process + IPC + user-space WM + shared surfaces + terminal—but make it the adversarial checklist. All user data crosses one checked slice/string layer; syscall tables contain handler or `ENOSYS`; messages are bounded kernel-owned envelopes; surfaces are opaque revocable handles; only the compositor owns scanout; parsers take byte lengths; device waits time out; and every teardown path is sanitizer-tested.

## Cross-repository driver plan for zlOS

### Device contract common to every driver

1. **Discovery receipt:** bus address, IDs/class, firmware/ACPI/DT provenance, BAR/range sizes, interrupt mode, DMA width, cache/coherency requirements.
2. **Capability negotiation:** semantic protocol version; required and optional bits; reject unknown mandatory bits. Sapphire is the clearest small reference.
3. **Resource authority:** driver receives explicit MMIO/PIO/IRQ/DMA capabilities, never ambient kernel power. Banana's loadable-driver breadth is useful; its ring-0 loader is the negative example.
4. **State machine:** probe -> configure -> ready -> degraded/resetting -> failed -> removed, with bounded wait and observable reason. mYOS/AthenX/protOS busy waits become negative tests.
5. **Data plane:** bounded descriptor/ring or stream, checked sizes/overflow, backpressure, completion ownership, cancellation, and teardown drain. Sapphire and AC97 show useful shapes; both still require hardening.
6. **Recovery:** timeouts, device reset, retry budget, idempotent reinitialization, media removal, and failure-injection tests.
7. **Evidence:** unit/protocol simulation, emulated happy/failure paths, real-hardware matrix, architecture matrix, and last-verified commit. A linked file is only `[R]`, never `[V]`.

### Suggested subsystem sequence

| Priority | zlOS work | Reference ideas | Required proof before “done” |
|---|---|---|---|
| 1 | boot-contract assertion kernel and media matrix | Hyper | BIOS/UEFI, exact origin, partition/filesystem, fragmentation, modules/framebuffer; positive and malformed configs |
| 2 | unified driver capability/receipt schema | Sapphire + snarkOS role contracts | mandatory-bit rejection, resource ownership, version migration, visible failure state |
| 3 | storage timeout/integrity expansion | Hyper parsers; mYOS/MaslOS negative cases | wedged device, ERR/DF, flush/power cut, corrupt headers, overflow, rollback |
| 4 | audio buffer/mixer and deterministic DSP | asm.fm + AC97/SB16 shapes | golden vectors, clipping, chunk invariance, underrun/overrun, reset, multiple sources |
| 5 | app/service supervision and stdio IPC | MaslOS + snarkOS | bounded typed messages, request correlation, quotas, restart, deadline, authority tests |
| 6 | signed app/driver bundles and installer | MaslOS SAF + Banana CAB/installer | manifest/hash/signature, transactional install/rollback, incompatible capability rejection |
| 7 | compositor-exclusive shared surfaces | protOS relationship, not implementation | opaque handles, geometry/clipping, owner-only refresh, revocation, focus/input authority |
| 8 | public browser demo | mission-control | authenticated VNC data plane, transactional allocation, containment, PID identity, rollback/reaper E2E |
| 9 | signed offline rescue image | privilegeOS workflow | immutable inputs, read-only default, target confirmation, repair log, backup/restore and failure probes |
| 10 | feature/provenance registry | unix-history + osdev-projects + vib negative lesson | schema/link validation, commit/evidence receipt, architecture/hardware freshness, no claim without oracle |

## Application/service architecture plan

- **App manifest:** stable ID/version, executable, required services/device capabilities, files/surfaces/network permissions, resource budgets, data-migration version, integrity hash/signature, and restart policy.
- **Supervisor contract:** explicit role, dependencies, readiness probe, health/metrics, start deadline, shutdown deadline, crash budget, backoff, checkpoint/restore, and audit events.
- **IPC contract:** generated typed schema, opaque endpoint/correlation IDs, sender identity, maximum envelope and queue quota, capacity-aware receive, backpressure/cancellation, and no kernel pointers in user data.
- **User experience:** file manager, terminal, editor, image/audio apps, settings/registry equivalent, task manager, installer/updater, dialogs, and games should each prove the underlying service rather than embed privileged direct calls.
- **Media path:** codecs are pure bounded parsers; audio/video/image device access is through services; offline golden corpora are separate from real-time scheduling/device tests.
- **Recovery/public demo:** operations are products too—signed rescue media, safe repair, logs, backup/restore, ephemeral demo leases, and automatic cleanup need first-class UI and tests.

## High-value negative tests to import immediately

1. Valid ELF magic succeeds; every wrong byte/class/data/machine/header size/offset/count/overflow/overlap fails before mapping.
2. Every syscall table slot resolves to a handler or `ENOSYS`; never null.
3. No kernel/service code dereferences a user pointer; all strings/slices have explicit maximum lengths and copy-fault recovery.
4. IPC receive cannot exceed caller capacity; queues have byte/message quotas; teardown races and free order survive sanitizers.
5. Shared mappings/surfaces require owner, grantee, permissions, collision check, reference count, revocation, and compositor-exclusive scanout.
6. Every ATA/AHCI/NVMe/PS2/audio/network wait has a deadline, error-bit check, diagnostic, retry/reset budget, and a wedged-device test.
7. Disk/filesystem lookup is directory- and type-qualified; duplicate names, parent loops, deep paths, corrupt sizes, and power loss cannot silently damage unrelated data.
8. A process/app slot is not committed until load/start succeeds; every failure rolls back PID/table slot/pages/files/handles.
9. A boot/CI timeout or ignored exit cannot report success. Require a commit-specific banner plus functional assertion and panic absence.
10. Public session API authentication also protects its VNC/WebSocket/data plane; lease ownership is checked on every page/touch/release/stream.
11. VM termination verifies process identity and death before reusing ports/slot; cleanup failures remain visible and retryable.
12. Generated headers compile under every supported target/compiler/language mode; atomics, alignment, varargs, limits, unwind, startup and link each have consumers.
13. Simulated commands are exposed as demos and cannot satisfy the production feature receipt.
14. Application/driver bundles reject unknown mandatory capabilities, wrong architecture/ABI, bad signature/hash, traversal, size overflow, and partial transaction recovery.

## Best ideas, compressed

1. Hyper: assertion-kernel boot matrices and exact boot-origin semantics.
2. Sapphire: capability descriptors plus explicit bounded control/payload streams.
3. snarkOS: role-specific service composition, tiered synchronization, lifecycle, metrics, checkpoint operations.
4. MaslOS-2: conversation IDs, stdio over IPC, supervised desktop modules, declarative app bundles.
5. Banana OS: device/application packaging breadth and installer/distribution UX—never its privilege boundary.
6. asm.fm: tiny deterministic DSP vectors that can become zl audio conformance tests.
7. protOS: user-space compositor + PTY terminal + VFS/devfs/initramfs vertical relationship—never its pointer/IPC/parser implementation.
8. mission-control: leased ephemeral browser demos with overlays/deadlines/reaping, after data-plane authentication and containment.
9. privilegeOS: signed offline rescue media with conservative target operations and paired restore.
10. unix-history/osdev-projects/vib-OS: provenance and machine-readable feature truth are themselves core engineering features.

## Final judgment

The focused set does not contain one codebase zlOS should imitate wholesale. Its value is a set of sharply different proven seams:

- **Hyper** supplies the best executable boot-verification strategy.
- **Sapphire** supplies the cleanest small device-protocol vocabulary.
- **snarkOS** supplies the strongest service-role and operations vocabulary.
- **MaslOS-2** supplies the most useful hobby-desktop application/IPC composition.
- **Banana** supplies breadth and packaging ideas, fenced off from unsafe authority.
- **asm.fm** supplies verified media golden vectors.
- **protOS** supplies a compact desktop vertical slice and a concrete security-negative suite.
- **mission-control** supplies the public-demo lifecycle, with serious authentication/containment work still required.
- **vib-OS, AthenX, mYOS, and MineBios** demonstrate why source, artifact, simulated output, and runtime proof must remain separate states.

The integration rule is simple: adopt contracts, behaviors, UX shapes, and adversarial tests; independently design and implement the zl/zlOS code; require a callable boot-to-user path and a failure oracle before assigning any feature badge.
