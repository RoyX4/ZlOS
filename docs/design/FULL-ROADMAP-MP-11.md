# MP-11: Give applications a complete runtime and shared services

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/apps/; kernel/src/graphics/; kernel/src/core/; stdlib/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-11` exports: A stable app ABI/SDK, launch lifecycle and minimal file/settings/notification portals for the first isolated app.

The handoff enables only its named subset. `CLOSE-11` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-11.01 — Freeze process ABI, SDK ownership, generated manifests, identity, resources and compatibility/version policy

Freeze process ABI, SDK ownership, generated manifests, identity, resources and compatibility/version policy.

**Requires:** `D-01`, `D-02`, `D-09`, `D-14`, `D-17`, `H-00`, `H-03`, `H-04`, `H-06`, `H-07`, `H-10`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.02 — Build admitted registry, launcher and handler resolution with no blank or unreachable catalogue entries

Build admitted registry, launcher and handler resolution with no blank or unreachable catalogue entries.

**Requires:** `M-11.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.03 — Implement create/start/ready/background/stop/crash/restart/update/remove and session-restore transitions

Implement create/start/ready/background/stop/crash/restart/update/remove and session-restore transitions.

**Requires:** `M-11.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.04 — Provide Files/Open/Save/Settings/Clipboard/Notifications/Share/Print/Secrets and device portals with bounded grants

Provide Files/Open/Save/Settings/Clipboard/Notifications/Share/Print/Secrets and device portals with bounded grants.

**Requires:** `M-11.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-11 — Bounded development handoff: Give applications a complete runtime and shared services

A stable app ABI/SDK, launch lifecycle and minimal file/settings/notification portals for the first isolated app.

**Requires:** `M-11.04`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-11.05 — Define document ownership, undo/redo, autosave, recovery and durable close behavior

Define document ownership, undo/redo, autosave, recovery and durable close behavior.

**Requires:** `M-11.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.06 — Add MIME, thumbnail, search/indexing, spelling and restricted decoder services through the same lifecycle contract

Add MIME, thumbnail, search/indexing, spelling and restricted decoder services through the same lifecycle contract.

**Requires:** `M-11.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.07 — Build a small real app against the SDK and use it to prove keyboard, semantics, scaling, persistence and portal denial

Build a small real app against the SDK and use it to prove keyboard, semantics, scaling, persistence and portal denial.

**Requires:** `M-11.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.08 — Exercise package updates, service restart, crash loops, resource limits and revoke while requests are in flight

Exercise package updates, service restart, crash loops, resource limits and revoke while requests are in flight.

**Requires:** `M-11.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-11.09 — Keep old app routes available until each process replacement preserves its complete workflow

Keep old app routes available until each process replacement preserves its complete workflow.

**Requires:** `M-11.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-AP-001](#f-ap-001) | feature | ApplicationManifest |
| [F-AP-002](#f-ap-002) | feature | app registry |
| [F-AP-003](#f-ap-003) | feature | app launch |
| [F-AP-004](#f-ap-004) | feature | app lifecycle |
| [F-AP-005](#f-ap-005) | feature | app supervision |
| [F-AP-006](#f-ap-006) | feature | app sandbox |
| [F-AP-007](#f-ap-007) | feature | app resource budgets |
| [F-AP-008](#f-ap-008) | feature | app data directories |
| [F-AP-009](#f-ap-009) | feature | settings service |
| [F-AP-010](#f-ap-010) | feature | secrets service |
| [F-AP-011](#f-ap-011) | feature | portal service |
| [F-AP-012](#f-ap-012) | feature | MIME/association service |
| [F-AP-013](#f-ap-013) | feature | localization service |
| [F-AP-014](#f-ap-014) | feature | font service |
| [F-AP-015](#f-ap-015) | feature | image decoder service |
| [F-AP-016](#f-ap-016) | feature | media decoder service |
| [F-AP-017](#f-ap-017) | feature | thumbnail service |
| [F-AP-018](#f-ap-018) | feature | search/index service |
| [F-AP-019](#f-ap-019) | feature | notification API |
| [F-AP-020](#f-ap-020) | feature | clipboard API |
| [F-AP-021](#f-ap-021) | feature | file API |
| [F-AP-022](#f-ap-022) | feature | network request API |
| [F-AP-023](#f-ap-023) | feature | audio API |
| [F-AP-024](#f-ap-024) | feature | camera/microphone API |
| [F-AP-025](#f-ap-025) | feature | print API |
| [F-AP-026](#f-ap-026) | feature | background-task API |
| [F-AP-027](#f-ap-027) | feature | app update/migration |
| [F-AP-028](#f-ap-028) | feature | app store/catalog |
| [F-AP-029](#f-ap-029) | feature | developer mode/sideload |
| [F-AP-030](#f-ap-030) | feature | app backup/export |
| [F-AP-031](#f-ap-031) | feature | document model |
| [F-AP-032](#f-ap-032) | feature | undo/redo service pattern |
| [F-AP-033](#f-ap-033) | feature | command/action registry |
| [F-AP-034](#f-ap-034) | feature | plugin/extension model |
| [F-AP-035](#f-ap-035) | feature | app evidence page |
| [F-AP-036](#f-ap-036) | feature | online-account broker |
| [F-AP-037](#f-ap-037) | feature | sync service |
| [F-AP-038](#f-ap-038) | feature | background transfer service |
| [F-AP-039](#f-ap-039) | feature | location service |
| [F-AP-040](#f-ap-040) | feature | sensor service |
| [C-P5.6](#c-p5-6) | contract | restricted parser workers |
| [C-P7.1](#c-p7-1) | contract | stable native app ABI and SDK |
| [C-DA-20C](#c-da-20c) | contract | settings and configuration service |
| [C-DA-21F](#c-da-21f) | contract | file operation and safe-open portal |
| [C-DA-23](#c-da-23) | contract | application manifest and stable identity |
| [C-DA-23L](#c-da-23l) | contract | launch and handler service |
| [C-DA-25](#c-da-25) | contract | application lifecycle and crash semantics |
| [C-DA-25N](#c-da-25n) | contract | notification and crash services |
| [C-VX-15](#c-vx-15) | contract | Notifications, quick settings and background tasks |
| [C-VX-16](#c-vx-16) | contract | Permission portals and chooser patterns |
| [C-VX-18](#c-vx-18) | contract | Application lifecycle, crash and restore |
| [C-VX-19](#c-vx-19) | contract | App catalogue/search/provenance |
| [T-SVC-038](#t-svc-038) | target | MIME/Type Registry |
| [T-SVC-039](#t-svc-039) | target | Thumbnail Service |
| [T-SVC-040](#t-svc-040) | target | Search/Index Service |
| [T-SVC-044](#t-svc-044) | target | File Portal |
| [T-SVC-045](#t-svc-045) | target | Archive Worker |
| [T-SVC-076](#t-svc-076) | target | Application Registry/Launcher |
| [T-SVC-100](#t-svc-100) | target | Settings Service |
| [T-SVC-101](#t-svc-101) | target | Clipboard Broker |
| [T-SVC-102](#t-svc-102) | target | Notification Service |
| [T-SVC-103](#t-svc-103) | target | Open/Save/Share Portal |
| [T-SVC-106](#t-svc-106) | target | Document/Autosave Service |
| [T-SVC-107](#t-svc-107) | target | Undo/History Service Library |
| [T-SVC-114](#t-svc-114) | target | Background Task Scheduler |
| [T-SVC-115](#t-svc-115) | target | Application Restore Service |

<a id="f-ap-001"></a>
## F-AP-001 — ApplicationManifest

**Original requirement:** identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-001.01 — Reconcile existing ApplicationManifest**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ApplicationManifest. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature
- [ ] **F-AP-001.02 — Freeze the exact contract for ApplicationManifest**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-001.03 — Implement/prove: identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature**
  - Action: For ApplicationManifest, implement or reuse and verify this exact obligation: identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identity/version/vendor/arch/ABI/files/dependencies/capabilities/services/licenses/signature; retain observable state/resource expectations.
- [ ] **F-AP-001.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ApplicationManifest: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-001.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-001.05 — Integrate into the real consumer and runtime route**
  - Action: Wire ApplicationManifest into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-001.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-001.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ApplicationManifest as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-001.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-002"></a>
## F-AP-002 — app registry

**Original requirement:** generated exact package/build/image/route metadata with no blank/dead IDs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-002.01 — Reconcile existing app registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated exact package/build/image/route metadata with no blank/dead IDs
- [ ] **F-AP-002.02 — Freeze the exact contract for app registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated exact package/build/image/route metadata with no blank/dead IDs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-002.03 — Implement/prove: generated exact package/build/image/route metadata with no blank/dead IDs**
  - Action: For app registry, implement or reuse and verify this exact obligation: generated exact package/build/image/route metadata with no blank/dead IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated exact package/build/image/route metadata with no blank/dead IDs; retain observable state/resource expectations.
- [ ] **F-AP-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app registry: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire app registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-003"></a>
## F-AP-003 — app launch

**Original requirement:** validate, allocate process/handles, publish, ready nonce or exact rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-003.01 — Reconcile existing app launch**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app launch. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validate, allocate process/handles, publish, ready nonce or exact rollback
- [ ] **F-AP-003.02 — Freeze the exact contract for app launch**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validate, allocate process/handles, publish, ready nonce or exact rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-003.03 — Implement/prove: validate**
  - Action: For app launch, implement or reuse and verify this exact obligation: validate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validate; retain observable state/resource expectations.
- [ ] **F-AP-003.04 — Implement/prove: allocate process/handles**
  - Action: For app launch, implement or reuse and verify this exact obligation: allocate process/handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allocate process/handles; retain observable state/resource expectations.
- [ ] **F-AP-003.05 — Implement/prove: publish**
  - Action: For app launch, implement or reuse and verify this exact obligation: publish. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for publish; retain observable state/resource expectations.
- [ ] **F-AP-003.06 — Implement/prove: ready nonce or exact rollback**
  - Action: For app launch, implement or reuse and verify this exact obligation: ready nonce or exact rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ready nonce or exact rollback; retain observable state/resource expectations.
- [ ] **F-AP-003.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app launch: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-003.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-003.08 — Integrate into the real consumer and runtime route**
  - Action: Wire app launch into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-003.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-003.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app launch as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-003.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-004"></a>
## F-AP-004 — app lifecycle

**Original requirement:** staged/running/background/suspended/stopping/crashed/quarantined with deadlines

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-004.01 — Reconcile existing app lifecycle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app lifecycle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: staged/running/background/suspended/stopping/crashed/quarantined with deadlines
- [ ] **F-AP-004.02 — Freeze the exact contract for app lifecycle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: staged/running/background/suspended/stopping/crashed/quarantined with deadlines. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-004.03 — Implement/prove: staged/running/background/suspended/stopping/crashed/quarantined with deadlines**
  - Action: For app lifecycle, implement or reuse and verify this exact obligation: staged/running/background/suspended/stopping/crashed/quarantined with deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for staged/running/background/suspended/stopping/crashed/quarantined with deadlines; retain observable state/resource expectations.
- [ ] **F-AP-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app lifecycle: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire app lifecycle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app lifecycle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-005"></a>
## F-AP-005 — app supervision

**Original requirement:** health, crash budget/backoff, cleanup, relaunch and user-visible state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-005.01 — Reconcile existing app supervision**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app supervision. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: health, crash budget/backoff, cleanup, relaunch and user-visible state
- [ ] **F-AP-005.02 — Freeze the exact contract for app supervision**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: health, crash budget/backoff, cleanup, relaunch and user-visible state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-005.03 — Implement/prove: health**
  - Action: For app supervision, implement or reuse and verify this exact obligation: health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for health; retain observable state/resource expectations.
- [ ] **F-AP-005.04 — Implement/prove: crash budget/backoff**
  - Action: For app supervision, implement or reuse and verify this exact obligation: crash budget/backoff. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crash budget/backoff; retain observable state/resource expectations.
- [ ] **F-AP-005.05 — Implement/prove: cleanup**
  - Action: For app supervision, implement or reuse and verify this exact obligation: cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cleanup; retain observable state/resource expectations.
- [ ] **F-AP-005.06 — Implement/prove: relaunch and user-visible state**
  - Action: For app supervision, implement or reuse and verify this exact obligation: relaunch and user-visible state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for relaunch and user-visible state; retain observable state/resource expectations.
- [ ] **F-AP-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app supervision: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire app supervision into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app supervision as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-006"></a>
## F-AP-006 — app sandbox

**Original requirement:** manifest baseline, portal mediation, monotonic reduction and hostile tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-006.01 — Reconcile existing app sandbox**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app sandbox. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: manifest baseline, portal mediation, monotonic reduction and hostile tests
- [ ] **F-AP-006.02 — Freeze the exact contract for app sandbox**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: manifest baseline, portal mediation, monotonic reduction and hostile tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-006.03 — Implement/prove: manifest baseline**
  - Action: For app sandbox, implement or reuse and verify this exact obligation: manifest baseline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for manifest baseline; retain observable state/resource expectations.
- [ ] **F-AP-006.04 — Implement/prove: portal mediation**
  - Action: For app sandbox, implement or reuse and verify this exact obligation: portal mediation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for portal mediation; retain observable state/resource expectations.
- [ ] **F-AP-006.05 — Implement/prove: monotonic reduction and hostile tests**
  - Action: For app sandbox, implement or reuse and verify this exact obligation: monotonic reduction and hostile tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for monotonic reduction and hostile tests; retain observable state/resource expectations.
- [ ] **F-AP-006.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app sandbox: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-006.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-006.07 — Integrate into the real consumer and runtime route**
  - Action: Wire app sandbox into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-006.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-006.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app sandbox as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-006.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-007"></a>
## F-AP-007 — app resource budgets

**Original requirement:** memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-007.01 — Reconcile existing app resource budgets**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app resource budgets. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced
- [ ] **F-AP-007.02 — Freeze the exact contract for app resource budgets**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-007.03 — Implement/prove: memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced**
  - Action: For app resource budgets, implement or reuse and verify this exact obligation: memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for memory/CPU/threads/handles/IPC/surfaces/files/network/log/time enforced; retain observable state/resource expectations.
- [ ] **F-AP-007.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app resource budgets: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-007.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-007.05 — Integrate into the real consumer and runtime route**
  - Action: Wire app resource budgets into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-007.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-007.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app resource budgets as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-007.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-008"></a>
## F-AP-008 — app data directories

**Original requirement:** package/read-only, per-user config/data/cache/state/temp with backup policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-008.01 — Reconcile existing app data directories**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app data directories. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: package/read-only, per-user config/data/cache/state/temp with backup policy
- [ ] **F-AP-008.02 — Freeze the exact contract for app data directories**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: package/read-only, per-user config/data/cache/state/temp with backup policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-008.03 — Implement/prove: package/read-only**
  - Action: For app data directories, implement or reuse and verify this exact obligation: package/read-only. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for package/read-only; retain observable state/resource expectations.
- [ ] **F-AP-008.04 — Implement/prove: per-user config/data/cache/state/temp with backup policy**
  - Action: For app data directories, implement or reuse and verify this exact obligation: per-user config/data/cache/state/temp with backup policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user config/data/cache/state/temp with backup policy; retain observable state/resource expectations.
- [ ] **F-AP-008.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app data directories: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-008.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-008.06 — Integrate into the real consumer and runtime route**
  - Action: Wire app data directories into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-008.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-008.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app data directories as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-008.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-009"></a>
## F-AP-009 — settings service

**Original requirement:** typed versioned schema, validation, transaction, notification, migration and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-009.01 — Reconcile existing settings service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for settings service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed versioned schema, validation, transaction, notification, migration and rollback
- [ ] **F-AP-009.02 — Freeze the exact contract for settings service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed versioned schema, validation, transaction, notification, migration and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-009.03 — Implement/prove: typed versioned schema**
  - Action: For settings service, implement or reuse and verify this exact obligation: typed versioned schema. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed versioned schema; retain observable state/resource expectations.
- [ ] **F-AP-009.04 — Implement/prove: validation**
  - Action: For settings service, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-AP-009.05 — Implement/prove: transaction**
  - Action: For settings service, implement or reuse and verify this exact obligation: transaction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for transaction; retain observable state/resource expectations.
- [ ] **F-AP-009.06 — Implement/prove: notification**
  - Action: For settings service, implement or reuse and verify this exact obligation: notification. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notification; retain observable state/resource expectations.
- [ ] **F-AP-009.07 — Implement/prove: migration and rollback**
  - Action: For settings service, implement or reuse and verify this exact obligation: migration and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-009.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migration and rollback; retain observable state/resource expectations.
- [ ] **F-AP-009.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to settings service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-009.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-009.09 — Integrate into the real consumer and runtime route**
  - Action: Wire settings service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-009.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-009.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for settings service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-009.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-010"></a>
## F-AP-010 — secrets service

**Original requirement:** app-scoped non-exportable handles, unlock/rotate/revoke and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-010.01 — Reconcile existing secrets service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for secrets service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: app-scoped non-exportable handles, unlock/rotate/revoke and audit
- [ ] **F-AP-010.02 — Freeze the exact contract for secrets service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: app-scoped non-exportable handles, unlock/rotate/revoke and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-010.03 — Implement/prove: app-scoped non-exportable handles**
  - Action: For secrets service, implement or reuse and verify this exact obligation: app-scoped non-exportable handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app-scoped non-exportable handles; retain observable state/resource expectations.
- [ ] **F-AP-010.04 — Implement/prove: unlock/rotate/revoke and audit**
  - Action: For secrets service, implement or reuse and verify this exact obligation: unlock/rotate/revoke and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unlock/rotate/revoke and audit; retain observable state/resource expectations.
- [ ] **F-AP-010.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to secrets service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-010.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-010.06 — Integrate into the real consumer and runtime route**
  - Action: Wire secrets service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-010.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-010.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for secrets service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-010.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-011"></a>
## F-AP-011 — portal service

**Original requirement:** authenticated open/save/share/capture/device/secret operations and user consent

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-011.01 — Reconcile existing portal service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for portal service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated open/save/share/capture/device/secret operations and user consent
- [ ] **F-AP-011.02 — Freeze the exact contract for portal service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated open/save/share/capture/device/secret operations and user consent. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-011.03 — Implement/prove: authenticated open/save/share/capture/device/secret operations and user consent**
  - Action: For portal service, implement or reuse and verify this exact obligation: authenticated open/save/share/capture/device/secret operations and user consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated open/save/share/capture/device/secret operations and user consent; retain observable state/resource expectations.
- [ ] **F-AP-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to portal service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire portal service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for portal service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-012"></a>
## F-AP-012 — MIME/association service

**Original requirement:** safe type resolution, defaults, open-with and app capability compatibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-012.01 — Reconcile existing MIME/association service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for MIME/association service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: safe type resolution, defaults, open-with and app capability compatibility
- [ ] **F-AP-012.02 — Freeze the exact contract for MIME/association service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: safe type resolution, defaults, open-with and app capability compatibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-012.03 — Implement/prove: safe type resolution**
  - Action: For MIME/association service, implement or reuse and verify this exact obligation: safe type resolution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe type resolution; retain observable state/resource expectations.
- [ ] **F-AP-012.04 — Implement/prove: defaults**
  - Action: For MIME/association service, implement or reuse and verify this exact obligation: defaults. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defaults; retain observable state/resource expectations.
- [ ] **F-AP-012.05 — Implement/prove: open-with and app capability compatibility**
  - Action: For MIME/association service, implement or reuse and verify this exact obligation: open-with and app capability compatibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for open-with and app capability compatibility; retain observable state/resource expectations.
- [ ] **F-AP-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to MIME/association service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire MIME/association service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for MIME/association service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-013"></a>
## F-AP-013 — localization service

**Original requirement:** resources, locale change, fallback, formats and restart/live-update policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-013.01 — Reconcile existing localization service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for localization service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: resources, locale change, fallback, formats and restart/live-update policy
- [ ] **F-AP-013.02 — Freeze the exact contract for localization service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: resources, locale change, fallback, formats and restart/live-update policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-013.03 — Implement/prove: resources**
  - Action: For localization service, implement or reuse and verify this exact obligation: resources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resources; retain observable state/resource expectations.
- [ ] **F-AP-013.04 — Implement/prove: locale change**
  - Action: For localization service, implement or reuse and verify this exact obligation: locale change. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale change; retain observable state/resource expectations.
- [ ] **F-AP-013.05 — Implement/prove: fallback**
  - Action: For localization service, implement or reuse and verify this exact obligation: fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback; retain observable state/resource expectations.
- [ ] **F-AP-013.06 — Implement/prove: formats and restart/live-update policy**
  - Action: For localization service, implement or reuse and verify this exact obligation: formats and restart/live-update policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-013.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats and restart/live-update policy; retain observable state/resource expectations.
- [ ] **F-AP-013.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to localization service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-013.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-013.08 — Integrate into the real consumer and runtime route**
  - Action: Wire localization service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-013.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-013.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for localization service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-013.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-014"></a>
## F-AP-014 — font service

**Original requirement:** discovery, shaping/rasterization caches, permissions, fallback and diagnostics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-014.01 — Reconcile existing font service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for font service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discovery, shaping/rasterization caches, permissions, fallback and diagnostics
- [ ] **F-AP-014.02 — Freeze the exact contract for font service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discovery, shaping/rasterization caches, permissions, fallback and diagnostics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-014.03 — Implement/prove: discovery**
  - Action: For font service, implement or reuse and verify this exact obligation: discovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery; retain observable state/resource expectations.
- [ ] **F-AP-014.04 — Implement/prove: shaping/rasterization caches**
  - Action: For font service, implement or reuse and verify this exact obligation: shaping/rasterization caches. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shaping/rasterization caches; retain observable state/resource expectations.
- [ ] **F-AP-014.05 — Implement/prove: permissions**
  - Action: For font service, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-AP-014.06 — Implement/prove: fallback and diagnostics**
  - Action: For font service, implement or reuse and verify this exact obligation: fallback and diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback and diagnostics; retain observable state/resource expectations.
- [ ] **F-AP-014.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to font service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-014.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-014.08 — Integrate into the real consumer and runtime route**
  - Action: Wire font service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-014.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-014.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for font service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-014.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-015"></a>
## F-AP-015 — image decoder service

**Original requirement:** restricted process, bounded input/output/time and typed surfaces

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-015.01 — Reconcile existing image decoder service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for image decoder service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: restricted process, bounded input/output/time and typed surfaces
- [ ] **F-AP-015.02 — Freeze the exact contract for image decoder service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: restricted process, bounded input/output/time and typed surfaces. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-015.03 — Implement/prove: restricted process**
  - Action: For image decoder service, implement or reuse and verify this exact obligation: restricted process. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restricted process; retain observable state/resource expectations.
- [ ] **F-AP-015.04 — Implement/prove: bounded input/output/time and typed surfaces**
  - Action: For image decoder service, implement or reuse and verify this exact obligation: bounded input/output/time and typed surfaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded input/output/time and typed surfaces; retain observable state/resource expectations.
- [ ] **F-AP-015.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to image decoder service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-015.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-015.06 — Integrate into the real consumer and runtime route**
  - Action: Wire image decoder service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-015.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-015.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for image decoder service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-015.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-016"></a>
## F-AP-016 — media decoder service

**Original requirement:** demux/decode workers, clocks, queues, failure containment and capability negotiation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-016.01 — Reconcile existing media decoder service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for media decoder service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: demux/decode workers, clocks, queues, failure containment and capability negotiation
- [ ] **F-AP-016.02 — Freeze the exact contract for media decoder service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: demux/decode workers, clocks, queues, failure containment and capability negotiation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-016.03 — Implement/prove: demux/decode workers**
  - Action: For media decoder service, implement or reuse and verify this exact obligation: demux/decode workers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for demux/decode workers; retain observable state/resource expectations.
- [ ] **F-AP-016.04 — Implement/prove: clocks**
  - Action: For media decoder service, implement or reuse and verify this exact obligation: clocks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clocks; retain observable state/resource expectations.
- [ ] **F-AP-016.05 — Implement/prove: queues**
  - Action: For media decoder service, implement or reuse and verify this exact obligation: queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for queues; retain observable state/resource expectations.
- [ ] **F-AP-016.06 — Implement/prove: failure containment and capability negotiation**
  - Action: For media decoder service, implement or reuse and verify this exact obligation: failure containment and capability negotiation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-016.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failure containment and capability negotiation; retain observable state/resource expectations.
- [ ] **F-AP-016.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to media decoder service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-016.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-016.08 — Integrate into the real consumer and runtime route**
  - Action: Wire media decoder service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-016.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-016.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for media decoder service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-016.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-017"></a>
## F-AP-017 — thumbnail service

**Original requirement:** asynchronous cancelable sandboxed previews with cache and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-017.01 — Reconcile existing thumbnail service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for thumbnail service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: asynchronous cancelable sandboxed previews with cache and privacy
- [ ] **F-AP-017.02 — Freeze the exact contract for thumbnail service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: asynchronous cancelable sandboxed previews with cache and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-017.03 — Implement/prove: asynchronous cancelable sandboxed previews with cache and privacy**
  - Action: For thumbnail service, implement or reuse and verify this exact obligation: asynchronous cancelable sandboxed previews with cache and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for asynchronous cancelable sandboxed previews with cache and privacy; retain observable state/resource expectations.
- [ ] **F-AP-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to thumbnail service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire thumbnail service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for thumbnail service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-018"></a>
## F-AP-018 — search/index service

**Original requirement:** permission-aware per-user indexing, query budgets and privacy controls

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-018.01 — Reconcile existing search/index service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for search/index service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permission-aware per-user indexing, query budgets and privacy controls
- [ ] **F-AP-018.02 — Freeze the exact contract for search/index service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permission-aware per-user indexing, query budgets and privacy controls. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-018.03 — Implement/prove: permission-aware per-user indexing**
  - Action: For search/index service, implement or reuse and verify this exact obligation: permission-aware per-user indexing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission-aware per-user indexing; retain observable state/resource expectations.
- [ ] **F-AP-018.04 — Implement/prove: query budgets and privacy controls**
  - Action: For search/index service, implement or reuse and verify this exact obligation: query budgets and privacy controls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for query budgets and privacy controls; retain observable state/resource expectations.
- [ ] **F-AP-018.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to search/index service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-018.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-018.06 — Integrate into the real consumer and runtime route**
  - Action: Wire search/index service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-018.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-018.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for search/index service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-018.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-019"></a>
## F-AP-019 — notification API

**Original requirement:** permissions, urgency, actions/progress, replacement and lifecycle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-019.01 — Reconcile existing notification API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for notification API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permissions, urgency, actions/progress, replacement and lifecycle
- [ ] **F-AP-019.02 — Freeze the exact contract for notification API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permissions, urgency, actions/progress, replacement and lifecycle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-019.03 — Implement/prove: permissions**
  - Action: For notification API, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-AP-019.04 — Implement/prove: urgency**
  - Action: For notification API, implement or reuse and verify this exact obligation: urgency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for urgency; retain observable state/resource expectations.
- [ ] **F-AP-019.05 — Implement/prove: actions/progress**
  - Action: For notification API, implement or reuse and verify this exact obligation: actions/progress. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for actions/progress; retain observable state/resource expectations.
- [ ] **F-AP-019.06 — Implement/prove: replacement and lifecycle**
  - Action: For notification API, implement or reuse and verify this exact obligation: replacement and lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for replacement and lifecycle; retain observable state/resource expectations.
- [ ] **F-AP-019.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to notification API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-019.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-019.08 — Integrate into the real consumer and runtime route**
  - Action: Wire notification API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-019.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-019.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for notification API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-019.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-020"></a>
## F-AP-020 — clipboard API

**Original requirement:** typed MIME offers/transfers, ownership and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-020.01 — Reconcile existing clipboard API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for clipboard API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed MIME offers/transfers, ownership and cancellation
- [ ] **F-AP-020.02 — Freeze the exact contract for clipboard API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed MIME offers/transfers, ownership and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-020.03 — Implement/prove: typed MIME offers/transfers**
  - Action: For clipboard API, implement or reuse and verify this exact obligation: typed MIME offers/transfers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed MIME offers/transfers; retain observable state/resource expectations.
- [ ] **F-AP-020.04 — Implement/prove: ownership and cancellation**
  - Action: For clipboard API, implement or reuse and verify this exact obligation: ownership and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ownership and cancellation; retain observable state/resource expectations.
- [ ] **F-AP-020.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to clipboard API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-020.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-020.06 — Integrate into the real consumer and runtime route**
  - Action: Wire clipboard API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-020.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-020.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for clipboard API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-020.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-021"></a>
## F-AP-021 — file API

**Original requirement:** handle-relative operations, atomic save, watch/change, cancellation and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-021.01 — Reconcile existing file API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: handle-relative operations, atomic save, watch/change, cancellation and errors
- [ ] **F-AP-021.02 — Freeze the exact contract for file API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: handle-relative operations, atomic save, watch/change, cancellation and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-021.03 — Implement/prove: handle-relative operations**
  - Action: For file API, implement or reuse and verify this exact obligation: handle-relative operations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handle-relative operations; retain observable state/resource expectations.
- [ ] **F-AP-021.04 — Implement/prove: atomic save**
  - Action: For file API, implement or reuse and verify this exact obligation: atomic save. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic save; retain observable state/resource expectations.
- [ ] **F-AP-021.05 — Implement/prove: watch/change**
  - Action: For file API, implement or reuse and verify this exact obligation: watch/change. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for watch/change; retain observable state/resource expectations.
- [ ] **F-AP-021.06 — Implement/prove: cancellation and errors**
  - Action: For file API, implement or reuse and verify this exact obligation: cancellation and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation and errors; retain observable state/resource expectations.
- [ ] **F-AP-021.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-021.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-021.08 — Integrate into the real consumer and runtime route**
  - Action: Wire file API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-021.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-021.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-021.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-022"></a>
## F-AP-022 — network request API

**Original requirement:** capability-scoped destinations, TLS policy, quotas, deadlines and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-022.01 — Reconcile existing network request API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network request API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: capability-scoped destinations, TLS policy, quotas, deadlines and provenance
- [ ] **F-AP-022.02 — Freeze the exact contract for network request API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: capability-scoped destinations, TLS policy, quotas, deadlines and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-022.03 — Implement/prove: capability-scoped destinations**
  - Action: For network request API, implement or reuse and verify this exact obligation: capability-scoped destinations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capability-scoped destinations; retain observable state/resource expectations.
- [ ] **F-AP-022.04 — Implement/prove: TLS policy**
  - Action: For network request API, implement or reuse and verify this exact obligation: TLS policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for TLS policy; retain observable state/resource expectations.
- [ ] **F-AP-022.05 — Implement/prove: quotas**
  - Action: For network request API, implement or reuse and verify this exact obligation: quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas; retain observable state/resource expectations.
- [ ] **F-AP-022.06 — Implement/prove: deadlines and provenance**
  - Action: For network request API, implement or reuse and verify this exact obligation: deadlines and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadlines and provenance; retain observable state/resource expectations.
- [ ] **F-AP-022.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network request API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-022.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-022.08 — Integrate into the real consumer and runtime route**
  - Action: Wire network request API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-022.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-022.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network request API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-022.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-023"></a>
## F-AP-023 — audio API

**Original requirement:** stream/session handles, formats, latency, volume and device changes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-023.01 — Reconcile existing audio API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stream/session handles, formats, latency, volume and device changes
- [ ] **F-AP-023.02 — Freeze the exact contract for audio API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stream/session handles, formats, latency, volume and device changes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-023.03 — Implement/prove: stream/session handles**
  - Action: For audio API, implement or reuse and verify this exact obligation: stream/session handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stream/session handles; retain observable state/resource expectations.
- [ ] **F-AP-023.04 — Implement/prove: formats**
  - Action: For audio API, implement or reuse and verify this exact obligation: formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats; retain observable state/resource expectations.
- [ ] **F-AP-023.05 — Implement/prove: latency**
  - Action: For audio API, implement or reuse and verify this exact obligation: latency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for latency; retain observable state/resource expectations.
- [ ] **F-AP-023.06 — Implement/prove: volume and device changes**
  - Action: For audio API, implement or reuse and verify this exact obligation: volume and device changes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for volume and device changes; retain observable state/resource expectations.
- [ ] **F-AP-023.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-023.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-023.08 — Integrate into the real consumer and runtime route**
  - Action: Wire audio API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-023.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-023.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-023.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-024"></a>
## F-AP-024 — camera/microphone API

**Original requirement:** explicit user-mediated device grants, indicator and revoke

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-024.01 — Reconcile existing camera/microphone API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for camera/microphone API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit user-mediated device grants, indicator and revoke
- [ ] **F-AP-024.02 — Freeze the exact contract for camera/microphone API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit user-mediated device grants, indicator and revoke. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-024.03 — Implement/prove: explicit user-mediated device grants**
  - Action: For camera/microphone API, implement or reuse and verify this exact obligation: explicit user-mediated device grants. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit user-mediated device grants; retain observable state/resource expectations.
- [ ] **F-AP-024.04 — Implement/prove: indicator and revoke**
  - Action: For camera/microphone API, implement or reuse and verify this exact obligation: indicator and revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for indicator and revoke; retain observable state/resource expectations.
- [ ] **F-AP-024.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to camera/microphone API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-024.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-024.06 — Integrate into the real consumer and runtime route**
  - Action: Wire camera/microphone API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-024.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-024.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for camera/microphone API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-024.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-025"></a>
## F-AP-025 — print API

**Original requirement:** document/job handle, options, progress, cancel and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-025.01 — Reconcile existing print API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for print API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: document/job handle, options, progress, cancel and privacy
- [ ] **F-AP-025.02 — Freeze the exact contract for print API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: document/job handle, options, progress, cancel and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-025.03 — Implement/prove: document/job handle**
  - Action: For print API, implement or reuse and verify this exact obligation: document/job handle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for document/job handle; retain observable state/resource expectations.
- [ ] **F-AP-025.04 — Implement/prove: options**
  - Action: For print API, implement or reuse and verify this exact obligation: options. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for options; retain observable state/resource expectations.
- [ ] **F-AP-025.05 — Implement/prove: progress**
  - Action: For print API, implement or reuse and verify this exact obligation: progress. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress; retain observable state/resource expectations.
- [ ] **F-AP-025.06 — Implement/prove: cancel and privacy**
  - Action: For print API, implement or reuse and verify this exact obligation: cancel and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-025.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancel and privacy; retain observable state/resource expectations.
- [ ] **F-AP-025.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to print API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-025.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-025.08 — Integrate into the real consumer and runtime route**
  - Action: Wire print API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-025.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-025.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for print API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-025.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-026"></a>
## F-AP-026 — background-task API

**Original requirement:** declared reason, time/resource budget, user visibility and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-026.01 — Reconcile existing background-task API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for background-task API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: declared reason, time/resource budget, user visibility and cancellation
- [ ] **F-AP-026.02 — Freeze the exact contract for background-task API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: declared reason, time/resource budget, user visibility and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-026.03 — Implement/prove: declared reason**
  - Action: For background-task API, implement or reuse and verify this exact obligation: declared reason. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared reason; retain observable state/resource expectations.
- [ ] **F-AP-026.04 — Implement/prove: time/resource budget**
  - Action: For background-task API, implement or reuse and verify this exact obligation: time/resource budget. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time/resource budget; retain observable state/resource expectations.
- [ ] **F-AP-026.05 — Implement/prove: user visibility and cancellation**
  - Action: For background-task API, implement or reuse and verify this exact obligation: user visibility and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user visibility and cancellation; retain observable state/resource expectations.
- [ ] **F-AP-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to background-task API: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire background-task API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for background-task API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-027"></a>
## F-AP-027 — app update/migration

**Original requirement:** signed atomic generation, schema migration, rollback and compatibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-027.01 — Reconcile existing app update/migration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app update/migration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed atomic generation, schema migration, rollback and compatibility
- [ ] **F-AP-027.02 — Freeze the exact contract for app update/migration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed atomic generation, schema migration, rollback and compatibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-027.03 — Implement/prove: signed atomic generation**
  - Action: For app update/migration, implement or reuse and verify this exact obligation: signed atomic generation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed atomic generation; retain observable state/resource expectations.
- [ ] **F-AP-027.04 — Implement/prove: schema migration**
  - Action: For app update/migration, implement or reuse and verify this exact obligation: schema migration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for schema migration; retain observable state/resource expectations.
- [ ] **F-AP-027.05 — Implement/prove: rollback and compatibility**
  - Action: For app update/migration, implement or reuse and verify this exact obligation: rollback and compatibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rollback and compatibility; retain observable state/resource expectations.
- [ ] **F-AP-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app update/migration: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire app update/migration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app update/migration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-028"></a>
## F-AP-028 — app store/catalog

**Original requirement:** signed index, provenance, permissions, compatibility, evidence and reviews policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-028.01 — Reconcile existing app store/catalog**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app store/catalog. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed index, provenance, permissions, compatibility, evidence and reviews policy
- [ ] **F-AP-028.02 — Freeze the exact contract for app store/catalog**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed index, provenance, permissions, compatibility, evidence and reviews policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-028.03 — Implement/prove: signed index**
  - Action: For app store/catalog, implement or reuse and verify this exact obligation: signed index. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed index; retain observable state/resource expectations.
- [ ] **F-AP-028.04 — Implement/prove: provenance**
  - Action: For app store/catalog, implement or reuse and verify this exact obligation: provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance; retain observable state/resource expectations.
- [ ] **F-AP-028.05 — Implement/prove: permissions**
  - Action: For app store/catalog, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-AP-028.06 — Implement/prove: compatibility**
  - Action: For app store/catalog, implement or reuse and verify this exact obligation: compatibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compatibility; retain observable state/resource expectations.
- [ ] **F-AP-028.07 — Implement/prove: evidence and reviews policy**
  - Action: For app store/catalog, implement or reuse and verify this exact obligation: evidence and reviews policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-028.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for evidence and reviews policy; retain observable state/resource expectations.
- [ ] **F-AP-028.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app store/catalog: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-028.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-028.09 — Integrate into the real consumer and runtime route**
  - Action: Wire app store/catalog into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-028.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-028.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app store/catalog as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-028.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-029"></a>
## F-AP-029 — developer mode/sideload

**Original requirement:** explicit risk boundary, signatures/keys, isolation, diagnostics and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-029.01 — Reconcile existing developer mode/sideload**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for developer mode/sideload. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit risk boundary, signatures/keys, isolation, diagnostics and revocation
- [ ] **F-AP-029.02 — Freeze the exact contract for developer mode/sideload**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit risk boundary, signatures/keys, isolation, diagnostics and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-029.03 — Implement/prove: explicit risk boundary**
  - Action: For developer mode/sideload, implement or reuse and verify this exact obligation: explicit risk boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit risk boundary; retain observable state/resource expectations.
- [ ] **F-AP-029.04 — Implement/prove: signatures/keys**
  - Action: For developer mode/sideload, implement or reuse and verify this exact obligation: signatures/keys. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signatures/keys; retain observable state/resource expectations.
- [ ] **F-AP-029.05 — Implement/prove: isolation**
  - Action: For developer mode/sideload, implement or reuse and verify this exact obligation: isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolation; retain observable state/resource expectations.
- [ ] **F-AP-029.06 — Implement/prove: diagnostics and revocation**
  - Action: For developer mode/sideload, implement or reuse and verify this exact obligation: diagnostics and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-029.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics and revocation; retain observable state/resource expectations.
- [ ] **F-AP-029.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to developer mode/sideload: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-029.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-029.08 — Integrate into the real consumer and runtime route**
  - Action: Wire developer mode/sideload into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-029.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-029.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for developer mode/sideload as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-029.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-030"></a>
## F-AP-030 — app backup/export

**Original requirement:** declared user-data set, secrets exclusion, version and restore proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-030.01 — Reconcile existing app backup/export**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app backup/export. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: declared user-data set, secrets exclusion, version and restore proof
- [ ] **F-AP-030.02 — Freeze the exact contract for app backup/export**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: declared user-data set, secrets exclusion, version and restore proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-030.03 — Implement/prove: declared user-data set**
  - Action: For app backup/export, implement or reuse and verify this exact obligation: declared user-data set. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared user-data set; retain observable state/resource expectations.
- [ ] **F-AP-030.04 — Implement/prove: secrets exclusion**
  - Action: For app backup/export, implement or reuse and verify this exact obligation: secrets exclusion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secrets exclusion; retain observable state/resource expectations.
- [ ] **F-AP-030.05 — Implement/prove: version and restore proof**
  - Action: For app backup/export, implement or reuse and verify this exact obligation: version and restore proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for version and restore proof; retain observable state/resource expectations.
- [ ] **F-AP-030.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app backup/export: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-030.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-030.07 — Integrate into the real consumer and runtime route**
  - Action: Wire app backup/export into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-030.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-030.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app backup/export as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-030.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-031"></a>
## F-AP-031 — document model

**Original requirement:** origin, dirty/conflict/autosave/recovery, atomic save and recent items

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-031.01 — Reconcile existing document model**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for document model. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: origin, dirty/conflict/autosave/recovery, atomic save and recent items
- [ ] **F-AP-031.02 — Freeze the exact contract for document model**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: origin, dirty/conflict/autosave/recovery, atomic save and recent items. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-031.03 — Implement/prove: origin**
  - Action: For document model, implement or reuse and verify this exact obligation: origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin; retain observable state/resource expectations.
- [ ] **F-AP-031.04 — Implement/prove: dirty/conflict/autosave/recovery**
  - Action: For document model, implement or reuse and verify this exact obligation: dirty/conflict/autosave/recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dirty/conflict/autosave/recovery; retain observable state/resource expectations.
- [ ] **F-AP-031.05 — Implement/prove: atomic save and recent items**
  - Action: For document model, implement or reuse and verify this exact obligation: atomic save and recent items. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic save and recent items; retain observable state/resource expectations.
- [ ] **F-AP-031.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to document model: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-031.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-031.07 — Integrate into the real consumer and runtime route**
  - Action: Wire document model into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-031.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-031.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for document model as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-031.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-032"></a>
## F-AP-032 — undo/redo service pattern

**Original requirement:** bounded command history, grouping, persistence policy and failure semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-032.01 — Reconcile existing undo/redo service pattern**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for undo/redo service pattern. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded command history, grouping, persistence policy and failure semantics
- [ ] **F-AP-032.02 — Freeze the exact contract for undo/redo service pattern**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded command history, grouping, persistence policy and failure semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-032.03 — Implement/prove: bounded command history**
  - Action: For undo/redo service pattern, implement or reuse and verify this exact obligation: bounded command history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded command history; retain observable state/resource expectations.
- [ ] **F-AP-032.04 — Implement/prove: grouping**
  - Action: For undo/redo service pattern, implement or reuse and verify this exact obligation: grouping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-032.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grouping; retain observable state/resource expectations.
- [ ] **F-AP-032.05 — Implement/prove: persistence policy and failure semantics**
  - Action: For undo/redo service pattern, implement or reuse and verify this exact obligation: persistence policy and failure semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-032.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistence policy and failure semantics; retain observable state/resource expectations.
- [ ] **F-AP-032.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to undo/redo service pattern: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-032.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-032.07 — Integrate into the real consumer and runtime route**
  - Action: Wire undo/redo service pattern into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-032.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-032.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for undo/redo service pattern as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-032.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-033"></a>
## F-AP-033 — command/action registry

**Original requirement:** semantic actions, labels, shortcuts, enablement and agent/a11y invocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-033.01 — Reconcile existing command/action registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for command/action registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic actions, labels, shortcuts, enablement and agent/a11y invocation
- [ ] **F-AP-033.02 — Freeze the exact contract for command/action registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic actions, labels, shortcuts, enablement and agent/a11y invocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-033.03 — Implement/prove: semantic actions**
  - Action: For command/action registry, implement or reuse and verify this exact obligation: semantic actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic actions; retain observable state/resource expectations.
- [ ] **F-AP-033.04 — Implement/prove: labels**
  - Action: For command/action registry, implement or reuse and verify this exact obligation: labels. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for labels; retain observable state/resource expectations.
- [ ] **F-AP-033.05 — Implement/prove: shortcuts**
  - Action: For command/action registry, implement or reuse and verify this exact obligation: shortcuts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shortcuts; retain observable state/resource expectations.
- [ ] **F-AP-033.06 — Implement/prove: enablement and agent/a11y invocation**
  - Action: For command/action registry, implement or reuse and verify this exact obligation: enablement and agent/a11y invocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for enablement and agent/a11y invocation; retain observable state/resource expectations.
- [ ] **F-AP-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to command/action registry: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire command/action registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for command/action registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-034"></a>
## F-AP-034 — plugin/extension model

**Original requirement:** signed scoped extension processes, versioned API, quotas and disable/recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-034.01 — Reconcile existing plugin/extension model**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for plugin/extension model. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed scoped extension processes, versioned API, quotas and disable/recovery
- [ ] **F-AP-034.02 — Freeze the exact contract for plugin/extension model**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed scoped extension processes, versioned API, quotas and disable/recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-034.03 — Implement/prove: signed scoped extension processes**
  - Action: For plugin/extension model, implement or reuse and verify this exact obligation: signed scoped extension processes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed scoped extension processes; retain observable state/resource expectations.
- [ ] **F-AP-034.04 — Implement/prove: versioned API**
  - Action: For plugin/extension model, implement or reuse and verify this exact obligation: versioned API. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned API; retain observable state/resource expectations.
- [ ] **F-AP-034.05 — Implement/prove: quotas and disable/recovery**
  - Action: For plugin/extension model, implement or reuse and verify this exact obligation: quotas and disable/recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-034.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas and disable/recovery; retain observable state/resource expectations.
- [ ] **F-AP-034.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to plugin/extension model: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-034.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-034.07 — Integrate into the real consumer and runtime route**
  - Action: Wire plugin/extension model into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-034.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-034.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for plugin/extension model as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-034.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-035"></a>
## F-AP-035 — app evidence page

**Original requirement:** routes, permissions, services, resource use, crashes, provenance and proof state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-035.01 — Reconcile existing app evidence page**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for app evidence page. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: routes, permissions, services, resource use, crashes, provenance and proof state
- [ ] **F-AP-035.02 — Freeze the exact contract for app evidence page**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: routes, permissions, services, resource use, crashes, provenance and proof state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-035.03 — Implement/prove: routes**
  - Action: For app evidence page, implement or reuse and verify this exact obligation: routes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for routes; retain observable state/resource expectations.
- [ ] **F-AP-035.04 — Implement/prove: permissions**
  - Action: For app evidence page, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-AP-035.05 — Implement/prove: services**
  - Action: For app evidence page, implement or reuse and verify this exact obligation: services. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for services; retain observable state/resource expectations.
- [ ] **F-AP-035.06 — Implement/prove: resource use**
  - Action: For app evidence page, implement or reuse and verify this exact obligation: resource use. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-035.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resource use; retain observable state/resource expectations.
- [ ] **F-AP-035.07 — Implement/prove: crashes**
  - Action: For app evidence page, implement or reuse and verify this exact obligation: crashes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-035.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crashes; retain observable state/resource expectations.
- [ ] **F-AP-035.08 — Implement/prove: provenance and proof state**
  - Action: For app evidence page, implement or reuse and verify this exact obligation: provenance and proof state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-035.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance and proof state; retain observable state/resource expectations.
- [ ] **F-AP-035.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to app evidence page: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-035.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-035.10 — Integrate into the real consumer and runtime route**
  - Action: Wire app evidence page into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-035.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-035.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for app evidence page as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-035.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-036"></a>
## F-AP-036 — online-account broker

**Original requirement:** provider identity/auth tokens behind scoped handles, revoke, refresh and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-036.01 — Reconcile existing online-account broker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for online-account broker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider identity/auth tokens behind scoped handles, revoke, refresh and privacy
- [ ] **F-AP-036.02 — Freeze the exact contract for online-account broker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider identity/auth tokens behind scoped handles, revoke, refresh and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-036.03 — Implement/prove: provider identity/auth tokens behind scoped handles**
  - Action: For online-account broker, implement or reuse and verify this exact obligation: provider identity/auth tokens behind scoped handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider identity/auth tokens behind scoped handles; retain observable state/resource expectations.
- [ ] **F-AP-036.04 — Implement/prove: revoke**
  - Action: For online-account broker, implement or reuse and verify this exact obligation: revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revoke; retain observable state/resource expectations.
- [ ] **F-AP-036.05 — Implement/prove: refresh and privacy**
  - Action: For online-account broker, implement or reuse and verify this exact obligation: refresh and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for refresh and privacy; retain observable state/resource expectations.
- [ ] **F-AP-036.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to online-account broker: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-036.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-036.07 — Integrate into the real consumer and runtime route**
  - Action: Wire online-account broker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-036.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-036.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for online-account broker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-036.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-037"></a>
## F-AP-037 — sync service

**Original requirement:** per-dataset conflict/offline/retry/encryption/quota/account and user-visible status

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-037.01 — Reconcile existing sync service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sync service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-dataset conflict/offline/retry/encryption/quota/account and user-visible status
- [ ] **F-AP-037.02 — Freeze the exact contract for sync service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-dataset conflict/offline/retry/encryption/quota/account and user-visible status. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-037.03 — Implement/prove: per-dataset conflict/offline/retry/encryption/quota/account and user-visible status**
  - Action: For sync service, implement or reuse and verify this exact obligation: per-dataset conflict/offline/retry/encryption/quota/account and user-visible status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-dataset conflict/offline/retry/encryption/quota/account and user-visible status; retain observable state/resource expectations.
- [ ] **F-AP-037.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sync service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-037.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-037.05 — Integrate into the real consumer and runtime route**
  - Action: Wire sync service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-037.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-037.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sync service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-037.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-038"></a>
## F-AP-038 — background transfer service

**Original requirement:** resumable bounded upload/download, connectivity/power policy, progress and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-038.01 — Reconcile existing background transfer service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for background transfer service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: resumable bounded upload/download, connectivity/power policy, progress and cancellation
- [ ] **F-AP-038.02 — Freeze the exact contract for background transfer service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: resumable bounded upload/download, connectivity/power policy, progress and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-038.03 — Implement/prove: resumable bounded upload/download**
  - Action: For background transfer service, implement or reuse and verify this exact obligation: resumable bounded upload/download. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resumable bounded upload/download; retain observable state/resource expectations.
- [ ] **F-AP-038.04 — Implement/prove: connectivity/power policy**
  - Action: For background transfer service, implement or reuse and verify this exact obligation: connectivity/power policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for connectivity/power policy; retain observable state/resource expectations.
- [ ] **F-AP-038.05 — Implement/prove: progress and cancellation**
  - Action: For background transfer service, implement or reuse and verify this exact obligation: progress and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress and cancellation; retain observable state/resource expectations.
- [ ] **F-AP-038.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to background transfer service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-038.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-038.07 — Integrate into the real consumer and runtime route**
  - Action: Wire background transfer service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-038.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-038.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for background transfer service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-038.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-039"></a>
## F-AP-039 — location service

**Original requirement:** device/provider fusion, precision, foreground/background consent and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-039.01 — Reconcile existing location service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for location service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: device/provider fusion, precision, foreground/background consent and audit
- [ ] **F-AP-039.02 — Freeze the exact contract for location service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: device/provider fusion, precision, foreground/background consent and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-039.03 — Implement/prove: device/provider fusion**
  - Action: For location service, implement or reuse and verify this exact obligation: device/provider fusion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device/provider fusion; retain observable state/resource expectations.
- [ ] **F-AP-039.04 — Implement/prove: precision**
  - Action: For location service, implement or reuse and verify this exact obligation: precision. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for precision; retain observable state/resource expectations.
- [ ] **F-AP-039.05 — Implement/prove: foreground/background consent and audit**
  - Action: For location service, implement or reuse and verify this exact obligation: foreground/background consent and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for foreground/background consent and audit; retain observable state/resource expectations.
- [ ] **F-AP-039.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to location service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-039.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-039.07 — Integrate into the real consumer and runtime route**
  - Action: Wire location service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-039.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-039.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for location service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-039.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ap-040"></a>
## F-AP-040 — sensor service

**Original requirement:** typed units/rates/batching/permissions/calibration and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AP-040.01 — Reconcile existing sensor service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sensor service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed units/rates/batching/permissions/calibration and privacy
- [ ] **F-AP-040.02 — Freeze the exact contract for sensor service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed units/rates/batching/permissions/calibration and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AP-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AP-040.03 — Implement/prove: typed units/rates/batching/permissions/calibration and privacy**
  - Action: For sensor service, implement or reuse and verify this exact obligation: typed units/rates/batching/permissions/calibration and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AP-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed units/rates/batching/permissions/calibration and privacy; retain observable state/resource expectations.
- [ ] **F-AP-040.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sensor service: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AP-040.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AP-040.05 — Integrate into the real consumer and runtime route**
  - Action: Wire sensor service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AP-040.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AP-040.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sensor service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AP-040.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p5-6"></a>
## C-P5.6 — restricted parser workers

**Original requirement:** restricted parser workers

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 318.

### Preserved original contract

- **Dependencies/current/provenance:** P3/P4 supervisor and File/Window services; Serenity disposable decoders, protOS TAR/ELF negatives; reject parser in kernel/trusted service.
- **I/O and state:** read-only input handle plus output surface/object handle and limits in; decoded object or typed error out; one-shot worker lifecycle.
- **Invariants/failure:** no ambient file/network/device rights; output bounds declared; crash/fault destroys worker and revokes memory; parser never returns raw internal pointers.
- **Deterministic proof:** mutation/fuzz/sanitizer corpora for image/font/archive/media; output-size bombs; worker kill/restart; nondeterminism check.
- **Target proof:** QEMU browser/files app survives malformed corpus without desktop failure.
- **Receipt/removal:** corpus version, crash/rejection/output hash; in-process parser remains disabled fallback only during migration; remove trusted parser after coverage parity.

### Execution steps

- [ ] **C-P5.6.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.6.02 — Resolve this contract's exact dependencies**
  - Action: P3/P4 supervisor and File/Window services; Serenity disposable decoders, protOS TAR/ELF negatives; reject parser in kernel/trusted service. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.6.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.6.03 — I/O and state — restricted parser workers**
  - Action: read-only input handle plus output surface/object handle and limits in; decoded object or typed error out; one-shot worker lifecycle.
  - Requires: C-P5.6.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.6.
- [ ] **C-P5.6.04 — Invariants/failure — restricted parser workers**
  - Action: no ambient file/network/device rights; output bounds declared; crash/fault destroys worker and revokes memory; parser never returns raw internal pointers.
  - Requires: C-P5.6.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.6.
- [ ] **C-P5.6.05 — Deterministic proof — restricted parser workers**
  - Action: mutation/fuzz/sanitizer corpora for image/font/archive/media; output-size bombs; worker kill/restart; nondeterminism check.
  - Requires: C-P5.6.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.6.
- [ ] **C-P5.6.06 — Target proof — restricted parser workers**
  - Action: QEMU browser/files app survives malformed corpus without desktop failure.
  - Requires: C-P5.6.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.6.
- [ ] **C-P5.6.07 — Receipt/removal — restricted parser workers**
  - Action: corpus version, crash/rejection/output hash; in-process parser remains disabled fallback only during migration; remove trusted parser after coverage parity.
  - Requires: C-P5.6.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.6.
- [ ] **C-P5.6.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.6. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.6.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p7-1"></a>
## C-P7.1 — stable native app ABI and SDK

**Original requirement:** stable native app ABI and SDK

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 380.

### Preserved original contract

- **Dependencies/current/provenance:** P3-P5 and zl exact types/records/errors; Brook/Zinnia conventional coverage; reject stable ABI declared before multiple consumers.
- **I/O and state:** ABI version and generated syscall/service declarations in; SDK/libs/manifests/debug info out; version `Experimental -> Candidate -> Stable -> Deprecated`.
- **Invariants/failure:** size/alignment/calling/wire layouts generated; compatibility matrix explicit; unsupported ABI refuses at load.
- **Deterministic proof:** layout/calling convention, old/new app matrix, symbol/version rejection, SDK sample corpus.
- **Target proof:** independently built apps run on QEMU and hardware image.
- **Receipt/removal:** ABI manifest and sample hashes; compatibility shim rollback; remove old ABI only after deprecation inventory reaches zero.

### Execution steps

- [ ] **C-P7.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P7.1.02 — Resolve this contract's exact dependencies**
  - Action: P3-P5 and zl exact types/records/errors; Brook/Zinnia conventional coverage; reject stable ABI declared before multiple consumers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P7.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P7.1.03 — I/O and state — stable native app ABI and SDK**
  - Action: ABI version and generated syscall/service declarations in; SDK/libs/manifests/debug info out; version `Experimental -> Candidate -> Stable -> Deprecated`.
  - Requires: C-P7.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P7.1.
- [ ] **C-P7.1.04 — Invariants/failure — stable native app ABI and SDK**
  - Action: size/alignment/calling/wire layouts generated; compatibility matrix explicit; unsupported ABI refuses at load.
  - Requires: C-P7.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P7.1.
- [ ] **C-P7.1.05 — Deterministic proof — stable native app ABI and SDK**
  - Action: layout/calling convention, old/new app matrix, symbol/version rejection, SDK sample corpus.
  - Requires: C-P7.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P7.1.
- [ ] **C-P7.1.06 — Target proof — stable native app ABI and SDK**
  - Action: independently built apps run on QEMU and hardware image.
  - Requires: C-P7.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P7.1.
- [ ] **C-P7.1.07 — Receipt/removal — stable native app ABI and SDK**
  - Action: ABI manifest and sample hashes; compatibility shim rollback; remove old ABI only after deprecation inventory reaches zero.
  - Requires: C-P7.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P7.1.
- [ ] **C-P7.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P7.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P7.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-20c"></a>
## C-DA-20C — settings and configuration service

**Original requirement:** settings and configuration service

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 517.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-20C.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-20C.02 — Resolve this contract's exact dependencies**
  - Action: DA-18 through DA-20 and persistent File/VFS handles. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-20C.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-20C.03 — Deliver — settings and configuration service**
  - Action: typed, versioned schemas with user/system scopes, validation, transactional writes, watches, migrations, export/import and reset-to-default.
  - Requires: C-DA-20C.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-20C.
- [ ] **C-DA-20C.04 — Invariants — settings and configuration service**
  - Action: system writes require explicit authority; unknown/invalid fields do not partially apply; watchers see one committed version; concurrent writers use a declared conflict policy; restart converges from the journal without losing the last committed configuration.
  - Requires: C-DA-20C.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-20C.
- [ ] **C-DA-20C.05 — Proof — settings and configuration service**
  - Action: invalid type/range, unknown version, migration failure, concurrent writers, watcher backlog/death, disk full, corrupt journal, service crash at each write stage, export/import mismatch and per-user/session isolation.
  - Requires: C-DA-20C.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-20C.
- [ ] **C-DA-20C.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-20C. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-20C.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-21f"></a>
## C-DA-21F — file operation and safe-open portal

**Original requirement:** file operation and safe-open portal

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 549.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-21F.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-21F.02 — Resolve this contract's exact dependencies**
  - Action: DA-21 and DA-18/19 supervision. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-21F.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-21F.03 — Deliver — file operation and safe-open portal**
  - Action: capability-scoped open/file-picker plus transactional copy, move, delete, trash and undo jobs with stable IDs, progress, conflict policy, cancellation and restart recovery. Cross-filesystem moves are explicit copy-verify-delete transactions rather than an implied atomic rename.
  - Requires: C-DA-21F.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-21F.
- [ ] **C-DA-21F.04 — Invariants — file operation and safe-open portal**
  - Action: apps receive selected handles, not ambient path authority; each job has exactly one terminal result; cancellation and service death preserve source and committed destination state; overwrite/merge/rename decisions are explicit; undo expires under a visible retention policy.
  - Requires: C-DA-21F.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-21F.
- [ ] **C-DA-21F.05 — Proof — file operation and safe-open portal**
  - Action: destination exists, permission change, disk full, source/destination device removal, cross-filesystem partial copy, cancel at every step, service crash/ restart, symlink race, trash full, undo conflict and safe-open denial/revocation.
  - Requires: C-DA-21F.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-21F.
- [ ] **C-DA-21F.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-21F. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-21F.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-23"></a>
## C-DA-23 — application manifest and stable identity

**Original requirement:** application manifest and stable identity

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 584.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-23.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-23.02 — Resolve this contract's exact dependencies**
  - Action: DA-00, DA-17 through DA-22. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-23.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-23.03 — Deliver — application manifest and stable identity**
  - Action: schema version; stable ID/name/version/publisher/content digest; signing key ID, trust chain, revocation policy and anti-rollback version; target architectures, kernel ABI range, executable ABI and entrypoint; required versus optional service capabilities; file types/protocols; requested handles and limits; windows/background work; complete file manifest and size limits; persistent-data schema; accessibility metadata; migrations, rollback and uninstall-data policy; deterministic/QEMU proof profiles; license and firmware provenance. Record implementation origin, immutable source revision, exact build/image targets, default reachability and separate code/data/firmware provenance. Declare dependency version constraints, conflicts and optional dependencies.
  - Requires: C-DA-23.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-23.
- [ ] **C-DA-23.04 — Invariants — application manifest and stable identity**
  - Action: unknown mandatory capability, wrong ABI/architecture, excessive budget or untrusted publisher fails before staging; display name/icon cannot replace stable identity; authority is least and explicit. Dependency resolution is deterministic; cycles, conflicts, missing or incompatible dependencies fail before publication.
  - Requires: C-DA-23.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-23.
- [ ] **C-DA-23.05 — Proof — application manifest and stable identity**
  - Action: duplicate/spoofed ID, name collision, bad hash/signature, downgrade, unknown capability, excessive resources, missing accessibility metadata and absent entrypoint; dependency cycle/conflict/missing version and code/data license mismatch.
  - Requires: C-DA-23.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-23.
- [ ] **C-DA-23.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-23. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-23.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-23l"></a>
## C-DA-23L — launch and handler service

**Original requirement:** launch and handler service

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 609.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-23L.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-23L.02 — Resolve this contract's exact dependencies**
  - Action: DA-19 through DA-23 and DA-21F safe-open portals. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-23L.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-23L.03 — Deliver — launch and handler service**
  - Action: registry-backed application, MIME, URL and file handlers; singleton versus multi-instance policy; dependency readiness; safe-open handle transfer; recent-document records; default/ambiguous-handler resolution; and typed launch failure explanation.
  - Requires: C-DA-23L.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-23L.
- [ ] **C-DA-23L.04 — Invariants — launch and handler service**
  - Action: menus/search/file associations derive from installed artifacts; handler choice cannot grant undeclared authority; singleton identity is generation- safe; dependency readiness precedes publication; handler update/removal invalidates stale routes without breaking an already authorized open transaction.
  - Requires: C-DA-23L.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-23L.
- [ ] **C-DA-23L.05 — Proof — launch and handler service**
  - Action: missing/stale handler, ambiguous/no default, spoofed MIME/URL scheme, revoked file handle, dependency timeout, singleton race, handler update/removal during launch, malformed executable and recent-document privacy/deletion.
  - Requires: C-DA-23L.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-23L.
- [ ] **C-DA-23L.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-23L. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-23L.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-25"></a>
## C-DA-25 — application lifecycle and crash semantics

**Original requirement:** application lifecycle and crash semantics

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 646.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-25.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-25.02 — Resolve this contract's exact dependencies**
  - Action: DA-19, DA-20 and DA-23. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-25.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-25.03 — Deliver — application lifecycle and crash semantics**
  - Action: installed/staged/start/ready/background/suspended/stop/crash/restart/ update/uninstall states with ready nonce, close receipt, crash report and unsaved- state policy.
  - Requires: C-DA-25.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-25.
- [ ] **C-DA-25.04 — Invariants — application lifecycle and crash semantics**
  - Action: window close and process lifetime are distinct; app publication follows readiness; restart does not duplicate effects; background tasks are bounded and visible; reports contain artifact/process identity but no secrets.
  - Requires: C-DA-25.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-25.
- [ ] **C-DA-25.05 — Proof — application lifecycle and crash semantics**
  - Action: loader failure, readiness timeout, close with unsaved data, crash before/ after publication, restart loop, session end, service loss, resource exhaustion and stale app handle after update.
  - Requires: C-DA-25.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-25.
- [ ] **C-DA-25.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-25. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-25.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-25n"></a>
## C-DA-25N — notification and crash services

**Original requirement:** notification and crash services

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 662.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-25N.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-25N.02 — Resolve this contract's exact dependencies**
  - Action: DA-08O, DA-19, DA-20 and DA-25. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-25N.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-25N.03 — Deliver — notification and crash services**
  - Action: stable replace/update/dismiss notification IDs, typed actions, per-app rate/quiet policy and quotas; artifact-bound crash capture and symbolization with explicit privacy, redaction, storage and upload/consent policy.
  - Requires: C-DA-25N.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-25N.
- [ ] **C-DA-25N.04 — Invariants — notification and crash services**
  - Action: notification actions retain sender/app/session generation and required authority; a stale or spoofed action cannot execute; crash collection cannot expose another user or secrets; full queues/storage degrade visibly; service restart preserves or discards state only by declared policy.
  - Requires: C-DA-25N.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-25N.
- [ ] **C-DA-25N.05 — Proof — notification and crash services**
  - Action: notification flood, replace/dismiss/action races, stale action after app update/session end, quiet mode, service crash/restart, missing/mismatched symbols, oversized crash, secret-redaction fixtures, storage full and denied upload.
  - Requires: C-DA-25N.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-25N.
- [ ] **C-DA-25N.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-25N. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-25N.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-15"></a>
## C-VX-15 — Notifications, quick settings and background tasks

**Original requirement:** Notifications, quick settings and background tasks

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 319.

### Preserved original contract

**Depends on:** VX-13, portal foundation.

**Deliver:** source-attributed toasts/history, urgency/privacy/actions, progress,
cancel, recording/capture indicators and actual provider-backed toggles.

**Invariants:** lock-screen redaction; app cannot spoof system source; action
handle expires; progress owner death closes or marks failed; unavailable setting
never appears as successful toggle.

**Proof:** spoof, flood/quota, stale action, provider loss, lock privacy, cancel
race and restart/history persistence.

### Execution steps

- [ ] **C-VX-15.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-15.02 — Resolve this contract's exact dependencies**
  - Action: VX-13, portal foundation. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-15.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-15.03 — Deliver — Notifications, quick settings and background tasks**
  - Action: source-attributed toasts/history, urgency/privacy/actions, progress, cancel, recording/capture indicators and actual provider-backed toggles.
  - Requires: C-VX-15.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-15.
- [ ] **C-VX-15.04 — Invariants — Notifications, quick settings and background tasks**
  - Action: lock-screen redaction; app cannot spoof system source; action handle expires; progress owner death closes or marks failed; unavailable setting never appears as successful toggle.
  - Requires: C-VX-15.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-15.
- [ ] **C-VX-15.05 — Proof — Notifications, quick settings and background tasks**
  - Action: spoof, flood/quota, stale action, provider loss, lock privacy, cancel race and restart/history persistence.
  - Requires: C-VX-15.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-15.
- [ ] **C-VX-15.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-15. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-15.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-16"></a>
## C-VX-16 — Permission portals and chooser patterns

**Original requirement:** Permission portals and chooser patterns

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 335.

### Preserved original contract

**Depends on:** VX-07, VX-10, handle authority, service layer.

**Deliver:** open/save/open-with/share, clipboard, secrets, notification,
capture, camera, microphone, device and location portals.

**Invariants:** grant is narrow, user-visible, revocable and bound to live app
identity; cancel returns no handle; selected resource cannot change under name;
session/peer death revokes transient grants.

**Proof:** confused-deputy attempts, stale dialog/app, selection swap, cancel/
timeout, revoke in use, lock/logout and wrong-app handle reuse.

### Execution steps

- [ ] **C-VX-16.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-16.02 — Resolve this contract's exact dependencies**
  - Action: VX-07, VX-10, handle authority, service layer. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-16.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-16.03 — Deliver — Permission portals and chooser patterns**
  - Action: open/save/open-with/share, clipboard, secrets, notification, capture, camera, microphone, device and location portals.
  - Requires: C-VX-16.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-16.
- [ ] **C-VX-16.04 — Invariants — Permission portals and chooser patterns**
  - Action: grant is narrow, user-visible, revocable and bound to live app identity; cancel returns no handle; selected resource cannot change under name; session/peer death revokes transient grants.
  - Requires: C-VX-16.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-16.
- [ ] **C-VX-16.05 — Proof — Permission portals and chooser patterns**
  - Action: confused-deputy attempts, stale dialog/app, selection swap, cancel/ timeout, revoke in use, lock/logout and wrong-app handle reuse.
  - Requires: C-VX-16.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-16.
- [ ] **C-VX-16.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-16. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-16.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-18"></a>
## C-VX-18 — Application lifecycle, crash and restore

**Original requirement:** Application lifecycle, crash and restore

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 365.

### Preserved original contract

**Depends on:** VX-11, VX-17, supervisor.

**Deliver:** launch/readiness/background/suspend/stop/exit/crash/restore/
quarantine state machine and versioned restore snapshots.

**Invariants:** ready nonce matches process identity; close deadline is bounded;
unsaved negotiation cannot stall session forever; crash loop backs off; exit
proves all child processes/handles/grants/surfaces gone.

**Proof:** wrong PID/nonce, start timeout, crash at every state, kill parent/
child, incompatible restore, repeated crash, logout deadline and leak census.

### Execution steps

- [ ] **C-VX-18.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-18.02 — Resolve this contract's exact dependencies**
  - Action: VX-11, VX-17, supervisor. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-18.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-18.03 — Deliver — Application lifecycle, crash and restore**
  - Action: launch/readiness/background/suspend/stop/exit/crash/restore/ quarantine state machine and versioned restore snapshots.
  - Requires: C-VX-18.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-18.
- [ ] **C-VX-18.04 — Invariants — Application lifecycle, crash and restore**
  - Action: ready nonce matches process identity; close deadline is bounded; unsaved negotiation cannot stall session forever; crash loop backs off; exit proves all child processes/handles/grants/surfaces gone.
  - Requires: C-VX-18.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-18.
- [ ] **C-VX-18.05 — Proof — Application lifecycle, crash and restore**
  - Action: wrong PID/nonce, start timeout, crash at every state, kill parent/ child, incompatible restore, repeated crash, logout deadline and leak census.
  - Requires: C-VX-18.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-18.
- [ ] **C-VX-18.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-18. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-18.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-19"></a>
## C-VX-19 — App catalogue/search/provenance

**Original requirement:** App catalogue/search/provenance

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 379.

### Preserved original contract

**Depends on:** VX-01, VX-17, VX-18.

**Deliver:** generated searchable catalogue with categories, capabilities,
provenance, install/launch state, accessibility metadata and launch receipts.

**Invariants:** catalogue is derived from shipped admitted artifacts; no static
dense-range inference; hidden/disabled/incompatible apps explain why; search
cannot launch ungranted operations.

**Proof:** every registry mutation from VX-01, package removal/update, corrupt
icon, incompatible app, launch crash and provenance mismatch.

### Execution steps

- [ ] **C-VX-19.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-19.02 — Resolve this contract's exact dependencies**
  - Action: VX-01, VX-17, VX-18. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-19.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-19.03 — Deliver — App catalogue/search/provenance**
  - Action: generated searchable catalogue with categories, capabilities, provenance, install/launch state, accessibility metadata and launch receipts.
  - Requires: C-VX-19.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-19.
- [ ] **C-VX-19.04 — Invariants — App catalogue/search/provenance**
  - Action: catalogue is derived from shipped admitted artifacts; no static dense-range inference; hidden/disabled/incompatible apps explain why; search cannot launch ungranted operations.
  - Requires: C-VX-19.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-19.
- [ ] **C-VX-19.05 — Proof — App catalogue/search/provenance**
  - Action: every registry mutation from VX-01, package removal/update, corrupt icon, incompatible app, launch crash and provenance mismatch.
  - Requires: C-VX-19.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-19.
- [ ] **C-VX-19.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-19. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-19.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-svc-038"></a>
## T-SVC-038 — MIME/Type Registry

**Original requirement:** sniffing bounds, extension policy and handlers

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 57.

### Execution steps

- [ ] **T-SVC-038.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve MIME/Type Registry to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-038.02 — Specify the complete target boundary**
  - Action: MIME/Type Registry must supply: sniffing bounds, extension policy and handlers. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-038.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-038.03 — Implement the exact target behavior**
  - Action: Implement or reuse MIME/Type Registry through the shared platform contract, delivering every part of: sniffing bounds, extension policy and handlers. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-038.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-038.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-038.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for MIME/Type Registry.
- [ ] **T-SVC-038.05 — Qualify and retain this target's own result**
  - Action: Bind MIME/Type Registry to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-038.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-039"></a>
## T-SVC-039 — Thumbnail Service

**Original requirement:** isolated decoders, cache, quotas and privacy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 58.

### Execution steps

- [ ] **T-SVC-039.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Thumbnail Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-039.02 — Specify the complete target boundary**
  - Action: Thumbnail Service must supply: isolated decoders, cache, quotas and privacy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-039.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-039.03 — Implement the exact target behavior**
  - Action: Implement or reuse Thumbnail Service through the shared platform contract, delivering every part of: isolated decoders, cache, quotas and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-039.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-039.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-039.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Thumbnail Service.
- [ ] **T-SVC-039.05 — Qualify and retain this target's own result**
  - Action: Bind Thumbnail Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-039.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-040"></a>
## T-SVC-040 — Search/Index Service

**Original requirement:** content/metadata index, permissions and cancellation

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 59.

### Execution steps

- [ ] **T-SVC-040.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Search/Index Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-040.02 — Specify the complete target boundary**
  - Action: Search/Index Service must supply: content/metadata index, permissions and cancellation. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-040.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-040.03 — Implement the exact target behavior**
  - Action: Implement or reuse Search/Index Service through the shared platform contract, delivering every part of: content/metadata index, permissions and cancellation. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-040.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-040.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-040.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Search/Index Service.
- [ ] **T-SVC-040.05 — Qualify and retain this target's own result**
  - Action: Bind Search/Index Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-040.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-044"></a>
## T-SVC-044 — File Portal

**Original requirement:** user-mediated open/save/directory grants with revocation

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 63.

### Execution steps

- [ ] **T-SVC-044.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve File Portal to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-044.02 — Specify the complete target boundary**
  - Action: File Portal must supply: user-mediated open/save/directory grants with revocation. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-044.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-044.03 — Implement the exact target behavior**
  - Action: Implement or reuse File Portal through the shared platform contract, delivering every part of: user-mediated open/save/directory grants with revocation. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-044.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-044.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-044.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for File Portal.
- [ ] **T-SVC-044.05 — Qualify and retain this target's own result**
  - Action: Bind File Portal to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-044.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-045"></a>
## T-SVC-045 — Archive Worker

**Original requirement:** length-first unpack/list/create in restricted process

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 64.

### Execution steps

- [ ] **T-SVC-045.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Archive Worker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-045.02 — Specify the complete target boundary**
  - Action: Archive Worker must supply: length-first unpack/list/create in restricted process. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-045.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-045.03 — Implement the exact target behavior**
  - Action: Implement or reuse Archive Worker through the shared platform contract, delivering every part of: length-first unpack/list/create in restricted process. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-045.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-045.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-045.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Archive Worker.
- [ ] **T-SVC-045.05 — Qualify and retain this target's own result**
  - Action: Bind Archive Worker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-045.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-076"></a>
## T-SVC-076 — Application Registry/Launcher

**Original requirement:** admitted manifests, exact IDs/routes and launch receipts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 96.

### Execution steps

- [ ] **T-SVC-076.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Application Registry/Launcher to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-076.02 — Specify the complete target boundary**
  - Action: Application Registry/Launcher must supply: admitted manifests, exact IDs/routes and launch receipts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-076.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-076.03 — Implement the exact target behavior**
  - Action: Implement or reuse Application Registry/Launcher through the shared platform contract, delivering every part of: admitted manifests, exact IDs/routes and launch receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-076.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-076.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-076.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Application Registry/Launcher.
- [ ] **T-SVC-076.05 — Qualify and retain this target's own result**
  - Action: Bind Application Registry/Launcher to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-076.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-100"></a>
## T-SVC-100 — Settings Service

**Original requirement:** schemas, per-user/system scopes, preview and rollback

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 126.

### Execution steps

- [ ] **T-SVC-100.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Settings Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-100.02 — Specify the complete target boundary**
  - Action: Settings Service must supply: schemas, per-user/system scopes, preview and rollback. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-100.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-100.03 — Implement the exact target behavior**
  - Action: Implement or reuse Settings Service through the shared platform contract, delivering every part of: schemas, per-user/system scopes, preview and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-100.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-100.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-100.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Settings Service.
- [ ] **T-SVC-100.05 — Qualify and retain this target's own result**
  - Action: Bind Settings Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-100.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-101"></a>
## T-SVC-101 — Clipboard Broker

**Original requirement:** MIME offers, history policy, consent and owner death

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 127.

### Execution steps

- [ ] **T-SVC-101.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Clipboard Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-101.02 — Specify the complete target boundary**
  - Action: Clipboard Broker must supply: MIME offers, history policy, consent and owner death. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-101.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-101.03 — Implement the exact target behavior**
  - Action: Implement or reuse Clipboard Broker through the shared platform contract, delivering every part of: MIME offers, history policy, consent and owner death. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-101.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-101.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-101.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Clipboard Broker.
- [ ] **T-SVC-101.05 — Qualify and retain this target's own result**
  - Action: Bind Clipboard Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-101.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-102"></a>
## T-SVC-102 — Notification Service

**Original requirement:** actions, grouping, quiet modes, persistence and a11y

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 128.

### Execution steps

- [ ] **T-SVC-102.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Notification Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-102.02 — Specify the complete target boundary**
  - Action: Notification Service must supply: actions, grouping, quiet modes, persistence and a11y. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-102.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-102.03 — Implement the exact target behavior**
  - Action: Implement or reuse Notification Service through the shared platform contract, delivering every part of: actions, grouping, quiet modes, persistence and a11y. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-102.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-102.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-102.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Notification Service.
- [ ] **T-SVC-102.05 — Qualify and retain this target's own result**
  - Action: Bind Notification Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-102.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-103"></a>
## T-SVC-103 — Open/Save/Share Portal

**Original requirement:** mediated target selection and capability grants

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 129.

### Execution steps

- [ ] **T-SVC-103.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Open/Save/Share Portal to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-103.02 — Specify the complete target boundary**
  - Action: Open/Save/Share Portal must supply: mediated target selection and capability grants. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-103.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-103.03 — Implement the exact target behavior**
  - Action: Implement or reuse Open/Save/Share Portal through the shared platform contract, delivering every part of: mediated target selection and capability grants. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-103.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-103.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-103.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Open/Save/Share Portal.
- [ ] **T-SVC-103.05 — Qualify and retain this target's own result**
  - Action: Bind Open/Save/Share Portal to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-103.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-106"></a>
## T-SVC-106 — Document/Autosave Service

**Original requirement:** revisions, recovery, conflicts and export

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 132.

### Execution steps

- [ ] **T-SVC-106.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Document/Autosave Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-106.02 — Specify the complete target boundary**
  - Action: Document/Autosave Service must supply: revisions, recovery, conflicts and export. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-106.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-106.03 — Implement the exact target behavior**
  - Action: Implement or reuse Document/Autosave Service through the shared platform contract, delivering every part of: revisions, recovery, conflicts and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-106.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-106.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-106.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Document/Autosave Service.
- [ ] **T-SVC-106.05 — Qualify and retain this target's own result**
  - Action: Bind Document/Autosave Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-106.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-107"></a>
## T-SVC-107 — Undo/History Service Library

**Original requirement:** bounded commands, merge, persistent recovery

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 133.

### Execution steps

- [ ] **T-SVC-107.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Undo/History Service Library to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-107.02 — Specify the complete target boundary**
  - Action: Undo/History Service Library must supply: bounded commands, merge, persistent recovery. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-107.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-107.03 — Implement the exact target behavior**
  - Action: Implement or reuse Undo/History Service Library through the shared platform contract, delivering every part of: bounded commands, merge, persistent recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-107.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-107.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-107.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Undo/History Service Library.
- [ ] **T-SVC-107.05 — Qualify and retain this target's own result**
  - Action: Bind Undo/History Service Library to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-107.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-114"></a>
## T-SVC-114 — Background Task Scheduler

**Original requirement:** quotas, deadlines, power/network policy and cancellation

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 140.

### Execution steps

- [ ] **T-SVC-114.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Background Task Scheduler to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-114.02 — Specify the complete target boundary**
  - Action: Background Task Scheduler must supply: quotas, deadlines, power/network policy and cancellation. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-114.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-114.03 — Implement the exact target behavior**
  - Action: Implement or reuse Background Task Scheduler through the shared platform contract, delivering every part of: quotas, deadlines, power/network policy and cancellation. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-114.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-114.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-114.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Background Task Scheduler.
- [ ] **T-SVC-114.05 — Qualify and retain this target's own result**
  - Action: Bind Background Task Scheduler to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-114.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-115"></a>
## T-SVC-115 — Application Restore Service

**Original requirement:** windows/documents, crash/reboot semantics and consent

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 141.

### Execution steps

- [ ] **T-SVC-115.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Application Restore Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-09, D-14, D-17, H-11.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-115.02 — Specify the complete target boundary**
  - Action: Application Restore Service must supply: windows/documents, crash/reboot semantics and consent. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-115.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-115.03 — Implement the exact target behavior**
  - Action: Implement or reuse Application Restore Service through the shared platform contract, delivering every part of: windows/documents, crash/reboot semantics and consent. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-115.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-115.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-SVC-115.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Application Restore Service.
- [ ] **T-SVC-115.05 — Qualify and retain this target's own result**
  - Action: Bind Application Restore Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-115.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
