# Dependency-resolved delivery phases

The destination is complete from the start. These phases order risk and
dependencies; they do not remove later features.

Each phase has one stable ID. Feature rows map to their primary delivery phase
in `FEATURE-MAP.md`; cross-cutting prerequisites remain explicit here and in the
specialized registries.

## Dependency spine

```text
MP-00 truth
  |
  +--> MP-01 baseline/physical receipts
  |       |
  |       +--> MP-02 boot/recovery
  |       +--> MP-07 display/input migration
  |       +--> MP-08 physical network
  |
  +--> MP-03 kernel execution
          |
          +--> MP-04 authority/IPC/supervision
                    |
                    +--> MP-05 driver framework
                    |       +--> MP-08 network/hardware breadth
                    |       +--> MP-09 audio/media devices
                    |       +--> MP-19 future hardware/architectures
                    |
                    +--> MP-06 storage/VFS/packages
                    +--> MP-07 display/input/compositor
                    +--> MP-10 design/a11y/localization
                            |
                            +--> MP-11 app platform/services
                                    |
                                    +--> MP-12 current apps/games
                                    +--> MP-13 browser/web
                                    +--> MP-14 productivity/media/PIM
                                    +--> MP-15 dev/compatibility/ports
                                    +--> MP-17 agents/automation

MP-16 zl toolchain consumes MP-03/04/06/15
MP-18 operations/release consumes all shipped services
MP-20 self-hosted rebuild consumes MP-02..19
```

## Machine-checked dependency manifest

This table is the canonical phase DAG consumed by the validator. A specialized
target may name additional providers inside its registry, but it cannot weaken
these prerequisites.

| Phase | Direct dependencies |
|---|---|
| MP-00 | none |
| MP-01 | MP-00 |
| MP-02 | MP-00, MP-01 |
| MP-03 | MP-00, MP-01 |
| MP-04 | MP-00, MP-03 |
| MP-05 | MP-00, MP-03, MP-04 |
| MP-06 | MP-00, MP-03, MP-04, MP-05 |
| MP-07 | MP-00, MP-01, MP-03, MP-04, MP-05 |
| MP-08 | MP-00, MP-01, MP-04, MP-05, MP-06 |
| MP-09 | MP-00, MP-04, MP-05, MP-07 |
| MP-10 | MP-00, MP-04, MP-07, MP-09 |
| MP-11 | MP-00, MP-03, MP-04, MP-06, MP-07, MP-10 |
| MP-12 | MP-00, MP-11 |
| MP-13 | MP-00, MP-08, MP-10, MP-11 |
| MP-14 | MP-00, MP-08, MP-09, MP-10, MP-11, MP-13 |
| MP-15 | MP-00, MP-03, MP-04, MP-06, MP-08, MP-11 |
| MP-16 | MP-00, MP-03, MP-04, MP-06, MP-15 |
| MP-17 | MP-00, MP-04, MP-06, MP-11, MP-15 |
| MP-18 | MP-00, MP-02, MP-04, MP-06, MP-08, MP-11, MP-17 |
| MP-19 | MP-00, MP-02, MP-03, MP-04, MP-05, MP-08, MP-09 |
| MP-20 | MP-00, MP-01, MP-02, MP-03, MP-04, MP-05, MP-06, MP-07, MP-08, MP-09, MP-10, MP-11, MP-12, MP-13, MP-14, MP-15, MP-16, MP-17, MP-18, MP-19 |

## MP-00 — canonical truth and enforceable evidence

Dependencies: none.

Deliver:

- generated feature, source, build, image, init, route, test and artifact
  registries;
- exact dependency/toolchain/license locks and artifact self-identity;
- strict exit propagation, test inventory parity and planted verifier canaries;
- benchmark, hardware, visual, accessibility, security, failure-injection and
  hostile-corpus receipts;
- this complete program validator and independent rejection path.

Exit: all 906 IDs map exactly once; every registry target has an owner, primary
phase and proof class; a planted missing feature, route, test and artifact makes
the gate fail nonzero.

## MP-01 — preserve current behavior and close physical baseline gates

Dependencies: MP-00.

Deliver:

- fresh clean-build and boot-media identity for current BIOS and UEFI routes;
- ThinkPad frame/input journal, retained-surface comparison and no-drop receipt;
- physical I219/dongle, xHCI/storage, Intel modeset/render and recovery evidence
  where hardware is present;
- exact current app registry repair, including Maze/blank-ID false green;
- frozen fallback oracles for software rendering, PS/2, GOP/BGA, zlfs, current
  browser and low-level console.

Exit: no known current feature is silently lost; each open physical claim stays
open or has a native-hardware receipt. This phase does not block safe host-only
foundational work, but it blocks physical promotion claims.

## MP-02 — owned boot manager, firmware handover and recovery generations

Dependencies: MP-00; consumes MP-01 boot oracles.

Deliver:

- BIOS/raw, GRUB/Multiboot and native UEFI acquisition providers;
- bounded zlOS boot manager with exact current/previous/recovery/debug/assertion
  selection, origin identity and typed immutable handover;
- checked ELF/module/archive/config parsers, firmware inventory, entropy and
  CPU/platform records;
- boot assertion matrix across raw/ISO/GPT/MBR/filesystem/fragmented/hostile
  media and BIOS/UEFI combinations;
- signed read-only-first recovery, atomic previous-generation rollback, early
  serial/framebuffer diagnostics and bounded firmware waits.

Exit: stale, mismatched, malformed and partial boot artifacts fail closed;
current/previous/recovery generations are identifiable and bootable; no loader
policy turns into a second kernel.

## MP-03 — preemptive process, memory and execution foundation

Dependencies: MP-00; preserves MP-01 cooperative Ring-3 oracle.

Deliver:

- typed physical/virtual memory ownership, guarded kernel/user stacks, W^X,
  memory accounting/pressure and failure-atomic page mapping;
- robust ELF process admission, per-process address spaces, exec, threads,
  shared objects, anonymous/file-backed memory and safe full-span usercopy;
- timer preemption, per-CPU scheduler, deadlines, synchronization, signals,
  wait/process tree and fault-only-the-offender behavior;
- generated syscall table with unsigned index checks and ABI/version receipts.

Exit: hostile unprivileged programs cannot access kernel/device memory or crash
siblings/system; timer-driven scheduling and teardown survive stress, OOM and
fault injection; cooperative mode remains a diagnostic fallback until promoted.

## MP-04 — capability authority, typed IPC, supervision and identity

Dependencies: MP-03, MP-00.

Deliver:

- opaque generation-checked handles, explicit rights, derivation graph and
  exhaustive revoke with preflight/rollback;
- bounded typed endpoints and zlIDL schemas with transactional handle transfer,
  backpressure, cancellation, deadlines and peer-death cleanup;
- default-deny operation-right matrix for device, DMA, MMIO, mount, process,
  power, display, input and administration actions;
- service supervisor, dependency/readiness graph, crash budgets, quarantine,
  reverse shutdown and authenticated management;
- users, credentials, sessions, secure password storage, secret service,
  policy/audit and portal-mediated grants.

Exit: source and destination rights are tested independently; every nth-handle
transfer/revoke failure leaves byte-for-byte prestate; fork/credential/drop and
privileged-operation hostile matrices pass.

## MP-05 — common driver framework, DMA isolation and device lifecycle

Dependencies: MP-03, MP-04, MP-00.

Deliver:

- versioned driver/provider descriptors, match/admit/reserve/start/online/
  quiesce/recover/abort/offline lifecycle and resource receipts;
- PCI/ACPI/FDT/USB/platform discovery, hotplug, binding, quarantine and rebind;
- BAR/IRQ/MSI/MSI-X, DMA buffer, IOMMU-domain and honest no-IOMMU policies;
- common block, network, display, input, audio, camera, sensor, power and VM
  integration contracts;
- simulator/host protocol tests, QEMU target tests and device-specific physical
  promotion.

Exit: failure at every lifecycle step releases every BAR, IRQ, DMA page, handle
and published endpoint; device removal/recovery creates no stale generation;
passthrough/bounce paths are never mislabeled isolated.

## MP-06 — storage, VFS, packages, updates and recovery

Dependencies: MP-03, MP-04, MP-05; preserves current zlfs/block oracles.

Deliver:

- asynchronous block contract, cache/writeback, flush/barrier/discard and
  removable-media semantics across all selected providers;
- VFS names, handles, directories, links, permissions, ownership, mounts,
  namespaces, quotas and file-backed mapping;
- zlfs depth plus selected FAT/ext/ISO/tmp/dev/proc/sys/archive/network providers;
- corruption admission, fsck/repair, snapshots/backups, encrypted volumes and
  power-cut/disk-full/concurrency matrices;
- signed reproducible packages, repositories, transactional install/update/
  remove, rollback, provenance, firmware/license policy and file portals.

Exit: every power cut yields old or new durable state, never invented state;
uninstall/update atomically revokes live processes/services/handles; malformed
media cannot corrupt the kernel or unrelated volumes.

## MP-07 — display, input, compositor and session migration

Dependencies: MP-03, MP-04, MP-05; consumes MP-01 performance oracles.

Deliver:

- display provider ABI for GOP/simplefb, BGA, virtio-gpu, VMware and Intel paths;
- compositor-owned scanout, bounded opaque surfaces, damage/occlusion, atomic
  commits, fences, pacing, color/scale and software fallback;
- input provider ABI for PS/2, USB HID, I2C-HID, virtio/tablet and future touch/
  stylus/gamepad, with focus/grab/remap/IME/latency policy;
- user-mode Window, Input, Session and shell services with secure attention,
  lock/login/logout, multi-display, workspaces and crash recovery.

Exit: user processes cannot own scanout or global input; move/resize/occlusion/
drag/typing workloads meet budgets without allocations in commit; provider or
compositor crashes recover to a usable console/session.

## MP-08 — network services and physical provider breadth

Dependencies: MP-04, MP-05; selected storage from MP-06.

Deliver:

- common NIC provider and virtio/e1000 preservation;
- Intel I219/e1000e/igb/igc, RTL8139/816x, NE2000, USB ECM/RNDIS and selected
  platform NICs;
- loopback, Ethernet, ARP/NDP, IPv4/IPv6, DHCP/SLAAC, ICMP, UDP, TCP reliability,
  DNS, TLS, sockets, routing/firewall, Wi-Fi/Bluetooth service boundaries;
- network settings/diagnostics, offline/captive/roaming states, remote desktop,
  SSH and bounded download/update seams.

Exit: host fault suites and QEMU providers pass; target physical providers prove
carrier/reset/reuse/teardown and real traffic; malformed packets, timeouts,
zero-window and device loss stay bounded.

## MP-09 — audio, camera, media devices and real-time streams

Dependencies: MP-04, MP-05, MP-07; storage/network as consumed.

Deliver:

- HDA, AC97, virtio-snd, SB16/PC speaker and USB Audio providers as applicable;
- AudioServer ownership, bounded shared rings, mixing, device selection, volume,
  latency, underrun, suspend and privacy indicators;
- UVC camera and capture portal; media clock/synchronization; software codec
  workers and later hardware codec negotiation;
- microphone/camera permission, revocation and peer-death cleanup.

Exit: untrusted apps cannot map device DMA or spy on capture; stream stress,
underrun, hotplug, crash and suspend/reconnect tests pass with explicit latency.

## MP-10 — design system, accessibility and localization

Dependencies: MP-04, MP-07; audio providers consumed for speech/captions.

Deliver:

- versioned semantic tokens, atomic themes, typography/shaping/fallback, icons,
  motion/reduced-motion and a complete component/state library;
- semantic accessibility tree, screen reader, speech/braille provider seams,
  magnifier, filters, captions, switch/dwell and keyboard accessibility;
- scale/reflow, bidirectional text, IME, locale/date/number/unit/timezone and
  translation catalogue tooling;
- visual, interaction, focus, contrast, semantic and localization goldens.

Exit: every primitive and shell path is keyboard and semantic-provider usable;
all scale/theme/locale/a11y states remain bounded and coherent; reduced motion
and high contrast are functional policy, not alternate screenshots.

## MP-11 — application runtime, SDK, packages and shared services

Dependencies: MP-03, MP-04, MP-06, MP-07, MP-10.

Deliver:

- stable process ABI, AppKit/UI SDK, manifest, registry, launcher and lifecycle;
- files/settings/clipboard/notifications/print/share/open/save/secret/location/
  camera/microphone portals;
- document model, undo/redo, autosave/recovery, MIME/thumbnail/search/indexing,
  spellcheck and common media/decoder worker services;
- crash-loop containment, resource budgets, background policy, app restore and
  transactional package lifecycle.

Exit: hello-world through full app workflows run outside the kernel with no raw
kernel/user pointers; install/launch/crash/restart/update/uninstall and portal
revocation proofs pass.

## MP-12 — migrate every current app, utility and game

Dependencies: MP-11 and the providers each app consumes.

Deliver:

- all 61 current named implementations plus All Applications as explicit
  registry entries/routes;
- all 24 named games as deterministic input/audio/render/persistence/runtime
  probes;
- kernel-resident implementations retired only after process versions preserve
  workflows, accessibility, persistence and fallback behavior;
- repair of the Maze/IDs/route false-green class with per-field mutation tests.

Exit: every current name is mapped, launchable, closable and absent only through
an explicit retirement/migration record; shipped image and registry agree.

## MP-13 — split browser and complete web platform

Dependencies: MP-08, MP-10, MP-11.

Deliver:

- browser UI, network, content renderer, JavaScript and decoder process split;
- navigation/history/bookmarks/downloads/tabs/session restore/devtools/privacy;
- URL/HTTP cache/cookies/storage, HTML/CSS/layout/paint/compositing, forms,
  accessibility, images/fonts and bounded JavaScript/DOM/events;
- same-origin/CSP/cert/site-permission/download sandboxing and hostile web corpus;
- incremental layout/paint, cancellation, scheduling and memory/process budgets.

Exit: current supported web corpus remains green; unsupported features are
honest; malicious content/parser/network failures kill only restricted workers;
interactive page budgets and browser accessibility gates pass.

## MP-14 — productivity, knowledge, media and personal applications

Dependencies: MP-11 plus relevant MP-08/09/13 services.

Deliver every app family in `APPLICATIONS.md`, including office document,
spreadsheet and presentation creation; PDF/document/image/media workflows;
mail/chat/calendar/contacts/tasks/notes; paint/vector/3D/animation/audio/video;
print/scan; maps/weather/RSS and accessibility/control-center experiences.

Exit: each product workflow has real data import/create/edit/save/reopen/export,
error/recovery, accessibility, localization, performance and package evidence;
painted/mock UI cannot satisfy the route.

## MP-15 — developer environment, compatibility, ports and virtualization

Dependencies: MP-03, MP-04, MP-06, MP-08, MP-11.

Deliver:

- terminal/PTY/shell/job control, core utilities, editor/IDE, debugger, profiler,
  build/test/package tools and documentation;
- POSIX/Linux compatibility driven by selected programs, ELF dynamic linking,
  shared libraries, foreign ABI containment and reproducible port recipes;
- Windows compatibility driven by selected programs: quarantined PE/DLL
  loading, NT/Win32/registry/window/graphics/audio/network/COM contracts and
  selected Direct3D translation outside ring 0;
- Windows/Linux VM guests, selected CPU/device emulation, containers/sandbox
  profiles, virtio guest integration, 9P/shared folders, clipboard and optional
  seamless windows with explicit authority;
- port availability/build/runtime/security/upgrade evidence, not recipe counts.

Exit: selected native, unmodified Windows and unmodified Linux reference
programs build or run through their declared compatibility contracts; one
Windows and one Linux guest complete boot/snapshot/reconnect/shutdown workflows;
host/target compiler and utility behavior agrees on golden corpora; translated,
emulated and guest integration paths cannot escape granted resources.

## MP-16 — zl systems language, compiler and self-hosting toolchain

Dependencies: MP-03, MP-04, MP-06, MP-15.

Deliver:

- exact integers/bytes, checked arithmetic, imports/modules/errors, layouts,
  packed/aligned/volatile data, pointers, MMIO/ports/barriers and ABI declarations;
- functions/closures/interfaces/generics as selected, reflection only where
  bounded, and strong diagnostics/tooling;
- ELF64 executable/relocatable/shared output, named sections, relocations,
  linker/archive/debug metadata and deterministic builds;
- zlIDL generation, first zl process/service/provider, compiler fixpoint and
  preservation of existing self-hosting behavior.

Exit: system ABI conformance and hostile compiler corpora pass; a real isolated
zl provider and application run on zlOS; fixpoint hashes/semantics remain
accounted for.

## MP-17 — capability-native agents and automation

Dependencies: MP-04, MP-06, MP-11, MP-15; network only where explicitly granted.

Deliver:

- typed plans, deterministic tool broker, attenuated subagents, budgets,
  cancellation and parent-death cleanup;
- signed/versioned/revocable skills and plugins with transactional install;
- provenance/taint/privacy, approval policy, secrets mediation, durable audit and
  independent completion review;
- desktop automation through semantic/accessibility APIs, scheduled jobs,
  workflows and recovery without ring-0 model/orchestrator authority.

Exit: revoke-then-spawn, parent kill, hostile tool, partial install, secret leak,
budget exhaustion and false-completion tests pass; every effect names authority
and receipt.

## MP-18 — public demo, operations, releases and support

Dependencies: MP-00, MP-02, MP-04, MP-06, MP-08, MP-11 and every promoted
surface being released.

Deliver:

- reproducible release images, signed updates/channels, compatibility/migration
  notes, rollback and long-term artifact retention;
- transactional leased QEMU demo with authenticated VNC/WebSocket data plane,
  PID identity, launch rollback, quotas, egress policy and zero-orphan reaping;
- telemetry/crash/support bundles with consent/redaction; provenance and feature
  viewers; rescue/repair and disaster recovery drills;
- CI matrices, canaries, fuzzing, performance/security/a11y/visual promotion and
  support lifecycle.

Exit: clean-room release rehearsal and rollback succeed; demo lease failure at
every allocation/launch/proxy/reap step restores exact prestate; no source-only
claim is promoted.

## MP-19 — broader hardware, product profiles and later architectures

Dependencies: MP-02, MP-03, MP-04, MP-05 and relevant services.

Deliver:

- AArch64 and RISC-V64 with real MMU/interrupt/timer/SMP/process/service gates;
- laptop/desktop/tablet/headless profiles, suspend/resume, battery/thermal,
  Wi-Fi/Bluetooth, sensors, camera, cellular and secure hardware;
- GPU/video acceleration, heterogeneous compute and selected FPGA/accelerator
  contracts with software/simulation fallback;
- hardware qualification matrix with exact topology/firmware receipts.

Exit: each architecture/device claim has its own build, boot, workload, teardown
and required physical evidence; x86-64 results never propagate automatically.

## MP-20 — in-zlOS rebuild and complete-system promotion

Dependencies: MP-02 through MP-19 for the selected release profile.

Deliver:

- compiler/build/package tools as isolated processes;
- reproducible source/package graph and cache inside zlOS;
- compiler fixpoint inside zlOS, then bootloader/kernel/services/apps/packages
  rebuilt into an identified bootable generation;
- a selected first-party profile whose source/build graph contains zl plus
  required assembly or emitted machine code, with no generated-C stage, C
  runtime, C kernel/service/driver source, GCC or Clang dependency;
- full assertion, hostile, power-cut, performance, accessibility, visual,
  security, update/rollback, physical and independent-refutation matrices.

Exit: the matching release is rebuilt inside zlOS and boots as the exact
identified generation; all selected feature atoms and registries are complete
or explicitly deferred to a later product profile with no false claims.
