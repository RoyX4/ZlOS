# Canonical complete zl/zlOS product feature catalog

Date: 2026-08-22

Purpose: one de-duplicated list of the meaningful features found across the 33 pinned starred repositories, the current zl and zlOS trees, and the clean-room destination selected by the audit. This is the product-level companion to the mechanism-heavy [`CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md`](CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md).

“Complete” here means complete at the level of user, platform, service, developer, security, operational, and proof capabilities. It deliberately does not list every internal helper function, generated file, vendor asset, historical branch, codec profile, PCI ID, shell alias, or third-party port as a separate product feature. Those inventories remain in the three driver/app deep dives and three visual/app deep dives.

## How to read this catalog

Each stable ID names one independently specifiable and testable product capability. A row does not mean zlOS has it.

| State | Meaning |
|---|---|
| `Keep` | substantive current zl/zlOS behavior exists and should survive migration |
| `Deepen` | current behavior exists but lacks the complete contract named here |
| `Add` | selected destination capability is absent or not substantive today |
| `Later` | part of the complete destination, dependency-late rather than discarded |
| `Fixture` | current demo/test surface is useful as a regression workload, not the final product boundary |
| `Reject` | observed shelf idea is retained only as a negative test; mock/dead/external/source-only evidence cannot satisfy it |

A feature is complete only when its implementation, route, authority, lifecycle, failures, accessibility, performance, persistence where applicable, and reproducible evidence all agree. “Source exists,” “built,” “boots,” “screenshot,” and “works on hardware” remain separate claims.

## 00. Evidence, provenance, and product truth

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| EV-001 | canonical feature registry | Deepen | one generated identity for every feature, owner, dependencies, maturity, routes, artifacts, tests, and weakest evidence |
| EV-002 | exact source snapshot | Keep | repository, commit, submodules, dirty state, patches, toolchain, and dependency digests recorded |
| EV-003 | license ledger | Deepen | per-component code, font, icon, firmware, media, dataset, and port licenses travel into artifacts |
| EV-004 | dependency lock | Deepen | all direct/transitive source and binary inputs are immutable and verified |
| EV-005 | reproducible toolchain manifest | Deepen | compiler, assembler, linker, sysroot, generated headers, flags, and target ABI are identified |
| EV-006 | clean-build proof | Deepen | empty output directory produces current artifacts without hidden cached prerequisites |
| EV-007 | build graph truth | Deepen | source, compiled object, linked image, package, registry, init, and launch-route inventories agree |
| EV-008 | strict exit propagation | Deepen | no wrapper, timeout, ignored command, or final-test status can manufacture green |
| EV-009 | artifact self-identity | Deepen | booted/running product reports exact image, commit, configuration, and boot origin |
| EV-010 | evidence ladder | Keep | claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct |
| EV-011 | feature maturity ledger | Deepen | each feature exposes current evidence, missing gates, known defects, and rejected claims |
| EV-012 | test inventory parity | Add | declared, compiled, registered, executed, skipped, failed, and shipped test sets compare automatically |
| EV-013 | canary/mutation gate | Add | planted failure in every required field proves the verifier can fail |
| EV-014 | benchmark receipt | Deepen | workload, clock, samples, percentile, peak, machine, backend, artifact, and regression budget recorded |
| EV-015 | hardware receipt | Keep | exact device/firmware/topology, first use, reuse, reset, teardown, recovery, and raw evidence |
| EV-016 | screenshot/video receipt | Deepen | asset names artifact, mode, scale, theme, locale, scenario, time, and evidence ceiling |
| EV-017 | crash/panic receipt | Keep | independent minimal recorder preserves symbols, input, state, and artifact identity |
| EV-018 | structured event trace | Deepen | typed monotonic events carry feature, process, authority, correlation, drop, and redaction fields |
| EV-019 | failure-injection registry | Add | every allocation, queue, I/O, provider, service, package, and lifecycle seam has forced failures |
| EV-020 | hostile corpus registry | Add | malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized |
| EV-021 | performance regression registry | Deepen | frame, input, I/O, network, launch, memory, and build budgets are machine checked |
| EV-022 | visual regression registry | Deepen | goldens cover scale, theme, locale, a11y, empty/error/loading, and backend variants |
| EV-023 | accessibility proof registry | Add | keyboard and assistive-provider workflows are required for each component/app route |
| EV-024 | security claim registry | Add | threat model, protected asset, enforcement point, adversarial proof, and residual risk are linked |
| EV-025 | independent refutation | Keep | a reviewer attempts counterexamples and can reject, not silently repair, completion evidence |
| EV-026 | historical decision log | Deepen | decisions, migrations, deprecations, reversals, and obsolete evidence stay attributable |
| EV-027 | release notes/changelog | Add | user-visible changes, compatibility, migrations, defects, and recovery steps derive from manifests |
| EV-028 | provenance viewer | Add | users can inspect version, origin, licenses, signatures, permissions, tests, and current health |

## 01. Boot, firmware, architecture, and platform

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| BT-001 | BIOS boot | Keep | bounded, identity-checked boot from supported raw/disk/optical layouts |
| BT-002 | UEFI boot | Keep | disciplined memory-map exit, GOP, exact image identity, and fallback |
| BT-003 | raw-sector boot | Keep | versioned bounded layout with capacity/range validation and recovery |
| BT-004 | Multiboot boot | Keep | exact protocol version, modules, framebuffer, map, and malformed-handover tests |
| BT-005 | network boot | Later | authenticated bounded payload with origin/digest and failure recovery |
| BT-006 | boot-media matrix | Deepen | BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads |
| BT-007 | boot-origin identity | Add | firmware/loader-selected device and partition become immutable kernel handles |
| BT-008 | typed boot handover | Deepen | versioned length-delimited ranges, modules, framebuffer, ACPI/FDT, entropy, and origin |
| BT-009 | kernel ELF loader | Deepen | length-first checked headers/segments/entry/permissions and failure-atomic mapping |
| BT-010 | relocatable kernel/KASLR | Later | supported relocation manifest, trustworthy entropy, collision checks, and exact slide receipt |
| BT-011 | initramfs/module archive | Deepen | signed length-known archive with checksum/path/link/bomb bounds |
| BT-012 | boot configuration | Deepen | parsed schema, required/optional fields, safe defaults, diagnostics, and rollback |
| BT-013 | boot selector | Add | keyboard-accessible exact-origin choices, timeout/policy, recovery and previous generation |
| BT-014 | boot splash/progress | Deepen | honest stage/state, accessible text alternative, no fixed fake telemetry |
| BT-015 | early serial console | Keep | bounded polling fallback with framing/errors and panic independence |
| BT-016 | early framebuffer console | Keep | validated format/pitch/geometry, bounded drawing, and handoff to display service |
| BT-017 | initialization DAG | Deepen | cycle detection, readiness, deadlines, criticality, rollback, reverse teardown, receipts |
| BT-018 | safe/recovery boot | Add | separately signed read-only-first environment with explicit target and logged repair |
| BT-019 | previous-generation rollback | Add | bootable known-good system and atomic selection after failed update |
| BT-020 | firmware inventory | Deepen | checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior |
| BT-021 | CPU feature discovery | Deepen | immutable required/optional capability record before feature use |
| BT-022 | x86-64 platform | Deepen | long mode, exception, MMU, timer, SMP, process, service, and device gates |
| BT-023 | i686 compatibility lane | Keep | deliberately scoped regression artifact, never confused with 64-bit completion |
| BT-024 | AArch64 port | Later | real EL/MMU/GIC/timer/SMP/process/service boot receipts |
| BT-025 | RISC-V 64 port | Later | SBI/Sv39/PLIC/boot/process/service matrix, not compile-only scaffolding |
| BT-026 | architecture abstraction | Deepen | common contracts plus per-architecture conformance and reachability inventories |
| BT-027 | SMP bring-up/offline | Deepen | exact CPU inventory, staged online, barriers, IPIs, TLB, failure and teardown |
| BT-028 | monotonic clock | Keep | one deadline clock with calibration, wrap, drift, suspend and fallback semantics |
| BT-029 | wall clock/RTC | Deepen | validated hardware time separated from deadlines; timezone/locale in user service |
| BT-030 | firmware wait policy | Deepen | every poll has deadline, cancel/reset, final status, and deterministic timeout proof |

## 02. Kernel, memory, execution, and process model

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| KR-001 | physical page allocator | Deepen | typed ownership, reserved-map proof, exhaustion, zero/reuse, and double-free detection |
| KR-002 | kernel heap | Deepen | checked size/alignment, explicit OOM, tags, poisoning, and failure injection |
| KR-003 | virtual address layout | Deepen | generated non-overlap manifest for kernel, user, devices, stacks, heaps, and shared regions |
| KR-004 | page-table construction | Deepen | reserve/validate/apply/flush/commit or exact rollback across full ranges |
| KR-005 | per-process address space | Add | unique root, protected kernel half, owned mappings, teardown and switch receipts |
| KR-006 | user/kernel privilege boundary | Add | ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present |
| KR-007 | safe user copy | Add | canonical, mapped, permission, full-span, bounded-string, and copyout-fault handling |
| KR-008 | NX/W^X | Add | no writable executable mapping; controlled authorized transition for JIT if ever supported |
| KR-009 | guard pages | Add | user/kernel stacks and critical arenas fault safely and kill only offender |
| KR-010 | anonymous memory | Add | reserved vs committed, zero fill, limits, reclaim, collision, and OOM semantics |
| KR-011 | file-backed mapping | Add | shared/private mappings integrated with cache, permissions, truncate, and revoke |
| KR-012 | copy-on-write | Later | atomic refcount/permission changes and concurrent-fault cleanup |
| KR-013 | shared-memory object | Add | opaque owner/grantee handle, size/rights/generation/refcount/revoke; no global IDs |
| KR-014 | page/file cache | Add | bounded ownership, dirty accounting, async writeback, reclaim, flush, and pressure tests |
| KR-015 | memory accounting | Add | process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance |
| KR-016 | memory pressure policy | Add | priority-aware reclaim, cache eviction, app notification, OOM selection, recovery |
| KR-017 | kernel stack management | Deepen | per-thread stacks, guards, high-water evidence, safe interrupt nesting |
| KR-018 | process creation | Add | validated executable, credentials, handles, limits, namespace, atomic publication |
| KR-019 | fork/clone semantics | Later | exact credential/handle/address-space copy or sharing matrix with rollback |
| KR-020 | exec semantics | Add | atomic image replacement preserving only declared process state |
| KR-021 | thread lifecycle | Add | create/join/exit/cancel, ownership, stack/TLS, peer/process death cleanup |
| KR-022 | scheduler | Deepen | per-CPU ownership, priorities, fairness, bounded queues, accounting, migration |
| KR-023 | real-time scheduling | Later | admission, utilization bounds, priority inversion control, deadline miss evidence |
| KR-024 | timer/deadline API | Deepen | monotonic absolute deadlines, cancellation, coalescing, wrap and suspend behavior |
| KR-025 | signals/events | Add | permission-checked delivery including signal 0/groups, bounded queues, defined interruption |
| KR-026 | process groups/sessions/jobs | Add | shell/terminal ownership, foreground control, hangup, stop/continue and cleanup |
| KR-027 | wait/exit status | Add | parent/child identity, exact termination reason, no PID reuse confusion |
| KR-028 | process handles | Add | generation-tagged opaque references replace ambient PIDs for privileged operations |
| KR-029 | resource limits | Add | memory, CPU, handles, threads, IPC, files, sockets, surfaces, log and time budgets |
| KR-030 | namespace/container boundary | Later | mount, process, network, IPC, device and user namespaces with explicit joins |
| KR-031 | exception/fault containment | Add | malformed user state kills current process only; desktop and system continue |
| KR-032 | interrupt/exception architecture | Deepen | complete vector ownership, nesting, fault frames, spurious paths, and panic fallback |
| KR-033 | TLB shootdown | Add | generation/ack protocol, offline CPUs, timeout/fail-stop, stress proof |
| KR-034 | kernel synchronization | Deepen | lock ordering, IRQ/preemption rules, wait queues, atomics, deadlock and race tests |
| KR-035 | random number service | Add | boot entropy accounting, CSPRNG reseed/fork semantics, blocking/readiness policy |
| KR-036 | syscall ABI | Deepen | generated table, unsigned bounds including negative/sign-bit indices, gaps, versioning |
| KR-037 | stable userspace ABI | Add | calling convention, layout, errors, handles, time, signals, compatibility policy |
| KR-038 | core dump | Add | permission/redaction-aware process image, registers, maps, build IDs and retrieval |
| KR-039 | kernel live diagnostics | Keep | read-only structured facts without exposing arbitrary memory or device control |
| KR-040 | reboot/shutdown | Deepen | capability-gated orderly service/device/filesystem teardown with forced fallback |

## 03. Authority, identity, IPC, and security

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| AU-001 | opaque capability handle | Add | unforgeable table index plus generation, type, rights, owner, and audit identity |
| AU-002 | least-privilege process grant | Add | launch receives only manifest/user/session-authorized handles |
| AU-003 | capability derivation | Add | source DERIVE/COPY right, destination insertion right, explicit parent-child graph |
| AU-004 | exhaustive revoke | Add | depth and total-node bounds are admitted before mutation; all descendants stale |
| AU-005 | atomic handle transfer | Add | destination reservation, rights matrix, single commit point, exact rollback |
| AU-006 | typed IPC schema | Add | generated length-delimited versioned messages with unknown-required rejection |
| AU-007 | bounded IPC endpoint | Add | message/byte/handle quotas, capacity-aware receive, backpressure, drop counters |
| AU-008 | request/reply correlation | Add | opaque IDs, deadline, cancel, duplicate/late reply behavior, peer identity |
| AU-009 | peer-death semantics | Add | waiters wake, transactions abort, handles/surfaces/rings clean up |
| AU-010 | service discovery | Add | authenticated typed endpoint registry with version/capability negotiation |
| AU-011 | service authority profile | Add | role, exposure, endpoints, budgets, dependencies, management and audit declared |
| AU-012 | default-deny authorization | Add | complete mediation with deny-overrides and missing-policy failure |
| AU-013 | privileged operation matrix | Add | mount, MMIO, DMA, device, input, scanout, signal, debug, power and admin rights explicit |
| AU-014 | credentials | Add | real/effective/saved UID/GID and groups with atomic pre-state transition matrix |
| AU-015 | credential drop | Add | failure checked and closed; session never starts after failed drop |
| AU-016 | login authentication | Add | salted memory-hard secret verification, rate limits, secret zero/log policy |
| AU-017 | user/account management | Add | identities, groups, home/profile, lock/disable/recovery and admin delegation |
| AU-018 | session identity | Add | authenticated local/remote session owns display, input, clipboard, notifications and apps |
| AU-019 | screen lock | Add | compositor-enforced secure surface; no app spoof/bypass; input and secret isolation |
| AU-020 | elevation/consent | Add | exact operation/target/arguments/duration/digest bound to fresh user approval |
| AU-021 | permission dashboard | Add | users inspect/revoke app data, device, network, background, notification and agent grants |
| AU-022 | secrets service | Add | per-user encrypted objects, app-scoped handles, unlock/revoke/rotation and no plaintext logs |
| AU-023 | trust store | Add | roots, key IDs, algorithms, policy version, rotation, revocation and time behavior |
| AU-024 | package signature | Add | publisher-authenticated asymmetric signature distinct from local integrity MAC |
| AU-025 | secure update | Add | signed metadata, version/rollback protection, staged atomic install and recovery |
| AU-026 | sandbox policy | Add | filesystem, network, device, IPC, process, debug and resource access default denied |
| AU-027 | executable admission | Add | origin/signature/ABI/permissions/W^X/dependencies checked before publication |
| AU-028 | audit trail | Add | durable append/anchor/drop evidence, authority identity, redaction and retention |
| AU-029 | privacy model | Add | data classes, collection purpose, minimization, retention, export, erase, sharing and consent |
| AU-030 | secure erase/key destruction | Later | defined media/flash limitations and key-based user-data retirement |
| AU-031 | exploit mitigation | Later | ASLR, stack canaries, CFI where viable, hardened allocators, guard pages and W^X |
| AU-032 | security update response | Add | vulnerability identity, affected artifacts, remediation, revocation and user communication |
| AU-033 | firewall policy | Add | default network posture, app/service rules, logging, user/admin views |
| AU-034 | VPN/tunnel authority | Later | explicit route/DNS/interface scopes and disconnect leak policy |
| AU-035 | remote access security | Add | authenticated encrypted endpoint, device/session identity, rate limits and revocation |
| AU-036 | service lifecycle | Add | admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback |
| AU-037 | service management endpoint | Add | separately authorized health/config/action interface, quotas and audit |
| AU-038 | cryptographic service/API | Add | versioned algorithms, secure randomness, key handles, constant-time policy and test vectors |
| AU-039 | certificate/key lifecycle | Add | generate/import/store/use/rotate/revoke/export policy and hardware-backed option |
| AU-040 | data-at-rest policy | Add | app/user/system classes, encryption keys, lock state, backup and recovery semantics |

## 04. Driver framework, hardware discovery, and device lifecycle

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| DV-001 | driver descriptor | Add | schema/version, IDs, hardware revisions, ABI ranges, capabilities, resources, signature, origin |
| DV-002 | driver matching | Deepen | exact bus identifiers and required/optional capability negotiation |
| DV-003 | driver admission | Add | authority, package trust, ABI, conflicts, resources, DMA width/coherency validated |
| DV-004 | resource plan/reservation | Add | BAR, port, IRQ, DMA, clock, pin and dependency ownership reserved atomically |
| DV-005 | early abort/rollback | Add | failures from match/admit/reserve/start release every object with receipts |
| DV-006 | online/readiness | Add | provider publishes only after health and protocol readiness |
| DV-007 | quiesce/stop | Add | new work refused, in-flight bounded, interrupts/DMA stopped before release |
| DV-008 | reset/recovery | Add | bounded staged reset, generation invalidation, state replay, quarantine |
| DV-009 | hotplug/remove | Add | dependency loss, user notice, safe unmount/eject, stale-handle failure |
| DV-010 | driver supervisor | Add | crash budget/backoff, health, dependency graph, rebind and management authority |
| DV-011 | device manager UI | Add | truthful inventory, driver, resources, health, permissions, reset/eject and evidence |
| DV-012 | MMIO/port handle | Add | exact-width volatile access, bounds, barriers, ownership and revocation |
| DV-013 | IRQ handle | Add | exclusive/shared policy, mask/ack ordering, generation, storm and teardown |
| DV-014 | DMA object | Add | device-scoped pinned pages, direction, lifetime, sync, completion and revoke |
| DV-015 | IOMMU domain | Later | default-deny per device, scoped maps, out-of-domain fault proof, detach cleanup |
| DV-016 | no-IOMMU policy | Add | visibly not-isolated trusted/bounce/pinned fallback; never mislabeled isolated |
| DV-017 | PCI/PCIe enumeration | Keep | typed BDF, multifunction/bridge recursion, checked config and unknown classes |
| DV-018 | PCI BAR ownership | Deepen | 64-bit sizing, type/prefetchability, collision, mapping, release and high-BAR tests |
| DV-019 | ACPI provider | Deepen | checked tables/AML boundary, bounded EC/waits, lifecycle and provenance |
| DV-020 | device-tree provider | Later | checked FDT cells/ranges/compatible registry and malformed corpus |
| DV-021 | SMBIOS inventory | Deepen | checked versioned read-only facts; never used alone as security identity |
| DV-022 | interrupt-controller provider | Deepen | topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown |
| DV-023 | timer provider | Deepen | PIT/HPET/APIC/TSC/architectural timer discovery, calibration and fallback |
| DV-024 | serial/UART provider | Keep | early polling plus bounded IRQ ring, errors and panic fallback |
| DV-025 | PS/2 input provider | Keep | bounded controller init, stateful decoder, overflow/loss/disconnect semantics |
| DV-026 | USB host core | Deepen | controller-independent topology, descriptor parser, address/config/interface lifecycle |
| DV-027 | xHCI provider | Keep | rings/slots/endpoints/hubs, timeout/reset/hotplug/repeated reuse |
| DV-028 | legacy USB controllers | Later | UHCI/OHCI/EHCI added only for target hardware under same core contract |
| DV-029 | USB HID keyboard | Deepen | report descriptors, layouts, LEDs, repeat, multiple devices, disconnect and physical receipt |
| DV-030 | USB HID pointer | Deepen | relative/absolute axes, buttons, wheel, high-resolution input and physical receipt |
| DV-031 | I2C-HID transport | Deepen | descriptor/report transport plus real x/y/button/touch decoder and session route |
| DV-032 | touch/multitouch | Later | contacts, slots, gestures, palm policy, calibration, orientation and a11y |
| DV-033 | stylus/tablet | Later | pressure, tilt, buttons, eraser, proximity, mapping and app permissions |
| DV-034 | game controller | Add | standardized axes/buttons/haptics, hotplug, calibration, per-app mapping |
| DV-035 | NVMe storage provider | Keep | queue lifecycle, PRP bounds, timeout/reset, namespaces, flush and repeated reuse |
| DV-036 | AHCI/SATA provider | Later | ports, command tables, DMA, timeout/reset, hotplug and flush |
| DV-037 | ATA/IDE fallback | Keep | bounded legacy reads/writes, identify, range, timeout and explicit scope |
| DV-038 | USB mass storage | Keep | BOT/SCSI state, capacity, sense, stalls/reset/reconnect and durable physical proof |
| DV-039 | virtio block | Deepen | negotiated queues, bounds, failure injection, flush/discard and QEMU oracle |
| DV-040 | SD/eMMC | Later | discovery, partitions, DMA, removal, flush and power-loss behavior |
| DV-041 | Ethernet provider | Deepen | bounded packet queues, link, MTU, offloads, reset and per-driver receipts |
| DV-042 | Wi-Fi provider | Later | radio/scan/auth/association/roam/power and firmware provenance |
| DV-043 | Bluetooth provider | Later | controller, pairing/bonding, profiles, privacy, permissions and recovery |
| DV-044 | framebuffer/GOP provider | Keep | validated scanout mapping, format, pitch, write-combining and fallback |
| DV-045 | Intel display provider | Deepen | native-boot probe/modeset/second/reset/fallback; host harness alone is not completion |
| DV-046 | virtio-gpu provider | Later | negotiated 2D resources/scanout/fences; 3D remains separate |
| DV-047 | GPU command provider | Later | validated grammar, per-client contexts, fences, hang reset and software oracle |
| DV-048 | HDA/audio provider | Later | codec, DMA stream, IRQ, timing, volume and reset receipts |
| DV-049 | virtio-snd provider | Add | negotiated streams and deterministic QEMU audio oracle |
| DV-050 | USB Audio provider | Later | descriptors, formats, clocking, feedback, hotplug and per-app service route |
| DV-051 | camera provider | Later | formats, frames, privacy indicator, exclusive/shared policy and disconnect |
| DV-052 | printer provider | Later | discovery, capabilities, job stream, status, cancellation and errors |
| DV-053 | scanner provider | Later | source, resolution/color, page stream, privacy and cancellation |
| DV-054 | battery/thermal/power provider | Later | units, unknown states, bounded firmware calls, events and policy separation |
| DV-055 | TPM/security device | Later | measured identity/sealing/attestation with clear trust and recovery semantics |

## 05. Storage, filesystems, packages, and recovery

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| FS-001 | block-device contract | Deepen | sector geometry, capacity, alignment, async requests, flush, discard, errors, detach |
| FS-002 | partition discovery | Deepen | MBR/GPT checksums/ranges/overlap/unknowns and exact boot-origin binding |
| FS-003 | volume identity | Add | stable UUID/label/generation handles, not enumeration order |
| FS-004 | asynchronous block I/O | Add | bounded queues, deadlines, cancellation, partial completion and retry policy |
| FS-005 | block cache | Add | coherent ownership, dirty accounting, eviction, writeback and device-loss behavior |
| FS-006 | VFS object model | Deepen | opaque file/dir/mount handles, types, rights, references and generations |
| FS-007 | path resolution | Deepen | rooted namespace, `.`/`..`, links, traversal bounds, race-safe operations |
| FS-008 | mount namespace | Add | per-session/container view, authority, propagation, detach and busy semantics |
| FS-009 | devfs | Deepen | generated device nodes from live handles, permission and disappearance behavior |
| FS-010 | initramfs/archive filesystem | Deepen | read-only checked archive, provenance and hostile input corpus |
| FS-011 | zlfs core | Keep | persistent create/read/write/list/edit path with exact on-disk version |
| FS-012 | zlfs directories | Deepen | nested directories, types, permissions, rename and iteration consistency |
| FS-013 | zlfs links | Add | hard/symbolic link policy, cycles, permissions and recovery |
| FS-014 | atomic rename/replace | Add | crash-consistent namespace transaction and exact failure states |
| FS-015 | file growth/truncate | Deepen | checked allocation, sparse policy, zeroing, concurrent maps and rollback |
| FS-016 | file permissions/ownership | Add | user/group/mode or ACL policy enforced on handles and transitions |
| FS-017 | timestamps/metadata | Add | monotonic/wall-clock semantics, nanosecond/unknown policy and durable update |
| FS-018 | extended attributes | Later | namespaced bounded metadata with permission and copy/backup behavior |
| FS-019 | file locking | Add | advisory/mandatory policy, range locks, owner death and deadlock behavior |
| FS-020 | durability API | Deepen | flush/fsync/barrier meaning from app through filesystem to device |
| FS-021 | journal/transaction log | Deepen | checksummed replay, ordering, torn-write and power-cut matrix |
| FS-022 | free-space accounting | Deepen | reserved blocks, disk-full admission, quotas, reclamation and truthful UI |
| FS-023 | filesystem checker | Add | read-only diagnose first, repair plan, backup/restore and durable receipt |
| FS-024 | corrupt-media admission | Add | length/checksum/range/cycle validation before mount and bounded refusal |
| FS-025 | removable-media lifecycle | Add | consent, automount policy, safe eject, busy state, detach and stale handles |
| FS-026 | encrypted volume | Later | key lifecycle, metadata protection, unlock/recovery and power-loss behavior |
| FS-027 | snapshot/versioning | Later | bounded snapshots, copy-on-write accounting, restore and retention |
| FS-028 | backup/restore | Add | manifest, incremental/full, verification, encryption, target confirmation and drill |
| FS-029 | trash/recovery | Add | per-user reversible delete, cross-volume behavior, retention and secure removal |
| FS-030 | package format | Add | signed manifest, complete file list, sizes, target ABI/arch, dependencies, conflicts, licenses |
| FS-031 | atomic package install | Add | stage, verify every object/script/grant, publish once, reboot recovery |
| FS-032 | atomic package uninstall | Add | stop/revoke/deregister/unpublish before deletion with explicit user-data policy |
| FS-033 | package dependency solver | Later | versions, conflicts, alternatives, architecture and reproducible decision receipt |
| FS-034 | immutable system generation | Later | content-addressed base, writable user state, atomic switch and rollback |
| FS-035 | update service | Add | signed metadata, download resume, staging, health gate, rollback and notices |
| FS-036 | firmware store/update | Later | origin/license/device match/signature, rollback and failure-safe flash policy |
| FS-037 | package cache/cleanup | Add | byte budgets, pinning, generation reachability, safe concurrent cleanup |
| FS-038 | file indexing/search | Add | per-user permissions, incremental updates, cancellation, privacy and rebuild |
| FS-039 | content type/association | Add | MIME sniff/extension policy, defaults, open-with, provenance and safe preview |
| FS-040 | file preview/thumbnail | Add | restricted decoder, size/time quotas, cache invalidation and privacy |

## 06. Networking, internet, and remote communication substrate

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| NW-001 | NIC service contract | Add | provider-neutral bounded packet buffers, link/MTU/features/reset/counters |
| NW-002 | Ethernet | Deepen | checked frame lengths/types, immutable packet ownership and bounded demux |
| NW-003 | ARP | Deepen | bounded neighbor states, retry/age/spoof policy and conflict handling |
| NW-004 | IPv4 | Deepen | header/options/checksum/fragment/reassembly/routing and hostile corpus |
| NW-005 | IPv6 | Later | NDP, SLAAC/DHCPv6, routes, extension headers, reassembly and dual-stack policy |
| NW-006 | ICMP/ICMPv6 | Deepen | real echo/error/PMTU path with checksums, IDs, timeouts and socket feedback |
| NW-007 | UDP | Deepen | bounded datagrams, bind/demux/source metadata/truncation/checksum/exhaustion |
| NW-008 | TCP | Deepen | complete state machine, windows, retransmit/congestion, loss/reorder/wrap/reset tests |
| NW-009 | DHCPv4 | Deepen | async discover/offer/request/lease/renew/rebind/expiry and hostile server handling |
| NW-010 | DHCPv6/SLAAC | Later | address/prefix/router/DNS lifecycles and privacy policy |
| NW-011 | DNS resolver | Add | restricted service, cache/negative TTL, compression bounds, UDP/TCP fallback |
| NW-012 | socket API | Add | handle-based bind/connect/listen/accept/send/recv/poll with deadlines/cancel |
| NW-013 | network namespaces | Later | interface/route/socket/firewall/DNS isolation and explicit joining |
| NW-014 | route table | Add | longest-prefix routing, metrics, source selection, changes and diagnostics |
| NW-015 | loopback | Add | deterministic local service/testing path with same socket semantics |
| NW-016 | TLS | Add | restricted user-space implementation, trust/time/hostname/revocation and hostile corpus |
| NW-017 | HTTP/1.1 | Deepen | bounded parser/body/redirect/deadline/cancel/connection reuse and truthful errors |
| NW-018 | HTTP/2 | Later | framed multiplexing, flow control, HPACK bounds, stream cancellation |
| NW-019 | HTTP/3/QUIC | Later | authenticated transport, congestion, migration, stream budgets and fallback |
| NW-020 | WebSocket | Later | authenticated upgrade, bounded frames/messages, ping/close and backpressure |
| NW-021 | proxy support | Later | explicit HTTP/SOCKS/PAC policy, credentials, bypass and privacy |
| NW-022 | firewall | Add | default policy, per-app/service rules, state, logs and user/admin UI |
| NW-023 | network time | Add | authenticated/bounded synchronization, drift/step policy and wall-clock separation |
| NW-024 | mDNS/service discovery | Later | scoped discovery, names/conflicts, privacy and untrusted-record bounds |
| NW-025 | SSH client/server | Later | real key exchange/auth/channel/terminal/file transfer; version banner alone is not SSH |
| NW-026 | remote desktop | Later | authenticated encrypted session, resize/input/clipboard, reconnect and privacy |
| NW-027 | captive-portal flow | Later | detected restricted state, sandboxed login, privacy and explicit completion |
| NW-028 | Wi-Fi management | Later | scan, profiles, secrets, association, roam, signal, failures and UI |
| NW-029 | VPN management | Later | tunnel identity, routes/DNS, secrets, kill-switch and recovery |
| NW-030 | connectivity diagnostics | Add | truthful link/address/route/DNS/TCP/TLS probes with timing and export |
| NW-031 | per-app network permission | Add | destination/protocol/listen/background policy with live revoke |
| NW-032 | traffic accounting | Add | per-interface/app/service bytes, drops, latency and privacy-aware history |
| NW-033 | egress policy for demos/agents | Add | allowlisted destinations, quotas, DNS consistency, audit and termination |
| NW-034 | offline mode | Add | deterministic network refusal, cached-data behavior and visible state |
| NW-035 | network recovery | Add | provider/service restart, connection invalidation, retry/backoff and user state |

## 07. Display, rendering, compositor, and visual performance

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| GR-001 | display-provider contract | Deepen | mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API |
| GR-002 | mode discovery/selection | Deepen | connector/EDID/mode validity, preferred/fallback, safe apply and rollback |
| GR-003 | multi-display topology | Later | position, primary, scale, rotation, mirror/extend, hotplug and per-display refresh |
| GR-004 | color format conversion | Keep | checked RGB/BGR/565/8888 conversions, alpha rules and differential tests |
| GR-005 | color management | Later | profiles, transfer functions, gamut, HDR policy and screenshot/export consistency |
| GR-006 | compositor-exclusive scanout | Add | apps never map global scanout; only compositor/provider presents |
| GR-007 | opaque surface/window handles | Add | owner/grantee rights, generations, mapping references, revoke and peer-death cleanup |
| GR-008 | surface admission | Add | checked geometry, stride, format, bytes, overflow, limits and memory accounting |
| GR-009 | double-buffer protocol | Add | configure/ack, attach, damage, commit, release, no reuse before release |
| GR-010 | retained client surfaces | Deepen | unchanged app content survives move/expose/focus without app redraw |
| GR-011 | retained shell/chrome | Deepen | borders, shadows, dock, menu and desktop layers invalidate independently |
| GR-012 | retained scene graph | Add | immutable/generation scene state, transforms, clips, opacity, resource ownership |
| GR-013 | damage regions | Keep | bounded area-aware rectangles, merge policy, cause, full fallback and properties |
| GR-014 | occlusion calculation | Add | opaque coverage suppresses hidden work without breaking translucency |
| GR-015 | hierarchical clipping | Deepen | screen/window/widget/scroll/rounded clips constrain work before pixel loops |
| GR-016 | atomic scene commit | Add | chrome/content/focus/z-order from one coherent generation |
| GR-017 | one-present frame | Deepen | one newest committed scene per deadline, no half-frame or redundant present |
| GR-018 | frame scheduler | Deepen | refresh-aware deadline, event-driven wake, cost estimate, stale-state drop |
| GR-019 | vblank/present timing | Deepen | negotiated refresh, bounded waits, missed-vblank accounting and fallback |
| GR-020 | frame attribution | Keep | input/app/compositor/wait/present time plus damage/windows/bytes/backend |
| GR-021 | input-to-present tracing | Keep | input sequence/timestamp survives through first affected present |
| GR-022 | worst-frame gate | Deepen | peak and percentiles plus zero unexplained late frames; average FPS insufficient |
| GR-023 | framebuffer mapping policy | Keep | correct cacheability/write-combining with architecture and physical receipts |
| GR-024 | software renderer oracle | Keep | deterministic blend/clip/shape/text/image output remains fallback and GPU comparator |
| GR-025 | GPU renderer backend | Later | same scene semantics, validated resources/commands, fences, reset and fallback |
| GR-026 | alpha composition | Keep | defined straight/premultiplied model, rounding, color space and property tests |
| GR-027 | rounded geometry | Keep | supersampled/analytic coverage, consistent radii, clipping and hit testing |
| GR-028 | shadows/elevation | Keep | semantic depth, scissor-aware bounded implementation, cache and opaque fallback |
| GR-029 | gradients/decorative fields | Keep | deterministic cached rendering with bounded setup and degradation |
| GR-030 | backdrop blur | Deepen | stationary/cacheable only unless backend budget proves live update; invalidation exact |
| GR-031 | wallpaper cache | Keep | full-resolution bounded cache, memory admission, invalidation and honest fallback |
| GR-032 | icon/image cache | Add | key by origin/scale/theme/profile, byte budget, generation and decoder isolation |
| GR-033 | glyph/shaping cache | Deepen | font/size/scale/script/features key, atlas budget, eviction and fallback |
| GR-034 | layout cache | Add | dependency-aware invalidation for text/widget/browser trees |
| GR-035 | virtualized rendering | Add | only visible list/text/table/directory/document ranges are created and painted |
| GR-036 | resize interaction | Deepen | configure/ack, minimums, aspect policy, retained preview, settled redraw |
| GR-037 | move/drag interaction | Keep | time-stable grab identity, old/new damage, retained node move and snap preview |
| GR-038 | window stacking/focus | Keep | deterministic z-order, modal/transient relationships, activation policy and a11y |
| GR-039 | workspaces | Add | create/switch/move/overview, keyboard route, persistence and notifications |
| GR-040 | fullscreen | Add | explicit user/app policy, overlays, escape route, multi-display and permission |
| GR-041 | direct scanout | Later | safe eligible fullscreen optimization with compositor revocation and fallback |
| GR-042 | hardware cursor | Later | provider cursor plane with software equivalence and capture behavior |
| GR-043 | screenshot/capture portal | Add | user-mediated target, privacy indicator, protected surfaces and typed result |
| GR-044 | screen recording | Later | consent, region/window/display, audio, frame pacing, storage and protected content |
| GR-045 | remote composition mode | Later | damage/video transport, congestion, input clock, scaling and privacy |
| GR-046 | graphics diagnostics | Keep | live backend/mode/cache/damage/frame/fence/drop information and export |
| GR-047 | graphics recovery | Add | provider hang/loss resets safely and returns to software/GOP usable shell |
| GR-048 | memory-pressure graphics policy | Add | deterministic cache/surface eviction without stale pixels or semantic loss |
| GR-049 | reduced-quality mode | Add | declared effect degradation while preserving content, contrast, focus and a11y |
| GR-050 | visual/frame regression lab | Deepen | shipping-code benchmark, screenshot oracles and mutation tests across modes |

## 08. Input, interaction, and device experience

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| IN-001 | normalized input event | Deepen | device, sequence, monotonic time, physical/logical code, value, flags and loss |
| IN-002 | raw input stream | Deepen | permissioned unaccelerated device events for games, diagnostics and assistive tools |
| IN-003 | compositor hit testing | Keep | z-order/clip/alpha-aware target selection with authenticated ownership |
| IN-004 | keyboard focus | Keep | explicit transfer, visual/semantic indication, modal policy and peer-death fallback |
| IN-005 | pointer focus | Keep | enter/leave/motion/buttons/wheel with stable surface generation |
| IN-006 | input grabs/capture | Deepen | owner, scope, cancellation, secure shortcuts and stuck-key recovery |
| IN-007 | global shortcut policy | Deepen | reserved/session/app shortcuts, conflicts, remapping and accessibility |
| IN-008 | key repeat | Deepen | configurable delay/rate, per-device state, focus transitions and no stuck repeats |
| IN-009 | keyboard layouts | Add | physical keys to locale layout, dead keys, compose, persistence and switching |
| IN-010 | input methods | Later | IME composition, candidate UI, script support, focus/security and a11y |
| IN-011 | pointer acceleration | Deepen | time/velocity-normalized curve, speed, raw toggle and reproducible aiming tests |
| IN-012 | pointer confinement/lock | Add | explicit app/user grant, escape route, focus loss and remote-session behavior |
| IN-013 | wheel/high-resolution scroll | Deepen | lines/pixels/phases/inertia, user settings and reduced-motion policy |
| IN-014 | drag and drop | Add | typed offers, target consent, actions, progress/cancel and source death |
| IN-015 | touch gestures | Later | tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation |
| IN-016 | stylus interaction | Later | pressure/tilt/eraser/palm behavior and app semantic events |
| IN-017 | gamepad mapping | Add | device profiles, dead zones, remap, hotplug, haptics and accessible alternatives |
| IN-018 | text input/selection | Deepen | cursor, range, word/line, bidi/grapheme, clipboard, composition and undo |
| IN-019 | context menu | Deepen | target-relative placement, keyboard invocation, bounds, focus and dismissal |
| IN-020 | tooltip/help affordance | Add | delayed/keyboard-accessible explanation that never blocks interaction |
| IN-021 | coarse-pointer mode | Add | hit targets/layout/hover alternatives for touch and remote use |
| IN-022 | device settings | Add | per-device speed/layout/buttons/calibration profile with rollback |
| IN-023 | input privacy | Add | password/secure surfaces, event isolation, recording indicator and permissions |
| IN-024 | input diagnostics | Keep | physical/logical events, modifiers, repeat, loss, device identity and export |
| IN-025 | input latency receipt | Deepen | per-class physical/QEMU path to first affected present, not controller-wide inference |

## 09. Audio, media devices, codecs, and timing

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| AD-001 | AudioServer | Add | exclusive hardware owner, authenticated clients, streams, routing, volume and health |
| AD-002 | audio stream contract | Add | rate/channels/format/buffer/latency negotiation and bounded shared rings |
| AD-003 | mixer | Add | per-client gain/mute/pan, saturation, deterministic mix and resource budgets |
| AD-004 | resampler | Add | declared quality/latency, rate changes and golden/property tests |
| AD-005 | channel conversion | Add | mono/stereo/multichannel mapping with explicit layout and clipping |
| AD-006 | audio clock | Add | hardware/monotonic relation, drift, timestamp, underrun and sync semantics |
| AD-007 | playback routing | Add | output selection, hotplug, default/fallback and app/session permissions |
| AD-008 | recording routing | Later | input selection, privacy indicator, grant, gain, echo policy and revoke |
| AD-009 | system sounds | Add | themed accessible cues, rate limiting, quiet mode and no direct device access |
| AD-010 | media session | Add | play/pause/seek/metadata/position/rate and global controls across apps |
| AD-011 | WAV/PCM | Deepen | checked headers/chunks/formats, streaming and golden output |
| AD-012 | compressed audio codecs | Later | isolated decoders, format matrix, output/time/memory limits and failure UX |
| AD-013 | image decode service | Add | isolated PNG/JPEG/etc. parser, dimensions/output budget, metadata and color profile |
| AD-014 | image encode/export | Add | explicit format/quality/profile/metadata/privacy and atomic file save |
| AD-015 | video demux/decode | Later | isolated parsers, bounded queues, seek/error/recovery and codec receipts |
| AD-016 | audio/video synchronization | Later | master clock, drift correction, frame drop/repeat and seek reset |
| AD-017 | subtitle/caption pipeline | Later | timed text, styling, language, accessibility, live captions and privacy |
| AD-018 | camera service | Later | device ownership, format negotiation, privacy, frame buffers and peer death |
| AD-019 | microphone service | Later | explicit recording state, device/source, privacy, level and cancellation |
| AD-020 | MIDI | Later | device/events/clock/routing, hotplug and synthesizer integration |
| AD-021 | synthesizer/DSP library | Deepen | reusable bounded realtime buffer API derived from deterministic golden DSP |
| AD-022 | media metadata | Add | safe parsing, tags/art, provenance, privacy and indexing |
| AD-023 | media library/index | Later | per-user catalog, removable media, duplicates, search and permission-aware thumbnails |
| AD-024 | media hardware acceleration | Later | optional capability-negotiated decode/encode with software fallback |
| AD-025 | audio diagnostics | Add | devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt |

## 10. Design system, components, and visual identity

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| DS-001 | named visual identity | Keep | current lime/graphite direction has documented principles and asset provenance |
| DS-002 | semantic color tokens | Deepen | canvas/surface/text/border/accent/status/selection/focus roles, not app hex copies |
| DS-003 | typography tokens | Deepen | display/title/body/label/code/caption roles with size/weight/line-height/fallback |
| DS-004 | spacing scale | Deepen | shared density-aware gaps/padding/insets and alignment rhythm |
| DS-005 | sizing metrics | Deepen | controls, rows, titlebars, panels, hit targets and minimum windows derive from tokens |
| DS-006 | radii/shape grammar | Keep | consistent corner families, masks, hit tests and scale behavior |
| DS-007 | border grammar | Deepen | hierarchy/focus/separator/error roles and high-contrast alternatives |
| DS-008 | elevation/shadow grammar | Keep | semantic depth levels with bounded renderer and opaque fallback |
| DS-009 | motion tokens | Deepen | durations/easing/distance/interruptibility/reduced-motion alternatives |
| DS-010 | icon system | Deepen | grid, stroke/fill, optical size, states, semantic names, fallback and licenses |
| DS-011 | illustration/wallpaper system | Keep | coherent art direction, scale/crop/color, cache and attribution |
| DS-012 | theme schema | Add | versioned required/optional tokens, validation, inheritance and unknown handling |
| DS-013 | light theme | Add | complete component/app coverage and contrast proof, not palette inversion |
| DS-014 | dark theme | Keep | complete semantic coverage with current identity |
| DS-015 | high-contrast themes | Add | non-color state cues and WCAG-like contrast across all primitives |
| DS-016 | density modes | Add | compact/default/touch geometry without hard-coded app overrides |
| DS-017 | live theme preview | Add | isolated preview, reversible apply, timeout rollback and persistence |
| DS-018 | design-token registry | Add | generated consumer map finds hard-coded duplicates and missing states |
| DS-019 | component registry | Deepen | one identity, variants, properties, states, semantics and ownership per primitive |
| DS-020 | button family | Keep | text/icon/toggle/destructive variants and full input/a11y states |
| DS-021 | text field family | Keep | label/help/error, selection, IME, password, validation, undo and a11y |
| DS-022 | selection controls | Deepen | checkbox/radio/switch with group, mixed, disabled, keyboard and semantic states |
| DS-023 | slider/spin control | Deepen | range/step/value format, keyboard, pointer, touch and validation |
| DS-024 | menu family | Keep | bar/context/popup/submenu, keyboard typeahead, bounds, dismissal and semantics |
| DS-025 | toolbar family | Keep | grouping, overflow, labels/tooltips, shortcuts and adaptive density |
| DS-026 | tabs family | Keep | selection/focus separation, overflow, reorder/close and semantic relationships |
| DS-027 | list/table/tree family | Deepen | virtualization, selection, sort, hierarchy, resize and accessible semantics |
| DS-028 | scroll container | Deepen | clipping, bars, wheel/touch/keyboard, anchoring and reduced motion |
| DS-029 | dialog/modal family | Keep | focus trap/restore, default/cancel, destructive confirmation and peer death |
| DS-030 | toast/notification primitive | Deepen | severity, action, timeout, pause, history and screen-reader announcement |
| DS-031 | progress family | Keep | determinate/indeterminate/staged, cancel, error and truthful backend state |
| DS-032 | empty/error/loading states | Deepen | actionable truthful states in every app/service workflow |
| DS-033 | card/panel/split-pane | Keep | semantic grouping, resize/collapse, keyboard and responsive layout |
| DS-034 | window chrome | Keep | title/icon/status, drag, controls, resize, focus, maximize and a11y |
| DS-035 | data visualization primitives | Deepen | axes/units/legend/contrast/text alternative and streaming limits |
| DS-036 | component gallery | Add | interactive states, tokens, semantics, performance and visual-regression fixture |
| DS-037 | declarative layout format | Add | versioned safe schema, validation, localization/scale and generated bindings |
| DS-038 | responsive breakpoints | Add | content-driven reflow for window, display, touch and remote viewport |
| DS-039 | content-writing rules | Add | clear labels, errors, empty states, dangerous actions and terminology registry |
| DS-040 | design-system versioning | Add | deprecations, migrations, compatibility ranges, consumers and release notes |

## 11. Accessibility, localization, and inclusive interaction

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| AX-001 | semantic UI tree | Add | role/name/value/state/relations/actions/bounds with authenticated ownership |
| AX-002 | semantic event stream | Add | focus/value/text/layout/live-region updates are bounded and generation-tagged |
| AX-003 | screen reader | Add | navigation, speech output, app/shell/browser coverage and privacy controls |
| AX-004 | braille provider | Later | contracted cells/routing keys, status and screen-reader integration |
| AX-005 | keyboard-complete navigation | Add | every route works without pointer with visible focus and logical order |
| AX-006 | focus appearance | Deepen | persistent high-contrast non-color indication in every theme/state |
| AX-007 | focus order/groups | Add | document/layout semantics, modal containment and restoration |
| AX-008 | shortcuts/access keys | Add | discoverable, remappable, conflict-resolved and localized |
| AX-009 | large text/system scale | Add | semantic scale without clipping, hidden controls or fixed-layout failure |
| AX-010 | magnifier | Add | full-screen/lens/docked modes, pointer/focus tracking and GPU/software paths |
| AX-011 | high contrast | Add | theme/system override with image/icon/state treatment and regression proofs |
| AX-012 | color filters | Add | grayscale/invert/color-deficiency transforms with capture/privacy policy |
| AX-013 | contrast checker | Add | design/user inspection for text, icons, focus and state pairs |
| AX-014 | reduced motion | Add | animations disabled/reduced without losing causality or progress |
| AX-015 | captions | Later | media/live system captions, language/style/position and privacy |
| AX-016 | audio descriptions | Later | alternate media track selection and controls |
| AX-017 | mono/balance audio | Add | system accessibility routing and test tone |
| AX-018 | sticky/filter/slow keys | Add | configurable keyboard transforms, indicators, persistence and escape |
| AX-019 | mouse keys | Add | keyboard pointer movement, acceleration and buttons |
| AX-020 | switch control | Later | scanning groups/actions, timing, device input and safe escape |
| AX-021 | dwell control | Later | configurable dwell targets/timing, feedback and cancellation |
| AX-022 | voice control | Later | on-device/remote policy, command grammar, confirmation and privacy |
| AX-023 | remapping | Add | keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints |
| AX-024 | touch target policy | Add | minimum target, spacing and coarse-pointer adaptation |
| AX-025 | flashing/seizure safety | Add | frequency/luminance limits and reduced-effects enforcement |
| AX-026 | cognitive accessibility | Add | plain language, predictable navigation, time extensions and recoverable workflows |
| AX-027 | Accessibility Center | Add | discover/test/preview/apply/rollback every assistive setting/provider |
| AX-028 | locale framework | Add | locale data, formats, plural rules, fallback and per-user choice |
| AX-029 | translation resource system | Add | stable message IDs, extraction, completeness, fallback and pseudo-locales |
| AX-030 | Unicode text | Deepen | UTF-8, grapheme/bidi/line break/normalization and malformed input policy |
| AX-031 | font fallback/shaping | Deepen | script/language coverage, emoji, missing-glyph diagnostics and cache keys |
| AX-032 | right-to-left layout | Later | bidi content plus mirrored shell/widgets and mixed-direction tests |
| AX-033 | timezone/calendar formats | Add | locale-aware wall time, calendars, first day, DST and user override |
| AX-034 | units/number formats | Add | decimal/grouping/currency/measurement preferences and round-trip parsing |
| AX-035 | pseudo-localization gate | Add | expansion, bidi, missing translation and hard-coded string detection |

## 12. Session, shell, windows, and desktop experience

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| SH-001 | login experience | Add | user selection, secure auth, recovery, accessibility and session start errors |
| SH-002 | session startup | Add | authenticated profile/settings/services/apps with readiness and rollback |
| SH-003 | lock/unlock | Add | compositor-owned secure surface, notifications/privacy policy and recovery |
| SH-004 | logout/shutdown/restart UX | Add | unsaved-work negotiation, deadlines, progress, errors and forced path |
| SH-005 | desktop background | Keep | wallpaper/color/theme, scaling, preview, cache, per-user persistence |
| SH-006 | top/status bar | Deepen | truthful clock/network/power/audio/session indicators and overflow |
| SH-007 | dock/taskbar | Keep | pinned/running/active/minimized states, grouping, reorder and multi-display policy |
| SH-008 | application menu | Keep | generated from admitted packages; search/category/recent and no dead routes |
| SH-009 | launcher/search | Deepen | app/file/setting/action search with ranking, keyboard and privacy |
| SH-010 | Run command | Keep | explicit parsing/authority/errors/history and app metadata resolution |
| SH-011 | All Applications catalog | Deepen | exact registry parity, metadata, install state, evidence and launchability |
| SH-012 | window create/close | Keep | owner authority, ready nonce, graceful close/deadline/force and cleanup |
| SH-013 | window move | Keep | drag handles/Alt path, multi-display bounds, retained composition |
| SH-014 | window resize | Keep | edge/corner/keyboard, minimum/maximum, configure/ack and preview |
| SH-015 | minimize/restore | Keep | taskbar state, focus fallback, animation/reduced motion and app notification |
| SH-016 | maximize/restore | Keep | work-area geometry, multi-display and correct hit regions |
| SH-017 | snap/tile | Add | edge/keyboard layouts, preview, gaps, restore and accessibility |
| SH-018 | fullscreen | Add | explicit entry/exit, overlays, shortcuts and focus/input policy |
| SH-019 | window switcher | Deepen | keyboard ordering, preview, titles/icons, minimized apps and a11y |
| SH-020 | workspace overview | Add | thumbnails, drag/move, keyboard, search and reduced-motion path |
| SH-021 | multiple workspaces | Add | persistent names/order, app/window association and shortcuts |
| SH-022 | modal/transient windows | Keep | parent ownership, stacking, focus, block scope and peer death |
| SH-023 | notification service | Add | app permissions, banners, actions, grouping, do-not-disturb and history |
| SH-024 | notification center | Add | searchable history, clear/settings, privacy and screen-reader announcements |
| SH-025 | clipboard service | Deepen | MIME offers, source/recipient, size, expiry, history, privacy and live revoke |
| SH-026 | clipboard history UI | Add | opt-in sensitive filtering, search, delete and per-app attribution |
| SH-027 | drag-and-drop broker | Add | capability transfer, MIME/actions, progress/cancel and failure rollback |
| SH-028 | file chooser portal | Add | open/save/folder modes, recent/places/search, scoped result and atomic save |
| SH-029 | open-with/share portal | Add | compatible apps/actions, user choice, remembered defaults and scoped handles |
| SH-030 | screen-capture portal | Add | target preview, consent, protected content, indicator and result handle |
| SH-031 | permission prompt | Add | exact action/resource/duration, understandable risk, deny/default and settings link |
| SH-032 | quick settings | Add | network/audio/brightness/power/a11y controls with effective-state feedback |
| SH-033 | system tray/status items | Later | authenticated bounded app indicators/actions, overflow and no arbitrary draw |
| SH-034 | clock/calendar panel | Deepen | locale/timezone, events, timers and notification integration |
| SH-035 | power/session menu | Add | lock/logout/suspend/restart/shutdown with permissions and unsaved-work state |
| SH-036 | onboarding | Add | theme/network/account/accessibility/privacy tour with skip/resume and truth |
| SH-037 | help system | Deepen | searchable contextual offline docs, shortcuts and version/provenance |
| SH-038 | crash UI | Add | app/service identity, recovery/relaunch/report details and no crash loop |
| SH-039 | busy/unresponsive UI | Add | progress, cancel, wait, terminate and preserved desktop input |
| SH-040 | session restore | Later | allowed apps/documents/windows restored with crash/privacy/user control |
| SH-041 | multi-user switching | Later | isolated sessions, device/display ownership, lock and resource policy |
| SH-042 | guest session | Later | ephemeral restricted state with verified teardown |
| SH-043 | kiosk/public-demo session | Add | fixed app/lease/limits/input/egress/reset and escape/admin policy |
| SH-044 | remote session | Later | authenticated lifecycle, resize/input/clipboard, reconnect and local privacy |
| SH-045 | shell performance telemetry | Deepen | launch/frame/input/service failure data surfaced without fake values |

## 13. Application platform, packaging, and common services

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| AP-001 | ApplicationManifest | Add | identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature |
| AP-002 | app registry | Deepen | generated exact package/build/image/route metadata with no blank/dead IDs |
| AP-003 | app launch | Deepen | validate, allocate process/handles, publish, ready nonce or exact rollback |
| AP-004 | app lifecycle | Add | staged/running/background/suspended/stopping/crashed/quarantined with deadlines |
| AP-005 | app supervision | Add | health, crash budget/backoff, cleanup, relaunch and user-visible state |
| AP-006 | app sandbox | Add | manifest baseline, portal mediation, monotonic reduction and hostile tests |
| AP-007 | app resource budgets | Add | memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced |
| AP-008 | app data directories | Add | package/read-only, per-user config/data/cache/state/temp with backup policy |
| AP-009 | settings service | Deepen | typed versioned schema, validation, transaction, notification, migration and rollback |
| AP-010 | secrets service | Add | app-scoped non-exportable handles, unlock/rotate/revoke and audit |
| AP-011 | portal service | Add | authenticated open/save/share/capture/device/secret operations and user consent |
| AP-012 | MIME/association service | Add | safe type resolution, defaults, open-with and app capability compatibility |
| AP-013 | localization service | Add | resources, locale change, fallback, formats and restart/live-update policy |
| AP-014 | font service | Add | discovery, shaping/rasterization caches, permissions, fallback and diagnostics |
| AP-015 | image decoder service | Add | restricted process, bounded input/output/time and typed surfaces |
| AP-016 | media decoder service | Later | demux/decode workers, clocks, queues, failure containment and capability negotiation |
| AP-017 | thumbnail service | Add | asynchronous cancelable sandboxed previews with cache and privacy |
| AP-018 | search/index service | Add | permission-aware per-user indexing, query budgets and privacy controls |
| AP-019 | notification API | Add | permissions, urgency, actions/progress, replacement and lifecycle |
| AP-020 | clipboard API | Add | typed MIME offers/transfers, ownership and cancellation |
| AP-021 | file API | Add | handle-relative operations, atomic save, watch/change, cancellation and errors |
| AP-022 | network request API | Add | capability-scoped destinations, TLS policy, quotas, deadlines and provenance |
| AP-023 | audio API | Add | stream/session handles, formats, latency, volume and device changes |
| AP-024 | camera/microphone API | Later | explicit user-mediated device grants, indicator and revoke |
| AP-025 | print API | Later | document/job handle, options, progress, cancel and privacy |
| AP-026 | background-task API | Add | declared reason, time/resource budget, user visibility and cancellation |
| AP-027 | app update/migration | Add | signed atomic generation, schema migration, rollback and compatibility |
| AP-028 | app store/catalog | Add | signed index, provenance, permissions, compatibility, evidence and reviews policy |
| AP-029 | developer mode/sideload | Add | explicit risk boundary, signatures/keys, isolation, diagnostics and revocation |
| AP-030 | app backup/export | Add | declared user-data set, secrets exclusion, version and restore proof |
| AP-031 | document model | Add | origin, dirty/conflict/autosave/recovery, atomic save and recent items |
| AP-032 | undo/redo service pattern | Add | bounded command history, grouping, persistence policy and failure semantics |
| AP-033 | command/action registry | Add | semantic actions, labels, shortcuts, enablement and agent/a11y invocation |
| AP-034 | plugin/extension model | Later | signed scoped extension processes, versioned API, quotas and disable/recovery |
| AP-035 | app evidence page | Add | routes, permissions, services, resource use, crashes, provenance and proof state |
| AP-036 | online-account broker | Later | provider identity/auth tokens behind scoped handles, revoke, refresh and privacy |
| AP-037 | sync service | Later | per-dataset conflict/offline/retry/encryption/quota/account and user-visible status |
| AP-038 | background transfer service | Add | resumable bounded upload/download, connectivity/power policy, progress and cancellation |
| AP-039 | location service | Later | device/provider fusion, precision, foreground/background consent and audit |
| AP-040 | sensor service | Later | typed units/rates/batching/permissions/calibration and privacy |

## 14. Browser and web platform

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| WB-001 | browser chrome | Deepen | address/search, back/forward/reload/stop, tabs, downloads, permissions and errors |
| WB-002 | URL parser | Deepen | standards-oriented checked parsing, normalization, schemes and display safety |
| WB-003 | navigation broker | Add | committed history, redirects, replacement cancellation, policy and provenance |
| WB-004 | browser profile | Add | per-user history/bookmarks/settings/storage/downloads with private mode |
| WB-005 | content-process isolation | Add | per-site/site-instance restricted processes with crash containment |
| WB-006 | request/network service | Add | DNS/TLS/HTTP outside content/UI with destination and response limits |
| WB-007 | decoder isolation | Add | image/font/media/document parsers in disposable bounded processes |
| WB-008 | HTML parser | Deepen | streaming tree construction, malformed recovery, limits and hostile corpus |
| WB-009 | DOM | Deepen | node/document/events/mutation/lifecycle with quotas and test ledger |
| WB-010 | CSS parser/cascade | Deepen | selectors, origins, inheritance, variables, media queries and explicit support table |
| WB-011 | style engine | Deepen | incremental invalidation, computed values, pseudo states/elements and budgets |
| WB-012 | layout engine | Deepen | flow, flex, grid stages, intrinsic sizing, overflow, writing modes and tests |
| WB-013 | web paint/display list | Deepen | retained clips/transforms/text/images/layers and damage propagation |
| WB-014 | browser compositing | Add | scrolling/layers/animations mapped safely into system surfaces |
| WB-015 | JavaScript engine | Deepen | explicit language/API support, execution budgets, errors, GC and isolation |
| WB-016 | event loop | Deepen | tasks/microtasks/timers/render steps, cancellation and background throttling |
| WB-017 | Web Workers | Deepen | isolated workers, message quotas, termination and origin policy |
| WB-018 | WebAssembly | Later | validated modules, bounded memory/tables, imports, execution budget and no fake exports |
| WB-019 | Canvas 2D | Deepen | bounded surfaces, drawing state, image/text integration and readback policy |
| WB-020 | SVG | Deepen | explicit supported subset, geometry/text/paint, parser limits and fallbacks |
| WB-021 | MathML | Later | explicit layout/accessibility subset and tests |
| WB-022 | forms/controls | Deepen | values, labels, validation, submission, keyboard, autofill policy and a11y |
| WB-023 | focus/selection/clipboard | Deepen | DOM focus, text selection, copy/paste permissions and system integration |
| WB-024 | scrolling | Deepen | viewport/element scroll, anchors, smooth/reduced motion and virtualization |
| WB-025 | cookies | Deepen | domain/path/expiry/SameSite/Secure/HttpOnly, quotas and clearing |
| WB-026 | local/session storage | Deepen | per-origin quotas, persistence/private mode, atomicity and eviction |
| WB-027 | IndexedDB-like storage | Later | transactional per-origin structured storage, quotas and recovery |
| WB-028 | cache | Deepen | HTTP semantics, bounded storage, vary/revalidate/evict and private mode |
| WB-029 | CORS/referrer/origin policy | Deepen | complete enforcement with preflight/cache and hostile tests |
| WB-030 | CSP/mixed-content policy | Add | enforce content sources, script policy, upgrade/block and reporting |
| WB-031 | permissions model | Add | origin-scoped camera/mic/location/clipboard/notification grants and dashboard |
| WB-032 | sandboxed frames | Deepen | iframe depth/resources/origin/sandbox/message policy and process boundary |
| WB-033 | downloads | Deepen | user-mediated destination, size/progress/cancel/hash/quarantine and provenance |
| WB-034 | uploads/file chooser | Add | portal-scoped file handles, names/metadata/privacy and cancellation |
| WB-035 | bookmarks/history | Add | durable searchable per-profile data, sync/export/privacy and clearing |
| WB-036 | tabs/windows | Add | lifecycle, suspension, crash state, restore, resource budget and a11y |
| WB-037 | private browsing | Add | isolated ephemeral storage/history/cache with explicit limits |
| WB-038 | password/autofill integration | Later | secrets handles, origin binding, consent, phishing resistance and no page access |
| WB-039 | certificate/security UI | Add | verified origin, connection/cert errors, override policy and details |
| WB-040 | page accessibility | Add | semantic tree, names/roles/states, focus, live regions and screen reader |
| WB-041 | developer tools | Later | DOM/style/layout/network/console/performance inspection with debug authority |
| WB-042 | reader mode | Add | extracted readable content, provenance, images, typography and a11y |
| WB-043 | find in page | Add | incremental cancelable Unicode search and highlighted semantics |
| WB-044 | print/PDF output | Later | paged layout, preview, privacy and print service integration |
| WB-045 | browser performance telemetry | Add | navigation phases, long tasks, layout/paint, frame drops, memory and crashes |
| WB-046 | hostile-page recovery | Add | stop/kill/reload content process while browser UI and session remain responsive |
| WB-047 | agent-readable web tools | Add | permissioned link/text/form/action model distinct from pixels and page authority |
| WB-048 | web compatibility ledger | Add | supported standards/tests/version and explicit gaps; never “Chrome parity” by implication |
| WB-049 | browser extensions | Later | signed restricted extension processes, declared page/network/UI grants and disable/recovery |
| WB-050 | installable web apps | Later | manifest/service-worker/storage/notifications/offline package with origin-bound sandbox |

## 15. Files, documents, office, and knowledge work

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| PD-001 | file manager | Deepen | places/tree/list/grid, search, open, copy/move/rename/trash, progress/cancel and permissions |
| PD-002 | file properties | Add | type/size/location/times/owner/permissions/hash/provenance and safe edits |
| PD-003 | disk usage explorer | Deepen | permission-aware cancelable snapshot with largest consumers and removable media |
| PD-004 | archive manager | Deepen | list/create/extract/test with traversal/link/bomb checks and transactional destination |
| PD-005 | text editor | Deepen | multi-document text, selection, find/replace, undo, encoding, autosave/recovery, atomic save |
| PD-006 | code editor | Later | syntax/LSP-like diagnostics, project search, symbols, build/debug and extension isolation |
| PD-007 | notes | Deepen | durable Markdown/rich notes, folders/tags/search/links/autosave/export and privacy |
| PD-008 | sticky notes | Deepen | small persistent desktop notes, per-user state, recovery and privacy |
| PD-009 | document viewer | Add | paged/reflow text, outline/search/select/copy/annotations and accessibility |
| PD-010 | PDF viewer | Add | isolated parser/rasterizer, pages/search/text/links/forms/annotations/print |
| PD-011 | ebook/reader | Add | EPUB-like package/parser isolation, navigation, typography, bookmarks and a11y |
| PD-012 | word processor | Later | structured document editing, styles, layout, tables/images, change tracking, import/export |
| PD-013 | spreadsheet | Later | typed cells/formulas/dependencies/recalc, sheets, charting, import/export and accessibility |
| PD-014 | presentation editor | Later | slides/layouts/themes/media/notes/presenter view, import/export and playback |
| PD-015 | presentation viewer | Later | safe deck playback, display selection, controls and accessible alternatives |
| PD-016 | plain/rich clipboard integration | Add | format offers, sanitization, source attribution and privacy |
| PD-017 | spelling/dictionary | Add | language dictionaries, suggestions, custom words, provenance and offline behavior |
| PD-018 | grammar/writing assistance | Later | local/remote policy, provenance, privacy, reversible suggestions and accessibility |
| PD-019 | diff/merge | Deepen | scalable line/word/syntax compare, files, conflicts, apply and undo |
| PD-020 | hex viewer/editor | Deepen | paged huge files/devices, offsets/search/types, read-only default and guarded writes |
| PD-021 | checksum verifier | Deepen | streaming algorithms, compare workflow, file handles, provenance and clear mismatch |
| PD-022 | regex tester | Deepen | named dialect, limits/timeouts, captures/highlights and malformed-input UX |
| PD-023 | calculator | Deepen | expression parser, precision, history, scientific/programmer modes and errors |
| PD-024 | unit converter | Deepen | versioned unit metadata, precision, locale, categories, favorites and history |
| PD-025 | base converter | Deepen | signed/arbitrary-width values, representation, overflow and copy |
| PD-026 | color picker | Deepen | palette/history/formats/contrast and user-mediated screen sampling |
| PD-027 | calendar | Add | local calendars, events/recurrence/reminders/timezones/import/export and privacy |
| PD-028 | tasks/todo | Add | lists, due/recurrence/priorities/reminders/search and offline data |
| PD-029 | contacts | Add | structured contacts, groups/search/import/export/account privacy |
| PD-030 | knowledge base | Add | notes/documents/links/search/citations/provenance and agent-readable handles |
| PD-031 | file picker service app | Add | reusable open/save workflows, recent/places/search and scoped returns |
| PD-032 | print preview | Later | pagination/options/scale/color/accessibility before job submission |
| PD-033 | print queue | Later | printer/jobs/status/reorder/pause/cancel/errors and privacy |
| PD-034 | scanner/document capture | Later | source/settings/preview/crop/OCR/save/privacy and cancellation |
| PD-035 | OCR | Later | isolated image/text extraction, language, confidence, layout and privacy |
| PD-036 | recent documents | Add | user-scoped provenance-aware history, missing items, privacy and clear |
| PD-037 | templates | Later | versioned document/sheet/deck templates with origin/license and preview |
| PD-038 | import/export broker | Add | explicit formats, isolated conversions, loss report and atomic output |
| PD-039 | document collaboration | Later | identity, sharing, conflict/merge, offline, audit and encryption |
| PD-040 | document recovery center | Add | discover autosaves/crash copies, compare, restore/discard and provenance |
| PD-041 | cloud/network drive | Later | account-scoped mount/cache/offline/conflict/quota and safe disconnect |
| PD-042 | cross-device document sync | Later | encrypted versioned sync, conflicts, deletion recovery and user-visible state |
| PD-043 | citation/reference manager | Later | sources/metadata/attachments/collections/export and provenance |

## 16. Media, creation, communication, and personal apps

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| MD-001 | image viewer | Deepen | folder navigation, zoom/pan/rotate/fullscreen, metadata, profiles and errors |
| MD-002 | gallery/photo library | Add | import, albums, search, metadata, thumbnails, duplicates, removable media and privacy |
| MD-003 | paint/raster editor | Deepen | tools, brushes, palette, selection, layers, history, formats, export and recovery |
| MD-004 | vector drawing | Later | paths/shapes/text/layers/transforms/import/export and accessibility |
| MD-005 | screenshot editor | Later | portal capture, crop/annotate/redact/export and protected-content policy |
| MD-006 | audio player | Add | library/queue/playback/seek/volume/metadata and media-session integration |
| MD-007 | video player | Later | demux/decode, seek/subtitles/fullscreen/audio sync and crash containment |
| MD-008 | music library | Later | indexed metadata/art/playlists/search/import and removable media |
| MD-009 | voice recorder | Later | microphone consent, level, pause/resume, format, naming and atomic save |
| MD-010 | audio editor | Later | waveform, selection, cut/mix/effects/undo/export and bounded processing |
| MD-011 | synthesizer | Deepen | notes/oscillators/envelopes/filters/presets/MIDI, realtime budget and export |
| MD-012 | mixer | Later | track/input/output routing, meters, gain/pan/effects/record and recovery |
| MD-013 | camera app | Later | live preview, capture/video, device/settings, privacy indicator and gallery handoff |
| MD-014 | screen recorder | Later | target/audio/quality/indicator/protected content, pause and atomic finalize |
| MD-015 | media converter | Later | isolated decode/encode, formats/presets/progress/cancel and provenance |
| MD-016 | animation/demo studio | Later | timeline/keyframes/assets/export and deterministic visual-regression mode |
| MD-017 | 3D viewer/demo | Deepen | scene/camera/input/assets/render backend and performance receipts |
| MD-018 | font atlas/inspector | Deepen | coverage/search/metrics/shaping/fallback/license and regression views |
| MD-019 | framebuffer/renderer inspector | Keep | capture, formats/layers/damage/performance with privileged read-only access |
| MD-020 | mail | Later | accounts, sync, folders/search/compose/attachments/offline/security/privacy |
| MD-021 | messaging/chat | Later | identity, conversations, attachments, notifications, encryption policy and offline |
| MD-022 | RSS/news reader | Add | feeds, update, articles, offline, read state, import/export and safe web content |
| MD-023 | maps | Later | tiles/search/routes/locations/offline/cache/privacy and accessibility |
| MD-024 | weather | Add | real provider/provenance, location consent, forecast/units/offline and errors |
| MD-025 | clock | Keep | locale/timezone, multiple clocks and trustworthy wall time |
| MD-026 | timer/stopwatch | Deepen | monotonic countdown/laps, background/suspend, alarms and notifications |
| MD-027 | alarms | Add | recurrence/sound/notification/suspend wake and dismissal/snooze |
| MD-028 | password manager UI | Later | human unlock, scoped fill/copy, generator, audit, export and secret-zero policy |
| MD-029 | download manager | Add | source/destination/progress/pause/resume/cancel/hash/quarantine and history |
| MD-030 | sharing center | Later | nearby/network/app targets, user consent, progress, revoke and audit |
| MD-031 | voice/video calling | Later | identity/signaling/media encryption/devices/quality/privacy and failure recovery |
| MD-032 | podcast/radio | Later | feeds/streams/downloads/queue/metadata/offline and network/audio integration |
| MD-033 | screen sharing | Later | target consent/indicator/protected content/input policy and revocation |

## 17. System settings, administration, diagnostics, and recovery UX

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| ST-001 | Settings application | Deepen | searchable categories, effective state, validation, preview, apply/rollback and profiles |
| ST-002 | display settings | Deepen | modes/scales/rotation/topology/night/color with timed rollback |
| ST-003 | appearance settings | Deepen | theme/wallpaper/density/font/icons/effects/reduced motion with live preview |
| ST-004 | input settings | Deepen | devices/layouts/repeat/pointer speed/acceleration/buttons/calibration/remap |
| ST-005 | audio settings | Add | devices/routes/volume/balance/test/per-app streams and errors |
| ST-006 | network settings | Add | interfaces/Wi-Fi/profiles/IP/DNS/proxy/VPN/firewall and diagnostics |
| ST-007 | accounts settings | Add | users/groups/password/recovery/admin/session and privacy |
| ST-008 | application settings | Add | defaults, permissions, storage, background, notifications, repair/uninstall |
| ST-009 | accessibility settings | Add | all assistive providers/preferences with preview/test/rollback |
| ST-010 | locale/time settings | Add | language/format/timezone/clock sync/keyboard/input methods |
| ST-011 | power settings | Later | sleep/display/battery/thermal/performance profiles and truthful capability |
| ST-012 | privacy settings | Add | permission history, telemetry, recent data, capture, location and clearing |
| ST-013 | update settings | Add | channels/checks/download/install/schedule/history/rollback and provenance |
| ST-014 | storage settings | Add | volumes/usage/cleanup/encryption/removable/backup and disk health |
| ST-015 | device manager | Add | hardware/driver/resources/status/permissions/reset/eject/evidence |
| ST-016 | printer/scanner settings | Later | discovery/defaults/options/queues/test/error and privacy |
| ST-017 | system information | Deepen | CPU/memory/firmware/devices/build/ABI/licenses/security and copy/export |
| ST-018 | system monitor | Deepen | processes/CPU/memory/I/O/network/graphics/services with units and control authority |
| ST-019 | process manager | Add | inspect/filter/signal/priority/resources/details with permission checks |
| ST-020 | services manager | Deepen | supervisor state/dependencies/health/logs/start/stop/restart/quarantine and auth |
| ST-021 | driver monitor | Add | lifecycle/resources/IRQs/DMA/faults/recovery and exact provider evidence |
| ST-022 | network monitor | Add | link/routes/sockets/DNS/traffic/errors and privacy-aware process attribution |
| ST-023 | storage monitor | Add | devices/queues/cache/fs health/space/writeback/flush/errors and detach |
| ST-024 | graphics monitor | Deepen | mode/backend/frame phases/damage/cache/fences/late frames and capture |
| ST-025 | audio monitor | Add | devices/streams/xruns/latency/clocks and permissions |
| ST-026 | kernel/system log viewer | Deepen | filter/search/correlation/redaction/persistent receipts/export and privilege |
| ST-027 | crash center | Add | app/service/kernel crashes, symbols, privacy, reports and recovery actions |
| ST-028 | benchmark app | Deepen | named reproducible workloads, comparisons, artifacts and thermal/host context |
| ST-029 | diagnostics suite | Add | guided CPU/memory/storage/display/input/network/audio tests and export |
| ST-030 | health dashboard | Add | service/device/update/backup/security state with actionable evidence |
| ST-031 | disk repair UI | Add | read-only diagnosis, target confirmation, backup, repair plan/result and rollback |
| ST-032 | boot recovery UI | Add | generations/safe mode/logs/export/repair with keyboard/a11y support |
| ST-033 | factory/user reset | Later | exact retained/erased data, confirmation, backup, cryptographic erase and receipts |
| ST-034 | support bundle | Add | user-reviewed redacted logs/config/hardware/crashes/artifact identities |
| ST-035 | About/product identity | Deepen | immutable version/build/origin/license/update/evidence, not hard-coded fake facts |
| ST-036 | security center | Add | update/boot/firewall/permissions/keys/events/risk status with actionable evidence |
| ST-037 | active sessions | Add | local/remote/login identity, devices, activity, lock/terminate and audit |

## 18. Terminal, shell, utilities, and development environment

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| DE-001 | TTY/virtual console | Deepen | byte-counted I/O, resize, sessions, permissions and stable low-level fallback |
| DE-002 | PTY | Add | bounded duplex master/slave, poll/close/hangup, controlling session and stress tests |
| DE-003 | Terminal application | Keep | PTY client, Unicode text, scrollback, selection, resize, profiles and child lifecycle |
| DE-004 | interactive shell | Deepen | tokenizer/parser, quoting, expansion, variables, status, errors and job control |
| DE-005 | pipelines/redirection | Add | handle-based stdin/out/err, files, append, errors and cancellation |
| DE-006 | shell job control | Add | foreground/background, groups, signals, wait, terminal ownership and notifications |
| DE-007 | shell scripting | Later | defined syntax, conditionals/loops/functions/errors/modules and deterministic tests |
| DE-008 | core file utilities | Deepen | truthful `ls/cat/cp/mv/rm/mkdir/touch/find` semantics with permissions/errors |
| DE-009 | text utilities | Deepen | grep/sort/head/tail/wc/diff/encoding with streaming bounds |
| DE-010 | process utilities | Deepen | ps/top/kill/time/uptime with real service data and authorization |
| DE-011 | storage utilities | Deepen | mount/df/du/fsck/format with explicit authority and destructive confirmation |
| DE-012 | network utilities | Deepen | ip/route/ping/dns/socket/curl-like tools using real services, never simulation |
| DE-013 | archive/hash utilities | Deepen | bounded parsers, streaming, verification and clear algorithms |
| DE-014 | system administration CLI | Add | service/user/package/update/device/power operations behind explicit handles |
| DE-015 | help/man pages | Add | version-matched searchable usage, examples, exit status and links |
| DE-016 | command completion/history | Add | shell-aware suggestions, per-user persistence, sensitive filtering and clearing |
| DE-017 | environment/config | Add | scoped variables, startup files, precedence, validation and provenance |
| DE-018 | zl interpreter | Keep | language semantics, diagnostics, module resolution and sandboxed execution modes |
| DE-019 | zl compiler | Keep | reproducible native/object outputs, diagnostics, optimization and target manifests |
| DE-020 | assembler/linker integration | Deepen | named sections/symbols/relocations/layout, map output and checked errors |
| DE-021 | package/build tool | Add | manifests, dependency graph, cache, targets, tests, provenance and strict failures |
| DE-022 | source debugger | Later | processes/threads/breakpoints/registers/memory/stack/source with debug authority |
| DE-023 | profiler | Later | CPU/allocation/I/O/frame samples, symbols, overhead and export |
| DE-024 | tracing tools | Add | typed events, filters/correlation, live/file views, privacy and drop evidence |
| DE-025 | test runner | Deepen | discovery/filter/shard/timeout/skip/failure/artifact and target/host lanes |
| DE-026 | fuzz runner | Add | production entrypoints, corpus, minimization, sanitizer and durable failures |
| DE-027 | benchmark runner | Deepen | warmup/samples/clock/statistics/baseline/regression and environment identity |
| DE-028 | emulator/QEMU launcher | Deepen | exact images/config/device matrix, assertions, timeout and artifact capture |
| DE-029 | SDK/sysroot | Add | headers/libraries/tools/ABI metadata/examples and exact consumer matrix |
| DE-030 | API documentation | Add | generated signatures/contracts/examples/errors/version and source links |
| DE-031 | component/app template | Add | minimal correct manifest/process/AppKit/tests/assets/provenance scaffold |
| DE-032 | IDE/workbench | Later | project/editor/build/test/debug/profile/docs/package workflows |
| DE-033 | version control client | Later | repository/status/diff/commit/branch/network auth with secrets isolation |
| DE-034 | self-hosted build | Later | matching compiler/system/packages rebuilt inside zlOS from declared inputs |
| DE-035 | developer diagnostics mode | Add | extra checks/traces without weakening production authority or changing semantics |
| DE-036 | crash symbolizer | Deepen | checked build-ID symbol sources, safe malformed-object handling and source mapping |
| DE-037 | binary inspectors | Deepen | ELF/PE/archive/fs/protocol structure with length-first parsing and read-only defaults |
| DE-038 | visual UI inspector | Add | widget/semantic/layout/tokens/damage/performance inspection with debug grant |
| DE-039 | browser developer tools | Later | console/network/DOM/style/layout/performance/storage under content isolation |
| DE-040 | documentation browser | Add | offline version-matched docs, search, examples, provenance and links |

## 19. Compatibility, ports, virtualization, and emulation

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| CP-001 | POSIX compatibility ledger | Deepen | per-function semantic tests driven by selected real programs |
| CP-002 | libc | Deepen | headers/runtime/threads/time/files/network/locale with conformance and ABI |
| CP-003 | C/C++ application ABI | Add | startup, TLS, exceptions where selected, dynamic/static linking and ownership |
| CP-004 | ELF dynamic loader | Add | length-first segments/relocations/dependencies/symbols/TLS/W^X and atomic caches |
| CP-005 | shared libraries | Add | ABI/version/soname/dependency/refcount/unload policy and reproducible packages |
| CP-006 | Windows/PE compatibility subsystem | Later | quarantined PE/DLL loader plus program-driven NT/Win32, files, registry, windows, graphics, input, audio, network, COM and selected Direct3D translation after native process contracts |
| CP-007 | Linux ABI and userland compatibility | Later | isolated program-driven Linux syscalls, ELF interpreter/dynamic linker, signals/futex, `/proc`/`/dev`, files/network and selected userland/container workloads after native process contracts |
| CP-008 | Wayland-like client protocol | Add | versioned bounded surface/input/output protocol and compatibility tests |
| CP-009 | SDL-like app layer | Later | graphics/input/audio/files/time/thread subset for selected ports |
| CP-010 | terminal/ANSI compatibility | Deepen | explicit escape/Unicode/resize/input table and hostile sequence bounds |
| CP-011 | third-party port framework | Add | pinned recipe, patches, licenses, dependencies, sandbox, build and scenario proof |
| CP-012 | port catalogue | Add | installed vs available, origin, license, version, capabilities and test status |
| CP-013 | DOOM/game ports | Fixture | integration workload with exact runtime/audio/input/file evidence, not subsystem proof |
| CP-014 | Unix utilities ports | Later | selected real workflows under native ownership/security semantics |
| CP-015 | language runtime ports | Later | runtime-specific sandbox/resource/FFI/file/network tests |
| CP-016 | Windows/Linux virtual machine monitor | Later | hardware-assisted guest CPU/memory/device isolation, lifecycle, licensed images, console, networking, snapshots and limits |
| CP-017 | Windows/Linux guest integration | Later | time/input/display/audio/network/clipboard/files and optional seamless windows with explicit consent and protocol versions |
| CP-018 | CPU and device emulation | Deepen | deterministic selected ISA plus virtio/legacy device models, translation caches, quotas and fault injection |
| CP-019 | Game Boy/retro emulation | Later | legal ROM boundary, CPU/video/audio/input/save correctness and quotas |
| CP-020 | compatibility evidence UI | Add | shows exact supported programs/APIs/tests and failures instead of broad labels |

## 20. Games, demos, learning, and validation experiences

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| GM-001 | deterministic game runtime | Deepen | monotonic tick, seeded randomness, replay, pause/background and resource limits |
| GM-002 | game input | Deepen | keyboard/pointer/gamepad mapping, focus/grab and accessible remapping |
| GM-003 | game audio | Add | AudioServer cues/music, per-app volume and device recovery |
| GM-004 | save/high scores | Add | per-user durable data, versioning, atomic save and privacy |
| GM-005 | game frame/latency gate | Add | input-to-present, peak frame, dropped updates and exact scenario receipt |
| GM-006 | color/motion/timing accessibility | Add | non-color cues, reduced motion, adjustable timing and keyboard alternatives |
| GM-007 | board-game surface | Keep | deterministic rules, keyboard/pointer semantics, status, reset and replay |
| GM-008 | arcade/collision surface | Keep | movement/collision/score/lives/timing and bounded frame behavior |
| GM-009 | cellular simulation | Keep | grid editing, stepping/running, speed, reset and deterministic patterns |
| GM-010 | rendering benchmark demo | Keep | workload identity, backend comparison, controls and honest results |
| GM-011 | 3D graphics demo | Keep | reusable scene/camera primitives and backend/performance oracle |
| GM-012 | audio/DSP demos | Deepen | golden outputs plus realtime-service integration when claimed |
| GM-013 | driver/input test apps | Keep | class-specific event visualization and physical evidence capture |
| GM-014 | UI component gallery | Add | all states/themes/scales/locales/a11y and visual-performance regression |
| GM-015 | onboarding/tutorial games | Later | teach input/window/accessibility without hiding product state |
| GM-016 | classic game ports | Later | packaged sandboxed external content with license/provenance and cleanup |
| GM-017 | game catalog | Deepen | generated exact route/ID/icon/package metadata and launch proof |
| GM-018 | replay/export | Add | portable versioned input/state trace for bugs and deterministic review |
| GM-019 | game crash containment | Add | process failure removes handles/surfaces/audio/grabs without harming session |
| GM-020 | benchmark anti-cheat/truth | Add | no simulated score/result; exact code/artifact/backend/workload receipt |

## 21. Agents, automation, and human-tool collaboration

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| AG-001 | agent role manifest | Add | identity, purpose, model/tool inputs, authority, exposure, resources and lifecycle |
| AG-002 | typed plan | Add | inspectable versioned steps/preconditions/effects/rollback/proof before execution |
| AG-003 | deterministic tool broker | Add | validates operation/arguments/handles/policy and emits effect receipt |
| AG-004 | scoped filesystem tools | Add | handle-relative read/write/search/patch with bounds, diff and rollback |
| AG-005 | scoped process tools | Add | exact executable/args/env/resources/deadline/output and cancellation |
| AG-006 | scoped network tools | Add | allowed destinations/methods/bytes/time/provenance and egress logging |
| AG-007 | scoped UI/browser tools | Add | semantic targets, authenticated session, action receipts and no ambient input |
| AG-008 | consent checkpoint | Add | binds exact effect/target/duration/digest to fresh human decision |
| AG-009 | human interruption/redirect | Add | safe cancellation boundary, state reconciliation and revised plan |
| AG-010 | subagent delegation | Add | live-parent attenuation, bounded concurrency/output/time, serialized revoke/spawn |
| AG-011 | agent termination | Add | task becomes dead and every child/tool/handle/process is stale or cleaned |
| AG-012 | role separation | Add | planner/executor/provider/reviewer/remote gateway carry different authority |
| AG-013 | independent review | Add | reviewer sees plan/diff/receipts, seeks counterexample, accepts/rejects only |
| AG-014 | agent memory | Add | explicit user-approved durable artifacts, provenance, retention and revocation |
| AG-015 | context assembly | Add | bounded task-scoped sources, freshness, citations and private-data exclusions |
| AG-016 | skill/package format | Add | signed versioned instructions/tools/assets, declared grants/dependencies and tests |
| AG-017 | atomic skill install | Add | stage/verify/admit/publish or exact rollback across files/tools/roles/grants |
| AG-018 | skill uninstall/revoke | Add | stop users, revoke live authority, deregister and remove per retention policy |
| AG-019 | model/provider abstraction | Add | capabilities, limits, privacy, health, fallback and exact model provenance |
| AG-020 | agent app workspace | Add | chat/action panes, tabs, files, traces, plans, artifacts, permissions and status |
| AG-021 | command/tool discovery | Add | searchable typed actions with parameters, permissions, examples and evidence |
| AG-022 | task lifecycle | Add | queued/running/waiting/review/failed/completed/cancelled with durable receipts |
| AG-023 | task budgets | Add | tokens/time/CPU/memory/process/network/storage/output and admission/backpressure |
| AG-024 | plan approval policy | Add | risk-based automatic vs human gates, non-reusable approvals and audit |
| AG-025 | provenance/taint labels | Add | scoped origin/effect tracking, conservative sinks and explicit declassification |
| AG-026 | agent notifications | Add | background progress/attention/completion with privacy and rate limits |
| AG-027 | scheduled automation | Later | authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics |
| AG-028 | remote worker | Later | authenticated role-minimal channel, health, checkpoint, budgets and revocation |
| AG-029 | agent observability | Add | plans/actions/authority/resources/failures/skips without leaking secrets |
| AG-030 | agent recovery | Add | checkpoint/idempotency/retry policy, partial-effect reconciliation and user control |

## 22. Public demo, operations, release, and support

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| OP-001 | release build pipeline | Add | clean locked build, tests, signatures, SBOM/provenance and immutable artifacts |
| OP-002 | release channels | Add | stable/testing/developer policy, compatibility, rollback and user choice |
| OP-003 | image publishing | Add | signed raw/ISO/VM/hardware artifacts with hashes, instructions and origin |
| OP-004 | upgrade migration | Add | settings/data/package/schema transition, failure rollback and recovery docs |
| OP-005 | telemetry policy | Add | opt-in/essential split, schema, minimization, retention, export/delete and no secrets |
| OP-006 | metrics/health | Add | service/device/session/resource health with stable units and drop evidence |
| OP-007 | log collection | Deepen | bounded local logs, rotation, crash survival, redaction and user export |
| OP-008 | alerting | Add | actionable severity/dedup/rate limits/ownership and recovery link |
| OP-009 | public landing page | Add | truthful product/artifact/requirements/status/accessibility and start path |
| OP-010 | demo capacity | Add | slots, CPU/RAM/PID/disk/egress quotas and visible availability |
| OP-011 | transactional demo lease | Add | allocate/overlay/launch/publish atomically or rollback every resource |
| OP-012 | disposable guest overlay | Add | known base, per-session writable layer, quota and verified deletion |
| OP-013 | supervised QEMU guest | Add | exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion |
| OP-014 | authenticated VNC/WebSocket route | Add | unenumerable per-lease data plane with expiry/revoke on every action |
| OP-015 | demo session states | Add | starting/ready/reconnecting/failed/expiring/ended with server authority |
| OP-016 | demo input UX | Add | focus/release/stuck-key/mobile keyboard/clipboard policy and a11y |
| OP-017 | end/release action | Add | explicit idempotent user termination and verified process/slot cleanup |
| OP-018 | reaper | Add | expiry/orphan/crash cleanup with identity checks and immutable receipt |
| OP-019 | reconnect | Add | authenticated bounded retry to same live lease, never a fake constructor loop |
| OP-020 | demo administration | Add | status/leases/kill/capacity/reap with auth, JSON and audit |
| OP-021 | abuse/rate protection | Add | start/input/network quotas, per-source policy and safe denial |
| OP-022 | service backup/restore | Add | configuration/state/artifact backup with periodic restore drill |
| OP-023 | incident mode | Add | preserve evidence, contain, communicate, recover and document without destroying state |
| OP-024 | support documentation | Add | install/boot/use/troubleshoot/recover/hardware matrix tied to versions |
| OP-025 | issue/report workflow | Add | user-reviewed diagnostics, reproduction, privacy, artifact identity and status |
| OP-026 | compatibility/hardware matrix | Add | exact machine/device/firmware/config/evidence and known limitations |
| OP-027 | rescue media distribution | Add | separately signed, read-only default, target confirmation and repair log |
| OP-028 | supply-chain incident response | Add | revoke keys/packages/images, identify affected installs, recover and notify |
| OP-029 | end-of-life policy | Later | support window, migration/export, key/package/archive retention and notice |
| OP-030 | public evidence portal | Add | browsable claims, receipts, tests, failures, source/artifacts and corrections |

## 23. zl language, compiler, runtime, and self-hosting

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| ZL-001 | lexical grammar | Keep | exact tokens/comments/literals/Unicode/error recovery shared across stages |
| ZL-002 | parser/AST | Keep | deterministic grammar, source spans, diagnostics, malformed corpus and no false recovery |
| ZL-003 | module/import identity | Deepen | canonical paths/namespaces, cycles, visibility, reproducible resolution and platform packages |
| ZL-004 | local/global scope | Deepen | local by default, explicit global mutation, closures and shadowing semantics |
| ZL-005 | functions/calls | Keep | signatures, defaults/variadics if selected, recursion, errors and ABI lowering |
| ZL-006 | records/structs | Add | named fields, construction, access, layout-independent semantics and ABI representation |
| ZL-007 | enums/sum types | Add | exhaustive variants/payloads/matching and stable diagnostics |
| ZL-008 | option/result/error values | Add | explicit absence/failure, propagation and FFI/syscall conversion |
| ZL-009 | generics/templates | Later | bounded compile-time instantiation, diagnostics, code size and ABI policy |
| ZL-010 | traits/interfaces | Later | explicit contracts, dispatch strategy, coherence and object representation |
| ZL-011 | exact-width integers | Deepen | signed/unsigned widths and identical interpreter/native overflow modes |
| ZL-012 | floating point | Deepen | format/rounding/NaN/conversion and deterministic target semantics |
| ZL-013 | booleans/chars | Deepen | distinct types, Unicode scalar policy and ABI representation |
| ZL-014 | byte slices | Add | pointer+length ownership, checked indexing/arithmetic and no implicit C strings |
| ZL-015 | UTF-8 strings | Deepen | length-aware bytes/text boundary, validation, grapheme iteration and conversion |
| ZL-016 | arrays/vectors/maps | Deepen | bounds, allocation/error, iteration, ownership and deterministic hashing policy |
| ZL-017 | pointers/references | Deepen | explicit unsafe boundary, nullability, mutability, lifetime/ownership and casts |
| ZL-018 | packed/aligned layouts | Add | `sizeof`/`alignof`/`offsetof`, packed fields and C/firmware conformance |
| ZL-019 | volatile access | Add | exact-width device loads/stores separated from ordinary memory |
| ZL-020 | atomics/memory ordering | Add | typed operations/orderings and architecture litmus/conformance tests |
| ZL-021 | compiler/CPU barriers | Add | named semantics with architecture implementation and mock transcripts |
| ZL-022 | inline assembly | Add | explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation |
| ZL-023 | extern/FFI | Deepen | generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes |
| ZL-024 | named sections/linker symbols | Add | source attributes become checked object sections/flags/alignment/symbols |
| ZL-025 | object emission | Deepen | ELF relocatable sections/symbols/relocations and independent inspection |
| ZL-026 | executable emission | Keep | freestanding/native startup, linker contract, no hidden host symbols and loader proof |
| ZL-027 | relocation emission | Add | explicit supported set, checked addends/symbols/types and round-trip loader corpus |
| ZL-028 | debug metadata | Later | source files/lines/types/scopes/variables/build IDs and debugger compatibility |
| ZL-029 | unwind metadata | Later | optimized/unoptimized stack traversal and checked panic/debug consumers |
| ZL-030 | interpreter | Keep | reference semantics, sandbox/resource limits and differential tests |
| ZL-031 | native backend | Keep | semantics-equivalent lowering, optimization, ABI and code-generation receipts |
| ZL-032 | LLVM backend | Deepen | differential semantics and explicit dependency/toolchain provenance |
| ZL-033 | optimizer | Deepen | behavior-preserving passes, debugability, size/speed budgets and mutation tests |
| ZL-034 | compile-time evaluation | Later | deterministic declared inputs, termination/resources and reproducible output |
| ZL-035 | code generation/IDL | Add | schema validation and generated IPC/ABI/manifests with readable contracts |
| ZL-036 | memory allocation API | Deepen | explicit allocator/failure/alignment/lifetime suitable for freestanding code |
| ZL-037 | ownership/lifetime helpers | Add | deterministic cleanup without mandating one hidden runtime model |
| ZL-038 | concurrency primitives | Add | threads/tasks/channels/locks/atomics with cancellation and bounded queues |
| ZL-039 | async/deadline model | Later | explicit scheduler integration, cancellation, structured lifetime and backpressure |
| ZL-040 | standard library core | Deepen | bytes/text/collections/math/time/errors/files abstractions split by platform authority |
| ZL-041 | freestanding standard library | Deepen | no host assumptions, allocation choices, panics/errors and target tests |
| ZL-042 | Linux platform library | Keep | files/process/network/time mappings with exact error and ABI semantics |
| ZL-043 | zlOS platform library | Add | handles/IPC/process/files/window/input/audio/network/services over native ABI |
| ZL-044 | package manifest/resolver | Add | immutable versions/dependencies/features/targets/licenses and reproducible resolution |
| ZL-045 | formatter/linter | Add | stable style, machine fixes, diagnostics, configuration and strict exit status |
| ZL-046 | documentation generator | Add | symbols/types/examples/links/version/search and source provenance |
| ZL-047 | language server | Later | parse/index/completion/diagnostics/refactor with bounded incremental work |
| ZL-048 | test framework | Deepen | host/target/property/fuzz/benchmark lanes, discovery parity and strict results |
| ZL-049 | compatibility/versioning | Add | language editions/features/deprecations/migrations and artifact metadata |
| ZL-050 | bootstrap seed | Keep | exact trusted seed retained and identified through stage chain |
| ZL-051 | compiler fixpoint | Keep | source-stage rebuild and semantic/artifact provenance, not stored hash alone |
| ZL-052 | self-hosted system build | Later | matching compiler, kernel, services, apps and packages rebuilt inside zlOS |
| ZL-053 | multiarchitecture targets | Later | x86-64/i686/AArch64/RISC-V target manifests and execution evidence |
| ZL-054 | language security audit | Add | unsafe surface, parser/compiler attack inputs, supply chain and generated-code trust |
| ZL-055 | language performance lab | Deepen | compile time, memory, binary size and runtime benchmarks with exact baselines |

## 24. Future hardware and product expansion

| ID | Feature | State | Complete product meaning |
|---|---|---|---|
| FH-001 | laptop power/suspend | Later | ACPI sleep/wake, device quiesce/resume, clock/network/session and failure recovery |
| FH-002 | lid/buttons/hotkeys | Later | platform events, policy, debounce and accessible configuration |
| FH-003 | brightness/backlight | Later | provider capabilities, smooth policy, keys/settings and persistence |
| FH-004 | external monitor/dock | Later | hotplug topology, mode/scale/audio/input/network and safe detach |
| FH-005 | HiDPI/4K | Deepen | per-display scale, asset/text/layout quality and performance/memory budgets |
| FH-006 | variable/high refresh | Later | negotiated deadlines, pacing, input latency and power policy |
| FH-007 | touchscreen convertible | Later | orientation, touch/pen, keyboard transition, layout and gestures |
| FH-008 | camera/microphone hardware | Later | privacy-first providers, indicators, permissions, apps and diagnostics |
| FH-009 | wireless headset | Later | Bluetooth audio/control, codec/latency/battery, handoff and recovery |
| FH-010 | printer/scanner hardware | Later | discovery/provider/spool/scan lifecycle, drivers, UI and error proof |
| FH-011 | removable SD media | Later | hotplug/storage/mount/eject/recovery and device policy |
| FH-012 | biometric authentication | Later | protected template/device, fallback, consent, liveness claims and revocation |
| FH-013 | secure boot/measured boot | Later | key policy, measurement log, recovery and user-owned trust roots |
| FH-014 | smart-glasses display | Later | low-latency HUD composition, power, privacy, glanceable UI and remote companion |
| FH-015 | sensor framework | Later | accelerometer/gyro/GPS/ambient sensors, units/rate/permissions and fusion |
| FH-016 | cellular modem | Later | modem lifecycle, SIM/eSIM, data/SMS/calls, privacy and carrier state |
| FH-017 | heterogeneous CPU/GPU | Later | topology/capabilities/scheduling/memory/coherency and backend selection |
| FH-018 | hardware video codec | Later | negotiated formats/buffers/fences/reset and software fallback |
| FH-019 | FPGA/custom accelerator | Later | versioned capability/stream protocol, isolation, simulation and recovery |
| FH-020 | headless/server profile | Later | serial/network management, no-display services, resource/security and update policy |

## Exact current zlOS named-implementation crosswalk

This appendix preserves every current named implementation even when the canonical product catalogue merges it into a broader family. It contains **63 named implementations**, plus the separate **All Applications** catalogue surface. `Current` means source/current-route evidence recorded by the current generated manifest; it does not imply process isolation or complete internal workflows.

### Shell and core surfaces

| Current name | Canonical destinations | Current disposition |
|---|---|---|
| Terminal | DE-002, DE-003, DE-004 | Keep UI/workflow; migrate from kernel path to PTY/session process |
| System Monitor | ST-018 | Deepen through typed telemetry and control capabilities |
| About | ST-035, EV-028 | Keep; replace mutable/fixed facts with immutable provenance |
| Menu | SH-008 | Keep as shell-owned surface; generate from admitted registry |
| Browser | WB-001 through WB-048 | Keep bounded current work; split UI/content/network/decoder processes |
| Settings | ST-001 through ST-016 | Keep current controls; add schemas, service truth, preview and rollback |
| Run | SH-010 | Keep; make parser, resolution, authority and errors explicit |
| Files | PD-001, AP-021, FS-038 through FS-040 | Keep current zlfs workflow; move to VFS/file handles and portals |
| Text Editor | PD-005, AP-031, AP-032 | Keep; add document recovery, editing depth and process boundary |
| All Applications | SH-011, AP-002 | Separate catalogue surface; current Maze/blank-ID gate is false green |
| System | ST-001 through ST-035 | Keep the PRESSWORK system pane; replace direct kernel facts with typed providers |
| Type | AX-001 through AX-040 | Keep the PRESSWORK typography pane; connect controls to persistent accessible settings |

### Creative, graphics, and inspection

| Current name | Canonical destinations | Current disposition |
|---|---|---|
| Paint | MD-003 | Deepen tools/layers/history/formats/recovery |
| 3D | MD-017, GM-011 | Fixture and future reusable renderer consumer |
| zlOS animation | MD-016, GM-010 | Fixture for motion/frame/visual regression |
| Pointer | IN-024, IN-025 | Fixture for input devices, acceleration and latency |
| Renderer | GR-024, GR-025, GR-050 | Fixture for backend correctness/performance |
| Framebuffer | GR-043, GR-046 | Keep diagnostic; mediate capture/privacy |
| Font Atlas | MD-018, AX-031 | Keep diagnostic; deepen coverage/shaping/fallback |
| Image Viewer | MD-001, AD-013 | Deepen isolated decoder and product workflow |
| Colour Picker | PD-026, AX-013 | Deepen screen-pick authority, palette and contrast |

### Utilities

| Current name | Canonical destinations | Current disposition |
|---|---|---|
| Clipboard | SH-025, SH-026, AP-020 | Deepen into brokered MIME and privacy model |
| Regex Tester | PD-022 | Keep; add dialect and resource limits |
| Base Converter | PD-025 | Keep; add width/precision/error contract |
| Text Diff | PD-019 | Keep; add files, scalable modes and merge |
| Checksum | PD-021 | Keep; add streaming verification workflow |
| Unit Converter | PD-024 | Keep; add metadata, locale and precision |
| Sticky Notes | PD-008 | Keep; add per-user durable recovery/privacy |
| Keyboard Tester | IN-024 | Keep as exact input diagnostic fixture |
| Benchmark | ST-028, EV-014 | Keep; tie results to artifact/workload/environment |
| Calculator | PD-023 | Keep; deepen parser, precision, history and modes |
| Clocks & Timers | MD-025 through MD-027 | Keep; split wall/monotonic and background alarm service |

### System and diagnostic surfaces

| Current name | Canonical destinations | Current disposition |
|---|---|---|
| System Info | ST-017 | Keep; consolidate structured read-only provider facts |
| Kernel Log | ST-026, EV-018 | Keep; add redaction, persistence, filters and privileges |
| Hex Viewer | PD-020, DE-037 | Keep; add huge-file paging and guarded writes |
| Console (`tty1`) | DE-001 | Keep as low-level fallback; define consolidation versus Terminal |
| Disk Usage | PD-003, ST-023 | Keep; add snapshot/cancel/permissions/removable handling |
| Services | ST-020, AU-011 | Keep UI idea; connect to real supervised services |
| Archive Manager | PD-004, FS-030 through FS-033 | Keep; isolate parser and make extraction transactional |
| Network | ST-006, NW-030 | Keep diagnostic surface; connect to real network service |

### All 24 named games

| Current game | Canonical coverage | Current disposition |
|---|---|---|
| Snake | GM-001 through GM-008 | Keep fixture |
| Word Guess | GM-001, GM-004, GM-006 | Keep fixture |
| Tic-Tac-Toe | GM-001, GM-007 | Keep fixture |
| Nim | GM-001, GM-007 | Keep fixture |
| Tower of Hanoi | GM-001, GM-007 | Keep fixture |
| Lights Out | GM-001, GM-007 | Keep fixture |
| Connect Four | GM-001, GM-007 | Keep fixture |
| Maze | GM-001, GM-008, GM-017 | Source/rules exist; current catalogue launch route is contradicted |
| Tetris | GM-001, GM-008 | Keep fixture |
| Pong | GM-001, GM-008 | Keep fixture |
| Breakout | GM-001, GM-008 | Keep fixture |
| Minesweeper | GM-001, GM-007 | Keep fixture |
| 2048 | GM-001, GM-007 | Keep fixture |
| Conway's Life | GM-001, GM-009 | Keep fixture |
| Asteroids | GM-001, GM-008 | Keep fixture |
| Invaders | GM-001, GM-008 | Keep fixture |
| 15 Puzzle | GM-001, GM-007 | Keep fixture |
| Reversi | GM-001, GM-007 | Keep fixture |
| Simon | GM-001, GM-006 through GM-008 | Keep fixture |
| Sokoban | GM-001, GM-007 | Keep fixture |
| Flappy | GM-001, GM-008 | Keep fixture |
| Missile Command | GM-001, GM-008 | Keep fixture |
| Blackjack | GM-001, GM-007 | Keep fixture |
| Frogger | GM-001, GM-008 | Keep fixture |

## Observed external application breadth without double counting

The canonical rows above intentionally normalize equivalent jobs. The exact original names and boundaries remain in the source-grounded inventories:

- Chitti: all 48 manifest packages, including 30 UI packages and the staged local browser.
- NexiOS: all 61 shell registrations.
- SerenityOS: 51 first-party apps, 29 services, 13 games, and 223 utilities.
- Astral: all 307 package/port recipes.
- banan-os: all 128 ports plus native apps/services.
- RustOS, Zinnia, Skift, Fudge, TacOS/Cyjon, Brook, LemonOS, MollenOS, duckOS: every local native app/service and external/fetched boundary.
- Focused set: every meaningful app, service, game, media workflow, utility, web portal, DSP program, and developer fixture across all 16 repositories.

This prevents two bad outcomes: inflating the catalogue by counting `Calculator` in several repositories as different product capabilities, and losing the original repository-specific implementation evidence after normalization.

## Feature completion checklist

Every row inherits the following questions. A row cannot be marked complete while one is silently inapplicable; it must be answered or explicitly waived with reason.

1. What exact user or system outcome exists?
2. Which process/service/provider owns it?
3. What typed inputs, outputs, state, and versioned protocol does it expose?
4. Which authority is required, and can it be revoked live?
5. What are its byte/count/time/resource limits?
6. How does it start, become ready, stop, crash, recover, update, and roll back?
7. What happens on every partial failure, timeout, cancellation, peer death, device loss, and full queue?
8. What persists, where, under which user/profile, with what durability and migration?
9. How is it keyboard-operable, semantically exposed, scalable, localizable, and usable with reduced motion/high contrast?
10. What is cached or asynchronous, and what is the worst interaction/frame/operation budget?
11. What privacy, redaction, consent, provenance, and audit rules apply?
12. Which source/build/image/init/user/QEMU/hardware evidence proves each claim?
13. What planted mutation proves the verifier notices a missing route, false state, bad rollback, or stale artifact?
14. What is the weakest remaining link?

## Coverage boundary and source trail

This catalogue is the front-door product inventory, not a replacement for the exact evidence dossiers. The underlying implementation patterns, donors, rejected defects, current-state qualifications, and exact source paths live in:

- [`CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md`](CANONICAL_FEATURE_IMPLEMENTATION_CATALOG_2026-08-21.md)
- [`ALL_33_FEATURE_MATRIX_2026-08-21.md`](ALL_33_FEATURE_MATRIX_2026-08-21.md)
- [`DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md`](DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md)
- [`ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md`](ALL_33_DRIVERS_AND_APPS_MATRIX_2026-08-21.md)
- [`ALL_33_VISUAL_WEB_AND_APP_EXPERIENCE_MATRIX_2026-08-21.md`](ALL_33_VISUAL_WEB_AND_APP_EXPERIENCE_MATRIX_2026-08-21.md)
- [`MATURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`](MATURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md)
- [`ARCHITECTURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`](ARCHITECTURE_OS_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md)
- [`FOCUSED_REPOSITORIES_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md`](FOCUSED_REPOSITORIES_DRIVERS_AND_APPS_DEEP_DIVE_2026-08-21.md)
- [`MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](MATURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](ARCHITECTURE_OS_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md`](FOCUSED_REPOSITORIES_VISUAL_WEB_AND_APP_EXPERIENCE_DEEP_DIVE_2026-08-21.md)
- [`RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md`](RESPONSIVENESS_RENDERING_AND_VISUAL_POLISH_DEEP_DIVE_2026-08-22.md)

No donor source code is imported. Features observed only in screenshots, fetched dependencies, external userlands, historical trees, disabled targets, mocks, or stubs remain useful ideas or rejection tests, not current zlOS claims.
