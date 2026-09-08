# MP-00: Preserve the baseline and make every claim traceable

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `tools/; gates/; kernel/metadata/; docs/program/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-00` exports: One selected source tree, working build/test oracles, declared inputs, and separate host/QEMU/physical status.

The handoff enables only its named subset. `CLOSE-00` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-00.01 — Freeze source, dependency and artifact identities

Freeze source, dependency and artifact identities; reconcile generated snapshots with the downloaded full-run ledger.

**Requires:** `D-01`, `D-02`, `D-03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-00.02 — Inventory active worktrees and file owners

Inventory active worktrees and file owners; retain dirty work and select one isolated implementation checkout.

**Requires:** `M-00.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-00.03 — Inventory shipped source, link inputs, boot entrypoints, app routes and tests

Inventory shipped source, link inputs, boot entrypoints, app routes and tests; reject a declared item with no consumer.

**Requires:** `M-00.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-00 — Bounded development handoff: Preserve the baseline and make every claim traceable

One selected source tree, working build/test oracles, declared inputs, and separate host/QEMU/physical status.

**Requires:** `M-00.03`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-00.04 — Preserve the current software display, PS/2, storage, browser and console paths as comparison oracles

Preserve the current software display, PS/2, storage, browser and console paths as comparison oracles.

**Requires:** `M-00.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-00.05 — Make status changes evidence-bound

Make status changes evidence-bound; distinguish positive checks, hardware skips, instruments and unavailable tests.

**Requires:** `M-00.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-00.06 — Attach fuzz, failure-injection, performance, accessibility, security and provenance evidence to the capability being delivered

Attach fuzz, failure-injection, performance, accessibility, security and provenance evidence to the capability being delivered.

**Requires:** `M-00.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-00.07 — Validate all feature, contract, target and current-app/game mappings without treating count coverage as implementation proof

Validate all feature, contract, target and current-app/game mappings without treating count coverage as implementation proof.

**Requires:** `M-00.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-EV-001](#f-ev-001) | feature | canonical feature registry |
| [F-EV-002](#f-ev-002) | feature | exact source snapshot |
| [F-EV-003](#f-ev-003) | feature | license ledger |
| [F-EV-004](#f-ev-004) | feature | dependency lock |
| [F-EV-005](#f-ev-005) | feature | reproducible toolchain manifest |
| [F-EV-006](#f-ev-006) | feature | clean-build proof |
| [F-EV-007](#f-ev-007) | feature | build graph truth |
| [F-EV-008](#f-ev-008) | feature | strict exit propagation |
| [F-EV-009](#f-ev-009) | feature | artifact self-identity |
| [F-EV-010](#f-ev-010) | feature | evidence ladder |
| [F-EV-011](#f-ev-011) | feature | feature maturity ledger |
| [F-EV-012](#f-ev-012) | feature | test inventory parity |
| [F-EV-013](#f-ev-013) | feature | canary/mutation gate |
| [F-EV-014](#f-ev-014) | feature | benchmark receipt |
| [F-EV-015](#f-ev-015) | feature | hardware receipt |
| [F-EV-016](#f-ev-016) | feature | screenshot/video receipt |
| [F-EV-017](#f-ev-017) | feature | crash/panic receipt |
| [F-EV-018](#f-ev-018) | feature | structured event trace |
| [F-EV-019](#f-ev-019) | feature | failure-injection registry |
| [F-EV-020](#f-ev-020) | feature | hostile corpus registry |
| [F-EV-021](#f-ev-021) | feature | performance regression registry |
| [F-EV-022](#f-ev-022) | feature | visual regression registry |
| [F-EV-023](#f-ev-023) | feature | accessibility proof registry |
| [F-EV-024](#f-ev-024) | feature | security claim registry |
| [F-EV-025](#f-ev-025) | feature | independent refutation |
| [F-EV-026](#f-ev-026) | feature | historical decision log |
| [F-EV-027](#f-ev-027) | feature | release notes/changelog |
| [F-EV-028](#f-ev-028) | feature | provenance viewer |
| [C-P0.1](#c-p0-1) | contract | machine-readable feature ledger |
| [C-P0.2](#c-p0-2) | contract | shipped-source and module inventory |
| [C-P0.3](#c-p0-3) | contract | current-asset receipt backfill |
| [C-P0.4](#c-p0-4) | contract | strict outer gate and documentation authority |
| [C-P5.1](#c-p5-1) | contract | Logger and ZLLOG importer/exporter |
| [C-DA-00](#c-da-00) | contract | generated application registry |
| [C-DA-01](#c-da-01) | contract | feature/evidence ledger |
| [C-DA-03](#c-da-03) | contract | artifact and scenario identity |
| [C-DA-08O](#c-da-08o) | contract | observability service |
| [C-DA-39](#c-da-39) | contract | provenance and architecture history |
| [C-DA-40](#c-da-40) | contract | host deterministic gate |
| [C-DA-44](#c-da-44) | contract | independent contradiction gate |
| [C-VX-00](#c-vx-00) | contract | Visual/app/browser evidence registry |
| [C-VX-01](#c-vx-01) | contract | Current application registry repair and oracle |
| [C-VX-48](#c-vx-48) | contract | Visual-regression oracle |
| [C-VX-49](#c-vx-49) | contract | Interaction replay and mutation gate |
| [C-VX-50](#c-vx-50) | contract | App workflow certification matrix |
| [T-SVC-009](#t-svc-009) | target | Audit/Event Service |
| [T-SVC-043](#t-svc-043) | target | Provenance/License Service |

<a id="f-ev-001"></a>
## F-EV-001 — canonical feature registry

**Original requirement:** one generated identity for every feature, owner, dependencies, maturity, routes, artifacts, tests, and weakest evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-001.01 — Reconcile existing canonical feature registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for canonical feature registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: one generated identity for every feature, owner, dependencies, maturity, routes, artifacts, tests, and weakest evidence
- [ ] **F-EV-001.02 — Freeze the exact contract for canonical feature registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: one generated identity for every feature, owner, dependencies, maturity, routes, artifacts, tests, and weakest evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-001.03 — Implement/prove: one generated identity for every feature**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: one generated identity for every feature. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for one generated identity for every feature; retain observable state/resource expectations.
- [ ] **F-EV-001.04 — Implement/prove: owner**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: owner. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owner; retain observable state/resource expectations.
- [ ] **F-EV-001.05 — Implement/prove: dependencies**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: dependencies. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependencies; retain observable state/resource expectations.
- [ ] **F-EV-001.06 — Implement/prove: maturity**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: maturity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for maturity; retain observable state/resource expectations.
- [ ] **F-EV-001.07 — Implement/prove: routes**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: routes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for routes; retain observable state/resource expectations.
- [ ] **F-EV-001.08 — Implement/prove: artifacts**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: artifacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for artifacts; retain observable state/resource expectations.
- [ ] **F-EV-001.09 — Implement/prove: tests**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tests; retain observable state/resource expectations.
- [ ] **F-EV-001.10 — Implement/prove: weakest evidence**
  - Action: For canonical feature registry, implement or reuse and verify this exact obligation: weakest evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-001.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for weakest evidence; retain observable state/resource expectations.
- [ ] **F-EV-001.11 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to canonical feature registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-001.10.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-001.12 — Integrate into the real consumer and runtime route**
  - Action: Wire canonical feature registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-001.11.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-001.13 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for canonical feature registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-001.12.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-002"></a>
## F-EV-002 — exact source snapshot

**Original requirement:** repository, commit, submodules, dirty state, patches, toolchain, and dependency digests recorded

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 off-host copies; unsigned receipt; not a whole-repository snapshot.

### Execution steps

- [ ] **F-EV-002.01 — Reconcile existing exact source snapshot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for exact source snapshot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: repository, commit, submodules, dirty state, patches, toolchain, and dependency digests recorded
- [ ] **F-EV-002.02 — Freeze the exact contract for exact source snapshot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: repository, commit, submodules, dirty state, patches, toolchain, and dependency digests recorded. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-002.03 — Implement/prove: repository**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: repository. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repository; retain observable state/resource expectations.
- [ ] **F-EV-002.04 — Implement/prove: commit**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: commit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for commit; retain observable state/resource expectations.
- [ ] **F-EV-002.05 — Implement/prove: submodules**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: submodules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for submodules; retain observable state/resource expectations.
- [ ] **F-EV-002.06 — Implement/prove: dirty state**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: dirty state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dirty state; retain observable state/resource expectations.
- [ ] **F-EV-002.07 — Implement/prove: patches**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: patches. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for patches; retain observable state/resource expectations.
- [ ] **F-EV-002.08 — Implement/prove: toolchain**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: toolchain. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for toolchain; retain observable state/resource expectations.
- [ ] **F-EV-002.09 — Implement/prove: dependency digests recorded**
  - Action: For exact source snapshot, implement or reuse and verify this exact obligation: dependency digests recorded. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-002.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependency digests recorded; retain observable state/resource expectations.
- [ ] **F-EV-002.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to exact source snapshot: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-002.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-002.11 — Integrate into the real consumer and runtime route**
  - Action: Wire exact source snapshot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-002.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-002.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for exact source snapshot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-002.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-003"></a>
## F-EV-003 — license ledger

**Original requirement:** per-component code, font, icon, firmware, media, dataset, and port licenses travel into artifacts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 license files; 168 inputs lack an established redistribution grant.

### Execution steps

- [ ] **F-EV-003.01 — Reconcile existing license ledger**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for license ledger. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-component code, font, icon, firmware, media, dataset, and port licenses travel into artifacts
- [ ] **F-EV-003.02 — Freeze the exact contract for license ledger**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-component code, font, icon, firmware, media, dataset, and port licenses travel into artifacts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-003.03 — Implement/prove: per-component code**
  - Action: For license ledger, implement or reuse and verify this exact obligation: per-component code. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-component code; retain observable state/resource expectations.
- [ ] **F-EV-003.04 — Implement/prove: font**
  - Action: For license ledger, implement or reuse and verify this exact obligation: font. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for font; retain observable state/resource expectations.
- [ ] **F-EV-003.05 — Implement/prove: icon**
  - Action: For license ledger, implement or reuse and verify this exact obligation: icon. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for icon; retain observable state/resource expectations.
- [ ] **F-EV-003.06 — Implement/prove: firmware**
  - Action: For license ledger, implement or reuse and verify this exact obligation: firmware. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for firmware; retain observable state/resource expectations.
- [ ] **F-EV-003.07 — Implement/prove: media**
  - Action: For license ledger, implement or reuse and verify this exact obligation: media. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for media; retain observable state/resource expectations.
- [ ] **F-EV-003.08 — Implement/prove: dataset**
  - Action: For license ledger, implement or reuse and verify this exact obligation: dataset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dataset; retain observable state/resource expectations.
- [ ] **F-EV-003.09 — Implement/prove: port licenses travel into artifacts**
  - Action: For license ledger, implement or reuse and verify this exact obligation: port licenses travel into artifacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-003.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for port licenses travel into artifacts; retain observable state/resource expectations.
- [ ] **F-EV-003.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to license ledger: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-003.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-003.11 — Integrate into the real consumer and runtime route**
  - Action: Wire license ledger into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-003.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-003.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for license ledger as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-003.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-004"></a>
## F-EV-004 — dependency lock

**Original requirement:** all direct/transitive source and binary inputs are immutable and verified

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** archive cache is retained only on the current host; rolling-distribution archive custody is bound to the recorded signed index, not future archive availability; off-host custody and hermetic rebuild remain EV-002/EV-005 work.

### Execution steps

- [ ] **F-EV-004.01 — Reconcile existing dependency lock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for dependency lock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: all direct/transitive source and binary inputs are immutable and verified
- [ ] **F-EV-004.02 — Freeze the exact contract for dependency lock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: all direct/transitive source and binary inputs are immutable and verified. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-004.03 — Implement/prove: all direct/transitive source and binary inputs are immutable and verified**
  - Action: For dependency lock, implement or reuse and verify this exact obligation: all direct/transitive source and binary inputs are immutable and verified. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for all direct/transitive source and binary inputs are immutable and verified; retain observable state/resource expectations.
- [ ] **F-EV-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to dependency lock: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire dependency lock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for dependency lock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-005"></a>
## F-EV-005 — reproducible toolchain manifest

**Original requirement:** compiler, assembler, linker, sysroot, generated headers, flags, and target ABI are identified

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 hermetic builds; unsigned toolchain; compiler/sysroot sources unarchived.

### Execution steps

- [ ] **F-EV-005.01 — Reconcile existing reproducible toolchain manifest**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reproducible toolchain manifest. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: compiler, assembler, linker, sysroot, generated headers, flags, and target ABI are identified
- [ ] **F-EV-005.02 — Freeze the exact contract for reproducible toolchain manifest**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: compiler, assembler, linker, sysroot, generated headers, flags, and target ABI are identified. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-005.03 — Implement/prove: compiler**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: compiler. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compiler; retain observable state/resource expectations.
- [ ] **F-EV-005.04 — Implement/prove: assembler**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: assembler. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for assembler; retain observable state/resource expectations.
- [ ] **F-EV-005.05 — Implement/prove: linker**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: linker. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for linker; retain observable state/resource expectations.
- [ ] **F-EV-005.06 — Implement/prove: sysroot**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: sysroot. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sysroot; retain observable state/resource expectations.
- [ ] **F-EV-005.07 — Implement/prove: generated headers**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: generated headers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated headers; retain observable state/resource expectations.
- [ ] **F-EV-005.08 — Implement/prove: flags**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: flags. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flags; retain observable state/resource expectations.
- [ ] **F-EV-005.09 — Implement/prove: target ABI are identified**
  - Action: For reproducible toolchain manifest, implement or reuse and verify this exact obligation: target ABI are identified. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-005.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target ABI are identified; retain observable state/resource expectations.
- [ ] **F-EV-005.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reproducible toolchain manifest: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-005.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-005.11 — Integrate into the real consumer and runtime route**
  - Action: Wire reproducible toolchain manifest into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-005.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-005.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reproducible toolchain manifest as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-005.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-006"></a>
## F-EV-006 — clean-build proof

**Original requirement:** empty output directory produces current artifacts without hidden cached prerequisites

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-006.01 — Reconcile existing clean-build proof**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for clean-build proof. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: empty output directory produces current artifacts without hidden cached prerequisites
- [ ] **F-EV-006.02 — Freeze the exact contract for clean-build proof**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: empty output directory produces current artifacts without hidden cached prerequisites. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-006.03 — Implement/prove: empty output directory produces current artifacts without hidden cached prerequisites**
  - Action: For clean-build proof, implement or reuse and verify this exact obligation: empty output directory produces current artifacts without hidden cached prerequisites. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for empty output directory produces current artifacts without hidden cached prerequisites; retain observable state/resource expectations.
- [ ] **F-EV-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to clean-build proof: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire clean-build proof into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for clean-build proof as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-007"></a>
## F-EV-007 — build graph truth

**Original requirement:** source, compiled object, linked image, package, registry, init, and launch-route inventories agree

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 11 conservative scope-only inputs; no per-object binary receipts; future package/service outputs absent.

### Execution steps

- [ ] **F-EV-007.01 — Reconcile existing build graph truth**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for build graph truth. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source, compiled object, linked image, package, registry, init, and launch-route inventories agree
- [ ] **F-EV-007.02 — Freeze the exact contract for build graph truth**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source, compiled object, linked image, package, registry, init, and launch-route inventories agree. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-007.03 — Implement/prove: source**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: source. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source; retain observable state/resource expectations.
- [ ] **F-EV-007.04 — Implement/prove: compiled object**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: compiled object. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compiled object; retain observable state/resource expectations.
- [ ] **F-EV-007.05 — Implement/prove: linked image**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: linked image. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for linked image; retain observable state/resource expectations.
- [ ] **F-EV-007.06 — Implement/prove: package**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: package. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for package; retain observable state/resource expectations.
- [ ] **F-EV-007.07 — Implement/prove: registry**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: registry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for registry; retain observable state/resource expectations.
- [ ] **F-EV-007.08 — Implement/prove: init**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: init. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for init; retain observable state/resource expectations.
- [ ] **F-EV-007.09 — Implement/prove: launch-route inventories agree**
  - Action: For build graph truth, implement or reuse and verify this exact obligation: launch-route inventories agree. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-007.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for launch-route inventories agree; retain observable state/resource expectations.
- [ ] **F-EV-007.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to build graph truth: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-007.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-007.11 — Integrate into the real consumer and runtime route**
  - Action: Wire build graph truth into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-007.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-007.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for build graph truth as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-007.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-008"></a>
## F-EV-008 — strict exit propagation

**Original requirement:** no wrapper, timeout, ignored command, or final-test status can manufacture green

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-008.01 — Reconcile existing strict exit propagation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for strict exit propagation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: no wrapper, timeout, ignored command, or final-test status can manufacture green
- [ ] **F-EV-008.02 — Freeze the exact contract for strict exit propagation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: no wrapper, timeout, ignored command, or final-test status can manufacture green. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-008.03 — Implement/prove: no wrapper**
  - Action: For strict exit propagation, implement or reuse and verify this exact obligation: no wrapper. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no wrapper; retain observable state/resource expectations.
- [ ] **F-EV-008.04 — Implement/prove: timeout**
  - Action: For strict exit propagation, implement or reuse and verify this exact obligation: timeout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout; retain observable state/resource expectations.
- [ ] **F-EV-008.05 — Implement/prove: ignored command**
  - Action: For strict exit propagation, implement or reuse and verify this exact obligation: ignored command. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ignored command; retain observable state/resource expectations.
- [ ] **F-EV-008.06 — Implement/prove: or final-test status can manufacture green**
  - Action: For strict exit propagation, implement or reuse and verify this exact obligation: or final-test status can manufacture green. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for or final-test status can manufacture green; retain observable state/resource expectations.
- [ ] **F-EV-008.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to strict exit propagation: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-008.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-008.08 — Integrate into the real consumer and runtime route**
  - Action: Wire strict exit propagation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-008.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-008.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for strict exit propagation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-008.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-009"></a>
## F-EV-009 — artifact self-identity

**Original requirement:** booted/running product reports exact image, commit, configuration, and boot origin

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-009.01 — Reconcile existing artifact self-identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for artifact self-identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: booted/running product reports exact image, commit, configuration, and boot origin
- [ ] **F-EV-009.02 — Freeze the exact contract for artifact self-identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: booted/running product reports exact image, commit, configuration, and boot origin. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-009.03 — Implement/prove: booted/running product reports exact image**
  - Action: For artifact self-identity, implement or reuse and verify this exact obligation: booted/running product reports exact image. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for booted/running product reports exact image; retain observable state/resource expectations.
- [ ] **F-EV-009.04 — Implement/prove: commit**
  - Action: For artifact self-identity, implement or reuse and verify this exact obligation: commit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for commit; retain observable state/resource expectations.
- [ ] **F-EV-009.05 — Implement/prove: configuration**
  - Action: For artifact self-identity, implement or reuse and verify this exact obligation: configuration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configuration; retain observable state/resource expectations.
- [ ] **F-EV-009.06 — Implement/prove: boot origin**
  - Action: For artifact self-identity, implement or reuse and verify this exact obligation: boot origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for boot origin; retain observable state/resource expectations.
- [ ] **F-EV-009.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to artifact self-identity: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-009.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-009.08 — Integrate into the real consumer and runtime route**
  - Action: Wire artifact self-identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-009.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-009.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for artifact self-identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-009.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-010"></a>
## F-EV-010 — evidence ladder

**Original requirement:** claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-010.01 — Reconcile existing evidence ladder**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for evidence ladder. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct
- [ ] **F-EV-010.02 — Freeze the exact contract for evidence ladder**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-010.03 — Implement/prove: claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct**
  - Action: For evidence ladder, implement or reuse and verify this exact obligation: claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for claim/source/build/image/init/user-route/test/QEMU/host-hardware/native-hardware remain distinct; retain observable state/resource expectations.
- [ ] **F-EV-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to evidence ladder: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire evidence ladder into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for evidence ladder as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-011"></a>
## F-EV-011 — feature maturity ledger

**Original requirement:** each feature exposes current evidence, missing gates, known defects, and rejected claims

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-011.01 — Reconcile existing feature maturity ledger**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for feature maturity ledger. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: each feature exposes current evidence, missing gates, known defects, and rejected claims
- [ ] **F-EV-011.02 — Freeze the exact contract for feature maturity ledger**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: each feature exposes current evidence, missing gates, known defects, and rejected claims. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-011.03 — Implement/prove: each feature exposes current evidence**
  - Action: For feature maturity ledger, implement or reuse and verify this exact obligation: each feature exposes current evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for each feature exposes current evidence; retain observable state/resource expectations.
- [ ] **F-EV-011.04 — Implement/prove: missing gates**
  - Action: For feature maturity ledger, implement or reuse and verify this exact obligation: missing gates. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for missing gates; retain observable state/resource expectations.
- [ ] **F-EV-011.05 — Implement/prove: known defects**
  - Action: For feature maturity ledger, implement or reuse and verify this exact obligation: known defects. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for known defects; retain observable state/resource expectations.
- [ ] **F-EV-011.06 — Implement/prove: rejected claims**
  - Action: For feature maturity ledger, implement or reuse and verify this exact obligation: rejected claims. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rejected claims; retain observable state/resource expectations.
- [ ] **F-EV-011.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to feature maturity ledger: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-011.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-011.08 — Integrate into the real consumer and runtime route**
  - Action: Wire feature maturity ledger into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-011.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-011.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for feature maturity ledger as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-011.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-012"></a>
## F-EV-012 — test inventory parity

**Original requirement:** declared, compiled, registered, executed, skipped, failed, and shipped test sets compare automatically

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PROVED_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** No gap recorded by that bounded ledger contract; revalidate scope before reuse..

### Execution steps

- [ ] **F-EV-012.01 — Reconcile existing test inventory parity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for test inventory parity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PROVED_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: declared, compiled, registered, executed, skipped, failed, and shipped test sets compare automatically
- [ ] **F-EV-012.02 — Freeze the exact contract for test inventory parity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: declared, compiled, registered, executed, skipped, failed, and shipped test sets compare automatically. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-012.03 — Implement/prove: declared**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: declared. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared; retain observable state/resource expectations.
- [ ] **F-EV-012.04 — Implement/prove: compiled**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: compiled. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compiled; retain observable state/resource expectations.
- [ ] **F-EV-012.05 — Implement/prove: registered**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: registered. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for registered; retain observable state/resource expectations.
- [ ] **F-EV-012.06 — Implement/prove: executed**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: executed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for executed; retain observable state/resource expectations.
- [ ] **F-EV-012.07 — Implement/prove: skipped**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: skipped. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for skipped; retain observable state/resource expectations.
- [ ] **F-EV-012.08 — Implement/prove: failed**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: failed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failed; retain observable state/resource expectations.
- [ ] **F-EV-012.09 — Implement/prove: shipped test sets compare automatically**
  - Action: For test inventory parity, implement or reuse and verify this exact obligation: shipped test sets compare automatically. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-012.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shipped test sets compare automatically; retain observable state/resource expectations.
- [ ] **F-EV-012.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to test inventory parity: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-012.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-012.11 — Integrate into the real consumer and runtime route**
  - Action: Wire test inventory parity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-012.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-012.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for test inventory parity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-012.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-013"></a>
## F-EV-013 — canary/mutation gate

**Original requirement:** planted failure in every required field proves the verifier can fail

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** one mutation per every future required field is not yet possible.

### Execution steps

- [ ] **F-EV-013.01 — Reconcile existing canary/mutation gate**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for canary/mutation gate. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: planted failure in every required field proves the verifier can fail
- [ ] **F-EV-013.02 — Freeze the exact contract for canary/mutation gate**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: planted failure in every required field proves the verifier can fail. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-013.03 — Implement/prove: planted failure in every required field proves the verifier can fail**
  - Action: For canary/mutation gate, implement or reuse and verify this exact obligation: planted failure in every required field proves the verifier can fail. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for planted failure in every required field proves the verifier can fail; retain observable state/resource expectations.
- [ ] **F-EV-013.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to canary/mutation gate: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-013.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-013.05 — Integrate into the real consumer and runtime route**
  - Action: Wire canary/mutation gate into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-013.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-013.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for canary/mutation gate as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-013.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-014"></a>
## F-EV-014 — benchmark receipt

**Original requirement:** workload, clock, samples, percentile, peak, machine, backend, artifact, and regression budget recorded

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 native-target measurements; host timings were collected on one contended desktop; no QEMU or physical performance distribution.

### Execution steps

- [ ] **F-EV-014.01 — Reconcile existing benchmark receipt**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for benchmark receipt. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: workload, clock, samples, percentile, peak, machine, backend, artifact, and regression budget recorded
- [ ] **F-EV-014.02 — Freeze the exact contract for benchmark receipt**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: workload, clock, samples, percentile, peak, machine, backend, artifact, and regression budget recorded. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-014.03 — Implement/prove: workload**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: workload. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for workload; retain observable state/resource expectations.
- [ ] **F-EV-014.04 — Implement/prove: clock**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: clock. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clock; retain observable state/resource expectations.
- [ ] **F-EV-014.05 — Implement/prove: samples**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: samples. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for samples; retain observable state/resource expectations.
- [ ] **F-EV-014.06 — Implement/prove: percentile**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: percentile. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for percentile; retain observable state/resource expectations.
- [ ] **F-EV-014.07 — Implement/prove: peak**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: peak. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for peak; retain observable state/resource expectations.
- [ ] **F-EV-014.08 — Implement/prove: machine**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: machine. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for machine; retain observable state/resource expectations.
- [ ] **F-EV-014.09 — Implement/prove: backend**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: backend. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backend; retain observable state/resource expectations.
- [ ] **F-EV-014.10 — Implement/prove: artifact**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: artifact. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for artifact; retain observable state/resource expectations.
- [ ] **F-EV-014.11 — Implement/prove: regression budget recorded**
  - Action: For benchmark receipt, implement or reuse and verify this exact obligation: regression budget recorded. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-014.10.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for regression budget recorded; retain observable state/resource expectations.
- [ ] **F-EV-014.12 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to benchmark receipt: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-014.11.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-014.13 — Integrate into the real consumer and runtime route**
  - Action: Wire benchmark receipt into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-014.12.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-014.14 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for benchmark receipt as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-014.13.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-015"></a>
## F-EV-015 — hardware receipt

**Original requirement:** exact device/firmware/topology, first use, reuse, reset, teardown, recovery, and raw evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 of 6 physical route cases have validated receipts; 0 exact executable or boot-medium artifact hashes have native physical proof.

### Execution steps

- [ ] **F-EV-015.01 — Reconcile existing hardware receipt**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hardware receipt. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact device/firmware/topology, first use, reuse, reset, teardown, recovery, and raw evidence
- [ ] **F-EV-015.02 — Freeze the exact contract for hardware receipt**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact device/firmware/topology, first use, reuse, reset, teardown, recovery, and raw evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-015.03 — Implement/prove: exact device/firmware/topology**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: exact device/firmware/topology. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact device/firmware/topology; retain observable state/resource expectations.
- [ ] **F-EV-015.04 — Implement/prove: first use**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: first use. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for first use; retain observable state/resource expectations.
- [ ] **F-EV-015.05 — Implement/prove: reuse**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: reuse. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reuse; retain observable state/resource expectations.
- [ ] **F-EV-015.06 — Implement/prove: reset**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: reset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reset; retain observable state/resource expectations.
- [ ] **F-EV-015.07 — Implement/prove: teardown**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: teardown. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for teardown; retain observable state/resource expectations.
- [ ] **F-EV-015.08 — Implement/prove: recovery**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery; retain observable state/resource expectations.
- [ ] **F-EV-015.09 — Implement/prove: raw evidence**
  - Action: For hardware receipt, implement or reuse and verify this exact obligation: raw evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-015.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for raw evidence; retain observable state/resource expectations.
- [ ] **F-EV-015.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hardware receipt: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-015.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-015.11 — Integrate into the real consumer and runtime route**
  - Action: Wire hardware receipt into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-015.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-015.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hardware receipt as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-015.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-016"></a>
## F-EV-016 — screenshot/video receipt

**Original requirement:** asset names artifact, mode, scale, theme, locale, scenario, time, and evidence ceiling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 41 historical visual assets remain unbound; scale/theme/locale/accessibility/ui-state/backend matrices remain incomplete; no current video or physical-display receipt.

### Execution steps

- [ ] **F-EV-016.01 — Reconcile existing screenshot/video receipt**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screenshot/video receipt. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: asset names artifact, mode, scale, theme, locale, scenario, time, and evidence ceiling
- [ ] **F-EV-016.02 — Freeze the exact contract for screenshot/video receipt**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: asset names artifact, mode, scale, theme, locale, scenario, time, and evidence ceiling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-016.03 — Implement/prove: asset names artifact**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: asset names artifact. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for asset names artifact; retain observable state/resource expectations.
- [ ] **F-EV-016.04 — Implement/prove: mode**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: mode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mode; retain observable state/resource expectations.
- [ ] **F-EV-016.05 — Implement/prove: scale**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: scale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scale; retain observable state/resource expectations.
- [ ] **F-EV-016.06 — Implement/prove: theme**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: theme. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for theme; retain observable state/resource expectations.
- [ ] **F-EV-016.07 — Implement/prove: locale**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: locale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale; retain observable state/resource expectations.
- [ ] **F-EV-016.08 — Implement/prove: scenario**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: scenario. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scenario; retain observable state/resource expectations.
- [ ] **F-EV-016.09 — Implement/prove: time**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time; retain observable state/resource expectations.
- [ ] **F-EV-016.10 — Implement/prove: evidence ceiling**
  - Action: For screenshot/video receipt, implement or reuse and verify this exact obligation: evidence ceiling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-016.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for evidence ceiling; retain observable state/resource expectations.
- [ ] **F-EV-016.11 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screenshot/video receipt: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-016.10.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-016.12 — Integrate into the real consumer and runtime route**
  - Action: Wire screenshot/video receipt into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-016.11.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-016.13 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screenshot/video receipt as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-016.12.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-017"></a>
## F-EV-017 — crash/panic receipt

**Original requirement:** independent minimal recorder preserves symbols, input, state, and artifact identity

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 durable crash receipts; broader exception-vector and spurious-interrupt coverage remains absent; no emergency-stack guard page, stack symbols, persistence or recovery service.

### Execution steps

- [ ] **F-EV-017.01 — Reconcile existing crash/panic receipt**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for crash/panic receipt. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: independent minimal recorder preserves symbols, input, state, and artifact identity
- [ ] **F-EV-017.02 — Freeze the exact contract for crash/panic receipt**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: independent minimal recorder preserves symbols, input, state, and artifact identity. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-017.03 — Implement/prove: independent minimal recorder preserves symbols**
  - Action: For crash/panic receipt, implement or reuse and verify this exact obligation: independent minimal recorder preserves symbols. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for independent minimal recorder preserves symbols; retain observable state/resource expectations.
- [ ] **F-EV-017.04 — Implement/prove: input**
  - Action: For crash/panic receipt, implement or reuse and verify this exact obligation: input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input; retain observable state/resource expectations.
- [ ] **F-EV-017.05 — Implement/prove: state**
  - Action: For crash/panic receipt, implement or reuse and verify this exact obligation: state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for state; retain observable state/resource expectations.
- [ ] **F-EV-017.06 — Implement/prove: artifact identity**
  - Action: For crash/panic receipt, implement or reuse and verify this exact obligation: artifact identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for artifact identity; retain observable state/resource expectations.
- [ ] **F-EV-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to crash/panic receipt: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire crash/panic receipt into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for crash/panic receipt as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-018"></a>
## F-EV-018 — structured event trace

**Original requirement:** typed monotonic events carry feature, process, authority, correlation, drop, and redaction fields

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 target emitters; single-owner core only; no QEMU/native route; no durable tamper-evident anchor or service export policy.

### Execution steps

- [ ] **F-EV-018.01 — Reconcile existing structured event trace**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for structured event trace. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed monotonic events carry feature, process, authority, correlation, drop, and redaction fields
- [ ] **F-EV-018.02 — Freeze the exact contract for structured event trace**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed monotonic events carry feature, process, authority, correlation, drop, and redaction fields. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-018.03 — Implement/prove: typed monotonic events carry feature**
  - Action: For structured event trace, implement or reuse and verify this exact obligation: typed monotonic events carry feature. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed monotonic events carry feature; retain observable state/resource expectations.
- [ ] **F-EV-018.04 — Implement/prove: process**
  - Action: For structured event trace, implement or reuse and verify this exact obligation: process. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for process; retain observable state/resource expectations.
- [ ] **F-EV-018.05 — Implement/prove: authority**
  - Action: For structured event trace, implement or reuse and verify this exact obligation: authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authority; retain observable state/resource expectations.
- [ ] **F-EV-018.06 — Implement/prove: correlation**
  - Action: For structured event trace, implement or reuse and verify this exact obligation: correlation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for correlation; retain observable state/resource expectations.
- [ ] **F-EV-018.07 — Implement/prove: drop**
  - Action: For structured event trace, implement or reuse and verify this exact obligation: drop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-018.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drop; retain observable state/resource expectations.
- [ ] **F-EV-018.08 — Implement/prove: redaction fields**
  - Action: For structured event trace, implement or reuse and verify this exact obligation: redaction fields. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-018.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for redaction fields; retain observable state/resource expectations.
- [ ] **F-EV-018.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to structured event trace: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-018.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-018.10 — Integrate into the real consumer and runtime route**
  - Action: Wire structured event trace into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-018.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-018.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for structured event trace as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-018.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-019"></a>
## F-EV-019 — failure-injection registry

**Original requirement:** every allocation, queue, I/O, provider, service, package, and lifecycle seam has forced failures

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** all 7 failure families remain non-exhaustive; provider, service and package injection are absent; queue, I/O and lifecycle seams remain bounded subsets; no physical-hardware failure-injection receipt exists.

### Execution steps

- [ ] **F-EV-019.01 — Reconcile existing failure-injection registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for failure-injection registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: every allocation, queue, I/O, provider, service, package, and lifecycle seam has forced failures
- [ ] **F-EV-019.02 — Freeze the exact contract for failure-injection registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: every allocation, queue, I/O, provider, service, package, and lifecycle seam has forced failures. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-019.03 — Implement/prove: every allocation**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: every allocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for every allocation; retain observable state/resource expectations.
- [ ] **F-EV-019.04 — Implement/prove: queue**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: queue. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for queue; retain observable state/resource expectations.
- [ ] **F-EV-019.05 — Implement/prove: I/O**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: I/O. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for I/O; retain observable state/resource expectations.
- [ ] **F-EV-019.06 — Implement/prove: provider**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: provider. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider; retain observable state/resource expectations.
- [ ] **F-EV-019.07 — Implement/prove: service**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: service. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for service; retain observable state/resource expectations.
- [ ] **F-EV-019.08 — Implement/prove: package**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: package. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for package; retain observable state/resource expectations.
- [ ] **F-EV-019.09 — Implement/prove: lifecycle seam has forced failures**
  - Action: For failure-injection registry, implement or reuse and verify this exact obligation: lifecycle seam has forced failures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-019.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifecycle seam has forced failures; retain observable state/resource expectations.
- [ ] **F-EV-019.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to failure-injection registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-019.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-019.11 — Integrate into the real consumer and runtime route**
  - Action: Wire failure-injection registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-019.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-019.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for failure-injection registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-019.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-020"></a>
## F-EV-020 — hostile corpus registry

**Original requirement:** malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no font parser/corpus exists; no typed IPC envelope parser/corpus exists.

### Execution steps

- [ ] **F-EV-020.01 — Reconcile existing hostile corpus registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hostile corpus registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized
- [ ] **F-EV-020.02 — Freeze the exact contract for hostile corpus registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-020.03 — Implement/prove: malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized**
  - Action: For hostile corpus registry, implement or reuse and verify this exact obligation: malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for malformed ELF/archive/fs/network/font/image/web/IPC inputs are durable and minimized; retain observable state/resource expectations.
- [ ] **F-EV-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hostile corpus registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire hostile corpus registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hostile corpus registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-021"></a>
## F-EV-021 — performance regression registry

**Original requirement:** frame, input, I/O, network, launch, memory, and build budgets are machine checked

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** any current host category outside its declared guard remains an open regression; the product artifact build has no current timed distribution; input/I/O/network/launch/memory values are host-gate runtimes, not product latency; 0 native-target and 0 physical categories.

### Execution steps

- [ ] **F-EV-021.01 — Reconcile existing performance regression registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for performance regression registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: frame, input, I/O, network, launch, memory, and build budgets are machine checked
- [ ] **F-EV-021.02 — Freeze the exact contract for performance regression registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: frame, input, I/O, network, launch, memory, and build budgets are machine checked. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-021.03 — Implement/prove: frame**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: frame. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frame; retain observable state/resource expectations.
- [ ] **F-EV-021.04 — Implement/prove: input**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input; retain observable state/resource expectations.
- [ ] **F-EV-021.05 — Implement/prove: I/O**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: I/O. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for I/O; retain observable state/resource expectations.
- [ ] **F-EV-021.06 — Implement/prove: network**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: network. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for network; retain observable state/resource expectations.
- [ ] **F-EV-021.07 — Implement/prove: launch**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: launch. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for launch; retain observable state/resource expectations.
- [ ] **F-EV-021.08 — Implement/prove: memory**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: memory. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for memory; retain observable state/resource expectations.
- [ ] **F-EV-021.09 — Implement/prove: build budgets are machine checked**
  - Action: For performance regression registry, implement or reuse and verify this exact obligation: build budgets are machine checked. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-021.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for build budgets are machine checked; retain observable state/resource expectations.
- [ ] **F-EV-021.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to performance regression registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-021.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-021.11 — Integrate into the real consumer and runtime route**
  - Action: Wire performance regression registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-021.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-021.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for performance regression registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-021.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-022"></a>
## F-EV-022 — visual regression registry

**Original requirement:** goldens cover scale, theme, locale, a11y, empty/error/loading, and backend variants

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** goldens cover only 2 routes and 2 states; scale/theme/locale/accessibility/ui-state/backend matrix incomplete; no physical display comparison.

### Execution steps

- [ ] **F-EV-022.01 — Reconcile existing visual regression registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for visual regression registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: goldens cover scale, theme, locale, a11y, empty/error/loading, and backend variants
- [ ] **F-EV-022.02 — Freeze the exact contract for visual regression registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: goldens cover scale, theme, locale, a11y, empty/error/loading, and backend variants. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-022.03 — Implement/prove: goldens cover scale**
  - Action: For visual regression registry, implement or reuse and verify this exact obligation: goldens cover scale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for goldens cover scale; retain observable state/resource expectations.
- [ ] **F-EV-022.04 — Implement/prove: theme**
  - Action: For visual regression registry, implement or reuse and verify this exact obligation: theme. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for theme; retain observable state/resource expectations.
- [ ] **F-EV-022.05 — Implement/prove: locale**
  - Action: For visual regression registry, implement or reuse and verify this exact obligation: locale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale; retain observable state/resource expectations.
- [ ] **F-EV-022.06 — Implement/prove: a11y**
  - Action: For visual regression registry, implement or reuse and verify this exact obligation: a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for a11y; retain observable state/resource expectations.
- [ ] **F-EV-022.07 — Implement/prove: empty/error/loading**
  - Action: For visual regression registry, implement or reuse and verify this exact obligation: empty/error/loading. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-022.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for empty/error/loading; retain observable state/resource expectations.
- [ ] **F-EV-022.08 — Implement/prove: backend variants**
  - Action: For visual regression registry, implement or reuse and verify this exact obligation: backend variants. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-022.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backend variants; retain observable state/resource expectations.
- [ ] **F-EV-022.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to visual regression registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-022.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-022.10 — Integrate into the real consumer and runtime route**
  - Action: Wire visual regression registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-022.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-022.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for visual regression registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-022.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-023"></a>
## F-EV-023 — accessibility proof registry

**Original requirement:** keyboard and assistive-provider workflows are required for each component/app route

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 9 capabilities missing; 0 complete target workflows.

### Execution steps

- [ ] **F-EV-023.01 — Reconcile existing accessibility proof registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for accessibility proof registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: keyboard and assistive-provider workflows are required for each component/app route
- [ ] **F-EV-023.02 — Freeze the exact contract for accessibility proof registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: keyboard and assistive-provider workflows are required for each component/app route. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-023.03 — Implement/prove: keyboard and assistive-provider workflows are required for each component/app route**
  - Action: For accessibility proof registry, implement or reuse and verify this exact obligation: keyboard and assistive-provider workflows are required for each component/app route. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard and assistive-provider workflows are required for each component/app route; retain observable state/resource expectations.
- [ ] **F-EV-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to accessibility proof registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire accessibility proof registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for accessibility proof registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-024"></a>
## F-EV-024 — security claim registry

**Original requirement:** threat model, protected asset, enforcement point, adversarial proof, and residual risk are linked

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 17 residual-risk claims open; 0 production-complete security claims.

### Execution steps

- [ ] **F-EV-024.01 — Reconcile existing security claim registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for security claim registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: threat model, protected asset, enforcement point, adversarial proof, and residual risk are linked
- [ ] **F-EV-024.02 — Freeze the exact contract for security claim registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: threat model, protected asset, enforcement point, adversarial proof, and residual risk are linked. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-024.03 — Implement/prove: threat model**
  - Action: For security claim registry, implement or reuse and verify this exact obligation: threat model. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for threat model; retain observable state/resource expectations.
- [ ] **F-EV-024.04 — Implement/prove: protected asset**
  - Action: For security claim registry, implement or reuse and verify this exact obligation: protected asset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for protected asset; retain observable state/resource expectations.
- [ ] **F-EV-024.05 — Implement/prove: enforcement point**
  - Action: For security claim registry, implement or reuse and verify this exact obligation: enforcement point. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for enforcement point; retain observable state/resource expectations.
- [ ] **F-EV-024.06 — Implement/prove: adversarial proof**
  - Action: For security claim registry, implement or reuse and verify this exact obligation: adversarial proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-024.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for adversarial proof; retain observable state/resource expectations.
- [ ] **F-EV-024.07 — Implement/prove: residual risk are linked**
  - Action: For security claim registry, implement or reuse and verify this exact obligation: residual risk are linked. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-024.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for residual risk are linked; retain observable state/resource expectations.
- [ ] **F-EV-024.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to security claim registry: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-024.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-024.09 — Integrate into the real consumer and runtime route**
  - Action: Wire security claim registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-024.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-024.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for security claim registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-024.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-025"></a>
## F-EV-025 — independent refutation

**Original requirement:** a reviewer attempts counterexamples and can reject, not silently repair, completion evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no independent reviewer receipt exists for the active implementation batch.

### Execution steps

- [ ] **F-EV-025.01 — Reconcile existing independent refutation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for independent refutation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: a reviewer attempts counterexamples and can reject, not silently repair, completion evidence
- [ ] **F-EV-025.02 — Freeze the exact contract for independent refutation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: a reviewer attempts counterexamples and can reject, not silently repair, completion evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-025.03 — Implement/prove: a reviewer attempts counterexamples and can reject**
  - Action: For independent refutation, implement or reuse and verify this exact obligation: a reviewer attempts counterexamples and can reject. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for a reviewer attempts counterexamples and can reject; retain observable state/resource expectations.
- [ ] **F-EV-025.04 — Implement/prove: not silently repair**
  - Action: For independent refutation, implement or reuse and verify this exact obligation: not silently repair. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not silently repair; retain observable state/resource expectations.
- [ ] **F-EV-025.05 — Implement/prove: completion evidence**
  - Action: For independent refutation, implement or reuse and verify this exact obligation: completion evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for completion evidence; retain observable state/resource expectations.
- [ ] **F-EV-025.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to independent refutation: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-025.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-025.07 — Integrate into the real consumer and runtime route**
  - Action: Wire independent refutation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-025.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-025.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for independent refutation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-025.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-026"></a>
## F-EV-026 — historical decision log

**Original requirement:** decisions, migrations, deprecations, reversals, and obsolete evidence stay attributable

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** system-wide decision inventory is incomplete; no signed or independent approval; no history viewer.

### Execution steps

- [ ] **F-EV-026.01 — Reconcile existing historical decision log**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for historical decision log. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: decisions, migrations, deprecations, reversals, and obsolete evidence stay attributable
- [ ] **F-EV-026.02 — Freeze the exact contract for historical decision log**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: decisions, migrations, deprecations, reversals, and obsolete evidence stay attributable. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-026.03 — Implement/prove: decisions**
  - Action: For historical decision log, implement or reuse and verify this exact obligation: decisions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for decisions; retain observable state/resource expectations.
- [ ] **F-EV-026.04 — Implement/prove: migrations**
  - Action: For historical decision log, implement or reuse and verify this exact obligation: migrations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migrations; retain observable state/resource expectations.
- [ ] **F-EV-026.05 — Implement/prove: deprecations**
  - Action: For historical decision log, implement or reuse and verify this exact obligation: deprecations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deprecations; retain observable state/resource expectations.
- [ ] **F-EV-026.06 — Implement/prove: reversals**
  - Action: For historical decision log, implement or reuse and verify this exact obligation: reversals. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-026.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reversals; retain observable state/resource expectations.
- [ ] **F-EV-026.07 — Implement/prove: obsolete evidence stay attributable**
  - Action: For historical decision log, implement or reuse and verify this exact obligation: obsolete evidence stay attributable. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-026.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for obsolete evidence stay attributable; retain observable state/resource expectations.
- [ ] **F-EV-026.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to historical decision log: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-026.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-026.09 — Integrate into the real consumer and runtime route**
  - Action: Wire historical decision log into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-026.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-026.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for historical decision log as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-026.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-027"></a>
## F-EV-027 — release notes/changelog

**Original requirement:** user-visible changes, compatibility, migrations, defects, and recovery steps derive from manifests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** 0 published entries; no version/channel/signature; public redistribution blocked; migration inventory incomplete; no previous signed rollback generation.

### Execution steps

- [ ] **F-EV-027.01 — Reconcile existing release notes/changelog**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for release notes/changelog. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user-visible changes, compatibility, migrations, defects, and recovery steps derive from manifests
- [ ] **F-EV-027.02 — Freeze the exact contract for release notes/changelog**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user-visible changes, compatibility, migrations, defects, and recovery steps derive from manifests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-027.03 — Implement/prove: user-visible changes**
  - Action: For release notes/changelog, implement or reuse and verify this exact obligation: user-visible changes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user-visible changes; retain observable state/resource expectations.
- [ ] **F-EV-027.04 — Implement/prove: compatibility**
  - Action: For release notes/changelog, implement or reuse and verify this exact obligation: compatibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compatibility; retain observable state/resource expectations.
- [ ] **F-EV-027.05 — Implement/prove: migrations**
  - Action: For release notes/changelog, implement or reuse and verify this exact obligation: migrations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migrations; retain observable state/resource expectations.
- [ ] **F-EV-027.06 — Implement/prove: defects**
  - Action: For release notes/changelog, implement or reuse and verify this exact obligation: defects. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-027.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defects; retain observable state/resource expectations.
- [ ] **F-EV-027.07 — Implement/prove: recovery steps derive from manifests**
  - Action: For release notes/changelog, implement or reuse and verify this exact obligation: recovery steps derive from manifests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-027.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery steps derive from manifests; retain observable state/resource expectations.
- [ ] **F-EV-027.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to release notes/changelog: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-027.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-027.09 — Integrate into the real consumer and runtime route**
  - Action: Wire release notes/changelog into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-027.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-027.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for release notes/changelog as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-027.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ev-028"></a>
## F-EV-028 — provenance viewer

**Original requirement:** users can inspect version, origin, licenses, signatures, permissions, tests, and current health

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no booted zlOS app route; 0 signatures; 0 per-app permission grants; no live health stream; no current screenshot or target accessibility receipt.

### Execution steps

- [ ] **F-EV-028.01 — Reconcile existing provenance viewer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for provenance viewer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-and-evidence comparison against the complete requirement: users can inspect version, origin, licenses, signatures, permissions, tests, and current health
- [ ] **F-EV-028.02 — Freeze the exact contract for provenance viewer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: users can inspect version, origin, licenses, signatures, permissions, tests, and current health. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-EV-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-EV-028.03 — Implement/prove: users can inspect version**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: users can inspect version. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for users can inspect version; retain observable state/resource expectations.
- [ ] **F-EV-028.04 — Implement/prove: origin**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin; retain observable state/resource expectations.
- [ ] **F-EV-028.05 — Implement/prove: licenses**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: licenses. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for licenses; retain observable state/resource expectations.
- [ ] **F-EV-028.06 — Implement/prove: signatures**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: signatures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signatures; retain observable state/resource expectations.
- [ ] **F-EV-028.07 — Implement/prove: permissions**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-EV-028.08 — Implement/prove: tests**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tests; retain observable state/resource expectations.
- [ ] **F-EV-028.09 — Implement/prove: current health**
  - Action: For provenance viewer, implement or reuse and verify this exact obligation: current health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-EV-028.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for current health; retain observable state/resource expectations.
- [ ] **F-EV-028.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to provenance viewer: missing/duplicate identity; stale source or artifact; skipped inner command; ignored exit status; an instrument counted as a gate; reproducibility drift. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-EV-028.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-EV-028.11 — Integrate into the real consumer and runtime route**
  - Action: Wire provenance viewer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-EV-028.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-EV-028.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for provenance viewer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-EV-028.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p0-1"></a>
## C-P0.1 — machine-readable feature ledger

**Original requirement:** machine-readable feature ledger

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 55.

### Preserved original contract

- **Dependencies/current/provenance:** extend the existing documentation/evidence culture and ZLLOG artifact identity; use RustOS receipts and banan-os maturity lists; reject README/file-count completion.
- **I/O and state:** input is one feature record plus source/build/test/hardware references; output is validated JSON or equivalent and generated Markdown; state follows the feature machine above.
- **Invariants/failure:** immutable feature ID; monotonic evidence; exact commit/artifact; one owner; impossible transitions reject without modifying the ledger.
- **Deterministic proof:** schema property tests, missing-field corpus, illegal-state transitions, duplicate IDs, stale artifact hash, generated-doc reproducibility.
- **Target proof:** QEMU and ThinkPad receipts ingest without manual rewriting and preserve their distinct profiles.
- **Receipt/removal:** retain schema version and generated diff; rollback is previous ledger generation; remove hand-maintained feature status tables only when every row is generated.

### Execution steps

- [ ] **C-P0.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P0.1.02 — Resolve this contract's exact dependencies**
  - Action: extend the existing documentation/evidence culture and ZLLOG artifact identity; use RustOS receipts and banan-os maturity lists; reject README/file-count completion. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P0.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P0.1.03 — I/O and state — machine-readable feature ledger**
  - Action: input is one feature record plus source/build/test/hardware references; output is validated JSON or equivalent and generated Markdown; state follows the feature machine above.
  - Requires: C-P0.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P0.1.
- [ ] **C-P0.1.04 — Invariants/failure — machine-readable feature ledger**
  - Action: immutable feature ID; monotonic evidence; exact commit/artifact; one owner; impossible transitions reject without modifying the ledger.
  - Requires: C-P0.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P0.1.
- [ ] **C-P0.1.05 — Deterministic proof — machine-readable feature ledger**
  - Action: schema property tests, missing-field corpus, illegal-state transitions, duplicate IDs, stale artifact hash, generated-doc reproducibility.
  - Requires: C-P0.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P0.1.
- [ ] **C-P0.1.06 — Target proof — machine-readable feature ledger**
  - Action: QEMU and ThinkPad receipts ingest without manual rewriting and preserve their distinct profiles.
  - Requires: C-P0.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P0.1.
- [ ] **C-P0.1.07 — Receipt/removal — machine-readable feature ledger**
  - Action: retain schema version and generated diff; rollback is previous ledger generation; remove hand-maintained feature status tables only when every row is generated.
  - Requires: C-P0.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P0.1.
- [ ] **C-P0.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P0.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P0.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p0-2"></a>
## C-P0.2 — shipped-source and module inventory

**Original requirement:** shipped-source and module inventory

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 64.

### Preserved original contract

- **Dependencies/current/provenance:** current `SOURCES` and source verification; Zinnia narrow modules and Brook build-boundary tests; reject source-present equals shipped.
- **I/O and state:** build graph and linked image in; source/object/module/symbol manifest out; state is `declared -> compiled -> linked -> loaded`.
- **Invariants/failure:** every linked object maps to declared source/license; every required declaration is linked; host-only files require explicit allowlist; unknown/missing entries fail build.
- **Deterministic proof:** add/remove/rename fixture, duplicate object, host-only misclassification, dead module, reverse-symbol reconciliation.
- **Target proof:** guest prints manifest digest; loader reports exactly the same required modules.
- **Receipt/removal:** manifest plus image digest; rollback is last known build graph; retire ad hoc reverse greps only after equivalent generated checks gate every build.

### Execution steps

- [ ] **C-P0.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P0.2.02 — Resolve this contract's exact dependencies**
  - Action: current `SOURCES` and source verification; Zinnia narrow modules and Brook build-boundary tests; reject source-present equals shipped. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P0.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P0.2.03 — I/O and state — shipped-source and module inventory**
  - Action: build graph and linked image in; source/object/module/symbol manifest out; state is `declared -> compiled -> linked -> loaded`.
  - Requires: C-P0.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P0.2.
- [ ] **C-P0.2.04 — Invariants/failure — shipped-source and module inventory**
  - Action: every linked object maps to declared source/license; every required declaration is linked; host-only files require explicit allowlist; unknown/missing entries fail build.
  - Requires: C-P0.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P0.2.
- [ ] **C-P0.2.05 — Deterministic proof — shipped-source and module inventory**
  - Action: add/remove/rename fixture, duplicate object, host-only misclassification, dead module, reverse-symbol reconciliation.
  - Requires: C-P0.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P0.2.
- [ ] **C-P0.2.06 — Target proof — shipped-source and module inventory**
  - Action: guest prints manifest digest; loader reports exactly the same required modules.
  - Requires: C-P0.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P0.2.
- [ ] **C-P0.2.07 — Receipt/removal — shipped-source and module inventory**
  - Action: manifest plus image digest; rollback is last known build graph; retire ad hoc reverse greps only after equivalent generated checks gate every build.
  - Requires: C-P0.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P0.2.
- [ ] **C-P0.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P0.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P0.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p0-3"></a>
## C-P0.3 — current-asset receipt backfill

**Original requirement:** current-asset receipt backfill

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 73.

### Preserved original contract

- **Dependencies/current/provenance:** existing ZLLOG, zlfs, framebuffer, WM, browser/net, Intel, xHCI, NVMe, host and QEMU gates; RustOS evidence discipline; reject inherited numbers.
- **I/O and state:** existing commands/logs/artifacts in; one normalized receipt per asset out; state remains at the strongest actually observed level.
- **Invariants/failure:** no new runtime claim; stale/unidentified artifacts remain unverified; conflicting docs are recorded, not averaged.
- **Deterministic proof:** receipt parser, missing identity, mismatched hash, contradictory result, stale date/commit fixtures.
- **Target proof:** current physical ZLLOG and current QEMU artifact are independently ingestible.
- **Receipt/removal:** evidence index links raw artifacts; rollback is deletion of derived index only; no raw evidence is removed.

### Execution steps

- [ ] **C-P0.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P0.3.02 — Resolve this contract's exact dependencies**
  - Action: existing ZLLOG, zlfs, framebuffer, WM, browser/net, Intel, xHCI, NVMe, host and QEMU gates; RustOS evidence discipline; reject inherited numbers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P0.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P0.3.03 — I/O and state — current-asset receipt backfill**
  - Action: existing commands/logs/artifacts in; one normalized receipt per asset out; state remains at the strongest actually observed level.
  - Requires: C-P0.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P0.3.
- [ ] **C-P0.3.04 — Invariants/failure — current-asset receipt backfill**
  - Action: no new runtime claim; stale/unidentified artifacts remain unverified; conflicting docs are recorded, not averaged.
  - Requires: C-P0.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P0.3.
- [ ] **C-P0.3.05 — Deterministic proof — current-asset receipt backfill**
  - Action: receipt parser, missing identity, mismatched hash, contradictory result, stale date/commit fixtures.
  - Requires: C-P0.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P0.3.
- [ ] **C-P0.3.06 — Target proof — current-asset receipt backfill**
  - Action: current physical ZLLOG and current QEMU artifact are independently ingestible.
  - Requires: C-P0.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P0.3.
- [ ] **C-P0.3.07 — Receipt/removal — current-asset receipt backfill**
  - Action: evidence index links raw artifacts; rollback is deletion of derived index only; no raw evidence is removed.
  - Requires: C-P0.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P0.3.
- [ ] **C-P0.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P0.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P0.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p0-4"></a>
## C-P0.4 — strict outer gate and documentation authority

**Original requirement:** strict outer gate and documentation authority

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 82.

### Preserved original contract

- **Dependencies/current/provenance:** current build/test scripts and orientation docs; Hyper/RustOS strict matrices; reject Cyjon/vib-OS/Lemon/Mollen false-green patterns.
- **I/O and state:** ordered gate steps in; one terminal status and per-step receipt out; docs classified `authority`, `active contract`, `history`, or `superseded`.
- **Invariants/failure:** first failure cannot be erased; cleanup runs separately; timeout is failure unless explicitly expected; authority precedence is machine-readable.
- **Deterministic proof:** planted failure at first/middle/last step, timeout, missing executable, skipped test, stale document link.
- **Target proof:** intentionally failing guest assertion makes CI red; successful guest sends exact success sentinel.
- **Receipt/removal:** gate graph and canary log; rollback preserves old commands behind diagnostic-only entry; remove obsolete top-level gates after two consecutive matching runs.

### Execution steps

- [ ] **C-P0.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P0.4.02 — Resolve this contract's exact dependencies**
  - Action: current build/test scripts and orientation docs; Hyper/RustOS strict matrices; reject Cyjon/vib-OS/Lemon/Mollen false-green patterns. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P0.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P0.4.03 — I/O and state — strict outer gate and documentation authority**
  - Action: ordered gate steps in; one terminal status and per-step receipt out; docs classified `authority`, `active contract`, `history`, or `superseded`.
  - Requires: C-P0.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P0.4.
- [ ] **C-P0.4.04 — Invariants/failure — strict outer gate and documentation authority**
  - Action: first failure cannot be erased; cleanup runs separately; timeout is failure unless explicitly expected; authority precedence is machine-readable.
  - Requires: C-P0.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P0.4.
- [ ] **C-P0.4.05 — Deterministic proof — strict outer gate and documentation authority**
  - Action: planted failure at first/middle/last step, timeout, missing executable, skipped test, stale document link.
  - Requires: C-P0.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P0.4.
- [ ] **C-P0.4.06 — Target proof — strict outer gate and documentation authority**
  - Action: intentionally failing guest assertion makes CI red; successful guest sends exact success sentinel.
  - Requires: C-P0.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P0.4.
- [ ] **C-P0.4.07 — Receipt/removal — strict outer gate and documentation authority**
  - Action: gate graph and canary log; rollback preserves old commands behind diagnostic-only entry; remove obsolete top-level gates after two consecutive matching runs.
  - Requires: C-P0.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P0.4.
- [ ] **C-P0.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P0.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P0.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p5-1"></a>
## C-P5.1 — Logger and ZLLOG importer/exporter

**Original requirement:** Logger and ZLLOG importer/exporter

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 273.

### Preserved original contract

- **Dependencies/current/provenance:** P4 supervisor/IPC, P2 files and current raw recorder; snarkOS telemetry and ZLLOG design; reject logging content/secrets or storage from IRQ.
- **I/O and state:** structured bounded records/counters and raw journal slots in; filtered streams/files/exports out; `RAMOnly -> RawMounted -> ServiceReady -> Exported`.
- **Invariants/failure:** reserved fault lane; drops counted; privacy schema; bad file service cannot block raw capture; import idempotent by boot/sequence ID.
- **Deterministic proof:** overflow/drop priority, malformed slots, duplicate import, service crash, privacy fixtures, slow/bad disk.
- **Target proof:** QEMU/ThinkPad raw record survives Logger absence/restart and later exports exactly once.
- **Receipt/removal:** raw and exported hash/sequence relation; raw path remains permanent rollback and is never removed.

### Execution steps

- [ ] **C-P5.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.1.02 — Resolve this contract's exact dependencies**
  - Action: P4 supervisor/IPC, P2 files and current raw recorder; snarkOS telemetry and ZLLOG design; reject logging content/secrets or storage from IRQ. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.1.03 — I/O and state — Logger and ZLLOG importer/exporter**
  - Action: structured bounded records/counters and raw journal slots in; filtered streams/files/exports out; `RAMOnly -> RawMounted -> ServiceReady -> Exported`.
  - Requires: C-P5.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.1.
- [ ] **C-P5.1.04 — Invariants/failure — Logger and ZLLOG importer/exporter**
  - Action: reserved fault lane; drops counted; privacy schema; bad file service cannot block raw capture; import idempotent by boot/sequence ID.
  - Requires: C-P5.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.1.
- [ ] **C-P5.1.05 — Deterministic proof — Logger and ZLLOG importer/exporter**
  - Action: overflow/drop priority, malformed slots, duplicate import, service crash, privacy fixtures, slow/bad disk.
  - Requires: C-P5.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.1.
- [ ] **C-P5.1.06 — Target proof — Logger and ZLLOG importer/exporter**
  - Action: QEMU/ThinkPad raw record survives Logger absence/restart and later exports exactly once.
  - Requires: C-P5.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.1.
- [ ] **C-P5.1.07 — Receipt/removal — Logger and ZLLOG importer/exporter**
  - Action: raw and exported hash/sequence relation; raw path remains permanent rollback and is never removed.
  - Requires: C-P5.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.1.
- [ ] **C-P5.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-00"></a>
## C-DA-00 — generated application registry

**Original requirement:** generated application registry

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 37.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-00.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-00.02 — Resolve this contract's exact dependencies**
  - Action: current app constants, catalogue, shell/dock routes and app dispatch functions. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-00.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-00.03 — Deliver — generated application registry**
  - Action: generate one registry from stable ID, name, category, package, entrypoint, draw/output route, event route, tick/background route, icon, accessibility label and supported launch surfaces. Generate dispatch tables and catalogue data from it rather than maintaining parallel ranges.
  - Requires: C-DA-00.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-00.
- [ ] **C-DA-00.04 — Invariants — generated application registry**
  - Action: IDs and names are unique; every registered item has an entrypoint; every named entry is reachable from at least one user route; every route resolves to exactly one entry; retired IDs remain reserved; no blank fallback tile counts as an app.
  - Requires: C-DA-00.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-00.
- [ ] **C-DA-00.05 — Proof — generated application registry**
  - Action: reproduce the current ID-14/Maze contradiction as a failing test; test duplicates, gaps, retired IDs, missing constructor/draw/event/tick metadata, unknown route and deliberate refusal. Boot the exact image and launch every item, requiring a visible ready nonce and clean close receipt.
  - Requires: C-DA-00.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-00.
- [ ] **C-DA-00.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-00. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-00.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-01"></a>
## C-DA-01 — feature/evidence ledger

**Original requirement:** feature/evidence ledger

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 57.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-01.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-01.02 — Resolve this contract's exact dependencies**
  - Action: build manifest and artifact nonce. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-01.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-01.03 — Deliver — feature/evidence ledger**
  - Action: machine-readable records for claim, source, build edge, shipped artifact, boot/init caller, launch route, positive scenario, failure scenario, QEMU profile, hardware profile and last verified revision.  Each record includes `implementation_origin = local | submodule | fetched_locked | system_dependency | bundled_content`, immutable source revision, build target, image target, default reachability, and separate code/data/firmware licenses.
  - Requires: C-DA-01.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-01.
- [ ] **C-DA-01.04 — Invariants — feature/evidence ledger**
  - Action: stronger states require all weaker prerequisites; one architecture or device does not promote another; checked-in artifacts never count as current; `external`, `stub`, `simulated` and `unreachable` remain first-class states.
  - Requires: C-DA-01.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-01.
- [ ] **C-DA-01.05 — Proof — feature/evidence ledger**
  - Action: reject missing source, stale image, absent init caller, simulated output, ignored exit, timeout, architecture mismatch and hardware receipt from a different artifact.
  - Requires: C-DA-01.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-01.
- [ ] **C-DA-01.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-01. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-01.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-03"></a>
## C-DA-03 — artifact and scenario identity

**Original requirement:** artifact and scenario identity

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 94.

### Preserved original contract

**Depends on:** build metadata and ZLLOG.

**Deliver:** signed or integrity-protected build ID, source revision, configuration
digest and randomized scenario nonce in kernel, service and app receipts.

**Invariants:** host, QEMU and hardware results bind to one artifact; a stale ISO,
kernel, package or screenshot cannot satisfy the current run.

**Proof:** deliberately run the previous image and require the gate to fail before
any behavioral assertion is accepted.

### Execution steps

- [ ] **C-DA-03.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-03.02 — Resolve this contract's exact dependencies**
  - Action: build metadata and ZLLOG. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-03.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-03.03 — Deliver — artifact and scenario identity**
  - Action: signed or integrity-protected build ID, source revision, configuration digest and randomized scenario nonce in kernel, service and app receipts.
  - Requires: C-DA-03.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-03.
- [ ] **C-DA-03.04 — Invariants — artifact and scenario identity**
  - Action: host, QEMU and hardware results bind to one artifact; a stale ISO, kernel, package or screenshot cannot satisfy the current run.
  - Requires: C-DA-03.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-03.
- [ ] **C-DA-03.05 — Proof — artifact and scenario identity**
  - Action: deliberately run the previous image and require the gate to fail before any behavioral assertion is accepted.
  - Requires: C-DA-03.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-03.
- [ ] **C-DA-03.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-03. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-03.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-08o"></a>
## C-DA-08O — observability service

**Original requirement:** observability service

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 252.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-08O.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-08O.02 — Resolve this contract's exact dependencies**
  - Action: DA-08, logger/ZLLOG import and process/service identity. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-08O.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-08O.03 — Deliver — observability service**
  - Action: typed read-only views for device lifecycle/resources/queues, storage health, network/link/socket state, battery/power/sensors, audio streams/xruns, logs, processes, sessions, services, packages and evidence receipts. Device Manager, System Monitor and diagnostics consume the same records.
  - Requires: C-DA-08O.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-08O.
- [ ] **C-DA-08O.04 — Invariants — observability service**
  - Action: unavailable is distinct from zero; records are generation- and artifact-bound; secrets and cross-user data are redacted; retention and subscriber queues are bounded; telemetry failure cannot stall providers; provider/service restart cannot merge old and new epochs.
  - Requires: C-DA-08O.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-08O.
- [ ] **C-DA-08O.05 — Proof — observability service**
  - Action: provider restart during subscription, unavailable field, counter wrap, retention exhaustion, slow/dead subscriber, redaction, inconsistent source fixtures, and byte-for-byte agreement between Device Manager/System Monitor for shared facts.
  - Requires: C-DA-08O.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-08O.
- [ ] **C-DA-08O.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-08O. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-08O.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-39"></a>
## C-DA-39 — provenance and architecture history

**Original requirement:** provenance and architecture history

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 896.

### Preserved original contract

**Depends on:** DA-01 and normal version control.

**Deliver:** architecture-epoch tags, parent-connected migrations, imported/generated
boundaries, per-driver/app feature receipts and a queryable change rationale.

**Proof:** schema/link/duplicate validation, missing evidence link, rewritten source
identity, generated artifact without inputs and license/provenance gap.

### Execution steps

- [ ] **C-DA-39.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-39.02 — Resolve this contract's exact dependencies**
  - Action: DA-01 and normal version control. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-39.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-39.03 — Deliver — provenance and architecture history**
  - Action: architecture-epoch tags, parent-connected migrations, imported/generated boundaries, per-driver/app feature receipts and a queryable change rationale.
  - Requires: C-DA-39.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-39.
- [ ] **C-DA-39.04 — Proof — provenance and architecture history**
  - Action: schema/link/duplicate validation, missing evidence link, rewritten source identity, generated artifact without inputs and license/provenance gap.
  - Requires: C-DA-39.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-39.
- [ ] **C-DA-39.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-39. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-39.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-40"></a>
## C-DA-40 — host deterministic gate

**Original requirement:** host deterministic gate

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 908.

### Preserved original contract

Run real shared logic where possible: descriptor/register/ring models, parser fuzz
and sanitizers, block/network/audio fault simulation, registry generation, app/game
logic, package transaction injection and protocol conformance. Build-only fixtures
are named build-only and cannot pass an execution gate.

### Execution steps

- [ ] **C-DA-40.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-40.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-40.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-40.03 — Execute the preserved contract requirements**
  - Action: Run real shared logic where possible: descriptor/register/ring models, parser fuzz and sanitizers, block/network/audio fault simulation, registry generation, app/game logic, package transaction injection and protocol conformance. Build-only fixtures are named build-only and cannot pass an execution gate.
  - Requires: C-DA-40.02.
  - Acceptance: Every requirement in the original contract has an independent observable result.
- [ ] **C-DA-40.04 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-40. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-40.03.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-44"></a>
## C-DA-44 — independent contradiction gate

**Original requirement:** independent contradiction gate

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 933.

### Preserved original contract

Before promotion, a separate reviewer tries to disprove the claim from source,
artifact identity, reachability, failure cleanup, teardown and architecture/hardware
coverage. Corrections update both the evidence ledger and the user-facing feature
surface; old broad wording cannot remain authoritative.

### Execution steps

- [ ] **C-DA-44.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-44.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-44.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-44.03 — Execute the preserved contract requirements**
  - Action: Before promotion, a separate reviewer tries to disprove the claim from source, artifact identity, reachability, failure cleanup, teardown and architecture/hardware coverage. Corrections update both the evidence ledger and the user-facing feature surface; old broad wording cannot remain authoritative.
  - Requires: C-DA-44.02.
  - Acceptance: Every requirement in the original contract has an independent observable result.
- [ ] **C-DA-44.04 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-44. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-44.03.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-00"></a>
## C-VX-00 — Visual/app/browser evidence registry

**Original requirement:** Visual/app/browser evidence registry

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 80.

### Preserved original contract

**Depends on:** existing source manifest and build artifact receipts.

**Deliver:** a machine-readable record per feature with repository/commit,
source path, build/image/init/user-route, screenshot vintage, host/QEMU/physical
run, exact behavior, date, artifact digest and known contradiction.

**Invariants:** evidence is per behavior; no transitive promotion; historical
screenshots are labeled; source names do not imply function.

**Proof:** seed one false source-only, old-screenshot, build-only, QEMU-only and
adjacent-hardware promotion; the validator rejects all five.

### Execution steps

- [ ] **C-VX-00.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-00.02 — Resolve this contract's exact dependencies**
  - Action: existing source manifest and build artifact receipts. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-00.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-00.03 — Deliver — Visual/app/browser evidence registry**
  - Action: a machine-readable record per feature with repository/commit, source path, build/image/init/user-route, screenshot vintage, host/QEMU/physical run, exact behavior, date, artifact digest and known contradiction.
  - Requires: C-VX-00.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-00.
- [ ] **C-VX-00.04 — Invariants — Visual/app/browser evidence registry**
  - Action: evidence is per behavior; no transitive promotion; historical screenshots are labeled; source names do not imply function.
  - Requires: C-VX-00.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-00.
- [ ] **C-VX-00.05 — Proof — Visual/app/browser evidence registry**
  - Action: seed one false source-only, old-screenshot, build-only, QEMU-only and adjacent-hardware promotion; the validator rejects all five.
  - Requires: C-VX-00.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-00.
- [ ] **C-VX-00.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-00. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-00.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-01"></a>
## C-VX-01 — Current application registry repair and oracle

**Original requirement:** Current application registry repair and oracle

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 94.

### Preserved original contract

**Depends on:** VX-00.

**Deliver:** generated single-source registry for every app/shell surface with
unique ID, exact name, family, icon, size constraints, entry point, draw/event/
tick requirements, package/image membership and launch routes.

**Invariants:** no blank IDs; no duplicate ID/name; every declared route reaches
the same app; dense numeric ranges never imply existence.

**Proof:** independently remove/mutate ID, name, icon, size, constructor, draw,
event, tick, package, catalogue, dock, shell and launch-ready nonce. Every
mutation fails. Reproduce and then eliminate the Maze/blank-ID false green.

### Execution steps

- [ ] **C-VX-01.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-01.02 — Resolve this contract's exact dependencies**
  - Action: VX-00. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-01.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-01.03 — Deliver — Current application registry repair and oracle**
  - Action: generated single-source registry for every app/shell surface with unique ID, exact name, family, icon, size constraints, entry point, draw/event/ tick requirements, package/image membership and launch routes.
  - Requires: C-VX-01.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-01.
- [ ] **C-VX-01.04 — Invariants — Current application registry repair and oracle**
  - Action: no blank IDs; no duplicate ID/name; every declared route reaches the same app; dense numeric ranges never imply existence.
  - Requires: C-VX-01.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-01.
- [ ] **C-VX-01.05 — Proof — Current application registry repair and oracle**
  - Action: independently remove/mutate ID, name, icon, size, constructor, draw, event, tick, package, catalogue, dock, shell and launch-ready nonce. Every mutation fails. Reproduce and then eliminate the Maze/blank-ID false green.
  - Requires: C-VX-01.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-01.
- [ ] **C-VX-01.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-01. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-01.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-48"></a>
## C-VX-48 — Visual-regression oracle

**Original requirement:** Visual-regression oracle

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 733.

### Preserved original contract

**Depends on:** VX-02..14.

**Deliver:** component and whole-desktop regional corpus; color/palette/hue/
structure/text/layout metrics; tolerances calibrated against renderer variance.

**Proof:** planted one-pixel structure, wrong accent, wrong font size, missing
focus, clipped text and stale screenshot defects each fail the expected metric.

### Execution steps

- [ ] **C-VX-48.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-48.02 — Resolve this contract's exact dependencies**
  - Action: VX-02..14. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-48.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-48.03 — Deliver — Visual-regression oracle**
  - Action: component and whole-desktop regional corpus; color/palette/hue/ structure/text/layout metrics; tolerances calibrated against renderer variance.
  - Requires: C-VX-48.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-48.
- [ ] **C-VX-48.04 — Proof — Visual-regression oracle**
  - Action: planted one-pixel structure, wrong accent, wrong font size, missing focus, clipped text and stale screenshot defects each fail the expected metric.
  - Requires: C-VX-48.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-48.
- [ ] **C-VX-48.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-48. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-48.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-49"></a>
## C-VX-49 — Interaction replay and mutation gate

**Original requirement:** Interaction replay and mutation gate

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 743.

### Preserved original contract

**Depends on:** VX-10, VX-13..25.

**Deliver:** deterministic input/event replay with screenshots, semantic trees,
focus owner, app state and side-effect assertions.

**Proof:** one mutation per required route/state/effect; no dense-ID inference;
lost/double/reordered input and process death.

### Execution steps

- [ ] **C-VX-49.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-49.02 — Resolve this contract's exact dependencies**
  - Action: VX-10, VX-13..25. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-49.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-49.03 — Deliver — Interaction replay and mutation gate**
  - Action: deterministic input/event replay with screenshots, semantic trees, focus owner, app state and side-effect assertions.
  - Requires: C-VX-49.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-49.
- [ ] **C-VX-49.04 — Proof — Interaction replay and mutation gate**
  - Action: one mutation per required route/state/effect; no dense-ID inference; lost/double/reordered input and process death.
  - Requires: C-VX-49.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-49.
- [ ] **C-VX-49.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-49. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-49.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-50"></a>
## C-VX-50 — App workflow certification matrix

**Original requirement:** App workflow certification matrix

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 753.

### Preserved original contract

**Depends on:** VX-18..47.

**Deliver:** per app: install -> launch -> ready -> empty/open/create -> action/
edit -> save/export/share -> close -> reopen/restore plus denial/failure/recovery.

**Invariants:** UI result and underlying effect checked separately; every app
has keyboard/accessibility route; cleanup census is zero.

**Proof:** automated matrix plus bounded manual/physical journeys for visuals
and device interactions that cannot be host-proven.

### Execution steps

- [ ] **C-VX-50.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-50.02 — Resolve this contract's exact dependencies**
  - Action: VX-18..47. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-50.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-50.03 — Deliver — App workflow certification matrix**
  - Action: per app: install -> launch -> ready -> empty/open/create -> action/ edit -> save/export/share -> close -> reopen/restore plus denial/failure/recovery.
  - Requires: C-VX-50.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-50.
- [ ] **C-VX-50.04 — Invariants — App workflow certification matrix**
  - Action: UI result and underlying effect checked separately; every app has keyboard/accessibility route; cleanup census is zero.
  - Requires: C-VX-50.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-50.
- [ ] **C-VX-50.05 — Proof — App workflow certification matrix**
  - Action: automated matrix plus bounded manual/physical journeys for visuals and device interactions that cannot be host-proven.
  - Requires: C-VX-50.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-50.
- [ ] **C-VX-50.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-50. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-50.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-svc-009"></a>
## T-SVC-009 — Audit/Event Service

**Original requirement:** typed effects, identity, redaction, anchoring and drops

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 26.

### Execution steps

- [ ] **T-SVC-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Audit/Event Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-009.02 — Specify the complete target boundary**
  - Action: Audit/Event Service must supply: typed effects, identity, redaction, anchoring and drops. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse Audit/Event Service through the shared platform contract, delivering every part of: typed effects, identity, redaction, anchoring and drops. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-009.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Audit/Event Service.
- [ ] **T-SVC-009.05 — Qualify and retain this target's own result**
  - Action: Bind Audit/Event Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-043"></a>
## T-SVC-043 — Provenance/License Service

**Original requirement:** artifact origin, SBOM, licenses, signatures and health

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 62.

### Execution steps

- [ ] **T-SVC-043.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Provenance/License Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-03, H-00.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-043.02 — Specify the complete target boundary**
  - Action: Provenance/License Service must supply: artifact origin, SBOM, licenses, signatures and health. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-043.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-043.03 — Implement the exact target behavior**
  - Action: Implement or reuse Provenance/License Service through the shared platform contract, delivering every part of: artifact origin, SBOM, licenses, signatures and health. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-043.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-043.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-043.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Provenance/License Service.
- [ ] **T-SVC-043.05 — Qualify and retain this target's own result**
  - Action: Bind Provenance/License Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-043.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
