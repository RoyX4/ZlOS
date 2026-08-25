# Driver and application clean-room integration plan for zlOS

Date: 2026-08-21

Status: end-state architecture and execution contracts. This is a destination
plan, not a claim that the process/service/driver boundaries already exist.

Inputs:

- [`ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md`](ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md)
- [`DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md`](DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md)
- [`CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md`](CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md)
- [`IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md`](IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md)
- the three all-repository driver/application deep dives in this directory.

## Outcome

The destination is not “more files called drivers” or “hundreds of icons.” It
is a zlOS where:

- each physical or virtual device is represented by an owned, restartable,
  observable provider with bounded failure behavior;
- every application is an ordinary isolated zl process with explicit file,
  network, device, clipboard, window and session handles;
- current zlOS drivers, desktop behaviors, persistent data and hardware
  evidence are migrated rather than thrown away;
- every new external idea enters as a clean-room behavior/state/test contract;
- a feature cannot advance from source to supported without build, launch,
  scenario, recovery and relevant hardware receipts.

## Non-negotiable requirements

### Product requirements

1. Keep UEFI GOP and the software renderer usable after every graphics change.
2. Keep ZLLOG usable before processes, filesystems and ordinary services.
3. Preserve current `zlfs` data and existing app behavior through migration.
4. Do not block current performance priorities—retained surfaces, asynchronous
   storage, 64-bit processes and physical wired networking—with breadth work.
5. Keep the Intel ThinkPad display work as the principal physical north star;
   its write/modeset proof is host-harness-only until native zlOS boot arms it.
6. Make the existing app catalogue launchable and truthful before expanding its
   number.

### Reliability requirements

- no infinite hardware or IPC wait;
- no source-file/registry/CI-name completion claims;
- no stale artifact can pass a boot or app scenario;
- no driver publishes `Online` before all required resources and dependencies;
- no failed operation leaves half-published handles, mappings, queues, files or
  package registrations;
- no user fault, bad app buffer or malformed media file halts the desktop;
- no device can DMA outside explicitly mapped pages once IOMMU isolation is
  claimed;
- when no IOMMU exists, the ledger explicitly says `not isolated` and permits
  only a reviewed in-kernel driver or bounded bounce/pinned-buffer policy; it
  never labels passthrough or unrestricted DMA as isolation;
- every provider has an idempotent stop/reset/remove path before suspend or
  hotplug is claimed.

## End-state architecture

```text
firmware + hardware + QEMU models
              |
       kernel mechanisms
  IRQ | VM | DMA | handles | scheduler | usercopy | clocks
              |
       Device Manager + Driver Supervisor
 discovery | matching | resource claims | lifecycle | health
              |
  +-----------+-----------+-----------+-----------+
  |           |           |           |           |
Display     Input       Block        NIC        Audio      Clock/Power
provider    provider    provider      provider   provider   provider
  |           |           |           |           |           |
Window      Session     File/VFS     Socket/     Mixer/     Settings/
service     service     service      Resolver    Media      Power UI
  +-----------+-----------+-----------+-----------+-----------+
                              |
                       ordinary zl processes
 Files | Editor | Browser | Terminal | Tools | Media | Games | Agents
```

The kernel enforces memory, handles, IRQ/DMA ownership, scheduling and usercopy.
It does not parse websites, images, archives, package scripts, HID descriptor
policy, filesystem namespaces, window policy or game logic in the destination.

## Common driver descriptor

Every driver package/module declares a deterministic descriptor equivalent to:

```text
DriverDescriptor {
  schema_version, id, version, source_digest, binary_digest, target_arches,
  package_manifest_version,
  device_match_rules,
  supported_hardware_revisions, dma_width, coherency_rules,
  required_kernel_abi_range, required_service_abi_ranges,
  required_capabilities, optional_capabilities,
  required_resources { bar, port, irq, dma, firmware, clock, power },
  provided_interfaces,
  queue_and_memory_limits,
  startup_deadline, request_deadlines, reset_deadline,
  supported_stop_reset_remove_suspend_resume,
  simulator_tests, qemu_profiles, hardware_profiles,
  license_and_firmware_provenance,
  signing_key_id, trust_chain, revocation_policy, anti_rollback_version
}
```

The descriptor is not proof. It makes claims enumerable and lets the system
refuse a driver before partial mutation when resources, ABI, firmware,
architecture, IOMMU policy or memory budgets cannot be satisfied.
Unknown mandatory capabilities are rejected; unknown optional capabilities are
ignored and recorded. Content digests identify bytes but do not authenticate a
publisher, trust root, policy version or non-revoked key.

## Driver lifecycle protocol

### State machine

```text
Absent
  -> Matched
  -> Admitted
  -> ResourcesReserved
  -> Starting
  -> Online
  -> Quiescing
  -> Offline
  -> Removed

Starting|Online|Quiescing|Recovering -> Failed
Failed -> Recovering -> Online|Offline|Quarantined
Matched|Admitted|ResourcesReserved|Starting -> Aborting -> Matched|Absent
Offline|Quarantined -> Rebinding|Removed
```

### Required transitions

- `match(device_descriptor) -> score/reason` is pure and cannot touch hardware;
- `admit(limits, dependencies, policy) -> plan` reserves budget before effects;
- `abort(plan)` releases every reserved dependency, BAR, port, IRQ, DMA mapping,
  firmware object, allocation and handle if admission/start does not commit;
- `start(plan) -> provider_handle` either publishes the complete provider or
  publishes nothing;
- `submit(request, deadline, cancel_handle) -> completion` never loses request
  ownership;
- `quiesce(deadline)` stops admission and completes/cancels existing requests;
- `reset(reason)` is bounded and invalidates stale queue generations;
- `stop()` is repeat-safe;
- `remove()` masks/quiesces IRQs and stops DMA before freeing mappings/pages;
- `health()` exposes state, last error, resets, timeouts, queue occupancy,
  dropped/completed work and current resource ownership.
- dependency loss invalidates the provider generation, completes each outstanding
  request exactly once and converges to degraded, offline or quarantined state;
- rebind occurs under a new generation and cannot accept late IRQs/completions
  from the old provider.

Where an IOMMU exists, each device starts in a default-deny DMA domain and maps
only pages authorized by a scoped DMA handle. Detach/revoke removes translations
before pages can be reused. A passthrough domain can support trusted bring-up but
can never satisfy the DMA-isolation gate.

### Driver failure receipt

Every failure records:

```text
boot/artifact/device identity
driver/version/state/transition
request type + deadline + queue generation
owned BAR/IRQ/DMA/firmware handles
last hardware status and decoded error
cleanup attempted/completed
fallback selected
system/app impact
```

## Common IPC and shared-object contract

Process separation is meaningful only through one generated, bounded transport:

```text
protocol/schema version
opaque endpoint, object and correlation IDs
authenticated sender/process/session identity
maximum envelope and per-endpoint byte/message quotas
required endpoint capability and transferred-handle rights
capacity-aware receive and explicit backpressure
deadline, cancellation and exactly-once terminal result
peer-close/provider-death behavior
generation and revocation state
```

No message contains a kernel/user pointer as authority. Sending bytes and
transferring handles is one transaction: validate source rights, pre-reserve the
destination, attach everything, then commit or restore both sides exactly.
Receiver-too-small retains the message and reports required capacity. Shared
objects require owner or live `SHARE/DERIVE` authority, receiver consent, mapping
rights, reference counts, generation invalidation and teardown on peer death.

Proof covers empty/full/last-slot queues, byte and message exhaustion, nth-handle
failure, receiver-too-small, duplicate/forged/stale IDs, peer close, revoke during
transfer, service restart and concurrent teardown. Unsupported messages return a
typed error; they are never silently discarded.

## Driver-family contracts

### Display

Provider operations:

- enumerate connectors/modes/planes/cursors;
- create bounded surface;
- validate atomic scene/plane update;
- submit present with fence/deadline;
- modeset, enable, blank, restore and capture diagnostic state;
- report vblank/present completion, underrun and hotplug;
- reset and fall back.

Surfaces and windows are opaque handles. The compositor alone owns scanout and
global input routing. The creator owns a surface; mappings, present/refresh,
sharing and revocation are separate rights. Dimensions, stride, format, byte
size, clipping and damage are admitted with checked arithmetic. Mappings have
reference counts and generations; peer death or revoke makes every stale mapping
and present request fail without touching reused memory.

Required proof:

1. exact framebuffer mode/cache/stride/format;
2. repeated modeset and second modeset;
3. software and accelerated scene hashes agree;
4. invalid scene cannot reach hardware;
5. ring/fence timeout triggers bounded recovery;
6. unplug/hotplug where supported;
7. failed acceleration returns to software/GOP without losing desktop;
8. host-harness and native-boot Intel receipts are separate; native completion
   names device, panel, firmware/mode, artifact nonce and observed display effect.

Current placement:

- GOP/software: retain permanently;
- BGA/virtio-gpu: QEMU providers;
- Intel display: principal hardware-provider target; current physical modeset is
  host-harness evidence, not a native-boot provider receipt;
- blitter/ring/cursor: staged capabilities, not unconditional defaults;
- future GPU/3D: a user-facing render service above provider primitives.

### Input

Provider output is a versioned stream:

```text
device_id, generation, monotonic_time, sequence,
kind, usage/code, value, modifiers, coordinates, contact,
loss_count
```

Contracts:

- device identity/generation prevents stale events after reconnect;
- report descriptors are length-admitted before allocation or interpretation;
- keyboard layout/text composition is session policy, not controller code;
- pointer acceleration/gestures are policy with raw-event access for tests;
- every queue has capacity, coalescing/drop policy and exact loss counter;
- disconnect synthesizes safe release/cancel state;
- accessibility/remapping operates on the normalized stream.

Migration order: PS/2 and current xHCI boot HID adapters -> generic HID parser ->
decode the existing I2C-HID raw diagnostic transport into normalized events ->
hubs/multiple devices -> touch/gamepad/other classes.

### Block/storage

Provider operations:

```text
identify, capacity, block_size, limits,
readv, writev, flush, discard(optional), cancel, health, reset
```

Contracts:

- 64-bit checked LBA/span arithmetic;
- explicit DMA buffer ownership and direction;
- request generation survives late/stale completion;
- write acknowledgement level is explicit: accepted, device-complete, durable;
- flush/barrier ordering is testable;
- device removal/cancel returns every buffer exactly once;
- reset cannot replay writes silently;
- filesystem never calls an NVMe/xHCI-specific function.

Migration: wrap NVMe and xHCI MSC without changing `zlfs` data -> introduce
async queues/cancel -> power-cut matrix -> add AHCI/virtio only through the same
contract.

### USB

Split into:

1. controller provider;
2. topology/address/configuration service;
3. endpoint/transfer handles;
4. independent class drivers.

The controller owns rings/slots/interrupters. The USB service owns descriptor
admission, configuration choice and class binding. Class drivers cannot access
unrelated controller memory or devices.

Proof includes malformed descriptor corpus, hub depth/port churn, short/error
transfers, reset during control/bulk/interrupt I/O, endpoint halt recovery,
unplug with work in flight, and repeated device/controller reuse.

### Network

NIC provider operations:

```text
identity, mac, mtu, feature bits, carrier,
rx-buffer supply, tx packet, cancel, counters, reset
```

Protocol/socket policy is separate. One NIC cannot become the network ABI.

Migration: wrap virtio-net -> deterministic packet simulator -> physical I219
provider -> DHCP/DNS/TCP/TLS service split -> later Wi-Fi. Each receipt keeps
driver/link/packet/socket/DNS/TLS/browser milestones separate.

### Audio

Introduce the AudioServer contract before selecting breadth:

- negotiated sample format/rate/channels;
- bounded per-client playback/record rings;
- one monotonic presentation clock;
- server mixer with volume/mute and clipping policy;
- underrun/overrun/xrun counters;
- device hotplug/reset and route changes;
- explicit microphone permission and visible recording state.

Start with deterministic software mixing and a fake provider. Add HDA or
`virtio-snd` only after the provider contract and generated tone/hash tests.

### Power and platform devices

Start read-only: battery, AC, temperature, fan, brightness, lid and power
button. Units, unknown/unavailable state and sensor provenance are explicit.
Power policy remains in a service. Suspend is not supported until every active
provider demonstrates quiesce, firmware transition, resume/rebind and failure
fallback.

## Application package/runtime model

### Application manifest

```text
ApplicationManifest {
  schema_version, stable_id, name, version, publisher, content_digest,
  signing_key_id, trust_chain, revocation_policy, anti_rollback_version,
  target_arches, kernel_abi_range, executable_abi, entrypoint,
  dependency_versions_and_conflicts,
  required_service_capabilities, optional_service_capabilities,
  declared_file_types_and_protocols,
  requested_handles_and_limits,
  windows/background_tasks,
  complete_file_manifest_and_size_limits,
  persistent_data_schema,
  accessibility_metadata,
  migrations, rollback, uninstall_data_policy,
  deterministic_tests, qemu_scenarios,
  license_and_firmware_provenance
}
```

An app does not receive ambient root, device IDs, arbitrary PIDs, raw scanout,
global clipboard, unrestricted network or the ability to load kernel code.
Unknown mandatory capabilities, wrong ABI/architecture, revoked keys, dependency
conflicts and excessive limits fail before any package bytes are published.

### Application lifecycle

```text
NotInstalled -> Downloaded -> Verifying -> Admitted -> Staged -> Installed
Installed -> Starting -> Ready -> Background|Suspended -> Stopping -> Installed
Starting|Ready|Background -> Crashed -> Reported -> Restarting|Installed
Installed -> Updating -> Installed|RolledBack
Installed -> Uninstalling -> NotInstalled
Verifying|Admitted|Staged|Updating|Uninstalling -> RollingBack
```

Required behavior:

- launch publishes a window/service only after process and required handles are
  ready;
- focus/window close and process lifetime are distinct;
- background tasks are declared, bounded and visible;
- crash report contains app/artifact/process identity without secret content;
- restart does not duplicate effects;
- unsaved state behavior is explicit;
- update migrates data transactionally and can roll back;
- uninstall revokes authority and registrations before deleting package bytes;
- user data is retained or deleted only under an explicit policy/confirmation.
- archive traversal, symlink escape, decompression bombs, duplicate paths,
  unbounded scripts and unexpected file types fail verification;
- installation maintains a journal that can recover after reboot at every step;
- stop has graceful and forced deadlines; process identity and death are proved
  before handles, registrations or slots are reused;
- crash budgets, exponential backoff and quarantine prevent restart storms;
- uninstall proves that processes, callbacks, tools, roles, services, grants and
  live handles are stopped, deregistered or revoked before package deletion.

## Service package and lifecycle model

Every long-lived service has a descriptor separate from its executable:

```text
ServiceDescriptor {
  stable_role, protocol_versions, authority_profile, exposure_profile,
  dependencies, startup_order, readiness_probe,
  authenticated_endpoints, connection_and_resource_budgets,
  request_deadlines, shutdown_deadline,
  health_and_metrics, crash_budget, backoff, quarantine,
  checkpoint_restore, dependency_loss_and_reconnect,
  management_authority, audit_and_redaction_policy
}
```

Lifecycle is `Installed -> Starting -> Ready -> Degraded -> Quiescing -> Stopped`,
with `Failed -> Backoff -> Restarting|Quarantined` and dependency-loss/reconnect
branches. Cycles, missing dependencies and impossible budgets fail before effects.
Readiness is an explicit protocol result, not process existence. Shutdown runs in
reverse dependency order with a deadline. Provider/service epochs reject stale
clients and late replies after restart. Authenticated management, checkpoint and
restore operations require separate capabilities.

## Existing zlOS app migration groups

### Group A — system truth clients

Migrate first because they force good service APIs:

- System Monitor;
- About;
- Clocks & Timers;
- Kernel Log;
- System Info;
- Services;
- Disk Usage;
- Network;
- Framebuffer/Renderer/Font Atlas;
- Keyboard Tester and Benchmark.

These should become read-only clients of telemetry, supervisor, file, network,
display and input providers. They must not scrape arbitrary kernel globals.

### Group B — file/state clients

- Files;
- Text Editor;
- Sticky Notes;
- Hex Viewer;
- Archive Manager;
- Image Viewer;
- Text Diff;
- Checksum.

Migrate behind File/VFS handles and restricted decoder workers. Preserve
current `zlfs` data and save behavior through differential tests.

### Group C — network/content clients

- Browser;
- future download manager, remote shell, messaging and package clients.

Split network fetch, TLS policy, hostile format parsing and page/application UI
into least-authority processes. A parser crash produces a typed failed object,
not desktop failure.

### Group D — desktop/session policy

- Terminal;
- Console (`tty1`), consolidated into the Terminal/PTY service while its stable
  ID is preserved as an alias or explicitly retired by registry migration;
- Run;
- Settings;
- Menu/catalogue/dock;
- Clipboard;
- notifications and future login/lock/accessibility.

These establish session capability roots, launch policy, clipboard privacy,
app discovery and accessibility semantics.

### Group E — deterministic tools and games

- calculator/converters/regex/diff/checksum and Colour Picker;
- Paint, 3D, animation and pointer demos;
- the current 24 named games.

Use them as low-risk process/runtime probes: separate address spaces, event
routing, timers, rendering, random-seed replay, save state, audio, crash and
resource quotas. Direct rule tests stay host-fast; QEMU proves launch/input/
render/close/restart.

## Application feature expansion

Do not expand by raw count. Add feature families when their dependencies exist:

### After process/session/file services

- truthful catalogue and package/app identity;
- lock/login/session switch;
- file picker, search, recent files and trash;
- document/PDF viewers in restricted decoders;
- accessibility focus/keyboard/semantic-name baseline;
- crash reporter and app restart.

### After physical network and package service

- download manager;
- package/software centre and updater/rollback UI;
- remote shell/file transfer;
- persisted browser bookmarks/history with privacy controls;
- optional mail/calendar/contact clients only after time, TLS, storage and
  notification contracts are stable.

### After audio/media providers

- mixer/volume UI;
- audio player/library and recorder;
- video player with restricted decoder;
- camera only after USB/UVC and explicit permission/indicator;
- game audio through AudioServer, never direct hardware access.

### After self-hosting/toolchain phase

- source editor/project browser;
- compiler/build/package UI;
- debugger/profiler/crash-symbol tools;
- source control client;
- system/package rebuild status and provenance viewer.

### Office creation and print/scan families

Office creation is a deferred but explicit product family: Writer/document,
Spreadsheet and Presenter share versioned document models, undo/redo, autosave,
recovery, import/export workers, accessibility semantics and print/export portals.
They arrive after process/session/file/config/parser services; they do not require
self-hosting first.

Print/scan starts only after the common device/provider lifecycle exists. A Print
Service owns printers, queues, job state, format conversion, cancellation, retry,
permissions and consumable/error telemetry. A Scan Service owns scanner/camera-like
capture, destination handles, page/image bounds and visible privacy/consent. Apps
receive job/capture handles, never raw USB or device authority. Spool files are
bounded, per-user and recoverable; device removal, paper/error states, cancel,
service restart and secret-document cleanup are acceptance tests.

## Truthful application catalogue gate

The current Maze/ID-14 defect establishes the minimum gate. For every expected
app, a generated inventory must prove:

```text
unique ID
exact name/category/version
package/source and shipped-image membership
constructor and process entrypoint
draw/output + event/input + tick/background routes as applicable
icon/size/accessibility metadata
at least one actual user launch route
launch -> visible ready nonce
close -> process/resource teardown
```

The inverse is also mandatory: every registered ID must resolve to a real app
and launch successfully or return an intentional typed refusal. Blank generic
tiles are failures. The checker must aggregate every failed field into its exit
status.

The checker itself has mutation proof. One fixture independently breaks each
field—existence, inverse blank ID, icon, size, constructor, draw/output, event,
tick/background, shipped-image membership, user launch route, ready nonce and
close/teardown—and every mutation must force a nonzero result. A dense numeric ID
range is never treated as evidence of a launch route.

## Boot, recovery, provenance and public-operation contracts

### Boot assertion matrix

A tiny assertion kernel verifies what the loader delivered. The matrix covers
BIOS, raw-sector, Multiboot64 and UEFI; exact boot origin; GPT/MBR and partition
selection; supported filesystems; fragmented kernel; modules; framebuffer and
memory map; oversized metadata/command line; stale artifact; malformed config;
panic absence; and a negative exit/result. Collected, skipped, passed and failed
cases are reported separately. An unrun firmware/architecture lane cannot inherit
another lane's result.

### Rescue and repair service

Ship a separately signed immutable rescue image with pinned input provenance,
read-only default mounts, exact target identity, dry-run/confirmation, typed
repair operations, a durable redacted repair log, and paired backup/restore.
Prove wrong-target refusal, disk full, corrupt metadata, power loss, failed restore
and the recovery-key/physical-presence policy. Credential bypass and passwordless
general operation are not zlOS features.

### Feature and provenance registry

Record claim, local/external origin, immutable revision, license/firmware/input
provenance, build target, shipped artifact, init and user route, tests, QEMU and
hardware profiles, recovery result and last verification. Generated compiler
headers additionally require consumers for each supported compiler, target and
language mode, including atomics, alignment, varargs, limits, unwind, startup and
link behavior.

### Public demo service

Prewarmed VM slots and COW overlays use one transactional
`reserve -> authenticate -> launch -> publish` operation with complete rollback.
Every lifecycle route, page and VNC/WebSocket stream checks the lease. The host
uses strict QEMU argument allowlists, per-VM identity, CPU/RAM/PID limits, network
egress policy, hard/idle deadlines, logs and a reaper. Termination proves process
identity and death before reusing a port or slot; cleanup failure remains visible
and retryable.

## Proof system

### Host deterministic layer

- driver descriptor and match tests;
- ring/queue/descriptor/register models;
- parser fuzz/sanitizer/property tests;
- block/network/audio fault simulators;
- app registry generation and reachability;
- direct app/game logic;
- package install/update/uninstall failure injection;
- service/app protocol conformance.

### QEMU layer

- fresh image/artifact nonce;
- driver start/online/request/stop/reset/restart;
- app install/launch/input/state/close/crash/restart;
- service kill/reconnect;
- malformed file/network/device inputs;
- resource exhaustion and deadline behavior;
- screenshot/surface hashes only when paired with semantic ready/results.
- boot assertion-kernel matrix with exact origin/media/artifact results.

### Hardware layer

- exact machine, firmware, PCI/USB identity and artifact;
- current/fallback driver selected;
- device effect and decoded completion;
- forced timeout/reset/retry where safe;
- repeated boot/start/stop and cold restart;
- persistent ZLLOG extraction after workload/failure;
- data/device integrity check after restart.

## Priority order

1. Fix the app-inventory gate so it tells the truth; do not count the blank ID
   14 tile as an app or Maze as reachable until a real route exists.
2. Finish retained client/shell surfaces and measured compositor deadlines.
3. Factor the async block provider around current NVMe/xHCI and preserve `zlfs`.
4. Establish UEFI64 processes, usercopy, handles, supervision and sessions.
5. Migrate system-truth and file/state apps first.
6. Factor NIC provider, add physical wired driver, then split network/browser.
7. Formalize driver lifecycle and migrate current display/input providers.
8. Add AudioServer plus one proved provider.
9. Add packages/updater and migrate the remaining utilities/games/apps.
10. Add the signed rescue image, provenance UI and contained public-demo path.
11. Deepen GPU, USB classes, power/suspend, Wi-Fi/Bluetooth and broader app
    families only through the same contracts.

This order preserves the full destination. It prevents breadth from hiding the
same missing foundations that the 33 external repositories repeatedly expose.

## What to revisit as the system grows

- whether drivers remain separate processes or selected boot-critical drivers
  stay in a constrained kernel domain;
- zero-copy versus copied IPC after measurements, not before;
- dynamic driver/plugin loading only after package authenticity, revocation
  and crash containment are proven;
- compatibility layers after several real applications define the ABI;
- multiarchitecture work only after x86 driver/service contracts are stable;
- richer distributed/agent applications after the ordinary application model
  is safe and observable.
