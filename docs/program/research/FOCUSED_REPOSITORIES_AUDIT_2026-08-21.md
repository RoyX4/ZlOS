# Focused, tooling, historical, and specialist repository audit

Date: 2026-08-21
Source shelf: `/home/roy/Documents/repos/zl-starred-sources`
Scope: fifteen of RoyX4's 32 starred repositories that are not covered by the mature-OS or architecture-OS reports.

## Result in one page

These repositories are valuable at very different layers. They must not be ranked as if they were fifteen competing operating systems.

- **Hyper** is the strongest boot and compatibility reference. Its compact assertion kernel and firmware/filesystem/path matrix provide the best directly reusable verification strategy in this group.
- **snarkOS** is a production distributed-node reference, not an OS reference. Its role separation, bounded peer/router policy, tiered synchronization, plugin lifecycle, shutdown contract, telemetry, and checkpoint operations are excellent patterns for an agent/service control plane.
- **mission-control** is the strongest reference for turning an OS image into an anonymous browser demo through leased QEMU slots and copy-on-write disks. The current implementation needs hardening before public deployment.
- **Sapphire GPU** is a useful hardware protocol laboratory: descriptive capabilities, separated command and payload stages, stream handshakes, DMA and SPI simulations. It is not yet a complete GPU.
- **MaslOS-2** and **Banana OS** show the breadth and user-facing cohesion possible in hobby desktops. MaslOS-2 is especially useful for message-oriented applications and stdio-over-IPC; Banana for modular drivers, packaged applications, and installer flow. Neither is a security model to copy.
- **AthenX-2.0**, **mYOS**, **MineBios**, and **vib-OS** are strongest as negative and learning evidence: source presence, attractive README claims, or a compiling object does not prove a connected, correct feature. Vib-OS is the clearest example of severe claim-to-implementation drift.
- **asm.fm** is a verified collection of tiny dependency-free audio/DSP examples. Its algorithms are useful for a future zl sound library and test tones, but it is offline WAV generation rather than an audio subsystem.
- **freestanding-c-hdrs-gnu** demonstrates how to generate and provenance-pin compiler-owned freestanding headers across architectures. It also exposed a cross-compiler compatibility gap in `stdatomic.h`.
- **unix-history-repo** is a software-archaeology dataset. Its value is history preservation, blame continuity, and empirical evolution research, not code adoption.
- **osdev-projects** is a discovery taxonomy, not a feature source. Its manually maintained tables need schema and link validation before being treated as data.
- **privilegeOS** is a Linux/BusyBox rescue image, not an independent OS. The removable-media safeguards and recovery-image workflow are relevant; its password-bypass tooling and passwordless root model are not integration candidates.

The clean-room integration direction is therefore: **Hyper's boot matrix + mission-control's hardened demo pipeline + snarkOS-style role/operation contracts + Sapphire's bounded device streams + MaslOS message-oriented app seams + generated freestanding toolchain inputs + independently verified feature truth**.

## Evidence model

- **`[C]` claim**: documentation, screenshot, release text, or UI says the feature exists.
- **`[S]` source**: relevant implementation source exists.
- **`[R]` reachable**: the source is connected to the normal build, boot, route, or service graph.
- **`[T]` covered**: a test or CI scenario targets it, whether or not this audit ran it.
- **`[V]` locally verified**: a build or execution passed during this audit.
- **`[F]` locally falsified or failed**: a claim or gate failed during this audit.

No `source exists` statement is treated as runtime proof. Checked-in binaries and screenshots are artifacts, not fresh verification. A successful compile proves only the compiled path.

## Immutable snapshots and licensing

| Repository | Audited commit | Branch | License evidence | Main qualification |
|---|---|---|---|---|
| `robotman2412/sapphire-gpu` | `575efe030d2ea69d1448bd43b95f6d7815249b47` | `main` | CERN-OHL-P-2.0 | hardware RTL laboratory, incomplete GPU |
| `UltraOS/Hyper` | `d4f446550ebb5df460e6c5ef8e5759b2d180d0a4` | `master` | MIT | compact bootloader with strong tests |
| `ProvableHQ/snarkOS` | `a8e2e33f6e769386e09c2dd6a32fafd3dc126424` | `staging` | Apache-2.0 | distributed blockchain node, not an OS |
| `dspinellis/unix-history-repo` | checkout initially at `d0d404ce50489bf40a96165ff2fa7291194301b1` | `Research-PDP7-Snapshot-Development` | mixed historical-source permissions in `LICENSE` | synthesized history dataset |
| `felipenlunkes/osdev-projects` | `0736c963d92aebc3c9f6810fb6213372ff87398a` | `main` | BSD-3-Clause | curated index only |
| `marceldobehere/MaslOS-2` | `154d42e53fc0320b38d120acb3bdfd8677fae60f` | `main` | AGPL-3.0 | broad desktop OS, fresh ISO built |
| `alexdboxall/Banana-Operating-System` | `f463540c60727237ea4fba49137d8c9c4ab83b98` | `main` | CC BY-NC 4.0 plus third-party notices | ambitious legacy desktop, study only |
| `KingVentrix007/AthenX-2.0` | `e6734419e24f70b97c4e5ad7d9bff97d17799764` | `main` | `LICENSE.txt` is GPL-3.0; README badge says MIT | license metadata conflict |
| `ktauchathuranga/privilegeos` | `1563a393fe6d6702bcacb4ccded786f4f30b3abe` | `main` | MIT | Linux/BusyBox rescue distribution |
| `yashvi-soni-30/mYOS` | `20bf154d8da58473922c61796875d93f6e4fb900` | `main` | no license found | very small persistence demo |
| `cppsever/MineBios` | `ca9831fa763c637e69b85e244dabe545343d84a8` | `main` | Apache-2.0 | bootable assembly game |
| `viralcode/vib-OS` | `73b64299bf6e591e738c58c9d1845f0babdd6f58` | `main` | README claims MIT; no root license file found | compiling ARM64 kernel with major claim drift |
| `whispem/asm.fm` | `a5572911ba9c21a0782076d2e343ed301c9dc455` | `main` | MIT | verified offline assembly DSP lessons |
| `brainboxdotcc/mission-control` | `a6b7ec4838eff0a51768ea68e08745f17429ada4` | `main` | Composer metadata says MIT; no root license file found | QEMU browser-demo control plane |
| `osdev0/freestanding-c-hdrs-gnu` | `cdd9b3cc11808d3259c082a77c33dfeb0f040adc` | `trunk` | GPL-3.0 plus GCC Runtime Library Exception | generated compiler headers |

License consequence: this project is extracting ideas and behavior, not code. Banana OS's non-commercial terms, MaslOS's AGPL, the historical Unix source permissions, and missing or contradictory license files make that clean-room boundary especially important.

## Comparative feature map

| Repository | Boot/hardware | Execution/services | Storage/network | UI/media | Verification quality | Best clean-room lesson |
|---|---|---|---|---|---|---|
| Sapphire GPU | DMA, SPI, ILI9341, framebuffer/texture RTL `[S]` | serial command/payload engines `[S]` | byte-stream DMA | pixel pipeline | protocol simulations `[T]`, not run | bounded hardware streams and capability descriptors |
| Hyper | BIOS/UEFI, i686/amd64/AArch64 `[S][R]` | Ultra handover/assertion kernel | FAT/ISO, GPT/MBR, PXE/TFTP `[S][R]` | none | 33 BIOS tests passed `[V]` | boot-path matrix and exact-origin semantics |
| snarkOS | host OS process | validator/client/prover/bootstrap roles, plugins, REST | RocksDB, CDN/P2P/BFT sync | CLI/telemetry | large test/CI corpus `[T]` | role-specific authority, lifecycle and operations |
| Unix history | historical architectures | source-control history | historical source snapshots | none | dataset integrity/provenance | preserve evolution and blame continuity |
| osdev-projects | architecture metadata | none | none | none | no schema/tests | discovery taxonomy, not evidence |
| MaslOS-2 | x86_64/Limine, AHCI/audio | tasks, ELF, IPC, app modules | MRAPS/MRAFS/SAF | desktop, WM, many apps/games | fresh ISO build `[V]`; no runtime suite | conversations, stdio-over-IPC, restartable apps |
| Banana OS | x86, extensive device tree | processes, modules, syscall surface | FAT/exFAT/ISO, storage, partial net | full desktop and installer | no CI/runtime gate | packaged desktop breadth and modular devices |
| AthenX-2.0 | GRUB x86, IDE/AHCI/PCI/PS2 | mostly kernel shell, partial ELF/syscalls | FAT/custom FS; UDP/TFTP partial | VESA/TUI/apps | dry build graph only | approachable vertical organization |
| privilegeOS | GRUB BIOS/UEFI Linux image | BusyBox root rescue shell | Linux drivers/NTFS3 | console | syntax only `[V]` | recoverable rescue-media workflow |
| mYOS | multiboot x86, ATA PIO | kernel shell only | raw serialized fixed FS | VGA text | no tests | smallest persistence demonstration |
| MineBios | BIOS FAT12 loader | one 16-bit game | FAT chain read | mode 13h tile game | no test gate | tiny stage loader and bounded game loop |
| vib-OS | ARM64 primary; separate x86 demo | scheduler/process/syscalls, many stubs | RamFS; disconnected/partial ext4/APFS/net | large GUI/media/demo surface | kernel compiles `[V]`; runtime unavailable | claim ledger must trace reachability and behavior |
| asm.fm | Linux x86-64 process ABI | standalone programs | WAV stdout | synthesis/effects | 25 builds and WAV outputs passed `[V]` | deterministic integer DSP test corpus |
| mission-control | launches QEMU/KVM/OVMF | leases, slots, reaper, admin CLI | qcow2 overlays | noVNC web page | PHP syntax `[V]`, tests unavailable | hardened ephemeral public demo service |
| freestanding headers | six compiler targets | generated toolchain inputs | n/a | n/a | six target smoke tests `[V]`; atomic mismatch `[F]` | regenerate ABI headers with provenance |

## 1. Sapphire GPU

Path: `/home/roy/Documents/repos/zl-starred-sources/robotman2412__sapphire-gpu`

### What exists

- `[S]` The repository is a compact SpinalHDL/Scala hardware design: 42 Scala files and roughly 4.5 thousand Scala lines, plus design notes and simulations.
- `[S]` `SerialCommandEngine` decodes a small command protocol: NOP/status, IRQ configuration, DMA setup, debug-register access and generic I/O. Configuration descriptors distinguish required and optional capabilities.
- `[S]` Payload stages separate command decoding from bounded data movement. IRQ state, DMA configuration, device status and stream handshakes are explicit state rather than hidden global side effects.
- `[S]` `AhbLiteToByteDma` adapts an AHB-Lite memory-side transaction stream into byte-oriented device traffic.
- `[S]` SPI components cover master, slave and memory-control use cases.
- `[S]` Graphics-oriented blocks include pixel unpacking, framebuffer scanout, texture read/write and an ILI9341 display path.
- `[T]` Simulation sources exercise command, DMA, SPI and pixel/protocol behavior.

### What does not yet exist

- `[F]` This is not a complete GPU. There is no connected rasterizer, shader execution path, compute pipeline, or top-level deployable design.
- `CrtScanout.scala` is empty; drawing and compute design notes are empty.
- `DmaMux` does not provide a completed arbitration/selection design. Descriptor values such as a zero scanout count expose unfinished composition.
- No generator/top-level connects every advertised block into a tested FPGA or ASIC target.
- The local machine lacked sbt and Verilator, so none of the simulations or synthesis paths are labelled `[V]`.

### Best lesson for zlOS

Use the **protocol shape** for future GPU/device work: query capabilities first; distinguish required from optional functions; separate control commands from bulk payload; make every valid/ready transition bounded; and keep protocol-level simulations independent of a complete device. zlOS should not schedule Sapphire GPU integration now—the value is in its contracts and tests.

## 2. Hyper

Path: `/home/roy/Documents/repos/zl-starred-sources/UltraOS__Hyper`

### Connected feature inventory

- `[S][R]` A compact C bootloader supports i686 BIOS, amd64 UEFI and AArch64 UEFI paths.
- `[S][R]` Storage/filesystem support covers FAT12/16/32, ISO9660, MBR, GPT, raw/whole-device images, optical media, embedded/hybrid layouts, and a selected boot partition.
- `[S][R]` Network boot covers PXE and TFTP.
- `[S][R]` The Ultra handover protocol transports memory, modules, command line, framebuffer and firmware/platform metadata to the kernel.
- `[S][R]` Path identifiers include disk/CD index, filesystem UUID, partition UUID, raw device and exact-origin semantics. Bounds and overflow handling are present around disk, partition and file calculations.
- `[S][R]` Filesystems register through a narrow interface instead of leaking format-specific behavior through the loader.

### Verification

- `[V]` The i686 BIOS bootloader compiled locally with clang.
- `[V]` Six assertion-kernel variants compiled.
- `[V]` `pytest -m bios` completed with **33 passed, 42 deselected** in 59.35 seconds.
- `[T]` The matrix covers normal FAT boot, ISO CD/HDD, UEFI, fragmented files, partition addressing, raw and embedded hybrid images, boot-partition choice, PXE, huge command lines, module descriptions and UEFI information.

### Gaps

- Chainload configuration remains TODO.
- AArch64 handover is implemented for EL1/EL2 and VHE and is represented in the collected test matrix. The remaining TODO is a cache-maintenance abstraction/refinement, not a missing handover.
- UEFI APM is unsupported.
- A hardened-string FIXME remains.
- The local pass is BIOS-focused. It does not convert every UEFI/AArch64 scenario into locally verified runtime evidence.

### Best lesson for zlOS

Create a tiny zl assertion kernel whose only job is to validate the boot contract, then run it across a parameterized matrix of firmware, architecture, medium, filesystem, fragmentation, partition selector and oversized metadata. Preserve exact origin information so the kernel can distinguish “the image containing me” from an arbitrary enumerated disk.

## 3. snarkOS

Path: `/home/roy/Documents/repos/zl-starred-sources/ProvableHQ__snarkOS`

### System structure

- `[S][R]` `cargo metadata` reports **21 workspace packages** for accounts, CLI, display, role-specific nodes, router/messages/TCP, synchronization and locators, CDN, BFT/events/ledger/storage, consensus, REST, metrics and utilities.
- `[S][R]` The runtime has explicit **validator**, **client**, **prover**, and **bootstrap client** roles. Each role constructs only its appropriate services and exposure.
- `[S][R]` The ledger uses RocksDB and automated checkpoints. The network combines peer routing, bounded connection/peer policy, rate limiting and version/SHA-aware handshakes.
- `[S][R]` Synchronization is tiered across CDN, peer-to-peer locators/blocks and BFT/consensus traffic instead of treating every data path identically.
- `[S][R]` Typed messages include challenge, block, peer, ping/pong, puzzle, unconfirmed transaction/solution and disconnect flows.
- `[S][R]` CLI areas cover account operations, cleanup, developer decrypt/deploy/execute/scan/private-transfer commands, node start and update.
- `[S][R]` The REST surface exposes blocks, transactions, solutions, peers, programs, mappings/history, sync state, committees/delegators, BFT and validator telemetry. JWT-authenticated operations cover node address, mapping operations, database backup and Slipstream plugin management.
- `[S][R]` Validators deliberately disable REST in sensitive deployment modes. That is stronger than exposing one universal admin API and relying only on endpoint authentication.

### Slipstream plugin mechanism

- `[S][R]` Dynamically loaded plugins implement canonical mapping and staking-reward hooks.
- Startup order and reverse shutdown order are explicit. Lifecycle callbacks warn and isolate plugin failure rather than taking down the node.
- Runtime load/unload exists, but reload remains TODO. Several BFT mock/translucent/prover paths are explicitly `unimplemented!`.

### Evidence and gaps

- `[T]` Seventy-six Rust files contain test modules; the checkout contains roughly 318 `#[test]` or Tokio test declarations.
- `[T]` CI and scripts cover devnets, restart, upgrade, network and benchmark scenarios.
- `[F]` A full local build was not attempted: the workspace pulls a very large git-based snarkVM dependency graph and exceeds the sensible proof cost on this four-core/15-GB machine. All runtime claims here remain source/CI claims.
- Sync code contains unfinished banning, integrity and rate-policy work. Several specialized consensus/prover components remain mocks or unimplemented.
- `[F]` `--rest-rps 0` passes CLI parsing and reaches `1_000_000_000 / rest_rps`, causing division by zero during REST construction.
- REST authentication is skipped when the global JWT secret is not initialized; direct library construction or explicit no-JWT mode can expose sensitive management routes. This must be an explicit deployment/authority contract, not an incidental global side effect.
- This is blockchain node software. Its consensus, proof and cryptocurrency semantics are not kernel features.

### Best lesson for zlOS and the agent system

Adopt typed **service roles**, not one all-powerful agent daemon. Give every role an exposure profile, peer/resource budget, lifecycle order, shutdown deadline, health/telemetry contract and data checkpoint strategy. Separate bulk bootstrap, ordinary synchronization and authoritative consensus/control paths. Treat dynamically loaded tools like Slipstream plugins: authenticated installation, bounded callbacks, visible failures, reversible load/unload and no power to crash the supervisor.

## 4. Unix history repository

Path: `/home/roy/Documents/repos/zl-starred-sources/dspinellis__unix-history-repo`

### What it is

- `[C][S]` This repository synthesizes historical Unix source snapshots and later public histories into a provenance-preserving Git graph spanning early Unix through modern descendants.
- `[C]` The README describes approximately 850,000 commits, 8,000 merges, 22 imported snapshots, around 41 million lines and about 2,700 contributors. Commit, merge, ref and object measurements were recomputed locally; snapshot, line and contributor totals remain upstream claims.
- `[S]` Named research/snapshot branches and release tags are part of the intended interface. The value is longitudinal blame, diff, authorship and evolution analysis across code that originally lived in different source-control systems or only as releases.

### Audit caveat

The original download was a one-commit shallow checkout on `Research-PDP7-Snapshot-Development` containing only 83 PDP-7-era files. That was not “all history.” This audit detected the error and fetched all remote heads, tags and the full object graph.

- `[V]` `git rev-parse --is-shallow-repository` now reports `false`.
- `[V]` `git rev-list --all --count` reports **849,161 commits**.
- `[V]` `git rev-list --all --merges --count` reports **8,037 merges**.
- `[V]` The local mirror contains **195 origin remote refs**, **39 tags**, and **5,455,351 packed objects** across two packs.
- `[V]` Git reports **1.98 GiB** of pack data; `.git` occupies approximately **2.1 GiB** on disk.

The working tree intentionally remains on the small PDP-7 research branch; the other lineages are present as refs and packed objects rather than simultaneously checked out files.

### Best lesson for zl/zlOS

Preserve design evolution as data. Tag architecture epochs, retain migrations as parent-connected history, record imported/provenance boundaries, and keep test/evidence artifacts close enough to releases that later work can answer *when and why* a contract changed. Do not copy historical Unix code merely because it is convenient: rights differ by lineage and the architectural value is empirical history.

## 5. osdev-projects

Path: `/home/roy/Documents/repos/zl-starred-sources/felipenlunkes__osdev-projects`

### Inventory

- `[S]` This is a curated README, not OS source. It lists projects by family: Unix/direct lineage, Unix-like, DOS, Windows-like, BeOS-like, original and mainframe.
- `[S]` A second taxonomy groups projects by implementation language: Assembly, BASIC, C, C++, C#, Common Lisp, D, Pascal, Rust and Swift.
- Rows attempt to record architecture, family, GitHub availability, activity and license.
- The README contains 219 project bullets and 168 direct GitHub links, plus large language tables.

### Data-quality gaps

- “Active” is a manually judged four-year window and can become stale without a refresh timestamp.
- Several rows are malformed or duplicated, including bad C++ links and a duplicate OS67 entry.
- Platform and license fields are incomplete, and there is no schema, link checker, deduplication test, CI or per-row evidence timestamp.
- A link in this index is a discovery lead, never evidence that a feature exists or works.

### Best lesson

Convert discovery catalogs into normalized, validated data: stable repository ID, upstream URL, immutable snapshot, architecture, language, family, license evidence, last verified date, claim source, and audit status. Generate human tables from that schema rather than editing Markdown as the database.

## 6. MaslOS-2

Path: `/home/roy/Documents/repos/zl-starred-sources/marceldobehere__MaslOS-2`

### Feature inventory

- `[S][R]` A 64-bit x86 kernel boots with Limine and provides a working scheduler, per-task page-table context, kernel/user stacks, priorities, yielding, wait-for-message states and task restart mechanisms.
- `[S][R]` ELF user programs, syscall entry, experimental threads, IPC message queues, conversation IDs and stdio connections over IPC are connected to the build.
- `[S][R]` The desktop includes a window manager, taskbar, start menu, explorer/file dialogs, terminal, editors, calculator, paint, image viewer, audio programs, games and Doom.
- `[S][R]` Hardware/storage includes AHCI, PC speaker, AC97, MRAPS partitioning, MRAFS and SAF archives.
- `[S][R]` SAF is a serialized application/data bundle. The build packages external programs and assets into `external/programs.saf` and installs it into the ISO.
- `[S][R]` Around thirty separate user programs are built and bundled. Desktop/start-menu tasks can be restarted, which is a useful supervisor-like recovery seam.

### Local build evidence

- `[F]` A direct `make -j4` failed immediately because the top-level ISO target runs `cleanExternalFolder` while assuming a parent `objects/` directory already exists. The default clean/build order is therefore not fresh-checkout robust.
- `[V]` After the repository's `make cleanObjFolder` prerequisite, `make -j4` compiled the kernel, libraries and programs, created the SAF bundle, produced a fresh 33,695-sector `MaslOS2.iso`, and deployed Limine successfully.
- The successful build is not a boot test. No automated runtime suite exists; “test” applications are ordinary bundled programs.

### Gaps and risks

- No SMP. Scheduler data and hand-written locking deserve race and interruption analysis before reuse.
- Process IDs are randomly chosen with collision risk. Priority scheduling includes random shuffling rather than a deterministic fairness/deadline contract.
- There are many raw pointers, panic paths, debug branches and provisional comments across the C++ kernel.
- MRAFS/SAF need explicit bounds validation, versioning, checksums and crash consistency before persistent user data is trusted.
- The ELF loader accepts only a pointer, not an input length, and trusts header-controlled offsets/counts/sizes. It does not enforce `p_filesz <= p_memsz`; malformed dynamic metadata with relocations and `DT_RELAENT == 0` can make relocation iteration zero-step forever.
- The build's undocumented clean-order dependency and absence of a boot gate make release artifacts less trustworthy than the source breadth suggests.

### Best lesson for zlOS

Use conversation IDs and typed messages to make application interactions traceable. Implement terminal/application stdio as an explicit IPC service rather than kernel-global console state. Make desktop components supervised, restartable processes. Keep application bundles declarative, but add a signed manifest, hashes, format version, size limits and transactional installation.

## 7. Banana Operating System

Path: `/home/roy/Documents/repos/zl-starred-sources/alexdboxall__Banana-Operating-System`

### Connected breadth

- `[S][R]` The build is an i386 freestanding system despite README references to x86-64. Existing kernels and applications are ELF32.
- `[S][R]` Kernel areas include preemptive tasks/processes, ring transitions, pages/address spaces, scheduler, alarms, signals, mutex/semaphore/pipe/mailbox scaffolding and an extensive syscall surface.
- `[S][R]` Dynamic ELF kernel drivers/modules use relocation and kernel-symbol resolution.
- `[S][R]` VFS support includes FAT, exFAT, ISO9660 and symlinks. Hardware source covers ATA/ATAPI/SATA/SATAPI, PCI/ISA/ACPI/ACPICA, APIC/PIC/PIT, VM86 BIOS services, VGA/VESA, PS/2, floppy, SB16/AC97 and an RTL8139 source path.
- `[S][R]` The UI includes a window/compositing layer, desktop, shell, registry editor, file cabinet, photo viewer, solitaire, minesweeper, text editor and ZIP tooling.
- `[S][R]` CAB packages and an installer form a real distribution layer. The checkout includes a 67.3-MB bootable `installer/BANANA.ISO` and many built artifacts.

### Evidence and claim gaps

- `[F]` There is no useful CI or automated runtime test suite. The GitHub configuration contains funding metadata only.
- Build scripts use Windows absolute paths and a custom `i386-banana-*` toolchain, so the checked-out source is not locally reproducible without reconstructing that environment.
- The README's x86-64 claim is not backed by the active build or tracked artifacts.
- Project TODOs explicitly retain TCP/IP and network-driver work, USB, shared memory, fork/exec, robust semaphores, ext2, copy-on-write, pthread and C++ runtime gaps, even where similarly named scaffolding exists.

### Security findings

- `SysLoadDLL` accepts a user-selected path and loads kernel-level driver code without an evident privilege gate.
- ELF validation largely checks magic while machine/class/data and bounds validation are weak or commented. Header-controlled offsets/counts can therefore reach dangerous loader behavior.
- Many syscalls use raw user pointers without a uniform copy/validate boundary. This is not a defensible process-isolation model.

### Best lesson

Study its breadth, module/device organization, installer and package experience. Do not copy code: the license is non-commercial, the toolchain is bespoke, and the loader/privilege boundary is unsafe. For zlOS, require signed/capability-authorized driver installation and validate an entire module image before mapping or relocation.

## 8. AthenX-2.0

Path: `/home/roy/Documents/repos/zl-starred-sources/KingVentrix007__AthenX-2.0`

### Feature inventory

- `[S][R]` A 32-bit GRUB kernel provides GDT, IDT, PIC, exceptions, syscall entry, FPU and paging scaffolding.
- `[S][R]` Device/UI source covers VGA/VESA, IDE, AHCI, PCI, PS/2, ACPI, serial, SB16 and speaker, plus an e1000 path.
- `[S][R]` Filesystems include a FAT library, custom filesystem and VFS; ELF loader source and separate user programs/libc exist.
- `[S][R]` Kernel shell, editor and TUI give it a visible vertical experience.
- `[S]` Network source covers DHCP, ARP, IPv4, UDP and TFTP.

### Reachability and correctness gaps

- `[V weak]` `make -n all` generated 210 commands and exited zero. The required i686 cross-compiler was not installed, so this proves graph expansion only.
- A checked-in `out/AthenX.bin` is a valid 32-bit static ELF, but there is no fresh ISO/image proof.
- Source discovery appears duplicated between an initial recursive find and dependency generation, risking objects being linked twice.
- Multitasking is a claim gap: `kernel/scheduler/multitasking.c` contains only an empty initializer.
- The syscall handler implements a small read/write/open/close/lseek subset; exit/fork are “coming soon,” `screen_ctrl` is a stub and userspace `syscall_write` is empty.
- Raw user pointers cross the syscall boundary. A `printf(message)` path is a format-string vulnerability.
- `security/user/user.c::validate_credentials` returns success before any lookup or hash comparison, so every username/password combination is accepted; unreachable debug code would also print password hashes.
- Networking is a single-interface learning stack: one-entry ARP behavior, missing checksums/timeouts/random IDs, and no TCP/HTTP.
- Storage, audio and TFTP paths include busy waits without reliable timeout/error contracts.
- No CI or automated tests exist. A large portion of source volume is embedded image/font data, not subsystem logic.

### Best lesson

Its source organization and manuals are approachable, and the shell/user-program packaging demonstrates vertical integration. Treat it as a teaching map, not a concurrency, security, networking or reliability donor. Also resolve the repository's GPL file versus MIT badge conflict before any reuse discussion.

## 9. privilegeOS

Path: `/home/roy/Documents/repos/zl-starred-sources/ktauchathuranga__privilegeos`

### What it actually is

- `[S][R]` A build recipe combines Linux 6.15.3 and BusyBox 1.36.1 into an x86_64 initramfs/rescue image with GRUB BIOS/UEFI and FAT/hybrid media handling.
- `[S]` The BusyBox configuration enables 875 options and the initramfs contains 405 applet symlinks; four are broken in the checked-in tree.
- `[S]` The Linux configuration enables SMP, EFI, initrd, modules, networking/IPv6/wireless, SCSI/ATA/NVMe/USB, DRM/input, virtio, tmpfs, security and NTFS3.
- `[S][R]` Rescue scripts expose disk inspection, NTFS access, logging and a removable-media-oriented workflow.

### Gaps and unsafe scope

- `[F]` No kernel or final boot image is present despite README artifact claims. The checkout contains a partial initramfs and configuration/scripts.
- Build downloads Linux and BusyBox without cryptographic checksum verification.
- The image boots passwordless root shells on multiple TTYs and serial. That may be intentional for offline rescue media but is not a general security posture.
- Windows sticky-keys replacement scripts deliberately create an administrator bypass. That functionality is not an integration candidate. Only the general ideas of explicit target confirmation, backup/restore pairing and logged offline recovery are relevant.
- Kernel module signing and lockdown are not configured. No CI or behavioral tests exist.
- `[V]` Shell syntax checks passed. The full build was not run because it requires privileged image creation, downloads and a large Linux build.

### Best lesson

Ship zlOS with a separately signed rescue image that mounts user disks conservatively, records every repair, requires explicit removable/target-device confirmation, and pairs each mutation with a verified restore path. Never include credential-bypass recipes.

## 10. mYOS

Path: `/home/roy/Documents/repos/zl-starred-sources/yashvi-soni-30__mYOS`

### What exists

- `[S][R]` A 32-bit multiboot C/assembly kernel provides VGA output, a keyboard-driven shell, PS/2 input, ATA PIO and a tiny persistent filesystem.
- `[S][R]` The filesystem serializes one fixed `FileSystem` structure at LBA 1. It uses a fixed entry table, parent indexes and inline file data.

### Gaps

- Most IDT entries exist structurally but point to address zero.
- Extended keyboard state handling is broken.
- Filesystem names lack robust bounds handling; the disk format has no version, checksum, journaling or atomic update. Duplicate names, current-directory/type mistakes and bounded-depth `pwd` behavior remain.
- ATA PIO lacks a reliable DRQ/ERR/timeout/flush contract, so a device fault can hang or lose data.
- There is no paging, protected userspace, process model, test suite or CI.
- No license was found, blocking safe code reuse even aside from the user's clean-room requirement.

### Best lesson

This is useful only as the smallest physical proof that a shell operation can persist a directory/file record to disk and reload it. zlfs must retain its existing stronger direction: versioned on-disk structures, bounds checks, failure injection, atomic metadata updates and cold-boot tests.

## 11. MineBios

Path: `/home/roy/Documents/repos/zl-starred-sources/cppsever__MineBios`

### What exists

- `[S][R]` Nine files implement a 16-bit NASM boot sector/loader and a mode-13h tile game.
- `[S][R]` The loader has a FAT12 BPB, BIOS disk retries and FAT-chain traversal for `KERNEL.BIN`.
- `[S][R]` The game uses a 450-tile-wide chunk/map representation, an 8x5 visible camera, collision/physics, textures and direct pixel/text output.

### Limits

- The loader hardcodes a data-area adjustment (`add ax, 31`), so it is not a general FAT12 loader across arbitrary BPB layouts.
- There is no memory manager, interrupt-driven device layer, filesystem API, processes, protected mode, test suite or CI. It is a game with a boot wrapper, not an OS architecture.

### Best lesson

Keep boot stages small, retry bounded BIOS reads, and follow file allocation chains rather than assuming contiguous content. For a demo/game, use a fixed logical world plus a tiny view window to cap render work.

## 12. vib-OS

Path: `/home/roy/Documents/repos/zl-starred-sources/viralcode__vib-OS`

### What successfully builds

- `[V]` `make kernel -j4` produced a 7,813,904-byte statically linked ARM64 ELF at `build/kernel/unixos.elf`.
- `[R]` The primary Makefile is ARM64-only. CI also builds only the ARM64 kernel; image creation is explicitly allowed to fail.
- `[S][R]` The linked kernel includes UART/GIC/timer, PMM/VMM/heap, scheduler/process scaffolding, signals/fork sources, syscall table, RamFS/VFS, GUI/desktop/window/terminal, media decoders/assets, ELF loader, PCI, virtio input/network/GPU, xHCI/HID/MSD sources, NVMe source, HDA, sockets/DNS/TCP-IP, sandbox and ASLR sources.
- `[S][R]` User init/login/shell binaries are embedded in RamFS and `process_create` is called for `/sbin/init`.
- `[S][R]` That primary `process_create` path explicitly runs programs in kernel space at EL1 without memory protection and supplies raw kernel API pointers. The separate `task_struct`/fork/syscall/userspace sources are not the booted process model; `syscall_init`, `do_fork` and `do_execve` are not wired into a production caller.

### Architecture and build gaps

- The Makefile excludes x86/x86_64 from the primary graph and redundantly adds ARM64 assembly already found by its first source scan.
- The separate `vib-os-x86_64` tree is a stripped standalone UEFI demo, not feature parity with the primary ARM64 kernel.
- The CI “test” uses a timeout and `|| true`, then reports success without checking a boot banner or panic. `scripts/run-tests.sh` checks tools/files and ELF type, not behavior.
- QEMU AArch64 is not installed locally, so the new ELF could not be boot-verified.

### Claim audit

- `[F]` **Python**: terminal help says MicroPython exists at `vendor/micropython/`, but that directory is absent. The `run` command prints source, recognizes literal `print`, hardcodes `add(X,Y)`, and special-cases `fibonacci.py`. It is simulated output, not a Python interpreter. CPython and Node directories contain host download/cross-build scripts, not integrated runtime artifacts.
- `[F]` **NanoLang**: the terminal tells the user to compile from a host-side vendor path and simulates a few print patterns. It is not an in-OS language runtime.
- `[F]` **exec/clone**: `sys_clone` returns `-ENOSYS`. `sys_execve` validates and loads ELF, marks the current task, then returns the entry address with the explicit comment that userspace execution is not implemented.
- `[F]` **memory isolation**: mmap/brk state is global rather than per-process; mmap directly zeroes selected virtual addresses and munmap is a no-op. ASLR is a timer-seeded linear congruential generator, not a cryptographic or strong isolation control.
- `[F]` **SMP**: ARM64 code discovers/counts only CPU0 and comments that secondary CPUs may be started “when ready.” x86 CPU discovery is TODO. No connected multi-core boot path was found.
- `[F]` **sandbox**: a global `current_sandbox` and setjmp/longjmp fault recovery exist, but no callers were found. Its allocated isolated stack is never switched into, and global rather than per-CPU state makes it unsuitable as a security boundary.
- `[F]` **ext4**: implementation source exists but no call site mounts it. It is a classic block-pointer subset, not modern ext4 coverage; unlink is explicitly incomplete and triple-indirect handling is missing. Journaling, extents, metadata checksums and robust feature negotiation are not established.
- `[F]` **APFS**: mount/source exists but `apfs_read_file` is a TODO that returns failure and no boot integration calls the mount.
- `[F]` **networking**: DNS send/wait/parse are TODO; socket connect/receive behavior is stubbed; TCP transmits to a broadcast MAC pending ARP; ICMP reply is TODO. The terminal `ping` command prints four simulated responses. This is not full internet access.
- `[F]` **drivers**: USB MSD read/write are stubs and xHCI does not initialize MSD; NVMe read/write are stubs; Bluetooth send is TODO; ACPI MADT and device-tree memory discovery are TODO.
- `[F]` **security/robustness**: syscall compilation emitted incompatible context-pointer warnings. Sandbox assembly emitted operand-width warnings. The JPEG decoder emits repeated undefined negative-shift warnings.

### Genuine strengths

The repository does contain a large, coherent graphical demo surface and a buildable ARM64 kernel. The desktop/window/terminal and embedded media make early boot visually impressive. The staged initialization order is readable. Those are legitimate strengths, but they must be labelled separately from real filesystems, protected userspace, networking, runtimes and SMP.

### Best lesson

Every zlOS feature needs a machine-readable evidence tuple: claim, source path, build reachability, boot reachability, positive runtime probe, negative/failure probe, architecture/hardware coverage and last verified commit. If a demo command simulates output, label it `demo`, never the name of the unimplemented subsystem.

## 13. asm.fm

Path: `/home/roy/Documents/repos/zl-starred-sources/whispem__asm.fm`

### Feature inventory

Twenty-five standalone Linux x86-64 NASM programs generate PCM WAV on standard output without libc:

- oscillators: square/beep, saw, triangle, noise/LFSR, supersaw and PWM;
- pitch/time: notes, scales, melodies, tempo/BPM conversion and vibrato;
- amplitude/voices: tremolo, strong tremolo, polyphony and fixed-point ADSR;
- modulation/nonlinearity: FM, ring modulation, bit crushing and distortion;
- filters: low-pass, state-variable/filter sweep and auto-wah;
- time effects: delay, chorus and reverb.

The implementations use fixed-point/integer arithmetic, static buffers, explicit saturation and direct RIFF/WAVE header generation.

### Verification

- `[V]` `make -j4` assembled and linked all 25 programs. NASM emitted deprecated implicit-absolute-addressing warnings but no failures.
- `[V]` All 25 executables exited successfully and generated output.
- `[V]` `file` recognized all 25 outputs as little-endian RIFF WAVE, Microsoft PCM, 16-bit mono at 44,100 Hz.
- `[V]` `soxi` parsed all 25 files, with durations from 1.0 to 4.0 seconds.

### Limits and lesson

There is no real-time audio device, mixer API, streaming/ring-buffer contract, test suite, spectral/golden comparison or shared DSP library; examples duplicate common setup. Use the algorithms as clean-room behavioral specifications for zl integer DSP and deterministic test tones. Build a common zl audio buffer API and verify output hashes/properties rather than copying the assembly.

## 14. mission-control

Path: `/home/roy/Documents/repos/zl-starred-sources/brainboxdotcc__mission-control`

### Connected feature inventory

- `[S][R]` A Laravel 12 application exposes anonymous “try” sessions backed by QEMU and noVNC/WebSocket access.
- `[S][R]` A database models preallocated slots and leases. Allocation is guarded by a cache lock; leases receive random 64-character tokens stored hashed in the database and associated with a Laravel session.
- `[S][R]` QEMU uses a base image plus per-session qcow2 copy-on-write overlay. Configurable modes include disk, CD-ROM and USB; Q35, KVM, OVMF and user networking are represented in structured arguments.
- `[S][R]` Hard and idle deadlines, touch/release APIs, an admin CLI, process logs and a reaper provide lifecycle operations.
- `[S][R]` The noVNC page maps a lease/session UUID to a slot-specific `/vnc/<slot>` path.
- `[T]` Feature tests cover allocation, session lifecycle, routes and policy areas. Composer/vendor dependencies were absent locally, so PHPUnit could not run.
- `[V]` PHP 8.4 syntax checks passed for **95 of 95** PHP files across application, configuration, routes, database and test paths.

### Serious gaps

- A QEMU launch failure after slot allocation has no complete rollback, leaving a stuck lease/slot and overlay.
- Multiple DB mutations are not one database transaction; correctness depends on every process sharing a coherent cache lock.
- **Critical:** the session page checks a UUID but does not itself revalidate the secret lease token. Apache exposes enumerable `/vnc/1` through `/vnc/99` WebSocket proxies directly to QEMU, with no lease middleware and no VNC password. Control-plane lease tokens do not authenticate the VNC data plane.
- Anonymous start lacks an adequate admission/rate limit, creating an abuse and resource-exhaustion path.
- The release path sends SIGKILL before SIGTERM despite a “cleanly” comment. PID identity/reuse is not verified.
- Kill/unlink failures are suppressed while the slot can still be marked free, permitting an orphan VM or disk leak.
- Extra QEMU arguments use an incomplete denylist. Structured known-safe options or a strict allowlist are required.
- KVM is always enabled even when a non-KVM accelerator is selected. One non-AHCI controller mode fails to attach its disk; base image format is hardcoded raw.
- There are no cgroup resource ceilings, seccomp/device restrictions or guest egress controls.

### Best lesson for zlOS

This is the direct foundation concept for a public zlOS demo: a prewarmed bounded slot pool, short opaque leases, COW disks, hard and idle expiration, structured launch arguments, logs and a reaper. Harden it with one transactional lease state machine, per-lease proxy authentication, allowlisted QEMU profiles, graceful-then-force termination, PID/start-time identity checks, cgroups, network egress policy, rollback on every partial failure and adversarial concurrency tests.

## 15. freestanding-c-hdrs-gnu

Path: `/home/roy/Documents/repos/zl-starred-sources/osdev0__freestanding-c-hdrs-gnu`

### What it provides

- `[S]` A generated snapshot of GCC compiler-provided freestanding headers for i686, x86_64, AArch64, RISC-V 64, LoongArch64 and m68k.
- `[S]` There are 371 headers: 138 each for i686/x86_64, 28 AArch64, 25 RISC-V, 22 LoongArch and 20 m68k. The two x86 trees are identical in this snapshot.
- `[S][R]` The generator downloads exact binutils 2.47 and GCC 16.2 archives, verifies BLAKE2 checksums, builds cross GCC/libgcc, extracts compiler includes and applies a clang-compatibility patch.
- `[T]` CI regenerates all six targets and pushes each architecture's results.
- Header sets include standard freestanding definitions plus compiler/architecture intrinsics, unwind and gcov support. This is not a libc.

### Local verification

- `[V]` Clang cross-target smoke compilation of `stddef.h`, `stdint.h`, `stdarg.h`, `limits.h`, pointer-size assertions and `INT32_C` passed for all six targets.
- `[V]` Native GCC `stdatomic.h` smoke compilation passed.
- `[F]` Clang `stdatomic.h` smoke compilation failed on all six sets because a GCC macro passes an `_Atomic` pointer shape that clang rejects for `__atomic_load`. The existing compatibility patch is incomplete.

### Best lesson for zl

Compiler-owned ABI headers should be generated from a pinned compiler release, not hand-copied and slowly edited. Record upstream URL, version, digest, generator revision and architecture in a manifest. Test both the intended GCC toolchain and any supported clang compatibility mode. Keep generated headers distinct from zl's libc and from language-level standard modules.

## Cross-repository integration decisions

### Adopt as concepts now

1. **Feature truth ledger** from the failures across vib-OS, AthenX, privilegeOS and Banana: claims must link to source, reachability and executable probes.
2. **Assertion-kernel boot matrix** from Hyper: firmware x architecture x medium x filesystem x partition selector x fragmentation x payload size.
3. **Hardened ephemeral demo service** from mission-control: leases, COW disks, deadlines and reaping, with the security fixes above.
4. **Role-specific services and operations** from snarkOS: separate validator-equivalent trusted roles from clients/workers; disable surfaces roles do not need.
5. **Generated toolchain provenance** from freestanding headers: pinned upstream archives, digests, generated manifests and compiler-matrix smoke tests.
6. **Typed message conversations and stdio services** from MaslOS-2, but implemented across protected process boundaries.
7. **Bounded streaming hardware protocols** from Sapphire GPU.
8. **Deterministic audio probes** from asm.fm for speaker/PCM/mixer validation.
9. **History/provenance preservation** from unix-history-repo for architecture epochs and migrations.

### Study, but do not inherit

- ring-0 dynamic module loading from untrusted paths;
- simulated terminal commands presented as real runtimes/networking;
- raw user pointers at syscall boundaries;
- passwordless privilege escalation or credential-bypass rescue tools;
- hand-maintained feature/link catalogs without validation;
- monolithic filesystem images without version, checksum and crash consistency;
- “test” scripts that ignore the process exit status or never inspect boot output.

## Weakest links in this audit

- The Unix history graph is now fully unshallowed and its commit/merge/ref/object counts were measured. The README's snapshot, line and contributor totals were not independently recomputed.
- snarkOS was source/CI audited but not locally built because its dependency and resource footprint is disproportionate on this machine.
- Vib-OS built but could not be booted because `qemu-system-aarch64` is unavailable locally.
- Banana OS depends on an unavailable bespoke Windows cross-toolchain; AthenX depends on an unavailable i686 cross-compiler.
- mission-control's PHP dependencies were absent, so syntax and source review—not its test suite—were locally verified.
- MaslOS-2 produced a fresh ISO, but there is no automated boot assertion proving the ISO reaches a healthy desktop.
