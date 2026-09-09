# MP-18: Make releases, recovery and operations trustworthy

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `docs/fleet/; .github/; gates/; kernel/tools/images/; kernel/apps/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-18` exports: An exact-artifact release/recovery rehearsal and an isolated operational control plane; publication is a separate authorized action.

The handoff enables only its named subset. `CLOSE-18` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-18.01 — Define release profiles, feature/target claim manifests, channels, signing custody and compatibility policy

Define release profiles, feature/target claim manifests, channels, signing custody and compatibility policy.

**Requires:** `D-01`, `D-02`, `D-03`, `D-10`, `D-24`, `H-00`, `H-02`, `H-04`, `H-06`, `H-08`, `H-11`, `H-17`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.02 — Build reproducible image/package generations with source/dependency/license closure and retained prior releases

Build reproducible image/package generations with source/dependency/license closure and retained prior releases.

**Requires:** `M-18.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.03 — Rehearse install/update/rollback and data migration on disposable targets before any real deployment

Rehearse install/update/rollback and data migration on disposable targets before any real deployment.

**Requires:** `M-18.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-18 — Bounded development handoff: Make releases, recovery and operations trustworthy

An exact-artifact release/recovery rehearsal and an isolated operational control plane; publication is a separate authorized action.

**Requires:** `M-18.03`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-18.04 — Implement transactional demo leases, authenticated transport, resource/egress limits and immutable per-lease overlays

Implement transactional demo leases, authenticated transport, resource/egress limits and immutable per-lease overlays.

**Requires:** `M-18.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.05 — Prove failure at every reserve/launch/proxy/stop/reap step returns to the exact resource baseline

Prove failure at every reserve/launch/proxy/stop/reap step returns to the exact resource baseline.

**Requires:** `M-18.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.06 — Provide consented redacted support/crash bundles, truthful health/provenance viewers and read-only-first repair

Provide consented redacted support/crash bundles, truthful health/provenance viewers and read-only-first repair.

**Requires:** `M-18.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.07 — Run CI, hostile/fuzz, performance, security, accessibility, visual and physical qualification for every advertised release surface

Run CI, hostile/fuzz, performance, security, accessibility, visual and physical qualification for every advertised release surface.

**Requires:** `M-18.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.08 — Record operational runbooks, incident/revocation response, retention, recovery drills and maintenance ownership

Record operational runbooks, incident/revocation response, retention, recovery drills and maintenance ownership.

**Requires:** `M-18.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-18.09 — Require explicit approval for publication, signing authority use and destructive target operations

Require explicit approval for publication, signing authority use and destructive target operations; no planning task itself grants those permissions.

**Requires:** `M-18.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-OP-001](#f-op-001) | feature | release build pipeline |
| [F-OP-002](#f-op-002) | feature | release channels |
| [F-OP-003](#f-op-003) | feature | image publishing |
| [F-OP-004](#f-op-004) | feature | upgrade migration |
| [F-OP-005](#f-op-005) | feature | telemetry policy |
| [F-OP-006](#f-op-006) | feature | metrics/health |
| [F-OP-007](#f-op-007) | feature | log collection |
| [F-OP-008](#f-op-008) | feature | alerting |
| [F-OP-009](#f-op-009) | feature | public landing page |
| [F-OP-010](#f-op-010) | feature | demo capacity |
| [F-OP-011](#f-op-011) | feature | transactional demo lease |
| [F-OP-012](#f-op-012) | feature | disposable guest overlay |
| [F-OP-013](#f-op-013) | feature | supervised QEMU guest |
| [F-OP-014](#f-op-014) | feature | authenticated VNC/WebSocket route |
| [F-OP-015](#f-op-015) | feature | demo session states |
| [F-OP-016](#f-op-016) | feature | demo input UX |
| [F-OP-017](#f-op-017) | feature | end/release action |
| [F-OP-018](#f-op-018) | feature | reaper |
| [F-OP-019](#f-op-019) | feature | reconnect |
| [F-OP-020](#f-op-020) | feature | demo administration |
| [F-OP-021](#f-op-021) | feature | abuse/rate protection |
| [F-OP-022](#f-op-022) | feature | service backup/restore |
| [F-OP-023](#f-op-023) | feature | incident mode |
| [F-OP-024](#f-op-024) | feature | support documentation |
| [F-OP-025](#f-op-025) | feature | issue/report workflow |
| [F-OP-026](#f-op-026) | feature | compatibility/hardware matrix |
| [F-OP-027](#f-op-027) | feature | rescue media distribution |
| [F-OP-028](#f-op-028) | feature | supply-chain incident response |
| [F-OP-029](#f-op-029) | feature | end-of-life policy |
| [F-OP-030](#f-op-030) | feature | public evidence portal |
| [C-P11.1](#c-p11-1) | contract | transactional lease allocator |
| [C-P11.2](#c-p11-2) | contract | allowlisted QEMU launch and immutable overlays |
| [C-P11.3](#c-p11-3) | contract | per-lease proxy authentication |
| [C-P11.4](#c-p11-4) | contract | resource and egress isolation |
| [C-P11.5](#c-p11-5) | contract | graceful stop, reaper and zero-orphan audit |
| [C-DA-37](#c-da-37) | contract | rescue and repair environment |
| [C-DA-38](#c-da-38) | contract | transactional public QEMU demo |
| [C-VX-47](#c-vx-47) | contract | Rescue and public-demo experiences |
| [C-VX-52](#c-vx-52) | contract | QEMU product matrix |
| [T-PLAT-031](#t-plat-031) | target | watchdog |
| [T-NIC-016](#t-nic-016) | target | bridge/VLAN interface |
| [T-SVC-012](#t-svc-012) | target | Crash/Recovery Manager |
| [T-SVC-061](#t-svc-061) | target | Remote Desktop Service |
| [T-SVC-135](#t-svc-135) | target | Release/Update Orchestrator |
| [T-SVC-136](#t-svc-136) | target | Public Demo Lease Manager |
| [T-SVC-137](#t-svc-137) | target | Authenticated VNC/WebSocket Proxy |
| [T-SVC-138](#t-svc-138) | target | Telemetry/Health Service |
| [T-SVC-139](#t-svc-139) | target | Support Bundle Service |
| [T-SVC-140](#t-svc-140) | target | Rescue/Repair Orchestrator |
| [T-SVC-141](#t-svc-141) | target | Feature/Provenance Viewer Provider |
| [T-APP-016](#t-app-016) | target | Feature/Provenance Viewer |
| [T-APP-100](#t-app-100) | target | Rescue Desktop |
| [T-APP-101](#t-app-101) | target | Backup/Restore |
| [T-APP-102](#t-app-102) | target | Boot Generation Manager |
| [T-APP-103](#t-app-103) | target | Installation/Deployment UI |
| [T-APP-104](#t-app-104) | target | Public Demo Portal |
| [T-APP-105](#t-app-105) | target | Support Bundle |
| [T-OPS-001](#t-ops-001) | target | hermetic release build |
| [T-OPS-002](#t-ops-002) | target | artifact identity/SBOM |
| [T-OPS-003](#t-ops-003) | target | signing and trust policy |
| [T-OPS-004](#t-ops-004) | target | release channels |
| [T-OPS-005](#t-ops-005) | target | transactional updater |
| [T-OPS-006](#t-ops-006) | target | boot generations |
| [T-OPS-007](#t-ops-007) | target | migration framework |
| [T-OPS-008](#t-ops-008) | target | release notes |
| [T-OPS-009](#t-ops-009) | target | CI build matrix |
| [T-OPS-010](#t-ops-010) | target | host deterministic matrix |
| [T-OPS-011](#t-ops-011) | target | QEMU system matrix |
| [T-OPS-012](#t-ops-012) | target | native hardware matrix |
| [T-OPS-013](#t-ops-013) | target | performance promotion |
| [T-OPS-014](#t-ops-014) | target | security promotion |
| [T-OPS-015](#t-ops-015) | target | accessibility promotion |
| [T-OPS-016](#t-ops-016) | target | visual promotion |
| [T-OPS-017](#t-ops-017) | target | crash/telemetry pipeline |
| [T-OPS-018](#t-ops-018) | target | support bundles |
| [T-OPS-019](#t-ops-019) | target | backup/disaster recovery drill |
| [T-OPS-020](#t-ops-020) | target | rescue image |
| [T-OPS-021](#t-ops-021) | target | public demo allocator |
| [T-OPS-022](#t-ops-022) | target | QEMU launcher |
| [T-OPS-023](#t-ops-023) | target | VNC/WebSocket authentication |
| [T-OPS-024](#t-ops-024) | target | demo containment |
| [T-OPS-025](#t-ops-025) | target | demo reaper |
| [T-OPS-026](#t-ops-026) | target | provenance/feature viewer |
| [T-OPS-027](#t-ops-027) | target | deprecation/support policy |
| [T-OPS-028](#t-ops-028) | target | independent release review |

<a id="f-op-001"></a>
## F-OP-001 — release build pipeline

**Original requirement:** clean locked build, tests, signatures, SBOM/provenance and immutable artifacts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-001.01 — Reconcile existing release build pipeline**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for release build pipeline. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: clean locked build, tests, signatures, SBOM/provenance and immutable artifacts
- [ ] **F-OP-001.02 — Freeze the exact contract for release build pipeline**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: clean locked build, tests, signatures, SBOM/provenance and immutable artifacts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-001.03 — Implement/prove: clean locked build**
  - Action: For release build pipeline, implement or reuse and verify this exact obligation: clean locked build. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clean locked build; retain observable state/resource expectations.
- [ ] **F-OP-001.04 — Implement/prove: tests**
  - Action: For release build pipeline, implement or reuse and verify this exact obligation: tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tests; retain observable state/resource expectations.
- [ ] **F-OP-001.05 — Implement/prove: signatures**
  - Action: For release build pipeline, implement or reuse and verify this exact obligation: signatures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signatures; retain observable state/resource expectations.
- [ ] **F-OP-001.06 — Implement/prove: SBOM/provenance and immutable artifacts**
  - Action: For release build pipeline, implement or reuse and verify this exact obligation: SBOM/provenance and immutable artifacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for SBOM/provenance and immutable artifacts; retain observable state/resource expectations.
- [ ] **F-OP-001.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to release build pipeline: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-001.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-001.08 — Integrate into the real consumer and runtime route**
  - Action: Wire release build pipeline into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-001.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-001.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for release build pipeline as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-001.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-002"></a>
## F-OP-002 — release channels

**Original requirement:** stable/testing/developer policy, compatibility, rollback and user choice

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-002.01 — Reconcile existing release channels**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for release channels. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stable/testing/developer policy, compatibility, rollback and user choice
- [ ] **F-OP-002.02 — Freeze the exact contract for release channels**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stable/testing/developer policy, compatibility, rollback and user choice. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-002.03 — Implement/prove: stable/testing/developer policy**
  - Action: For release channels, implement or reuse and verify this exact obligation: stable/testing/developer policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stable/testing/developer policy; retain observable state/resource expectations.
- [ ] **F-OP-002.04 — Implement/prove: compatibility**
  - Action: For release channels, implement or reuse and verify this exact obligation: compatibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compatibility; retain observable state/resource expectations.
- [ ] **F-OP-002.05 — Implement/prove: rollback and user choice**
  - Action: For release channels, implement or reuse and verify this exact obligation: rollback and user choice. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rollback and user choice; retain observable state/resource expectations.
- [ ] **F-OP-002.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to release channels: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-002.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-002.07 — Integrate into the real consumer and runtime route**
  - Action: Wire release channels into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-002.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-002.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for release channels as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-002.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-003"></a>
## F-OP-003 — image publishing

**Original requirement:** signed raw/ISO/VM/hardware artifacts with hashes, instructions and origin

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-003.01 — Reconcile existing image publishing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for image publishing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed raw/ISO/VM/hardware artifacts with hashes, instructions and origin
- [ ] **F-OP-003.02 — Freeze the exact contract for image publishing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed raw/ISO/VM/hardware artifacts with hashes, instructions and origin. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-003.03 — Implement/prove: signed raw/ISO/VM/hardware artifacts with hashes**
  - Action: For image publishing, implement or reuse and verify this exact obligation: signed raw/ISO/VM/hardware artifacts with hashes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed raw/ISO/VM/hardware artifacts with hashes; retain observable state/resource expectations.
- [ ] **F-OP-003.04 — Implement/prove: instructions and origin**
  - Action: For image publishing, implement or reuse and verify this exact obligation: instructions and origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for instructions and origin; retain observable state/resource expectations.
- [ ] **F-OP-003.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to image publishing: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-003.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-003.06 — Integrate into the real consumer and runtime route**
  - Action: Wire image publishing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-003.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-003.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for image publishing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-003.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-004"></a>
## F-OP-004 — upgrade migration

**Original requirement:** settings/data/package/schema transition, failure rollback and recovery docs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-004.01 — Reconcile existing upgrade migration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for upgrade migration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: settings/data/package/schema transition, failure rollback and recovery docs
- [ ] **F-OP-004.02 — Freeze the exact contract for upgrade migration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: settings/data/package/schema transition, failure rollback and recovery docs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-004.03 — Implement/prove: settings/data/package/schema transition**
  - Action: For upgrade migration, implement or reuse and verify this exact obligation: settings/data/package/schema transition. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for settings/data/package/schema transition; retain observable state/resource expectations.
- [ ] **F-OP-004.04 — Implement/prove: failure rollback and recovery docs**
  - Action: For upgrade migration, implement or reuse and verify this exact obligation: failure rollback and recovery docs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failure rollback and recovery docs; retain observable state/resource expectations.
- [ ] **F-OP-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to upgrade migration: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire upgrade migration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for upgrade migration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-005"></a>
## F-OP-005 — telemetry policy

**Original requirement:** opt-in/essential split, schema, minimization, retention, export/delete and no secrets

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-005.01 — Reconcile existing telemetry policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for telemetry policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: opt-in/essential split, schema, minimization, retention, export/delete and no secrets
- [ ] **F-OP-005.02 — Freeze the exact contract for telemetry policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: opt-in/essential split, schema, minimization, retention, export/delete and no secrets. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-005.03 — Implement/prove: opt-in/essential split**
  - Action: For telemetry policy, implement or reuse and verify this exact obligation: opt-in/essential split. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opt-in/essential split; retain observable state/resource expectations.
- [ ] **F-OP-005.04 — Implement/prove: schema**
  - Action: For telemetry policy, implement or reuse and verify this exact obligation: schema. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for schema; retain observable state/resource expectations.
- [ ] **F-OP-005.05 — Implement/prove: minimization**
  - Action: For telemetry policy, implement or reuse and verify this exact obligation: minimization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimization; retain observable state/resource expectations.
- [ ] **F-OP-005.06 — Implement/prove: retention**
  - Action: For telemetry policy, implement or reuse and verify this exact obligation: retention. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retention; retain observable state/resource expectations.
- [ ] **F-OP-005.07 — Implement/prove: export/delete and no secrets**
  - Action: For telemetry policy, implement or reuse and verify this exact obligation: export/delete and no secrets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-005.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for export/delete and no secrets; retain observable state/resource expectations.
- [ ] **F-OP-005.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to telemetry policy: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-005.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-005.09 — Integrate into the real consumer and runtime route**
  - Action: Wire telemetry policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-005.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-005.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for telemetry policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-005.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-006"></a>
## F-OP-006 — metrics/health

**Original requirement:** service/device/session/resource health with stable units and drop evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-006.01 — Reconcile existing metrics/health**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for metrics/health. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: service/device/session/resource health with stable units and drop evidence
- [ ] **F-OP-006.02 — Freeze the exact contract for metrics/health**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: service/device/session/resource health with stable units and drop evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-006.03 — Implement/prove: service/device/session/resource health with stable units and drop evidence**
  - Action: For metrics/health, implement or reuse and verify this exact obligation: service/device/session/resource health with stable units and drop evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for service/device/session/resource health with stable units and drop evidence; retain observable state/resource expectations.
- [ ] **F-OP-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to metrics/health: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire metrics/health into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for metrics/health as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-007"></a>
## F-OP-007 — log collection

**Original requirement:** bounded local logs, rotation, crash survival, redaction and user export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-007.01 — Reconcile existing log collection**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for log collection. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded local logs, rotation, crash survival, redaction and user export
- [ ] **F-OP-007.02 — Freeze the exact contract for log collection**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded local logs, rotation, crash survival, redaction and user export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-007.03 — Implement/prove: bounded local logs**
  - Action: For log collection, implement or reuse and verify this exact obligation: bounded local logs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded local logs; retain observable state/resource expectations.
- [ ] **F-OP-007.04 — Implement/prove: rotation**
  - Action: For log collection, implement or reuse and verify this exact obligation: rotation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rotation; retain observable state/resource expectations.
- [ ] **F-OP-007.05 — Implement/prove: crash survival**
  - Action: For log collection, implement or reuse and verify this exact obligation: crash survival. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crash survival; retain observable state/resource expectations.
- [ ] **F-OP-007.06 — Implement/prove: redaction and user export**
  - Action: For log collection, implement or reuse and verify this exact obligation: redaction and user export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for redaction and user export; retain observable state/resource expectations.
- [ ] **F-OP-007.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to log collection: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-007.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-007.08 — Integrate into the real consumer and runtime route**
  - Action: Wire log collection into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-007.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-007.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for log collection as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-007.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-008"></a>
## F-OP-008 — alerting

**Original requirement:** actionable severity/dedup/rate limits/ownership and recovery link

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-008.01 — Reconcile existing alerting**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for alerting. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: actionable severity/dedup/rate limits/ownership and recovery link
- [ ] **F-OP-008.02 — Freeze the exact contract for alerting**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: actionable severity/dedup/rate limits/ownership and recovery link. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-008.03 — Implement/prove: actionable severity/dedup/rate limits/ownership and recovery link**
  - Action: For alerting, implement or reuse and verify this exact obligation: actionable severity/dedup/rate limits/ownership and recovery link. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for actionable severity/dedup/rate limits/ownership and recovery link; retain observable state/resource expectations.
- [ ] **F-OP-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to alerting: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire alerting into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for alerting as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-009"></a>
## F-OP-009 — public landing page

**Original requirement:** truthful product/artifact/requirements/status/accessibility and start path

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-009.01 — Reconcile existing public landing page**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for public landing page. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: truthful product/artifact/requirements/status/accessibility and start path
- [ ] **F-OP-009.02 — Freeze the exact contract for public landing page**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: truthful product/artifact/requirements/status/accessibility and start path. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-009.03 — Implement/prove: truthful product/artifact/requirements/status/accessibility and start path**
  - Action: For public landing page, implement or reuse and verify this exact obligation: truthful product/artifact/requirements/status/accessibility and start path. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for truthful product/artifact/requirements/status/accessibility and start path; retain observable state/resource expectations.
- [ ] **F-OP-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to public landing page: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire public landing page into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for public landing page as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-010"></a>
## F-OP-010 — demo capacity

**Original requirement:** slots, CPU/RAM/PID/disk/egress quotas and visible availability

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-010.01 — Reconcile existing demo capacity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for demo capacity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: slots, CPU/RAM/PID/disk/egress quotas and visible availability
- [ ] **F-OP-010.02 — Freeze the exact contract for demo capacity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: slots, CPU/RAM/PID/disk/egress quotas and visible availability. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-010.03 — Implement/prove: slots**
  - Action: For demo capacity, implement or reuse and verify this exact obligation: slots. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for slots; retain observable state/resource expectations.
- [ ] **F-OP-010.04 — Implement/prove: CPU/RAM/PID/disk/egress quotas and visible availability**
  - Action: For demo capacity, implement or reuse and verify this exact obligation: CPU/RAM/PID/disk/egress quotas and visible availability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CPU/RAM/PID/disk/egress quotas and visible availability; retain observable state/resource expectations.
- [ ] **F-OP-010.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to demo capacity: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-010.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-010.06 — Integrate into the real consumer and runtime route**
  - Action: Wire demo capacity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-010.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-010.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for demo capacity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-010.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-011"></a>
## F-OP-011 — transactional demo lease

**Original requirement:** allocate/overlay/launch/publish atomically or rollback every resource

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-011.01 — Reconcile existing transactional demo lease**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for transactional demo lease. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: allocate/overlay/launch/publish atomically or rollback every resource
- [ ] **F-OP-011.02 — Freeze the exact contract for transactional demo lease**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: allocate/overlay/launch/publish atomically or rollback every resource. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-011.03 — Implement/prove: allocate/overlay/launch/publish atomically or rollback every resource**
  - Action: For transactional demo lease, implement or reuse and verify this exact obligation: allocate/overlay/launch/publish atomically or rollback every resource. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allocate/overlay/launch/publish atomically or rollback every resource; retain observable state/resource expectations.
- [ ] **F-OP-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to transactional demo lease: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire transactional demo lease into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for transactional demo lease as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-012"></a>
## F-OP-012 — disposable guest overlay

**Original requirement:** known base, per-session writable layer, quota and verified deletion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-012.01 — Reconcile existing disposable guest overlay**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for disposable guest overlay. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: known base, per-session writable layer, quota and verified deletion
- [ ] **F-OP-012.02 — Freeze the exact contract for disposable guest overlay**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: known base, per-session writable layer, quota and verified deletion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-012.03 — Implement/prove: known base**
  - Action: For disposable guest overlay, implement or reuse and verify this exact obligation: known base. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for known base; retain observable state/resource expectations.
- [ ] **F-OP-012.04 — Implement/prove: per-session writable layer**
  - Action: For disposable guest overlay, implement or reuse and verify this exact obligation: per-session writable layer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-session writable layer; retain observable state/resource expectations.
- [ ] **F-OP-012.05 — Implement/prove: quota and verified deletion**
  - Action: For disposable guest overlay, implement or reuse and verify this exact obligation: quota and verified deletion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quota and verified deletion; retain observable state/resource expectations.
- [ ] **F-OP-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to disposable guest overlay: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire disposable guest overlay into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for disposable guest overlay as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-013"></a>
## F-OP-013 — supervised QEMU guest

**Original requirement:** exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-013.01 — Reconcile existing supervised QEMU guest**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for supervised QEMU guest. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion
- [ ] **F-OP-013.02 — Freeze the exact contract for supervised QEMU guest**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-013.03 — Implement/prove: exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion**
  - Action: For supervised QEMU guest, implement or reuse and verify this exact obligation: exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact PID/start time/artifact/resources/readiness/death and no PID reuse confusion; retain observable state/resource expectations.
- [ ] **F-OP-013.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to supervised QEMU guest: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-013.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-013.05 — Integrate into the real consumer and runtime route**
  - Action: Wire supervised QEMU guest into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-013.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-013.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for supervised QEMU guest as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-013.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-014"></a>
## F-OP-014 — authenticated VNC/WebSocket route

**Original requirement:** unenumerable per-lease data plane with expiry/revoke on every action

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-014.01 — Reconcile existing authenticated VNC/WebSocket route**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for authenticated VNC/WebSocket route. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: unenumerable per-lease data plane with expiry/revoke on every action
- [ ] **F-OP-014.02 — Freeze the exact contract for authenticated VNC/WebSocket route**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: unenumerable per-lease data plane with expiry/revoke on every action. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-014.03 — Implement/prove: unenumerable per-lease data plane with expiry/revoke on every action**
  - Action: For authenticated VNC/WebSocket route, implement or reuse and verify this exact obligation: unenumerable per-lease data plane with expiry/revoke on every action. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unenumerable per-lease data plane with expiry/revoke on every action; retain observable state/resource expectations.
- [ ] **F-OP-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to authenticated VNC/WebSocket route: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire authenticated VNC/WebSocket route into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for authenticated VNC/WebSocket route as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-015"></a>
## F-OP-015 — demo session states

**Original requirement:** starting/ready/reconnecting/failed/expiring/ended with server authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-015.01 — Reconcile existing demo session states**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for demo session states. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: starting/ready/reconnecting/failed/expiring/ended with server authority
- [ ] **F-OP-015.02 — Freeze the exact contract for demo session states**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: starting/ready/reconnecting/failed/expiring/ended with server authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-015.03 — Implement/prove: starting/ready/reconnecting/failed/expiring/ended with server authority**
  - Action: For demo session states, implement or reuse and verify this exact obligation: starting/ready/reconnecting/failed/expiring/ended with server authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for starting/ready/reconnecting/failed/expiring/ended with server authority; retain observable state/resource expectations.
- [ ] **F-OP-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to demo session states: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire demo session states into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for demo session states as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-016"></a>
## F-OP-016 — demo input UX

**Original requirement:** focus/release/stuck-key/mobile keyboard/clipboard policy and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-016.01 — Reconcile existing demo input UX**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for demo input UX. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: focus/release/stuck-key/mobile keyboard/clipboard policy and a11y
- [ ] **F-OP-016.02 — Freeze the exact contract for demo input UX**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: focus/release/stuck-key/mobile keyboard/clipboard policy and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-016.03 — Implement/prove: focus/release/stuck-key/mobile keyboard/clipboard policy and a11y**
  - Action: For demo input UX, implement or reuse and verify this exact obligation: focus/release/stuck-key/mobile keyboard/clipboard policy and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus/release/stuck-key/mobile keyboard/clipboard policy and a11y; retain observable state/resource expectations.
- [ ] **F-OP-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to demo input UX: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire demo input UX into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for demo input UX as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-017"></a>
## F-OP-017 — end/release action

**Original requirement:** explicit idempotent user termination and verified process/slot cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-017.01 — Reconcile existing end/release action**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for end/release action. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit idempotent user termination and verified process/slot cleanup
- [ ] **F-OP-017.02 — Freeze the exact contract for end/release action**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit idempotent user termination and verified process/slot cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-017.03 — Implement/prove: explicit idempotent user termination and verified process/slot cleanup**
  - Action: For end/release action, implement or reuse and verify this exact obligation: explicit idempotent user termination and verified process/slot cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit idempotent user termination and verified process/slot cleanup; retain observable state/resource expectations.
- [ ] **F-OP-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to end/release action: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire end/release action into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for end/release action as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-018"></a>
## F-OP-018 — reaper

**Original requirement:** expiry/orphan/crash cleanup with identity checks and immutable receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-018.01 — Reconcile existing reaper**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reaper. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: expiry/orphan/crash cleanup with identity checks and immutable receipt
- [ ] **F-OP-018.02 — Freeze the exact contract for reaper**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: expiry/orphan/crash cleanup with identity checks and immutable receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-018.03 — Implement/prove: expiry/orphan/crash cleanup with identity checks and immutable receipt**
  - Action: For reaper, implement or reuse and verify this exact obligation: expiry/orphan/crash cleanup with identity checks and immutable receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for expiry/orphan/crash cleanup with identity checks and immutable receipt; retain observable state/resource expectations.
- [ ] **F-OP-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reaper: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire reaper into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reaper as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-019"></a>
## F-OP-019 — reconnect

**Original requirement:** authenticated bounded retry to same live lease, never a fake constructor loop

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-019.01 — Reconcile existing reconnect**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reconnect. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated bounded retry to same live lease, never a fake constructor loop
- [ ] **F-OP-019.02 — Freeze the exact contract for reconnect**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated bounded retry to same live lease, never a fake constructor loop. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-019.03 — Implement/prove: authenticated bounded retry to same live lease**
  - Action: For reconnect, implement or reuse and verify this exact obligation: authenticated bounded retry to same live lease. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated bounded retry to same live lease; retain observable state/resource expectations.
- [ ] **F-OP-019.04 — Implement/prove: never a fake constructor loop**
  - Action: For reconnect, implement or reuse and verify this exact obligation: never a fake constructor loop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for never a fake constructor loop; retain observable state/resource expectations.
- [ ] **F-OP-019.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reconnect: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-019.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-019.06 — Integrate into the real consumer and runtime route**
  - Action: Wire reconnect into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-019.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-019.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reconnect as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-019.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-020"></a>
## F-OP-020 — demo administration

**Original requirement:** status/leases/kill/capacity/reap with auth, JSON and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-020.01 — Reconcile existing demo administration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for demo administration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: status/leases/kill/capacity/reap with auth, JSON and audit
- [ ] **F-OP-020.02 — Freeze the exact contract for demo administration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: status/leases/kill/capacity/reap with auth, JSON and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-020.03 — Implement/prove: status/leases/kill/capacity/reap with auth**
  - Action: For demo administration, implement or reuse and verify this exact obligation: status/leases/kill/capacity/reap with auth. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for status/leases/kill/capacity/reap with auth; retain observable state/resource expectations.
- [ ] **F-OP-020.04 — Implement/prove: JSON and audit**
  - Action: For demo administration, implement or reuse and verify this exact obligation: JSON and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for JSON and audit; retain observable state/resource expectations.
- [ ] **F-OP-020.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to demo administration: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-020.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-020.06 — Integrate into the real consumer and runtime route**
  - Action: Wire demo administration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-020.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-020.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for demo administration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-020.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-021"></a>
## F-OP-021 — abuse/rate protection

**Original requirement:** start/input/network quotas, per-source policy and safe denial

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-021.01 — Reconcile existing abuse/rate protection**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for abuse/rate protection. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: start/input/network quotas, per-source policy and safe denial
- [ ] **F-OP-021.02 — Freeze the exact contract for abuse/rate protection**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: start/input/network quotas, per-source policy and safe denial. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-021.03 — Implement/prove: start/input/network quotas**
  - Action: For abuse/rate protection, implement or reuse and verify this exact obligation: start/input/network quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for start/input/network quotas; retain observable state/resource expectations.
- [ ] **F-OP-021.04 — Implement/prove: per-source policy and safe denial**
  - Action: For abuse/rate protection, implement or reuse and verify this exact obligation: per-source policy and safe denial. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-source policy and safe denial; retain observable state/resource expectations.
- [ ] **F-OP-021.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to abuse/rate protection: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-021.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-021.06 — Integrate into the real consumer and runtime route**
  - Action: Wire abuse/rate protection into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-021.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-021.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for abuse/rate protection as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-021.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-022"></a>
## F-OP-022 — service backup/restore

**Original requirement:** configuration/state/artifact backup with periodic restore drill

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-022.01 — Reconcile existing service backup/restore**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for service backup/restore. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: configuration/state/artifact backup with periodic restore drill
- [ ] **F-OP-022.02 — Freeze the exact contract for service backup/restore**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: configuration/state/artifact backup with periodic restore drill. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-022.03 — Implement/prove: configuration/state/artifact backup with periodic restore drill**
  - Action: For service backup/restore, implement or reuse and verify this exact obligation: configuration/state/artifact backup with periodic restore drill. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configuration/state/artifact backup with periodic restore drill; retain observable state/resource expectations.
- [ ] **F-OP-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to service backup/restore: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire service backup/restore into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for service backup/restore as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-023"></a>
## F-OP-023 — incident mode

**Original requirement:** preserve evidence, contain, communicate, recover and document without destroying state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-023.01 — Reconcile existing incident mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for incident mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: preserve evidence, contain, communicate, recover and document without destroying state
- [ ] **F-OP-023.02 — Freeze the exact contract for incident mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: preserve evidence, contain, communicate, recover and document without destroying state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-023.03 — Implement/prove: preserve evidence**
  - Action: For incident mode, implement or reuse and verify this exact obligation: preserve evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preserve evidence; retain observable state/resource expectations.
- [ ] **F-OP-023.04 — Implement/prove: contain**
  - Action: For incident mode, implement or reuse and verify this exact obligation: contain. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for contain; retain observable state/resource expectations.
- [ ] **F-OP-023.05 — Implement/prove: communicate**
  - Action: For incident mode, implement or reuse and verify this exact obligation: communicate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for communicate; retain observable state/resource expectations.
- [ ] **F-OP-023.06 — Implement/prove: recover and document without destroying state**
  - Action: For incident mode, implement or reuse and verify this exact obligation: recover and document without destroying state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recover and document without destroying state; retain observable state/resource expectations.
- [ ] **F-OP-023.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to incident mode: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-023.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-023.08 — Integrate into the real consumer and runtime route**
  - Action: Wire incident mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-023.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-023.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for incident mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-023.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-024"></a>
## F-OP-024 — support documentation

**Original requirement:** install/boot/use/troubleshoot/recover/hardware matrix tied to versions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-024.01 — Reconcile existing support documentation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for support documentation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: install/boot/use/troubleshoot/recover/hardware matrix tied to versions
- [ ] **F-OP-024.02 — Freeze the exact contract for support documentation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: install/boot/use/troubleshoot/recover/hardware matrix tied to versions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-024.03 — Implement/prove: install/boot/use/troubleshoot/recover/hardware matrix tied to versions**
  - Action: For support documentation, implement or reuse and verify this exact obligation: install/boot/use/troubleshoot/recover/hardware matrix tied to versions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for install/boot/use/troubleshoot/recover/hardware matrix tied to versions; retain observable state/resource expectations.
- [ ] **F-OP-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to support documentation: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire support documentation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for support documentation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-025"></a>
## F-OP-025 — issue/report workflow

**Original requirement:** user-reviewed diagnostics, reproduction, privacy, artifact identity and status

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-025.01 — Reconcile existing issue/report workflow**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for issue/report workflow. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user-reviewed diagnostics, reproduction, privacy, artifact identity and status
- [ ] **F-OP-025.02 — Freeze the exact contract for issue/report workflow**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user-reviewed diagnostics, reproduction, privacy, artifact identity and status. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-025.03 — Implement/prove: user-reviewed diagnostics**
  - Action: For issue/report workflow, implement or reuse and verify this exact obligation: user-reviewed diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user-reviewed diagnostics; retain observable state/resource expectations.
- [ ] **F-OP-025.04 — Implement/prove: reproduction**
  - Action: For issue/report workflow, implement or reuse and verify this exact obligation: reproduction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reproduction; retain observable state/resource expectations.
- [ ] **F-OP-025.05 — Implement/prove: privacy**
  - Action: For issue/report workflow, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-OP-025.06 — Implement/prove: artifact identity and status**
  - Action: For issue/report workflow, implement or reuse and verify this exact obligation: artifact identity and status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-025.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for artifact identity and status; retain observable state/resource expectations.
- [ ] **F-OP-025.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to issue/report workflow: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-025.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-025.08 — Integrate into the real consumer and runtime route**
  - Action: Wire issue/report workflow into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-025.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-025.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for issue/report workflow as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-025.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-026"></a>
## F-OP-026 — compatibility/hardware matrix

**Original requirement:** exact machine/device/firmware/config/evidence and known limitations

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-026.01 — Reconcile existing compatibility/hardware matrix**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compatibility/hardware matrix. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact machine/device/firmware/config/evidence and known limitations
- [ ] **F-OP-026.02 — Freeze the exact contract for compatibility/hardware matrix**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact machine/device/firmware/config/evidence and known limitations. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-026.03 — Implement/prove: exact machine/device/firmware/config/evidence and known limitations**
  - Action: For compatibility/hardware matrix, implement or reuse and verify this exact obligation: exact machine/device/firmware/config/evidence and known limitations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact machine/device/firmware/config/evidence and known limitations; retain observable state/resource expectations.
- [ ] **F-OP-026.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compatibility/hardware matrix: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-026.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-026.05 — Integrate into the real consumer and runtime route**
  - Action: Wire compatibility/hardware matrix into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-026.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-026.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compatibility/hardware matrix as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-026.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-027"></a>
## F-OP-027 — rescue media distribution

**Original requirement:** separately signed, read-only default, target confirmation and repair log

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-027.01 — Reconcile existing rescue media distribution**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for rescue media distribution. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: separately signed, read-only default, target confirmation and repair log
- [ ] **F-OP-027.02 — Freeze the exact contract for rescue media distribution**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: separately signed, read-only default, target confirmation and repair log. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-027.03 — Implement/prove: separately signed**
  - Action: For rescue media distribution, implement or reuse and verify this exact obligation: separately signed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for separately signed; retain observable state/resource expectations.
- [ ] **F-OP-027.04 — Implement/prove: read-only default**
  - Action: For rescue media distribution, implement or reuse and verify this exact obligation: read-only default. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read-only default; retain observable state/resource expectations.
- [ ] **F-OP-027.05 — Implement/prove: target confirmation and repair log**
  - Action: For rescue media distribution, implement or reuse and verify this exact obligation: target confirmation and repair log. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target confirmation and repair log; retain observable state/resource expectations.
- [ ] **F-OP-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to rescue media distribution: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire rescue media distribution into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for rescue media distribution as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-028"></a>
## F-OP-028 — supply-chain incident response

**Original requirement:** revoke keys/packages/images, identify affected installs, recover and notify

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-028.01 — Reconcile existing supply-chain incident response**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for supply-chain incident response. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: revoke keys/packages/images, identify affected installs, recover and notify
- [ ] **F-OP-028.02 — Freeze the exact contract for supply-chain incident response**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: revoke keys/packages/images, identify affected installs, recover and notify. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-028.03 — Implement/prove: revoke keys/packages/images**
  - Action: For supply-chain incident response, implement or reuse and verify this exact obligation: revoke keys/packages/images. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revoke keys/packages/images; retain observable state/resource expectations.
- [ ] **F-OP-028.04 — Implement/prove: identify affected installs**
  - Action: For supply-chain incident response, implement or reuse and verify this exact obligation: identify affected installs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identify affected installs; retain observable state/resource expectations.
- [ ] **F-OP-028.05 — Implement/prove: recover and notify**
  - Action: For supply-chain incident response, implement or reuse and verify this exact obligation: recover and notify. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recover and notify; retain observable state/resource expectations.
- [ ] **F-OP-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to supply-chain incident response: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire supply-chain incident response into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for supply-chain incident response as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-029"></a>
## F-OP-029 — end-of-life policy

**Original requirement:** support window, migration/export, key/package/archive retention and notice

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-029.01 — Reconcile existing end-of-life policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for end-of-life policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: support window, migration/export, key/package/archive retention and notice
- [ ] **F-OP-029.02 — Freeze the exact contract for end-of-life policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: support window, migration/export, key/package/archive retention and notice. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-029.03 — Implement/prove: support window**
  - Action: For end-of-life policy, implement or reuse and verify this exact obligation: support window. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for support window; retain observable state/resource expectations.
- [ ] **F-OP-029.04 — Implement/prove: migration/export**
  - Action: For end-of-life policy, implement or reuse and verify this exact obligation: migration/export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migration/export; retain observable state/resource expectations.
- [ ] **F-OP-029.05 — Implement/prove: key/package/archive retention and notice**
  - Action: For end-of-life policy, implement or reuse and verify this exact obligation: key/package/archive retention and notice. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for key/package/archive retention and notice; retain observable state/resource expectations.
- [ ] **F-OP-029.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to end-of-life policy: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-029.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-029.07 — Integrate into the real consumer and runtime route**
  - Action: Wire end-of-life policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-029.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-029.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for end-of-life policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-029.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-op-030"></a>
## F-OP-030 — public evidence portal

**Original requirement:** browsable claims, receipts, tests, failures, source/artifacts and corrections

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-OP-030.01 — Reconcile existing public evidence portal**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for public evidence portal. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: browsable claims, receipts, tests, failures, source/artifacts and corrections
- [ ] **F-OP-030.02 — Freeze the exact contract for public evidence portal**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: browsable claims, receipts, tests, failures, source/artifacts and corrections. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-OP-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-OP-030.03 — Implement/prove: browsable claims**
  - Action: For public evidence portal, implement or reuse and verify this exact obligation: browsable claims. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for browsable claims; retain observable state/resource expectations.
- [ ] **F-OP-030.04 — Implement/prove: receipts**
  - Action: For public evidence portal, implement or reuse and verify this exact obligation: receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for receipts; retain observable state/resource expectations.
- [ ] **F-OP-030.05 — Implement/prove: tests**
  - Action: For public evidence portal, implement or reuse and verify this exact obligation: tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tests; retain observable state/resource expectations.
- [ ] **F-OP-030.06 — Implement/prove: failures**
  - Action: For public evidence portal, implement or reuse and verify this exact obligation: failures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failures; retain observable state/resource expectations.
- [ ] **F-OP-030.07 — Implement/prove: source/artifacts and corrections**
  - Action: For public evidence portal, implement or reuse and verify this exact obligation: source/artifacts and corrections. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-OP-030.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source/artifacts and corrections; retain observable state/resource expectations.
- [ ] **F-OP-030.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to public evidence portal: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-OP-030.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-OP-030.09 — Integrate into the real consumer and runtime route**
  - Action: Wire public evidence portal into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-OP-030.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-OP-030.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for public evidence portal as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-OP-030.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p11-1"></a>
## C-P11.1 — transactional lease allocator

**Original requirement:** transactional lease allocator

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 585.

### Preserved original contract

- **Dependencies/current/provenance:** host service DB/lock and immutable image registry; mission-control concept; reject cache-lock-only multirow state.
- **I/O and state:** requester/rate policy/profile in; opaque lease secret, slot and deadlines out; lease common state.
- **Invariants/failure:** one DB transaction/serialization point; secret stored hashed; slot never double-owned; launch failure returns to clean/quarantined state.
- **Deterministic proof:** concurrent allocate/release/expire, process crash each transition, clock edges, exhausted pool, stale token.
- **Target proof:** real host launches multiple QEMU sessions under stress; no guest trust required.
- **Receipt/removal:** lease-state audit without raw secret; manual admin quarantine rollback; old allocator disabled only after concurrency/failure parity.

### Execution steps

- [ ] **C-P11.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P11.1.02 — Resolve this contract's exact dependencies**
  - Action: host service DB/lock and immutable image registry; mission-control concept; reject cache-lock-only multirow state. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P11.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P11.1.03 — I/O and state — transactional lease allocator**
  - Action: requester/rate policy/profile in; opaque lease secret, slot and deadlines out; lease common state.
  - Requires: C-P11.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P11.1.
- [ ] **C-P11.1.04 — Invariants/failure — transactional lease allocator**
  - Action: one DB transaction/serialization point; secret stored hashed; slot never double-owned; launch failure returns to clean/quarantined state.
  - Requires: C-P11.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P11.1.
- [ ] **C-P11.1.05 — Deterministic proof — transactional lease allocator**
  - Action: concurrent allocate/release/expire, process crash each transition, clock edges, exhausted pool, stale token.
  - Requires: C-P11.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P11.1.
- [ ] **C-P11.1.06 — Target proof — transactional lease allocator**
  - Action: real host launches multiple QEMU sessions under stress; no guest trust required.
  - Requires: C-P11.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P11.1.
- [ ] **C-P11.1.07 — Receipt/removal — transactional lease allocator**
  - Action: lease-state audit without raw secret; manual admin quarantine rollback; old allocator disabled only after concurrency/failure parity.
  - Requires: C-P11.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P11.1.
- [ ] **C-P11.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P11.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P11.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p11-2"></a>
## C-P11.2 — allowlisted QEMU launch and immutable overlays

**Original requirement:** allowlisted QEMU launch and immutable overlays

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 594.

### Preserved original contract

- **Dependencies/current/provenance:** P11.1, signed base image; mission-control COW; reject extra-argument denylist and hardcoded wrong formats/controllers.
- **I/O and state:** approved profile/base digest/lease ID in; PID identity, overlay, ports and log handles out; `Prepared -> Spawned -> Healthy|Failed`.
- **Invariants/failure:** structured arguments only; base read-only; overlay per lease; accelerator/controller/format compatible; partial artifacts rolled back.
- **Deterministic proof:** every profile, invalid combination, spawn failure, overlay failure, base mismatch, argument injection.
- **Target proof:** host boots exact zlOS digest and guest reports matching identity.
- **Receipt/removal:** argv profile/base/overlay/PID-start/artifact/guest identity; failed launch quarantines; no arbitrary-argument escape hatch.

### Execution steps

- [ ] **C-P11.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P11.2.02 — Resolve this contract's exact dependencies**
  - Action: P11.1, signed base image; mission-control COW; reject extra-argument denylist and hardcoded wrong formats/controllers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P11.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P11.2.03 — I/O and state — allowlisted QEMU launch and immutable overlays**
  - Action: approved profile/base digest/lease ID in; PID identity, overlay, ports and log handles out; `Prepared -> Spawned -> Healthy|Failed`.
  - Requires: C-P11.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P11.2.
- [ ] **C-P11.2.04 — Invariants/failure — allowlisted QEMU launch and immutable overlays**
  - Action: structured arguments only; base read-only; overlay per lease; accelerator/controller/format compatible; partial artifacts rolled back.
  - Requires: C-P11.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P11.2.
- [ ] **C-P11.2.05 — Deterministic proof — allowlisted QEMU launch and immutable overlays**
  - Action: every profile, invalid combination, spawn failure, overlay failure, base mismatch, argument injection.
  - Requires: C-P11.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P11.2.
- [ ] **C-P11.2.06 — Target proof — allowlisted QEMU launch and immutable overlays**
  - Action: host boots exact zlOS digest and guest reports matching identity.
  - Requires: C-P11.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P11.2.
- [ ] **C-P11.2.07 — Receipt/removal — allowlisted QEMU launch and immutable overlays**
  - Action: argv profile/base/overlay/PID-start/artifact/guest identity; failed launch quarantines; no arbitrary-argument escape hatch.
  - Requires: C-P11.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P11.2.
- [ ] **C-P11.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P11.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P11.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p11-3"></a>
## C-P11.3 — per-lease proxy authentication

**Original requirement:** per-lease proxy authentication

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 603.

### Preserved original contract

- **Dependencies/current/provenance:** P11.1/P11.2 and WebSocket/VNC proxy; mission-control negative evidence; reject slot/UUID-only access and passwordless VNC trust.
- **I/O and state:** lease secret/session binding and proxy request in; authorized bounded connection out.
- **Invariants/failure:** secret checked for every new/renewed channel; expiry/release revokes immediately; slot reuse cannot inherit access; origin/rate policy explicit.
- **Deterministic proof:** wrong/stale/replayed/cross-slot token, reconnect, expiry race, slot reuse, brute-rate limit.
- **Target proof:** host browser sessions cannot cross-connect; released lease loses channel.
- **Receipt/removal:** redacted auth/lease/channel events; admin console separate; old unauthenticated route removed immediately after parity.

### Execution steps

- [ ] **C-P11.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P11.3.02 — Resolve this contract's exact dependencies**
  - Action: P11.1/P11.2 and WebSocket/VNC proxy; mission-control negative evidence; reject slot/UUID-only access and passwordless VNC trust. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P11.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P11.3.03 — I/O and state — per-lease proxy authentication**
  - Action: lease secret/session binding and proxy request in; authorized bounded connection out.
  - Requires: C-P11.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P11.3.
- [ ] **C-P11.3.04 — Invariants/failure — per-lease proxy authentication**
  - Action: secret checked for every new/renewed channel; expiry/release revokes immediately; slot reuse cannot inherit access; origin/rate policy explicit.
  - Requires: C-P11.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P11.3.
- [ ] **C-P11.3.05 — Deterministic proof — per-lease proxy authentication**
  - Action: wrong/stale/replayed/cross-slot token, reconnect, expiry race, slot reuse, brute-rate limit.
  - Requires: C-P11.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P11.3.
- [ ] **C-P11.3.06 — Target proof — per-lease proxy authentication**
  - Action: host browser sessions cannot cross-connect; released lease loses channel.
  - Requires: C-P11.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P11.3.
- [ ] **C-P11.3.07 — Receipt/removal — per-lease proxy authentication**
  - Action: redacted auth/lease/channel events; admin console separate; old unauthenticated route removed immediately after parity.
  - Requires: C-P11.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P11.3.
- [ ] **C-P11.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P11.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P11.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p11-4"></a>
## C-P11.4 — resource and egress isolation

**Original requirement:** resource and egress isolation

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 612.

### Preserved original contract

- **Dependencies/current/provenance:** P11.2 and host cgroup/seccomp/firewall controls; snarkOS role budgets; reject unrestricted guest user-NAT.
- **I/O and state:** role/profile limits in; isolated QEMU process/network namespace and counters out.
- **Invariants/failure:** CPU/RAM/I/O/PID/disk ceilings; device allowlist; egress default deny/allow policy; limit breach throttles/terminates visibly.
- **Deterministic proof:** CPU/memory/fork/disk/network abuse, forbidden destination/device/syscall, multiple-session fairness.
- **Target proof:** host stress shows bounded impact and correct enforcement.
- **Receipt/removal:** limits/usage/violations/outcome; deny-all diagnostic profile rollback; no unlimited public profile.

### Execution steps

- [ ] **C-P11.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P11.4.02 — Resolve this contract's exact dependencies**
  - Action: P11.2 and host cgroup/seccomp/firewall controls; snarkOS role budgets; reject unrestricted guest user-NAT. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P11.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P11.4.03 — I/O and state — resource and egress isolation**
  - Action: role/profile limits in; isolated QEMU process/network namespace and counters out.
  - Requires: C-P11.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P11.4.
- [ ] **C-P11.4.04 — Invariants/failure — resource and egress isolation**
  - Action: CPU/RAM/I/O/PID/disk ceilings; device allowlist; egress default deny/allow policy; limit breach throttles/terminates visibly.
  - Requires: C-P11.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P11.4.
- [ ] **C-P11.4.05 — Deterministic proof — resource and egress isolation**
  - Action: CPU/memory/fork/disk/network abuse, forbidden destination/device/syscall, multiple-session fairness.
  - Requires: C-P11.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P11.4.
- [ ] **C-P11.4.06 — Target proof — resource and egress isolation**
  - Action: host stress shows bounded impact and correct enforcement.
  - Requires: C-P11.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P11.4.
- [ ] **C-P11.4.07 — Receipt/removal — resource and egress isolation**
  - Action: limits/usage/violations/outcome; deny-all diagnostic profile rollback; no unlimited public profile.
  - Requires: C-P11.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P11.4.
- [ ] **C-P11.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P11.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P11.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p11-5"></a>
## C-P11.5 — graceful stop, reaper and zero-orphan audit

**Original requirement:** graceful stop, reaper and zero-orphan audit

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 621.

### Preserved original contract

- **Dependencies/current/provenance:** P11.1-P11.4; mission-control lifecycle gaps/snarkOS reverse shutdown; reject SIGKILL-before-grace and PID-only identity.
- **I/O and state:** expiry/release/admin stop in; confirmed dead process, closed proxy, removed overlay and freed slot out; `Active -> Expiring -> GracefulStop -> ForceStop -> Cleaning -> Free|Quarantined`.
- **Invariants/failure:** verify PID plus start identity; free only after death/cleanup; failures quarantine rather than reuse; reaper idempotent.
- **Deterministic proof:** ignored TERM, PID reuse, unlink failure, proxy hang, host crash/restart, repeated reaper, overlay open handle.
- **Target proof:** host failure campaign ends with zero orphan processes/overlays/ports/double leases.
- **Receipt/removal:** lifecycle/identity/cleanup inventory; quarantine/manual cleanup rollback; old kill/free path removed before public exposure.

### Execution steps

- [ ] **C-P11.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P11.5.02 — Resolve this contract's exact dependencies**
  - Action: P11.1-P11.4; mission-control lifecycle gaps/snarkOS reverse shutdown; reject SIGKILL-before-grace and PID-only identity. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P11.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P11.5.03 — I/O and state — graceful stop, reaper and zero-orphan audit**
  - Action: expiry/release/admin stop in; confirmed dead process, closed proxy, removed overlay and freed slot out; `Active -> Expiring -> GracefulStop -> ForceStop -> Cleaning -> Free|Quarantined`.
  - Requires: C-P11.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P11.5.
- [ ] **C-P11.5.04 — Invariants/failure — graceful stop, reaper and zero-orphan audit**
  - Action: verify PID plus start identity; free only after death/cleanup; failures quarantine rather than reuse; reaper idempotent.
  - Requires: C-P11.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P11.5.
- [ ] **C-P11.5.05 — Deterministic proof — graceful stop, reaper and zero-orphan audit**
  - Action: ignored TERM, PID reuse, unlink failure, proxy hang, host crash/restart, repeated reaper, overlay open handle.
  - Requires: C-P11.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P11.5.
- [ ] **C-P11.5.06 — Target proof — graceful stop, reaper and zero-orphan audit**
  - Action: host failure campaign ends with zero orphan processes/overlays/ports/double leases.
  - Requires: C-P11.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P11.5.
- [ ] **C-P11.5.07 — Receipt/removal — graceful stop, reaper and zero-orphan audit**
  - Action: lifecycle/identity/cleanup inventory; quarantine/manual cleanup rollback; old kill/free path removed before public exposure.
  - Requires: C-P11.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P11.5.
- [ ] **C-P11.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P11.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P11.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-37"></a>
## C-DA-37 — rescue and repair environment

**Original requirement:** rescue and repair environment

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 868.

### Preserved original contract

**Depends on:** DA-03, DA-11, DA-21 and package authenticity.

**Deliver:** separately signed immutable rescue image, read-only default mounts,
exact target identity, dry run/confirmation, typed repair actions, backup/restore
pair and durable redacted audit.

**Proof:** wrong target, mounted target, power loss, full backup medium, corrupt
metadata, failed restore and recovery-key/physical-presence policy. No credential
bypass tool or passwordless general operation counts as a product feature.

### Execution steps

- [ ] **C-DA-37.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-37.02 — Resolve this contract's exact dependencies**
  - Action: DA-03, DA-11, DA-21 and package authenticity. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-37.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-37.03 — Deliver — rescue and repair environment**
  - Action: separately signed immutable rescue image, read-only default mounts, exact target identity, dry run/confirmation, typed repair actions, backup/restore pair and durable redacted audit.
  - Requires: C-DA-37.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-37.
- [ ] **C-DA-37.04 — Proof — rescue and repair environment**
  - Action: wrong target, mounted target, power loss, full backup medium, corrupt metadata, failed restore and recovery-key/physical-presence policy. No credential bypass tool or passwordless general operation counts as a product feature.
  - Requires: C-DA-37.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-37.
- [ ] **C-DA-37.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-37. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-37.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-38"></a>
## C-DA-38 — transactional public QEMU demo

**Original requirement:** transactional public QEMU demo

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 880.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-38.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-38.02 — Resolve this contract's exact dependencies**
  - Action: DA-03, DA-19, DA-24 and network isolation. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-38.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-38.03 — Deliver — transactional public QEMU demo**
  - Action: prewarmed slots, random lease, COW overlay, hard/idle deadlines, authenticated lifecycle and display/data plane, strict QEMU argument allowlist, per-VM identity, CPU/RAM/PID/egress limits, logs and reaper.
  - Requires: C-DA-38.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-38.
- [ ] **C-DA-38.04 — Invariants — transactional public QEMU demo**
  - Action: allocate/launch is one recoverable transaction; stored tokens are hashed; PID/VM identity is proven; death is verified before port/slot reuse; cleanup failure remains visible and retryable.
  - Requires: C-DA-38.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-38.
- [ ] **C-DA-38.05 — Proof — transactional public QEMU demo**
  - Action: nth-stage launch failure, cache/database split, guessed/enumerated VNC path, missing/expired token on every route/stream, PID reuse, unkillable VM, orphan overlay, egress attempt, resource exhaustion, concurrent allocator and reaper.
  - Requires: C-DA-38.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-38.
- [ ] **C-DA-38.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-38. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-38.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-47"></a>
## C-VX-47 — Rescue and public-demo experiences

**Original requirement:** Rescue and public-demo experiences

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 720.

### Preserved original contract

**Depends on:** VX-13, VX-17, operations/service contracts.

**Deliver:** signed read-only rescue UI plus transactional leased QEMU/noVNC
demo with authenticated control and data planes, containment and cleanup.

**Proof:** wrong-disk confirmation, backup/restore, interrupted repair, lease
theft, enumerable socket, launch failure rollback, PID reuse, process death,
resource/egress limits and cross-session privacy.

### Execution steps

- [ ] **C-VX-47.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-47.02 — Resolve this contract's exact dependencies**
  - Action: VX-13, VX-17, operations/service contracts. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-47.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-47.03 — Deliver — Rescue and public-demo experiences**
  - Action: signed read-only rescue UI plus transactional leased QEMU/noVNC demo with authenticated control and data planes, containment and cleanup.
  - Requires: C-VX-47.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-47.
- [ ] **C-VX-47.04 — Proof — Rescue and public-demo experiences**
  - Action: wrong-disk confirmation, backup/restore, interrupted repair, lease theft, enumerable socket, launch failure rollback, PID reuse, process death, resource/egress limits and cross-session privacy.
  - Requires: C-VX-47.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-47.
- [ ] **C-VX-47.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-47. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-47.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-52"></a>
## C-VX-52 — QEMU product matrix

**Original requirement:** QEMU product matrix

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 776.

### Preserved original contract

**Depends on:** VX-50, boot assertion matrix.

**Deliver:** exact artifact across BIOS/UEFI/media/resolution/input/network/
storage/audio profiles with launch/workflow/close and fault scenarios.

**Proof:** artifact digest and nonce, negative exits, panic absence, durable
side-effect checks and post-workload filesystem integrity.

### Execution steps

- [ ] **C-VX-52.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-52.02 — Resolve this contract's exact dependencies**
  - Action: VX-50, boot assertion matrix. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-52.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-52.03 — Deliver — QEMU product matrix**
  - Action: exact artifact across BIOS/UEFI/media/resolution/input/network/ storage/audio profiles with launch/workflow/close and fault scenarios.
  - Requires: C-VX-52.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-52.
- [ ] **C-VX-52.04 — Proof — QEMU product matrix**
  - Action: artifact digest and nonce, negative exits, panic absence, durable side-effect checks and post-workload filesystem integrity.
  - Requires: C-VX-52.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-52.
- [ ] **C-VX-52.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-52. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-52.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-plat-031"></a>
## T-PLAT-031 — watchdog

**Original requirement:** arm/pet/disarm, crash receipt and reboot cause

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 60.

### Execution steps

- [ ] **T-PLAT-031.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve watchdog to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-031.02 — Specify the complete target boundary**
  - Action: watchdog must supply: arm/pet/disarm, crash receipt and reboot cause. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-031.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-031.03 — Implement the exact target behavior**
  - Action: Implement or reuse watchdog through the shared platform contract, delivering every part of: arm/pet/disarm, crash receipt and reboot cause. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-031.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-031.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-031.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for watchdog.
- [ ] **T-PLAT-031.05 — Qualify and retain this target's own result**
  - Action: Bind watchdog to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-031.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-016"></a>
## T-NIC-016 — bridge/VLAN interface

**Original requirement:** loop prevention, policy and lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 219.

### Execution steps

- [ ] **T-NIC-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve bridge/VLAN interface to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-016.02 — Specify the complete target boundary**
  - Action: bridge/VLAN interface must supply: loop prevention, policy and lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse bridge/VLAN interface through the shared platform contract, delivering every part of: loop prevention, policy and lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-016.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for bridge/VLAN interface.
- [ ] **T-NIC-016.05 — Qualify and retain this target's own result**
  - Action: Bind bridge/VLAN interface to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-012"></a>
## T-SVC-012 — Crash/Recovery Manager

**Original requirement:** crash collection, symbols, quarantine and recovery action

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 29.

### Execution steps

- [ ] **T-SVC-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Crash/Recovery Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-012.02 — Specify the complete target boundary**
  - Action: Crash/Recovery Manager must supply: crash collection, symbols, quarantine and recovery action. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse Crash/Recovery Manager through the shared platform contract, delivering every part of: crash collection, symbols, quarantine and recovery action. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-012.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Crash/Recovery Manager.
- [ ] **T-SVC-012.05 — Qualify and retain this target's own result**
  - Action: Bind Crash/Recovery Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-061"></a>
## T-SVC-061 — Remote Desktop Service

**Original requirement:** authenticated display/input/clipboard and sessions

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 81.

### Execution steps

- [ ] **T-SVC-061.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Remote Desktop Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-061.02 — Specify the complete target boundary**
  - Action: Remote Desktop Service must supply: authenticated display/input/clipboard and sessions. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-061.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-061.03 — Implement the exact target behavior**
  - Action: Implement or reuse Remote Desktop Service through the shared platform contract, delivering every part of: authenticated display/input/clipboard and sessions. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-061.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-061.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-061.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Remote Desktop Service.
- [ ] **T-SVC-061.05 — Qualify and retain this target's own result**
  - Action: Bind Remote Desktop Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-061.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-135"></a>
## T-SVC-135 — Release/Update Orchestrator

**Original requirement:** build/sign/promote/rollback/revoke generations

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 162.

### Execution steps

- [ ] **T-SVC-135.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Release/Update Orchestrator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-135.02 — Specify the complete target boundary**
  - Action: Release/Update Orchestrator must supply: build/sign/promote/rollback/revoke generations. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-135.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-135.03 — Implement the exact target behavior**
  - Action: Implement or reuse Release/Update Orchestrator through the shared platform contract, delivering every part of: build/sign/promote/rollback/revoke generations. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-135.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-135.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-135.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Release/Update Orchestrator.
- [ ] **T-SVC-135.05 — Qualify and retain this target's own result**
  - Action: Bind Release/Update Orchestrator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-135.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-136"></a>
## T-SVC-136 — Public Demo Lease Manager

**Original requirement:** atomic allocation, quotas, launch and reclaim

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 163.

### Execution steps

- [ ] **T-SVC-136.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Public Demo Lease Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-136.02 — Specify the complete target boundary**
  - Action: Public Demo Lease Manager must supply: atomic allocation, quotas, launch and reclaim. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-136.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-136.03 — Implement the exact target behavior**
  - Action: Implement or reuse Public Demo Lease Manager through the shared platform contract, delivering every part of: atomic allocation, quotas, launch and reclaim. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-136.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-136.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-136.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Public Demo Lease Manager.
- [ ] **T-SVC-136.05 — Qualify and retain this target's own result**
  - Action: Bind Public Demo Lease Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-136.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-137"></a>
## T-SVC-137 — Authenticated VNC/WebSocket Proxy

**Original requirement:** per-lease data-plane identity and expiration

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 164.

### Execution steps

- [ ] **T-SVC-137.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Authenticated VNC/WebSocket Proxy to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-137.02 — Specify the complete target boundary**
  - Action: Authenticated VNC/WebSocket Proxy must supply: per-lease data-plane identity and expiration. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-137.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-137.03 — Implement the exact target behavior**
  - Action: Implement or reuse Authenticated VNC/WebSocket Proxy through the shared platform contract, delivering every part of: per-lease data-plane identity and expiration. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-137.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-137.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-137.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Authenticated VNC/WebSocket Proxy.
- [ ] **T-SVC-137.05 — Qualify and retain this target's own result**
  - Action: Bind Authenticated VNC/WebSocket Proxy to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-137.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-138"></a>
## T-SVC-138 — Telemetry/Health Service

**Original requirement:** metrics, budgets, retention, privacy and alerts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 165.

### Execution steps

- [ ] **T-SVC-138.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Telemetry/Health Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-138.02 — Specify the complete target boundary**
  - Action: Telemetry/Health Service must supply: metrics, budgets, retention, privacy and alerts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-138.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-138.03 — Implement the exact target behavior**
  - Action: Implement or reuse Telemetry/Health Service through the shared platform contract, delivering every part of: metrics, budgets, retention, privacy and alerts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-138.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-138.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-138.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Telemetry/Health Service.
- [ ] **T-SVC-138.05 — Qualify and retain this target's own result**
  - Action: Bind Telemetry/Health Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-138.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-139"></a>
## T-SVC-139 — Support Bundle Service

**Original requirement:** consent, redaction, reproduction and export

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 166.

### Execution steps

- [ ] **T-SVC-139.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Support Bundle Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-139.02 — Specify the complete target boundary**
  - Action: Support Bundle Service must supply: consent, redaction, reproduction and export. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-139.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-139.03 — Implement the exact target behavior**
  - Action: Implement or reuse Support Bundle Service through the shared platform contract, delivering every part of: consent, redaction, reproduction and export. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-139.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-139.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-139.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Support Bundle Service.
- [ ] **T-SVC-139.05 — Qualify and retain this target's own result**
  - Action: Bind Support Bundle Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-139.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-140"></a>
## T-SVC-140 — Rescue/Repair Orchestrator

**Original requirement:** read-only default, target confirmation and repair log

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 167.

### Execution steps

- [ ] **T-SVC-140.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Rescue/Repair Orchestrator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-140.02 — Specify the complete target boundary**
  - Action: Rescue/Repair Orchestrator must supply: read-only default, target confirmation and repair log. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-140.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-140.03 — Implement the exact target behavior**
  - Action: Implement or reuse Rescue/Repair Orchestrator through the shared platform contract, delivering every part of: read-only default, target confirmation and repair log. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-140.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-140.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-140.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Rescue/Repair Orchestrator.
- [ ] **T-SVC-140.05 — Qualify and retain this target's own result**
  - Action: Bind Rescue/Repair Orchestrator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-140.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-141"></a>
## T-SVC-141 — Feature/Provenance Viewer Provider

**Original requirement:** live registry, receipts, defects and evidence ceiling

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 168.

### Execution steps

- [ ] **T-SVC-141.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Feature/Provenance Viewer Provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-141.02 — Specify the complete target boundary**
  - Action: Feature/Provenance Viewer Provider must supply: live registry, receipts, defects and evidence ceiling. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-141.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-141.03 — Implement the exact target behavior**
  - Action: Implement or reuse Feature/Provenance Viewer Provider through the shared platform contract, delivering every part of: live registry, receipts, defects and evidence ceiling. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-141.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-141.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-SVC-141.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Feature/Provenance Viewer Provider.
- [ ] **T-SVC-141.05 — Qualify and retain this target's own result**
  - Action: Bind Feature/Provenance Viewer Provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-141.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-016"></a>
## T-APP-016 — Feature/Provenance Viewer

**Original requirement:** inspect implementation, version, source, license and evidence

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 143.

### Execution steps

- [ ] **T-APP-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Feature/Provenance Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-016.02 — Specify the complete target boundary**
  - Action: Feature/Provenance Viewer must supply: inspect implementation, version, source, license and evidence. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse Feature/Provenance Viewer through the shared platform contract, delivering every part of: inspect implementation, version, source, license and evidence. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-016.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Feature/Provenance Viewer.
- [ ] **T-APP-016.05 — Qualify and retain this target's own result**
  - Action: Bind Feature/Provenance Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-100"></a>
## T-APP-100 — Rescue Desktop

**Original requirement:** signed read-only-first diagnostics and repair entry

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 228.

### Execution steps

- [ ] **T-APP-100.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Rescue Desktop to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-100.02 — Specify the complete target boundary**
  - Action: Rescue Desktop must supply: signed read-only-first diagnostics and repair entry. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-100.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-100.03 — Implement the exact target behavior**
  - Action: Implement or reuse Rescue Desktop through the shared platform contract, delivering every part of: signed read-only-first diagnostics and repair entry. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-100.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-100.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-100.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Rescue Desktop.
- [ ] **T-APP-100.05 — Qualify and retain this target's own result**
  - Action: Bind Rescue Desktop to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-100.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-101"></a>
## T-APP-101 — Backup/Restore

**Original requirement:** select, verify, restore, dry run and rollback

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 229.

### Execution steps

- [ ] **T-APP-101.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Backup/Restore to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-101.02 — Specify the complete target boundary**
  - Action: Backup/Restore must supply: select, verify, restore, dry run and rollback. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-101.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-101.03 — Implement the exact target behavior**
  - Action: Implement or reuse Backup/Restore through the shared platform contract, delivering every part of: select, verify, restore, dry run and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-101.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-101.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-101.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Backup/Restore.
- [ ] **T-APP-101.05 — Qualify and retain this target's own result**
  - Action: Bind Backup/Restore to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-101.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-102"></a>
## T-APP-102 — Boot Generation Manager

**Original requirement:** current/previous/recovery identity and selection

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 230.

### Execution steps

- [ ] **T-APP-102.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Boot Generation Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-102.02 — Specify the complete target boundary**
  - Action: Boot Generation Manager must supply: current/previous/recovery identity and selection. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-102.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-102.03 — Implement the exact target behavior**
  - Action: Implement or reuse Boot Generation Manager through the shared platform contract, delivering every part of: current/previous/recovery identity and selection. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-102.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-102.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-102.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Boot Generation Manager.
- [ ] **T-APP-102.05 — Qualify and retain this target's own result**
  - Action: Bind Boot Generation Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-102.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-103"></a>
## T-APP-103 — Installation/Deployment UI

**Original requirement:** target identity, partition preview, copy, verify and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 231.

### Execution steps

- [ ] **T-APP-103.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Installation/Deployment UI to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-103.02 — Specify the complete target boundary**
  - Action: Installation/Deployment UI must supply: target identity, partition preview, copy, verify and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-103.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-103.03 — Implement the exact target behavior**
  - Action: Implement or reuse Installation/Deployment UI through the shared platform contract, delivering every part of: target identity, partition preview, copy, verify and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-103.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-103.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-103.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Installation/Deployment UI.
- [ ] **T-APP-103.05 — Qualify and retain this target's own result**
  - Action: Bind Installation/Deployment UI to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-103.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-104"></a>
## T-APP-104 — Public Demo Portal

**Original requirement:** authenticated lease, visible expiry, reconnect and cleanup

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 232.

### Execution steps

- [ ] **T-APP-104.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Public Demo Portal to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-104.02 — Specify the complete target boundary**
  - Action: Public Demo Portal must supply: authenticated lease, visible expiry, reconnect and cleanup. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-104.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-104.03 — Implement the exact target behavior**
  - Action: Implement or reuse Public Demo Portal through the shared platform contract, delivering every part of: authenticated lease, visible expiry, reconnect and cleanup. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-104.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-104.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-104.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Public Demo Portal.
- [ ] **T-APP-104.05 — Qualify and retain this target's own result**
  - Action: Bind Public Demo Portal to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-104.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-105"></a>
## T-APP-105 — Support Bundle

**Original requirement:** consent, redaction, preview, export and reproduction ID

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 233.

### Execution steps

- [ ] **T-APP-105.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Support Bundle to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-105.02 — Specify the complete target boundary**
  - Action: Support Bundle must supply: consent, redaction, preview, export and reproduction ID. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-105.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-105.03 — Implement the exact target behavior**
  - Action: Implement or reuse Support Bundle through the shared platform contract, delivering every part of: consent, redaction, preview, export and reproduction ID. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-105.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-105.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-APP-105.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Support Bundle.
- [ ] **T-APP-105.05 — Qualify and retain this target's own result**
  - Action: Bind Support Bundle to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-105.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-001"></a>
## T-OPS-001 — hermetic release build

**Original requirement:** empty tree, locked toolchain/dependencies and strict exit

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 143.

### Execution steps

- [ ] **T-OPS-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve hermetic release build to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-001.02 — Specify the complete target boundary**
  - Action: hermetic release build must supply: empty tree, locked toolchain/dependencies and strict exit. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse hermetic release build through the shared platform contract, delivering every part of: empty tree, locked toolchain/dependencies and strict exit. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-001.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for hermetic release build.
- [ ] **T-OPS-001.05 — Qualify and retain this target's own result**
  - Action: Bind hermetic release build to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-002"></a>
## T-OPS-002 — artifact identity/SBOM

**Original requirement:** source/config/tool/package/license/signature and boot origin

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 144.

### Execution steps

- [ ] **T-OPS-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve artifact identity/SBOM to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-002.02 — Specify the complete target boundary**
  - Action: artifact identity/SBOM must supply: source/config/tool/package/license/signature and boot origin. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse artifact identity/SBOM through the shared platform contract, delivering every part of: source/config/tool/package/license/signature and boot origin. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-002.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for artifact identity/SBOM.
- [ ] **T-OPS-002.05 — Qualify and retain this target's own result**
  - Action: Bind artifact identity/SBOM to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-003"></a>
## T-OPS-003 — signing and trust policy

**Original requirement:** offline roots, online roles, rotation, revocation and recovery

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 145.

### Execution steps

- [ ] **T-OPS-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve signing and trust policy to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-003.02 — Specify the complete target boundary**
  - Action: signing and trust policy must supply: offline roots, online roles, rotation, revocation and recovery. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse signing and trust policy through the shared platform contract, delivering every part of: offline roots, online roles, rotation, revocation and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-003.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for signing and trust policy.
- [ ] **T-OPS-003.05 — Qualify and retain this target's own result**
  - Action: Bind signing and trust policy to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-004"></a>
## T-OPS-004 — release channels

**Original requirement:** stable/beta/developer/recovery with compatibility and promotion

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 146.

### Execution steps

- [ ] **T-OPS-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve release channels to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-004.02 — Specify the complete target boundary**
  - Action: release channels must supply: stable/beta/developer/recovery with compatibility and promotion. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse release channels through the shared platform contract, delivering every part of: stable/beta/developer/recovery with compatibility and promotion. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-004.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for release channels.
- [ ] **T-OPS-004.05 — Qualify and retain this target's own result**
  - Action: Bind release channels to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-005"></a>
## T-OPS-005 — transactional updater

**Original requirement:** download/verify/stage/switch/reboot/health/rollback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 147.

### Execution steps

- [ ] **T-OPS-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve transactional updater to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-005.02 — Specify the complete target boundary**
  - Action: transactional updater must supply: download/verify/stage/switch/reboot/health/rollback. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse transactional updater through the shared platform contract, delivering every part of: download/verify/stage/switch/reboot/health/rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-005.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for transactional updater.
- [ ] **T-OPS-005.05 — Qualify and retain this target's own result**
  - Action: Bind transactional updater to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-006"></a>
## T-OPS-006 — boot generations

**Original requirement:** current/previous/recovery identity and atomic selection

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 148.

### Execution steps

- [ ] **T-OPS-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve boot generations to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-006.02 — Specify the complete target boundary**
  - Action: boot generations must supply: current/previous/recovery identity and atomic selection. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse boot generations through the shared platform contract, delivering every part of: current/previous/recovery identity and atomic selection. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-006.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for boot generations.
- [ ] **T-OPS-006.05 — Qualify and retain this target's own result**
  - Action: Bind boot generations to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-007"></a>
## T-OPS-007 — migration framework

**Original requirement:** versioned data/config/package migrations and rollback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 149.

### Execution steps

- [ ] **T-OPS-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve migration framework to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-007.02 — Specify the complete target boundary**
  - Action: migration framework must supply: versioned data/config/package migrations and rollback. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse migration framework through the shared platform contract, delivering every part of: versioned data/config/package migrations and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-007.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for migration framework.
- [ ] **T-OPS-007.05 — Qualify and retain this target's own result**
  - Action: Bind migration framework to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-008"></a>
## T-OPS-008 — release notes

**Original requirement:** generated changes, compatibility, defects and recovery steps

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 150.

### Execution steps

- [ ] **T-OPS-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve release notes to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-008.02 — Specify the complete target boundary**
  - Action: release notes must supply: generated changes, compatibility, defects and recovery steps. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse release notes through the shared platform contract, delivering every part of: generated changes, compatibility, defects and recovery steps. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-008.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for release notes.
- [ ] **T-OPS-008.05 — Qualify and retain this target's own result**
  - Action: Bind release notes to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-009"></a>
## T-OPS-009 — CI build matrix

**Original requirement:** x86 lanes plus honest architecture build/boot/test distinctions

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 151.

### Execution steps

- [ ] **T-OPS-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve CI build matrix to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-009.02 — Specify the complete target boundary**
  - Action: CI build matrix must supply: x86 lanes plus honest architecture build/boot/test distinctions. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse CI build matrix through the shared platform contract, delivering every part of: x86 lanes plus honest architecture build/boot/test distinctions. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-009.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for CI build matrix.
- [ ] **T-OPS-009.05 — Qualify and retain this target's own result**
  - Action: Bind CI build matrix to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-010"></a>
## T-OPS-010 — host deterministic matrix

**Original requirement:** unit/property/fuzz/sanitizer/failure/mutation tests

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 152.

### Execution steps

- [ ] **T-OPS-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve host deterministic matrix to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-010.02 — Specify the complete target boundary**
  - Action: host deterministic matrix must supply: unit/property/fuzz/sanitizer/failure/mutation tests. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse host deterministic matrix through the shared platform contract, delivering every part of: unit/property/fuzz/sanitizer/failure/mutation tests. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-010.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for host deterministic matrix.
- [ ] **T-OPS-010.05 — Qualify and retain this target's own result**
  - Action: Bind host deterministic matrix to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-011"></a>
## T-OPS-011 — QEMU system matrix

**Original requirement:** BIOS/UEFI/media/device/workflow/assertion kernels

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 153.

### Execution steps

- [ ] **T-OPS-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve QEMU system matrix to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-011.02 — Specify the complete target boundary**
  - Action: QEMU system matrix must supply: BIOS/UEFI/media/device/workflow/assertion kernels. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse QEMU system matrix through the shared platform contract, delivering every part of: BIOS/UEFI/media/device/workflow/assertion kernels. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-011.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for QEMU system matrix.
- [ ] **T-OPS-011.05 — Qualify and retain this target's own result**
  - Action: Bind QEMU system matrix to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-012"></a>
## T-OPS-012 — native hardware matrix

**Original requirement:** exact device/firmware/topology and workload receipts

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 154.

### Execution steps

- [ ] **T-OPS-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve native hardware matrix to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-012.02 — Specify the complete target boundary**
  - Action: native hardware matrix must supply: exact device/firmware/topology and workload receipts. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse native hardware matrix through the shared platform contract, delivering every part of: exact device/firmware/topology and workload receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-012.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for native hardware matrix.
- [ ] **T-OPS-012.05 — Qualify and retain this target's own result**
  - Action: Bind native hardware matrix to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-013"></a>
## T-OPS-013 — performance promotion

**Original requirement:** budgets, distributions, machine/backend and regression action

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 155.

### Execution steps

- [ ] **T-OPS-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve performance promotion to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-013.02 — Specify the complete target boundary**
  - Action: performance promotion must supply: budgets, distributions, machine/backend and regression action. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse performance promotion through the shared platform contract, delivering every part of: budgets, distributions, machine/backend and regression action. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-013.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for performance promotion.
- [ ] **T-OPS-013.05 — Qualify and retain this target's own result**
  - Action: Bind performance promotion to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-014"></a>
## T-OPS-014 — security promotion

**Original requirement:** threat assets, enforcement, hostile proof and residual risk

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 156.

### Execution steps

- [ ] **T-OPS-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve security promotion to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-014.02 — Specify the complete target boundary**
  - Action: security promotion must supply: threat assets, enforcement, hostile proof and residual risk. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse security promotion through the shared platform contract, delivering every part of: threat assets, enforcement, hostile proof and residual risk. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-014.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for security promotion.
- [ ] **T-OPS-014.05 — Qualify and retain this target's own result**
  - Action: Bind security promotion to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-015"></a>
## T-OPS-015 — accessibility promotion

**Original requirement:** semantic/keyboard/provider/scale/contrast/motion/locales

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 157.

### Execution steps

- [ ] **T-OPS-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve accessibility promotion to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-015.02 — Specify the complete target boundary**
  - Action: accessibility promotion must supply: semantic/keyboard/provider/scale/contrast/motion/locales. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse accessibility promotion through the shared platform contract, delivering every part of: semantic/keyboard/provider/scale/contrast/motion/locales. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-015.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for accessibility promotion.
- [ ] **T-OPS-015.05 — Qualify and retain this target's own result**
  - Action: Bind accessibility promotion to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-016"></a>
## T-OPS-016 — visual promotion

**Original requirement:** regional goldens, layout/text metrics, animation traces

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 158.

### Execution steps

- [ ] **T-OPS-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve visual promotion to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-016.02 — Specify the complete target boundary**
  - Action: visual promotion must supply: regional goldens, layout/text metrics, animation traces. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse visual promotion through the shared platform contract, delivering every part of: regional goldens, layout/text metrics, animation traces. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-016.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for visual promotion.
- [ ] **T-OPS-016.05 — Qualify and retain this target's own result**
  - Action: Bind visual promotion to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-017"></a>
## T-OPS-017 — crash/telemetry pipeline

**Original requirement:** consent, redaction, symbol identity, retention and alerting

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 159.

### Execution steps

- [ ] **T-OPS-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve crash/telemetry pipeline to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-017.02 — Specify the complete target boundary**
  - Action: crash/telemetry pipeline must supply: consent, redaction, symbol identity, retention and alerting. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse crash/telemetry pipeline through the shared platform contract, delivering every part of: consent, redaction, symbol identity, retention and alerting. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-017.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for crash/telemetry pipeline.
- [ ] **T-OPS-017.05 — Qualify and retain this target's own result**
  - Action: Bind crash/telemetry pipeline to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-018"></a>
## T-OPS-018 — support bundles

**Original requirement:** user preview, scrub, reproduce and correlation ID

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 160.

### Execution steps

- [ ] **T-OPS-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve support bundles to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-018.02 — Specify the complete target boundary**
  - Action: support bundles must supply: user preview, scrub, reproduce and correlation ID. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse support bundles through the shared platform contract, delivering every part of: user preview, scrub, reproduce and correlation ID. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-018.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for support bundles.
- [ ] **T-OPS-018.05 — Qualify and retain this target's own result**
  - Action: Bind support bundles to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-019"></a>
## T-OPS-019 — backup/disaster recovery drill

**Original requirement:** verified backup, destructive simulation, restore and audit

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 161.

### Execution steps

- [ ] **T-OPS-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve backup/disaster recovery drill to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-019.02 — Specify the complete target boundary**
  - Action: backup/disaster recovery drill must supply: verified backup, destructive simulation, restore and audit. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse backup/disaster recovery drill through the shared platform contract, delivering every part of: verified backup, destructive simulation, restore and audit. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-019.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for backup/disaster recovery drill.
- [ ] **T-OPS-019.05 — Qualify and retain this target's own result**
  - Action: Bind backup/disaster recovery drill to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-020"></a>
## T-OPS-020 — rescue image

**Original requirement:** separately signed, read-only default, explicit target and repair log

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 162.

### Execution steps

- [ ] **T-OPS-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve rescue image to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-020.02 — Specify the complete target boundary**
  - Action: rescue image must supply: separately signed, read-only default, explicit target and repair log. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse rescue image through the shared platform contract, delivering every part of: separately signed, read-only default, explicit target and repair log. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-020.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for rescue image.
- [ ] **T-OPS-020.05 — Qualify and retain this target's own result**
  - Action: Bind rescue image to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-021"></a>
## T-OPS-021 — public demo allocator

**Original requirement:** atomic slot/port/disk/resource lease and rollback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 163.

### Execution steps

- [ ] **T-OPS-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve public demo allocator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-021.02 — Specify the complete target boundary**
  - Action: public demo allocator must supply: atomic slot/port/disk/resource lease and rollback. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse public demo allocator through the shared platform contract, delivering every part of: atomic slot/port/disk/resource lease and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-021.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for public demo allocator.
- [ ] **T-OPS-021.05 — Qualify and retain this target's own result**
  - Action: Bind public demo allocator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-022"></a>
## T-OPS-022 — QEMU launcher

**Original requirement:** allowlisted arguments, immutable base, per-lease overlay and limits

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 164.

### Execution steps

- [ ] **T-OPS-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve QEMU launcher to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-022.02 — Specify the complete target boundary**
  - Action: QEMU launcher must supply: allowlisted arguments, immutable base, per-lease overlay and limits. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse QEMU launcher through the shared platform contract, delivering every part of: allowlisted arguments, immutable base, per-lease overlay and limits. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-022.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for QEMU launcher.
- [ ] **T-OPS-022.05 — Qualify and retain this target's own result**
  - Action: Bind QEMU launcher to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-023"></a>
## T-OPS-023 — VNC/WebSocket authentication

**Original requirement:** unenumerable per-lease data plane and expiration

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 165.

### Execution steps

- [ ] **T-OPS-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VNC/WebSocket authentication to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-023.02 — Specify the complete target boundary**
  - Action: VNC/WebSocket authentication must supply: unenumerable per-lease data plane and expiration. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse VNC/WebSocket authentication through the shared platform contract, delivering every part of: unenumerable per-lease data plane and expiration. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-023.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VNC/WebSocket authentication.
- [ ] **T-OPS-023.05 — Qualify and retain this target's own result**
  - Action: Bind VNC/WebSocket authentication to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-024"></a>
## T-OPS-024 — demo containment

**Original requirement:** CPU/RAM/PID/disk/egress/time/privacy policy

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 166.

### Execution steps

- [ ] **T-OPS-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve demo containment to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-024.02 — Specify the complete target boundary**
  - Action: demo containment must supply: CPU/RAM/PID/disk/egress/time/privacy policy. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse demo containment through the shared platform contract, delivering every part of: CPU/RAM/PID/disk/egress/time/privacy policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-024.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for demo containment.
- [ ] **T-OPS-024.05 — Qualify and retain this target's own result**
  - Action: Bind demo containment to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-025"></a>
## T-OPS-025 — demo reaper

**Original requirement:** prove process identity/death before resource reuse, zero orphans

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 167.

### Execution steps

- [ ] **T-OPS-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve demo reaper to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-025.02 — Specify the complete target boundary**
  - Action: demo reaper must supply: prove process identity/death before resource reuse, zero orphans. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse demo reaper through the shared platform contract, delivering every part of: prove process identity/death before resource reuse, zero orphans. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-025.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for demo reaper.
- [ ] **T-OPS-025.05 — Qualify and retain this target's own result**
  - Action: Bind demo reaper to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-026"></a>
## T-OPS-026 — provenance/feature viewer

**Original requirement:** live maturity, evidence, licenses, permissions and health

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 168.

### Execution steps

- [ ] **T-OPS-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve provenance/feature viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-026.02 — Specify the complete target boundary**
  - Action: provenance/feature viewer must supply: live maturity, evidence, licenses, permissions and health. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse provenance/feature viewer through the shared platform contract, delivering every part of: live maturity, evidence, licenses, permissions and health. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-026.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for provenance/feature viewer.
- [ ] **T-OPS-026.05 — Qualify and retain this target's own result**
  - Action: Bind provenance/feature viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-027"></a>
## T-OPS-027 — deprecation/support policy

**Original requirement:** lifecycle, compatibility window, migrations and archives

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 169.

### Execution steps

- [ ] **T-OPS-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve deprecation/support policy to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-027.02 — Specify the complete target boundary**
  - Action: deprecation/support policy must supply: lifecycle, compatibility window, migrations and archives. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse deprecation/support policy through the shared platform contract, delivering every part of: lifecycle, compatibility window, migrations and archives. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-027.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for deprecation/support policy.
- [ ] **T-OPS-027.05 — Qualify and retain this target's own result**
  - Action: Bind deprecation/support policy to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-ops-028"></a>
## T-OPS-028 — independent release review

**Original requirement:** reviewer can reject; no self-certified completion

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 170.

### Execution steps

- [ ] **T-OPS-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve independent release review to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, D-10, D-24, H-18.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-OPS-028.02 — Specify the complete target boundary**
  - Action: independent release review must supply: reviewer can reject; no self-certified completion. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-OPS-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-OPS-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse independent release review through the shared platform contract, delivering every part of: reviewer can reject; no self-certified completion. Do not fork a duplicate subsystem for this row.
  - Requires: T-OPS-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-OPS-028.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: wrong artifact/signature; interrupted migration; missing previous generation; lease/proxy failure; orphan PID reuse; quota or egress bypass; unredacted telemetry; broken rollback.
  - Requires: T-OPS-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for independent release review.
- [ ] **T-OPS-028.05 — Qualify and retain this target's own result**
  - Action: Bind independent release review to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-OPS-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
