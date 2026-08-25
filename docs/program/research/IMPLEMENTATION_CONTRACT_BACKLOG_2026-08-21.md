# Implementation contract backlog for zl and zlOS

Date: 2026-08-21

Status: implementation-ready decomposition of [`CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md`](CLEAN_ROOM_ZL_ZLOS_INTEGRATION_PLAN_2026-08-21.md). A row is a contract to implement and prove, not a claim that the deliverable exists.

## Current authority and ordering

This backlog preserves the current product sequence from [`kernel/HANDOFF.md`](../../../kernel/HANDOFF.md), [`performance-architecture-roadmap.md`](../../../kernel/docs/plans/performance-architecture-roadmap.md), [`retained-window-surfaces.md`](../../../kernel/docs/plans/retained-window-surfaces.md), and [`docs/EXECUTION-ROADMAP.md`](../../EXECUTION-ROADMAP.md). The immediate zlOS work remains retained client and shell surfaces, regions/deadlines, async block/storage, 64-bit processes, real networking, then GPU/audio/application depth.

Language dependencies follow the retained [`HANDOFF.md`](language/HANDOFF.md), [`ULTIMATE_PLAN.md`](language/ULTIMATE_PLAN.md), [`ROADMAP.md`](language/ROADMAP.md), and [`MASTER_PLAN.md`](language/MASTER_PLAN.md) snapshots: imports; scoping/errors; records/maps; exact integers/bytes; types/unboxing; FFI/allocators/comptime; ecosystem; source self-hosting.

External ideas and rejection boundaries come from [`ALL_33_FEATURE_MATRIX_2026-08-21.md`](ALL_33_FEATURE_MATRIX_2026-08-21.md), [`PROTOS_KERNEL_AUDIT_2026-08-21.md`](PROTOS_KERNEL_AUDIT_2026-08-21.md), the first-pass [`MATURE_OS_AUDIT_2026-08-21.md`](MATURE_OS_AUDIT_2026-08-21.md), [`ARCHITECTURE_OS_AUDIT_2026-08-21.md`](ARCHITECTURE_OS_AUDIT_2026-08-21.md), and [`FOCUSED_REPOSITORIES_AUDIT_2026-08-21.md`](FOCUSED_REPOSITORIES_AUDIT_2026-08-21.md), corrected by [`MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md), [`ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](ARCHITECTURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md), and [`FOCUSED_REFUTATION_AND_COVERAGE_2026-08-21.md`](FOCUSED_REFUTATION_AND_COVERAGE_2026-08-21.md). No row authorizes copying external source.

## Contract schema

Every backlog row supplies:

- **Dependencies/current/provenance:** what must exist first, what zl/zlOS asset is extended, which external idea shaped the behavior, and what is explicitly rejected.
- **I/O and state:** accepted inputs, observable outputs, and the lifecycle or transaction state machine.
- **Invariants/failure:** properties that must always hold and the exact failure behavior.
- **Deterministic proof:** host/unit/property/fuzz/differential tests that do not need timing luck.
- **Target proof:** QEMU and hardware gates where they add evidence unavailable on the host.
- **Receipt/removal:** artifact evidence to retain, safe rollback, and the condition for deleting or demoting the legacy path.

Common state machines:

```text
feature: claimed -> source -> reachable -> built -> booted -> scenario -> hardware
service: Absent -> Starting -> Ready -> Degraded -> Recovering -> Stopping -> Absent
request: Created -> Admitted -> Queued -> Running -> Completed|Failed|Cancelled
package: Absent -> Staged -> Verified -> Committing -> Active -> Retiring -> Absent
lease: Free -> Reserved -> Launching -> Active -> Expiring -> Stopping -> Cleaning -> Free
```

No transition may skip admission or publish success before durable commit. Every state has one owner, bounded time/resource use, cancellation, cleanup, counters, and a terminal error.

## Universal receipt and gate rules

Every successful row emits a receipt containing contract ID, source commit, configuration, toolchain digest, artifact digest, host/QEMU/hardware profile, command, exit status, expected/observed result, timestamps, logs or ZLLOG slot, failure injection used, skipped scope, reviewer, and weakest link.

Every gate must:

1. fail if its inner command fails;
2. include one canary proving failure propagates;
3. identify the exact booted artifact inside the guest;
4. keep failing artifacts and logs;
5. distinguish host, QEMU, and physical evidence;
6. refuse promotion when a dependency has weaker evidence than required.

## Phase 0 — preserve and inventory the proven baseline

Phase exit: every current headline feature has an exact evidence state, shipped-module identity, hazard owner, and weakest link. Nothing current is replaced.

### P0.1 — machine-readable feature ledger

- **Dependencies/current/provenance:** extend the existing documentation/evidence culture and ZLLOG artifact identity; use RustOS receipts and banan-os maturity lists; reject README/file-count completion.
- **I/O and state:** input is one feature record plus source/build/test/hardware references; output is validated JSON or equivalent and generated Markdown; state follows the feature machine above.
- **Invariants/failure:** immutable feature ID; monotonic evidence; exact commit/artifact; one owner; impossible transitions reject without modifying the ledger.
- **Deterministic proof:** schema property tests, missing-field corpus, illegal-state transitions, duplicate IDs, stale artifact hash, generated-doc reproducibility.
- **Target proof:** QEMU and ThinkPad receipts ingest without manual rewriting and preserve their distinct profiles.
- **Receipt/removal:** retain schema version and generated diff; rollback is previous ledger generation; remove hand-maintained feature status tables only when every row is generated.

### P0.2 — shipped-source and module inventory

- **Dependencies/current/provenance:** current `SOURCES` and source verification; Zinnia narrow modules and Brook build-boundary tests; reject source-present equals shipped.
- **I/O and state:** build graph and linked image in; source/object/module/symbol manifest out; state is `declared -> compiled -> linked -> loaded`.
- **Invariants/failure:** every linked object maps to declared source/license; every required declaration is linked; host-only files require explicit allowlist; unknown/missing entries fail build.
- **Deterministic proof:** add/remove/rename fixture, duplicate object, host-only misclassification, dead module, reverse-symbol reconciliation.
- **Target proof:** guest prints manifest digest; loader reports exactly the same required modules.
- **Receipt/removal:** manifest plus image digest; rollback is last known build graph; retire ad hoc reverse greps only after equivalent generated checks gate every build.

### P0.3 — current-asset receipt backfill

- **Dependencies/current/provenance:** existing ZLLOG, zlfs, framebuffer, WM, browser/net, Intel, xHCI, NVMe, host and QEMU gates; RustOS evidence discipline; reject inherited numbers.
- **I/O and state:** existing commands/logs/artifacts in; one normalized receipt per asset out; state remains at the strongest actually observed level.
- **Invariants/failure:** no new runtime claim; stale/unidentified artifacts remain unverified; conflicting docs are recorded, not averaged.
- **Deterministic proof:** receipt parser, missing identity, mismatched hash, contradictory result, stale date/commit fixtures.
- **Target proof:** current physical ZLLOG and current QEMU artifact are independently ingestible.
- **Receipt/removal:** evidence index links raw artifacts; rollback is deletion of derived index only; no raw evidence is removed.

### P0.4 — strict outer gate and documentation authority

- **Dependencies/current/provenance:** current build/test scripts and orientation docs; Hyper/RustOS strict matrices; reject Cyjon/vib-OS/Lemon/Mollen false-green patterns.
- **I/O and state:** ordered gate steps in; one terminal status and per-step receipt out; docs classified `authority`, `active contract`, `history`, or `superseded`.
- **Invariants/failure:** first failure cannot be erased; cleanup runs separately; timeout is failure unless explicitly expected; authority precedence is machine-readable.
- **Deterministic proof:** planted failure at first/middle/last step, timeout, missing executable, skipped test, stale document link.
- **Target proof:** intentionally failing guest assertion makes CI red; successful guest sends exact success sentinel.
- **Receipt/removal:** gate graph and canary log; rollback preserves old commands behind diagnostic-only entry; remove obsolete top-level gates after two consecutive matching runs.

## Phase 1 — measured retained desktop architecture

Phase exit: the same physical interaction trace proves unchanged clients and shells are not redrawn, pixels are correct, deadlines/drop contracts hold, and direct software rendering remains available.

### P1.1 — retained client surfaces

- **Dependencies/current/provenance:** existing WM, 64 MiB heap, direct draw path and [`retained-window-surfaces.md`](../../../kernel/docs/plans/retained-window-surfaces.md); Serenity/Lemon/banan buffers; reject blank-on-allocation-failure and raw client pointers.
- **I/O and state:** window/client dimensions and explicit app invalidation in; bounded RGB32 surface plus generation/validity out; state `None -> Allocated/Invalid -> Rendering -> Valid -> Retiring -> None`.
- **Invariants/failure:** overflow checked; 48 MiB initial aggregate budget; target/clip restored on every exit; allocation refusal uses current direct renderer; free occurs after compositor retirement.
- **Deterministic proof:** byte-identical direct/surface draw, allocation refusal, resize/minimize/close, repeated lifecycle heap audit, invalidation generation race.
- **Target proof:** QEMU boot/drag; physical trace shows zero `hook_draw` for unchanged covered app and no ZLLOG drops.
- **Receipt/removal:** scene hashes, allocation counters and trace; disable switch returns to direct rendering; do not remove direct path.

### P1.2 — retained shell/chrome/shadow and desktop layers

- **Dependencies/current/provenance:** P1.1 and measured chrome/desktop phase data; DirectComposition/Core Animation and Serenity boundaries; reject client-only cache called complete.
- **I/O and state:** geometry/focus/hover/title/theme/dock changes in; separately versioned shell/shadow/desktop layers out; each layer has `Invalid -> Rendering -> Valid`.
- **Invariants/failure:** another window's move cannot invalidate stable shell/client; focus/theme changes invalidate only required layers; memory accounting is separate.
- **Deterministic proof:** move/raise/focus/hover/theme/workspace matrix with draw counters and scene hashes; eviction/refusal fallback.
- **Target proof:** physical phase trace materially removes the measured chrome owner and reports desktop cache budget.
- **Receipt/removal:** before/after phase journal and layer-hit counters; per-layer feature flags roll back independently; direct chrome remains until parity.

### P1.3 — precise invalidation and bounded regions

- **Dependencies/current/provenance:** P1.1; current damage list; Serenity disjoint regions and Linux DRM semantics; reject bounding touching/L-shaped rectangles blindly.
- **I/O and state:** app invalidation, screen damage and occlusion geometry in; bounded disjoint/area-aware region plus fallback reason out.
- **Invariants/failure:** all changed pixels included; no false exclusion; merge-waste threshold explicit; fragmentation cap causes counted full damage.
- **Deterministic proof:** L shape, touching chain, partial occlusion, transparency, screen edges, overflow, full-list fallback, randomized oracle against pixel mask.
- **Target proof:** QEMU scene suite; physical journal shows reduced damage pixels/window visits without stale pixels.
- **Receipt/removal:** region corpus hash and telemetry; switch restores old conservative full/bounding damage; remove old merger after visual/property parity.

### P1.4 — atomic WM commit and newest-frame pacing

- **Dependencies/current/provenance:** P1.1-P1.3, current input/frame telemetry; mainstream compositor deadline concepts and NexiOS boundedness; reject blanket realtime and deep visual queues.
- **I/O and state:** input/app mutations and layer generations in; one atomic scene commit, predicted deadline and at most one pending visual frame out; `Idle -> Collecting -> Committed -> Composing -> Presented|DroppedAsStale`.
- **Invariants/failure:** newest state wins; no partially visible metadata; HID path does not paint; missed deadline and queue depth counted; idle returns to `hlt`.
- **Deterministic proof:** controlled clock, event burst/coalescing, commit atomicity, deadline edge, stale-frame replacement, idle wake.
- **Target proof:** QEMU causal trace; ThinkPad HID-to-route p95 below 1 ms, no visible frame above current 16.67 ms contract, `late=0`, `lost=0` for scripted run.
- **Receipt/removal:** percentile/raw trace and commit counters; rollback selects old frame loop; remove old scheduling only after physical parity.

## Phase 2 — asynchronous block layer and crash-consistent zlfs

Phase exit: NVMe, xHCI USB and images share one contract; no foreground storage in IRQ/input/paint; every injected write boundary recovers to an old or new committed state.

### P2.1 — common block provider

- **Dependencies/current/provenance:** current NVMe/xHCI/ZLLOG paths; Zinnia driver seams, Astral storage questions; reject controller-specific filesystem calls.
- **I/O and state:** provider handle, LBA/count, bounded buffer and operation in; capacity/block-size/result/residue out; provider state follows service lifecycle plus `Online -> Quiescing -> Offline`.
- **Invariants/failure:** checked range arithmetic; exact buffer length; no request beyond capacity; timeout/cancel/reset explicit; unsupported trim returns typed error.
- **Deterministic proof:** fake provider for zero/end/overflow/short I/O, timeout, retry, reset, flush ordering and hot removal.
- **Target proof:** QEMU NVMe/USB where modelled; physical xHCI ZLLOG and NVMe named-file operations through same suite.
- **Receipt/removal:** provider/version/geometry and operation trace; adapters retain old direct functions; remove controller-specific upper calls after caller inventory is zero.

### P2.2 — bounded async request/completion queue

- **Dependencies/current/provenance:** P2.1 and scheduler clocks; NexiOS bounded queues, Brook host tests; reject unbounded waits and I/O in IRQ/frame context.
- **I/O and state:** admitted request with deadline/cancel token in; completion handle/result out; request machine uses universal states.
- **Invariants/failure:** fixed capacity/ownership; IRQ only records completion; cancellation is terminal/idempotent; full queue returns backpressure; late completions cannot touch reused requests.
- **Deterministic proof:** controlled completions, full/one-over, cancel races, reset while queued/running, duplicate/late completion, wraparound IDs.
- **Target proof:** QEMU save while dragging; physical journal shows no block work in input/paint/IRQ and bounded latency counters.
- **Receipt/removal:** queue high-water/latencies/outcomes; sync adapter rollback; remove direct synchronous UI writes when trace caller count is zero.

### P2.3 — page/block cache and writeback worker

- **Dependencies/current/provenance:** P2.2 and heap/accounting; Astral page-cache boundary; reject acknowledgment that implies durability before flush.
- **I/O and state:** read/write pages, dirty policy and explicit sync in; cache hit/data or completion/failure out; page `Absent -> Clean -> Dirty -> Writeback -> Clean|Error`.
- **Invariants/failure:** bounded bytes; pin/ref counts; dirty data not evicted; explicit Save waits for metadata durability; failed provider retains dirty/error state.
- **Deterministic proof:** hit/miss/eviction, dirty-pressure, concurrent reads/writes, writeback error/retry, forced sync, shutdown drain, no-frame-allocation assertion.
- **Target proof:** QEMU fault-injected storage during desktop workload; hardware queue/cache telemetry in ZLLOG.
- **Receipt/removal:** hit/miss/dirty/flush metrics and disk hash; disable cache uses P2.1 sync provider; no removal of safe sync fallback.

### P2.4 — zlfs format and crash transaction

- **Dependencies/current/provenance:** P2.1-P2.3 and existing zlfs/fstest/probe-files; Brook tests and Mollen immutable packaging; reject mYOS-style monolithic unversioned state.
- **I/O and state:** versioned volume and file transaction in; committed generation/file handle out; transaction `Open -> DataWritten -> DataFlushed -> MetadataWritten -> MetadataFlushed -> Published`.
- **Invariants/failure:** CRC/version/range checks; data before metadata; old generation remains valid until publish; disk-full never truncates existing file; names/lengths bounded.
- **Deterministic proof:** power cut after every write, corrupt super/metadata/data, full disk, rename/write race, stale handle, unsupported version, model/reference comparison.
- **Target proof:** separate QEMU cold boots reopen exact bytes on NVMe and USB-backed test media; physical safe target validates commit/recovery.
- **Receipt/removal:** pre/post volume digest, cut point and recovered generation; read-only mount rollback; old flat format removed only after migration/import and recovery proof.

### P2.5 — named-file migration and ZLLOG continuity

- **Dependencies/current/provenance:** P2.4, current Files/editor/Settings/browser data and raw ZLLOG; unix-history provenance; reject dual sources of truth without migration state.
- **I/O and state:** old slot/private sector and destination path in; copied/verified marker out; `Unseen -> Copied -> Verified -> Switched -> Retired`.
- **Invariants/failure:** source not deleted before byte verification and reboot; repeated migration idempotent; ZLLOG writes remain independent of VFS readiness.
- **Deterministic proof:** interruption at each state, duplicate run, corrupt source/destination, rollback, zero-caller inventory for old API.
- **Target proof:** QEMU and physical cold reboot for Settings/editor/Files/browser data; early crash still appears in raw ZLLOG.
- **Receipt/removal:** per-item hashes and caller inventory; switch back before retirement; delete numbered slots/private paths only after verified migration and zero callers.

## Phase 3 — real 64-bit processes and hostile-user boundary

Phase exit: malicious user programs fault alone and cannot access kernel/device/other-process authority; one minimal zl executable runs and exits on the UEFI64 path.

### P3.1 — syscall entry, kernel stack and return

- **Dependencies/current/provenance:** current 32-bit Ring-3 proof, GDT/TSS/IDT and x86-64 boot; Serenity/Brook process boundaries; reject Rings 1/2 and premature `sysret`.
- **I/O and state:** validated syscall number/register frame in; result/error and restored user frame out; `User -> EntryStub -> KernelStack -> Dispatch -> ReturnCheck -> User|Killed`.
- **Invariants/failure:** syscall index is unsigned and checked before scaling/indexing; interrupts/flags/canonical addresses checked; unknown/null slot returns `ENOSYS`; generated table length matches dispatch; kernel stack per running thread; no user stack trusted during dispatch.
- **Deterministic proof:** `-1`, sign-bit/`INT64_MIN`, maximum, every valid number and gap, null entry, malformed return RIP/RSP/RFLAGS, nested interrupt, generated table length/non-null assertion, TacOS negative-index and protOS NULL-unshare regressions.
- **Target proof:** QEMU UEFI64 round trip and malicious return corpus; hardware boot retains exact fault/return records.
- **Receipt/removal:** ABI version/register transcript; conservative `iretq` remains rollback; add `sysret` only as separately gated optimization.

### P3.2 — process object and address space

- **Dependencies/current/provenance:** P3.1, PMM/VMM/scheduler; RustOS atomic mapping, NexiOS ownership; reject fixed shared user stacks and ambient mappings.
- **I/O and state:** image/manifest/limits/parent in; PID, PML4, stacks, handles and state out; `New -> Loading -> Runnable -> Running/Blocked -> Exiting -> Reaped`.
- **Invariants/failure:** one PML4 per process; supervisor-only kernel/device; W^X/NX; guarded unique stacks; full-span admission before commit; cleanup reverses partial load.
- **Deterministic proof:** overlap/overflow/OOM plus fail map/remap/unmap/flush/shootdown on every page and compare logical ranges/PTEs/object refs; W+X denial, stack collision, exact fork credential/mapping copy, exit during block, sanitizer lifetime tests including Skift/Zinnia partial-map and protOS exit-UAF regressions.
- **Target proof:** QEMU starts multiple processes and kills one without desktop impact; hardware receipt records CR3/process identity transitions.
- **Receipt/removal:** mapping manifest and leak counters; process feature flag retains current kernel apps; no removal until P3.4 and Phase 4 gates.

### P3.3 — central usercopy and fault containment

- **Dependencies/current/provenance:** P3.2 and page-fault handler; Serenity fault outcomes and protOS negative evidence; reject raw `strlen/memcpy/VFS` on user pointers.
- **I/O and state:** process plus pointer/span/direction in; copied bytes or typed fault out; `ValidateEntireSpan -> Pin/Copy -> Unpin -> Complete`.
- **Invariants/failure:** checked arithmetic and every page/protection before mutation; bounded strings/slices; validation failure dominates access; copy faults are contained; only the offending process can die; partial output policy explicit and never reported as success.
- **Deterministic proof:** null/noncanonical/cross-page/first-byte-unmapped/last-byte-unmapped/read-only-destination/one-past-VMO/overflow/racing-unmap input and output buffers; fault during copyout; fuzz all syscall shapes; Cyjon/Brook/hhuOS/duckOS regressions.
- **Target proof:** QEMU hostile corpus proves desktop/system and unrelated processes continue after each offender; ThinkPad fault retained without system halt.
- **Receipt/removal:** per-case fault/result and no kernel panic; no unsafe compatibility bypass; direct pointer handlers removed when syscall inventory proves none.

### P3.4 — ELF admission and minimal zl process

- **Dependencies/current/provenance:** P3.1-P3.3 and compiler ELF output; Hyper assertion payload and Brook host shape; Zinnia/TacOS/Cyjon/Fudge/protOS are negative parser inputs; reject magic-only, short entry reads and unchecked offsets.
- **I/O and state:** byte slice plus ABI/manifest in; admitted segment plan/entry or precise rejection out; `Parse -> ValidateAll -> Reserve -> Map/Copy/Zero -> Commit`.
- **Invariants/failure:** magic/class/data/version/machine/header sizes; bounded tables; `filesz <= memsz`; checked offsets/addresses; no overlap/W+X; no mappings before total validation.
- **Deterministic proof:** mutation/fuzz corpus for undersized/zero/wrong entry sizes, count-by-size and offset overflow, `filesz > memsz`, overlap/noncanonical/W+X/bad entry, valid PIE/static fixtures, concurrent exec, OOM rollback, symbol data bounds.
- **Target proof:** assertion process runs/yields/exits; malformed image kills loader request, not desktop.
- **Receipt/removal:** image/plan/mapping digests and rejection code; current embedded/kernel apps remain rollback until process migrations complete.

## Phase 4 — handles, zlIDL, supervision and sessions

Phase exit: bounded versioned IPC survives malformed peers, peer death and restart; children cannot widen rights; sessions authenticate and own process authority.

### P4.1 — typed handle table, derivation and revoke

- **Dependencies/current/provenance:** P3 process object; Serenity/RustOS authority contracts plus NexiOS/Skift as adversarial counterexamples; reject global integer identity and reject treating NexiOS as a working derivation tree.
- **I/O and state:** object, type, rights and optional parent handle in; opaque generation-tagged slot out; `Free -> Reserved -> Live -> Revoking -> Free`.
- **Invariants/failure:** source operation rights and destination insertion rights are checked separately; explicit parent/child graph; child derives from current live parent handles at commit; rights only shrink; spawn/revoke is serialized; stale generation fails; depth and total-node bounds are independent and exhausted before mutation; revoke cannot return success with a reachable descendant; slot/object refs balance.
- **Deterministic proof:** empty/full/one-over, stale reuse, source and destination rights matrix for copy/mint, attempted widening, depth max/max+1, breadth max/max+1 including 9 and 64 child nodes, explicit traversal exhaustion, unrelated aliases, concurrent close/transfer.
- **Target proof:** QEMU processes exchange/revoke shared object; hostile app cannot guess another handle.
- **Receipt/removal:** derivation/revoke trace and leak count; compatibility wrappers map old IDs to restricted handles; remove global IDs at zero callers.

### P4.2 — bounded endpoint and transactional transfer

- **Dependencies/current/provenance:** P4.1 and wait objects; NexiOS/Skift/Lemon; reject Lemon capacity ambiguity and protOS unbounded/UAF IPC.
- **I/O and state:** bounded bytes, handle list, request ID, deadline/cancel in; receive/call/reply/error out; request state plus endpoint `Open -> PeerClosed -> Drained -> Closed`.
- **Invariants/failure:** declared capacity owner; source authority and destination capacity prevalidated; destination slots reserved before source mutation; one journaled commit point; kernel owns envelopes; transfer all-or-nothing; peer death completes waiters; late reply rejected.
- **Deterministic proof:** empty/full/last/one-over and one-way-credit boundaries, receiver-too-small, every nth-handle failure, source/destination rights matrix, peer-close at every stage, cancel race, duplicate reply and rollback; on failure compare byte queues, both handle tables and object refs byte-for-byte; consume-last-message/free-order sanitizer and quotas.
- **Target proof:** QEMU stress between multiple processes and service restart; no desktop crash/leak.
- **Receipt/removal:** queue/latency/outcome/ref counters; old local call adapter remains; remove ambient topics/PID sends after protocol migration.

### P4.3 — zlIDL schema and generated conformance

- **Dependencies/current/provenance:** zl imports/records/errors/exact ints and P4.2; Mollen/Lemon generators; reject handwritten client/server layout drift.
- **I/O and state:** versioned interface schema in; zl client/server, kernel validator, docs, trace decoder and tests out; schema `Draft -> FrozenVersion -> Deprecated -> Removed`.
- **Invariants/failure:** deterministic generation; explicit bounds/rights/deadlines/restart/idempotency; unknown version/method returns typed error.
- **Deterministic proof:** golden generation, cross-version matrix, malformed encoding fuzz, max/over-max, generator self-test and generated failing canary.
- **Target proof:** independently built client/server interoperate in QEMU; trace decoder matches wire bytes.
- **Receipt/removal:** schema/generator/output digests; old version supported through stated window; remove handwritten ABI only after differential parity.

### P4.4 — service supervisor

- **Dependencies/current/provenance:** P3/P4 IPC and clocks; Mollen split, snarkOS lifecycle, RustOS fail-closed; reject silent fake fallback.
- **I/O and state:** service manifest/dependencies/limits/restart policy in; ready/degraded/failed handle and health stream out; common service machine.
- **Invariants/failure:** dependency DAG; bounded restart/backoff; reverse shutdown; crash-loop quarantine; fake success forbidden; authority reissued minimally after restart.
- **Deterministic proof:** dependency failure, crash at each state, restart exhaustion, cyclic graph, stale client handle, shutdown ordering.
- **Target proof:** QEMU kills each migratory service; desktop either recovers or exposes precise degraded state.
- **Receipt/removal:** lifecycle trace, restart count and authority grants; boot can select old in-kernel provider; remove fallback per service only after Phase 5 gate.

### P4.5 — authenticated sessions and process control

- **Dependencies/current/provenance:** P4.1-P4.4 plus persistent credential store; Serenity sessions; banan `setreuid`, Lemon fork/drop/kill, duckOS SHM and signal-branch, Brook/hhuOS privileged-operation failures as negative evidence; reject username-only login/elevation.
- **I/O and state:** credential/auth mechanism and session manifest in; session/process/window/file root handles out; `LoggedOut -> Authenticating -> Active -> Locking/Ending -> LoggedOut`.
- **Invariants/failure:** salted memory-hard password verification with secret zeroing and no secret/digest logging; UID/GID/effective/saved transitions validate atomically against pre-state; credential drops are checked and fail closed; fork copies every credential field exactly; elevation requires explicit authenticated grant; signal including signal 0, debug, raw MMIO, mount/unmount, power, scanout and input-grab are completely mediated by distinct handles; logout revokes descendants.
- **Deterministic proof:** wrong/replay/locked credential; `setreuid(-1,0)` and complete real/effective/saved-ID matrix; failed `setgid` blocks session; fork credential snapshot; ordinary-process denial for signal 0/MMIO/mount/power/scanout/input; session cross-access, revoke on logout/crash, elevation denial.
- **Target proof:** QEMU two sessions cannot access each other's processes/files/windows; hardware login/logout leaves system healthy.
- **Receipt/removal:** redacted auth/session/revoke evidence; single trusted recovery session rollback; remove implicit auto-root only after recovery image works.

## Phase 5 — migrate system services without losing fallbacks

Phase exit: each migrated service has protocol, capability, restart, boundedness, old/new differential, QEMU and relevant hardware proof before old policy is removed.

### P5.1 — Logger and ZLLOG importer/exporter

- **Dependencies/current/provenance:** P4 supervisor/IPC, P2 files and current raw recorder; snarkOS telemetry and ZLLOG design; reject logging content/secrets or storage from IRQ.
- **I/O and state:** structured bounded records/counters and raw journal slots in; filtered streams/files/exports out; `RAMOnly -> RawMounted -> ServiceReady -> Exported`.
- **Invariants/failure:** reserved fault lane; drops counted; privacy schema; bad file service cannot block raw capture; import idempotent by boot/sequence ID.
- **Deterministic proof:** overflow/drop priority, malformed slots, duplicate import, service crash, privacy fixtures, slow/bad disk.
- **Target proof:** QEMU/ThinkPad raw record survives Logger absence/restart and later exports exactly once.
- **Receipt/removal:** raw and exported hash/sequence relation; raw path remains permanent rollback and is never removed.

### P5.2 — File/VFS broker and zlfs provider

- **Dependencies/current/provenance:** P2 zlfs, P4 handles/IDL/supervisor; Mollen filed and Serenity VFS caps; reject global ambient root.
- **I/O and state:** directory/file handle plus bounded path component and operation in; typed file/dir handle or result out; open handle lifecycle.
- **Invariants/failure:** resolution rooted at capability; checked UTF-8/byte policy; no traversal escape; provider restart yields explicit stale/reopen behavior; rename atomic contract.
- **Deterministic proof:** path fuzz, long/NUL/dot components, permissions, concurrent rename/open, provider crash, stale handles, capacity-aware readdir.
- **Target proof:** QEMU and physical Files/editor/browser flows through broker; corrupt provider cannot crash kernel.
- **Receipt/removal:** protocol/provider/artifact and workflow receipts; adapter to old builtins; remove direct FS app calls at zero inventory.

### P5.3 — Window and Input services

- **Dependencies/current/provenance:** Phase 1, P4, P3 app processes; Serenity/Lemon/duckOS/protOS user WM; reject client scanout access and unchecked geometry.
- **I/O and state:** surface/window/input handles, validated geometry/damage and events in/out; window `Created -> Mapped -> Visible/Hidden -> Closing -> Destroyed`; surface generations retire by fence.
- **Invariants/failure:** compositor exclusively owns scanout; client owns only granted surface; geometry/stride/format overflow checked; focus/window ownership enforced; bounded events.
- **Deterministic proof:** malicious dimensions/damage, stale surface, sender ownership, focus/workspace/modal matrix, client crash, compositor restart policy.
- **Target proof:** QEMU process apps render/input; ThinkPad trace preserves Phase 1 latency/visual contracts.
- **Receipt/removal:** scene hashes, event ownership and timing; old hook bridge per app; remove app hook only after workflow parity.

### P5.4 — Audio service and provider ABI

- **Dependencies/current/provenance:** P4 and driver DMA primitives; Serenity/duckOS audio servers, asm.fm probes; reject app-direct hardware and offline WAV as architecture.
- **I/O and state:** stream handle, negotiated format/rate/channels and bounded ring in; mixed provider frames and counters out; stream `Created -> Configured -> Running -> Draining -> Closed/Error`.
- **Invariants/failure:** bounded queue; disconnect cleanup; clipping/resampling defined; underrun/overrun counted; provider failure degrades without blocking clients forever.
- **Deterministic proof:** integer tone/golden properties, format negotiation, mix/clipping, ring wrap, underrun/overrun, client/provider crash, controlled clock latency.
- **Target proof:** QEMU/null/file provider then physical HDA stream/reset/recovery with audio/device receipt.
- **Receipt/removal:** input/output hashes, timing/counters/provider state; silent/null provider rollback; remove direct audio calls after all clients migrate.

### P5.5 — Network/Resolver/TLS service APIs

- **Dependencies/current/provenance:** P4 and current protocol code; Brook host TCP, Serenity services, snarkOS role budgets; reject NIC-global API and one-fetch completion.
- **I/O and state:** network capability/socket/query plus bounds/deadline in; socket/data/DNS/TLS result out; standard socket states and service lifecycle.
- **Invariants/failure:** per-process quotas; no cross-handle access; deterministic timers; resolver/TLS policy explicit; driver restart terminates/rebinds sockets visibly.
- **Deterministic proof:** protocol simulator loss/reorder/duplicate/delay/wrap/congestion, malformed packet fuzz, DNS/TLS failures, quota, service crash.
- **Target proof:** QEMU existing virtio flow via service; physical driver proof in Phase 6.
- **Receipt/removal:** packet/timer/state/cert policy trace; legacy `net_link` adapter; remove direct app stack calls after browser migration.

### P5.6 — restricted parser workers

- **Dependencies/current/provenance:** P3/P4 supervisor and File/Window services; Serenity disposable decoders, protOS TAR/ELF negatives; reject parser in kernel/trusted service.
- **I/O and state:** read-only input handle plus output surface/object handle and limits in; decoded object or typed error out; one-shot worker lifecycle.
- **Invariants/failure:** no ambient file/network/device rights; output bounds declared; crash/fault destroys worker and revokes memory; parser never returns raw internal pointers.
- **Deterministic proof:** mutation/fuzz/sanitizer corpora for image/font/archive/media; output-size bombs; worker kill/restart; nondeterminism check.
- **Target proof:** QEMU browser/files app survives malformed corpus without desktop failure.
- **Receipt/removal:** corpus version, crash/rejection/output hash; in-process parser remains disabled fallback only during migration; remove trusted parser after coverage parity.

### P5.7 — Package service skeleton

- **Dependencies/current/provenance:** P5.2, P4 session/caps, signatures; Mollen images, banan recipes; reject read-only bytes as isolation.
- **I/O and state:** signed package/manifest in; staged verified files, capability request and installed record out; package common state.
- **Invariants/failure:** algorithm/key ID/trust root/policy version/rotation/revocation/anti-rollback and digest/ABI/protocol/dependency checks before commit; staged files/tools/roles/services/grants/handles publish atomically; writable mounts explicit; install cannot grant undeclared rights; uninstall first stops processes, revokes caps and deregisters callbacks, then applies explicit user-data retention policy.
- **Deterministic proof:** local-integrity versus publisher-authenticity, mislabeled algorithm, bad signature/digest/dependency/capability, failure/power cut at every write/registry/persist step, duplicate/install conflict, exact pre/post residue comparison, uninstall with live process, rollback and provider crash.
- **Target proof:** QEMU installs and launches an assertion package under requested handles.
- **Receipt/removal:** manifest/content/transaction/app receipt; uninstall/rollback preserves prior active package; no old bundle removal yet.

## Phase 6 — physical wired networking and browser split

Phase exit: physical carrier, DHCP, DNS, faulted TCP, TLS policy, browser load and persisted download have separate exact receipts; browser/network crashes remain process-local.

### P6.1 — NIC provider selection and resource contract

- **Dependencies/current/provenance:** P5.5, P4 caps and driver descriptor; Zinnia modules, Astral NIC boundary; reject virtio as network API.
- **I/O and state:** PCI/USB resource handles and device descriptor in; link provider handle/mac/carrier/counters out; driver lifecycle plus link `Down -> Negotiating -> Up -> Lost`.
- **Invariants/failure:** explicit DMA/IRQ/BAR ownership; bounded reset/rings; carrier truthful; one selected route policy; device removal completes requests.
- **Deterministic proof:** fake descriptor rings, wrap/full/reset, missing carrier, removal, DMA bounds, provider competition.
- **Target proof:** QEMU virtio; supported USB Ethernet/tether; I219 only with physical carrier/dongle.
- **Receipt/removal:** device IDs/resources/rings/carrier/packet counters; virtio stays fallback; no provider removed merely for adding hardware.

### P6.2 — DHCP/DNS/TCP reliability on physical link

- **Dependencies/current/provenance:** P6.1 and P5.5 simulator; Brook/Astral/Serenity; reject hard-coded QEMU configuration.
- **I/O and state:** carrier and configured policy in; lease/routes/resolvers/reliable byte stream out; protocol states with controlled deadlines.
- **Invariants/failure:** checksums/IDs/sequence windows validated; retransmit and reorder bounded; lease expiry/rebind explicit; no busy wait.
- **Deterministic proof:** packet fault matrix, long response, connection reuse, simultaneous close/reset, lease renew/loss, DNS truncation/failure.
- **Target proof:** same corpus over QEMU and real wired link with packet/state ZLLOG summaries.
- **Receipt/removal:** separate carrier/DHCP/DNS/TCP receipts; static config remains diagnostic fallback; remove default hard-coded path after DHCP success/failure gates.

### P6.3 — TLS/request boundary

- **Dependencies/current/provenance:** P6.2, clocks/cert store and restricted process; Serenity RequestServer; reject silent certificate downgrade.
- **I/O and state:** URL/request handle, policy, deadline in; response stream/metadata or typed TLS/network error out.
- **Invariants/failure:** hostname/time/chain validation; bounded headers/body; redirects/protocols explicit; cancellation closes resources; unsupported feature refuses.
- **Deterministic proof:** known test certificates, wrong host/expired/untrusted/truncated records, large/slow responses, cancellation, parser separation.
- **Target proof:** QEMU and physical HTTPS fetch with exact policy receipt, not body logging.
- **Receipt/removal:** endpoint/cert-policy/result digests; current direct TLS path can be selected for diagnosis; remove only after browser parity.

### P6.4 — browser process split and durable data

- **Dependencies/current/provenance:** P5 Window/File/parsers, P6.3 and current browser; Serenity browser processes; reject one privileged monolith and simulated compatibility.
- **I/O and state:** navigation/user events in; UI/content/request/decoder processes, surfaces and persisted history/bookmark/download out; navigation state explicit.
- **Invariants/failure:** content never gets raw NIC/filesystem/device; download destination is granted handle; process crash isolated; unsupported web feature visible.
- **Deterministic proof:** host render oracles, navigation cancellation, malformed content/decoder crash, process restart, persistence reboot, capability denial.
- **Target proof:** QEMU and physical link render/fetch/download; one process deliberately crashes while desktop/UI recover.
- **Receipt/removal:** scene/document/request/data hashes and process map; current browser remains fallback until workflow parity; remove kernel browser only after all gates.

## Phase 7 — application ecosystem and reproducible packages

Phase exit: isolated apps complete real workflows, signed packages transact safely, and every listed port has build/install/launch evidence.

### P7.1 — stable native app ABI and SDK

- **Dependencies/current/provenance:** P3-P5 and zl exact types/records/errors; Brook/Zinnia conventional coverage; reject stable ABI declared before multiple consumers.
- **I/O and state:** ABI version and generated syscall/service declarations in; SDK/libs/manifests/debug info out; version `Experimental -> Candidate -> Stable -> Deprecated`.
- **Invariants/failure:** size/alignment/calling/wire layouts generated; compatibility matrix explicit; unsupported ABI refuses at load.
- **Deterministic proof:** layout/calling convention, old/new app matrix, symbol/version rejection, SDK sample corpus.
- **Target proof:** independently built apps run on QEMU and hardware image.
- **Receipt/removal:** ABI manifest and sample hashes; compatibility shim rollback; remove old ABI only after deprecation inventory reaches zero.

### P7.2 — ordered application migration

- **Dependencies/current/provenance:** P7.1 and service protocols; TacOS/protOS vertical slices, MaslOS conversations; reject big-bang app rewrite.
- **I/O and state:** current app workflow/oracle in; process app package out; order is assertion -> terminal -> editor/Files -> System Monitor -> Settings -> browser -> rest.
- **Invariants/failure:** visual/workflow parity; save authority explicit; one app crash isolated; no direct kernel/global state after migration.
- **Deterministic proof:** per-app action script, scene hash, input/focus, persistence, crash, resource leak and denied-capability tests.
- **Target proof:** QEMU all apps; ThinkPad primary workflows/latency after each migration.
- **Receipt/removal:** old/new workflow diff and resource/cap map; per-app feature switch; remove hook/global only after exact caller inventory zero.

### P7.3 — transactional package install/upgrade/remove

- **Dependencies/current/provenance:** P5.7, P7.1 and zlfs crash transactions; Mollen packages; reject mutation in place.
- **I/O and state:** signed version/dependencies/caps in; active immutable generation and writable roots out; package state machine.
- **Invariants/failure:** previous version remains active until publish; rollback after any failure/power cut; uninstall cannot delete user data without explicit policy.
- **Deterministic proof:** cut each transition, dependency cycles/conflicts, downgrade policy, active app during upgrade, rollback/uninstall leaks.
- **Target proof:** QEMU cold boots at every injected state; physical install/upgrade/rollback assertion app.
- **Receipt/removal:** transaction/generation/content/app smoke receipts; prior generation retained per policy; delete old generation only after health and rollback window.

### P7.4 — reproducible port recipe and availability gate

- **Dependencies/current/provenance:** P7.3 and toolchain; banan recipes/freestanding header provenance; reject recipe/build as runtime availability.
- **I/O and state:** pinned URL/commit/archive digest, patches, dependency/toolchain digests in; content-addressed package out.
- **Invariants/failure:** no mutable/unverified fetch; patch hash recorded; network optional after source cache; host contamination detected.
- **Deterministic proof:** rebuild comparison, wrong digest, changed patch/dependency, offline cache, install and launch smoke.
- **Target proof:** QEMU app launch/behavior; hardware only when driver/media interaction matters.
- **Receipt/removal:** source/build/package/install/launch chain; failed port remains unlisted; old artifact retained until replacement passes.

### P7.5 — program-driven POSIX/Linux compatibility

- **Dependencies/current/provenance:** P7.1 and selected target program; Brook/Astral/Zinnia; reject syscall-count roadmaps.
- **I/O and state:** program plus observed ABI requirements in; conformance items/shims out.
- **Invariants/failure:** native zlOS semantics stay explicit; error behavior tested; unknown call returns `ENOSYS`; no broad ambient authority introduced.
- **Deterministic proof:** differential pure userspace behavior, syscall/error corpus, selected program regression.
- **Target proof:** QEMU launch and workflow of chosen port; hardware only if program requires it.
- **Receipt/removal:** requirement-to-test-to-program receipt; shims remain isolated; no native API removal.

## Phase 8 — driver depth, audio and Intel composition

Phase exit: every promoted driver completes lifecycle/failure/hardware gates; Intel acceleration improves measured output while software/GOP recovery remains.

### P8.1 — declarative driver descriptor and lifecycle

- **Dependencies/current/provenance:** P4 caps/supervision and P2/P6 provider ABIs; Zinnia/Brook/Sapphire; reject probe-only feature claims.
- **I/O and state:** IDs/dependencies/resources/DMA/IOMMU/operations in; claimed handles/provider/counters out; driver lifecycle common state.
- **Invariants/failure:** resources cannot overlap without policy; each device has a default-deny DMA domain; mappings require a scoped DMA handle and detach/revoke removes translations; passthrough never satisfies isolation; cleanup idempotent; bounded transitions; recovery never publishes fake online.
- **Deterministic proof:** descriptor validation, resource conflict, DMA outside every granted edge, stale/revoked mapping, fail each transition, double stop, reset storm, stale IRQ/completion and IOMMU fault telemetry.
- **Target proof:** QEMU models plus exact physical device receipt including a denied out-of-domain DMA attempt where hardware safely permits.
- **Receipt/removal:** descriptor/resources/state/counters/artifact; old direct driver selectable until same gates; remove only at zero callers and successful fallback drill.

### P8.2 — USB core/class decomposition

- **Dependencies/current/provenance:** P8.1 and current xHCI/HID/MSC; Mollen HID split, Astral USB depth; reject parser/painting in IRQ and unchecked descriptors.
- **I/O and state:** topology/control endpoint and descriptor bytes in; bounded device/class provider handles out; `Detected -> Addressed -> Configured -> Bound -> Removed`.
- **Invariants/failure:** descriptor lengths/features validated; bounded retries/deadlines; class cannot access unrelated controller; unplug cancels and revokes.
- **Deterministic proof:** descriptor fuzz, hub topology, short/error transfers, reset/unplug during I/O, HID reports, MSC sense/residue.
- **Target proof:** QEMU keyboard/mouse/storage; ThinkPad exact xHCI devices and unplug/replug where safe.
- **Receipt/removal:** topology/class/error/lifecycle trace; monolithic path retained behind adapter; remove class logic from core after parity.

### P8.3 — additional storage/NIC/audio providers

- **Dependencies/current/provenance:** P8.1 and block/net/audio contracts; Astral/banan/Serenity; reject breadth before recovery.
- **I/O and state:** AHCI/VirtIO/E1000/I219/HDA resources in; standard provider handles out.
- **Invariants/failure:** each provider passes generic contract plus controller-specific reset/flush/IRQ/DMA rules; unsupported hardware refuses.
- **Deterministic proof:** register/descriptor model, queue wrap, forced timeout/reset, DMA bounds, repeated lifecycle, concurrency.
- **Target proof:** QEMU models; corresponding physical device transcript with carrier/audio/storage effect.
- **Receipt/removal:** generic and device-specific receipts; existing provider fallback retained; no provider removed for coverage vanity.

### P8.4 — Intel render/composition provider

- **Dependencies/current/provenance:** Phase 1 stable scene/lifetimes, P8.1, current Intel modeset/ring and software renderer; RustOS failed GPU receipts and mainstream atomic composition; reject mandatory GPU path.
- **I/O and state:** validated scene commands/surfaces/fences in; completion or reset/fallback out; `Software -> GPUStarting -> GPUReady -> Submitted -> Completed|Hung -> Recovering -> Software`.
- **Invariants/failure:** GOP/software always available; bounded command/ring memory; exact fence ownership; hang triggers recovery; development scene agreement required.
- **Deterministic proof:** command encoder/model, fence wrap, forced hang/reset, stale surface, scene hash corpus, fallback transition.
- **Target proof:** ThinkPad second modeset/teardown and composition timing; QEMU uses software/compatible model only and cannot stand in for Gen9.
- **Receipt/removal:** software/GPU hashes, fences/reset and physical timing; one boot switch disables GPU; software/GOP never removed.

### P8.5 — system driver diagnostics

- **Dependencies/current/provenance:** P5 Logger/System Monitor and P8 providers; Brook hazard ledgers/snarkOS telemetry; reject last-error hidden in debug console.
- **I/O and state:** bounded provider counters/events in; capability-filtered health views and ZLLOG summaries out.
- **Invariants/failure:** no secret/payload logging; counter overflow defined; reads cannot mutate driver; degraded/offline visible.
- **Deterministic proof:** counter/event schema, overflow/reset, permission, provider disappearance, decoder compatibility.
- **Target proof:** injected QEMU and physical failures appear in System Monitor and ZLLOG with matching IDs.
- **Receipt/removal:** schema/event/diagnosis match; old debug output remains diagnostic until consumers migrate.

## Phase 9 — zl-native systems toolchain convergence

Phase exit: exact system semantics agree across interpreter/LLVM/own-native paths; one noncritical zl provider loads and revokes safely; fixpoint remains green.

### P9.1 — imports, scope/errors and data contracts

- **Dependencies/current/provenance:** active zl sequence; zl own roadmap, Lemon/Mollen schema needs; reject OS-only syntax fork.
- **I/O and state:** modules, local/global declarations, result/error values, records/maps/tuples/optionals/enums in; stable AST/semantics across engines out.
- **Invariants/failure:** import identity/cycle policy; local-by-default semantics; annotation neutrality later; errors never silently become sentinel success.
- **Deterministic proof:** interpreter/compiler/self-host differential, module cycles/duplicates, scoping corpus, error propagation, serialization of protocol records.
- **Target proof:** host is primary; QEMU runs generated assertion module when backend supports it.
- **Receipt/removal:** fixpoint and engine-output receipts; old copy-pasted modules remain until all imports resolve; no syntax fork.

### P9.2 — exact integers, bytes and arithmetic modes

- **Dependencies/current/provenance:** P9.1; freestanding headers/Lunaris layouts; reject doubles for addresses and silent overflow.
- **I/O and state:** typed fixed-width values/slices and checked/wrapping operations in; exact bits or typed overflow out.
- **Invariants/failure:** widths/sign/shift semantics identical; 64-bit addresses preserve low bits; bounds checked; byte/string distinction explicit.
- **Deterministic proof:** exhaustive small widths, boundary/property corpus for all widths, interpreter/LLVM/native/C ABI comparison, endian/unaligned helpers.
- **Target proof:** QEMU passes addresses above 2^53 through zl unchanged and validates a descriptor/file/packet fixture.
- **Receipt/removal:** semantic version/corpus hashes and IR/assembly samples; boxed number mode stays compatible; remove double-only system bridges after caller migration.

### P9.3 — native bitwise, volatile MMIO, ports and barriers

- **Dependencies/current/provenance:** P9.2 and typed backend; existing `compilel` measurements, Sapphire protocol discipline; reject boxed name dispatch in drivers.
- **I/O and state:** typed bitwise/volatile/port/barrier operations in; direct IR/machine operations out.
- **Invariants/failure:** no conversion to double/boxed runtime; volatile order preserved; unsafe/danger marker required; target unsupported returns compile error.
- **Deterministic proof:** IR/assembly structural checks, semantic differential, host MMIO mock ordering, barrier litmus models, benchmark rerun.
- **Target proof:** QEMU small noncritical device operation; physical read-only probe before any write.
- **Receipt/removal:** source/IR/object/benchmark and hardware trace; C bridge rollback; remove boxed builtin route for typed calls only after parity.

### P9.4 — layouts, calling conventions and function values

- **Dependencies/current/provenance:** P9.2/types and scoping/closures; Lunaris concept, freestanding headers; reject implicit platform layout.
- **I/O and state:** packed/aligned records, extern/asm symbols, typed function values and ABI declarations in; layout/call manifest out.
- **Invariants/failure:** `size/align/offset` compile-time exact; calling convention explicit; invalid packed/reference combinations reject; callback lifetime owned.
- **Deterministic proof:** compare generated layouts/calls against C/compiler ABI fixtures on supported targets, register preservation, callback/interrupt-table corpus.
- **Target proof:** QEMU calls assembly stub and back; no general user callback executes in IRQ context.
- **Receipt/removal:** ABI manifest/object disassembly; C wrapper remains fallback; remove handwritten layout constants after generated equality.

### P9.5 — ELF64 executable/relocatable and named sections

- **Dependencies/current/provenance:** P9.2-P9.4, current own/LLVM backends and P3 loader; Lunaris mechanism, Hyper assertion; reject file-validity by `file` alone.
- **I/O and state:** zl source, target/ABI/link manifest in; ELF executable/PIE or relocatable object with sections/symbols/relocations out.
- **Invariants/failure:** deterministic sections/symbols; relocations bounded/supported; no CRT when freestanding; undefined extern clear; loader and linker manifests agree.
- **Deterministic proof:** readelf/objdump structural assertions, relocate/link/load fixtures, malformed/unsupported relocation, deterministic rebuild, debug-symbol sanity.
- **Target proof:** QEMU loads minimal app then signed noncritical provider; hardware only after QEMU/host oracle.
- **Receipt/removal:** source/compiler/object/link/image digests; C-generated kernel remains primary; do not remove until Phase 12 rebuild proof.

### P9.6 — first zl provider and fixpoint preservation

- **Dependencies/current/provenance:** P8.1, P9.5 and signatures/caps; Brook load-boundary tests; reject translating Intel/xHCI first.
- **I/O and state:** small non-load-bearing provider plus manifest in; loaded provider handle and behavior out; driver lifecycle.
- **Invariants/failure:** whole image validated before map; only declared imports/resources; unload/revoke cleans all; compiler fixpoint unchanged.
- **Deterministic proof:** C-oracle differential, bad manifest/symbol/relocation, load/unload loops, resource denial, interpreter/LLVM/native compiler gates.
- **Target proof:** QEMU provider effect and recovery; hardware only if harmless/read-only.
- **Receipt/removal:** source/object/load/behavior/unload/fixpoint receipts; C provider remains selectable; remove only after multiple releases and exact parity.

## Phase 10 — capability-native agent runtime

Phase exit: agents are ordinary isolated processes; ungranted actions fail; subagent authority is bounded/revoked; independent review can reject false completion.

### P10.1 — typed plan and deterministic tool broker

- **Dependencies/current/provenance:** P4/P5 services, zlIDL and process isolation; Chitti grammar corrected by RustOS receipts; reject ring-0 orchestrator/unconstrained shell.
- **I/O and state:** intent plus registered tool grammar and granted handles in; immutable typed plan, step results and receipt out; `Draft -> Validated -> Authorized -> Executing -> Complete|Failed|Cancelled`.
- **Invariants/failure:** only registered operations; plan cannot gain handles; arguments/bounds validated before effects; cancellation/rollback declared; every effect attributable.
- **Deterministic proof:** grammar mutation/fuzz, missing/wrong rights, replay, cancellation at each step, partial failure, deterministic plan replay.
- **Target proof:** QEMU agent performs granted file/app task and is denied ungranted network/device/process action.
- **Receipt/removal:** plan/authority/effect/artifact/reviewer digests; manual trusted client remains; broker cannot be bypassed by agent process.

### P10.2 — roles, subagents and resource budgets

- **Dependencies/current/provenance:** P10.1 and supervisor; Chitti attenuation, snarkOS roles, NexiOS bounds; reject universal daemon.
- **I/O and state:** parent plan/grants/deadline/CPU-memory-output limits in; subagent process/result out; process lifecycle plus grant derivation.
- **Invariants/failure:** child grants derive at commit from current live parent handles; spawn/revocation serialized; child subset only; no grant widening or authority resurrection; parent/child/root-orchestrator termination makes the task dead and every descendant handle stale; deadline/cancel/revoke terminal; quota exhaustion explicit.
- **Deterministic proof:** revoke-then-spawn, kill root orchestrator with no orphan root-cap task, widening attempts, nested depth/count, deadline, output/memory/queue exhaustion, parent/child crash and revocation races.
- **Target proof:** QEMU multi-agent task under enforced budgets; desktop remains responsive.
- **Receipt/removal:** role/grant/resource/outcome tree; single-process execution rollback; no unbounded mode in production.

### P10.3 — signed skills/plugins

- **Dependencies/current/provenance:** package service and P10.1; Chitti's separated local-integrity/public-index/P-256 concepts and snarkOS plugin lifecycle; reject Chitti's baked-key MAC mislabeled Ed25519, unfinished foreign ingestion, unsigned dynamic code and crash-propagating callbacks.
- **I/O and state:** signed package, requested ceiling and versioned callbacks in; installed/loaded plugin handle out; package plus service lifecycle.
- **Invariants/failure:** publisher authenticity and package integrity are distinct; user approval; granted intersection only; staged install atomically publishes body/assets/tools/role/index/record or rolls all back; uninstall removes every artifact and registration; bounded callbacks; reverse shutdown; reload unsupported until proved.
- **Deterministic proof:** mislabeled algorithm, local-MAC versus publisher-signature cases, bad signature, excessive grants, failure injection at every install/persist/register step, uninstall residue scan, callback timeout/crash, load/unload loops, version mismatch, dependency failure.
- **Target proof:** QEMU installs/uses/removes harmless skill; supervisor survives plugin fault.
- **Receipt/removal:** package/grant/lifecycle/tool results; disable/unload rollback; old version removed only after no active plan references it.

### P10.4 — provenance, taint and privacy

- **Dependencies/current/provenance:** P10.1 plus Logger/File/Network sinks; Chitti taint/citation with honest limits; reject claim of complete IFC.
- **I/O and state:** labelled values/artifacts and sink policy in; allowed/denied effect plus explanation/citation metadata out.
- **Invariants/failure:** labels cannot be silently dropped; policy version recorded; secrets/private content excluded from logs; ambiguous flow fails closed where policy demands.
- **Deterministic proof:** source-label combinations, transform/merge, declassification approval, file/network/clipboard sinks, log redaction, citation mismatch.
- **Target proof:** QEMU agent cannot exfiltrate labelled local data through ungranted sink.
- **Receipt/removal:** labels/policy/decision without content; manual explicit declassification rollback; never remove privacy boundary.

### P10.5 — independent completion reviewer

- **Dependencies/current/provenance:** universal receipts and P10 plans; RustOS independent evidence; reject executing agent self-certification.
- **I/O and state:** intent/contract/diff/artifacts/receipts in; accept/reject/findings and missing proof out; `Pending -> Challenging -> Accepted|Rejected`.
- **Invariants/failure:** reviewer cannot mutate execution evidence; planted contradiction must be reported; skipped scope remains visible; human override recorded.
- **Deterministic proof:** false success, stale artifact, missing gate, inconsistent hash, model-only proof, planted unauthorized change.
- **Target proof:** QEMU scenario deliberately omits/forges one result and reviewer rejects it.
- **Receipt/removal:** review input/output/model/version; manual review remains fallback; automated review never becomes sole security approval.

## Phase 11 — hardened public demo and operations

Phase exit: concurrent and failure-injected sessions leave no double leases, unauthorized access, orphan VMs/overlays, resource escape or unrecorded artifact identity.

### P11.1 — transactional lease allocator

- **Dependencies/current/provenance:** host service DB/lock and immutable image registry; mission-control concept; reject cache-lock-only multirow state.
- **I/O and state:** requester/rate policy/profile in; opaque lease secret, slot and deadlines out; lease common state.
- **Invariants/failure:** one DB transaction/serialization point; secret stored hashed; slot never double-owned; launch failure returns to clean/quarantined state.
- **Deterministic proof:** concurrent allocate/release/expire, process crash each transition, clock edges, exhausted pool, stale token.
- **Target proof:** real host launches multiple QEMU sessions under stress; no guest trust required.
- **Receipt/removal:** lease-state audit without raw secret; manual admin quarantine rollback; old allocator disabled only after concurrency/failure parity.

### P11.2 — allowlisted QEMU launch and immutable overlays

- **Dependencies/current/provenance:** P11.1, signed base image; mission-control COW; reject extra-argument denylist and hardcoded wrong formats/controllers.
- **I/O and state:** approved profile/base digest/lease ID in; PID identity, overlay, ports and log handles out; `Prepared -> Spawned -> Healthy|Failed`.
- **Invariants/failure:** structured arguments only; base read-only; overlay per lease; accelerator/controller/format compatible; partial artifacts rolled back.
- **Deterministic proof:** every profile, invalid combination, spawn failure, overlay failure, base mismatch, argument injection.
- **Target proof:** host boots exact zlOS digest and guest reports matching identity.
- **Receipt/removal:** argv profile/base/overlay/PID-start/artifact/guest identity; failed launch quarantines; no arbitrary-argument escape hatch.

### P11.3 — per-lease proxy authentication

- **Dependencies/current/provenance:** P11.1/P11.2 and WebSocket/VNC proxy; mission-control negative evidence; reject slot/UUID-only access and passwordless VNC trust.
- **I/O and state:** lease secret/session binding and proxy request in; authorized bounded connection out.
- **Invariants/failure:** secret checked for every new/renewed channel; expiry/release revokes immediately; slot reuse cannot inherit access; origin/rate policy explicit.
- **Deterministic proof:** wrong/stale/replayed/cross-slot token, reconnect, expiry race, slot reuse, brute-rate limit.
- **Target proof:** host browser sessions cannot cross-connect; released lease loses channel.
- **Receipt/removal:** redacted auth/lease/channel events; admin console separate; old unauthenticated route removed immediately after parity.

### P11.4 — resource and egress isolation

- **Dependencies/current/provenance:** P11.2 and host cgroup/seccomp/firewall controls; snarkOS role budgets; reject unrestricted guest user-NAT.
- **I/O and state:** role/profile limits in; isolated QEMU process/network namespace and counters out.
- **Invariants/failure:** CPU/RAM/I/O/PID/disk ceilings; device allowlist; egress default deny/allow policy; limit breach throttles/terminates visibly.
- **Deterministic proof:** CPU/memory/fork/disk/network abuse, forbidden destination/device/syscall, multiple-session fairness.
- **Target proof:** host stress shows bounded impact and correct enforcement.
- **Receipt/removal:** limits/usage/violations/outcome; deny-all diagnostic profile rollback; no unlimited public profile.

### P11.5 — graceful stop, reaper and zero-orphan audit

- **Dependencies/current/provenance:** P11.1-P11.4; mission-control lifecycle gaps/snarkOS reverse shutdown; reject SIGKILL-before-grace and PID-only identity.
- **I/O and state:** expiry/release/admin stop in; confirmed dead process, closed proxy, removed overlay and freed slot out; `Active -> Expiring -> GracefulStop -> ForceStop -> Cleaning -> Free|Quarantined`.
- **Invariants/failure:** verify PID plus start identity; free only after death/cleanup; failures quarantine rather than reuse; reaper idempotent.
- **Deterministic proof:** ignored TERM, PID reuse, unlink failure, proxy hang, host crash/restart, repeated reaper, overlay open handle.
- **Target proof:** host failure campaign ends with zero orphan processes/overlays/ports/double leases.
- **Receipt/removal:** lifecycle/identity/cleanup inventory; quarantine/manual cleanup rollback; old kill/free path removed before public exposure.

## Phase 12 — in-zlOS rebuild, distribution self-hosting and later architectures

Phase exit: zlOS rebuilds matching compiler/system/package artifacts inside itself; rebuilt image passes the boot matrix. Secondary targets earn evidence independently.

### P12.1 — compiler/build tools as isolated zlOS processes

- **Dependencies/current/provenance:** Phase 7 packages, Phase 9 toolchain, processes/files; zl self-host proof and banan build recipes; reject hidden host compiler requirement.
- **I/O and state:** source tree/toolchain manifest/build request in; compiler/object/package artifacts out; build request common state.
- **Invariants/failure:** explicit inputs/environment; bounded resources; no undeclared host files/network; build failure preserves diagnostics and prior artifacts.
- **Deterministic proof:** clean/offline build, missing input/tool, resource exhaustion, parallel dependency order, hermeticity probe.
- **Target proof:** QEMU then hardware zlOS runs compiler/build without host assistance.
- **Receipt/removal:** complete dependency/toolchain/artifact chain; imported bootstrap binary retained; no deletion yet.

### P12.2 — source/package build graph and cache

- **Dependencies/current/provenance:** P12.1 and package/ports; banan dependency hashes, freestanding header generator; reject timestamps/mutable network as hidden input.
- **I/O and state:** target/dependency/source/toolchain digests in; ordered jobs and content-addressed results out; node `Unknown -> Cached|Building -> Passed|Failed`.
- **Invariants/failure:** cycle detection; cache key covers every input; failed node cannot publish; reproducible log ordering/identity.
- **Deterministic proof:** cache hit/miss, changed transitive input, cycle, interrupted build, corrupt cache, parallel determinism.
- **Target proof:** two clean zlOS builds from same inputs produce matching declared artifacts.
- **Receipt/removal:** graph/cache/artifact digests; cache bypass rebuild; no legacy build removal until P12.4.

### P12.3 — compiler fixpoint inside zlOS

- **Dependencies/current/provenance:** P12.1/P12.2 and zl self-host compiler; unix-history provenance; reject fixed historical hash as fixpoint.
- **I/O and state:** compiler source and stage-0 compiler in; gen1/gen2 compiler artifacts plus outputs out.
- **Invariants/failure:** fixpoint means same compiler function reaches byte/semantic equality under declared nondeterminism policy; source changes may change final hash.
- **Deterministic proof:** clean two-generation build, intentional compiler/source perturbation, stage identity, interpreter/LLVM/native semantic corpus.
- **Target proof:** QEMU/hardware in-zlOS fixpoint with no host compilation step.
- **Receipt/removal:** source/stage/gen/toolchain/artifact digests and comparisons; bootstrap compiler retained until independent recovery path.

### P12.4 — rebuild bootable zlOS and pass assertion matrix

- **Dependencies/current/provenance:** P12.2/P12.3, P9 ELF, Phase 0 boot matrix; Hyper assertion boot; reject “build succeeded” as self-host OS proof.
- **I/O and state:** declared system source/package graph in; signed boot image out.
- **Invariants/failure:** shipped inventory matches graph; boot artifact self-identifies; firmware/media/filesystem/failure matrix uses rebuilt artifact; no stale image can pass.
- **Deterministic proof:** image structure, source-to-object manifest, reproducible regions, intentional stale/wrong module/hash failures.
- **Target proof:** QEMU full matrix then ThinkPad boot/ZLLOG/desktop/file/network subset with rebuilt artifact.
- **Receipt/removal:** end-to-end source-to-hardware chain; known-good previous image remains boot fallback; bootstrap C becomes optional only now.

### P12.5 — secondary architecture evidence lane

- **Dependencies/current/provenance:** stable kernel/service/ABI contracts and P12.4; Serenity multiarchitecture build/QEMU-boot CI with x86-64-only target tests, Zinnia honesty, Nyaux/vib claim failures; reject target JSON/linker script as support.
- **I/O and state:** architecture ABI/boot/platform manifest in; separately versioned artifact and evidence lane out.
- **Invariants/failure:** no claim inheritance from x86; unsupported subsystems report precise gaps; source/build/test/runtime/hardware states independent.
- **Deterministic proof:** cross-ABI layouts/calling/endianness, compile inventory, emulator boot/process/service corpus.
- **Target proof:** emulator first, then exact physical machine if support is claimed.
- **Receipt/removal:** architecture-specific toolchain/artifact/gate/hardware receipt; x86 remains primary; no x86 path removed for parity aesthetics.

## Cross-phase dependency and ownership index

| Consumer | Required contracts |
|---|---|
| retained desktop | P0.3, P1.1-P1.4 |
| durable async files | P0.1-P0.4, P2.1-P2.5 |
| isolated apps | P3.1-P3.4, P4.1-P4.5, P5.2-P5.3, P7.1-P7.2 |
| package ecosystem | P2.4, P4.5, P5.7, P7.1, P7.3-P7.5 |
| physical browser | P5.2-P5.6, P6.1-P6.4 |
| GPU desktop | P1.1-P1.4, P8.1, P8.4-P8.5 |
| audio | P4.1-P4.4, P5.4, P8.1/P8.3/P8.5 |
| zl-native provider | P4.1-P4.4, P8.1, P9.1-P9.6 |
| agent platform | P4/P5/P7 services, P10.1-P10.5 |
| public demo | P0 artifact identity, P11.1-P11.5 |
| in-OS self-hosting | P7 packages, P9 toolchain, P12.1-P12.5 |

Ownership rule: one implementation owner per contract and one independent evidence reviewer. A contract may have multiple consumers, but consumers cannot silently weaken its bounds, error semantics, authority, or removal gate.

## Backlog completion rule

A row is complete only when its implementation, deterministic proof, required target proof, receipt, rollback drill, and removal decision are all present. If a hardware gate is genuinely impossible on the available target, the row remains partially complete with that exact gap; QEMU or source review is not promoted to hardware proof.

The immediate executable slice is P0.1-P0.4 only where it does not interrupt shared in-flight work, then P1.1. P1.1 extends the current measured contract and is the first product change. The rest remains dependency-ordered: no IPC before hostile process isolation, no services before bounded IPC, no package claims before crash transactions, no agent authority before capabilities, no public demo before cleanup isolation, and no source-purity deletion before an in-zlOS rebuild boots and proves itself.
