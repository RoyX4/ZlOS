# All 33 starred repositories: driver, service, and application matrix

Date: 2026-08-21

Scope: every repository in the immutable 33-repository manifest, with its
meaningful device, service, application, utility, game, media, package, and
operations surface normalized into one comparison.

This matrix compresses, but does not replace, the complete inventories in:

- [`MATURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`](MATURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md)
- [`ARCHITECTURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`](ARCHITECTURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md)
- [`FOCUSED_REPOSITORIES_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`](FOCUSED_REPOSITORIES_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md)
- [`ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md`](ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md)

## Evidence and ownership vocabulary

- **`S`** — substantive source inspected.
- **`R`** — active build, image, boot, manifest, registry, route, or package graph
  reaches the source.
- **`T`** — focused test/model/scenario exists; it was not necessarily run.
- **`CI`** — automation invokes the relevant path. `CI-build` is compilation or
  packaging only.
- **`V`** — a specific runtime behavior was locally recorded by this research
  suite.
- **`V-build`** — the exact target built locally, without implying boot/runtime.
- **`F`** — source or an executable check contradicted the broad claim.
- **`stub`** — callable shell/probe/fixed-result surface without the device or
  product behavior.
- **`local`** — implemented in the audited top-level repository.
- **`submodule`** — implemented in an initialized nested repository.
- **`fetched`** — supplied by a locked or downloaded external project.
- **`3P`** — third-party port, library, asset, kernel, or binary recipe.
- **`external`** — product behavior is outside this checkout.
- **`D`** — disabled, removed, or disconnected from the active graph.

Evidence is per feature. A repository-level `V` does not verify every row. A
driver is not complete at discovery or compilation; an app is not complete at
manifest, menu, or package presence.

## Exact 33-repository normalized ledger

| # | Repository | Driver/device reality | App/service/utility reality | Strongest clean-room use | Required rejection/correction |
|---:|---|---|---|---|---|
| 1 | `IanNorris/Brook` | `local S/R`: PCI/ACPI/APIC, VirtIO block/net/GPU/input/RNG, PS/2, xHCI HID/MSC, Bochs, HDA, RTC/KVM clock, watchdog, FAT/ext2/ext4; 12 module targets | `local S/R`: recovery shell/UI, 21 normal static probes, Wayland relay, files, editor, player, console, calculator, package and diagnostic tools; mixed ports/demos | Compile shared kernel logic into host tests; module-import gate; driver hazard ledger; end-to-end input probe | Host wrapper masks failures; unload/IRQ teardown weak; about 60 unsafe user-pointer sites; compositor control lacks authority; 3P packages are not native apps |
| 2 | `Mathewnd/Astral` | `local S/R`: uACPI, PCI/APIC/SMP, framebuffer/serial, AHCI/NVMe/VirtIO block, xHCI/hub/HID, RTL8169/VirtIO net, HDA, many FS | five local admin/trace/profile tools plus distro/session scripts; **307 3P recipes** spanning toolchain, X11, Qt/GTK, WebKit, Wine, media and games | Broad modern-hardware and ports target map; central deny-first authorization listener | Init is ordering-only: no cycle/failure/timeout/rollback/teardown; no native tests/CI; recipes do not prove install or launch |
| 3 | `Bananymous/banan-os` | `local S/R`: ACPI/EC/battery, ATA/AHCI/NVMe/SCSI, xHCI/hub/HID/joystick/MSC, E1000/E1000E/RTL8169, AC97/HDA, framebuffer, ext2/FAT/pseudo devices | **62 local programs**, including login, WindowServer, TaskBar, launcher, clipboard/audio servers, terminal/shell, image viewer, network/admin tools; 16 build-only tests; **128 3P ports** | Coherent desktop/server composition, observability, broad storage/USB/input, pinned/checksummed port recipes | `setreuid(-1,0)` escalation and passwordless login; test binaries not run; Banos module boundary lacks safe unload/provenance; partial TCP |
| 4 | `LemonOSProject/LemonOS` | `local S/R`: ACPI/APIC/PCI/PS2/SMP, ATA/AHCI/NVMe, FAT/tmp/TAR/ext2, IP/UDP/TCP/e1000, AC97/HDA; xHCI compiled but not initialized | five system targets, **22 installed apps**, **9 utilities**, four loadable modules; File Manager, Terminal, editor, media players, monitor, DeviceManager, settings/welcome/games/tests | Generated IDL/interface seam; compact coherent GUI and device-management suite | Endpoint credits mismatch; fork corrupts credentials; setgid fake-success; kill permissionless; HDA largely stubbed; tests and CI are false-green/build-only |
| 5 | `SerenityOS/serenity` | `local S/R/T`: broad PCI/USB/storage/input/network/GPU/audio/serial/time/platform drivers across x86-64/AArch64/RISC-V; shared async/device/connector managers | **29 services, 51 graphical apps, 13 games, 223 C/C++ utilities**; native brokered web/media/parser, settings, accessibility, creation and admin ecosystem | Deepest shared app platform; isolated parsers; server ownership; async driver frameworks; x86-64 target and host sanitizer gates | Secondary-architecture runner skips target tests while reporting zero failures; breadth is not current physical proof; adopt contracts, not scale wholesale |
| 6 | `Meulengracht/MollenOS` | `local S/R`: PS/2, UHCI/OHCI/EHCI, hub/HID/MSC, AHCI, MFS, ValiFS; xHCI/E1000/syslogd are disconnected | seven active services, two local utilities (`osstat`, `systat`), PE/VaFS/package substrate; actual GUI is external `vali-userspace` | Schema-driven driver match/service manifests; policy services; immutable app images | Module cache frees raw image buffer on duplicate PE; service tests mostly unreachable/runner weak; external GUI cannot count as local; lifecycle/revocation incomplete |
| 7 | `hhuOS/hhuOS` | `local S/R`: ISA/PCI/APIC/PIC, VBE, PS/2, NE2000/RTL8139, IDE/AHCI/floppy, FAT/ISO/TAR, SoundBlaster/PC speaker, ports/time/platform | **45 targets**: shell/core/admin/network/audio tools, 2D/3D demos, four native games and multiple 3P emulator/engine clients | Readable central system composition and deliberate workload ladder from terminal to 3D/games | Ring 3 has arbitrary MMIO, mount, kill and shutdown power; raw syscall pointers; build-only CI; submodules/assets retain separate rights |
| 8 | `byteduck/duckOS` | `local S/R`: i686 PS/2/PATA/Bochs/AC97/PCI/time, RPi framebuffer/platform, E1000, ext2/PTY/socket FS | four services (`init`, Pond, Quack, DHCP), **12 app/VM targets**, `dsh`, **22 utilities**; compact bundles and file associations | Small coherent desktop; River typed RPC, Pond compositor, Quack shared audio, app-bundle shape | Guessed SHM IDs permit ownership bypass; TCP/DHCP lifecycle partial; signal-0/group defects; tests not CI-run; buffers need capability/version/bounds |
| 9 | `chittios/chitti` | mostly `local S/R/T`, with Apple M2/m1n1 as bundled submodule/3P: widest device source—x86/ARM/M2, ACPI/FDT, storage/USB/input/display/wired/radio/audio/camera/power/VM integration; many physical paths partial | **48 manifests**, 33 declared WASM modules, 30 UI packages, three autostarts; productivity, browser/media/voice, games, agents, skills, MCP and admin workflows; Freedoom WAD is bundled 3P content | Widest product idea surface; typed manifests/plans; capability and provenance questions; cross-device feature vocabulary | Large ring-0 TCB; local MAC mislabeled Ed25519; foreign install unfinished/nontransactional; uninstall incomplete; kill no-op; subagent grants are reconstructed from a manifest snapshot rather than the parent's live handles |
| 10 | `staycool1374-Ger/nexios` | `local S/R/T/V-build`: framebuffer, PS/2, time/RTC, PCI/MSI/DMA, AHCI/ATA/VirtIO block/net, FAT/dev/proc/tmp; registry is metadata, not true loader | init/vfsd/iocd/shell, file/core/diagnostic programs, one graphical demo and **61 shell registrations** | Explicit fixed budgets, bounded waits, probes and deterministic state testing | Driver registry disconnected from boot init; no TCP/DHCP/DNS/mouse/USB; COPY source right bypass and truncated revoke; local target tests produced no result |
| 11 | `ierwarf/rustos` | local kernel/provider protocols and local DVM agent/provider glue plus a **fetched pinned Linux DVM appliance/3P drivers** for broad hardware; IOMMU/VFIO, authenticated block/net/input/display transport, narrow native fallback | 13 local Rust service daemons, shell and six qualification apps plus PE demo and compatibility libraries; fetched Linux DVM appliance; no consumer suite | Strongest provider ownership, fail-closed handoff, device epochs, formal/fuzz/failure receipts | Fetched Linux breadth is not local driver code; DVM adds availability/attestation/recovery TCB; physical GPU recovery has failed evidence; heavy service graph |
| 12 | `zinnia-os/zinnia` | `local S/R/V-build`: narrow modules for NVMe, VirtIO block/net/GPU, IGC, xHCI/hub/HID/MSC, ext2; kernel PCI/DRM/input/network/time/VFS | no first-party userland distribution; only kernel `/init`, TTY/PTY, sockets and platform APIs | Cleanest small local driver-module and common-bus organization | Dynamic loader lacks complete dependency/init/fini/unload lifecycle; malformed ELF can panic; uACPI hooks stub; compile-only CI and no boot proof |
| 13 | `skift-org/skift` | local microkernel plus boot framebuffer, x86 COM/PIC/PIT, CMOS, port I/O, PCI, PS/2 keyboard/mouse and Strata device/input/power/CM/FS/shell; no local block/USB/NIC/audio/camera/GPU | advertised Hideo/Karm/Vaev/Luna/Games product is **fetched and absent locally**; ideas list is a wishlist | Typed object API, service placement and locked-project bill of materials | Send-side capability acquisition has rollback, but inverted destination-capacity accounting makes receive-side transfer unusable, so transactional receive is unproven; map failure is separately non-atomic; target spawn/network/PTY/DNS/sandbox APIs unsupported |
| 14 | `jezze/fudge` | `local S/R`: ACPI/APIC/PIC/PIT/RTC/UART/PCI/SMP, typed device/audio modules, IDE/ATA/ATAPI/VirtIO block, BGA/Cirrus/VGA/VBE/partial i915, RTL8139/VirtIO net, PS/2; AHCI/NVMe/USB hosts are stubs | shell/filesystem/network/admin tools, WM plus seven GUI tools, Game Boy emulator, PCX tools, demos and manual probes | Typed mailbox/event services and real ring-3/per-task page-table mechanism | Oversize receive destroys message; ambient IDs/global pools; unsupported events drop; ELF unsafe; fixed-pool failure weak; drivers lack cancel/reset/remove tests |
| 15 | `UnmappedStack/TacOS` | `local S/R`: ACPI/APIC/PIT/PCI/serial/PS2/framebuffer, PTY, AF_UNIX/local sockets, USTAR/TempFS; broken NVMe; no persistent block/external network/audio/USB | init, shell, cat/ls/echo/clear, Vim-like `dim`, WM, graphical terminal, info, `helloworld`, local Doom engine with bundled 3P WAD, and three probes | Smallest genuinely connected desktop vertical slice | Negative syscall index; ELF permission inversion and `filesz > memsz`; raw shared buffers; no persistent editor proof; no automated gate |
| 16 | `rayanmargham/NyauxKC` | `local S/R partial`: x86/RISC-V kernel, framebuffer/flanterm, uACPI, PCI ECAM, HPET/LAPIC/serial, VT-d pass-through, VirtIO-GPU display-info only, RAMFS/USTAR | no syscalls, ELF or userland; wallpaper/archive assets are not apps | Honest staged bring-up labels and a compact GPU transport probe | `pci_map_bar` reverses bus/function positions when reading BAR configuration; USTAR stores header as payload; pass-through is not DMA isolation; polling lacks timeout; four build recipes overstate two source ports |
| 17 | `CorruptedByCPU/Cyjon` | local assembly kernel: x86 SMP/ACPI/APIC/PS2/RTC/serial/framebuffer/IPC; no PCI/block/USB/net/audio; initialized pinned Fern-Night submodule supplies VFS userland | submodule Fern WM, console, shell, cat/ls/free/ps/uptime and privileged GC daemon; two menu actions point to removed apps | Cross-language ABI conformance and generated menu-from-artifact lesson | Both build wrappers mask errors; raw user pointers and magic-only ELF; user fault can halt system; submodule app dependency; weak framebuffer-owner lifecycle |
| 18 | `robotman2412/sapphire-gpu` | `local S/T`: command/status/IRQ, capability descriptor, DMA, SPI, framebuffer/texture/ILI9341 blocks; no complete connected GPU | no OS apps; simulation targets are device-protocol tests | Best small required/optional capability and split command/payload vocabulary | DMA mux select never changes; CRT scanout empty; raster/compute docs empty; no connected rasterizer/shader/top-level or local simulation run |
| 19 | `UltraOS/Hyper` | `local S/R/T/V`: BIOS/UEFI, i686/amd64/AArch64, FAT/ISO, GPT/MBR/raw/PXE/TFTP, Ultra handover | configuration/menu/boot service, not an app ecosystem; six assertion kernels and 75 scenarios | Best executable boot-origin/media/firmware assertion matrix | Only 33 BIOS scenarios locally passed; collected UEFI/AArch64 cases unrun; chainload missing and AArch64 cache TODO remains |
| 20 | `ProvableHQ/snarkOS` | no OS hardware drivers; host network/storage stack for a blockchain node | 21 packages: validator/client/prover/bootstrap roles, P2P/CDN/BFT/sync, REST, metrics, plugins, checkpoints, CLI and operations | Role-specific supervisors, exposure, bounded peers, lifecycle, telemetry and checkpoints | Not an OS/app UI donor; JWT can be disabled/uninitialized and zero REST RPS divides by zero; plugin reload/specialized paths incomplete |
| 21 | `dspinellis/unix-history-repo` | historical source corpus, not a present driver graph | Git history/refs/blame are the user-facing research tool; no OS app delivery | Provenance-preserving architecture evolution and evidence history | Rights are lineage-specific; generated dataset is not its reconstruction pipeline; historical presence is not current feature proof |
| 22 | `felipenlunkes/osdev-projects` | none; metadata links only | manually curated repository catalogue, no app implementation | Candidate discovery taxonomy | No schema/tests/CI; stale/duplicate/malformed rows; list membership is never implementation evidence |
| 23 | `marceldobehere/MaslOS-2` | `local S/R/V-build`: PCI AHCI/SATA, AC97/PC speaker, custom partitions/FS/SAF, graphics/input/platform | roughly 30 bundled desktop modules/programs: explorer, terminals/shell, editors, paint, calc, music, dialogs, games/demos; conversation/stdin/stdout IPC | Conversation IDs, stdio over IPC, declarative SAF app bundle and cohesive desktop composition | Lengthless ELF loader trusts offsets and can zero-step relocations; FS lacks integrity/transactions; raw-pointer C++; fresh ISO has no healthy boot oracle |
| 24 | `alexdboxall/Banana-Operating-System` | `local S/R`: i386 PCI/ISA/DMA/APIC/ACPI, UART/PS2/floppy/VM86/VGA/VESA, IDE/AHCI, audio modules; RTL8139 present but network incomplete | local Regedit, file cabinet, clipboard test, shell/console/desktop, games, photo/text/ZIP; package/CAB/installer system | Broad legacy driver/app packaging and distribution UX | Active build is i386; user can request ring-0 ELF module load without sound gate; loader validation incomplete; USB/net/fork/SHM/COW incomplete; non-commercial license |
| 25 | `KingVentrix007/AthenX-2.0` | `local S/R`: i386 VGA/VESA, PCI/ACPI, PS2, IDE/AHCI, ISA DMA, AC97/SB16/speaker, e1000; UDP/TFTP learning stack | kernel shell/TUI/editor/manuals plus user amix/bin/byte/cat/edit/Kilo/shell/simulator/tar/TCC/test/init/libc | Readable teaching vertical structure and manuals | Credential validation immediately accepts all; multitasking and syscall paths stubbed; format-string user pointer; networking not robust; no tests/fresh build |
| 26 | `ktauchathuranga/privilegeos` | **external** Linux 6.15.3 device breadth configured by image builder; sound disabled | BusyBox rescue shell, drive/mount/backup/restore/write/QEMU scripts; no independent desktop apps | Separately signed conservative rescue environment and reversible repair workflow | Inputs unpinned by digest/signature; passwordless root; credential-bypass tool out of scope for product; absent built image/runtime; Linux features are not local drivers |
| 27 | `yashvi-soni-30/mYOS` | `local S/R`: VGA, PS2 keyboard, ATA PIO, fixed serialized in-memory FS structure | shell with help/clear/about/color/echo/reboot/halt/ls/touch/write/cat/rm/mkdir/cd/pwd | Minimal shell-to-persistent-disk vertical proof | ATA can wait forever and ignores ERR/DF/flush; FS ignores directory/type and lacks bounds/version/checksum/journal; no processes/tests/license |
| 28 | `cppsever/MineBios` | `local S/R/V-build`: FAT12 BIOS chain loader and direct BIOS keyboard/RTC/mode-13h game | one tile game with bounded viewport, movement/collision/camera/interaction | Tiny staged boot and bounded rendering workload | Hard-coded FAT geometry; build target launches interactive QEMU; no assertions; game appliance is not an OS service model |
| 29 | `viralcode/vib-OS` | `local S/R/V-build/F`: ARM64 linked graphics/media kernel with many nominal drivers; actual NVMe/USB MSC/DNS/APFS/FAT paths stub/disconnected | embedded EL1 desktop/apps, terminal, media, launcher; separate userspace tree disconnected; Python/ping output simulated | Visually rich demo and negative evidence-receipt example | No protected production userspace, real Python/ping/SMP/sandbox/storage/network; CI treats timed-out run as success; build presence wildly overstates behavior |
| 30 | `whispem/asm.fm` | no device driver; 25 Linux x86-64 fixed/integer DSP programs | oscillators, sequencing, envelopes, modulation, distortion, filters, delay/chorus/reverb; all 25 produced valid WAV locally | Deterministic audio golden vectors and saturation/chunk invariance tests | Offline duplicated examples, no mixer/device/ring/clock/backpressure/portability; behaviors must be independently reimplemented |
| 31 | `brainboxdotcc/mission-control` | QEMU host control plane, not guest driver code | Laravel slot/lease/COW-overlay/QEMU/noVNC lifecycle, admin commands, logs, reaper and feature tests | Ephemeral public demo operations model | Lifecycle token does not protect enumerable VNC/WebSocket plane; allocation/launch nontransactional; termination frees slot without identity/death proof; no containment/egress limits |
| 32 | `osdev0/freestanding-c-hdrs-gnu` | generated compiler ABI headers for six target architectures, not device drivers | toolchain generation/CI utility only; no OS apps/libc/POSIX | Immutable compiler-input manifest and target/compiler smoke matrix | Transitive downloads not fully pinned; GCC atomics fail under Clang on all six tested targets; generated headers do not prove runtime/ABI completion |
| 33 | `Ellicode/protOS-kernel` | `local S/R/F`: active PS2 + PCI, framebuffer/console, VFS/TAR/devfs/PTY/process/IPC; ACPI omitted; no storage/net/audio/USB | shell, protofetch, ls/read/panic, user WM, terminal; edit unfinished; eight targets build-connected but full build fails | Compact VFS/devfs/PTY/IPC/shared-surface/user-WM vertical relationship | Inverted ELF validation, null syscall, raw user pointers, unbounded/racy IPC, UAFs, ambient SHM and globally writable framebuffer; no tests/clean build |

## Driver-family comparison

### Platform, discovery, interrupts, DMA, and power

| Capability | Strong positive ideas | Partial/negative evidence to turn into tests | zlOS destination |
|---|---|---|---|
| Boot/firmware | Hyper assertion kernels; Serenity architecture split; current zlOS GOP/ZLLOG | false-green timeouts in vib; recipe/source architecture overclaims in Nyaux; Serenity non-x86 test skip | artifact-bound BIOS/UEFI/media/origin matrix; platform provider with explicit architecture parity |
| ACPI/DT | Serenity broad ACPI/DT platform graph; Astral uACPI integration; banan EC/battery observability | Zinnia uACPI stub hooks; protOS ACPI disconnected; partial AML frequently promoted as complete | firmware parser/service with full hook contract, typed resources, bounds, failure and suspend states |
| PCI/device discovery | Zinnia narrow binding; Serenity device management; Mollen manifest matches | Nyaux `pci_map_bar` bus/function reversal; enumeration-only mistaken for device support; missing multifunction/high-BAR cases | immutable generation-tagged identity, deterministic matching and one authoritative registry |
| IRQ/MMIO/DMA | RustOS provider ownership/IOMMU/VFIO model; Sapphire stream protocol | hhu arbitrary user MMIO; Brook unsafe control; Nyaux pass-through called isolation; ring-0 module loaders | explicit resource handles, per-device default-deny DMA domains, revoke-before-free and late-event rejection |
| Time/power/sensors | Serenity clock/platform breadth; banan battery → TaskBar; Chitti multi-platform surface | missing timeout/failover/resume; unavailable values represented as success/zero; ambient shutdown | read-only typed sensors first; power policy service; suspend only after every provider proves quiesce/resume/fallback |

### Storage, USB, input, display, network, audio, and media

| Family | Best input | Other useful input | Rejected evidence/pattern | Required zlOS proof |
|---|---|---|---|---|
| Block/storage | Serenity common async request/management; Brook host-tested real FS logic | Zinnia narrow NVMe/VirtIO; Astral breadth; RustOS broker; current NVMe/zlfs | infinite polls in mYOS; Fudge AHCI/NVMe stubs; vib stubs; TacOS broken NVMe; recipe/external claims | checked spans, buffer ownership, queue/cancel/reset, flush/barrier, media removal, power cut, cold integrity and hardware receipt |
| USB | Serenity core + UHCI/EHCI/xHCI + hub/HID/BOT/UAS | banan xHCI; Mollen service split; Zinnia narrow modules; current zlOS xHCI HID/MSC | Fudge host stubs; Lemon compiled-uninitialized xHCI; descriptor presence; weak unplug cleanup | malformed descriptor corpus, topology churn, short/error transfer, halt/reset, unplug in flight and generation-safe reuse |
| Input | Serenity HID/device breadth; current PS2/xHCI | RustOS input broker; Skift/Fudge event separation; current I2C-HID raw diagnostic transport | ambient grabs, direct device/framebuffer access, missing disconnect releases, raw I2C bytes called an input provider | normalized event schema, descriptor decoder, per-seat policy, bounded/loss-counted queues, reconnect, focus/grab authority and accessibility |
| Display/GPU | Serenity connector/adapter split; current GOP/software/Intel/BGA/virtio paths | RustOS provider boundary; Chitti virtual breadth; Sapphire capabilities | Nyaux display-info called GPU; empty Sapphire raster/CRT blocks; global framebuffer mappings; source-only acceleration | atomic validate/commit, bounded surfaces, fences/timeouts/reset, second modeset, fallback, scene equivalence and named Intel/QEMU receipts |
| Network | Brook real-logic host tests; Serenity stack/services | Zinnia conventional stack; RustOS provider; snarkOS role/operations; current virtio-net/browser | static/partial stacks promoted as network; vib fake ping/DNS; RTL8139 file as proof; no timeouts/recovery | NIC simulator with loss/dup/reorder/delay/reset; separate link/packet/DHCP/DNS/TCP/TLS/browser receipts; physical wired proof |
| Audio | Serenity/banan server + hardware separation; duck Quack | Brook HDA; MaslOS AC97; hhu workload; asm.fm golden behavior | Lemon HDA stub, source-only audio, offline WAV mistaken for streaming | fake provider then real provider, bounded client rings, clock, format negotiation, mix/clipping goldens, xrun, device loss/reset and mic permission |
| Camera/radio/other | Chitti supplies partial UVC/Wi-Fi/BT taxonomy | Serenity/hhu serial/pseudo devices; privilegeOS external Linux discovery | partial scan/identify promoted as connectivity; capture without visible authority | defer until common contract; explicit discoverability/capture policy, pinned firmware, hostile descriptors and real hardware profiles |

## Application and service-family comparison

| User/product family | Best observed source | Important secondary examples | zlOS application contract |
|---|---|---|---|
| Session, launcher, taskbar | Serenity services/apps; banan server suite | duck Sandbar/Pond; Lemon shell/login; Chitti onboarding | authenticated session root, generated registry, readiness, safe logout/lock, searchable launch and failure explanation |
| Files and productivity | Serenity FileManager/editor/spreadsheet/presenter; Chitti 48-package breadth | Brook files/editor; banan/duck/Lemon suites; MaslOS explorer/editors | capability file portal, transactions, safe save/autosave/recovery, isolated parsers and persistent schema migration |
| System/admin | Serenity settings/device/users/partition/monitor/crash; Lemon DeviceManager | banan monitors/battery, NexiOS shell/probes, RustOS services, Fudge tools | typed read-only observability plus explicit privileged-operation rights, redaction, rollback and degraded states |
| Terminal/development | native terminals/shells across most OSes; Serenity debugger/tooling | Brook diagnostics/package tools; hhu workload; future self-hosted zl | PTY/process tree, hostile arguments, child cleanup, editor/build/debug/provenance tools as ordinary processes |
| Browser/network/comms | Serenity browser/content/request services and Mail/Maps/Weather | Chitti browser/download/git/research/mail; Brook/Fudge web tools; Astral 3P WebKit | separate UI, brokered fetch/TLS and disposable hostile-content parsers; scoped file/network rights and privacy controls |
| Media/creation | Serenity PixelPaint/Piano/players/3D/font/theme; Chitti media/paint/synth/recorder | Lemon/Brook players, duck viewer, MaslOS suite, asm.fm vectors | decoder workers, AudioServer/media provider, versioned projects, safe save, route/capture permission and recovery |
| Accessibility | Serenity Magnifier and settings are the clearest native examples | hhu/duck shared UI libraries; current zlOS keyboard routes | keyboard-only/focus/semantic labels, scaling/high contrast/reduced motion, per-user persistence and inspectable metadata |
| Games/ports | Serenity 13 native games; hhu workload ladder; current zlOS 24 named games | Chitti games, MaslOS, duck, Fudge emulator, Astral/banan port catalogues | deterministic rules and seed replay plus process/event/timer/render/audio/save-state/crash/quota proofs; separate code/data licenses |
| Packages/updates | banan reproducible recipes; Mollen/duck manifests; MaslOS/Banana bundles | Serenity launch/file services; mission-control COW; RustOS receipts | signed identity, content-addressed stage, all-object validation, atomic publish/update/rollback/uninstall and explicit user-data policy |
| Agents/automation | Chitti typed plans/workflows corrected by RustOS evidence and snarkOS roles | NexiOS bounds; Fudge event envelopes | ordinary isolated orchestrator, tiny broker, live attenuated handles, consent, deadlines/revoke, child cleanup and independent receipt review |
| Recovery/public demo | Hyper boot matrix; mission-control lifecycle; privilegeOS rescue shape | snarkOS operations/checkpoints; unix-history provenance | signed rescue image and transactional public VM leases with data-plane auth, containment, exact process identity and audit |

## Complete-inventory receipts

The three deep dives preserve the long lists so the matrix does not hide breadth:

- all 12 Brook modules and every reachable local/port/probe layer;
- all 307 Astral recipes;
- all 62 banan programs, 16 build-only tests and 128 ports;
- all Lemon system/application/utility/module targets;
- all 29 Serenity services, 51 applications, 13 games and exact 223-utility list;
- every reachable and disconnected Mollen driver/service/app target;
- all 45 hhuOS application targets;
- all duckOS services, apps, shell and 22 utilities;
- all 48 Chitti packages and native product facilities;
- all 61 NexiOS shell registrations and boot/service programs;
- every local RustOS service, qualification app and compatibility payload;
- every Zinnia driver module and the explicit absence of a first-party userland;
- every local Skift service plus the fetched/absent product boundary;
- every Fudge driver/stub/service/utility/GUI/emulator/demo;
- every TacOS app and device class;
- the exact absence of Nyaux userland and exact VirtIO-GPU bring-up level;
- every shipped Fern-Night daemon/app and every dead menu action;
- every focused repository's device/app/operation surface, including all 25
  asm.fm DSP programs and all protOS programs.

This is exhaustive at the meaningful build/product target level of the pinned
trees. It does not claim that every vendored helper, generated file, historical
branch, uninitialized external dependency, or individual source function was run.

## Clean-room donor ranking

| Need | Primary source | Corrective/secondary source | Final zlOS rule |
|---|---|---|---|
| Driver module shape | Zinnia | Mollen + Brook | narrow module, schema-generated match/dependency/import graph, atomic publication and real unload |
| Device ownership | RustOS | Serenity + current zlOS fallbacks | brokered provider epochs and typed resources; retain boot-safe native fallbacks |
| Driver lifecycle | Serenity patterns | NexiOS bounds + cross-repo failures | lifecycle state machine includes cancel/reset/remove/restart, not only probe/I/O |
| Driver protocol tests | Brook + Hyper | Sapphire + asm.fm | host real-logic/protocol goldens plus artifact-bound full-system assertions |
| App/service platform | Serenity | duck + banan + Lemon | reusable brokered services and generated bounded IPC rather than per-app privilege |
| Product breadth | Chitti + Serenity | current zlOS catalogue | track user jobs and shared dependencies; never copy source or chase raw app count |
| Packaging | banan ports | Mollen/duck/MaslOS/Banana negative cases | pinned inputs, signed manifests, atomic install/update/uninstall and launch receipts |
| Operations | snarkOS | mission-control + privilegeOS | role-specific supervision, checkpoints, recovery and public-demo containment |
| Truth/evidence | RustOS | Hyper + unix-history + false-green cases | immutable claim/source/build/launch/scenario/recovery/hardware receipts with independent contradiction |

## Direct consequences for current zlOS

1. Keep GOP/software, ZLLOG, the Intel display investigation, xHCI HID/MSC, NVMe,
   zlfs and virtio-net/browser paths as retained assets; wrap them before replacing
   them. Intel write/modeset evidence is host-harness-only until native zlOS boot
   arms it; physical xHCI storage evidence does not promote HID keyboard/pointer.
2. Fix the generated app inventory before expanding app count. The present
   Catalogue checker can return success while ID 14 is blank/unroutable and Maze
   is implemented but excluded.
3. Introduce provider boundaries in this order: block, process/session, NIC,
   display/input, then audio. This preserves current performance priorities.
4. Migrate System Monitor/Kernel Log/System Info/Services/Disk/Network first,
   then Files/Editor/state apps, then desktop/session policy, then deterministic
   tools and all games.
5. Use the existing 24-game set as a process/event/timer/render/audio/save-state
   conformance workload, not as proof that the app platform is already isolated.
6. Add Device Manager, Welcome, package/update/recovery, accessibility and media
   families only on the shared contracts; do not grow privileged monoliths.

## Evidence boundary and weakest link

The deep dives are source/build-graph audits. The suite has narrow local builds
and tests recorded elsewhere, but no report here upgrades unrun QEMU or physical
paths. The weakest link remains runtime and hardware qualification—especially
device reset/removal/suspend, full application launch matrices, accessibility,
and recovery after provider/service death. Those gaps are preserved explicitly
in the implementation backlog rather than hidden by source counts.
