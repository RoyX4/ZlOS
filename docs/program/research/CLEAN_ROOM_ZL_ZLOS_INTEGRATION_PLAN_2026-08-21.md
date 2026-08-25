# Clean-room integration architecture for zl, zlOS, agents, and the application ecosystem

Date: 2026-08-21

Status: complete target architecture and dependency-ordered execution plan. This document proposes future work; it does not relabel unimplemented work as current capability.

## Authority and non-negotiable constraints

This plan is grounded in the current orientation and evidence, in this order:

1. zlOS's current hardware truth and immediate performance work: [`kernel/HANDOFF.md`](../../../kernel/HANDOFF.md), [`performance-architecture-roadmap.md`](../../../kernel/docs/performance-architecture-roadmap.md), and [`retained-window-surfaces.md`](../../../kernel/docs/retained-window-surfaces.md).
2. zlOS's system dependency spine: [`docs/EXECUTION-ROADMAP.md`](../../EXECUTION-ROADMAP.md), checked against the older audit ledger in [`STATE-OF-THE-PROJECT.md`](../../STATE-OF-THE-PROJECT.md). The execution roadmap and current handoff supersede stale rankings in that ledger. The active order remains retained surfaces/damage/deadline work, async storage, 64-bit processes, real networking, then load-bearing GPU composition and broader applications.
3. zl's language direction: the retained [`HANDOFF.md`](language/HANDOFF.md), [`ULTIMATE_PLAN.md`](language/ULTIMATE_PLAN.md), [`ROADMAP.md`](language/ROADMAP.md), and [`MASTER_PLAN.md`](language/MASTER_PLAN.md) snapshots. The language spine remains imports, scoping/errors, records/maps, exact integers, types/unboxing, FFI, comptime/security, ecosystem, and eventual source self-hosting.
4. The evidence-normalized external research: [`ALL_33_FEATURE_MATRIX_2026-08-21.md`](ALL_33_FEATURE_MATRIX_2026-08-21.md), the detailed [`ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md`](ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md), the 33rd repository's dedicated [`PROTOS_KERNEL_AUDIT_2026-08-21.md`](PROTOS_KERNEL_AUDIT_2026-08-21.md), the first-pass [`MATURE_OS_AUDIT_2026-08-21.md`](MATURE_OS_AUDIT_2026-08-21.md), [`ARCHITECTURE_OS_AUDIT_2026-08-21.md`](ARCHITECTURE_OS_AUDIT_2026-08-21.md), and [`FOCUSED_REPOSITORIES_AUDIT_2026-08-21.md`](FOCUSED_REPOSITORIES_AUDIT_2026-08-21.md), overridden where necessary by the independent [`MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md), [`ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md), and [`FOCUSED_REFUTATION_AND_COVERAGE_2026-08-21.md`](FOCUSED_REFUTATION_AND_COVERAGE_2026-08-21.md), plus fresh build evidence in [`ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md`](ADDITIONAL_EXECUTABLE_EVIDENCE_2026-08-21.md), the focused driver/app architecture in [`DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md`](DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md), and the earlier [`OS_REPOSITORY_SURVEY_2026-08-20.md`](OS_REPOSITORY_SURVEY_2026-08-20.md).

Locked rules:

- **Do not replace what is already real.** Keep the current compositor, retained-surface direction, zlfs, ZLLOG, network protocol code, browser, Intel display work, xHCI/NVMe work, visual system, applications, and software renderer.
- **The Intel display work remains the physical-hardware north star, not current native-boot proof.** The host harness has physical modeset evidence, but native zlOS does not yet arm that write path. UEFI GOP remains the guaranteed fallback until the native artifact proves modeset, second modeset, teardown, reset, and recovery.
- **The current language sequence remains primary.** Do not derail imports/scoping/errors/data shapes/exact integers/types to translate drivers prematurely.
- **The full destination is preserved.** Staging controls risk and dependencies; it does not redefine the target as a tiny MVP.
- **Ideas are clean-room inputs, not source imports.** Re-specify behavior from public concepts and observed contracts, implement in zl/zlOS conventions, and leave independent conformance evidence.
- **Current dirty checkouts are shared work.** This architecture assigns migration seams; it does not authorize overwriting concurrent implementation.

## End-state architecture first

The complete target is a self-hosting zl toolchain and a general-purpose, physically proven zlOS desktop in which applications, system services, drivers, agents, and public demo infrastructure share one capability/evidence model.

```text
                         zl source / packages / skills
                                    |
                 +------------------+------------------+
                 |                                     |
        interpreter oracle                    typed compiler pipeline
        and fixpoint proof              LLVM speed + own native backends
                                                      |
                                  PE / ELF executable / ELF relocatable
                                  ABI manifests + generated zlIDL stubs
                                                      |
  +--------------------------- zlOS USER SPACE ---------------------------+
  |                                                                      |
  | Session/App Manager     Agent Orchestrator       Package/Port Manager |
  | Window/Audio/Input      Tool Broker              Build/Test Services  |
  | File/Storage providers  Network/Resolver/TLS     Browser/Decoders      |
  | Device/Driver services  Logger/Telemetry         Terminal/Editor/Apps  |
  |                                                                      |
  | Every process: private address space, bounded queues, opaque handles, |
  | monotonic rights, declared resources, deadlines, restart policy,     |
  | versioned protocol, provenance, and evidence receipts.               |
  +-------------------------------+--------------------------------------+
                                  | zlIDL: bounded bytes + typed handles
  +----------------------------- zlOS KERNEL -----------------------------+
  | Threads/scheduler | address spaces | faults/usercopy | handle tables |
  | IPC/wait/futex     | clocks/timers  | IRQ ownership   | DMA/IOMMU     |
  | process lifecycle | capability derivation/revoke     | primitive VM  |
  | minimal early boot/storage/display paths needed for recovery          |
  +-------------------------------+--------------------------------------+
                                  |
  +-------------------------- HARDWARE PROVIDERS -------------------------+
  | UEFI/BIOS/raw boot | ACPI/PCI | NVMe/AHCI/USB block | HID | NIC      |
  | Intel display/render + GOP fallback | HDA/audio | VirtIO/QEMU models |
  +-------------------------------+--------------------------------------+
                                  |
           ZLLOG + feature ledger + receipts + independent review
```

This is not a pure microkernel mandate. Early-boot and currently load-bearing code may remain in the kernel until a service replacement has equal boot, failure, recovery, and hardware evidence. The stable boundary is the contract, not a marketing label.

## Placement: what belongs where

| Layer | Owns permanently | May remain temporarily | Must not live there |
|---|---|---|---|
| Firmware/stage zero | minimal UEFI witness, image identity, bounded pre-kernel failure record, handover | platform-specific compatibility | filesystem policy, apps, model/agent logic |
| Kernel | threads, scheduling, address spaces, page faults, usercopy, typed handles, IPC/wait, clocks, interrupt/DMA/IOMMU resource ownership, primitive process lifecycle | zlfs/boot display/critical drivers until services are proven | browser, package policy, GUI toolkit policy, agent planner, media parsers, general admin APIs |
| Driver/provider process | one device or tightly related controller family, bounded queues, reset/stop/recover, counters | a kernel module for hardware that cannot yet be isolated | global PCI enumeration authority, arbitrary MMIO, unrelated devices, package/network policy |
| Core system service | namespace/VFS broker, block/page cache, network/socket broker, compositor, audio mixer, input routing, session manager, package manager, telemetry supervisor | adapters over old kernel APIs during migration | raw unvalidated user pointers, ambient global object IDs |
| Application | terminal, editor, Files, Settings, System Monitor, browser UI/content processes, games, tools | current kernel-resident app while its process/IPC replacement is gated | direct hardware access or kernel-global UI state |
| Agent process | model runtime, planning, memory/context, tool selection, subagent orchestration | no privileged exception | direct syscalls to arbitrary files/network/devices, ring-0 execution, unsigned tool loading |
| Tool broker | grammar registry, capability checks, plan execution, taint/provenance policy, cancellation, audit receipt | narrow kernel helper only for handle validation | model inference, unconstrained shell, hidden authority escalation |
| Host operations | build farm, image signing, public QEMU demo pool, artifact registry | recovery tooling | secrets inside guest images, unauthenticated control paths |

## System-wide contracts

### 1. Feature truth contract

Every meaningful feature has one machine-readable record:

```text
id, owner, source_commit, source_targets, shipped_targets,
state = claimed|source|reachable|built|booted|scenario_passed|hardware_passed,
dependencies, capability_surface, failure_policy,
host_test, qemu_test, hardware_test, last_artifact_digest,
known_hazards, skipped_gates, license/provenance, reviewer
```

State promotion is monotonic and mechanically checked. `source` cannot jump to `scenario_passed`; `hardware_passed` names the exact artifact and machine. A test/model/port recipe from a dependency is never counted as native coverage.

### 2. zlIDL protocol contract

Every service method declares:

- protocol and interface version;
- bounded request and response sizes;
- typed handles and required rights;
- synchronous deadline or asynchronous completion/cancellation;
- queue-capacity owner and backpressure result;
- idempotency/retry semantics;
- peer-death, service-restart, and version-mismatch behavior;
- authority returned or delegated;
- trace fields and conformance/fuzz cases.

The generator emits zl client/server code, kernel validation metadata, a trace decoder, protocol documentation, boundary/fuzz corpus skeletons, and tests for empty/full/one-over-limit queues. Handle transfer is `preflight -> reserve -> move -> commit`, with complete rollback on failure.

### 3. Resource/lifecycle contract

Every driver, service, process, package operation, and agent tool declares a finite state machine and bounded resources. The default lifecycle is:

```text
Absent -> Starting -> Ready -> Degraded -> Recovering -> Stopping -> Absent
                                   |                         |
                                   +--------> Failed <-------+
```

Every transition has an owner, maximum wait, cancellation path, cleanup result, counter, and receipt. “Wait forever,” “kill and mark free,” and “ignore exit status” are forbidden.

### 4. Authority contract

Processes receive opaque handles, never ambient global IDs. Rights are typed (`read`, `write`, `map`, `signal`, `wait`, `transfer`, `derive`, `admin`) and can only be reduced. Child grants are subsets; bounded derivation supports revocation. Exec receives a manifest baseline and fork inherits the reduced set. VFS, process control, network, device, clipboard, window capture, agent tools, and package install all enforce the same authority model.

### 5. Evidence receipt

Each gate emits:

```text
feature + source commit + toolchain digest + artifact digest
machine/QEMU profile + action + expected + observed + exit status
timestamps + logs/journal slot + screenshots/hashes where applicable
failure injection + skipped scope + reviewer + weakest link
```

ZLLOG remains the early-boot and hardware flight recorder. Later file-backed logging supplements it; it never deletes the raw early-boot path.

## Idea provenance and clean-room disposition

| Repository | Take as a behavioral/design input | Explicitly do not copy/inherit |
|---|---|---|
| Brook | host harnesses around real kernel logic, scheduler-module boundary tests, driver hazard ledgers | fixed pools without pressure policy, incomplete IRQ/device teardown, wholesale Linux-ABI scope before processes |
| Astral | declarative init ordering, centralized deny-first authorization listeners, modern hardware questions, ports breadth | no init cycle/failure/rollback/teardown semantics; enumeration-as-completion |
| banan-os | pinned/checksummed port recipes, SHM damage GUI, candid maturity table | critical `setreuid` privilege escalation, username-only login, unload-less drivers |
| LemonOS | generated IPC, service registry, SHM/damage/occlusion | deterministic endpoint-credit mismatch, fork credential corruption, fake-success `setgid`, permissionless kill, false-green tests |
| SerenityOS | monotonic authority, process-separated parsers/services, compositor/audio boundaries, host and x86-64 target tests, multiarch boot CI, post-test FS check | non-x86 target runner skips tests; entire custom application/library scale as one milestone |
| MollenOS | kernel/service boundary, provider schemas, immutable application images | invalid duplicate-cache destructor call, false-green service-test reach, incomplete ownership/revocation |
| hhuOS | readable composition root and educational subsystem map | Ring 3 without meaningful authority: arbitrary user MMIO mapping, mount, kill and shutdown; raw user pointers |
| duckOS | small Window/Audio/RPC services, SHM audio ring, loader split | shared-memory ownership bypass, SafePointer continuation, VM off-by-one, message bounds and weak TCP timers |
| ChittiOS | typed tool grammar, compiled plans, grant attenuation, consent, provenance/audit shape | built-ins use a baked-key MAC mislabeled Ed25519; foreign signed ingestion unfinished; non-atomic install/uninstall; ring-0 orchestrator |
| NexiOS | explicit queue/pool/deadline bounds and deterministic state rewind | not a derivation-tree donor: source COPY unchecked and broad revoke truncates; no local self-test summary; RT claims remain unmeasured |
| RustOS | ownership/authority contracts, failure-atomic admission, formal/source mapping, preserved failed receipts | architectural ceremony without a product gate; models treated as implementation proof |
| Zinnia | narrow separately built drivers, generated shipped-module inventory, compact Unix object coverage, locally verified release compilation | build is not boot; uACPI hooks fake success/panic; ELF and VM admission are not failure-safe |
| Skift | typed object handles and send-side transfer rollback | receive transfer is broken by inverted capacity; VM mapping is not failure-atomic; product layers are externally fetched |
| Fudge | versioned event envelope, reactive service composition, real Ring-3 per-task page tables | ambient resource IDs, globally shared kernel regions, destructive oversize-message handling, unsafe ELF tooling, no verification |
| TacOS | small boot-to-init-to-terminal-to-file smoke path | negative syscall index, ELF write inversion and `filesz > memsz` overflow; raw usercopy and false-green lint |
| Ellicode/protOS-kernel | compact Limine-to-Ring-3 vertical path with ELF processes, RR scheduling, VFS/USTAR/devfs/PTYS/AIO, pub/sub/shared-memory IPC, and a userspace framebuffer WM | ordinary build failures; inverted ELF magic and unchecked image ranges; raw user pointers; NULL syscall entry; ambient share/IPC; IPC-consume and scheduler-exit use-after-free; unbounded TAR/path/devfs parsing |
| NyauxKC | honest roadmap, simple CPU-local queues and VFS boundaries | reversed PCI tuple, USTAR copies header as payload, passthrough-only IOMMU, recipes as port proof |
| Cyjon | readable boot/state layouts | masked exit status, magic-only ELF, raw user pointers plus global halt exception path, restrictive license |
| Sapphire GPU | required/optional device capabilities, separated control/payload, ready/valid simulations | incomplete top-level/GPU blocks or source copied into the Intel path |
| Hyper | assertion-kernel boot matrix, exact-origin paths, compact filesystem registration | assuming BIOS-local proof establishes all UEFI/AArch64 paths |
| snarkOS | explicit roles/exposure, ordered lifecycle, checkpoints, health/telemetry, bounded peer/resource policy, reversible plugins | blockchain consensus/proof semantics in zlOS |
| unix-history-repo | provenance-connected evolution, tagged architecture epochs, evidence history | historical Unix source or unverified upstream dataset counts |
| osdev-projects | normalized discovery taxonomy | hand-edited links/activities/licenses as evidence |
| MaslOS-2 | conversation IDs, stdio over IPC, restartable app modules | raw-pointer queues, custom FS integrity model, copyleft source reuse |
| Banana Operating System | installer/package UX and broad modular device organization | unsafe kernel-module loading, bespoke toolchain assumptions, non-commercial code |
| AthenX-2.0 | approachable vertical manuals/source organization | raw user pointers, shell/network demos presented as concurrency/reliability/security donors |
| privilegeOS | separate rescue image, explicit target confirmation, paired backup/restore | password bypass tooling, passwordless general system, unverified downloads |
| mYOS | smallest persistence smoke scenario | unversioned monolithic filesystem structure and indefinite ATA polling |
| MineBios | tiny boot stage, bounded disk retries and FAT-chain traversal | hardcoded geometry or game wrapper as OS architecture |
| vib-OS | adversarial claim-ledger examples | simulated runtime/network output, disconnected/stubbed subsystems, timeout-success CI |
| asm.fm | deterministic integer DSP/test tones | offline WAV programs as mixer/device architecture |
| mission-control | prewarmed slots, opaque leases, COW disks, hard/idle expiry, logs/reaper | nontransactional leases, slot-only VNC auth, kill-before-grace, unrestricted resources/egress |
| freestanding-c-hdrs-gnu | generated ABI headers from pinned compiler releases and cross-target smoke matrix | compiler headers treated as libc, GCC headers assumed clang-compatible |
| Lunaris (prior survey) | custom-language-to-ELF-relocatable kernel shape: named sections, packed layouts, extern/asm symbols, relocations | source reuse; it has no declared license and is not self-hosting |

## zl language and compiler work required by the complete system

The OS must not force a forked “kernel zl” that permanently diverges from ordinary zl. Kernel/user ABI needs become staged language features on the existing sequence.

### Preserve the active language order

1. **Imports/module identity** so standard and platform modules stop being copied.
2. **Local-by-default scoping plus explicit global and error handling** so inference, closures, services, and failure returns have stable semantics.
3. **Records/maps/tuples/optionals/enums** so protocols, manifests, ASTs, hardware descriptors, and process state have names rather than magic list slots.
4. **Exact-width integers and byte buffers** before FFI, binary formats, DMA addresses, BARs, descriptors, filesystems, packets, or cryptography.
5. **Gradual types and unboxing** so adding an annotation cannot change semantics and hot OS/application code can leave boxed dispatch.
6. **FFI and explicit allocators/lifetimes** for host libraries and temporary C/ABI bridges.
7. **Comptime and security/capability types** for generated protocols, manifest validation, signed skills, and policy checks.
8. **Package/build/test/LSP/formatter/embedding tools**, then complete source self-hosting.

### OS-specific compiler convergence gates

After the corresponding general language features exist:

| Need | Compiler/language capability | Gate before OS use |
|---|---|---|
| register/packet/FS exactness | `u8/u16/u32/u64`, signed variants, checked/wrapping operations, byte slices | differential interpreter/LLVM/native semantics at boundaries and overflow cases |
| driver speed | native bitwise lowering, shifts/rotates, no name dispatch/boxing | generated IR/assembly inspection plus benchmark against equivalent C and interpreter output |
| MMIO/ports | typed `volatile`, `in/out`, barriers/fences, explicit unsafe/danger marker | host mock verifies ordering; QEMU/hardware transcript verifies effect |
| layouts | packed/aligned records, `sizeof/alignof/offsetof`, endian helpers | ABI/layout manifest agrees with C/firmware definitions on every target |
| callbacks/interrupt tables | function values with typed calling conventions and `extern` symbols | call/return/register preservation tests; no general user callback in IRQ context |
| freestanding kernel/user output | no-CRT mode, named sections, symbol visibility, relocations, linker contract | readelf/objdump manifest, load test, exact section/symbol/relocation assertions |
| process programs | ELF64 executable/PIE, stack/aux-vector ABI, debug/unwind metadata | assertion loader plus hostile malformed-ELF corpus |
| modules/drivers | ELF64 relocatable output and signed manifest | whole-image validation before mapping; capability-authorized load; unload/revoke test |
| generated IPC | compile-time schema/code generator and stable wire layout | cross-version client/server conformance and fuzz tests |
| self-hosting | compiler rebuilds itself and produces matching toolchain artifacts inside zlOS | fixpoint plus artifact provenance; never a stored-hash-only test |

Do **not** translate `intel.c`, `xhci.c`, or other load-bearing drivers merely to increase a zl line count. Prove the compiler path first on a small non-critical provider with a working C behavior oracle. Assembly boot/trampoline primitives may remain assembly permanently.

## Subsystem architecture

### Boot, firmware, and evidence

- Keep the existing UEFI witness and ZLLOG partition safety contract.
- Generate a boot DAG with prerequisites, critical/optional status, deadline, retry, teardown, and receipt fields while preserving a human-readable composition view.
- Build a Hyper-style assertion kernel that validates memory maps, framebuffer, ACPI, modules, command line, exact boot origin, image identity, and oversized/fragmented metadata.
- Preserve BIOS multiboot, raw 512-byte, and native UEFI paths as distinct evidence lanes; do not call one proof universal.

Required boot matrix:

| Dimension | Scenarios | Minimum assertion |
|---|---|---|
| firmware/path | UEFI witness chain, BIOS multiboot, raw loader | correct artifact identity, origin, memory map and visible failure code |
| architecture/mode | current i386 regression, x86-64 production; future ports separately | exact ABI/page mode; unsupported paths refuse loudly |
| medium | NVMe, xHCI USB, QEMU virtio/model, read-only recovery image | capacity/block size/range/flush and selected-origin identity |
| filesystem/layout | FAT ESP, raw ZLLOG, zlfs; fragmented/oversized/invalid images | correct mount/refusal and no writes outside the selected range |
| memory profile | minimum supported, normal, high-memory, holes/reserved regions | no fixed-address collision, allocator bounds, matching shipped module inventory |
| failure | corrupt GPT/superblock, absent device, timeout/reset, truncated kernel, stale backup GPT | bounded refusal/recovery, fallback boots, failure persists in ZLLOG |
| output | GOP/software always; Intel path opt-in until promoted | desktop or deterministic serial milestone; scene/artifact identity recorded |

### Processes, memory, scheduling, and IPC

- Implement the current execution roadmap's 64-bit Ring-3 path: dedicated kernel stack, conservative `iretq`, one PML4 per process, U/S separation, NX/W^X, guard pages, kill-on-user-fault, and complete-span `copy_from_user/copy_to_user`.
- A process owns PID, credentials/session, address space, threads, handle table, limits, parent/supervisor, exit status, and evidence identity.
- Scheduler starts with fair per-CPU queues and exact continuation ownership. Display/input latency receives a bounded class; background writeback/build/agent work has admission and throttling. No blanket “real-time” claim.
- Add waitable typed kernel objects and transactional zlIDL endpoints only after the malicious-process fault gate passes.
- Process control (`signal`, inspect, debug, kill) checks capabilities and credentials. Unknown syscalls/protocols return explicit errors.
- The syscall table is generated or statically asserted so no reachable number has a NULL handler. Process/IPC teardown tests deliberately consume the last message and exit a scheduled task under sanitizers; this converts protOS-kernel's concrete use-after-free shapes into permanent negative regressions.

### Storage, zlfs, and recovery

- Keep zlfs. First place it behind one block contract shared by NVMe, USB, future AHCI, and test images: `identify/capacity/block_size/read/write/flush/trim-or-unsupported`.
- Add bounded request queues, asynchronous completion, page/block cache, dirty accounting, and a writeback worker. IRQ/input/frame paths never perform filesystem or USB writes.
- Deepen zlfs before adding breadth: format/version negotiation, checksummed generations, data-before-metadata ordering, explicit barriers, replay/recovery, disk-full behavior, stale-handle handling, concurrent rename/write, corrupt-image refusal, and power-cut injection between write boundaries.
- Migrate Settings, editor/Files, browser history/bookmarks/downloads, packages, logs, and user data to named files. Delete old numbered RAM slots only when a migration gate proves no callers.
- Add directories and per-process roots/permissions after process identity and handles exist. Path resolution begins from a directory capability, never ambient global root.
- Retain ZLLOG as bounded raw early-boot/crash evidence. Later Logger service imports/export records to files; a bad filesystem cannot erase the early recorder.
- Ship a separately signed recovery image that mounts conservatively, requires explicit target confirmation, records mutations, and validates a restore path.

### Graphics, desktop, input, and GPU

- Complete the already measured retained architecture in the existing WM: client surfaces, then shell/chrome/shadow surfaces, then desktop/dock caching where evidence justifies memory.
- Separate app-content invalidation from screen damage. Move/raise/exposure composes cached surfaces and must not call unchanged apps.
- Replace lossy bounding-box merging with a bounded disjoint/area-aware region representation and counted full-damage fallback.
- Add atomic WM commits, one newest pending visual frame, input-driven wake, deadline pacing, and correlation from HID completion through present.
- When processes arrive, move each app behind a Window service surface/event protocol. Server owns window metadata, validates dimensions/stride/format/damage/handles, and retires buffers with fences/generations.
- Keep software rendering as the byte/scene oracle and safe fallback. Intel GPU acceleration starts only after retention/lifecycle is correct, uses fences/reset/hang recovery, compares development scene hashes, and never makes GOP/software boot unavailable.
- Existing visual identity, workspaces, applications, and reference-widget oracle remain product assets; protocol migration must preserve their behavior rather than restart visual design.

### Audio and media

- Add an Audio service with bounded per-client rings, negotiated sample format/rate/channels, stream lifetime, volume/mute, disconnect cleanup, underrun/overrun counters, and one hardware mixer/provider interface.
- Use deterministic integer tones/DSP properties inspired by asm.fm to validate PCM, resampling, mixing, clipping, latency, and channel order.
- Start with a QEMU/virtual audio path and a software null/file provider, then HDA hardware with bounded codec discovery, stream reset, DMA and interrupt recovery.
- Run image/font/archive/audio/video parsers in restricted disposable processes with file/surface handles, never raw global filesystem/network/device authority.

### USB, storage controllers, and other drivers

- Preserve the physical xHCI/ZLLOG chain and existing NVMe/Intel work.
- Standardize driver descriptors: IDs, dependencies, claimed BAR/ports/IRQs, DMA/IOMMU need, capability/version, queues, `probe/start/online/quiesce/stop/recover`, counters and last error.
- USB core owns topology/control transfers; HID, mass storage and future audio/network classes become separable providers. Descriptor parsers are length-first and fuzzed.
- A driver earns “complete” only after parser/register tests, queue model, QEMU I/O where available, injected timeout/reset/error, repeated init/teardown, exact hardware receipt, and unplug/replug/concurrency where hardware permits.
- Add AHCI/VirtIO/E1000/I219/HDA using the same contracts only when they serve the execution roadmap. AX201 Wi-Fi remains later because firmware/radio transport is a separate system.

### Networking and browser

- Keep `net_link(send,poll,mac)` and make virtio-net one NIC provider rather than the network API.
- Add a Network service that owns NIC queues and returns socket handles. Stage and gate Ethernet, ARP, IPv4/IPv6 policy, ICMP, UDP, DHCP, DNS, TCP, then TLS.
- Host simulations run the real protocol state machines with loss, duplication, reorder, delay, congestion, retransmit, wraparound, exhaustion, reset and clock control.
- Real-network order remains USB Ethernet/tethering or a supported adapter, then the ThinkPad I219 with carrier available, then AX201.
- Move the browser into processes after Ring 3: UI process, restricted network/request process, content/layout process, and disposable decoders. Preserve the existing bounded HTML/CSS/PNG/JS work and explicit unsupported-feature refusals.
- Persist history/bookmarks/downloads through File service; never claim general web compatibility from one successful fetch.

### Applications, sessions, packages, and ports

- Current kernel-resident apps continue using zlfs and retained surfaces while process foundations land.
- Migrate in dependency order: non-graphical assertion app -> terminal -> editor/Files -> System Monitor -> Settings -> browser -> remaining apps/games.
- Session service owns authentication, identity, launch, focus/session resources, clean logout, and crash restart. No username-only login or unverified elevation.
- Packages contain signed manifests, content-addressed read-only program/resources, requested capabilities, protocol/ABI versions, dependencies, writable mount declarations, provenance, and test receipt.
- Port recipes pin source URL/commit/archive digest, patches and dependency digests. “Available” requires build, install, launch/smoke and artifact receipt. Vendored tests do not become zlOS coverage.
- Add a package/app catalog only after install/uninstall/upgrade/rollback are transactional and power-cut tested.
- POSIX/Linux compatibility is an ecosystem multiplier after native processes/files/IPC work; it is not the kernel's identity and should grow by program-driven conformance tests, not syscall count.

### Agent runtime and installable tools

- Model runtime and orchestrator are ordinary isolated processes.
- Intent becomes a typed, inspectable plan. The broker accepts only registered grammar, validates handles/scopes/provenance/consent, executes deterministically, and emits a receipt.
- Agent roles are explicit: orchestrator, interactive client, bounded worker/subagent, tool provider, reviewer, and optional remote model gateway. Each role has a different exposure profile and resource budget.
- Subagents receive strict capability subsets, deadlines, output/artifact budgets, cancellation, and automatic revocation. They cannot widen grants.
- Skills/plugins are signed packages with requested ceilings, approved grants, versioned callbacks, load/unload, reverse-order shutdown, bounded failure isolation and telemetry. Runtime reload is not promised until proved.
- Taint/provenance travels with values/artifacts. It blocks configured sinks but is labelled accurately; syntactic provenance is not claimed to be complete information-flow control.
- Independent review consumes the plan, diff/artifacts and receipts, tries to refute completion, and cannot silently rewrite the executing agent's evidence.
- User journal/personal notes, credentials, private messages, and unrelated data remain outside agent authority unless explicitly granted for one task.

### Public demo hosting and operations

- Build a hardened mission-control-style service around the real zlOS image: prewarmed finite QEMU slots, one transactional lease row/state machine, high-entropy short lease secret, COW overlay, hard/idle expiry, health, logs and reaper.
- Proxy/VNC/WebSocket access is authenticated per lease, not per slot/UUID. QEMU profiles are allowlisted; extra arbitrary arguments are forbidden.
- Launch rollback removes partial lease/overlay/process state. Termination is graceful then forceful, verifies PID plus process start identity, and does not free a slot until death and cleanup are confirmed.
- Apply cgroup CPU/RAM/I/O/process limits, seccomp/device restrictions and guest egress policy. Rate-limit anonymous creation and cap concurrent leases by role.
- Base images and boot artifacts are immutable and digest-addressed. Per-lease disks are disposable; optional saved demos export explicit user artifacts, not the whole machine.
- Operations expose role-specific health, queue depth, checkpoint, build/version and current-artifact identity. No universal unauthenticated admin API.

## Integrated dependency plan

Two active tracks proceed without stepping on each other, then join at explicit gates.

```text
zl language track
imports -> scoping/errors -> records/maps -> exact ints/bytes -> types/unboxing
       -> FFI/allocators/comptime -> packed ABI/relocatable output -> self-host tools
                                      |                    |
                                      | JOIN A             | JOIN C
                                      v                    v
zlOS system track
retained client/shell -> regions/deadlines -> async block/zlfs -> 64-bit processes
       -> handles/zlIDL -> services/apps -> real NIC/browser -> GPU/audio/packages
                                      |                    |
                                      +---- JOIN B --------+
                                                |
                                  capability-native agents + in-OS rebuild
```

### Phase 0 — preserve and inventory the proven baseline

Build:

- materialize the feature ledger and shipped-module inventory;
- attach exact artifact/machine identities to existing ZLLOG, zlfs, compositor, browser/network, Intel, xHCI, NVMe and build gates;
- classify old docs as authority/history/superseded without deleting evidence;
- make every outer build/test gate propagate inner failures.

Exit gate: every current headline feature has a source/reachability/test/hardware status and a named weakest link; no current artifact is silently rebuilt or replaced.

### Phase 1 — finish the measured desktop architecture (current immediate work)

Build: retained client surfaces, retained shell/chrome/shadow, allocation fallback, separate invalidation, desktop measurement, area-aware regions, atomic commit, input-driven newest-frame pacing.

Exit gate: on the same physical ThinkPad interaction trace, unchanged covered windows execute zero app/shell redraws during another window's move; no stale pixels; `late=0`, `lost=0`; HID-to-route p95 remains below 1 ms; visible frames meet the existing 16.67 ms contract; recorder drops stay zero; software rendering remains selectable.

### Phase 2 — asynchronous block layer and crash-consistent zlfs

Build: common block provider, bounded queue/completion, cache/writeback worker, zlfs version/generation/barrier/recovery work, named-file migration, ZLLOG integration without early-path removal.

Exit gate: save/journal I/O never runs in IRQ/input/paint context; NVMe and USB providers pass one contract suite; forced reset between each metadata write boundary yields old or new committed state; disk-full/corrupt/stale-handle tests are deterministic; cold-reboot app path reopens exact bytes.

### Phase 3 — real 64-bit processes and hostile-user boundary

Build: syscall entry/return, process object, per-process PML4, guarded stacks, W^X, usercopy, kill-on-fault, scheduler/CR3/kernel-stack switching, minimal ELF/assertion program.

Exit gate: a malicious program cannot read kernel memory, map/write device memory, cross an invalid pointer, make W+X memory, disable interrupts, corrupt another process, or kill the desktop. Its fault is recorded and the desktop continues.

### Phase 4 — handles, zlIDL, supervision, and sessions

Build: typed handle tables, rights attenuation/derivation/revoke, wait/IPC, transactional transfer, generated protocols, service supervisor, credentials/session/authentication.

Exit gate: empty/full/over-limit and peer-death/restart tests pass; cap transfer is all-or-nothing; a child cannot widen rights; revoked handles fail; malformed messages do not mutate state; login/elevation/process control require explicit authority.

**Join A with zl:** records, errors, exact integers/bytes and stable layout are required before zlIDL becomes the permanent source language for these protocols. Until then, use a narrow generated bridge, not duplicate handwritten ABIs.

### Phase 5 — migrate system services without losing fallbacks

Build in order: Logger/export -> File/VFS broker -> block providers -> Window/Input -> Audio -> Network/Resolver/TLS -> Package -> isolated parsers. Each service first wraps current functionality, then assumes policy ownership.

Exit gate per service: versioned protocol conformance, restart/peer-death behavior, capability test, bounded queue/deadline, old/new path differential scenario, and rollback. Remove an old in-kernel policy path only after the new path has QEMU and relevant hardware proof.

### Phase 6 — real wired networking and browser process split

Build: NIC selection, supported USB/wired provider, I219 where physical carrier exists, DHCP/DNS/TCP reliability, socket service, browser UI/content/request/decoder split, durable user browser data.

Exit gate: a physical link earns separate receipts for carrier, DHCP, DNS, lossy/reordered TCP transfer, TLS policy, browser load and persisted download. Network service crash/restart does not crash the desktop or leak another app's sockets.

### Phase 7 — application/process ecosystem and reproducible packages

Build: migrate terminal, editor/Files, System Monitor, Settings, browser and remaining apps; stable native ABI; package manifests/signatures; transactional install/upgrade/rollback; reproducible ports; optional POSIX compatibility driven by chosen programs.

Exit gate: clean image installs a signed app, grants only declared handles, launches, saves through zlfs, survives reboot, upgrades, rolls back after injected failure and uninstalls without orphan state. A port is listed only when the same artifact passes install-and-launch smoke.

### Phase 8 — hardware providers, audio, and load-bearing Intel composition

Build: standard driver lifecycle, deeper USB classes, AHCI/VirtIO/E1000/I219/HDA as justified, Intel render composition behind the scene oracle, system-visible counters/recovery.

Exit gate per driver: parser/queue host tests, QEMU where modelled, injected timeout/reset, repeated init/stop, hardware transcript, resource revocation and fallback. Intel acceleration must improve measured physical timing, match the software scene, recover from a forced hang and fall back without losing the desktop.

### Phase 9 — zl-native systems toolchain convergence

Build: complete the language sequence first; then exact system types, native bitwise/MMIO/barriers, packed layouts, calling conventions, function values, ELF64 executable/relocatable output, named sections/relocations, generated ABI manifests, debug metadata.

Exit gate: interpreter/LLVM/own-native semantics agree on boundary corpus; a small non-critical zl provider compiles independently, loads through the signed manifest path, passes differential behavior and unload/revoke; no 64-bit address loses bits; the fixpoint remains green.

**Join B:** new user services/apps are written naturally in zl once its types/errors/modules support their contracts. Existing C remains a migration oracle, not a deadline-driven rewrite target.

### Phase 10 — capability-native agents and tool ecosystem

Build: userspace orchestrator, deterministic broker, typed plans, role manifests, signed skills, provenance/taint/citation policy, attenuated subagents, independent reviewer and persistent non-sensitive receipts.

Exit gate: adversarial prompts cannot invoke ungranted file/network/device/process tools; subagents cannot widen authority or outlive deadline/revocation; tool failure is explicit and reversible; reviewer can reject a false completion; agent crash cannot crash kernel/desktop.

### Phase 11 — hardened public demo and operations

Build: transactional lease service, allowlisted QEMU, per-lease proxy auth, COW overlays, resource/egress isolation, expiry/reaper, metrics and artifact identity.

Exit gate: concurrent allocation has no double lease; every injected launch/kill/storage/proxy failure rolls back or remains visibly quarantined; expired sessions lose access; orphan process/overlay count is zero; guest cannot escape resource or egress policy.

### Phase 12 — in-zlOS rebuild, self-hosting distribution, and later architectures

Build: compiler/build/package tools as ordinary processes, source and binary package graph, deterministic in-OS rebuild, then architecture ports through the same contracts.

Exit gate: zlOS rebuilds the zl compiler and a bootable matching system artifact from declared sources inside zlOS; the fixpoint and artifact manifests agree; the rebuilt image boots through the assertion matrix. A secondary architecture is never promoted from target JSON/linker script alone—it must pass its own boot/process/service/hardware evidence lane.

**Join C:** source self-hosting becomes a product property only here. Deleting bootstrap C earlier would discard working oracles before the replacement can refute itself.

## Migration map from today's system

| Current asset | First seam | Final placement | Removal condition |
|---|---|---|---|
| existing WM/compositor | retained client then shell surfaces | Window service + compositor process/provider | never replace visual behavior; remove direct app hooks only after process protocol parity |
| kernel-resident apps | retained surface + explicit state/save API | ordinary app processes | each app passes workflow, crash isolation and persistence gates |
| direct framebuffer/GOP | buffer/display handle interface | display provider; GOP fallback retained | GOP fallback is not removed |
| Intel modeset/ring C | lifecycle descriptor and telemetry | kernel/provider split chosen by evidence | only after second modeset/teardown/reset/fallback hardware proof |
| zlfs direct builtins | common block contract and named file API | File/VFS + zlfs provider | old numbered slots have zero callers and migration proof |
| NVMe/xHCI storage | common block tests/queue | controller providers + cache/writeback service | direct early path can remain for recovery if documented |
| virtio-net global path | NIC-provider interface | Network service owns sockets | current QEMU browser path remains a regression oracle |
| browser in current system | file/network/window adapters | UI/content/request/decoder processes | end-to-end fetch/render/persist/error parity |
| scheduler/demo Ring 3 | 64-bit process object/usercopy | process manager + kernel scheduler | 32-bit proof remains regression lane |
| raw builtin calls | generated adapter | zlIDL protocols and handles | caller/build inventory reports no legacy use |
| C support/drivers | ABI manifest + differential oracle | zl modules where valuable | never rewrite load-bearing code merely for purity |
| ZLLOG raw partition | Logger imports/exports | early raw recorder + file-backed runtime logs | raw recorder remains while FS/services can fail before ready |
| manual app/source bundles | signed immutable package manifest | Package service/store | transactional upgrade/rollback and provenance gate |

## Risk register and controls

| Risk | Failure mode | Control |
|---|---|---|
| Architecture rewrite | replacing current working desktop/storage/drivers with an external project's shape | migrate one boundary at a time; old path remains oracle/fallback until parity proof |
| Language/OS sequence collision | driver rewrite steals effort before imports/types/exact ints; OS ABI forks language | explicit joins A/B/C; temporary generated bridge; one language semantics |
| Evidence inflation | source/test/model/artifact called working | state machine and receipts; independent reviewer; booted artifact self-identifies |
| False-green automation | wrapper ignores exit, timeout treated success, suite logs but exits zero | strict outer exit propagation, intentional failing canary, guest success/failure protocol |
| Unsafe vertical-demo code | impressive boot-to-WM path hides broken ELF admission, NULL syscalls, raw user pointers or lifetime bugs | require complete loader bounds/magic/class/machine checks, generated syscall inventory, hostile usercopy corpus and sanitizer teardown tests before UI breadth counts |
| Capability complexity | handle bugs undermine core security | property/fuzz tests at empty/full/transfer/revoke boundaries; small typed kernel surface |
| Service explosion | debugging/restart overhead exceeds benefit | migrate only after process/IPC observability; group boot-critical policy until split pays |
| Data loss | async cache or package update acknowledges before durable commit | explicit sync semantics, generation/checksum, power-cut injection, recovery receipts |
| Driver breadth | many probes, no recovery | lifecycle completion gates before new controller family |
| Performance regression | IPC/process/security migration makes UI slower | retain phase telemetry; bounded fast paths; compare same physical interaction journal |
| GPU lock-in | accelerated path becomes only boot path | software scene oracle and GOP fallback are permanent |
| Agent privilege | planner obtains ambient file/network/device authority | ordinary process + broker + attenuated handles + signed tools + revocation/audit |
| Public-demo abuse | resource exhaustion, proxy hijack, guest egress misuse | per-lease auth, rate limits, cgroups/seccomp/egress, finite pool, transactional cleanup |
| Supply chain | mutable downloads/ports/plugins/toolchains | pin versions/digests/patches, sign manifests, record generator/toolchain provenance |
| License contamination | source copied from restrictive/missing-license repos | behavioral spec and fresh implementation; provenance log; separate legal review for any code reuse |
| Self-hosting too early | removes C/reference before replacement is trustworthy | keep bootstrap seeds and differential oracles until in-OS fixpoint/artifact proof |

## Measurable proof of the complete target

The project reaches the full target only when all rows below carry exact receipts. “Implemented” without the named evidence remains partial.

| Area | Proof of done |
|---|---|
| Boot | BIOS/raw/UEFI lanes boot their exact identified artifact or refuse deterministically; assertion matrix covers media/filesystem/fragmentation/memory/failure; ZLLOG retains failure |
| Physical desktop | retained-surface trace proves zero redraw of unchanged covered clients/shells; no stale pixels; current latency/frame/drop contracts pass on ThinkPad |
| Memory/process | hostile process corpus cannot cross kernel/device/other-process boundaries; faults terminate only offender; W^X, guard, usercopy and resource cleanup gates pass |
| IPC/capabilities | bounded transfer/call/reply/cancel/death/restart/version corpus passes; attenuation/revoke enforced; malformed input has no partial mutation |
| Files/storage | common provider suite passes NVMe/USB and images; power-cut injection yields committed old/new state; cold reboot verifies exact app bytes; disk-full/corruption recovery is explicit |
| USB/drivers | each promoted driver passes queue model, forced timeout/reset, repeated lifecycle, QEMU if possible and exact hardware receipt; counters visible in System Monitor/ZLLOG |
| Network | deterministic fault simulator passes; physical carrier/DHCP/DNS/TCP/TLS/browser receipts are distinct; service restart isolates clients correctly |
| Graphics/GPU | software and accelerated scene outputs agree for corpus; forced GPU hang recovers/falls back; physical timing materially improves; GOP always boots |
| Audio/media | mixer golden/property tests, bounded latency/queue and underrun/overrun evidence; HDA/virtual provider lifecycle; malformed parser processes die without system effect |
| Applications/session | terminal/editor/Files/System Monitor/Settings/browser run as isolated apps, complete real workflows, save across reboot and survive one app crash; authenticated session owns authority |
| Packages/ports | signed package install/upgrade/injected-failure rollback/uninstall passes; every listed port has source/build/install/launch receipt |
| zl toolchain | imports through system types/ABI/relocatable output pass interpreter/LLVM/native differential gates; exact layouts and 64-bit addresses verified; compiler fixpoint remains green |
| Agents | ungranted actions denied, provenance preserved, subagent grants bounded/revoked, tool failures reversible, independent reviewer catches planted false completion |
| Demo hosting | concurrency/failure/expiry/cleanup/admission/egress tests pass; zero orphan slots/processes/overlays; each session proves image digest |
| Self-host distribution | zlOS rebuilds matching zl compiler/system/package artifacts inside itself; rebuilt image passes the boot matrix; bootstrap C is optional rather than secretly required |

## Final architecture decision

No single starred repository becomes the blueprint. The combined clean-room result is:

**zl's own complete language and compiler sequence + the existing measured zlOS hardware/storage/desktop spine + Hyper boot truth + Brook/Serenity verification + RustOS ownership/evidence contracts + NexiOS boundedness questions + independently specified handles/atomic transfer informed by the failed Skift/NexiOS cases + Mollen/Lemon generated services + Zinnia driver seams + Serenity desktop/security isolation + protOS's user-WM vertical slice corrected by strict loader/usercopy/lifetime invariants + snarkOS operational roles + Chitti's corrected userspace agent protocol + mission-control's hardened demo lifecycle.**

The immediate implementation remains retained client and shell surfaces because current physical evidence names them. The architecture then deepens zlfs/block I/O, establishes real 64-bit processes, adds capabilities and typed service boundaries, migrates existing applications without throwing them away, proves real networking and hardware lifecycles, completes zl's system-grade output, and only then makes agents, packages, demo hosting, and in-OS self-hosting ordinary parts of one evidence-backed system.

The weakest link is not the destination design. It is integration proof across the joins: language ABI to process loader, service protocol to hardware recovery, and source/model evidence to the exact physical artifact. The plan therefore makes those joins explicit gates rather than assuming that locally strong components compose automatically.
