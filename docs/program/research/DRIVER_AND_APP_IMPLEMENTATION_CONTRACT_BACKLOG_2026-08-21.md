# zlOS driver and application implementation-contract backlog

Date: 2026-08-21

Status: implementation specification. Nothing in this document is a claim that
the destination process, provider, package, or hardware behavior already exists.

Inputs:

- [`ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md`](ZLOS_CURRENT_DRIVER_AND_APP_BASELINE_2026-08-21.md)
- [`DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md`](DRIVER_AND_APP_AUDIT_TAXONOMY_2026-08-21.md)
- [`DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md`](DRIVER_AND_APP_CLEAN_ROOM_INTEGRATION_PLAN_2026-08-21.md)
- the three repository-family driver/application deep dives and the all-33
  matrix in this directory.

## Contract template

Each item below is complete only when its implementation receipt contains:

1. exact source, compiler, configuration, dependency and artifact identities;
2. declared inputs, outputs, state, resource ownership and authority;
3. bounded queues, memory, retries, deadlines and failure result;
4. a deterministic positive test and the listed hostile tests;
5. QEMU proof where a virtual device or full app path exists;
6. named physical-machine proof where hardware support is claimed;
7. teardown, rollback and stale-handle proof;
8. an evidence-ledger update that distinguishes source, build, launch, scenario,
   recovery and hardware states.

The common transaction rule is `validate -> reserve -> mutate -> commit`. Every
failure before commit preserves the pre-state byte-for-byte or returns an
explicit, inspectable degraded state. No test wrapper may discard a failing exit
status or call a timeout success.

## Wave 0 — truthful inventory and retained fallbacks

### DA-00 — generated application registry

**Depends on:** current app constants, catalogue, shell/dock routes and app
dispatch functions.

**Deliver:** generate one registry from stable ID, name, category, package,
entrypoint, draw/output route, event route, tick/background route, icon,
accessibility label and supported launch surfaces. Generate dispatch tables and
catalogue data from it rather than maintaining parallel ranges.

**Invariants:** IDs and names are unique; every registered item has an entrypoint;
every named entry is reachable from at least one user route; every route resolves
to exactly one entry; retired IDs remain reserved; no blank fallback tile counts
as an app.

**Proof:** reproduce the current ID-14/Maze contradiction as a failing test; test
duplicates, gaps, retired IDs, missing constructor/draw/event/tick metadata,
unknown route and deliberate refusal. Boot the exact image and launch every item,
requiring a visible ready nonce and clean close receipt.

### DA-01 — feature/evidence ledger

**Depends on:** build manifest and artifact nonce.

**Deliver:** machine-readable records for claim, source, build edge, shipped
artifact, boot/init caller, launch route, positive scenario, failure scenario,
QEMU profile, hardware profile and last verified revision.

Each record includes `implementation_origin = local | submodule |
fetched_locked | system_dependency | bundled_content`, immutable source revision,
build target, image target, default reachability, and separate code/data/firmware
licenses.

**Invariants:** stronger states require all weaker prerequisites; one architecture
or device does not promote another; checked-in artifacts never count as current;
`external`, `stub`, `simulated` and `unreachable` remain first-class states.

**Proof:** reject missing source, stale image, absent init caller, simulated output,
ignored exit, timeout, architecture mismatch and hardware receipt from a different
artifact.

### DA-02 — immutable fallback paths

**Depends on:** GOP/software framebuffer, ZLLOG, PS/2 input, current NVMe/xHCI MSC
and zlfs read/write path.

**Deliver:** named fallback providers and a boot-safe selection policy. Keep them
available while accelerated or isolated replacements are introduced.

**Invariants:** a failed optional provider cannot remove the last display, log,
input or boot-storage path; fallback selection is visible; persistent data format
does not change implicitly.

**Proof:** fail each accelerated provider at every startup step and assert that
the fallback remains usable; recover ZLLOG after a desktop crash; read existing
zlfs data before and after provider wrapping.

### DA-03 — artifact and scenario identity

**Depends on:** build metadata and ZLLOG.

**Deliver:** signed or integrity-protected build ID, source revision, configuration
digest and randomized scenario nonce in kernel, service and app receipts.

**Invariants:** host, QEMU and hardware results bind to one artifact; a stale ISO,
kernel, package or screenshot cannot satisfy the current run.

**Proof:** deliberately run the previous image and require the gate to fail before
any behavioral assertion is accepted.

### DA-03F — firmware and platform service

**Depends on:** DA-03, early boot memory ownership and a bounded byte parser.

**Deliver:** architecture-neutral typed discovery from EFI, ACPI/uACPI, device
tree/FDT and SMBIOS. Record table origin, revision, checksum, byte length, firmware
hook availability, parsed resources, reserved memory, power methods and exact
unsupported/degraded states.

**Invariants:** every table/header/subtable/resource span is bounds- and checksum-
admitted before use; uACPI/AML OS hooks return truthful failure; duplicate/conflicting
resources do not publish; architecture-specific sources map into the same typed
model; parser or firmware-method failure cannot corrupt the live device graph.

**Proof:** truncated/oversized/looping tables, bad checksum, unknown revision,
missing hook, malformed AML/resource descriptor, duplicate device/range, absent
ACPI with valid DT, unsupported architecture, shutdown refusal and exact parity
receipts for every claimed architecture.

## Wave 1 — common device foundation

### DA-04 — device identity and discovery graph

**Depends on:** DA-03F and bus-specific enumeration.

**Deliver:** immutable device identities containing bus coordinates, generation,
IDs/class, firmware provenance, BAR/range sizes, interrupt options, DMA width,
coherency and parent topology.

**Invariants:** bus/slot/function order is canonical; multifunction devices are
enumerated; arithmetic is checked; unplug/re-enumeration changes generation;
drivers cannot invent global device IDs.

**Proof:** Nyaux-shaped `pci_map_bar` bus/function reversal, absent function zero,
64-bit/high BAR, malformed
capability list loop, overlapping resources, hot-remove/re-add and unknown firmware
node.

### DA-05 — driver descriptor and deterministic matching

**Depends on:** DA-04.

**Deliver:** signed descriptor with driver/ABI version, architecture, match rules,
required/optional features, resource needs, limits, deadlines, lifecycle support,
firmware/license provenance and proof profiles.

**Invariants:** matching is pure; ties resolve deterministically; unknown mandatory
capabilities and incompatible ABI/architecture fail before resource mutation.

**Proof:** ambiguous matches, missing dependency, unknown mandatory bit, unsupported
DMA width, bad signature/hash, wrong architecture and downgrade/rollback policy.

### DA-06 — resource and DMA authority

**Depends on:** handles and VM; an IOMMU mechanism is optional but changes the
evidence and isolation state.

**Deliver:** typed MMIO, PIO, IRQ, DMA, firmware, clock and power handles granted
only for the admitted device.

**Invariants:** no raw physical-address API for ordinary processes. With an IOMMU,
DMA mappings belong to a per-device default-deny domain and detach/revoke removes
translations before page reuse. Without one, the ledger says `not isolated` and
permits only a reviewed in-kernel driver or bounded bounce/pinned-buffer policy;
passthrough or unrestricted DMA never earns isolation. Interrupt and memory
ownership cannot overlap silently.

**Proof:** out-of-range MMIO, arbitrary physical mapping, DMA outside granted pages,
stale generation, double map/unmap, device reset with in-flight DMA, detach before
late completion, passthrough isolation-claim rejection, and explicit no-IOMMU
bounce/pinned-buffer bounds and page-reuse tests.

### DA-07 — driver lifecycle supervisor

**Depends on:** DA-04 through DA-06 plus process/service supervision.

**Deliver:** `Matched -> Admitted -> ResourcesReserved -> Starting -> Online ->
Degraded -> Quiescing -> Offline -> Removed`, with `Failed/Recovering/Quarantined`
branches. `Degraded` records lost functionality, user-visible effect, retry policy
and transition back to Online, Failed or Offline. Every pre-commit state can enter
`Aborting`; `abort(plan)` releases every dependency, BAR/port, IRQ, DMA map/domain,
firmware object, allocation and handle and returns a release receipt before the
device becomes Matched or Absent again.

**Invariants:** provider publication is atomic; every wait has a deadline; stop is
idempotent; remove masks interrupts and stops DMA before freeing resources; retry
budgets are finite; stale completions cannot mutate a new generation.

Every provider request carries provider epoch, device generation, request ID,
deadline, buffer provenance and cancellation state, and completes exactly once as:

```text
OK(result) | UNSUPPORTED(detail) | INVALID_INPUT(detail)
PERMISSION_DENIED(required_capability)
RESOURCE_EXHAUSTED(retryable, minimum_required) | TIMEOUT(stage)
CANCELLED(by) | DEVICE_GONE(generation) | PROVIDER_DIED(epoch)
PARTIAL(progress, continuation) | INTERNAL_ERROR(receipt_id)
```

Provider death stops admission, revokes DMA/IRQ/MMIO, settles every outstanding
request exactly once, resets or quarantines the device, and only then permits a
new provider epoch.

**Proof:** inject failure at every transition and allocation; repeated start/stop;
peer/service death; IRQ storm; timeout; reset failure; unplug with work in flight;
late IRQ/completion after resource reuse; abort after each reservation and compare
the complete resource/handle graph with pre-state.

### DA-07M — kernel/driver module loader

**Depends on:** DA-05 through DA-07 and DA-24 package authenticity. Until DA-24
exists, boot-critical providers remain statically linked and dynamic loading is
unsupported.

**Deliver:** stage the complete dependency closure; validate module byte length,
architecture, kernel ABI, imports/exports, symbol versions/conflicts, relocations,
constructors/init arrays, descriptor and resource plan; initialize privately and
publish atomically. Support fini/unload only for modules declaring and proving it.

**Invariants:** ordinary apps cannot request ring-0 loading; no symbol or driver is
visible before the transaction commits; failure rolls back dependency refs, memory,
registries and resources; unload quiesces workers/IRQs/DMA and refuses while live
objects, callbacks, mappings or references remain.

**Proof:** malformed/truncated module, wrong machine/ABI, missing/duplicate symbol,
relocation overflow, dependency cycle/version conflict, constructor/init failure,
nth-module rollback, concurrent load, unload under traffic, live-reference refusal,
worker timeout and late IRQ/callback after unload.

### DA-08 — driver health and failure receipts

**Depends on:** DA-07 and the DA-01 evidence/receipt sink.

**Deliver:** state, generation, resource ownership, queue occupancy, progress,
timeouts, resets, drops, last decoded hardware status and cleanup result as a
provider-side typed telemetry record. DA-08O consumes and aggregates it; DA-08 does
not depend on that service being alive.

**Invariants:** counters are monotonic or explicitly generation-scoped; health
reads cannot stall the driver; sensitive device data is redacted; dropped receipts
are themselves counted and anchored.

**Proof:** telemetry during wedge/reset/remove; counter saturation; log-full and
observer-death behavior; exact artifact/device identity on hardware.

### DA-08O — observability service

**Depends on:** DA-08, logger/ZLLOG import and process/service identity.

**Deliver:** typed read-only views for device lifecycle/resources/queues, storage
health, network/link/socket state, battery/power/sensors, audio streams/xruns, logs,
processes, sessions, services, packages and evidence receipts. Device Manager,
System Monitor and diagnostics consume the same records.

**Invariants:** unavailable is distinct from zero; records are generation- and
artifact-bound; secrets and cross-user data are redacted; retention and subscriber
queues are bounded; telemetry failure cannot stall providers; provider/service
restart cannot merge old and new epochs.

**Proof:** provider restart during subscription, unavailable field, counter wrap,
retention exhaustion, slow/dead subscriber, redaction, inconsistent source fixtures,
and byte-for-byte agreement between Device Manager/System Monitor for shared facts.

## Wave 2 — device-family providers

### DA-09 — display provider

**Depends on:** DA-04 through DA-08; preserve GOP/software.

**Deliver:** connector/mode/plane/cursor discovery, bounded surfaces, validated
atomic scene updates, present fences, vblank/hotplug, modeset, diagnostic capture,
reset and fallback. Wrap BGA/virtio-gpu first; bring the Intel investigation into
native boot without treating its host-harness modeset as a provider receipt.

**Invariants:** compositor alone owns scanout; stride/format/range and scene geometry
are checked; invalid work never reaches hardware; fence waits expire; failure falls
back without losing the session.

**Proof:** invalid mode/stride/plane, noncanonical address, ring full, fence timeout,
underrun, second modeset, repeated reset, software/accelerated scene equivalence,
QEMU BGA/virtio, separate Intel host-harness evidence, then a named native-boot
Intel panel/artifact/display-effect receipt.

### DA-10 — normalized input provider

**Depends on:** DA-04 through DA-08.

**Deliver:** device/generation/sequence/time plus normalized key, pointer, wheel,
touch and loss events. Move layout, composition, acceleration, gestures, focus and
accessibility to session policy.

**Invariants:** descriptor parsing is length-bounded; disconnect synthesizes safe
release; queue capacity/coalescing/drop is explicit; reconnect cannot revive stale
state; input grab requires the session/compositor right.

**Proof:** malformed HID descriptors, short reports, multiple keyboards/pointers,
hub churn, stuck key/button, sequence wrap, queue exhaustion, disconnect during
drag, focus theft and ordinary-process input-grab denial.

### DA-11 — asynchronous block provider

**Depends on:** DA-04 through DA-08.

**Deliver:** identity/capacity/block limits, `readv`, `writev`, `flush`, optional
discard, cancel, health and reset. First wrap NVMe and xHCI mass storage without
changing zlfs format.

**Invariants:** checked 64-bit LBA/span arithmetic; DMA direction/ownership explicit;
each buffer completes exactly once; stale completions are rejected; acknowledged
durability level is explicit; reset never silently replays a write.

**Proof:** zero/unaligned/out-of-range spans, queue exhaustion, timeout, controller
fatal/error bits, partial completion, cancel race, media removal, reset, flush
ordering, power cut and cold-read integrity.

### DA-12 — USB controller/topology/class split

**Depends on:** DA-07, DA-10 and DA-11.

**Deliver:** controller owns rings/slots/interrupters; USB service owns descriptor
admission, topology, address/configuration and class binding; class drivers receive
endpoint transfer handles only.

**Invariants:** descriptor totals and topology depth are bounded; a class driver
cannot access unrelated controller/device memory; endpoint ownership survives
short/error completion; unplug cancels every transfer exactly once.

**Proof:** malformed/recursive descriptors, hub-depth and port churn, short packets,
endpoint halt, reset during control/bulk/interrupt I/O, slot reuse, unplug with work
in flight, HID and mass-storage reconnect.

### DA-13 — NIC provider and packet simulator

**Depends on:** DA-04 through DA-08.

**Deliver:** identity/MAC/MTU/features/carrier, RX-buffer supply, TX packet, cancel,
counters and reset. Wrap virtio-net, add deterministic simulation, then physical
wired hardware; protocol/socket policy remains separate.

**Invariants:** packets are checked slices; RX buffers have single ownership;
backpressure and drops are counted; device reset cannot duplicate completion;
ordinary apps cannot configure raw NIC state.

**Proof:** runt/oversized frames, bad checksum/offload metadata, loss/duplicate/
reorder/delay, ring exhaustion, link flap, reset during TX/RX, DHCP/DNS/TCP/TLS
milestones kept separate, QEMU virtio and named physical NIC receipt.

### DA-14 — audio provider and AudioServer

**Depends on:** DA-07 and session authority.

**Deliver:** fake provider first, then HDA or `virtio-snd`; negotiated PCM formats,
bounded per-client rings, monotonic presentation clock, mixer, volume/mute, route,
underrun/overrun counters and explicit microphone permission/indicator.

**Invariants:** clients never access hardware; sample arithmetic clips by declared
policy; chunking does not alter deterministic integer output; capture cannot start
without a visible permission state; device reset preserves ownership.

**Proof:** asm.fm-derived original golden vectors, silence/max/mixed formats,
chunk-size invariance, ring under/overrun, client death, route change, clock drift,
reset, simultaneous clients and denied microphone access.

### DA-15 — clock, sensors and power provider

**Depends on:** DA-03F and DA-07.

**Deliver:** monotonic/wall clocks; read-only battery, AC, temperature, fan,
brightness, lid and power-button records with units, provenance and unknown state.
Power policy is a service, not a driver decision.

**Invariants:** time never moves backward within a monotonic generation; unavailable
sensor data is not zero; shutdown/reboot requires explicit power authority; suspend
cannot be advertised until every active provider supports quiesce/resume/fallback.

**Proof:** clock wrap/drift, RTC invalid fields, absent sensor, unit mismatch,
ordinary-process shutdown denial, provider refusing suspend, resume rebind failure
and fallback boot.

### DA-15S — system suspend and resume orchestration

**Depends on:** DA-03F, DA-07, DA-15 and every active provider declaring support.

**Deliver:** system `Online -> Suspending -> Suspended -> Resuming -> Online`
states, driven by a dependency DAG. Quiesce occurs in dependency order, resume in
the inverse required order, with per-provider state save/restore, deadlines,
rollback and explicit degraded/fallback convergence.

**Invariants:** no suspend claim while any active provider lacks the transition;
new work is stopped before state capture; DMA/IRQs are quiesced; resumed devices
use new generations; stale IRQs, mappings, handles and completions cannot reach
restored or reused state; partial resume never reports ordinary success.

**Proof:** dependency cycle, refusal/timeout at every suspend and resume position,
partial-provider failure, device missing after wake, clock discontinuity, stale
IRQ/DMA/completion, fallback display/storage/input, and repeated suspend/resume
cycles under I/O and app load.

### DA-16 — later device classes

**Depends on:** stable DA-05 through DA-08 and a real product need.

**Deliver:** common-contract providers for AHCI/virtio block, Wi-Fi, Bluetooth,
camera/UVC, touch/gamepad, printers, serial/GPIO/I2C/SPI and virtualization devices.

**Invariants:** none bypass the common authority, lifecycle, receipt or evidence
model; radio/camera discoverability and capture are user-visible policy; firmware
is pinned and attributed.

**Proof:** class-specific malformed corpora, hotplug, denial, reset and hardware
profiles. A source file or enumerated device remains below supported status.

## Wave 3 — process, session and application substrate

### DA-17 — hostile user boundary

**Depends on:** UEFI64 processes, address spaces, usercopy, handles and scheduler.

**Deliver:** one checked string/slice/copy layer and generated syscall dispatch.

**Invariants:** negative/sign-bit/gap/null syscall indices return a typed error;
kernel never dereferences raw user pointers; validation failure dominates access;
only the offending process dies on a copy fault.

**Proof:** `-1`, minimum integer, maximum, gaps, null entry; first/last byte unmapped,
read-only output, one-past object, arithmetic wrap, copyout fault and desktop
survival.

### DA-17L — hostile executable loader

**Depends on:** DA-17, File/VFS handles and transactional VM mapping.

**Deliver:** length-first ELF loader and, only if retained as a product goal, a
separately scoped PE loader. Validate complete headers, architecture/ABI, entry
sizes/counts, all offset+size arithmetic, segment/section/import/relocation spans,
entrypoint and permissions before constructing a private image and atomically
publishing the process.

**Invariants:** `filesz <= memsz`; mapped spans are canonical, non-overlapping and
within admitted image limits; W+X is rejected by policy; zero/nonadvancing entry
sizes fail; duplicate module/import ownership is explicit; partial load leaves no
PID, mappings, pages, files or handles.

**Proof:** every wrong magic/class/data/machine/header/entry size, zero/wrong entry
size, count*size and offset+size overflow, truncated table, overlap, W+X,
noncanonical/bad entry, `filesz > memsz`, zero-step relocation, missing/duplicate
import, concurrent exec and failure at every allocation/map/relocation/publish step.

### DA-18 — bounded typed IPC and shared objects

**Depends on:** DA-17.

**Deliver:** generated schemas, opaque endpoints/correlation IDs, sender identity,
message/byte quotas, capacity-aware receive, cancellation and revocable shared
buffers/surfaces.

**Invariants:** send/handle transfer is failure-atomic; destination is pre-reserved;
only owner or live `SHARE/DERIVE` authority grants; receiver consent and mapping
rights are explicit; no global sequential IDs; teardown makes handles stale. IPC
uses the DA-07 request envelope and terminal results. A bounded revoke that exhausts
traversal capacity returns `PARTIAL(progress, continuation)`; it never truncates
silently or returns ordinary success while descendants remain live.

**Proof:** nth-handle failure, empty/full/last-slot, receiver too small, peer close,
guessed ID, grant-self against foreign object, revoke, generation reuse, queue
exhaustion and concurrent teardown.

### DA-19 — service and app supervisor

**Depends on:** DA-17 and DA-18.

**Deliver:** stable role/protocol version, authority and exposure profiles,
dependency graph, authenticated endpoints, connection/message/memory/CPU budgets,
readiness/health, startup/shutdown deadlines, crash budget/backoff/quarantine,
checkpoint/restore, dependency-loss/reconnect, management authority, audit/redaction
policy and ordered reverse teardown.

**Invariants:** cycles and missing dependencies fail before effects; service
publication follows readiness; failure never becomes success; process identity is
generation-safe; termination proves death before resource reuse; complete mediation
and default-deny apply to endpoints and management; exposure never exceeds role;
restart invalidates old endpoint generations and settles requests exactly once.

**Proof:** cycle, dependency refusal, readiness timeout, crash loop, PID reuse,
kill failure, orphan child, restart during request, checkpoint corruption and
shutdown deadline; unauthenticated/cross-role endpoint, management denial, connection/
message/memory/CPU exhaustion, dependency loss/reconnect, audit overflow and secret
redaction.

### DA-20 — session, compositor and surface authority

**Depends on:** DA-09, DA-10, DA-18 and DA-19.

**Deliver:** login/session capability root, user-space window service, compositor-
exclusive scanout/input routing, opaque bounded surfaces, clipboard, notifications,
focus, grabs, drag-and-drop, per-seat state and accessibility semantics. Lock/logout
tears down or suspends session authority explicitly; restoration is versioned and
crash-safe.

**Invariants:** client geometry/stride/format/name are checked; only compositor can
present or globally grab input; clipboard reads require current policy/gesture;
revoked or closed surface cannot refresh; one client cannot starve the desktop;
focus, grab and drag objects are generation-bound; compositor restart reconstructs
only admitted live windows and cannot preserve stale authority.

**Proof:** oversized/negative geometry, stale surface, fake window ID, focus theft,
clipboard snoop, input grab denial, event flood, client crash during present and
compositor restart with session recovery; cross-session drag/drop, seat disconnect,
lock/logout with background work and stale focus/grab/drag after restart.

### DA-20C — settings and configuration service

**Depends on:** DA-18 through DA-20 and persistent File/VFS handles.

**Deliver:** typed, versioned schemas with user/system scopes, validation,
transactional writes, watches, migrations, export/import and reset-to-default.

**Invariants:** system writes require explicit authority; unknown/invalid fields do
not partially apply; watchers see one committed version; concurrent writers use a
declared conflict policy; restart converges from the journal without losing the
last committed configuration.

**Proof:** invalid type/range, unknown version, migration failure, concurrent
writers, watcher backlog/death, disk full, corrupt journal, service crash at each
write stage, export/import mismatch and per-user/session isolation.

### DA-21 — file/VFS and restricted parser services

**Depends on:** DA-11, DA-18 and DA-19.

**Deliver:** directory/file handles, namespace and mount authority, file picker,
search/recent/trash policy, and restartable image/font/archive/document/media parser
workers receiving only bounded bytes and output handles.

**Invariants:** no ambient paths or mount power; parsers receive explicit lengths
and budgets; a parser crash returns a failed object; user data survives service
restart; file mutations are transactional where promised.

**Proof:** traversal, symlink/parent loops, duplicate/case names, corrupt size/
offset/count, archive bomb, parser OOM/timeout/crash, unauthorized mount, disk full,
concurrent rename/write and power cut.

### DA-21F — file operation and safe-open portal

**Depends on:** DA-21 and DA-18/19 supervision.

**Deliver:** capability-scoped open/file-picker plus transactional copy, move,
delete, trash and undo jobs with stable IDs, progress, conflict policy, cancellation
and restart recovery. Cross-filesystem moves are explicit copy-verify-delete
transactions rather than an implied atomic rename.

**Invariants:** apps receive selected handles, not ambient path authority; each job
has exactly one terminal result; cancellation and service death preserve source and
committed destination state; overwrite/merge/rename decisions are explicit; undo
expires under a visible retention policy.

**Proof:** destination exists, permission change, disk full, source/destination
device removal, cross-filesystem partial copy, cancel at every step, service crash/
restart, symlink race, trash full, undo conflict and safe-open denial/revocation.

### DA-22 — socket, resolver and network policy services

**Depends on:** DA-13, DA-18 and DA-19.

**Deliver:** capability-scoped socket handles, DHCP/address service, resolver, TCP/
UDP and TLS policy separated from NIC provider; explicit raw-packet authority.

**Invariants:** network access follows app manifest and session policy; DNS/TCP/TLS
completion are distinct; secret material is never logged; service restart fails or
reconnects requests explicitly.

**Proof:** malformed packets/certificates, loss/reorder/timeouts, DNS poisoning
fixture, revoked network right, raw-socket denial, resolver/TLS crash and browser
survival.

## Wave 4 — package and application lifecycle

### DA-23 — application manifest and stable identity

**Depends on:** DA-00, DA-17 through DA-22.

**Deliver:** schema version; stable ID/name/version/publisher/content digest;
signing key ID, trust chain, revocation policy and anti-rollback version; target
architectures, kernel ABI range, executable ABI and entrypoint; required versus
optional service capabilities; file types/protocols; requested handles and limits;
windows/background work; complete file manifest and size limits; persistent-data
schema; accessibility metadata; migrations, rollback and uninstall-data policy;
deterministic/QEMU proof profiles; license and firmware provenance. Record
implementation origin, immutable source revision, exact build/image targets,
default reachability and separate code/data/firmware provenance. Declare dependency
version constraints, conflicts and optional dependencies.

**Invariants:** unknown mandatory capability, wrong ABI/architecture, excessive
budget or untrusted publisher fails before staging; display name/icon cannot replace
stable identity; authority is least and explicit. Dependency resolution is
deterministic; cycles, conflicts, missing or incompatible dependencies fail before
publication.

**Proof:** duplicate/spoofed ID, name collision, bad hash/signature, downgrade,
unknown capability, excessive resources, missing accessibility metadata and absent
entrypoint; dependency cycle/conflict/missing version and code/data license mismatch.

### DA-23L — launch and handler service

**Depends on:** DA-19 through DA-23 and DA-21F safe-open portals.

**Deliver:** registry-backed application, MIME, URL and file handlers; singleton
versus multi-instance policy; dependency readiness; safe-open handle transfer;
recent-document records; default/ambiguous-handler resolution; and typed launch
failure explanation.

**Invariants:** menus/search/file associations derive from installed artifacts;
handler choice cannot grant undeclared authority; singleton identity is generation-
safe; dependency readiness precedes publication; handler update/removal invalidates
stale routes without breaking an already authorized open transaction.

**Proof:** missing/stale handler, ambiguous/no default, spoofed MIME/URL scheme,
revoked file handle, dependency timeout, singleton race, handler update/removal
during launch, malformed executable and recent-document privacy/deletion.

### DA-24 — transactional install, update and uninstall

**Depends on:** DA-21 and DA-23.

**Deliver:** `stage -> validate all bytes/metadata/tools/roles/grants -> atomically
publish`; update with data migration/rollback; uninstall atomically unpublishes,
stops processes, revokes handles and deregisters callbacks before byte deletion.

**Invariants:** local integrity hash is distinct from publisher signature and
runtime audit; trust root/key/algorithm/policy version/rotation/revocation/anti-
rollback are recorded; failure preserves files, registry, roles, grants and handles.
The complete dependency set publishes atomically. Uninstall has an explicit refuse,
cascade or leave-dependent-broken policy; it never silently strands dependents.

**Proof:** inject every file, registry, migration, persist and publication failure;
path traversal, duplicate module, partial archive, wrong size, live process,
revocation, rollback, dependency-set partial failure, dependent uninstall and
explicit retain/delete user-data choices.

### DA-25 — application lifecycle and crash semantics

**Depends on:** DA-19, DA-20 and DA-23.

**Deliver:** installed/staged/start/ready/background/suspended/stop/crash/restart/
update/uninstall states with ready nonce, close receipt, crash report and unsaved-
state policy.

**Invariants:** window close and process lifetime are distinct; app publication
follows readiness; restart does not duplicate effects; background tasks are bounded
and visible; reports contain artifact/process identity but no secrets.

**Proof:** loader failure, readiness timeout, close with unsaved data, crash before/
after publication, restart loop, session end, service loss, resource exhaustion and
stale app handle after update.

### DA-25N — notification and crash services

**Depends on:** DA-08O, DA-19, DA-20 and DA-25.

**Deliver:** stable replace/update/dismiss notification IDs, typed actions, per-app
rate/quiet policy and quotas; artifact-bound crash capture and symbolization with
explicit privacy, redaction, storage and upload/consent policy.

**Invariants:** notification actions retain sender/app/session generation and
required authority; a stale or spoofed action cannot execute; crash collection
cannot expose another user or secrets; full queues/storage degrade visibly; service
restart preserves or discards state only by declared policy.

**Proof:** notification flood, replace/dismiss/action races, stale action after app
update/session end, quiet mode, service crash/restart, missing/mismatched symbols,
oversized crash, secret-redaction fixtures, storage full and denied upload.

### DA-26 — credential and privileged-operation matrix

**Depends on:** session/auth service and typed handles.

**Deliver:** atomic real/effective/saved UID/GID transition policy or native zlOS
identity equivalent; explicit rights for signal, debug, mount, MMIO, DMA, power,
scanout and input grab.

**Invariants:** checks use pre-state and fail closed; fork copies every identity
field exactly; failed credential drop cannot start a session; password secrets use
a salted memory-hard KDF and are zeroed/not logged.

**Proof:** ordinary `setreuid(-1,0)` analogue, all transition combinations, fork
snapshot, failed group drop, signal zero, foreign process, mount/MMIO/power/scanout/
grab denial and authentication log redaction.

## Wave 5 — migrate every current application family

### DA-27 — system-truth applications

**Depends on:** DA-08O, DA-19 and the relevant provider/service schemas.

**Apps:** System Monitor, About, Clocks & Timers, Kernel Log, System Info, Services,
Disk Usage, Network, Framebuffer, Renderer, Font Atlas, Keyboard Tester and
Benchmark.

**Deliver:** read-only clients of typed telemetry/provider/supervisor interfaces.

**Proof:** no arbitrary kernel-global access; permission/redaction tests; provider
restart and unavailable states; displayed values match recorded fixtures and never
silently render unknown as zero.

### DA-28 — file and state applications

**Depends on:** DA-20C, DA-21/DA-21F, DA-23L and DA-25.

**Apps:** Files, Text Editor, Sticky Notes, Hex Viewer, Archive Manager, Image
Viewer, Text Diff and Checksum.

**Deliver:** directory/file handles, file picker, safe-save/rename, autosave/recovery,
restricted parsers and persistent schema migration.

**Proof:** current zlfs differential fixtures, disk full, rename collision, crash
during save, corrupt input, huge input budget, revoked handle, cold boot and recovery.

### DA-29 — desktop and session applications

**Depends on:** DA-20, DA-20C, DA-23L, DA-25 and DA-25N.

**Apps:** Terminal, Console (`tty1`), Run, Settings, Menu/catalogue/dock, Clipboard,
notifications, login/lock and accessibility tools. Console consolidates into the
Terminal/PTY service while its stable ID is preserved as a migration alias or
explicitly retired; it cannot disappear as an untracked gap.

**Deliver:** shell/PTY process tree; clients of DA-23L launch/handler routing,
DA-20C settings and DA-25N notification/crash services; clipboard privacy, focus
navigation and semantic names.

**Proof:** hostile command arguments, child/orphan cleanup, missing executable,
catalogue inverse mapping, clipboard snoop denial, lock-session isolation, keyboard-
only operation and screen-reader metadata inspection.

### DA-30 — network and content applications

**Apps:** Browser, download manager, package client, remote shell/file transfer and
later mail/calendar/contact clients.

**Deliver:** split UI, fetch/TLS and hostile parser/content processes with scoped
network/file rights and privacy controls for history/bookmarks/cache.

**Proof:** parser/network service crash, malicious page/file/certificate, download
path collision, revoked network, offline mode, cache/history deletion and no desktop
or credential compromise.

### DA-31 — deterministic tools and creation apps

**Apps:** Calculator, Colour Picker, converters, regex, diff/checksum, Paint, 3D,
animation, pointer demos and later editor/compiler/build/debug/profile/source-
control tools.

**Deliver:** pure cores separated from UI and privileged effects; versioned project
formats; reproducible compiler/build receipts after self-hosting.

**Proof:** host golden/property tests, locale/overflow/large input, malformed project,
undo/redo and safe-save, app crash/restart, QEMU launch/input/render/close.

### DA-31O — office document creation suite

**Depends on:** DA-20C, DA-21/21F, DA-23L, DA-25 and restricted parser workers.

**Apps:** Writer/document editor, Spreadsheet and Presenter.

**Deliver:** shared versioned document object models, undo/redo, autosave/recovery,
templates/styles, formulas/layout/slides, import/export workers, accessibility
semantics and print/export portal integration.

**Invariants:** hostile imported formats execute no code and have byte/time/memory/
output bounds; save is transactional; autosave never overwrites the last recoverable
version; formulas/layout cannot escape document authority; embedded media are
separate restricted objects.

**Proof:** corrupt/oversized import, formula cycles and resource exhaustion,
layout/font/image parser crash, disk full, crash during save/autosave, schema
migration/rollback, undo/redo persistence, keyboard/screen-reader operation and
round-trip fixtures with explicit fidelity limits.

### DA-32 — all games as runtime probes

**Apps:** the current 24 named games, including Maze only after its route exists.

**Deliver:** deterministic seeded rules, event/timer/render separation, save state,
resource budgets and AudioServer use. Preserve each current game's behavior while
moving it into an ordinary process.

**Proof:** direct rule suites plus registry/launch/ready/input/render/close/restart;
seed replay, pause/time wrap, malformed save, quota exceed, audio loss and one-game
crash while desktop and other apps continue.

### DA-33 — media applications

**Apps:** mixer, audio player/library, recorder, video player and camera after
providers exist.

**Deliver:** media library/index, playback queue, visible recording state and
restricted codecs. Device ownership remains with AudioServer/camera provider.

**Proof:** corrupt/hostile formats, seek/end-of-stream, underrun, route change,
library rescan, revoked file/microphone/camera access, decoder crash and exact UI
indication of capture.

### DA-33P — print, spool and scan services/apps

**Depends on:** DA-07/12/16 device contracts, DA-21F, DA-23L and session consent.

**Deliver:** printer/scanner providers; per-user Print Service queue and spool;
typed job/cancel/retry/status; bounded format-conversion worker; Scan Service with
page/image limits, destination handles and visible capture permission/indicator;
printer/scanner settings and job UI.

**Invariants:** apps never receive raw USB/device authority; spool data is private,
bounded and recoverable; cancellation and device/service death settle each job
once; completed/cancelled secret documents follow explicit retention/deletion;
capture cannot begin without current consent and visible indication.

**Proof:** unsupported printer/scanner, malformed descriptor/job/document, spool
disk full, paper/ink/device error, unplug mid-job/page, cancel/retry race, service
crash/restart, cross-user spool access, stale job handle, denied capture, oversized
scan and restricted converter crash.

### DA-34 — software centre and provenance UI

**Depends on:** DA-24.

**Deliver:** package discovery, installed/update state, permissions, publisher/key,
artifact/proof receipts, migration/rollback and recovery UI.

**Proof:** offline/stale metadata, compromised key/revocation, partial download,
disk full, rollback, live app update, incompatible ABI and honest display of source-
only versus verified features.

### DA-35 — agent and automation applications

**Depends on:** safe ordinary app model, DA-18/19/23/24/26.

**Deliver:** isolated model/orchestrator process, small deterministic tool broker,
compiled typed plans, consent, provenance, bounded jobs, attenuated child handles,
deadlines/revocation and independent result receipts.

**Invariants:** no ring-0 orchestrator; child grants derive at commit from live parent
handles; revoke serializes with spawn; killing parent/root makes descendants dead
and every handle stale; plugins/tools cannot gain undeclared authority.

**Proof:** revoke-then-spawn, concurrent spawn/revoke, parent/root kill, orphan
search, prompt/tool injection, deadline, queue/memory exhaustion, bad tool package,
partial effect, consent denial and independent reviewer contradiction.

## Wave 6 — operations, recovery and public use

### DA-36 — boot-contract matrix

**Depends on:** DA-03 and a tiny assertion kernel.

**Deliver:** BIOS/UEFI and supported architecture/media/origin cross-products for
partition, filesystem, fragmentation, modules, command line and framebuffer.

**Proof:** correct origin and metadata plus malformed configuration/header/size/
offset/partition inputs. Report collected, skipped, passed and failed separately;
never average an unrun architecture into success.

### DA-37 — rescue and repair environment

**Depends on:** DA-03, DA-11, DA-21 and package authenticity.

**Deliver:** separately signed immutable rescue image, read-only default mounts,
exact target identity, dry run/confirmation, typed repair actions, backup/restore
pair and durable redacted audit.

**Proof:** wrong target, mounted target, power loss, full backup medium, corrupt
metadata, failed restore and recovery-key/physical-presence policy. No credential
bypass tool or passwordless general operation counts as a product feature.

### DA-38 — transactional public QEMU demo

**Depends on:** DA-03, DA-19, DA-24 and network isolation.

**Deliver:** prewarmed slots, random lease, COW overlay, hard/idle deadlines,
authenticated lifecycle and display/data plane, strict QEMU argument allowlist,
per-VM identity, CPU/RAM/PID/egress limits, logs and reaper.

**Invariants:** allocate/launch is one recoverable transaction; stored tokens are
hashed; PID/VM identity is proven; death is verified before port/slot reuse; cleanup
failure remains visible and retryable.

**Proof:** nth-stage launch failure, cache/database split, guessed/enumerated VNC
path, missing/expired token on every route/stream, PID reuse, unkillable VM, orphan
overlay, egress attempt, resource exhaustion, concurrent allocator and reaper.

### DA-39 — provenance and architecture history

**Depends on:** DA-01 and normal version control.

**Deliver:** architecture-epoch tags, parent-connected migrations, imported/generated
boundaries, per-driver/app feature receipts and a queryable change rationale.

**Proof:** schema/link/duplicate validation, missing evidence link, rewritten source
identity, generated artifact without inputs and license/provenance gap.

## Wave 7 — promotion gates

### DA-40 — host deterministic gate

Run real shared logic where possible: descriptor/register/ring models, parser fuzz
and sanitizers, block/network/audio fault simulation, registry generation, app/game
logic, package transaction injection and protocol conformance. Build-only fixtures
are named build-only and cannot pass an execution gate.

### DA-41 — QEMU system gate

Boot a fresh artifact nonce; start/reset/stop/restart providers; install/launch/
interact/close/crash/restart apps; kill/reconnect services; inject malformed data,
timeouts and exhaustion. A screenshot needs a semantic ready/result oracle.

### DA-42 — physical hardware gate

Record machine, firmware, exact PCI/USB identity, artifact, selected/fallback
provider, request/effect/completion, safe forced recovery, repeated start/stop/cold
boot and post-workload data/device integrity. One machine proves only that profile.

### DA-43 — unsupported and degraded-state gate

Every missing feature has a typed visible state: unsupported, external, simulated,
stub, source-only, built, unreachable, unavailable, degraded, failed or quarantined.
No fixed success, silent drop, generic blank tile or fake output is permitted.

### DA-44 — independent contradiction gate

Before promotion, a separate reviewer tries to disprove the claim from source,
artifact identity, reachability, failure cleanup, teardown and architecture/hardware
coverage. Corrections update both the evidence ledger and the user-facing feature
surface; old broad wording cannot remain authoritative.

## Dependency and delivery order

```text
DA-00..03 + DA-03F truth/fallback/firmware
    -> DA-04..08 + DA-08O common devices/observability
    -> DA-09..16 + DA-15S family providers and suspend/resume
    -> DA-17..22 + DA-17L + DA-20C + DA-21F process/session/services
    -> DA-23..26 + DA-23L + DA-25N package/app/security lifecycle
    -> DA-07M authenticated dynamic kernel/driver modules
    -> DA-27..35 + DA-31O + DA-33P migrate and expand apps
    -> DA-36..39 operations/recovery/history
    -> DA-40..44 promotion gates applied continuously
```

The first user-visible checkpoint fixes the Catalogue/Maze false green and keeps
all retained fallbacks. The first architectural checkpoint wraps existing NVMe,
xHCI, display, input and virtio-net behind the common contracts without deleting
the proven code. The first isolation checkpoint launches system-truth and file
apps as ordinary processes. Breadth follows those contracts; it does not substitute
for them.

## Definition of the complete driver/application program

The program is complete only when:

- every current and added device appears in the discovery/driver/provider ledger
  with an honest evidence state;
- every supported provider has bounded startup, request, reset, stop and removal
  receipts on each claimed QEMU/hardware profile;
- every current named application is uniquely registered, shipped, launchable,
  isolated, permissioned, closable, restartable and covered by its family contract;
- package install/update/uninstall and persistent data survive injected failure;
- hostile files, packets, descriptors, user pointers and device faults cannot halt
  the desktop or corrupt unrelated state;
- unsupported, partial and simulated features remain visibly labeled;
- no external repository code is copied: only independently written contracts,
  tests, behaviors and UX ideas enter zl/zlOS.
