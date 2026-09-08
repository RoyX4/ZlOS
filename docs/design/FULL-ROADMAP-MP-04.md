# MP-04: Give programs explicit authority and safe communication

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/core/; kernel/src/runtime/; kernel/metadata/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-04` exports: Generation-checked rights, bounded IPC and a minimal restartable supervisor with declared service identity.

The handoff enables only its named subset. `CLOSE-04` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-04.01 — Define process-local handle types, rights, owner identity, derivation, generation reuse and lifetime rules

Define process-local handle types, rights, owner identity, derivation, generation reuse and lifetime rules.

**Requires:** `D-01`, `D-02`, `D-07`, `D-08`, `D-09`, `D-10`, `H-00`, `H-03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.02 — Implement source and destination authorization independently

Implement source and destination authorization independently; reserve transfer destinations before committing authority changes.

**Requires:** `M-04.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.03 — Admit complete revoke depth and node counts before mutation

Admit complete revoke depth and node counts before mutation; prove every descendant becomes stale atomically.

**Requires:** `M-04.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.04 — Specify versioned typed messages and bounded endpoint queues, bytes, handles, correlation IDs and unknown-required rejection

Specify versioned typed messages and bounded endpoint queues, bytes, handles, correlation IDs and unknown-required rejection.

**Requires:** `M-04.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.05 — Implement deadlines, cancellation, duplicate/late replies, backpressure and peer-death rollback across all requests

Implement deadlines, cancellation, duplicate/late replies, backpressure and peer-death rollback across all requests.

**Requires:** `M-04.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.06 — Build service discovery, dependency readiness, restart budgets, quarantine and reverse shutdown with a minimal supervisor

Build service discovery, dependency readiness, restart budgets, quarantine and reverse shutdown with a minimal supervisor.

**Requires:** `M-04.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.07 — Define credentials, account/session identity, privilege drop and default-deny operation matrices before wiring login or elevation

Define credentials, account/session identity, privilege drop and default-deny operation matrices before wiring login or elevation.

**Requires:** `M-04.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-04 — Bounded development handoff: Give programs explicit authority and safe communication

Generation-checked rights, bounded IPC and a minimal restartable supervisor with declared service identity.

**Requires:** `M-04.07`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-04.08 — Integrate secrets, trust stores, package signatures, update policy and audited consent using reviewed cryptographic implementations

Integrate secrets, trust stores, package signatures, update policy and audited consent using reviewed cryptographic implementations.

**Requires:** `M-04.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.09 — Join process launch, driver grants, file portals, global input/display and remote operations to the same authority checks

Join process launch, driver grants, file portals, global input/display and remote operations to the same authority checks.

**Requires:** `M-04.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-04.10 — Prove hostile cross-user/cross-process requests, parent death, revocation races, stale handles and unavailable dependencies

Prove hostile cross-user/cross-process requests, parent death, revocation races, stale handles and unavailable dependencies.

**Requires:** `M-04.09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-AU-001](#f-au-001) | feature | opaque capability handle |
| [F-AU-002](#f-au-002) | feature | least-privilege process grant |
| [F-AU-003](#f-au-003) | feature | capability derivation |
| [F-AU-004](#f-au-004) | feature | exhaustive revoke |
| [F-AU-005](#f-au-005) | feature | atomic handle transfer |
| [F-AU-006](#f-au-006) | feature | typed IPC schema |
| [F-AU-007](#f-au-007) | feature | bounded IPC endpoint |
| [F-AU-008](#f-au-008) | feature | request/reply correlation |
| [F-AU-009](#f-au-009) | feature | peer-death semantics |
| [F-AU-010](#f-au-010) | feature | service discovery |
| [F-AU-011](#f-au-011) | feature | service authority profile |
| [F-AU-012](#f-au-012) | feature | default-deny authorization |
| [F-AU-013](#f-au-013) | feature | privileged operation matrix |
| [F-AU-014](#f-au-014) | feature | credentials |
| [F-AU-015](#f-au-015) | feature | credential drop |
| [F-AU-016](#f-au-016) | feature | login authentication |
| [F-AU-017](#f-au-017) | feature | user/account management |
| [F-AU-018](#f-au-018) | feature | session identity |
| [F-AU-019](#f-au-019) | feature | screen lock |
| [F-AU-020](#f-au-020) | feature | elevation/consent |
| [F-AU-021](#f-au-021) | feature | permission dashboard |
| [F-AU-022](#f-au-022) | feature | secrets service |
| [F-AU-023](#f-au-023) | feature | trust store |
| [F-AU-024](#f-au-024) | feature | package signature |
| [F-AU-025](#f-au-025) | feature | secure update |
| [F-AU-026](#f-au-026) | feature | sandbox policy |
| [F-AU-027](#f-au-027) | feature | executable admission |
| [F-AU-028](#f-au-028) | feature | audit trail |
| [F-AU-029](#f-au-029) | feature | privacy model |
| [F-AU-030](#f-au-030) | feature | secure erase/key destruction |
| [F-AU-031](#f-au-031) | feature | exploit mitigation |
| [F-AU-032](#f-au-032) | feature | security update response |
| [F-AU-033](#f-au-033) | feature | firewall policy |
| [F-AU-034](#f-au-034) | feature | VPN/tunnel authority |
| [F-AU-035](#f-au-035) | feature | remote access security |
| [F-AU-036](#f-au-036) | feature | service lifecycle |
| [F-AU-037](#f-au-037) | feature | service management endpoint |
| [F-AU-038](#f-au-038) | feature | cryptographic service/API |
| [F-AU-039](#f-au-039) | feature | certificate/key lifecycle |
| [F-AU-040](#f-au-040) | feature | data-at-rest policy |
| [C-P4.1](#c-p4-1) | contract | typed handle table, derivation and revoke |
| [C-P4.2](#c-p4-2) | contract | bounded endpoint and transactional transfer |
| [C-P4.3](#c-p4-3) | contract | zlIDL schema and generated conformance |
| [C-P4.4](#c-p4-4) | contract | service supervisor |
| [C-P4.5](#c-p4-5) | contract | authenticated sessions and process control |
| [C-DA-18](#c-da-18) | contract | bounded typed IPC and shared objects |
| [C-DA-19](#c-da-19) | contract | service and app supervisor |
| [C-DA-26](#c-da-26) | contract | credential and privileged-operation matrix |
| [T-SVC-001](#t-svc-001) | target | Init and Service Supervisor |
| [T-SVC-002](#t-svc-002) | target | Process Manager |
| [T-SVC-003](#t-svc-003) | target | Capability/Handle Broker |
| [T-SVC-004](#t-svc-004) | target | zlIDL Schema Registry |
| [T-SVC-005](#t-svc-005) | target | Session Manager |
| [T-SVC-006](#t-svc-006) | target | User/Credential Service |
| [T-SVC-007](#t-svc-007) | target | Policy/Authorization Service |
| [T-SVC-008](#t-svc-008) | target | Secrets/Keyring Service |
| [T-SVC-010](#t-svc-010) | target | Clock/Timezone Service |
| [T-SVC-011](#t-svc-011) | target | Resource/Quota Manager |

<a id="f-au-001"></a>
## F-AU-001 — opaque capability handle

**Original requirement:** unforgeable table index plus generation, type, rights, owner, and audit identity

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-001.01 — Reconcile existing opaque capability handle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for opaque capability handle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: unforgeable table index plus generation, type, rights, owner, and audit identity
- [ ] **F-AU-001.02 — Freeze the exact contract for opaque capability handle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: unforgeable table index plus generation, type, rights, owner, and audit identity. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-001.03 — Implement/prove: unforgeable table index plus generation**
  - Action: For opaque capability handle, implement or reuse and verify this exact obligation: unforgeable table index plus generation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unforgeable table index plus generation; retain observable state/resource expectations.
- [ ] **F-AU-001.04 — Implement/prove: type**
  - Action: For opaque capability handle, implement or reuse and verify this exact obligation: type. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for type; retain observable state/resource expectations.
- [ ] **F-AU-001.05 — Implement/prove: rights**
  - Action: For opaque capability handle, implement or reuse and verify this exact obligation: rights. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rights; retain observable state/resource expectations.
- [ ] **F-AU-001.06 — Implement/prove: owner**
  - Action: For opaque capability handle, implement or reuse and verify this exact obligation: owner. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owner; retain observable state/resource expectations.
- [ ] **F-AU-001.07 — Implement/prove: audit identity**
  - Action: For opaque capability handle, implement or reuse and verify this exact obligation: audit identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for audit identity; retain observable state/resource expectations.
- [ ] **F-AU-001.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to opaque capability handle: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-001.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-001.09 — Integrate into the real consumer and runtime route**
  - Action: Wire opaque capability handle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-001.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-001.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for opaque capability handle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-001.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-002"></a>
## F-AU-002 — least-privilege process grant

**Original requirement:** launch receives only manifest/user/session-authorized handles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-002.01 — Reconcile existing least-privilege process grant**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for least-privilege process grant. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: launch receives only manifest/user/session-authorized handles
- [ ] **F-AU-002.02 — Freeze the exact contract for least-privilege process grant**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: launch receives only manifest/user/session-authorized handles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-002.03 — Implement/prove: launch receives only manifest/user/session-authorized handles**
  - Action: For least-privilege process grant, implement or reuse and verify this exact obligation: launch receives only manifest/user/session-authorized handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for launch receives only manifest/user/session-authorized handles; retain observable state/resource expectations.
- [ ] **F-AU-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to least-privilege process grant: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire least-privilege process grant into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for least-privilege process grant as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-003"></a>
## F-AU-003 — capability derivation

**Original requirement:** source DERIVE/COPY right, destination insertion right, explicit parent-child graph

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-003.01 — Reconcile existing capability derivation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for capability derivation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source DERIVE/COPY right, destination insertion right, explicit parent-child graph
- [ ] **F-AU-003.02 — Freeze the exact contract for capability derivation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source DERIVE/COPY right, destination insertion right, explicit parent-child graph. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-003.03 — Implement/prove: source DERIVE/COPY right**
  - Action: For capability derivation, implement or reuse and verify this exact obligation: source DERIVE/COPY right. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source DERIVE/COPY right; retain observable state/resource expectations.
- [ ] **F-AU-003.04 — Implement/prove: destination insertion right**
  - Action: For capability derivation, implement or reuse and verify this exact obligation: destination insertion right. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for destination insertion right; retain observable state/resource expectations.
- [ ] **F-AU-003.05 — Implement/prove: explicit parent-child graph**
  - Action: For capability derivation, implement or reuse and verify this exact obligation: explicit parent-child graph. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit parent-child graph; retain observable state/resource expectations.
- [ ] **F-AU-003.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to capability derivation: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-003.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-003.07 — Integrate into the real consumer and runtime route**
  - Action: Wire capability derivation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-003.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-003.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for capability derivation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-003.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-004"></a>
## F-AU-004 — exhaustive revoke

**Original requirement:** depth and total-node bounds are admitted before mutation; all descendants stale

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-004.01 — Reconcile existing exhaustive revoke**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for exhaustive revoke. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: depth and total-node bounds are admitted before mutation; all descendants stale
- [ ] **F-AU-004.02 — Freeze the exact contract for exhaustive revoke**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: depth and total-node bounds are admitted before mutation; all descendants stale. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-004.03 — Implement/prove: depth and total-node bounds are admitted before mutation**
  - Action: For exhaustive revoke, implement or reuse and verify this exact obligation: depth and total-node bounds are admitted before mutation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for depth and total-node bounds are admitted before mutation; retain observable state/resource expectations.
- [ ] **F-AU-004.04 — Implement/prove: all descendants stale**
  - Action: For exhaustive revoke, implement or reuse and verify this exact obligation: all descendants stale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for all descendants stale; retain observable state/resource expectations.
- [ ] **F-AU-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to exhaustive revoke: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire exhaustive revoke into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for exhaustive revoke as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-005"></a>
## F-AU-005 — atomic handle transfer

**Original requirement:** destination reservation, rights matrix, single commit point, exact rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-005.01 — Reconcile existing atomic handle transfer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomic handle transfer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: destination reservation, rights matrix, single commit point, exact rollback
- [ ] **F-AU-005.02 — Freeze the exact contract for atomic handle transfer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: destination reservation, rights matrix, single commit point, exact rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-005.03 — Implement/prove: destination reservation**
  - Action: For atomic handle transfer, implement or reuse and verify this exact obligation: destination reservation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for destination reservation; retain observable state/resource expectations.
- [ ] **F-AU-005.04 — Implement/prove: rights matrix**
  - Action: For atomic handle transfer, implement or reuse and verify this exact obligation: rights matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rights matrix; retain observable state/resource expectations.
- [ ] **F-AU-005.05 — Implement/prove: single commit point**
  - Action: For atomic handle transfer, implement or reuse and verify this exact obligation: single commit point. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for single commit point; retain observable state/resource expectations.
- [ ] **F-AU-005.06 — Implement/prove: exact rollback**
  - Action: For atomic handle transfer, implement or reuse and verify this exact obligation: exact rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact rollback; retain observable state/resource expectations.
- [ ] **F-AU-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomic handle transfer: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire atomic handle transfer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomic handle transfer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-006"></a>
## F-AU-006 — typed IPC schema

**Original requirement:** generated length-delimited versioned messages with unknown-required rejection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-006.01 — Reconcile existing typed IPC schema**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for typed IPC schema. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated length-delimited versioned messages with unknown-required rejection
- [ ] **F-AU-006.02 — Freeze the exact contract for typed IPC schema**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated length-delimited versioned messages with unknown-required rejection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-006.03 — Implement/prove: generated length-delimited versioned messages with unknown-required rejection**
  - Action: For typed IPC schema, implement or reuse and verify this exact obligation: generated length-delimited versioned messages with unknown-required rejection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated length-delimited versioned messages with unknown-required rejection; retain observable state/resource expectations.
- [ ] **F-AU-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to typed IPC schema: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire typed IPC schema into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for typed IPC schema as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-007"></a>
## F-AU-007 — bounded IPC endpoint

**Original requirement:** message/byte/handle quotas, capacity-aware receive, backpressure, drop counters

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-007.01 — Reconcile existing bounded IPC endpoint**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for bounded IPC endpoint. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: message/byte/handle quotas, capacity-aware receive, backpressure, drop counters
- [ ] **F-AU-007.02 — Freeze the exact contract for bounded IPC endpoint**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: message/byte/handle quotas, capacity-aware receive, backpressure, drop counters. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-007.03 — Implement/prove: message/byte/handle quotas**
  - Action: For bounded IPC endpoint, implement or reuse and verify this exact obligation: message/byte/handle quotas. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for message/byte/handle quotas; retain observable state/resource expectations.
- [ ] **F-AU-007.04 — Implement/prove: capacity-aware receive**
  - Action: For bounded IPC endpoint, implement or reuse and verify this exact obligation: capacity-aware receive. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capacity-aware receive; retain observable state/resource expectations.
- [ ] **F-AU-007.05 — Implement/prove: backpressure**
  - Action: For bounded IPC endpoint, implement or reuse and verify this exact obligation: backpressure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backpressure; retain observable state/resource expectations.
- [ ] **F-AU-007.06 — Implement/prove: drop counters**
  - Action: For bounded IPC endpoint, implement or reuse and verify this exact obligation: drop counters. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drop counters; retain observable state/resource expectations.
- [ ] **F-AU-007.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to bounded IPC endpoint: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-007.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-007.08 — Integrate into the real consumer and runtime route**
  - Action: Wire bounded IPC endpoint into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-007.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-007.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for bounded IPC endpoint as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-007.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-008"></a>
## F-AU-008 — request/reply correlation

**Original requirement:** opaque IDs, deadline, cancel, duplicate/late reply behavior, peer identity

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-008.01 — Reconcile existing request/reply correlation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for request/reply correlation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: opaque IDs, deadline, cancel, duplicate/late reply behavior, peer identity
- [ ] **F-AU-008.02 — Freeze the exact contract for request/reply correlation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: opaque IDs, deadline, cancel, duplicate/late reply behavior, peer identity. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-008.03 — Implement/prove: opaque IDs**
  - Action: For request/reply correlation, implement or reuse and verify this exact obligation: opaque IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opaque IDs; retain observable state/resource expectations.
- [ ] **F-AU-008.04 — Implement/prove: deadline**
  - Action: For request/reply correlation, implement or reuse and verify this exact obligation: deadline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadline; retain observable state/resource expectations.
- [ ] **F-AU-008.05 — Implement/prove: cancel**
  - Action: For request/reply correlation, implement or reuse and verify this exact obligation: cancel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancel; retain observable state/resource expectations.
- [ ] **F-AU-008.06 — Implement/prove: duplicate/late reply behavior**
  - Action: For request/reply correlation, implement or reuse and verify this exact obligation: duplicate/late reply behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for duplicate/late reply behavior; retain observable state/resource expectations.
- [ ] **F-AU-008.07 — Implement/prove: peer identity**
  - Action: For request/reply correlation, implement or reuse and verify this exact obligation: peer identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-008.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for peer identity; retain observable state/resource expectations.
- [ ] **F-AU-008.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to request/reply correlation: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-008.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-008.09 — Integrate into the real consumer and runtime route**
  - Action: Wire request/reply correlation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-008.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-008.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for request/reply correlation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-008.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-009"></a>
## F-AU-009 — peer-death semantics

**Original requirement:** waiters wake, transactions abort, handles/surfaces/rings clean up

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-009.01 — Reconcile existing peer-death semantics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for peer-death semantics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: waiters wake, transactions abort, handles/surfaces/rings clean up
- [ ] **F-AU-009.02 — Freeze the exact contract for peer-death semantics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: waiters wake, transactions abort, handles/surfaces/rings clean up. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-009.03 — Implement/prove: waiters wake**
  - Action: For peer-death semantics, implement or reuse and verify this exact obligation: waiters wake. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for waiters wake; retain observable state/resource expectations.
- [ ] **F-AU-009.04 — Implement/prove: transactions abort**
  - Action: For peer-death semantics, implement or reuse and verify this exact obligation: transactions abort. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for transactions abort; retain observable state/resource expectations.
- [ ] **F-AU-009.05 — Implement/prove: handles/surfaces/rings clean up**
  - Action: For peer-death semantics, implement or reuse and verify this exact obligation: handles/surfaces/rings clean up. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handles/surfaces/rings clean up; retain observable state/resource expectations.
- [ ] **F-AU-009.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to peer-death semantics: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-009.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-009.07 — Integrate into the real consumer and runtime route**
  - Action: Wire peer-death semantics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-009.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-009.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for peer-death semantics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-009.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-010"></a>
## F-AU-010 — service discovery

**Original requirement:** authenticated typed endpoint registry with version/capability negotiation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-010.01 — Reconcile existing service discovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for service discovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated typed endpoint registry with version/capability negotiation
- [ ] **F-AU-010.02 — Freeze the exact contract for service discovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated typed endpoint registry with version/capability negotiation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-010.03 — Implement/prove: authenticated typed endpoint registry with version/capability negotiation**
  - Action: For service discovery, implement or reuse and verify this exact obligation: authenticated typed endpoint registry with version/capability negotiation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated typed endpoint registry with version/capability negotiation; retain observable state/resource expectations.
- [ ] **F-AU-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to service discovery: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire service discovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for service discovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-011"></a>
## F-AU-011 — service authority profile

**Original requirement:** role, exposure, endpoints, budgets, dependencies, management and audit declared

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-011.01 — Reconcile existing service authority profile**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for service authority profile. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: role, exposure, endpoints, budgets, dependencies, management and audit declared
- [ ] **F-AU-011.02 — Freeze the exact contract for service authority profile**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: role, exposure, endpoints, budgets, dependencies, management and audit declared. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-011.03 — Implement/prove: role**
  - Action: For service authority profile, implement or reuse and verify this exact obligation: role. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for role; retain observable state/resource expectations.
- [ ] **F-AU-011.04 — Implement/prove: exposure**
  - Action: For service authority profile, implement or reuse and verify this exact obligation: exposure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exposure; retain observable state/resource expectations.
- [ ] **F-AU-011.05 — Implement/prove: endpoints**
  - Action: For service authority profile, implement or reuse and verify this exact obligation: endpoints. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for endpoints; retain observable state/resource expectations.
- [ ] **F-AU-011.06 — Implement/prove: budgets**
  - Action: For service authority profile, implement or reuse and verify this exact obligation: budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for budgets; retain observable state/resource expectations.
- [ ] **F-AU-011.07 — Implement/prove: dependencies**
  - Action: For service authority profile, implement or reuse and verify this exact obligation: dependencies. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-011.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependencies; retain observable state/resource expectations.
- [ ] **F-AU-011.08 — Implement/prove: management and audit declared**
  - Action: For service authority profile, implement or reuse and verify this exact obligation: management and audit declared. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-011.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for management and audit declared; retain observable state/resource expectations.
- [ ] **F-AU-011.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to service authority profile: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-011.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-011.10 — Integrate into the real consumer and runtime route**
  - Action: Wire service authority profile into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-011.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-011.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for service authority profile as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-011.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-012"></a>
## F-AU-012 — default-deny authorization

**Original requirement:** complete mediation with deny-overrides and missing-policy failure

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-012.01 — Reconcile existing default-deny authorization**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for default-deny authorization. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: complete mediation with deny-overrides and missing-policy failure
- [ ] **F-AU-012.02 — Freeze the exact contract for default-deny authorization**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: complete mediation with deny-overrides and missing-policy failure. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-012.03 — Implement/prove: complete mediation with deny-overrides and missing-policy failure**
  - Action: For default-deny authorization, implement or reuse and verify this exact obligation: complete mediation with deny-overrides and missing-policy failure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete mediation with deny-overrides and missing-policy failure; retain observable state/resource expectations.
- [ ] **F-AU-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to default-deny authorization: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire default-deny authorization into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for default-deny authorization as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-013"></a>
## F-AU-013 — privileged operation matrix

**Original requirement:** mount, MMIO, DMA, device, input, scanout, signal, debug, power and admin rights explicit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-013.01 — Reconcile existing privileged operation matrix**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for privileged operation matrix. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: mount, MMIO, DMA, device, input, scanout, signal, debug, power and admin rights explicit
- [ ] **F-AU-013.02 — Freeze the exact contract for privileged operation matrix**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: mount, MMIO, DMA, device, input, scanout, signal, debug, power and admin rights explicit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-013.03 — Implement/prove: mount**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: mount. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mount; retain observable state/resource expectations.
- [ ] **F-AU-013.04 — Implement/prove: MMIO**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: MMIO. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MMIO; retain observable state/resource expectations.
- [ ] **F-AU-013.05 — Implement/prove: DMA**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: DMA. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DMA; retain observable state/resource expectations.
- [ ] **F-AU-013.06 — Implement/prove: device**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device; retain observable state/resource expectations.
- [ ] **F-AU-013.07 — Implement/prove: input**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input; retain observable state/resource expectations.
- [ ] **F-AU-013.08 — Implement/prove: scanout**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: scanout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scanout; retain observable state/resource expectations.
- [ ] **F-AU-013.09 — Implement/prove: signal**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: signal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signal; retain observable state/resource expectations.
- [ ] **F-AU-013.10 — Implement/prove: debug**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: debug. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for debug; retain observable state/resource expectations.
- [ ] **F-AU-013.11 — Implement/prove: power and admin rights explicit**
  - Action: For privileged operation matrix, implement or reuse and verify this exact obligation: power and admin rights explicit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-013.10.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for power and admin rights explicit; retain observable state/resource expectations.
- [ ] **F-AU-013.12 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to privileged operation matrix: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-013.11.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-013.13 — Integrate into the real consumer and runtime route**
  - Action: Wire privileged operation matrix into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-013.12.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-013.14 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for privileged operation matrix as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-013.13.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-014"></a>
## F-AU-014 — credentials

**Original requirement:** real/effective/saved UID/GID and groups with atomic pre-state transition matrix

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-014.01 — Reconcile existing credentials**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for credentials. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: real/effective/saved UID/GID and groups with atomic pre-state transition matrix
- [ ] **F-AU-014.02 — Freeze the exact contract for credentials**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: real/effective/saved UID/GID and groups with atomic pre-state transition matrix. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-014.03 — Implement/prove: real/effective/saved UID/GID and groups with atomic pre-state transition matrix**
  - Action: For credentials, implement or reuse and verify this exact obligation: real/effective/saved UID/GID and groups with atomic pre-state transition matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for real/effective/saved UID/GID and groups with atomic pre-state transition matrix; retain observable state/resource expectations.
- [ ] **F-AU-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to credentials: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire credentials into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for credentials as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-015"></a>
## F-AU-015 — credential drop

**Original requirement:** failure checked and closed; session never starts after failed drop

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-015.01 — Reconcile existing credential drop**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for credential drop. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: failure checked and closed; session never starts after failed drop
- [ ] **F-AU-015.02 — Freeze the exact contract for credential drop**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: failure checked and closed; session never starts after failed drop. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-015.03 — Implement/prove: failure checked and closed**
  - Action: For credential drop, implement or reuse and verify this exact obligation: failure checked and closed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failure checked and closed; retain observable state/resource expectations.
- [ ] **F-AU-015.04 — Implement/prove: session never starts after failed drop**
  - Action: For credential drop, implement or reuse and verify this exact obligation: session never starts after failed drop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for session never starts after failed drop; retain observable state/resource expectations.
- [ ] **F-AU-015.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to credential drop: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-015.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-015.06 — Integrate into the real consumer and runtime route**
  - Action: Wire credential drop into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-015.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-015.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for credential drop as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-015.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-016"></a>
## F-AU-016 — login authentication

**Original requirement:** salted memory-hard secret verification, rate limits, secret zero/log policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-016.01 — Reconcile existing login authentication**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for login authentication. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: salted memory-hard secret verification, rate limits, secret zero/log policy
- [ ] **F-AU-016.02 — Freeze the exact contract for login authentication**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: salted memory-hard secret verification, rate limits, secret zero/log policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-016.03 — Implement/prove: salted memory-hard secret verification**
  - Action: For login authentication, implement or reuse and verify this exact obligation: salted memory-hard secret verification. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for salted memory-hard secret verification; retain observable state/resource expectations.
- [ ] **F-AU-016.04 — Implement/prove: rate limits**
  - Action: For login authentication, implement or reuse and verify this exact obligation: rate limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rate limits; retain observable state/resource expectations.
- [ ] **F-AU-016.05 — Implement/prove: secret zero/log policy**
  - Action: For login authentication, implement or reuse and verify this exact obligation: secret zero/log policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secret zero/log policy; retain observable state/resource expectations.
- [ ] **F-AU-016.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to login authentication: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-016.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-016.07 — Integrate into the real consumer and runtime route**
  - Action: Wire login authentication into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-016.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-016.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for login authentication as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-016.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-017"></a>
## F-AU-017 — user/account management

**Original requirement:** identities, groups, home/profile, lock/disable/recovery and admin delegation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-017.01 — Reconcile existing user/account management**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for user/account management. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: identities, groups, home/profile, lock/disable/recovery and admin delegation
- [ ] **F-AU-017.02 — Freeze the exact contract for user/account management**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: identities, groups, home/profile, lock/disable/recovery and admin delegation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-017.03 — Implement/prove: identities**
  - Action: For user/account management, implement or reuse and verify this exact obligation: identities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identities; retain observable state/resource expectations.
- [ ] **F-AU-017.04 — Implement/prove: groups**
  - Action: For user/account management, implement or reuse and verify this exact obligation: groups. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for groups; retain observable state/resource expectations.
- [ ] **F-AU-017.05 — Implement/prove: home/profile**
  - Action: For user/account management, implement or reuse and verify this exact obligation: home/profile. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for home/profile; retain observable state/resource expectations.
- [ ] **F-AU-017.06 — Implement/prove: lock/disable/recovery and admin delegation**
  - Action: For user/account management, implement or reuse and verify this exact obligation: lock/disable/recovery and admin delegation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lock/disable/recovery and admin delegation; retain observable state/resource expectations.
- [ ] **F-AU-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to user/account management: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire user/account management into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for user/account management as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-018"></a>
## F-AU-018 — session identity

**Original requirement:** authenticated local/remote session owns display, input, clipboard, notifications and apps

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-018.01 — Reconcile existing session identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for session identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated local/remote session owns display, input, clipboard, notifications and apps
- [ ] **F-AU-018.02 — Freeze the exact contract for session identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated local/remote session owns display, input, clipboard, notifications and apps. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-018.03 — Implement/prove: authenticated local/remote session owns display**
  - Action: For session identity, implement or reuse and verify this exact obligation: authenticated local/remote session owns display. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated local/remote session owns display; retain observable state/resource expectations.
- [ ] **F-AU-018.04 — Implement/prove: input**
  - Action: For session identity, implement or reuse and verify this exact obligation: input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input; retain observable state/resource expectations.
- [ ] **F-AU-018.05 — Implement/prove: clipboard**
  - Action: For session identity, implement or reuse and verify this exact obligation: clipboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clipboard; retain observable state/resource expectations.
- [ ] **F-AU-018.06 — Implement/prove: notifications and apps**
  - Action: For session identity, implement or reuse and verify this exact obligation: notifications and apps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notifications and apps; retain observable state/resource expectations.
- [ ] **F-AU-018.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to session identity: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-018.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-018.08 — Integrate into the real consumer and runtime route**
  - Action: Wire session identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-018.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-018.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for session identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-018.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-019"></a>
## F-AU-019 — screen lock

**Original requirement:** compositor-enforced secure surface; no app spoof/bypass; input and secret isolation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-019.01 — Reconcile existing screen lock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screen lock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: compositor-enforced secure surface; no app spoof/bypass; input and secret isolation
- [ ] **F-AU-019.02 — Freeze the exact contract for screen lock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: compositor-enforced secure surface; no app spoof/bypass; input and secret isolation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-019.03 — Implement/prove: compositor-enforced secure surface**
  - Action: For screen lock, implement or reuse and verify this exact obligation: compositor-enforced secure surface. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compositor-enforced secure surface; retain observable state/resource expectations.
- [ ] **F-AU-019.04 — Implement/prove: no app spoof/bypass**
  - Action: For screen lock, implement or reuse and verify this exact obligation: no app spoof/bypass. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no app spoof/bypass; retain observable state/resource expectations.
- [ ] **F-AU-019.05 — Implement/prove: input and secret isolation**
  - Action: For screen lock, implement or reuse and verify this exact obligation: input and secret isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input and secret isolation; retain observable state/resource expectations.
- [ ] **F-AU-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screen lock: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire screen lock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screen lock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-020"></a>
## F-AU-020 — elevation/consent

**Original requirement:** exact operation/target/arguments/duration/digest bound to fresh user approval

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-020.01 — Reconcile existing elevation/consent**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for elevation/consent. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact operation/target/arguments/duration/digest bound to fresh user approval
- [ ] **F-AU-020.02 — Freeze the exact contract for elevation/consent**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact operation/target/arguments/duration/digest bound to fresh user approval. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-020.03 — Implement/prove: exact operation/target/arguments/duration/digest bound to fresh user approval**
  - Action: For elevation/consent, implement or reuse and verify this exact obligation: exact operation/target/arguments/duration/digest bound to fresh user approval. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact operation/target/arguments/duration/digest bound to fresh user approval; retain observable state/resource expectations.
- [ ] **F-AU-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to elevation/consent: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire elevation/consent into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for elevation/consent as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-021"></a>
## F-AU-021 — permission dashboard

**Original requirement:** users inspect/revoke app data, device, network, background, notification and agent grants

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-021.01 — Reconcile existing permission dashboard**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for permission dashboard. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: users inspect/revoke app data, device, network, background, notification and agent grants
- [ ] **F-AU-021.02 — Freeze the exact contract for permission dashboard**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: users inspect/revoke app data, device, network, background, notification and agent grants. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-021.03 — Implement/prove: users inspect/revoke app data**
  - Action: For permission dashboard, implement or reuse and verify this exact obligation: users inspect/revoke app data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for users inspect/revoke app data; retain observable state/resource expectations.
- [ ] **F-AU-021.04 — Implement/prove: device**
  - Action: For permission dashboard, implement or reuse and verify this exact obligation: device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device; retain observable state/resource expectations.
- [ ] **F-AU-021.05 — Implement/prove: network**
  - Action: For permission dashboard, implement or reuse and verify this exact obligation: network. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for network; retain observable state/resource expectations.
- [ ] **F-AU-021.06 — Implement/prove: background**
  - Action: For permission dashboard, implement or reuse and verify this exact obligation: background. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for background; retain observable state/resource expectations.
- [ ] **F-AU-021.07 — Implement/prove: notification and agent grants**
  - Action: For permission dashboard, implement or reuse and verify this exact obligation: notification and agent grants. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-021.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notification and agent grants; retain observable state/resource expectations.
- [ ] **F-AU-021.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to permission dashboard: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-021.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-021.09 — Integrate into the real consumer and runtime route**
  - Action: Wire permission dashboard into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-021.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-021.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for permission dashboard as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-021.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-022"></a>
## F-AU-022 — secrets service

**Original requirement:** per-user encrypted objects, app-scoped handles, unlock/revoke/rotation and no plaintext logs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-022.01 — Reconcile existing secrets service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for secrets service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-user encrypted objects, app-scoped handles, unlock/revoke/rotation and no plaintext logs
- [ ] **F-AU-022.02 — Freeze the exact contract for secrets service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-user encrypted objects, app-scoped handles, unlock/revoke/rotation and no plaintext logs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-022.03 — Implement/prove: per-user encrypted objects**
  - Action: For secrets service, implement or reuse and verify this exact obligation: per-user encrypted objects. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user encrypted objects; retain observable state/resource expectations.
- [ ] **F-AU-022.04 — Implement/prove: app-scoped handles**
  - Action: For secrets service, implement or reuse and verify this exact obligation: app-scoped handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app-scoped handles; retain observable state/resource expectations.
- [ ] **F-AU-022.05 — Implement/prove: unlock/revoke/rotation and no plaintext logs**
  - Action: For secrets service, implement or reuse and verify this exact obligation: unlock/revoke/rotation and no plaintext logs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unlock/revoke/rotation and no plaintext logs; retain observable state/resource expectations.
- [ ] **F-AU-022.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to secrets service: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-022.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-022.07 — Integrate into the real consumer and runtime route**
  - Action: Wire secrets service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-022.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-022.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for secrets service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-022.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-023"></a>
## F-AU-023 — trust store

**Original requirement:** roots, key IDs, algorithms, policy version, rotation, revocation and time behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-023.01 — Reconcile existing trust store**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for trust store. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: roots, key IDs, algorithms, policy version, rotation, revocation and time behavior
- [ ] **F-AU-023.02 — Freeze the exact contract for trust store**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: roots, key IDs, algorithms, policy version, rotation, revocation and time behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-023.03 — Implement/prove: roots**
  - Action: For trust store, implement or reuse and verify this exact obligation: roots. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for roots; retain observable state/resource expectations.
- [ ] **F-AU-023.04 — Implement/prove: key IDs**
  - Action: For trust store, implement or reuse and verify this exact obligation: key IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for key IDs; retain observable state/resource expectations.
- [ ] **F-AU-023.05 — Implement/prove: algorithms**
  - Action: For trust store, implement or reuse and verify this exact obligation: algorithms. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for algorithms; retain observable state/resource expectations.
- [ ] **F-AU-023.06 — Implement/prove: policy version**
  - Action: For trust store, implement or reuse and verify this exact obligation: policy version. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for policy version; retain observable state/resource expectations.
- [ ] **F-AU-023.07 — Implement/prove: rotation**
  - Action: For trust store, implement or reuse and verify this exact obligation: rotation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-023.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rotation; retain observable state/resource expectations.
- [ ] **F-AU-023.08 — Implement/prove: revocation and time behavior**
  - Action: For trust store, implement or reuse and verify this exact obligation: revocation and time behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-023.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revocation and time behavior; retain observable state/resource expectations.
- [ ] **F-AU-023.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to trust store: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-023.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-023.10 — Integrate into the real consumer and runtime route**
  - Action: Wire trust store into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-023.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-023.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for trust store as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-023.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-024"></a>
## F-AU-024 — package signature

**Original requirement:** publisher-authenticated asymmetric signature distinct from local integrity MAC

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-024.01 — Reconcile existing package signature**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for package signature. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: publisher-authenticated asymmetric signature distinct from local integrity MAC
- [ ] **F-AU-024.02 — Freeze the exact contract for package signature**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: publisher-authenticated asymmetric signature distinct from local integrity MAC. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-024.03 — Implement/prove: publisher-authenticated asymmetric signature distinct from local integrity MAC**
  - Action: For package signature, implement or reuse and verify this exact obligation: publisher-authenticated asymmetric signature distinct from local integrity MAC. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for publisher-authenticated asymmetric signature distinct from local integrity MAC; retain observable state/resource expectations.
- [ ] **F-AU-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to package signature: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire package signature into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for package signature as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-025"></a>
## F-AU-025 — secure update

**Original requirement:** signed metadata, version/rollback protection, staged atomic install and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-025.01 — Reconcile existing secure update**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for secure update. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed metadata, version/rollback protection, staged atomic install and recovery
- [ ] **F-AU-025.02 — Freeze the exact contract for secure update**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed metadata, version/rollback protection, staged atomic install and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-025.03 — Implement/prove: signed metadata**
  - Action: For secure update, implement or reuse and verify this exact obligation: signed metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed metadata; retain observable state/resource expectations.
- [ ] **F-AU-025.04 — Implement/prove: version/rollback protection**
  - Action: For secure update, implement or reuse and verify this exact obligation: version/rollback protection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for version/rollback protection; retain observable state/resource expectations.
- [ ] **F-AU-025.05 — Implement/prove: staged atomic install and recovery**
  - Action: For secure update, implement or reuse and verify this exact obligation: staged atomic install and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for staged atomic install and recovery; retain observable state/resource expectations.
- [ ] **F-AU-025.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to secure update: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-025.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-025.07 — Integrate into the real consumer and runtime route**
  - Action: Wire secure update into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-025.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-025.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for secure update as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-025.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-026"></a>
## F-AU-026 — sandbox policy

**Original requirement:** filesystem, network, device, IPC, process, debug and resource access default denied

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-026.01 — Reconcile existing sandbox policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sandbox policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: filesystem, network, device, IPC, process, debug and resource access default denied
- [ ] **F-AU-026.02 — Freeze the exact contract for sandbox policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: filesystem, network, device, IPC, process, debug and resource access default denied. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-026.03 — Implement/prove: filesystem**
  - Action: For sandbox policy, implement or reuse and verify this exact obligation: filesystem. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for filesystem; retain observable state/resource expectations.
- [ ] **F-AU-026.04 — Implement/prove: network**
  - Action: For sandbox policy, implement or reuse and verify this exact obligation: network. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for network; retain observable state/resource expectations.
- [ ] **F-AU-026.05 — Implement/prove: device**
  - Action: For sandbox policy, implement or reuse and verify this exact obligation: device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device; retain observable state/resource expectations.
- [ ] **F-AU-026.06 — Implement/prove: IPC**
  - Action: For sandbox policy, implement or reuse and verify this exact obligation: IPC. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-026.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IPC; retain observable state/resource expectations.
- [ ] **F-AU-026.07 — Implement/prove: process**
  - Action: For sandbox policy, implement or reuse and verify this exact obligation: process. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-026.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for process; retain observable state/resource expectations.
- [ ] **F-AU-026.08 — Implement/prove: debug and resource access default denied**
  - Action: For sandbox policy, implement or reuse and verify this exact obligation: debug and resource access default denied. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-026.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for debug and resource access default denied; retain observable state/resource expectations.
- [ ] **F-AU-026.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sandbox policy: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-026.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-026.10 — Integrate into the real consumer and runtime route**
  - Action: Wire sandbox policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-026.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-026.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sandbox policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-026.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-027"></a>
## F-AU-027 — executable admission

**Original requirement:** origin/signature/ABI/permissions/W^X/dependencies checked before publication

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-027.01 — Reconcile existing executable admission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for executable admission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: origin/signature/ABI/permissions/W^X/dependencies checked before publication
- [ ] **F-AU-027.02 — Freeze the exact contract for executable admission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: origin/signature/ABI/permissions/W^X/dependencies checked before publication. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-027.03 — Implement/prove: origin/signature/ABI/permissions/W^X/dependencies checked before publication**
  - Action: For executable admission, implement or reuse and verify this exact obligation: origin/signature/ABI/permissions/W^X/dependencies checked before publication. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin/signature/ABI/permissions/W^X/dependencies checked before publication; retain observable state/resource expectations.
- [ ] **F-AU-027.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to executable admission: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-027.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-027.05 — Integrate into the real consumer and runtime route**
  - Action: Wire executable admission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-027.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-027.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for executable admission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-027.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-028"></a>
## F-AU-028 — audit trail

**Original requirement:** durable append/anchor/drop evidence, authority identity, redaction and retention

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-028.01 — Reconcile existing audit trail**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audit trail. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: durable append/anchor/drop evidence, authority identity, redaction and retention
- [ ] **F-AU-028.02 — Freeze the exact contract for audit trail**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: durable append/anchor/drop evidence, authority identity, redaction and retention. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-028.03 — Implement/prove: durable append/anchor/drop evidence**
  - Action: For audit trail, implement or reuse and verify this exact obligation: durable append/anchor/drop evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for durable append/anchor/drop evidence; retain observable state/resource expectations.
- [ ] **F-AU-028.04 — Implement/prove: authority identity**
  - Action: For audit trail, implement or reuse and verify this exact obligation: authority identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authority identity; retain observable state/resource expectations.
- [ ] **F-AU-028.05 — Implement/prove: redaction and retention**
  - Action: For audit trail, implement or reuse and verify this exact obligation: redaction and retention. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for redaction and retention; retain observable state/resource expectations.
- [ ] **F-AU-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audit trail: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire audit trail into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audit trail as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-029"></a>
## F-AU-029 — privacy model

**Original requirement:** data classes, collection purpose, minimization, retention, export, erase, sharing and consent

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-029.01 — Reconcile existing privacy model**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for privacy model. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: data classes, collection purpose, minimization, retention, export, erase, sharing and consent
- [ ] **F-AU-029.02 — Freeze the exact contract for privacy model**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: data classes, collection purpose, minimization, retention, export, erase, sharing and consent. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-029.03 — Implement/prove: data classes**
  - Action: For privacy model, implement or reuse and verify this exact obligation: data classes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for data classes; retain observable state/resource expectations.
- [ ] **F-AU-029.04 — Implement/prove: collection purpose**
  - Action: For privacy model, implement or reuse and verify this exact obligation: collection purpose. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for collection purpose; retain observable state/resource expectations.
- [ ] **F-AU-029.05 — Implement/prove: minimization**
  - Action: For privacy model, implement or reuse and verify this exact obligation: minimization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimization; retain observable state/resource expectations.
- [ ] **F-AU-029.06 — Implement/prove: retention**
  - Action: For privacy model, implement or reuse and verify this exact obligation: retention. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retention; retain observable state/resource expectations.
- [ ] **F-AU-029.07 — Implement/prove: export**
  - Action: For privacy model, implement or reuse and verify this exact obligation: export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for export; retain observable state/resource expectations.
- [ ] **F-AU-029.08 — Implement/prove: erase**
  - Action: For privacy model, implement or reuse and verify this exact obligation: erase. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for erase; retain observable state/resource expectations.
- [ ] **F-AU-029.09 — Implement/prove: sharing and consent**
  - Action: For privacy model, implement or reuse and verify this exact obligation: sharing and consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-029.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sharing and consent; retain observable state/resource expectations.
- [ ] **F-AU-029.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to privacy model: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-029.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-029.11 — Integrate into the real consumer and runtime route**
  - Action: Wire privacy model into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-029.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-029.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for privacy model as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-029.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-030"></a>
## F-AU-030 — secure erase/key destruction

**Original requirement:** defined media/flash limitations and key-based user-data retirement

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-030.01 — Reconcile existing secure erase/key destruction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for secure erase/key destruction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: defined media/flash limitations and key-based user-data retirement
- [ ] **F-AU-030.02 — Freeze the exact contract for secure erase/key destruction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: defined media/flash limitations and key-based user-data retirement. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-030.03 — Implement/prove: defined media/flash limitations and key-based user-data retirement**
  - Action: For secure erase/key destruction, implement or reuse and verify this exact obligation: defined media/flash limitations and key-based user-data retirement. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defined media/flash limitations and key-based user-data retirement; retain observable state/resource expectations.
- [ ] **F-AU-030.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to secure erase/key destruction: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-030.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-030.05 — Integrate into the real consumer and runtime route**
  - Action: Wire secure erase/key destruction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-030.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-030.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for secure erase/key destruction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-030.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-031"></a>
## F-AU-031 — exploit mitigation

**Original requirement:** ASLR, stack canaries, CFI where viable, hardened allocators, guard pages and W^X

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-031.01 — Reconcile existing exploit mitigation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for exploit mitigation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ASLR, stack canaries, CFI where viable, hardened allocators, guard pages and W^X
- [ ] **F-AU-031.02 — Freeze the exact contract for exploit mitigation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ASLR, stack canaries, CFI where viable, hardened allocators, guard pages and W^X. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-031.03 — Implement/prove: ASLR**
  - Action: For exploit mitigation, implement or reuse and verify this exact obligation: ASLR. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ASLR; retain observable state/resource expectations.
- [ ] **F-AU-031.04 — Implement/prove: stack canaries**
  - Action: For exploit mitigation, implement or reuse and verify this exact obligation: stack canaries. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stack canaries; retain observable state/resource expectations.
- [ ] **F-AU-031.05 — Implement/prove: CFI where viable**
  - Action: For exploit mitigation, implement or reuse and verify this exact obligation: CFI where viable. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CFI where viable; retain observable state/resource expectations.
- [ ] **F-AU-031.06 — Implement/prove: hardened allocators**
  - Action: For exploit mitigation, implement or reuse and verify this exact obligation: hardened allocators. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-031.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hardened allocators; retain observable state/resource expectations.
- [ ] **F-AU-031.07 — Implement/prove: guard pages and W^X**
  - Action: For exploit mitigation, implement or reuse and verify this exact obligation: guard pages and W^X. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-031.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for guard pages and W^X; retain observable state/resource expectations.
- [ ] **F-AU-031.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to exploit mitigation: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-031.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-031.09 — Integrate into the real consumer and runtime route**
  - Action: Wire exploit mitigation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-031.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-031.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for exploit mitigation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-031.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-032"></a>
## F-AU-032 — security update response

**Original requirement:** vulnerability identity, affected artifacts, remediation, revocation and user communication

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-032.01 — Reconcile existing security update response**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for security update response. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: vulnerability identity, affected artifacts, remediation, revocation and user communication
- [ ] **F-AU-032.02 — Freeze the exact contract for security update response**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: vulnerability identity, affected artifacts, remediation, revocation and user communication. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-032.03 — Implement/prove: vulnerability identity**
  - Action: For security update response, implement or reuse and verify this exact obligation: vulnerability identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for vulnerability identity; retain observable state/resource expectations.
- [ ] **F-AU-032.04 — Implement/prove: affected artifacts**
  - Action: For security update response, implement or reuse and verify this exact obligation: affected artifacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-032.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for affected artifacts; retain observable state/resource expectations.
- [ ] **F-AU-032.05 — Implement/prove: remediation**
  - Action: For security update response, implement or reuse and verify this exact obligation: remediation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-032.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for remediation; retain observable state/resource expectations.
- [ ] **F-AU-032.06 — Implement/prove: revocation and user communication**
  - Action: For security update response, implement or reuse and verify this exact obligation: revocation and user communication. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-032.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revocation and user communication; retain observable state/resource expectations.
- [ ] **F-AU-032.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to security update response: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-032.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-032.08 — Integrate into the real consumer and runtime route**
  - Action: Wire security update response into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-032.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-032.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for security update response as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-032.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-033"></a>
## F-AU-033 — firewall policy

**Original requirement:** default network posture, app/service rules, logging, user/admin views

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-033.01 — Reconcile existing firewall policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for firewall policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: default network posture, app/service rules, logging, user/admin views
- [ ] **F-AU-033.02 — Freeze the exact contract for firewall policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: default network posture, app/service rules, logging, user/admin views. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-033.03 — Implement/prove: default network posture**
  - Action: For firewall policy, implement or reuse and verify this exact obligation: default network posture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for default network posture; retain observable state/resource expectations.
- [ ] **F-AU-033.04 — Implement/prove: app/service rules**
  - Action: For firewall policy, implement or reuse and verify this exact obligation: app/service rules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/service rules; retain observable state/resource expectations.
- [ ] **F-AU-033.05 — Implement/prove: logging**
  - Action: For firewall policy, implement or reuse and verify this exact obligation: logging. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for logging; retain observable state/resource expectations.
- [ ] **F-AU-033.06 — Implement/prove: user/admin views**
  - Action: For firewall policy, implement or reuse and verify this exact obligation: user/admin views. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user/admin views; retain observable state/resource expectations.
- [ ] **F-AU-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to firewall policy: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire firewall policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for firewall policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-034"></a>
## F-AU-034 — VPN/tunnel authority

**Original requirement:** explicit route/DNS/interface scopes and disconnect leak policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-034.01 — Reconcile existing VPN/tunnel authority**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for VPN/tunnel authority. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit route/DNS/interface scopes and disconnect leak policy
- [ ] **F-AU-034.02 — Freeze the exact contract for VPN/tunnel authority**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit route/DNS/interface scopes and disconnect leak policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-034.03 — Implement/prove: explicit route/DNS/interface scopes and disconnect leak policy**
  - Action: For VPN/tunnel authority, implement or reuse and verify this exact obligation: explicit route/DNS/interface scopes and disconnect leak policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit route/DNS/interface scopes and disconnect leak policy; retain observable state/resource expectations.
- [ ] **F-AU-034.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to VPN/tunnel authority: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-034.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-034.05 — Integrate into the real consumer and runtime route**
  - Action: Wire VPN/tunnel authority into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-034.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-034.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for VPN/tunnel authority as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-034.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-035"></a>
## F-AU-035 — remote access security

**Original requirement:** authenticated encrypted endpoint, device/session identity, rate limits and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-035.01 — Reconcile existing remote access security**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for remote access security. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated encrypted endpoint, device/session identity, rate limits and revocation
- [ ] **F-AU-035.02 — Freeze the exact contract for remote access security**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated encrypted endpoint, device/session identity, rate limits and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-035.03 — Implement/prove: authenticated encrypted endpoint**
  - Action: For remote access security, implement or reuse and verify this exact obligation: authenticated encrypted endpoint. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated encrypted endpoint; retain observable state/resource expectations.
- [ ] **F-AU-035.04 — Implement/prove: device/session identity**
  - Action: For remote access security, implement or reuse and verify this exact obligation: device/session identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device/session identity; retain observable state/resource expectations.
- [ ] **F-AU-035.05 — Implement/prove: rate limits and revocation**
  - Action: For remote access security, implement or reuse and verify this exact obligation: rate limits and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rate limits and revocation; retain observable state/resource expectations.
- [ ] **F-AU-035.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to remote access security: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-035.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-035.07 — Integrate into the real consumer and runtime route**
  - Action: Wire remote access security into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-035.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-035.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for remote access security as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-035.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-036"></a>
## F-AU-036 — service lifecycle

**Original requirement:** admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-036.01 — Reconcile existing service lifecycle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for service lifecycle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback
- [ ] **F-AU-036.02 — Freeze the exact contract for service lifecycle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-036.03 — Implement/prove: admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback**
  - Action: For service lifecycle, implement or reuse and verify this exact obligation: admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for admit/start/ready/dependency-loss/stop/crash/recover/quarantine with bounded rollback; retain observable state/resource expectations.
- [ ] **F-AU-036.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to service lifecycle: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-036.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-036.05 — Integrate into the real consumer and runtime route**
  - Action: Wire service lifecycle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-036.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-036.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for service lifecycle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-036.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-037"></a>
## F-AU-037 — service management endpoint

**Original requirement:** separately authorized health/config/action interface, quotas and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-037.01 — Reconcile existing service management endpoint**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for service management endpoint. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: separately authorized health/config/action interface, quotas and audit
- [ ] **F-AU-037.02 — Freeze the exact contract for service management endpoint**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: separately authorized health/config/action interface, quotas and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-037.03 — Implement/prove: separately authorized health/config/action interface**
  - Action: For service management endpoint, implement or reuse and verify this exact obligation: separately authorized health/config/action interface. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for separately authorized health/config/action interface; retain observable state/resource expectations.
- [ ] **F-AU-037.04 — Implement/prove: quotas and audit**
  - Action: For service management endpoint, implement or reuse and verify this exact obligation: quotas and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quotas and audit; retain observable state/resource expectations.
- [ ] **F-AU-037.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to service management endpoint: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-037.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-037.06 — Integrate into the real consumer and runtime route**
  - Action: Wire service management endpoint into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-037.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-037.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for service management endpoint as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-037.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-038"></a>
## F-AU-038 — cryptographic service/API

**Original requirement:** versioned algorithms, secure randomness, key handles, constant-time policy and test vectors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-038.01 — Reconcile existing cryptographic service/API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cryptographic service/API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned algorithms, secure randomness, key handles, constant-time policy and test vectors
- [ ] **F-AU-038.02 — Freeze the exact contract for cryptographic service/API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned algorithms, secure randomness, key handles, constant-time policy and test vectors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-038.03 — Implement/prove: versioned algorithms**
  - Action: For cryptographic service/API, implement or reuse and verify this exact obligation: versioned algorithms. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned algorithms; retain observable state/resource expectations.
- [ ] **F-AU-038.04 — Implement/prove: secure randomness**
  - Action: For cryptographic service/API, implement or reuse and verify this exact obligation: secure randomness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secure randomness; retain observable state/resource expectations.
- [ ] **F-AU-038.05 — Implement/prove: key handles**
  - Action: For cryptographic service/API, implement or reuse and verify this exact obligation: key handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for key handles; retain observable state/resource expectations.
- [ ] **F-AU-038.06 — Implement/prove: constant-time policy and test vectors**
  - Action: For cryptographic service/API, implement or reuse and verify this exact obligation: constant-time policy and test vectors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-038.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for constant-time policy and test vectors; retain observable state/resource expectations.
- [ ] **F-AU-038.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cryptographic service/API: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-038.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-038.08 — Integrate into the real consumer and runtime route**
  - Action: Wire cryptographic service/API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-038.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-038.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cryptographic service/API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-038.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-039"></a>
## F-AU-039 — certificate/key lifecycle

**Original requirement:** generate/import/store/use/rotate/revoke/export policy and hardware-backed option

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-039.01 — Reconcile existing certificate/key lifecycle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for certificate/key lifecycle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generate/import/store/use/rotate/revoke/export policy and hardware-backed option
- [ ] **F-AU-039.02 — Freeze the exact contract for certificate/key lifecycle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generate/import/store/use/rotate/revoke/export policy and hardware-backed option. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-039.03 — Implement/prove: generate/import/store/use/rotate/revoke/export policy and hardware-backed option**
  - Action: For certificate/key lifecycle, implement or reuse and verify this exact obligation: generate/import/store/use/rotate/revoke/export policy and hardware-backed option. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generate/import/store/use/rotate/revoke/export policy and hardware-backed option; retain observable state/resource expectations.
- [ ] **F-AU-039.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to certificate/key lifecycle: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-039.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-039.05 — Integrate into the real consumer and runtime route**
  - Action: Wire certificate/key lifecycle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-039.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-039.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for certificate/key lifecycle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-039.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-au-040"></a>
## F-AU-040 — data-at-rest policy

**Original requirement:** app/user/system classes, encryption keys, lock state, backup and recovery semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AU-040.01 — Reconcile existing data-at-rest policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for data-at-rest policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: app/user/system classes, encryption keys, lock state, backup and recovery semantics
- [ ] **F-AU-040.02 — Freeze the exact contract for data-at-rest policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: app/user/system classes, encryption keys, lock state, backup and recovery semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AU-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AU-040.03 — Implement/prove: app/user/system classes**
  - Action: For data-at-rest policy, implement or reuse and verify this exact obligation: app/user/system classes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/user/system classes; retain observable state/resource expectations.
- [ ] **F-AU-040.04 — Implement/prove: encryption keys**
  - Action: For data-at-rest policy, implement or reuse and verify this exact obligation: encryption keys. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for encryption keys; retain observable state/resource expectations.
- [ ] **F-AU-040.05 — Implement/prove: lock state**
  - Action: For data-at-rest policy, implement or reuse and verify this exact obligation: lock state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lock state; retain observable state/resource expectations.
- [ ] **F-AU-040.06 — Implement/prove: backup and recovery semantics**
  - Action: For data-at-rest policy, implement or reuse and verify this exact obligation: backup and recovery semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AU-040.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backup and recovery semantics; retain observable state/resource expectations.
- [ ] **F-AU-040.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to data-at-rest policy: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AU-040.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AU-040.08 — Integrate into the real consumer and runtime route**
  - Action: Wire data-at-rest policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AU-040.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AU-040.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for data-at-rest policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AU-040.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p4-1"></a>
## C-P4.1 — typed handle table, derivation and revoke

**Original requirement:** typed handle table, derivation and revoke

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 224.

### Preserved original contract

- **Dependencies/current/provenance:** P3 process object; Serenity/RustOS authority contracts plus NexiOS/Skift as adversarial counterexamples; reject global integer identity and reject treating NexiOS as a working derivation tree.
- **I/O and state:** object, type, rights and optional parent handle in; opaque generation-tagged slot out; `Free -> Reserved -> Live -> Revoking -> Free`.
- **Invariants/failure:** source operation rights and destination insertion rights are checked separately; explicit parent/child graph; child derives from current live parent handles at commit; rights only shrink; spawn/revoke is serialized; stale generation fails; depth and total-node bounds are independent and exhausted before mutation; revoke cannot return success with a reachable descendant; slot/object refs balance.
- **Deterministic proof:** empty/full/one-over, stale reuse, source and destination rights matrix for copy/mint, attempted widening, depth max/max+1, breadth max/max+1 including 9 and 64 child nodes, explicit traversal exhaustion, unrelated aliases, concurrent close/transfer.
- **Target proof:** QEMU processes exchange/revoke shared object; hostile app cannot guess another handle.
- **Receipt/removal:** derivation/revoke trace and leak count; compatibility wrappers map old IDs to restricted handles; remove global IDs at zero callers.

### Execution steps

- [ ] **C-P4.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P4.1.02 — Resolve this contract's exact dependencies**
  - Action: P3 process object; Serenity/RustOS authority contracts plus NexiOS/Skift as adversarial counterexamples; reject global integer identity and reject treating NexiOS as a working derivation tree. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P4.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P4.1.03 — I/O and state — typed handle table, derivation and revoke**
  - Action: object, type, rights and optional parent handle in; opaque generation-tagged slot out; `Free -> Reserved -> Live -> Revoking -> Free`.
  - Requires: C-P4.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P4.1.
- [ ] **C-P4.1.04 — Invariants/failure — typed handle table, derivation and revoke**
  - Action: source operation rights and destination insertion rights are checked separately; explicit parent/child graph; child derives from current live parent handles at commit; rights only shrink; spawn/revoke is serialized; stale generation fails; depth and total-node bounds are independent and exhausted before mutation; revoke cannot return success with a reachable descendant; slot/object refs balance.
  - Requires: C-P4.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P4.1.
- [ ] **C-P4.1.05 — Deterministic proof — typed handle table, derivation and revoke**
  - Action: empty/full/one-over, stale reuse, source and destination rights matrix for copy/mint, attempted widening, depth max/max+1, breadth max/max+1 including 9 and 64 child nodes, explicit traversal exhaustion, unrelated aliases, concurrent close/transfer.
  - Requires: C-P4.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P4.1.
- [ ] **C-P4.1.06 — Target proof — typed handle table, derivation and revoke**
  - Action: QEMU processes exchange/revoke shared object; hostile app cannot guess another handle.
  - Requires: C-P4.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P4.1.
- [ ] **C-P4.1.07 — Receipt/removal — typed handle table, derivation and revoke**
  - Action: derivation/revoke trace and leak count; compatibility wrappers map old IDs to restricted handles; remove global IDs at zero callers.
  - Requires: C-P4.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P4.1.
- [ ] **C-P4.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P4.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P4.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p4-2"></a>
## C-P4.2 — bounded endpoint and transactional transfer

**Original requirement:** bounded endpoint and transactional transfer

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 233.

### Preserved original contract

- **Dependencies/current/provenance:** P4.1 and wait objects; NexiOS/Skift/Lemon; reject Lemon capacity ambiguity and protOS unbounded/UAF IPC.
- **I/O and state:** bounded bytes, handle list, request ID, deadline/cancel in; receive/call/reply/error out; request state plus endpoint `Open -> PeerClosed -> Drained -> Closed`.
- **Invariants/failure:** declared capacity owner; source authority and destination capacity prevalidated; destination slots reserved before source mutation; one journaled commit point; kernel owns envelopes; transfer all-or-nothing; peer death completes waiters; late reply rejected.
- **Deterministic proof:** empty/full/last/one-over and one-way-credit boundaries, receiver-too-small, every nth-handle failure, source/destination rights matrix, peer-close at every stage, cancel race, duplicate reply and rollback; on failure compare byte queues, both handle tables and object refs byte-for-byte; consume-last-message/free-order sanitizer and quotas.
- **Target proof:** QEMU stress between multiple processes and service restart; no desktop crash/leak.
- **Receipt/removal:** queue/latency/outcome/ref counters; old local call adapter remains; remove ambient topics/PID sends after protocol migration.

### Execution steps

- [ ] **C-P4.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P4.2.02 — Resolve this contract's exact dependencies**
  - Action: P4.1 and wait objects; NexiOS/Skift/Lemon; reject Lemon capacity ambiguity and protOS unbounded/UAF IPC. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P4.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P4.2.03 — I/O and state — bounded endpoint and transactional transfer**
  - Action: bounded bytes, handle list, request ID, deadline/cancel in; receive/call/reply/error out; request state plus endpoint `Open -> PeerClosed -> Drained -> Closed`.
  - Requires: C-P4.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P4.2.
- [ ] **C-P4.2.04 — Invariants/failure — bounded endpoint and transactional transfer**
  - Action: declared capacity owner; source authority and destination capacity prevalidated; destination slots reserved before source mutation; one journaled commit point; kernel owns envelopes; transfer all-or-nothing; peer death completes waiters; late reply rejected.
  - Requires: C-P4.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P4.2.
- [ ] **C-P4.2.05 — Deterministic proof — bounded endpoint and transactional transfer**
  - Action: empty/full/last/one-over and one-way-credit boundaries, receiver-too-small, every nth-handle failure, source/destination rights matrix, peer-close at every stage, cancel race, duplicate reply and rollback; on failure compare byte queues, both handle tables and object refs byte-for-byte; consume-last-message/free-order sanitizer and quotas.
  - Requires: C-P4.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P4.2.
- [ ] **C-P4.2.06 — Target proof — bounded endpoint and transactional transfer**
  - Action: QEMU stress between multiple processes and service restart; no desktop crash/leak.
  - Requires: C-P4.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P4.2.
- [ ] **C-P4.2.07 — Receipt/removal — bounded endpoint and transactional transfer**
  - Action: queue/latency/outcome/ref counters; old local call adapter remains; remove ambient topics/PID sends after protocol migration.
  - Requires: C-P4.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P4.2.
- [ ] **C-P4.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P4.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P4.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p4-3"></a>
## C-P4.3 — zlIDL schema and generated conformance

**Original requirement:** zlIDL schema and generated conformance

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 242.

### Preserved original contract

- **Dependencies/current/provenance:** zl imports/records/errors/exact ints and P4.2; Mollen/Lemon generators; reject handwritten client/server layout drift.
- **I/O and state:** versioned interface schema in; zl client/server, kernel validator, docs, trace decoder and tests out; schema `Draft -> FrozenVersion -> Deprecated -> Removed`.
- **Invariants/failure:** deterministic generation; explicit bounds/rights/deadlines/restart/idempotency; unknown version/method returns typed error.
- **Deterministic proof:** golden generation, cross-version matrix, malformed encoding fuzz, max/over-max, generator self-test and generated failing canary.
- **Target proof:** independently built client/server interoperate in QEMU; trace decoder matches wire bytes.
- **Receipt/removal:** schema/generator/output digests; old version supported through stated window; remove handwritten ABI only after differential parity.

### Execution steps

- [ ] **C-P4.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P4.3.02 — Resolve this contract's exact dependencies**
  - Action: zl imports/records/errors/exact ints and P4.2; Mollen/Lemon generators; reject handwritten client/server layout drift. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P4.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P4.3.03 — I/O and state — zlIDL schema and generated conformance**
  - Action: versioned interface schema in; zl client/server, kernel validator, docs, trace decoder and tests out; schema `Draft -> FrozenVersion -> Deprecated -> Removed`.
  - Requires: C-P4.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P4.3.
- [ ] **C-P4.3.04 — Invariants/failure — zlIDL schema and generated conformance**
  - Action: deterministic generation; explicit bounds/rights/deadlines/restart/idempotency; unknown version/method returns typed error.
  - Requires: C-P4.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P4.3.
- [ ] **C-P4.3.05 — Deterministic proof — zlIDL schema and generated conformance**
  - Action: golden generation, cross-version matrix, malformed encoding fuzz, max/over-max, generator self-test and generated failing canary.
  - Requires: C-P4.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P4.3.
- [ ] **C-P4.3.06 — Target proof — zlIDL schema and generated conformance**
  - Action: independently built client/server interoperate in QEMU; trace decoder matches wire bytes.
  - Requires: C-P4.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P4.3.
- [ ] **C-P4.3.07 — Receipt/removal — zlIDL schema and generated conformance**
  - Action: schema/generator/output digests; old version supported through stated window; remove handwritten ABI only after differential parity.
  - Requires: C-P4.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P4.3.
- [ ] **C-P4.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P4.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P4.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p4-4"></a>
## C-P4.4 — service supervisor

**Original requirement:** service supervisor

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 251.

### Preserved original contract

- **Dependencies/current/provenance:** P3/P4 IPC and clocks; Mollen split, snarkOS lifecycle, RustOS fail-closed; reject silent fake fallback.
- **I/O and state:** service manifest/dependencies/limits/restart policy in; ready/degraded/failed handle and health stream out; common service machine.
- **Invariants/failure:** dependency DAG; bounded restart/backoff; reverse shutdown; crash-loop quarantine; fake success forbidden; authority reissued minimally after restart.
- **Deterministic proof:** dependency failure, crash at each state, restart exhaustion, cyclic graph, stale client handle, shutdown ordering.
- **Target proof:** QEMU kills each migratory service; desktop either recovers or exposes precise degraded state.
- **Receipt/removal:** lifecycle trace, restart count and authority grants; boot can select old in-kernel provider; remove fallback per service only after Phase 5 gate.

### Execution steps

- [ ] **C-P4.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P4.4.02 — Resolve this contract's exact dependencies**
  - Action: P3/P4 IPC and clocks; Mollen split, snarkOS lifecycle, RustOS fail-closed; reject silent fake fallback. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P4.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P4.4.03 — I/O and state — service supervisor**
  - Action: service manifest/dependencies/limits/restart policy in; ready/degraded/failed handle and health stream out; common service machine.
  - Requires: C-P4.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P4.4.
- [ ] **C-P4.4.04 — Invariants/failure — service supervisor**
  - Action: dependency DAG; bounded restart/backoff; reverse shutdown; crash-loop quarantine; fake success forbidden; authority reissued minimally after restart.
  - Requires: C-P4.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P4.4.
- [ ] **C-P4.4.05 — Deterministic proof — service supervisor**
  - Action: dependency failure, crash at each state, restart exhaustion, cyclic graph, stale client handle, shutdown ordering.
  - Requires: C-P4.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P4.4.
- [ ] **C-P4.4.06 — Target proof — service supervisor**
  - Action: QEMU kills each migratory service; desktop either recovers or exposes precise degraded state.
  - Requires: C-P4.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P4.4.
- [ ] **C-P4.4.07 — Receipt/removal — service supervisor**
  - Action: lifecycle trace, restart count and authority grants; boot can select old in-kernel provider; remove fallback per service only after Phase 5 gate.
  - Requires: C-P4.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P4.4.
- [ ] **C-P4.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P4.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P4.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p4-5"></a>
## C-P4.5 — authenticated sessions and process control

**Original requirement:** authenticated sessions and process control

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 260.

### Preserved original contract

- **Dependencies/current/provenance:** P4.1-P4.4 plus persistent credential store; Serenity sessions; banan `setreuid`, Lemon fork/drop/kill, duckOS SHM and signal-branch, Brook/hhuOS privileged-operation failures as negative evidence; reject username-only login/elevation.
- **I/O and state:** credential/auth mechanism and session manifest in; session/process/window/file root handles out; `LoggedOut -> Authenticating -> Active -> Locking/Ending -> LoggedOut`.
- **Invariants/failure:** salted memory-hard password verification with secret zeroing and no secret/digest logging; UID/GID/effective/saved transitions validate atomically against pre-state; credential drops are checked and fail closed; fork copies every credential field exactly; elevation requires explicit authenticated grant; signal including signal 0, debug, raw MMIO, mount/unmount, power, scanout and input-grab are completely mediated by distinct handles; logout revokes descendants.
- **Deterministic proof:** wrong/replay/locked credential; `setreuid(-1,0)` and complete real/effective/saved-ID matrix; failed `setgid` blocks session; fork credential snapshot; ordinary-process denial for signal 0/MMIO/mount/power/scanout/input; session cross-access, revoke on logout/crash, elevation denial.
- **Target proof:** QEMU two sessions cannot access each other's processes/files/windows; hardware login/logout leaves system healthy.
- **Receipt/removal:** redacted auth/session/revoke evidence; single trusted recovery session rollback; remove implicit auto-root only after recovery image works.

### Execution steps

- [ ] **C-P4.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P4.5.02 — Resolve this contract's exact dependencies**
  - Action: P4.1-P4.4 plus persistent credential store; Serenity sessions; banan `setreuid`, Lemon fork/drop/kill, duckOS SHM and signal-branch, Brook/hhuOS privileged-operation failures as negative evidence; reject username-only login/elevation. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P4.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P4.5.03 — I/O and state — authenticated sessions and process control**
  - Action: credential/auth mechanism and session manifest in; session/process/window/file root handles out; `LoggedOut -> Authenticating -> Active -> Locking/Ending -> LoggedOut`.
  - Requires: C-P4.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P4.5.
- [ ] **C-P4.5.04 — Invariants/failure — authenticated sessions and process control**
  - Action: salted memory-hard password verification with secret zeroing and no secret/digest logging; UID/GID/effective/saved transitions validate atomically against pre-state; credential drops are checked and fail closed; fork copies every credential field exactly; elevation requires explicit authenticated grant; signal including signal 0, debug, raw MMIO, mount/unmount, power, scanout and input-grab are completely mediated by distinct handles; logout revokes descendants.
  - Requires: C-P4.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P4.5.
- [ ] **C-P4.5.05 — Deterministic proof — authenticated sessions and process control**
  - Action: wrong/replay/locked credential; `setreuid(-1,0)` and complete real/effective/saved-ID matrix; failed `setgid` blocks session; fork credential snapshot; ordinary-process denial for signal 0/MMIO/mount/power/scanout/input; session cross-access, revoke on logout/crash, elevation denial.
  - Requires: C-P4.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P4.5.
- [ ] **C-P4.5.06 — Target proof — authenticated sessions and process control**
  - Action: QEMU two sessions cannot access each other's processes/files/windows; hardware login/logout leaves system healthy.
  - Requires: C-P4.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P4.5.
- [ ] **C-P4.5.07 — Receipt/removal — authenticated sessions and process control**
  - Action: redacted auth/session/revoke evidence; single trusted recovery session rollback; remove implicit auto-root only after recovery image works.
  - Requires: C-P4.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P4.5.
- [ ] **C-P4.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P4.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P4.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-18"></a>
## C-DA-18 — bounded typed IPC and shared objects

**Original requirement:** bounded typed IPC and shared objects

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 455.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-18.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-18.02 — Resolve this contract's exact dependencies**
  - Action: DA-17. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-18.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-18.03 — Deliver — bounded typed IPC and shared objects**
  - Action: generated schemas, opaque endpoints/correlation IDs, sender identity, message/byte quotas, capacity-aware receive, cancellation and revocable shared buffers/surfaces.
  - Requires: C-DA-18.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-18.
- [ ] **C-DA-18.04 — Invariants — bounded typed IPC and shared objects**
  - Action: send/handle transfer is failure-atomic; destination is pre-reserved; only owner or live `SHARE/DERIVE` authority grants; receiver consent and mapping rights are explicit; no global sequential IDs; teardown makes handles stale. IPC uses the DA-07 request envelope and terminal results. A bounded revoke that exhausts traversal capacity returns `PARTIAL(progress, continuation)`; it never truncates silently or returns ordinary success while descendants remain live.
  - Requires: C-DA-18.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-18.
- [ ] **C-DA-18.05 — Proof — bounded typed IPC and shared objects**
  - Action: nth-handle failure, empty/full/last-slot, receiver too small, peer close, guessed ID, grant-self against foreign object, revoke, generation reuse, queue exhaustion and concurrent teardown.
  - Requires: C-DA-18.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-18.
- [ ] **C-DA-18.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-18. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-18.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-19"></a>
## C-DA-19 — service and app supervisor

**Original requirement:** service and app supervisor

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 474.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-19.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-19.02 — Resolve this contract's exact dependencies**
  - Action: DA-17 and DA-18. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-19.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-19.03 — Deliver — service and app supervisor**
  - Action: stable role/protocol version, authority and exposure profiles, dependency graph, authenticated endpoints, connection/message/memory/CPU budgets, readiness/health, startup/shutdown deadlines, crash budget/backoff/quarantine, checkpoint/restore, dependency-loss/reconnect, management authority, audit/redaction policy and ordered reverse teardown.
  - Requires: C-DA-19.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-19.
- [ ] **C-DA-19.04 — Invariants — service and app supervisor**
  - Action: cycles and missing dependencies fail before effects; service publication follows readiness; failure never becomes success; process identity is generation-safe; termination proves death before resource reuse; complete mediation and default-deny apply to endpoints and management; exposure never exceeds role; restart invalidates old endpoint generations and settles requests exactly once.
  - Requires: C-DA-19.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-19.
- [ ] **C-DA-19.05 — Proof — service and app supervisor**
  - Action: cycle, dependency refusal, readiness timeout, crash loop, PID reuse, kill failure, orphan child, restart during request, checkpoint corruption and shutdown deadline; unauthenticated/cross-role endpoint, management denial, connection/ message/memory/CPU exhaustion, dependency loss/reconnect, audit overflow and secret redaction.
  - Requires: C-DA-19.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-19.
- [ ] **C-DA-19.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-19. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-19.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-26"></a>
## C-DA-26 — credential and privileged-operation matrix

**Original requirement:** credential and privileged-operation matrix

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 679.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-26.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-26.02 — Resolve this contract's exact dependencies**
  - Action: session/auth service and typed handles. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-26.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-26.03 — Deliver — credential and privileged-operation matrix**
  - Action: atomic real/effective/saved UID/GID transition policy or native zlOS identity equivalent; explicit rights for signal, debug, mount, MMIO, DMA, power, scanout and input grab.
  - Requires: C-DA-26.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-26.
- [ ] **C-DA-26.04 — Invariants — credential and privileged-operation matrix**
  - Action: checks use pre-state and fail closed; fork copies every identity field exactly; failed credential drop cannot start a session; password secrets use a salted memory-hard KDF and are zeroed/not logged.
  - Requires: C-DA-26.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-26.
- [ ] **C-DA-26.05 — Proof — credential and privileged-operation matrix**
  - Action: ordinary `setreuid(-1,0)` analogue, all transition combinations, fork snapshot, failed group drop, signal zero, foreign process, mount/MMIO/power/scanout/ grab denial and authentication log redaction.
  - Requires: C-DA-26.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-26.
- [ ] **C-DA-26.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-26. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-26.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-svc-001"></a>
## T-SVC-001 — Init and Service Supervisor

**Original requirement:** replace ad-hoc init with DAG, readiness, rollback and reaper

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 18.

### Execution steps

- [ ] **T-SVC-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Init and Service Supervisor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-001.02 — Specify the complete target boundary**
  - Action: Init and Service Supervisor must supply: replace ad-hoc init with DAG, readiness, rollback and reaper. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse Init and Service Supervisor through the shared platform contract, delivering every part of: replace ad-hoc init with DAG, readiness, rollback and reaper. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-001.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Init and Service Supervisor.
- [ ] **T-SVC-001.05 — Qualify and retain this target's own result**
  - Action: Bind Init and Service Supervisor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-002"></a>
## T-SVC-002 — Process Manager

**Original requirement:** create/exec/wait/signal/control through capabilities

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 19.

### Execution steps

- [ ] **T-SVC-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Process Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-002.02 — Specify the complete target boundary**
  - Action: Process Manager must supply: create/exec/wait/signal/control through capabilities. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse Process Manager through the shared platform contract, delivering every part of: create/exec/wait/signal/control through capabilities. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-002.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Process Manager.
- [ ] **T-SVC-002.05 — Qualify and retain this target's own result**
  - Action: Bind Process Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-003"></a>
## T-SVC-003 — Capability/Handle Broker

**Original requirement:** derivation, transfer, revoke, delegation receipts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 20.

### Execution steps

- [ ] **T-SVC-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Capability/Handle Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-003.02 — Specify the complete target boundary**
  - Action: Capability/Handle Broker must supply: derivation, transfer, revoke, delegation receipts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse Capability/Handle Broker through the shared platform contract, delivering every part of: derivation, transfer, revoke, delegation receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-003.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Capability/Handle Broker.
- [ ] **T-SVC-003.05 — Qualify and retain this target's own result**
  - Action: Bind Capability/Handle Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-004"></a>
## T-SVC-004 — zlIDL Schema Registry

**Original requirement:** generated protocols, versions and conformance

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 21.

### Execution steps

- [ ] **T-SVC-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zlIDL Schema Registry to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-004.02 — Specify the complete target boundary**
  - Action: zlIDL Schema Registry must supply: generated protocols, versions and conformance. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse zlIDL Schema Registry through the shared platform contract, delivering every part of: generated protocols, versions and conformance. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-004.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zlIDL Schema Registry.
- [ ] **T-SVC-004.05 — Qualify and retain this target's own result**
  - Action: Bind zlIDL Schema Registry to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-005"></a>
## T-SVC-005 — Session Manager

**Original requirement:** login/session lifecycle, seats and resource roots

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 22.

### Execution steps

- [ ] **T-SVC-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Session Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-005.02 — Specify the complete target boundary**
  - Action: Session Manager must supply: login/session lifecycle, seats and resource roots. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Session Manager through the shared platform contract, delivering every part of: login/session lifecycle, seats and resource roots. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-005.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Session Manager.
- [ ] **T-SVC-005.05 — Qualify and retain this target's own result**
  - Action: Bind Session Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-006"></a>
## T-SVC-006 — User/Credential Service

**Original requirement:** users/groups/credential transitions/password policy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 23.

### Execution steps

- [ ] **T-SVC-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve User/Credential Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-006.02 — Specify the complete target boundary**
  - Action: User/Credential Service must supply: users/groups/credential transitions/password policy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse User/Credential Service through the shared platform contract, delivering every part of: users/groups/credential transitions/password policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-006.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for User/Credential Service.
- [ ] **T-SVC-006.05 — Qualify and retain this target's own result**
  - Action: Bind User/Credential Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-007"></a>
## T-SVC-007 — Policy/Authorization Service

**Original requirement:** deny-by-default operation and object policy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 24.

### Execution steps

- [ ] **T-SVC-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Policy/Authorization Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-007.02 — Specify the complete target boundary**
  - Action: Policy/Authorization Service must supply: deny-by-default operation and object policy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse Policy/Authorization Service through the shared platform contract, delivering every part of: deny-by-default operation and object policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-007.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Policy/Authorization Service.
- [ ] **T-SVC-007.05 — Qualify and retain this target's own result**
  - Action: Bind Policy/Authorization Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-008"></a>
## T-SVC-008 — Secrets/Keyring Service

**Original requirement:** encrypted scoped secrets, consent, revoke and zeroing

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 25.

### Execution steps

- [ ] **T-SVC-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Secrets/Keyring Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-008.02 — Specify the complete target boundary**
  - Action: Secrets/Keyring Service must supply: encrypted scoped secrets, consent, revoke and zeroing. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse Secrets/Keyring Service through the shared platform contract, delivering every part of: encrypted scoped secrets, consent, revoke and zeroing. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-008.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Secrets/Keyring Service.
- [ ] **T-SVC-008.05 — Qualify and retain this target's own result**
  - Action: Bind Secrets/Keyring Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-010"></a>
## T-SVC-010 — Clock/Timezone Service

**Original requirement:** wall clock/timezone separate from kernel deadlines

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 27.

### Execution steps

- [ ] **T-SVC-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Clock/Timezone Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-010.02 — Specify the complete target boundary**
  - Action: Clock/Timezone Service must supply: wall clock/timezone separate from kernel deadlines. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Clock/Timezone Service through the shared platform contract, delivering every part of: wall clock/timezone separate from kernel deadlines. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-010.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Clock/Timezone Service.
- [ ] **T-SVC-010.05 — Qualify and retain this target's own result**
  - Action: Bind Clock/Timezone Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-011"></a>
## T-SVC-011 — Resource/Quota Manager

**Original requirement:** process/service memory, CPU, handles, I/O and connection budgets

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 28.

### Execution steps

- [ ] **T-SVC-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Resource/Quota Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-07, D-08, D-09, D-10, H-04.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-011.02 — Specify the complete target boundary**
  - Action: Resource/Quota Manager must supply: process/service memory, CPU, handles, I/O and connection budgets. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse Resource/Quota Manager through the shared platform contract, delivering every part of: process/service memory, CPU, handles, I/O and connection budgets. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-011.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Resource/Quota Manager.
- [ ] **T-SVC-011.05 — Qualify and retain this target's own result**
  - Action: Bind Resource/Quota Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
