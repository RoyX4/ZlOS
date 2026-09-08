# MP-01: Prove and preserve the current desktop and hardware baseline

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/tests/; kernel/tools/probes/; kernel/docs/guides/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-01` exports: An exact-image regression baseline plus explicit unresolved physical claims; safe host work can proceed.

The handoff enables only its named subset. `CLOSE-01` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-01.01 — Retain the current clean hosted build and BIOS/raw/GRUB/native-UEFI results when source and inputs still match

Retain the current clean hosted build and BIOS/raw/GRUB/native-UEFI results when source and inputs still match.

**Requires:** `D-01`, `D-02`, `H-00`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-01 — Bounded development handoff: Prove and preserve the current desktop and hardware baseline

An exact-image regression baseline plus explicit unresolved physical claims; safe host work can proceed.

**Requires:** `M-01.01`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-01.02 — Record exact firmware, topology, boot medium and fallback paths for each physical qualification profile

Record exact firmware, topology, boot medium and fallback paths for each physical qualification profile.

**Requires:** `M-01.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-01.03 — Prepare a read-only-first ThinkPad test sheet: boot stages, display mode, input devices, files, browser, frame/input journal and recovery

Prepare a read-only-first ThinkPad test sheet: boot stages, display mode, input devices, files, browser, frame/input journal and recovery.

**Requires:** `M-01.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-01.04 — Obtain confirmation for the exact removable target immediately before any flash

Obtain confirmation for the exact removable target immediately before any flash; preserve recovery media and identify the root disk separately.

**Requires:** `M-01.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-01.05 — Boot the exact image physically and retain ZLLOG plus image identity

Boot the exact image physically and retain ZLLOG plus image identity; classify a hardware absence or failure separately from QEMU.

**Requires:** `M-01.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-01.06 — Measure retained client/chrome drawing, damage, glyph work, input age, dropped events and presentation deadlines under real interaction

Measure retained client/chrome drawing, damage, glyph work, input age, dropped events and presentation deadlines under real interaction.

**Requires:** `M-01.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-01.07 — Repair measured owners in bounded changes while replaying app open/close, drag, typing, scrolling and save/reopen journeys

Repair measured owners in bounded changes while replaying app open/close, drag, typing, scrolling and save/reopen journeys.

**Requires:** `M-01.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-01.08 — Keep every unavailable device on the qualification queue with named hardware and expected evidence

Keep every unavailable device on the qualification queue with named hardware and expected evidence; do not delete its destination scope.

**Requires:** `M-01.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [C-P1.1](#c-p1-1) | contract | retained client surfaces |
| [C-P1.2](#c-p1-2) | contract | retained shell/chrome/shadow and desktop layers |
| [C-P1.3](#c-p1-3) | contract | precise invalidation and bounded regions |
| [C-P1.4](#c-p1-4) | contract | atomic WM commit and newest-frame pacing |
| [C-DA-02](#c-da-02) | contract | immutable fallback paths |
| [C-DA-41](#c-da-41) | contract | QEMU system gate |
| [C-DA-42](#c-da-42) | contract | physical hardware gate |
| [C-VX-51](#c-vx-51) | contract | Performance and responsiveness gate |
| [C-VX-53](#c-vx-53) | contract | Physical promotion matrix |
| [T-VM-001](#t-vm-001) | target | QEMU/KVM machine profile |

<a id="c-p1-1"></a>
## C-P1.1 — retained client surfaces

**Original requirement:** retained client surfaces

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 95.

### Preserved original contract

- **Dependencies/current/provenance:** existing WM, 64 MiB heap, direct draw path and [`retained-window-surfaces.md`](../../kernel/docs/plans/retained-window-surfaces.md); Serenity/Lemon/banan buffers; reject blank-on-allocation-failure and raw client pointers.
- **I/O and state:** window/client dimensions and explicit app invalidation in; bounded RGB32 surface plus generation/validity out; state `None -> Allocated/Invalid -> Rendering -> Valid -> Retiring -> None`.
- **Invariants/failure:** overflow checked; 48 MiB initial aggregate budget; target/clip restored on every exit; allocation refusal uses current direct renderer; free occurs after compositor retirement.
- **Deterministic proof:** byte-identical direct/surface draw, allocation refusal, resize/minimize/close, repeated lifecycle heap audit, invalidation generation race.
- **Target proof:** QEMU boot/drag; physical trace shows zero `hook_draw` for unchanged covered app and no ZLLOG drops.
- **Receipt/removal:** scene hashes, allocation counters and trace; disable switch returns to direct rendering; do not remove direct path.

### Execution steps

- [ ] **C-P1.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P1.1.02 — Resolve this contract's exact dependencies**
  - Action: existing WM, 64 MiB heap, direct draw path and [`retained-window-surfaces.md`](../../kernel/docs/plans/retained-window-surfaces.md); Serenity/Lemon/banan buffers; reject blank-on-allocation-failure and raw client pointers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P1.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P1.1.03 — I/O and state — retained client surfaces**
  - Action: window/client dimensions and explicit app invalidation in; bounded RGB32 surface plus generation/validity out; state `None -> Allocated/Invalid -> Rendering -> Valid -> Retiring -> None`.
  - Requires: C-P1.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P1.1.
- [ ] **C-P1.1.04 — Invariants/failure — retained client surfaces**
  - Action: overflow checked; 48 MiB initial aggregate budget; target/clip restored on every exit; allocation refusal uses current direct renderer; free occurs after compositor retirement.
  - Requires: C-P1.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P1.1.
- [ ] **C-P1.1.05 — Deterministic proof — retained client surfaces**
  - Action: byte-identical direct/surface draw, allocation refusal, resize/minimize/close, repeated lifecycle heap audit, invalidation generation race.
  - Requires: C-P1.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P1.1.
- [ ] **C-P1.1.06 — Target proof — retained client surfaces**
  - Action: QEMU boot/drag; physical trace shows zero `hook_draw` for unchanged covered app and no ZLLOG drops.
  - Requires: C-P1.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P1.1.
- [ ] **C-P1.1.07 — Receipt/removal — retained client surfaces**
  - Action: scene hashes, allocation counters and trace; disable switch returns to direct rendering; do not remove direct path.
  - Requires: C-P1.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P1.1.
- [ ] **C-P1.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P1.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P1.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p1-2"></a>
## C-P1.2 — retained shell/chrome/shadow and desktop layers

**Original requirement:** retained shell/chrome/shadow and desktop layers

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 104.

### Preserved original contract

- **Dependencies/current/provenance:** P1.1 and measured chrome/desktop phase data; DirectComposition/Core Animation and Serenity boundaries; reject client-only cache called complete.
- **I/O and state:** geometry/focus/hover/title/theme/dock changes in; separately versioned shell/shadow/desktop layers out; each layer has `Invalid -> Rendering -> Valid`.
- **Invariants/failure:** another window's move cannot invalidate stable shell/client; focus/theme changes invalidate only required layers; memory accounting is separate.
- **Deterministic proof:** move/raise/focus/hover/theme/workspace matrix with draw counters and scene hashes; eviction/refusal fallback.
- **Target proof:** physical phase trace materially removes the measured chrome owner and reports desktop cache budget.
- **Receipt/removal:** before/after phase journal and layer-hit counters; per-layer feature flags roll back independently; direct chrome remains until parity.

### Execution steps

- [ ] **C-P1.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P1.2.02 — Resolve this contract's exact dependencies**
  - Action: P1.1 and measured chrome/desktop phase data; DirectComposition/Core Animation and Serenity boundaries; reject client-only cache called complete. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P1.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P1.2.03 — I/O and state — retained shell/chrome/shadow and desktop layers**
  - Action: geometry/focus/hover/title/theme/dock changes in; separately versioned shell/shadow/desktop layers out; each layer has `Invalid -> Rendering -> Valid`.
  - Requires: C-P1.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P1.2.
- [ ] **C-P1.2.04 — Invariants/failure — retained shell/chrome/shadow and desktop layers**
  - Action: another window's move cannot invalidate stable shell/client; focus/theme changes invalidate only required layers; memory accounting is separate.
  - Requires: C-P1.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P1.2.
- [ ] **C-P1.2.05 — Deterministic proof — retained shell/chrome/shadow and desktop layers**
  - Action: move/raise/focus/hover/theme/workspace matrix with draw counters and scene hashes; eviction/refusal fallback.
  - Requires: C-P1.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P1.2.
- [ ] **C-P1.2.06 — Target proof — retained shell/chrome/shadow and desktop layers**
  - Action: physical phase trace materially removes the measured chrome owner and reports desktop cache budget.
  - Requires: C-P1.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P1.2.
- [ ] **C-P1.2.07 — Receipt/removal — retained shell/chrome/shadow and desktop layers**
  - Action: before/after phase journal and layer-hit counters; per-layer feature flags roll back independently; direct chrome remains until parity.
  - Requires: C-P1.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P1.2.
- [ ] **C-P1.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P1.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P1.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p1-3"></a>
## C-P1.3 — precise invalidation and bounded regions

**Original requirement:** precise invalidation and bounded regions

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 113.

### Preserved original contract

- **Dependencies/current/provenance:** P1.1; current damage list; Serenity disjoint regions and Linux DRM semantics; reject bounding touching/L-shaped rectangles blindly.
- **I/O and state:** app invalidation, screen damage and occlusion geometry in; bounded disjoint/area-aware region plus fallback reason out.
- **Invariants/failure:** all changed pixels included; no false exclusion; merge-waste threshold explicit; fragmentation cap causes counted full damage.
- **Deterministic proof:** L shape, touching chain, partial occlusion, transparency, screen edges, overflow, full-list fallback, randomized oracle against pixel mask.
- **Target proof:** QEMU scene suite; physical journal shows reduced damage pixels/window visits without stale pixels.
- **Receipt/removal:** region corpus hash and telemetry; switch restores old conservative full/bounding damage; remove old merger after visual/property parity.

### Execution steps

- [ ] **C-P1.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P1.3.02 — Resolve this contract's exact dependencies**
  - Action: P1.1; current damage list; Serenity disjoint regions and Linux DRM semantics; reject bounding touching/L-shaped rectangles blindly. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P1.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P1.3.03 — I/O and state — precise invalidation and bounded regions**
  - Action: app invalidation, screen damage and occlusion geometry in; bounded disjoint/area-aware region plus fallback reason out.
  - Requires: C-P1.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P1.3.
- [ ] **C-P1.3.04 — Invariants/failure — precise invalidation and bounded regions**
  - Action: all changed pixels included; no false exclusion; merge-waste threshold explicit; fragmentation cap causes counted full damage.
  - Requires: C-P1.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P1.3.
- [ ] **C-P1.3.05 — Deterministic proof — precise invalidation and bounded regions**
  - Action: L shape, touching chain, partial occlusion, transparency, screen edges, overflow, full-list fallback, randomized oracle against pixel mask.
  - Requires: C-P1.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P1.3.
- [ ] **C-P1.3.06 — Target proof — precise invalidation and bounded regions**
  - Action: QEMU scene suite; physical journal shows reduced damage pixels/window visits without stale pixels.
  - Requires: C-P1.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P1.3.
- [ ] **C-P1.3.07 — Receipt/removal — precise invalidation and bounded regions**
  - Action: region corpus hash and telemetry; switch restores old conservative full/bounding damage; remove old merger after visual/property parity.
  - Requires: C-P1.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P1.3.
- [ ] **C-P1.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P1.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P1.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p1-4"></a>
## C-P1.4 — atomic WM commit and newest-frame pacing

**Original requirement:** atomic WM commit and newest-frame pacing

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 122.

### Preserved original contract

- **Dependencies/current/provenance:** P1.1-P1.3, current input/frame telemetry; mainstream compositor deadline concepts and NexiOS boundedness; reject blanket realtime and deep visual queues.
- **I/O and state:** input/app mutations and layer generations in; one atomic scene commit, predicted deadline and at most one pending visual frame out; `Idle -> Collecting -> Committed -> Composing -> Presented|DroppedAsStale`.
- **Invariants/failure:** newest state wins; no partially visible metadata; HID path does not paint; missed deadline and queue depth counted; idle returns to `hlt`.
- **Deterministic proof:** controlled clock, event burst/coalescing, commit atomicity, deadline edge, stale-frame replacement, idle wake.
- **Target proof:** QEMU causal trace; ThinkPad HID-to-route p95 below 1 ms, no visible frame above current 16.67 ms contract, `late=0`, `lost=0` for scripted run.
- **Receipt/removal:** percentile/raw trace and commit counters; rollback selects old frame loop; remove old scheduling only after physical parity.

### Execution steps

- [ ] **C-P1.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P1.4.02 — Resolve this contract's exact dependencies**
  - Action: P1.1-P1.3, current input/frame telemetry; mainstream compositor deadline concepts and NexiOS boundedness; reject blanket realtime and deep visual queues. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P1.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P1.4.03 — I/O and state — atomic WM commit and newest-frame pacing**
  - Action: input/app mutations and layer generations in; one atomic scene commit, predicted deadline and at most one pending visual frame out; `Idle -> Collecting -> Committed -> Composing -> Presented|DroppedAsStale`.
  - Requires: C-P1.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P1.4.
- [ ] **C-P1.4.04 — Invariants/failure — atomic WM commit and newest-frame pacing**
  - Action: newest state wins; no partially visible metadata; HID path does not paint; missed deadline and queue depth counted; idle returns to `hlt`.
  - Requires: C-P1.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P1.4.
- [ ] **C-P1.4.05 — Deterministic proof — atomic WM commit and newest-frame pacing**
  - Action: controlled clock, event burst/coalescing, commit atomicity, deadline edge, stale-frame replacement, idle wake.
  - Requires: C-P1.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P1.4.
- [ ] **C-P1.4.06 — Target proof — atomic WM commit and newest-frame pacing**
  - Action: QEMU causal trace; ThinkPad HID-to-route p95 below 1 ms, no visible frame above current 16.67 ms contract, `late=0`, `lost=0` for scripted run.
  - Requires: C-P1.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P1.4.
- [ ] **C-P1.4.07 — Receipt/removal — atomic WM commit and newest-frame pacing**
  - Action: percentile/raw trace and commit counters; rollback selects old frame loop; remove old scheduling only after physical parity.
  - Requires: C-P1.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P1.4.
- [ ] **C-P1.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P1.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P1.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-02"></a>
## C-DA-02 — immutable fallback paths

**Original requirement:** immutable fallback paths

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 78.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-02.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-02.02 — Resolve this contract's exact dependencies**
  - Action: GOP/software framebuffer, ZLLOG, PS/2 input, current NVMe/xHCI MSC and zlfs read/write path. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-02.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-02.03 — Deliver — immutable fallback paths**
  - Action: named fallback providers and a boot-safe selection policy. Keep them available while accelerated or isolated replacements are introduced.
  - Requires: C-DA-02.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-02.
- [ ] **C-DA-02.04 — Invariants — immutable fallback paths**
  - Action: a failed optional provider cannot remove the last display, log, input or boot-storage path; fallback selection is visible; persistent data format does not change implicitly.
  - Requires: C-DA-02.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-02.
- [ ] **C-DA-02.05 — Proof — immutable fallback paths**
  - Action: fail each accelerated provider at every startup step and assert that the fallback remains usable; recover ZLLOG after a desktop crash; read existing zlfs data before and after provider wrapping.
  - Requires: C-DA-02.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-02.
- [ ] **C-DA-02.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-02. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-02.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-41"></a>
## C-DA-41 — QEMU system gate

**Original requirement:** QEMU system gate

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 915.

### Preserved original contract

Boot a fresh artifact nonce; start/reset/stop/restart providers; install/launch/
interact/close/crash/restart apps; kill/reconnect services; inject malformed data,
timeouts and exhaustion. A screenshot needs a semantic ready/result oracle.

### Execution steps

- [ ] **C-DA-41.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-41.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-41.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-41.03 — Execute the preserved contract requirements**
  - Action: Boot a fresh artifact nonce; start/reset/stop/restart providers; install/launch/ interact/close/crash/restart apps; kill/reconnect services; inject malformed data, timeouts and exhaustion. A screenshot needs a semantic ready/result oracle.
  - Requires: C-DA-41.02.
  - Acceptance: Every requirement in the original contract has an independent observable result.
- [ ] **C-DA-41.04 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-41. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-41.03.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-42"></a>
## C-DA-42 — physical hardware gate

**Original requirement:** physical hardware gate

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 921.

### Preserved original contract

Record machine, firmware, exact PCI/USB identity, artifact, selected/fallback
provider, request/effect/completion, safe forced recovery, repeated start/stop/cold
boot and post-workload data/device integrity. One machine proves only that profile.

### Execution steps

- [ ] **C-DA-42.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-42.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-42.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-42.03 — Execute the preserved contract requirements**
  - Action: Record machine, firmware, exact PCI/USB identity, artifact, selected/fallback provider, request/effect/completion, safe forced recovery, repeated start/stop/cold boot and post-workload data/device integrity. One machine proves only that profile.
  - Requires: C-DA-42.02.
  - Acceptance: Every requirement in the original contract has an independent observable result.
- [ ] **C-DA-42.04 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-42. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-42.03.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-51"></a>
## C-VX-51 — Performance and responsiveness gate

**Original requirement:** Performance and responsiveness gate

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 766.

### Preserved original contract

**Depends on:** VX-12, VX-18, VX-33.

**Deliver:** stage timing for input, IPC, app, scene, raster, composition,
present, browser parse/layout/paint; median/p95/p99/max and missed/lost counts.

**Proof:** supported resolution/scale/backend matrix, app/browser load, damaged-
area extremes, provider restart and effect degradation without input failure.

### Execution steps

- [ ] **C-VX-51.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-51.02 — Resolve this contract's exact dependencies**
  - Action: VX-12, VX-18, VX-33. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-51.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-51.03 — Deliver — Performance and responsiveness gate**
  - Action: stage timing for input, IPC, app, scene, raster, composition, present, browser parse/layout/paint; median/p95/p99/max and missed/lost counts.
  - Requires: C-VX-51.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-51.
- [ ] **C-VX-51.04 — Proof — Performance and responsiveness gate**
  - Action: supported resolution/scale/backend matrix, app/browser load, damaged- area extremes, provider restart and effect degradation without input failure.
  - Requires: C-VX-51.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-51.
- [ ] **C-VX-51.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-51. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-51.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-53"></a>
## C-VX-53 — Physical promotion matrix

**Original requirement:** Physical promotion matrix

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 786.

### Preserved original contract

**Depends on:** VX-51, VX-52, driver/hardware contracts.

**Deliver:** named hardware profiles for display, keyboard, pointer, storage,
network, audio, power and multi-monitor; native artifact/effect receipts.

**Invariants:** host harness does not count as native boot; controller proof does
not promote every class; fallback remains available; DMA isolation state is
honest when no IOMMU exists.

**Proof:** cold/repeated boot, exact user journey, unplug/replug/reset/suspend,
provider recovery, durable log and physical effect evidence.

### Execution steps

- [ ] **C-VX-53.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-53.02 — Resolve this contract's exact dependencies**
  - Action: VX-51, VX-52, driver/hardware contracts. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-53.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-53.03 — Deliver — Physical promotion matrix**
  - Action: named hardware profiles for display, keyboard, pointer, storage, network, audio, power and multi-monitor; native artifact/effect receipts.
  - Requires: C-VX-53.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-53.
- [ ] **C-VX-53.04 — Invariants — Physical promotion matrix**
  - Action: host harness does not count as native boot; controller proof does not promote every class; fallback remains available; DMA isolation state is honest when no IOMMU exists.
  - Requires: C-VX-53.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-53.
- [ ] **C-VX-53.05 — Proof — Physical promotion matrix**
  - Action: cold/repeated boot, exact user journey, unplug/replug/reset/suspend, provider recovery, durable log and physical effect evidence.
  - Requires: C-VX-53.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-53.
- [ ] **C-VX-53.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-53. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-53.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-vm-001"></a>
## T-VM-001 — QEMU/KVM machine profile

**Original requirement:** exact machine/device identity and gates

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 258.

### Execution steps

- [ ] **T-VM-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve QEMU/KVM machine profile to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, H-01.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-001.02 — Specify the complete target boundary**
  - Action: QEMU/KVM machine profile must supply: exact machine/device identity and gates. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse QEMU/KVM machine profile through the shared platform contract, delivering every part of: exact machine/device identity and gates. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-001.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for QEMU/KVM machine profile.
- [ ] **T-VM-001.05 — Qualify and retain this target's own result**
  - Action: Bind QEMU/KVM machine profile to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
