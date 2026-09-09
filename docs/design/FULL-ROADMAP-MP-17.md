# MP-17: Add agents and automation through ordinary system authority

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `docs/program/LANGUAGE-AGENTS-OPERATIONS.md; kernel/apps/; stdlib/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-17` exports: A typed broker executing bounded, inspectable plans through capabilities, with cancellation and audit.

The handoff enables only its named subset. `CLOSE-17` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-17.01 — Define typed plans, tool descriptors, input/output schemas and deterministic effect validation

Define typed plans, tool descriptors, input/output schemas and deterministic effect validation.

**Requires:** `D-01`, `D-02`, `D-23`, `H-00`, `H-04`, `H-06`, `H-11`, `H-15`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-17.02 — Bind every tool call to explicit process/session authority, resource budgets, deadline and cancellation

Bind every tool call to explicit process/session authority, resource budgets, deadline and cancellation.

**Requires:** `M-17.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-17.03 — Implement child-agent attenuation, parent-death cleanup and revoke-before-spawn tests

Implement child-agent attenuation, parent-death cleanup and revoke-before-spawn tests.

**Requires:** `M-17.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-17 — Bounded development handoff: Add agents and automation through ordinary system authority

A typed broker executing bounded, inspectable plans through capabilities, with cancellation and audit.

**Requires:** `M-17.03`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-17.04 — Define secret mediation, untrusted-content taint, privacy classes, redaction and durable effect receipts

Define secret mediation, untrusted-content taint, privacy classes, redaction and durable effect receipts.

**Requires:** `M-17.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-17.05 — Build versioned signed skills/plugins with transactional installation, revocation and rollback

Build versioned signed skills/plugins with transactional installation, revocation and rollback.

**Requires:** `M-17.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-17.06 — Provide semantic desktop automation, scheduled jobs and workflow recovery without unrestricted kernel or device authority

Provide semantic desktop automation, scheduled jobs and workflow recovery without unrestricted kernel or device authority.

**Requires:** `M-17.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-17.07 — Separate proposed actions, required consent, execution and independent completion verification

Separate proposed actions, required consent, execution and independent completion verification.

**Requires:** `M-17.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-17.08 — Test hostile tools/content, false success, data exfiltration attempts, exhausted budgets, interrupted installs and recovery

Test hostile tools/content, false success, data exfiltration attempts, exhausted budgets, interrupted installs and recovery.

**Requires:** `M-17.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-AG-001](#f-ag-001) | feature | agent role manifest |
| [F-AG-002](#f-ag-002) | feature | typed plan |
| [F-AG-003](#f-ag-003) | feature | deterministic tool broker |
| [F-AG-004](#f-ag-004) | feature | scoped filesystem tools |
| [F-AG-005](#f-ag-005) | feature | scoped process tools |
| [F-AG-006](#f-ag-006) | feature | scoped network tools |
| [F-AG-007](#f-ag-007) | feature | scoped UI/browser tools |
| [F-AG-008](#f-ag-008) | feature | consent checkpoint |
| [F-AG-009](#f-ag-009) | feature | human interruption/redirect |
| [F-AG-010](#f-ag-010) | feature | subagent delegation |
| [F-AG-011](#f-ag-011) | feature | agent termination |
| [F-AG-012](#f-ag-012) | feature | role separation |
| [F-AG-013](#f-ag-013) | feature | independent review |
| [F-AG-014](#f-ag-014) | feature | agent memory |
| [F-AG-015](#f-ag-015) | feature | context assembly |
| [F-AG-016](#f-ag-016) | feature | skill/package format |
| [F-AG-017](#f-ag-017) | feature | atomic skill install |
| [F-AG-018](#f-ag-018) | feature | skill uninstall/revoke |
| [F-AG-019](#f-ag-019) | feature | model/provider abstraction |
| [F-AG-020](#f-ag-020) | feature | agent app workspace |
| [F-AG-021](#f-ag-021) | feature | command/tool discovery |
| [F-AG-022](#f-ag-022) | feature | task lifecycle |
| [F-AG-023](#f-ag-023) | feature | task budgets |
| [F-AG-024](#f-ag-024) | feature | plan approval policy |
| [F-AG-025](#f-ag-025) | feature | provenance/taint labels |
| [F-AG-026](#f-ag-026) | feature | agent notifications |
| [F-AG-027](#f-ag-027) | feature | scheduled automation |
| [F-AG-028](#f-ag-028) | feature | remote worker |
| [F-AG-029](#f-ag-029) | feature | agent observability |
| [F-AG-030](#f-ag-030) | feature | agent recovery |
| [C-P10.1](#c-p10-1) | contract | typed plan and deterministic tool broker |
| [C-P10.2](#c-p10-2) | contract | roles, subagents and resource budgets |
| [C-P10.3](#c-p10-3) | contract | signed skills/plugins |
| [C-P10.4](#c-p10-4) | contract | provenance, taint and privacy |
| [C-P10.5](#c-p10-5) | contract | independent completion reviewer |
| [C-DA-35](#c-da-35) | contract | agent and automation applications |
| [C-VX-46](#c-vx-46) | contract | Agent and automation experience |
| [T-SVC-095](#t-svc-095) | target | Speech Recognition Service |
| [T-SVC-130](#t-svc-130) | target | Agent Orchestrator |
| [T-SVC-131](#t-svc-131) | target | Tool Broker |
| [T-SVC-132](#t-svc-132) | target | Skill/Plugin Manager |
| [T-SVC-133](#t-svc-133) | target | Automation/Scheduler Service |
| [T-SVC-134](#t-svc-134) | target | Independent Review Service |
| [T-APP-090](#t-app-090) | target | Agent/Automation Center |
| [T-APP-091](#t-app-091) | target | Workflow Editor |
| [T-APP-092](#t-app-092) | target | Scheduled Tasks |
| [T-APP-093](#t-app-093) | target | Skill/Plugin Manager |
| [T-AGT-001](#t-agt-001) | target | typed intent/plan schema |
| [T-AGT-002](#t-agt-002) | target | deterministic tool broker |
| [T-AGT-003](#t-agt-003) | target | attenuated subagents |
| [T-AGT-004](#t-agt-004) | target | cancellation and parent-death |
| [T-AGT-005](#t-agt-005) | target | resource budgets |
| [T-AGT-006](#t-agt-006) | target | approval policy |
| [T-AGT-007](#t-agt-007) | target | secret mediation |
| [T-AGT-008](#t-agt-008) | target | provenance/taint graph |
| [T-AGT-009](#t-agt-009) | target | durable audit receipts |
| [T-AGT-010](#t-agt-010) | target | signed skill/plugin format |
| [T-AGT-011](#t-agt-011) | target | transactional skill install |
| [T-AGT-012](#t-agt-012) | target | complete uninstall/revoke |
| [T-AGT-013](#t-agt-013) | target | tool and role registry |
| [T-AGT-014](#t-agt-014) | target | scheduled automation |
| [T-AGT-015](#t-agt-015) | target | desktop semantic automation |
| [T-AGT-016](#t-agt-016) | target | filesystem/workspace automation |
| [T-AGT-017](#t-agt-017) | target | network/API automation |
| [T-AGT-018](#t-agt-018) | target | interactive progress/control UI |
| [T-AGT-019](#t-agt-019) | target | independent completion reviewer |
| [T-AGT-020](#t-agt-020) | target | evaluation/regression harness |
| [T-AGT-021](#t-agt-021) | target | offline/local model provider seam |
| [T-AGT-022](#t-agt-022) | target | remote model provider seam |
| [T-AGT-023](#t-agt-023) | target | human handoff/continuation |
| [T-AGT-024](#t-agt-024) | target | agent recovery/checkpoint |

<a id="f-ag-001"></a>
## F-AG-001 — agent role manifest

**Original requirement:** identity, purpose, model/tool inputs, authority, exposure, resources and lifecycle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-001.01 — Reconcile existing agent role manifest**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent role manifest. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: identity, purpose, model/tool inputs, authority, exposure, resources and lifecycle
- [ ] **F-AG-001.02 — Freeze the exact contract for agent role manifest**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: identity, purpose, model/tool inputs, authority, exposure, resources and lifecycle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-001.03 — Implement/prove: identity**
  - Action: For agent role manifest, implement or reuse and verify this exact obligation: identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identity; retain observable state/resource expectations.
- [ ] **F-AG-001.04 — Implement/prove: purpose**
  - Action: For agent role manifest, implement or reuse and verify this exact obligation: purpose. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for purpose; retain observable state/resource expectations.
- [ ] **F-AG-001.05 — Implement/prove: model/tool inputs**
  - Action: For agent role manifest, implement or reuse and verify this exact obligation: model/tool inputs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for model/tool inputs; retain observable state/resource expectations.
- [ ] **F-AG-001.06 — Implement/prove: authority**
  - Action: For agent role manifest, implement or reuse and verify this exact obligation: authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authority; retain observable state/resource expectations.
- [ ] **F-AG-001.07 — Implement/prove: exposure**
  - Action: For agent role manifest, implement or reuse and verify this exact obligation: exposure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exposure; retain observable state/resource expectations.
- [ ] **F-AG-001.08 — Implement/prove: resources and lifecycle**
  - Action: For agent role manifest, implement or reuse and verify this exact obligation: resources and lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-001.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resources and lifecycle; retain observable state/resource expectations.
- [ ] **F-AG-001.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent role manifest: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-001.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-001.10 — Integrate into the real consumer and runtime route**
  - Action: Wire agent role manifest into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-001.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-001.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent role manifest as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-001.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-002"></a>
## F-AG-002 — typed plan

**Original requirement:** inspectable versioned steps/preconditions/effects/rollback/proof before execution

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-002.01 — Reconcile existing typed plan**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for typed plan. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: inspectable versioned steps/preconditions/effects/rollback/proof before execution
- [ ] **F-AG-002.02 — Freeze the exact contract for typed plan**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: inspectable versioned steps/preconditions/effects/rollback/proof before execution. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-002.03 — Implement/prove: inspectable versioned steps/preconditions/effects/rollback/proof before execution**
  - Action: For typed plan, implement or reuse and verify this exact obligation: inspectable versioned steps/preconditions/effects/rollback/proof before execution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for inspectable versioned steps/preconditions/effects/rollback/proof before execution; retain observable state/resource expectations.
- [ ] **F-AG-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to typed plan: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire typed plan into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for typed plan as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-003"></a>
## F-AG-003 — deterministic tool broker

**Original requirement:** validates operation/arguments/handles/policy and emits effect receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-003.01 — Reconcile existing deterministic tool broker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for deterministic tool broker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validates operation/arguments/handles/policy and emits effect receipt
- [ ] **F-AG-003.02 — Freeze the exact contract for deterministic tool broker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validates operation/arguments/handles/policy and emits effect receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-003.03 — Implement/prove: validates operation/arguments/handles/policy and emits effect receipt**
  - Action: For deterministic tool broker, implement or reuse and verify this exact obligation: validates operation/arguments/handles/policy and emits effect receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validates operation/arguments/handles/policy and emits effect receipt; retain observable state/resource expectations.
- [ ] **F-AG-003.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to deterministic tool broker: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-003.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-003.05 — Integrate into the real consumer and runtime route**
  - Action: Wire deterministic tool broker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-003.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-003.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for deterministic tool broker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-003.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-004"></a>
## F-AG-004 — scoped filesystem tools

**Original requirement:** handle-relative read/write/search/patch with bounds, diff and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-004.01 — Reconcile existing scoped filesystem tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scoped filesystem tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: handle-relative read/write/search/patch with bounds, diff and rollback
- [ ] **F-AG-004.02 — Freeze the exact contract for scoped filesystem tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: handle-relative read/write/search/patch with bounds, diff and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-004.03 — Implement/prove: handle-relative read/write/search/patch with bounds**
  - Action: For scoped filesystem tools, implement or reuse and verify this exact obligation: handle-relative read/write/search/patch with bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handle-relative read/write/search/patch with bounds; retain observable state/resource expectations.
- [ ] **F-AG-004.04 — Implement/prove: diff and rollback**
  - Action: For scoped filesystem tools, implement or reuse and verify this exact obligation: diff and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diff and rollback; retain observable state/resource expectations.
- [ ] **F-AG-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scoped filesystem tools: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire scoped filesystem tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scoped filesystem tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-005"></a>
## F-AG-005 — scoped process tools

**Original requirement:** exact executable/args/env/resources/deadline/output and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-005.01 — Reconcile existing scoped process tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scoped process tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact executable/args/env/resources/deadline/output and cancellation
- [ ] **F-AG-005.02 — Freeze the exact contract for scoped process tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact executable/args/env/resources/deadline/output and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-005.03 — Implement/prove: exact executable/args/env/resources/deadline/output and cancellation**
  - Action: For scoped process tools, implement or reuse and verify this exact obligation: exact executable/args/env/resources/deadline/output and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact executable/args/env/resources/deadline/output and cancellation; retain observable state/resource expectations.
- [ ] **F-AG-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scoped process tools: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire scoped process tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scoped process tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-006"></a>
## F-AG-006 — scoped network tools

**Original requirement:** allowed destinations/methods/bytes/time/provenance and egress logging

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-006.01 — Reconcile existing scoped network tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scoped network tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: allowed destinations/methods/bytes/time/provenance and egress logging
- [ ] **F-AG-006.02 — Freeze the exact contract for scoped network tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: allowed destinations/methods/bytes/time/provenance and egress logging. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-006.03 — Implement/prove: allowed destinations/methods/bytes/time/provenance and egress logging**
  - Action: For scoped network tools, implement or reuse and verify this exact obligation: allowed destinations/methods/bytes/time/provenance and egress logging. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allowed destinations/methods/bytes/time/provenance and egress logging; retain observable state/resource expectations.
- [ ] **F-AG-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scoped network tools: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire scoped network tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scoped network tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-007"></a>
## F-AG-007 — scoped UI/browser tools

**Original requirement:** semantic targets, authenticated session, action receipts and no ambient input

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-007.01 — Reconcile existing scoped UI/browser tools**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scoped UI/browser tools. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic targets, authenticated session, action receipts and no ambient input
- [ ] **F-AG-007.02 — Freeze the exact contract for scoped UI/browser tools**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic targets, authenticated session, action receipts and no ambient input. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-007.03 — Implement/prove: semantic targets**
  - Action: For scoped UI/browser tools, implement or reuse and verify this exact obligation: semantic targets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic targets; retain observable state/resource expectations.
- [ ] **F-AG-007.04 — Implement/prove: authenticated session**
  - Action: For scoped UI/browser tools, implement or reuse and verify this exact obligation: authenticated session. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated session; retain observable state/resource expectations.
- [ ] **F-AG-007.05 — Implement/prove: action receipts and no ambient input**
  - Action: For scoped UI/browser tools, implement or reuse and verify this exact obligation: action receipts and no ambient input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for action receipts and no ambient input; retain observable state/resource expectations.
- [ ] **F-AG-007.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scoped UI/browser tools: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-007.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-007.07 — Integrate into the real consumer and runtime route**
  - Action: Wire scoped UI/browser tools into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-007.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-007.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scoped UI/browser tools as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-007.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-008"></a>
## F-AG-008 — consent checkpoint

**Original requirement:** binds exact effect/target/duration/digest to fresh human decision

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-008.01 — Reconcile existing consent checkpoint**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for consent checkpoint. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: binds exact effect/target/duration/digest to fresh human decision
- [ ] **F-AG-008.02 — Freeze the exact contract for consent checkpoint**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: binds exact effect/target/duration/digest to fresh human decision. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-008.03 — Implement/prove: binds exact effect/target/duration/digest to fresh human decision**
  - Action: For consent checkpoint, implement or reuse and verify this exact obligation: binds exact effect/target/duration/digest to fresh human decision. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for binds exact effect/target/duration/digest to fresh human decision; retain observable state/resource expectations.
- [ ] **F-AG-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to consent checkpoint: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire consent checkpoint into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for consent checkpoint as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-009"></a>
## F-AG-009 — human interruption/redirect

**Original requirement:** safe cancellation boundary, state reconciliation and revised plan

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-009.01 — Reconcile existing human interruption/redirect**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for human interruption/redirect. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: safe cancellation boundary, state reconciliation and revised plan
- [ ] **F-AG-009.02 — Freeze the exact contract for human interruption/redirect**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: safe cancellation boundary, state reconciliation and revised plan. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-009.03 — Implement/prove: safe cancellation boundary**
  - Action: For human interruption/redirect, implement or reuse and verify this exact obligation: safe cancellation boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe cancellation boundary; retain observable state/resource expectations.
- [ ] **F-AG-009.04 — Implement/prove: state reconciliation and revised plan**
  - Action: For human interruption/redirect, implement or reuse and verify this exact obligation: state reconciliation and revised plan. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for state reconciliation and revised plan; retain observable state/resource expectations.
- [ ] **F-AG-009.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to human interruption/redirect: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-009.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-009.06 — Integrate into the real consumer and runtime route**
  - Action: Wire human interruption/redirect into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-009.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-009.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for human interruption/redirect as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-009.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-010"></a>
## F-AG-010 — subagent delegation

**Original requirement:** live-parent attenuation, bounded concurrency/output/time, serialized revoke/spawn

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-010.01 — Reconcile existing subagent delegation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for subagent delegation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: live-parent attenuation, bounded concurrency/output/time, serialized revoke/spawn
- [ ] **F-AG-010.02 — Freeze the exact contract for subagent delegation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: live-parent attenuation, bounded concurrency/output/time, serialized revoke/spawn. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-010.03 — Implement/prove: live-parent attenuation**
  - Action: For subagent delegation, implement or reuse and verify this exact obligation: live-parent attenuation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for live-parent attenuation; retain observable state/resource expectations.
- [ ] **F-AG-010.04 — Implement/prove: bounded concurrency/output/time**
  - Action: For subagent delegation, implement or reuse and verify this exact obligation: bounded concurrency/output/time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded concurrency/output/time; retain observable state/resource expectations.
- [ ] **F-AG-010.05 — Implement/prove: serialized revoke/spawn**
  - Action: For subagent delegation, implement or reuse and verify this exact obligation: serialized revoke/spawn. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for serialized revoke/spawn; retain observable state/resource expectations.
- [ ] **F-AG-010.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to subagent delegation: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-010.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-010.07 — Integrate into the real consumer and runtime route**
  - Action: Wire subagent delegation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-010.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-010.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for subagent delegation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-010.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-011"></a>
## F-AG-011 — agent termination

**Original requirement:** task becomes dead and every child/tool/handle/process is stale or cleaned

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-011.01 — Reconcile existing agent termination**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent termination. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: task becomes dead and every child/tool/handle/process is stale or cleaned
- [ ] **F-AG-011.02 — Freeze the exact contract for agent termination**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: task becomes dead and every child/tool/handle/process is stale or cleaned. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-011.03 — Implement/prove: task becomes dead and every child/tool/handle/process is stale or cleaned**
  - Action: For agent termination, implement or reuse and verify this exact obligation: task becomes dead and every child/tool/handle/process is stale or cleaned. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for task becomes dead and every child/tool/handle/process is stale or cleaned; retain observable state/resource expectations.
- [ ] **F-AG-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent termination: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire agent termination into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent termination as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-012"></a>
## F-AG-012 — role separation

**Original requirement:** planner/executor/provider/reviewer/remote gateway carry different authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-012.01 — Reconcile existing role separation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for role separation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: planner/executor/provider/reviewer/remote gateway carry different authority
- [ ] **F-AG-012.02 — Freeze the exact contract for role separation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: planner/executor/provider/reviewer/remote gateway carry different authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-012.03 — Implement/prove: planner/executor/provider/reviewer/remote gateway carry different authority**
  - Action: For role separation, implement or reuse and verify this exact obligation: planner/executor/provider/reviewer/remote gateway carry different authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for planner/executor/provider/reviewer/remote gateway carry different authority; retain observable state/resource expectations.
- [ ] **F-AG-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to role separation: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire role separation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for role separation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-013"></a>
## F-AG-013 — independent review

**Original requirement:** reviewer sees plan/diff/receipts, seeks counterexample, accepts/rejects only

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-013.01 — Reconcile existing independent review**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for independent review. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reviewer sees plan/diff/receipts, seeks counterexample, accepts/rejects only
- [ ] **F-AG-013.02 — Freeze the exact contract for independent review**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reviewer sees plan/diff/receipts, seeks counterexample, accepts/rejects only. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-013.03 — Implement/prove: reviewer sees plan/diff/receipts**
  - Action: For independent review, implement or reuse and verify this exact obligation: reviewer sees plan/diff/receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reviewer sees plan/diff/receipts; retain observable state/resource expectations.
- [ ] **F-AG-013.04 — Implement/prove: seeks counterexample**
  - Action: For independent review, implement or reuse and verify this exact obligation: seeks counterexample. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for seeks counterexample; retain observable state/resource expectations.
- [ ] **F-AG-013.05 — Implement/prove: accepts/rejects only**
  - Action: For independent review, implement or reuse and verify this exact obligation: accepts/rejects only. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accepts/rejects only; retain observable state/resource expectations.
- [ ] **F-AG-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to independent review: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire independent review into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for independent review as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-014"></a>
## F-AG-014 — agent memory

**Original requirement:** explicit user-approved durable artifacts, provenance, retention and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-014.01 — Reconcile existing agent memory**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent memory. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit user-approved durable artifacts, provenance, retention and revocation
- [ ] **F-AG-014.02 — Freeze the exact contract for agent memory**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit user-approved durable artifacts, provenance, retention and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-014.03 — Implement/prove: explicit user-approved durable artifacts**
  - Action: For agent memory, implement or reuse and verify this exact obligation: explicit user-approved durable artifacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit user-approved durable artifacts; retain observable state/resource expectations.
- [ ] **F-AG-014.04 — Implement/prove: provenance**
  - Action: For agent memory, implement or reuse and verify this exact obligation: provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance; retain observable state/resource expectations.
- [ ] **F-AG-014.05 — Implement/prove: retention and revocation**
  - Action: For agent memory, implement or reuse and verify this exact obligation: retention and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retention and revocation; retain observable state/resource expectations.
- [ ] **F-AG-014.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent memory: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-014.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-014.07 — Integrate into the real consumer and runtime route**
  - Action: Wire agent memory into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-014.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-014.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent memory as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-014.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-015"></a>
## F-AG-015 — context assembly

**Original requirement:** bounded task-scoped sources, freshness, citations and private-data exclusions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-015.01 — Reconcile existing context assembly**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for context assembly. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded task-scoped sources, freshness, citations and private-data exclusions
- [ ] **F-AG-015.02 — Freeze the exact contract for context assembly**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded task-scoped sources, freshness, citations and private-data exclusions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-015.03 — Implement/prove: bounded task-scoped sources**
  - Action: For context assembly, implement or reuse and verify this exact obligation: bounded task-scoped sources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded task-scoped sources; retain observable state/resource expectations.
- [ ] **F-AG-015.04 — Implement/prove: freshness**
  - Action: For context assembly, implement or reuse and verify this exact obligation: freshness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for freshness; retain observable state/resource expectations.
- [ ] **F-AG-015.05 — Implement/prove: citations and private-data exclusions**
  - Action: For context assembly, implement or reuse and verify this exact obligation: citations and private-data exclusions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for citations and private-data exclusions; retain observable state/resource expectations.
- [ ] **F-AG-015.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to context assembly: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-015.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-015.07 — Integrate into the real consumer and runtime route**
  - Action: Wire context assembly into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-015.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-015.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for context assembly as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-015.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-016"></a>
## F-AG-016 — skill/package format

**Original requirement:** signed versioned instructions/tools/assets, declared grants/dependencies and tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-016.01 — Reconcile existing skill/package format**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for skill/package format. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed versioned instructions/tools/assets, declared grants/dependencies and tests
- [ ] **F-AG-016.02 — Freeze the exact contract for skill/package format**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed versioned instructions/tools/assets, declared grants/dependencies and tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-016.03 — Implement/prove: signed versioned instructions/tools/assets**
  - Action: For skill/package format, implement or reuse and verify this exact obligation: signed versioned instructions/tools/assets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed versioned instructions/tools/assets; retain observable state/resource expectations.
- [ ] **F-AG-016.04 — Implement/prove: declared grants/dependencies and tests**
  - Action: For skill/package format, implement or reuse and verify this exact obligation: declared grants/dependencies and tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared grants/dependencies and tests; retain observable state/resource expectations.
- [ ] **F-AG-016.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to skill/package format: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-016.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-016.06 — Integrate into the real consumer and runtime route**
  - Action: Wire skill/package format into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-016.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-016.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for skill/package format as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-016.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-017"></a>
## F-AG-017 — atomic skill install

**Original requirement:** stage/verify/admit/publish or exact rollback across files/tools/roles/grants

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-017.01 — Reconcile existing atomic skill install**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomic skill install. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stage/verify/admit/publish or exact rollback across files/tools/roles/grants
- [ ] **F-AG-017.02 — Freeze the exact contract for atomic skill install**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stage/verify/admit/publish or exact rollback across files/tools/roles/grants. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-017.03 — Implement/prove: stage/verify/admit/publish or exact rollback across files/tools/roles/grants**
  - Action: For atomic skill install, implement or reuse and verify this exact obligation: stage/verify/admit/publish or exact rollback across files/tools/roles/grants. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stage/verify/admit/publish or exact rollback across files/tools/roles/grants; retain observable state/resource expectations.
- [ ] **F-AG-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomic skill install: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire atomic skill install into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomic skill install as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-018"></a>
## F-AG-018 — skill uninstall/revoke

**Original requirement:** stop users, revoke live authority, deregister and remove per retention policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-018.01 — Reconcile existing skill uninstall/revoke**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for skill uninstall/revoke. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stop users, revoke live authority, deregister and remove per retention policy
- [ ] **F-AG-018.02 — Freeze the exact contract for skill uninstall/revoke**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stop users, revoke live authority, deregister and remove per retention policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-018.03 — Implement/prove: stop users**
  - Action: For skill uninstall/revoke, implement or reuse and verify this exact obligation: stop users. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stop users; retain observable state/resource expectations.
- [ ] **F-AG-018.04 — Implement/prove: revoke live authority**
  - Action: For skill uninstall/revoke, implement or reuse and verify this exact obligation: revoke live authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revoke live authority; retain observable state/resource expectations.
- [ ] **F-AG-018.05 — Implement/prove: deregister and remove per retention policy**
  - Action: For skill uninstall/revoke, implement or reuse and verify this exact obligation: deregister and remove per retention policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deregister and remove per retention policy; retain observable state/resource expectations.
- [ ] **F-AG-018.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to skill uninstall/revoke: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-018.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-018.07 — Integrate into the real consumer and runtime route**
  - Action: Wire skill uninstall/revoke into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-018.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-018.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for skill uninstall/revoke as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-018.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-019"></a>
## F-AG-019 — model/provider abstraction

**Original requirement:** capabilities, limits, privacy, health, fallback and exact model provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-019.01 — Reconcile existing model/provider abstraction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for model/provider abstraction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: capabilities, limits, privacy, health, fallback and exact model provenance
- [ ] **F-AG-019.02 — Freeze the exact contract for model/provider abstraction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: capabilities, limits, privacy, health, fallback and exact model provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-019.03 — Implement/prove: capabilities**
  - Action: For model/provider abstraction, implement or reuse and verify this exact obligation: capabilities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capabilities; retain observable state/resource expectations.
- [ ] **F-AG-019.04 — Implement/prove: limits**
  - Action: For model/provider abstraction, implement or reuse and verify this exact obligation: limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for limits; retain observable state/resource expectations.
- [ ] **F-AG-019.05 — Implement/prove: privacy**
  - Action: For model/provider abstraction, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-AG-019.06 — Implement/prove: health**
  - Action: For model/provider abstraction, implement or reuse and verify this exact obligation: health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for health; retain observable state/resource expectations.
- [ ] **F-AG-019.07 — Implement/prove: fallback and exact model provenance**
  - Action: For model/provider abstraction, implement or reuse and verify this exact obligation: fallback and exact model provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-019.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback and exact model provenance; retain observable state/resource expectations.
- [ ] **F-AG-019.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to model/provider abstraction: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-019.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-019.09 — Integrate into the real consumer and runtime route**
  - Action: Wire model/provider abstraction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-019.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-019.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for model/provider abstraction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-019.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-020"></a>
## F-AG-020 — agent app workspace

**Original requirement:** chat/action panes, tabs, files, traces, plans, artifacts, permissions and status

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-020.01 — Reconcile existing agent app workspace**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent app workspace. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: chat/action panes, tabs, files, traces, plans, artifacts, permissions and status
- [ ] **F-AG-020.02 — Freeze the exact contract for agent app workspace**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: chat/action panes, tabs, files, traces, plans, artifacts, permissions and status. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-020.03 — Implement/prove: chat/action panes**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: chat/action panes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for chat/action panes; retain observable state/resource expectations.
- [ ] **F-AG-020.04 — Implement/prove: tabs**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: tabs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tabs; retain observable state/resource expectations.
- [ ] **F-AG-020.05 — Implement/prove: files**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: files. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files; retain observable state/resource expectations.
- [ ] **F-AG-020.06 — Implement/prove: traces**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: traces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for traces; retain observable state/resource expectations.
- [ ] **F-AG-020.07 — Implement/prove: plans**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: plans. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for plans; retain observable state/resource expectations.
- [ ] **F-AG-020.08 — Implement/prove: artifacts**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: artifacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for artifacts; retain observable state/resource expectations.
- [ ] **F-AG-020.09 — Implement/prove: permissions and status**
  - Action: For agent app workspace, implement or reuse and verify this exact obligation: permissions and status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-020.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions and status; retain observable state/resource expectations.
- [ ] **F-AG-020.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent app workspace: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-020.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-020.11 — Integrate into the real consumer and runtime route**
  - Action: Wire agent app workspace into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-020.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-020.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent app workspace as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-020.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-021"></a>
## F-AG-021 — command/tool discovery

**Original requirement:** searchable typed actions with parameters, permissions, examples and evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-021.01 — Reconcile existing command/tool discovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for command/tool discovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: searchable typed actions with parameters, permissions, examples and evidence
- [ ] **F-AG-021.02 — Freeze the exact contract for command/tool discovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: searchable typed actions with parameters, permissions, examples and evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-021.03 — Implement/prove: searchable typed actions with parameters**
  - Action: For command/tool discovery, implement or reuse and verify this exact obligation: searchable typed actions with parameters. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for searchable typed actions with parameters; retain observable state/resource expectations.
- [ ] **F-AG-021.04 — Implement/prove: permissions**
  - Action: For command/tool discovery, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-AG-021.05 — Implement/prove: examples and evidence**
  - Action: For command/tool discovery, implement or reuse and verify this exact obligation: examples and evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for examples and evidence; retain observable state/resource expectations.
- [ ] **F-AG-021.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to command/tool discovery: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-021.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-021.07 — Integrate into the real consumer and runtime route**
  - Action: Wire command/tool discovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-021.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-021.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for command/tool discovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-021.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-022"></a>
## F-AG-022 — task lifecycle

**Original requirement:** queued/running/waiting/review/failed/completed/cancelled with durable receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-022.01 — Reconcile existing task lifecycle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for task lifecycle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: queued/running/waiting/review/failed/completed/cancelled with durable receipts
- [ ] **F-AG-022.02 — Freeze the exact contract for task lifecycle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: queued/running/waiting/review/failed/completed/cancelled with durable receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-022.03 — Implement/prove: queued/running/waiting/review/failed/completed/cancelled with durable receipts**
  - Action: For task lifecycle, implement or reuse and verify this exact obligation: queued/running/waiting/review/failed/completed/cancelled with durable receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for queued/running/waiting/review/failed/completed/cancelled with durable receipts; retain observable state/resource expectations.
- [ ] **F-AG-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to task lifecycle: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire task lifecycle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for task lifecycle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-023"></a>
## F-AG-023 — task budgets

**Original requirement:** tokens/time/CPU/memory/process/network/storage/output and admission/backpressure

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-023.01 — Reconcile existing task budgets**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for task budgets. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tokens/time/CPU/memory/process/network/storage/output and admission/backpressure
- [ ] **F-AG-023.02 — Freeze the exact contract for task budgets**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tokens/time/CPU/memory/process/network/storage/output and admission/backpressure. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-023.03 — Implement/prove: tokens/time/CPU/memory/process/network/storage/output and admission/backpressure**
  - Action: For task budgets, implement or reuse and verify this exact obligation: tokens/time/CPU/memory/process/network/storage/output and admission/backpressure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tokens/time/CPU/memory/process/network/storage/output and admission/backpressure; retain observable state/resource expectations.
- [ ] **F-AG-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to task budgets: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire task budgets into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for task budgets as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-024"></a>
## F-AG-024 — plan approval policy

**Original requirement:** risk-based automatic vs human gates, non-reusable approvals and audit

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-024.01 — Reconcile existing plan approval policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for plan approval policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: risk-based automatic vs human gates, non-reusable approvals and audit
- [ ] **F-AG-024.02 — Freeze the exact contract for plan approval policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: risk-based automatic vs human gates, non-reusable approvals and audit. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-024.03 — Implement/prove: risk-based automatic vs human gates**
  - Action: For plan approval policy, implement or reuse and verify this exact obligation: risk-based automatic vs human gates. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for risk-based automatic vs human gates; retain observable state/resource expectations.
- [ ] **F-AG-024.04 — Implement/prove: non-reusable approvals and audit**
  - Action: For plan approval policy, implement or reuse and verify this exact obligation: non-reusable approvals and audit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for non-reusable approvals and audit; retain observable state/resource expectations.
- [ ] **F-AG-024.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to plan approval policy: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-024.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-024.06 — Integrate into the real consumer and runtime route**
  - Action: Wire plan approval policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-024.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-024.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for plan approval policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-024.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-025"></a>
## F-AG-025 — provenance/taint labels

**Original requirement:** scoped origin/effect tracking, conservative sinks and explicit declassification

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-025.01 — Reconcile existing provenance/taint labels**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for provenance/taint labels. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scoped origin/effect tracking, conservative sinks and explicit declassification
- [ ] **F-AG-025.02 — Freeze the exact contract for provenance/taint labels**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scoped origin/effect tracking, conservative sinks and explicit declassification. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-025.03 — Implement/prove: scoped origin/effect tracking**
  - Action: For provenance/taint labels, implement or reuse and verify this exact obligation: scoped origin/effect tracking. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scoped origin/effect tracking; retain observable state/resource expectations.
- [ ] **F-AG-025.04 — Implement/prove: conservative sinks and explicit declassification**
  - Action: For provenance/taint labels, implement or reuse and verify this exact obligation: conservative sinks and explicit declassification. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conservative sinks and explicit declassification; retain observable state/resource expectations.
- [ ] **F-AG-025.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to provenance/taint labels: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-025.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-025.06 — Integrate into the real consumer and runtime route**
  - Action: Wire provenance/taint labels into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-025.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-025.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for provenance/taint labels as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-025.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-026"></a>
## F-AG-026 — agent notifications

**Original requirement:** background progress/attention/completion with privacy and rate limits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-026.01 — Reconcile existing agent notifications**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent notifications. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: background progress/attention/completion with privacy and rate limits
- [ ] **F-AG-026.02 — Freeze the exact contract for agent notifications**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: background progress/attention/completion with privacy and rate limits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-026.03 — Implement/prove: background progress/attention/completion with privacy and rate limits**
  - Action: For agent notifications, implement or reuse and verify this exact obligation: background progress/attention/completion with privacy and rate limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for background progress/attention/completion with privacy and rate limits; retain observable state/resource expectations.
- [ ] **F-AG-026.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent notifications: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-026.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-026.05 — Integrate into the real consumer and runtime route**
  - Action: Wire agent notifications into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-026.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-026.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent notifications as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-026.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-027"></a>
## F-AG-027 — scheduled automation

**Original requirement:** authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-027.01 — Reconcile existing scheduled automation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scheduled automation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics
- [ ] **F-AG-027.02 — Freeze the exact contract for scheduled automation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-027.03 — Implement/prove: authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics**
  - Action: For scheduled automation, implement or reuse and verify this exact obligation: authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated schedule/timezone/missed-run/concurrency/cancel/result semantics; retain observable state/resource expectations.
- [ ] **F-AG-027.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scheduled automation: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-027.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-027.05 — Integrate into the real consumer and runtime route**
  - Action: Wire scheduled automation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-027.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-027.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scheduled automation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-027.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-028"></a>
## F-AG-028 — remote worker

**Original requirement:** authenticated role-minimal channel, health, checkpoint, budgets and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-028.01 — Reconcile existing remote worker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for remote worker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated role-minimal channel, health, checkpoint, budgets and revocation
- [ ] **F-AG-028.02 — Freeze the exact contract for remote worker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated role-minimal channel, health, checkpoint, budgets and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-028.03 — Implement/prove: authenticated role-minimal channel**
  - Action: For remote worker, implement or reuse and verify this exact obligation: authenticated role-minimal channel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated role-minimal channel; retain observable state/resource expectations.
- [ ] **F-AG-028.04 — Implement/prove: health**
  - Action: For remote worker, implement or reuse and verify this exact obligation: health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for health; retain observable state/resource expectations.
- [ ] **F-AG-028.05 — Implement/prove: checkpoint**
  - Action: For remote worker, implement or reuse and verify this exact obligation: checkpoint. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checkpoint; retain observable state/resource expectations.
- [ ] **F-AG-028.06 — Implement/prove: budgets and revocation**
  - Action: For remote worker, implement or reuse and verify this exact obligation: budgets and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for budgets and revocation; retain observable state/resource expectations.
- [ ] **F-AG-028.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to remote worker: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-028.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-028.08 — Integrate into the real consumer and runtime route**
  - Action: Wire remote worker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-028.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-028.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for remote worker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-028.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-029"></a>
## F-AG-029 — agent observability

**Original requirement:** plans/actions/authority/resources/failures/skips without leaking secrets

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-029.01 — Reconcile existing agent observability**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent observability. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: plans/actions/authority/resources/failures/skips without leaking secrets
- [ ] **F-AG-029.02 — Freeze the exact contract for agent observability**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: plans/actions/authority/resources/failures/skips without leaking secrets. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-029.03 — Implement/prove: plans/actions/authority/resources/failures/skips without leaking secrets**
  - Action: For agent observability, implement or reuse and verify this exact obligation: plans/actions/authority/resources/failures/skips without leaking secrets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for plans/actions/authority/resources/failures/skips without leaking secrets; retain observable state/resource expectations.
- [ ] **F-AG-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent observability: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire agent observability into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent observability as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ag-030"></a>
## F-AG-030 — agent recovery

**Original requirement:** checkpoint/idempotency/retry policy, partial-effect reconciliation and user control

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AG-030.01 — Reconcile existing agent recovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for agent recovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checkpoint/idempotency/retry policy, partial-effect reconciliation and user control
- [ ] **F-AG-030.02 — Freeze the exact contract for agent recovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checkpoint/idempotency/retry policy, partial-effect reconciliation and user control. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AG-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AG-030.03 — Implement/prove: checkpoint/idempotency/retry policy**
  - Action: For agent recovery, implement or reuse and verify this exact obligation: checkpoint/idempotency/retry policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checkpoint/idempotency/retry policy; retain observable state/resource expectations.
- [ ] **F-AG-030.04 — Implement/prove: partial-effect reconciliation and user control**
  - Action: For agent recovery, implement or reuse and verify this exact obligation: partial-effect reconciliation and user control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AG-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for partial-effect reconciliation and user control; retain observable state/resource expectations.
- [ ] **F-AG-030.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to agent recovery: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AG-030.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AG-030.06 — Integrate into the real consumer and runtime route**
  - Action: Wire agent recovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AG-030.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AG-030.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for agent recovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AG-030.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p10-1"></a>
## C-P10.1 — typed plan and deterministic tool broker

**Original requirement:** typed plan and deterministic tool broker

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 536.

### Preserved original contract

- **Dependencies/current/provenance:** P4/P5 services, zlIDL and process isolation; Chitti grammar corrected by RustOS receipts; reject ring-0 orchestrator/unconstrained shell.
- **I/O and state:** intent plus registered tool grammar and granted handles in; immutable typed plan, step results and receipt out; `Draft -> Validated -> Authorized -> Executing -> Complete|Failed|Cancelled`.
- **Invariants/failure:** only registered operations; plan cannot gain handles; arguments/bounds validated before effects; cancellation/rollback declared; every effect attributable.
- **Deterministic proof:** grammar mutation/fuzz, missing/wrong rights, replay, cancellation at each step, partial failure, deterministic plan replay.
- **Target proof:** QEMU agent performs granted file/app task and is denied ungranted network/device/process action.
- **Receipt/removal:** plan/authority/effect/artifact/reviewer digests; manual trusted client remains; broker cannot be bypassed by agent process.

### Execution steps

- [ ] **C-P10.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P10.1.02 — Resolve this contract's exact dependencies**
  - Action: P4/P5 services, zlIDL and process isolation; Chitti grammar corrected by RustOS receipts; reject ring-0 orchestrator/unconstrained shell. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P10.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P10.1.03 — I/O and state — typed plan and deterministic tool broker**
  - Action: intent plus registered tool grammar and granted handles in; immutable typed plan, step results and receipt out; `Draft -> Validated -> Authorized -> Executing -> Complete|Failed|Cancelled`.
  - Requires: C-P10.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P10.1.
- [ ] **C-P10.1.04 — Invariants/failure — typed plan and deterministic tool broker**
  - Action: only registered operations; plan cannot gain handles; arguments/bounds validated before effects; cancellation/rollback declared; every effect attributable.
  - Requires: C-P10.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P10.1.
- [ ] **C-P10.1.05 — Deterministic proof — typed plan and deterministic tool broker**
  - Action: grammar mutation/fuzz, missing/wrong rights, replay, cancellation at each step, partial failure, deterministic plan replay.
  - Requires: C-P10.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P10.1.
- [ ] **C-P10.1.06 — Target proof — typed plan and deterministic tool broker**
  - Action: QEMU agent performs granted file/app task and is denied ungranted network/device/process action.
  - Requires: C-P10.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P10.1.
- [ ] **C-P10.1.07 — Receipt/removal — typed plan and deterministic tool broker**
  - Action: plan/authority/effect/artifact/reviewer digests; manual trusted client remains; broker cannot be bypassed by agent process.
  - Requires: C-P10.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P10.1.
- [ ] **C-P10.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P10.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P10.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p10-2"></a>
## C-P10.2 — roles, subagents and resource budgets

**Original requirement:** roles, subagents and resource budgets

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 545.

### Preserved original contract

- **Dependencies/current/provenance:** P10.1 and supervisor; Chitti attenuation, snarkOS roles, NexiOS bounds; reject universal daemon.
- **I/O and state:** parent plan/grants/deadline/CPU-memory-output limits in; subagent process/result out; process lifecycle plus grant derivation.
- **Invariants/failure:** child grants derive at commit from current live parent handles; spawn/revocation serialized; child subset only; no grant widening or authority resurrection; parent/child/root-orchestrator termination makes the task dead and every descendant handle stale; deadline/cancel/revoke terminal; quota exhaustion explicit.
- **Deterministic proof:** revoke-then-spawn, kill root orchestrator with no orphan root-cap task, widening attempts, nested depth/count, deadline, output/memory/queue exhaustion, parent/child crash and revocation races.
- **Target proof:** QEMU multi-agent task under enforced budgets; desktop remains responsive.
- **Receipt/removal:** role/grant/resource/outcome tree; single-process execution rollback; no unbounded mode in production.

### Execution steps

- [ ] **C-P10.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P10.2.02 — Resolve this contract's exact dependencies**
  - Action: P10.1 and supervisor; Chitti attenuation, snarkOS roles, NexiOS bounds; reject universal daemon. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P10.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P10.2.03 — I/O and state — roles, subagents and resource budgets**
  - Action: parent plan/grants/deadline/CPU-memory-output limits in; subagent process/result out; process lifecycle plus grant derivation.
  - Requires: C-P10.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P10.2.
- [ ] **C-P10.2.04 — Invariants/failure — roles, subagents and resource budgets**
  - Action: child grants derive at commit from current live parent handles; spawn/revocation serialized; child subset only; no grant widening or authority resurrection; parent/child/root-orchestrator termination makes the task dead and every descendant handle stale; deadline/cancel/revoke terminal; quota exhaustion explicit.
  - Requires: C-P10.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P10.2.
- [ ] **C-P10.2.05 — Deterministic proof — roles, subagents and resource budgets**
  - Action: revoke-then-spawn, kill root orchestrator with no orphan root-cap task, widening attempts, nested depth/count, deadline, output/memory/queue exhaustion, parent/child crash and revocation races.
  - Requires: C-P10.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P10.2.
- [ ] **C-P10.2.06 — Target proof — roles, subagents and resource budgets**
  - Action: QEMU multi-agent task under enforced budgets; desktop remains responsive.
  - Requires: C-P10.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P10.2.
- [ ] **C-P10.2.07 — Receipt/removal — roles, subagents and resource budgets**
  - Action: role/grant/resource/outcome tree; single-process execution rollback; no unbounded mode in production.
  - Requires: C-P10.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P10.2.
- [ ] **C-P10.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P10.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P10.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p10-3"></a>
## C-P10.3 — signed skills/plugins

**Original requirement:** signed skills/plugins

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 554.

### Preserved original contract

- **Dependencies/current/provenance:** package service and P10.1; Chitti's separated local-integrity/public-index/P-256 concepts and snarkOS plugin lifecycle; reject Chitti's baked-key MAC mislabeled Ed25519, unfinished foreign ingestion, unsigned dynamic code and crash-propagating callbacks.
- **I/O and state:** signed package, requested ceiling and versioned callbacks in; installed/loaded plugin handle out; package plus service lifecycle.
- **Invariants/failure:** publisher authenticity and package integrity are distinct; user approval; granted intersection only; staged install atomically publishes body/assets/tools/role/index/record or rolls all back; uninstall removes every artifact and registration; bounded callbacks; reverse shutdown; reload unsupported until proved.
- **Deterministic proof:** mislabeled algorithm, local-MAC versus publisher-signature cases, bad signature, excessive grants, failure injection at every install/persist/register step, uninstall residue scan, callback timeout/crash, load/unload loops, version mismatch, dependency failure.
- **Target proof:** QEMU installs/uses/removes harmless skill; supervisor survives plugin fault.
- **Receipt/removal:** package/grant/lifecycle/tool results; disable/unload rollback; old version removed only after no active plan references it.

### Execution steps

- [ ] **C-P10.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P10.3.02 — Resolve this contract's exact dependencies**
  - Action: package service and P10.1; Chitti's separated local-integrity/public-index/P-256 concepts and snarkOS plugin lifecycle; reject Chitti's baked-key MAC mislabeled Ed25519, unfinished foreign ingestion, unsigned dynamic code and crash-propagating callbacks. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P10.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P10.3.03 — I/O and state — signed skills/plugins**
  - Action: signed package, requested ceiling and versioned callbacks in; installed/loaded plugin handle out; package plus service lifecycle.
  - Requires: C-P10.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P10.3.
- [ ] **C-P10.3.04 — Invariants/failure — signed skills/plugins**
  - Action: publisher authenticity and package integrity are distinct; user approval; granted intersection only; staged install atomically publishes body/assets/tools/role/index/record or rolls all back; uninstall removes every artifact and registration; bounded callbacks; reverse shutdown; reload unsupported until proved.
  - Requires: C-P10.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P10.3.
- [ ] **C-P10.3.05 — Deterministic proof — signed skills/plugins**
  - Action: mislabeled algorithm, local-MAC versus publisher-signature cases, bad signature, excessive grants, failure injection at every install/persist/register step, uninstall residue scan, callback timeout/crash, load/unload loops, version mismatch, dependency failure.
  - Requires: C-P10.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P10.3.
- [ ] **C-P10.3.06 — Target proof — signed skills/plugins**
  - Action: QEMU installs/uses/removes harmless skill; supervisor survives plugin fault.
  - Requires: C-P10.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P10.3.
- [ ] **C-P10.3.07 — Receipt/removal — signed skills/plugins**
  - Action: package/grant/lifecycle/tool results; disable/unload rollback; old version removed only after no active plan references it.
  - Requires: C-P10.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P10.3.
- [ ] **C-P10.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P10.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P10.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p10-4"></a>
## C-P10.4 — provenance, taint and privacy

**Original requirement:** provenance, taint and privacy

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 563.

### Preserved original contract

- **Dependencies/current/provenance:** P10.1 plus Logger/File/Network sinks; Chitti taint/citation with honest limits; reject claim of complete IFC.
- **I/O and state:** labelled values/artifacts and sink policy in; allowed/denied effect plus explanation/citation metadata out.
- **Invariants/failure:** labels cannot be silently dropped; policy version recorded; secrets/private content excluded from logs; ambiguous flow fails closed where policy demands.
- **Deterministic proof:** source-label combinations, transform/merge, declassification approval, file/network/clipboard sinks, log redaction, citation mismatch.
- **Target proof:** QEMU agent cannot exfiltrate labelled local data through ungranted sink.
- **Receipt/removal:** labels/policy/decision without content; manual explicit declassification rollback; never remove privacy boundary.

### Execution steps

- [ ] **C-P10.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P10.4.02 — Resolve this contract's exact dependencies**
  - Action: P10.1 plus Logger/File/Network sinks; Chitti taint/citation with honest limits; reject claim of complete IFC. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P10.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P10.4.03 — I/O and state — provenance, taint and privacy**
  - Action: labelled values/artifacts and sink policy in; allowed/denied effect plus explanation/citation metadata out.
  - Requires: C-P10.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P10.4.
- [ ] **C-P10.4.04 — Invariants/failure — provenance, taint and privacy**
  - Action: labels cannot be silently dropped; policy version recorded; secrets/private content excluded from logs; ambiguous flow fails closed where policy demands.
  - Requires: C-P10.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P10.4.
- [ ] **C-P10.4.05 — Deterministic proof — provenance, taint and privacy**
  - Action: source-label combinations, transform/merge, declassification approval, file/network/clipboard sinks, log redaction, citation mismatch.
  - Requires: C-P10.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P10.4.
- [ ] **C-P10.4.06 — Target proof — provenance, taint and privacy**
  - Action: QEMU agent cannot exfiltrate labelled local data through ungranted sink.
  - Requires: C-P10.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P10.4.
- [ ] **C-P10.4.07 — Receipt/removal — provenance, taint and privacy**
  - Action: labels/policy/decision without content; manual explicit declassification rollback; never remove privacy boundary.
  - Requires: C-P10.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P10.4.
- [ ] **C-P10.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P10.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P10.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p10-5"></a>
## C-P10.5 — independent completion reviewer

**Original requirement:** independent completion reviewer

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 572.

### Preserved original contract

- **Dependencies/current/provenance:** universal receipts and P10 plans; RustOS independent evidence; reject executing agent self-certification.
- **I/O and state:** intent/contract/diff/artifacts/receipts in; accept/reject/findings and missing proof out; `Pending -> Challenging -> Accepted|Rejected`.
- **Invariants/failure:** reviewer cannot mutate execution evidence; planted contradiction must be reported; skipped scope remains visible; human override recorded.
- **Deterministic proof:** false success, stale artifact, missing gate, inconsistent hash, model-only proof, planted unauthorized change.
- **Target proof:** QEMU scenario deliberately omits/forges one result and reviewer rejects it.
- **Receipt/removal:** review input/output/model/version; manual review remains fallback; automated review never becomes sole security approval.

### Execution steps

- [ ] **C-P10.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P10.5.02 — Resolve this contract's exact dependencies**
  - Action: universal receipts and P10 plans; RustOS independent evidence; reject executing agent self-certification. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P10.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P10.5.03 — I/O and state — independent completion reviewer**
  - Action: intent/contract/diff/artifacts/receipts in; accept/reject/findings and missing proof out; `Pending -> Challenging -> Accepted|Rejected`.
  - Requires: C-P10.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P10.5.
- [ ] **C-P10.5.04 — Invariants/failure — independent completion reviewer**
  - Action: reviewer cannot mutate execution evidence; planted contradiction must be reported; skipped scope remains visible; human override recorded.
  - Requires: C-P10.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P10.5.
- [ ] **C-P10.5.05 — Deterministic proof — independent completion reviewer**
  - Action: false success, stale artifact, missing gate, inconsistent hash, model-only proof, planted unauthorized change.
  - Requires: C-P10.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P10.5.
- [ ] **C-P10.5.06 — Target proof — independent completion reviewer**
  - Action: QEMU scenario deliberately omits/forges one result and reviewer rejects it.
  - Requires: C-P10.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P10.5.
- [ ] **C-P10.5.07 — Receipt/removal — independent completion reviewer**
  - Action: review input/output/model/version; manual review remains fallback; automated review never becomes sole security approval.
  - Requires: C-P10.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P10.5.
- [ ] **C-P10.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P10.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P10.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-35"></a>
## C-DA-35 — agent and automation applications

**Original requirement:** agent and automation applications

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 839.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-35.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-35.02 — Resolve this contract's exact dependencies**
  - Action: safe ordinary app model, DA-18/19/23/24/26. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-35.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-35.03 — Deliver — agent and automation applications**
  - Action: isolated model/orchestrator process, small deterministic tool broker, compiled typed plans, consent, provenance, bounded jobs, attenuated child handles, deadlines/revocation and independent result receipts.
  - Requires: C-DA-35.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-35.
- [ ] **C-DA-35.04 — Invariants — agent and automation applications**
  - Action: no ring-0 orchestrator; child grants derive at commit from live parent handles; revoke serializes with spawn; killing parent/root makes descendants dead and every handle stale; plugins/tools cannot gain undeclared authority.
  - Requires: C-DA-35.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-35.
- [ ] **C-DA-35.05 — Proof — agent and automation applications**
  - Action: revoke-then-spawn, concurrent spawn/revoke, parent/root kill, orphan search, prompt/tool injection, deadline, queue/memory exhaustion, bad tool package, partial effect, consent denial and independent reviewer contradiction.
  - Requires: C-DA-35.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-35.
- [ ] **C-DA-35.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-35. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-35.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-46"></a>
## C-VX-46 — Agent and automation experience

**Original requirement:** Agent and automation experience

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 706.

### Preserved original contract

**Depends on:** VX-16, VX-18, tool broker and capability system.

**Deliver:** visible plan, requested grants, child tasks, progress, receipts,
cancel/revoke, rollback and independent review; no ring-0 orchestrator.

**Invariants:** child authority derives from live parent handles at commit;
revoke serialized with spawn; termination makes every handle stale; model text
cannot bypass broker policy.

**Proof:** revoke-then-spawn, kill root orchestrator, stale manifest grant,
tool timeout/partial failure, consent expiry and no orphan privileged child.

### Execution steps

- [ ] **C-VX-46.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-46.02 — Resolve this contract's exact dependencies**
  - Action: VX-16, VX-18, tool broker and capability system. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-46.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-46.03 — Deliver — Agent and automation experience**
  - Action: visible plan, requested grants, child tasks, progress, receipts, cancel/revoke, rollback and independent review; no ring-0 orchestrator.
  - Requires: C-VX-46.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-46.
- [ ] **C-VX-46.04 — Invariants — Agent and automation experience**
  - Action: child authority derives from live parent handles at commit; revoke serialized with spawn; termination makes every handle stale; model text cannot bypass broker policy.
  - Requires: C-VX-46.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-46.
- [ ] **C-VX-46.05 — Proof — Agent and automation experience**
  - Action: revoke-then-spawn, kill root orchestrator, stale manifest grant, tool timeout/partial failure, consent expiry and no orphan privileged child.
  - Requires: C-VX-46.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-46.
- [ ] **C-VX-46.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-46. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-46.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-svc-095"></a>
## T-SVC-095 — Speech Recognition Service

**Original requirement:** opt-in audio, model identity, privacy and cancellation

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 117.

### Execution steps

- [ ] **T-SVC-095.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Speech Recognition Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-095.02 — Specify the complete target boundary**
  - Action: Speech Recognition Service must supply: opt-in audio, model identity, privacy and cancellation. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-095.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-095.03 — Implement the exact target behavior**
  - Action: Implement or reuse Speech Recognition Service through the shared platform contract, delivering every part of: opt-in audio, model identity, privacy and cancellation. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-095.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-095.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-SVC-095.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Speech Recognition Service.
- [ ] **T-SVC-095.05 — Qualify and retain this target's own result**
  - Action: Bind Speech Recognition Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-095.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-130"></a>
## T-SVC-130 — Agent Orchestrator

**Original requirement:** typed plans outside ring 0, lifecycle and budgets

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 157.

### Execution steps

- [ ] **T-SVC-130.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Agent Orchestrator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-130.02 — Specify the complete target boundary**
  - Action: Agent Orchestrator must supply: typed plans outside ring 0, lifecycle and budgets. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-130.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-130.03 — Implement the exact target behavior**
  - Action: Implement or reuse Agent Orchestrator through the shared platform contract, delivering every part of: typed plans outside ring 0, lifecycle and budgets. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-130.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-130.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-SVC-130.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Agent Orchestrator.
- [ ] **T-SVC-130.05 — Qualify and retain this target's own result**
  - Action: Bind Agent Orchestrator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-130.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-131"></a>
## T-SVC-131 — Tool Broker

**Original requirement:** deterministic authorized operations and receipts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 158.

### Execution steps

- [ ] **T-SVC-131.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Tool Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-131.02 — Specify the complete target boundary**
  - Action: Tool Broker must supply: deterministic authorized operations and receipts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-131.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-131.03 — Implement the exact target behavior**
  - Action: Implement or reuse Tool Broker through the shared platform contract, delivering every part of: deterministic authorized operations and receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-131.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-131.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-SVC-131.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Tool Broker.
- [ ] **T-SVC-131.05 — Qualify and retain this target's own result**
  - Action: Bind Tool Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-131.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-132"></a>
## T-SVC-132 — Skill/Plugin Manager

**Original requirement:** signed transactional install/revoke/uninstall

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 159.

### Execution steps

- [ ] **T-SVC-132.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Skill/Plugin Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-132.02 — Specify the complete target boundary**
  - Action: Skill/Plugin Manager must supply: signed transactional install/revoke/uninstall. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-132.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-132.03 — Implement the exact target behavior**
  - Action: Implement or reuse Skill/Plugin Manager through the shared platform contract, delivering every part of: signed transactional install/revoke/uninstall. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-132.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-132.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-SVC-132.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Skill/Plugin Manager.
- [ ] **T-SVC-132.05 — Qualify and retain this target's own result**
  - Action: Bind Skill/Plugin Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-132.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-133"></a>
## T-SVC-133 — Automation/Scheduler Service

**Original requirement:** user-owned workflows, triggers, cancellation and history

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 160.

### Execution steps

- [ ] **T-SVC-133.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Automation/Scheduler Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-133.02 — Specify the complete target boundary**
  - Action: Automation/Scheduler Service must supply: user-owned workflows, triggers, cancellation and history. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-133.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-133.03 — Implement the exact target behavior**
  - Action: Implement or reuse Automation/Scheduler Service through the shared platform contract, delivering every part of: user-owned workflows, triggers, cancellation and history. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-133.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-133.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-SVC-133.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Automation/Scheduler Service.
- [ ] **T-SVC-133.05 — Qualify and retain this target's own result**
  - Action: Bind Automation/Scheduler Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-133.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-134"></a>
## T-SVC-134 — Independent Review Service

**Original requirement:** locked criteria and reject-capable evidence review

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 161.

### Execution steps

- [ ] **T-SVC-134.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Independent Review Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-134.02 — Specify the complete target boundary**
  - Action: Independent Review Service must supply: locked criteria and reject-capable evidence review. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-134.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-134.03 — Implement the exact target behavior**
  - Action: Implement or reuse Independent Review Service through the shared platform contract, delivering every part of: locked criteria and reject-capable evidence review. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-134.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-134.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-SVC-134.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Independent Review Service.
- [ ] **T-SVC-134.05 — Qualify and retain this target's own result**
  - Action: Bind Independent Review Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-134.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-090"></a>
## T-APP-090 — Agent/Automation Center

**Original requirement:** plans, tools, live grants, children, progress, receipts and cancel

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 219.

### Execution steps

- [ ] **T-APP-090.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Agent/Automation Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-090.02 — Specify the complete target boundary**
  - Action: Agent/Automation Center must supply: plans, tools, live grants, children, progress, receipts and cancel. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-090.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-090.03 — Implement the exact target behavior**
  - Action: Implement or reuse Agent/Automation Center through the shared platform contract, delivering every part of: plans, tools, live grants, children, progress, receipts and cancel. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-090.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-090.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-APP-090.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Agent/Automation Center.
- [ ] **T-APP-090.05 — Qualify and retain this target's own result**
  - Action: Bind Agent/Automation Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-090.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-091"></a>
## T-APP-091 — Workflow Editor

**Original requirement:** triggers, typed steps, conditions, budgets, history and rollback

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 220.

### Execution steps

- [ ] **T-APP-091.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Workflow Editor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-091.02 — Specify the complete target boundary**
  - Action: Workflow Editor must supply: triggers, typed steps, conditions, budgets, history and rollback. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-091.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-091.03 — Implement the exact target behavior**
  - Action: Implement or reuse Workflow Editor through the shared platform contract, delivering every part of: triggers, typed steps, conditions, budgets, history and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-091.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-091.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-APP-091.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Workflow Editor.
- [ ] **T-APP-091.05 — Qualify and retain this target's own result**
  - Action: Bind Workflow Editor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-091.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-092"></a>
## T-APP-092 — Scheduled Tasks

**Original requirement:** user-owned schedules, next runs, failures and disable/delete

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 221.

### Execution steps

- [ ] **T-APP-092.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Scheduled Tasks to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-092.02 — Specify the complete target boundary**
  - Action: Scheduled Tasks must supply: user-owned schedules, next runs, failures and disable/delete. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-092.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-092.03 — Implement the exact target behavior**
  - Action: Implement or reuse Scheduled Tasks through the shared platform contract, delivering every part of: user-owned schedules, next runs, failures and disable/delete. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-092.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-092.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-APP-092.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Scheduled Tasks.
- [ ] **T-APP-092.05 — Qualify and retain this target's own result**
  - Action: Bind Scheduled Tasks to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-092.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-093"></a>
## T-APP-093 — Skill/Plugin Manager

**Original requirement:** trust, permissions, install/update/revoke/uninstall

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 222.

### Execution steps

- [ ] **T-APP-093.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Skill/Plugin Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-093.02 — Specify the complete target boundary**
  - Action: Skill/Plugin Manager must supply: trust, permissions, install/update/revoke/uninstall. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-093.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-093.03 — Implement the exact target behavior**
  - Action: Implement or reuse Skill/Plugin Manager through the shared platform contract, delivering every part of: trust, permissions, install/update/revoke/uninstall. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-093.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-093.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-APP-093.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Skill/Plugin Manager.
- [ ] **T-APP-093.05 — Qualify and retain this target's own result**
  - Action: Bind Skill/Plugin Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-093.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-001"></a>
## T-AGT-001 — typed intent/plan schema

**Original requirement:** versioned goals, steps, dependencies, limits and expected evidence

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 109.

### Execution steps

- [ ] **T-AGT-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve typed intent/plan schema to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-001.02 — Specify the complete target boundary**
  - Action: typed intent/plan schema must supply: versioned goals, steps, dependencies, limits and expected evidence. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse typed intent/plan schema through the shared platform contract, delivering every part of: versioned goals, steps, dependencies, limits and expected evidence. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-001.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for typed intent/plan schema.
- [ ] **T-AGT-001.05 — Qualify and retain this target's own result**
  - Action: Bind typed intent/plan schema to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-002"></a>
## T-AGT-002 — deterministic tool broker

**Original requirement:** declared inputs/effects, authority checks, idempotence and receipts

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 110.

### Execution steps

- [ ] **T-AGT-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve deterministic tool broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-002.02 — Specify the complete target boundary**
  - Action: deterministic tool broker must supply: declared inputs/effects, authority checks, idempotence and receipts. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse deterministic tool broker through the shared platform contract, delivering every part of: declared inputs/effects, authority checks, idempotence and receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-002.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for deterministic tool broker.
- [ ] **T-AGT-002.05 — Qualify and retain this target's own result**
  - Action: Bind deterministic tool broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-003"></a>
## T-AGT-003 — attenuated subagents

**Original requirement:** live parent-handle derivation, budgets and no authority resurrection

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 111.

### Execution steps

- [ ] **T-AGT-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve attenuated subagents to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-003.02 — Specify the complete target boundary**
  - Action: attenuated subagents must supply: live parent-handle derivation, budgets and no authority resurrection. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse attenuated subagents through the shared platform contract, delivering every part of: live parent-handle derivation, budgets and no authority resurrection. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-003.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for attenuated subagents.
- [ ] **T-AGT-003.05 — Qualify and retain this target's own result**
  - Action: Bind attenuated subagents to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-004"></a>
## T-AGT-004 — cancellation and parent-death

**Original requirement:** task dead, every handle stale, no orphan root-cap process

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 112.

### Execution steps

- [ ] **T-AGT-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve cancellation and parent-death to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-004.02 — Specify the complete target boundary**
  - Action: cancellation and parent-death must supply: task dead, every handle stale, no orphan root-cap process. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse cancellation and parent-death through the shared platform contract, delivering every part of: task dead, every handle stale, no orphan root-cap process. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-004.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for cancellation and parent-death.
- [ ] **T-AGT-004.05 — Qualify and retain this target's own result**
  - Action: Bind cancellation and parent-death to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-005"></a>
## T-AGT-005 — resource budgets

**Original requirement:** CPU/memory/time/I/O/network/tool/subtask ceilings and exhaustion result

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 113.

### Execution steps

- [ ] **T-AGT-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve resource budgets to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-005.02 — Specify the complete target boundary**
  - Action: resource budgets must supply: CPU/memory/time/I/O/network/tool/subtask ceilings and exhaustion result. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse resource budgets through the shared platform contract, delivering every part of: CPU/memory/time/I/O/network/tool/subtask ceilings and exhaustion result. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-005.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for resource budgets.
- [ ] **T-AGT-005.05 — Qualify and retain this target's own result**
  - Action: Bind resource budgets to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-006"></a>
## T-AGT-006 — approval policy

**Original requirement:** risk-classified effects, explicit user decision and expiring grants

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 114.

### Execution steps

- [ ] **T-AGT-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve approval policy to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-006.02 — Specify the complete target boundary**
  - Action: approval policy must supply: risk-classified effects, explicit user decision and expiring grants. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse approval policy through the shared platform contract, delivering every part of: risk-classified effects, explicit user decision and expiring grants. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-006.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for approval policy.
- [ ] **T-AGT-006.05 — Qualify and retain this target's own result**
  - Action: Bind approval policy to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-007"></a>
## T-AGT-007 — secret mediation

**Original requirement:** scoped non-readable use where possible, redaction and zeroing

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 115.

### Execution steps

- [ ] **T-AGT-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve secret mediation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-007.02 — Specify the complete target boundary**
  - Action: secret mediation must supply: scoped non-readable use where possible, redaction and zeroing. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse secret mediation through the shared platform contract, delivering every part of: scoped non-readable use where possible, redaction and zeroing. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-007.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for secret mediation.
- [ ] **T-AGT-007.05 — Qualify and retain this target's own result**
  - Action: Bind secret mediation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-008"></a>
## T-AGT-008 — provenance/taint graph

**Original requirement:** input/source/effect/derivation and privacy propagation

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 116.

### Execution steps

- [ ] **T-AGT-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve provenance/taint graph to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-008.02 — Specify the complete target boundary**
  - Action: provenance/taint graph must supply: input/source/effect/derivation and privacy propagation. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse provenance/taint graph through the shared platform contract, delivering every part of: input/source/effect/derivation and privacy propagation. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-008.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for provenance/taint graph.
- [ ] **T-AGT-008.05 — Qualify and retain this target's own result**
  - Action: Bind provenance/taint graph to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-009"></a>
## T-AGT-009 — durable audit receipts

**Original requirement:** actor/authority/input/output/effect with anchoring and drop evidence

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 117.

### Execution steps

- [ ] **T-AGT-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve durable audit receipts to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-009.02 — Specify the complete target boundary**
  - Action: durable audit receipts must supply: actor/authority/input/output/effect with anchoring and drop evidence. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse durable audit receipts through the shared platform contract, delivering every part of: actor/authority/input/output/effect with anchoring and drop evidence. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-009.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for durable audit receipts.
- [ ] **T-AGT-009.05 — Qualify and retain this target's own result**
  - Action: Bind durable audit receipts to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-010"></a>
## T-AGT-010 — signed skill/plugin format

**Original requirement:** asymmetric publisher identity, trust roots, rotation/revocation

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 118.

### Execution steps

- [ ] **T-AGT-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve signed skill/plugin format to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-010.02 — Specify the complete target boundary**
  - Action: signed skill/plugin format must supply: asymmetric publisher identity, trust roots, rotation/revocation. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse signed skill/plugin format through the shared platform contract, delivering every part of: asymmetric publisher identity, trust roots, rotation/revocation. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-010.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for signed skill/plugin format.
- [ ] **T-AGT-010.05 — Qualify and retain this target's own result**
  - Action: Bind signed skill/plugin format to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-011"></a>
## T-AGT-011 — transactional skill install

**Original requirement:** stage/verify/admit/publish; exact rollback at every failure

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 119.

### Execution steps

- [ ] **T-AGT-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve transactional skill install to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-011.02 — Specify the complete target boundary**
  - Action: transactional skill install must supply: stage/verify/admit/publish; exact rollback at every failure. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse transactional skill install through the shared platform contract, delivering every part of: stage/verify/admit/publish; exact rollback at every failure. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-011.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for transactional skill install.
- [ ] **T-AGT-011.05 — Qualify and retain this target's own result**
  - Action: Bind transactional skill install to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-012"></a>
## T-AGT-012 — complete uninstall/revoke

**Original requirement:** stop tasks, revoke handles, deregister tools/roles, retain data by policy

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 120.

### Execution steps

- [ ] **T-AGT-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve complete uninstall/revoke to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-012.02 — Specify the complete target boundary**
  - Action: complete uninstall/revoke must supply: stop tasks, revoke handles, deregister tools/roles, retain data by policy. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse complete uninstall/revoke through the shared platform contract, delivering every part of: stop tasks, revoke handles, deregister tools/roles, retain data by policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-012.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for complete uninstall/revoke.
- [ ] **T-AGT-012.05 — Qualify and retain this target's own result**
  - Action: Bind complete uninstall/revoke to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-013"></a>
## T-AGT-013 — tool and role registry

**Original requirement:** generated admitted capabilities and current health

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 121.

### Execution steps

- [ ] **T-AGT-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve tool and role registry to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-013.02 — Specify the complete target boundary**
  - Action: tool and role registry must supply: generated admitted capabilities and current health. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse tool and role registry through the shared platform contract, delivering every part of: generated admitted capabilities and current health. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-013.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for tool and role registry.
- [ ] **T-AGT-013.05 — Qualify and retain this target's own result**
  - Action: Bind tool and role registry to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-014"></a>
## T-AGT-014 — scheduled automation

**Original requirement:** user-owned triggers, deadlines, missed-run policy and history

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 122.

### Execution steps

- [ ] **T-AGT-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve scheduled automation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-014.02 — Specify the complete target boundary**
  - Action: scheduled automation must supply: user-owned triggers, deadlines, missed-run policy and history. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse scheduled automation through the shared platform contract, delivering every part of: user-owned triggers, deadlines, missed-run policy and history. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-014.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for scheduled automation.
- [ ] **T-AGT-014.05 — Qualify and retain this target's own result**
  - Action: Bind scheduled automation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-015"></a>
## T-AGT-015 — desktop semantic automation

**Original requirement:** accessibility/action APIs, no pixel-only ambient control

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 123.

### Execution steps

- [ ] **T-AGT-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve desktop semantic automation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-015.02 — Specify the complete target boundary**
  - Action: desktop semantic automation must supply: accessibility/action APIs, no pixel-only ambient control. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse desktop semantic automation through the shared platform contract, delivering every part of: accessibility/action APIs, no pixel-only ambient control. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-015.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for desktop semantic automation.
- [ ] **T-AGT-015.05 — Qualify and retain this target's own result**
  - Action: Bind desktop semantic automation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-016"></a>
## T-AGT-016 — filesystem/workspace automation

**Original requirement:** file portals, transaction scopes and rollback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 124.

### Execution steps

- [ ] **T-AGT-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve filesystem/workspace automation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-016.02 — Specify the complete target boundary**
  - Action: filesystem/workspace automation must supply: file portals, transaction scopes and rollback. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse filesystem/workspace automation through the shared platform contract, delivering every part of: file portals, transaction scopes and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-016.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for filesystem/workspace automation.
- [ ] **T-AGT-016.05 — Qualify and retain this target's own result**
  - Action: Bind filesystem/workspace automation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-017"></a>
## T-AGT-017 — network/API automation

**Original requirement:** origin/endpoint grants, quotas and credential mediation

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 125.

### Execution steps

- [ ] **T-AGT-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve network/API automation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-017.02 — Specify the complete target boundary**
  - Action: network/API automation must supply: origin/endpoint grants, quotas and credential mediation. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse network/API automation through the shared platform contract, delivering every part of: origin/endpoint grants, quotas and credential mediation. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-017.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for network/API automation.
- [ ] **T-AGT-017.05 — Qualify and retain this target's own result**
  - Action: Bind network/API automation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-018"></a>
## T-AGT-018 — interactive progress/control UI

**Original requirement:** plan, tools, grants, children, evidence, cancel and recovery

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 126.

### Execution steps

- [ ] **T-AGT-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve interactive progress/control UI to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-018.02 — Specify the complete target boundary**
  - Action: interactive progress/control UI must supply: plan, tools, grants, children, evidence, cancel and recovery. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse interactive progress/control UI through the shared platform contract, delivering every part of: plan, tools, grants, children, evidence, cancel and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-018.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for interactive progress/control UI.
- [ ] **T-AGT-018.05 — Qualify and retain this target's own result**
  - Action: Bind interactive progress/control UI to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-019"></a>
## T-AGT-019 — independent completion reviewer

**Original requirement:** locked spec, separate evidence access and reject outcome

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 127.

### Execution steps

- [ ] **T-AGT-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve independent completion reviewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-019.02 — Specify the complete target boundary**
  - Action: independent completion reviewer must supply: locked spec, separate evidence access and reject outcome. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse independent completion reviewer through the shared platform contract, delivering every part of: locked spec, separate evidence access and reject outcome. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-019.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for independent completion reviewer.
- [ ] **T-AGT-019.05 — Qualify and retain this target's own result**
  - Action: Bind independent completion reviewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-020"></a>
## T-AGT-020 — evaluation/regression harness

**Original requirement:** golden tasks, adversarial cases, longitudinal reliability

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 128.

### Execution steps

- [ ] **T-AGT-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve evaluation/regression harness to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-020.02 — Specify the complete target boundary**
  - Action: evaluation/regression harness must supply: golden tasks, adversarial cases, longitudinal reliability. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse evaluation/regression harness through the shared platform contract, delivering every part of: golden tasks, adversarial cases, longitudinal reliability. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-020.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for evaluation/regression harness.
- [ ] **T-AGT-020.05 — Qualify and retain this target's own result**
  - Action: Bind evaluation/regression harness to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-021"></a>
## T-AGT-021 — offline/local model provider seam

**Original requirement:** model identity, capability, resource and privacy contract

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 129.

### Execution steps

- [ ] **T-AGT-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve offline/local model provider seam to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-021.02 — Specify the complete target boundary**
  - Action: offline/local model provider seam must supply: model identity, capability, resource and privacy contract. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse offline/local model provider seam through the shared platform contract, delivering every part of: model identity, capability, resource and privacy contract. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-021.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for offline/local model provider seam.
- [ ] **T-AGT-021.05 — Qualify and retain this target's own result**
  - Action: Bind offline/local model provider seam to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-022"></a>
## T-AGT-022 — remote model provider seam

**Original requirement:** endpoint/auth/data policy, cancellation and retention contract

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 130.

### Execution steps

- [ ] **T-AGT-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve remote model provider seam to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-022.02 — Specify the complete target boundary**
  - Action: remote model provider seam must supply: endpoint/auth/data policy, cancellation and retention contract. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse remote model provider seam through the shared platform contract, delivering every part of: endpoint/auth/data policy, cancellation and retention contract. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-022.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for remote model provider seam.
- [ ] **T-AGT-022.05 — Qualify and retain this target's own result**
  - Action: Bind remote model provider seam to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-023"></a>
## T-AGT-023 — human handoff/continuation

**Original requirement:** durable state, unresolved decisions, weakest evidence and replay

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 131.

### Execution steps

- [ ] **T-AGT-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve human handoff/continuation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-023.02 — Specify the complete target boundary**
  - Action: human handoff/continuation must supply: durable state, unresolved decisions, weakest evidence and replay. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse human handoff/continuation through the shared platform contract, delivering every part of: durable state, unresolved decisions, weakest evidence and replay. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-023.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for human handoff/continuation.
- [ ] **T-AGT-023.05 — Qualify and retain this target's own result**
  - Action: Bind human handoff/continuation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-agt-024"></a>
## T-AGT-024 — agent recovery/checkpoint

**Original requirement:** safe restart without replaying already committed effects

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 132.

### Execution steps

- [ ] **T-AGT-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve agent recovery/checkpoint to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-23, H-17.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-AGT-024.02 — Specify the complete target boundary**
  - Action: agent recovery/checkpoint must supply: safe restart without replaying already committed effects. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-AGT-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-AGT-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse agent recovery/checkpoint through the shared platform contract, delivering every part of: safe restart without replaying already committed effects. Do not fork a duplicate subsystem for this row.
  - Requires: T-AGT-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-AGT-024.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: untrusted instruction in tool output; revoked child authority; parent kill; exhausted budget; forged completion; secret leak; interrupted plugin install; cancelled effects; restart recovery.
  - Requires: T-AGT-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for agent recovery/checkpoint.
- [ ] **T-AGT-024.05 — Qualify and retain this target's own result**
  - Action: Bind agent recovery/checkpoint to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-AGT-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
