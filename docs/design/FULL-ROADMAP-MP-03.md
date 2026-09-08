# MP-03: Make independent programs a reliable kernel facility

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/arch/x86/; kernel/src/core/; freestanding/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-03` exports: Bounded isolated execution, safe copy, ownership, timer scheduling and termination custody; credentials and advanced mappings join later.

The handoff enables only its named subset. `CLOSE-03` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-03.01 — Reconcile typed frame/heap ownership, address layout and page-table rollback with the current allocator and mapping tests

Reconcile typed frame/heap ownership, address layout and page-table rollback with the current allocator and mapping tests.

**Requires:** `D-01`, `D-02`, `D-05`, `D-06`, `D-07`, `H-00`, `H-01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.02 — Close all entry/return state contracts: stack, interrupt frame, CPU flags, FP/vector controls, privilege and address-space selection

Close all entry/return state contracts: stack, interrupt frame, CPU flags, FP/vector controls, privilege and address-space selection.

**Requires:** `M-03.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.03 — Separate child image preparation from active process selection

Separate child image preparation from active process selection; execute the detailed spawn/wait checklist in FULL-ROADMAP-NEXT-PROCESS.md.

**Requires:** `M-03.02`, `N-PROCESS.32`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-03 — Bounded development handoff: Make independent programs a reliable kernel facility

Bounded isolated execution, safe copy, ownership, timer scheduling and termination custody; credentials and advanced mappings join later.

**Requires:** `M-03.03`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-03.04 — Define stable executable and syscall ABI contracts, exact errors, full-width handles, layouts and version rejection

Define stable executable and syscall ABI contracts, exact errors, full-width handles, layouts and version rejection.

**Requires:** `M-03.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.05 — Generalize executable admission and exec replacement only after rollback and parent-state preservation are proved

Generalize executable admission and exec replacement only after rollback and parent-state preservation are proved.

**Requires:** `M-03.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.06 — Add thread stacks, TLS, synchronization and process/thread death semantics before adding concurrent mutators

Add thread stacks, TLS, synchronization and process/thread death semantics before adding concurrent mutators.

**Requires:** `M-03.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.07 — Extend scheduling to per-CPU ownership, fairness, priorities, migration, deadlines and bounded real-time admission

Extend scheduling to per-CPU ownership, fairness, priorities, migration, deadlines and bounded real-time admission.

**Requires:** `M-03.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.08 — Join shared objects, COW and file-backed mappings with authority and VFS handoffs

Join shared objects, COW and file-backed mappings with authority and VFS handoffs; keep their final completion gates open until those joins pass.

**Requires:** `M-03.07`, `H-04`, `H-06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.09 — Implement memory accounting, limits, pressure, reclaim and explicit OOM selection across every consumer class

Implement memory accounting, limits, pressure, reclaim and explicit OOM selection across every consumer class.

**Requires:** `M-03.08`, `H-04`, `H-05`, `H-06`, `H-07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.10 — Integrate signals, groups, sessions, jobs, core dumps, diagnostics and orderly shutdown through explicit authority

Integrate signals, groups, sessions, jobs, core dumps, diagnostics and orderly shutdown through explicit authority.

**Requires:** `M-03.09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-03.11 — Stress stale identities, every allocation/mapping failure, concurrent faults and CPU offline

Stress stale identities, every allocation/mapping failure, concurrent faults and CPU offline; retain exact resource baselines.

**Requires:** `M-03.10`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-KR-001](#f-kr-001) | feature | physical page allocator |
| [F-KR-002](#f-kr-002) | feature | kernel heap |
| [F-KR-003](#f-kr-003) | feature | virtual address layout |
| [F-KR-004](#f-kr-004) | feature | page-table construction |
| [F-KR-005](#f-kr-005) | feature | per-process address space |
| [F-KR-006](#f-kr-006) | feature | user/kernel privilege boundary |
| [F-KR-007](#f-kr-007) | feature | safe user copy |
| [F-KR-008](#f-kr-008) | feature | NX/W^X |
| [F-KR-009](#f-kr-009) | feature | guard pages |
| [F-KR-010](#f-kr-010) | feature | anonymous memory |
| [F-KR-011](#f-kr-011) | feature | file-backed mapping |
| [F-KR-012](#f-kr-012) | feature | copy-on-write |
| [F-KR-013](#f-kr-013) | feature | shared-memory object |
| [F-KR-014](#f-kr-014) | feature | page/file cache |
| [F-KR-015](#f-kr-015) | feature | memory accounting |
| [F-KR-016](#f-kr-016) | feature | memory pressure policy |
| [F-KR-017](#f-kr-017) | feature | kernel stack management |
| [F-KR-018](#f-kr-018) | feature | process creation |
| [F-KR-019](#f-kr-019) | feature | fork/clone semantics |
| [F-KR-020](#f-kr-020) | feature | exec semantics |
| [F-KR-021](#f-kr-021) | feature | thread lifecycle |
| [F-KR-022](#f-kr-022) | feature | scheduler |
| [F-KR-023](#f-kr-023) | feature | real-time scheduling |
| [F-KR-024](#f-kr-024) | feature | timer/deadline API |
| [F-KR-025](#f-kr-025) | feature | signals/events |
| [F-KR-026](#f-kr-026) | feature | process groups/sessions/jobs |
| [F-KR-027](#f-kr-027) | feature | wait/exit status |
| [F-KR-028](#f-kr-028) | feature | process handles |
| [F-KR-029](#f-kr-029) | feature | resource limits |
| [F-KR-030](#f-kr-030) | feature | namespace/container boundary |
| [F-KR-031](#f-kr-031) | feature | exception/fault containment |
| [F-KR-032](#f-kr-032) | feature | interrupt/exception architecture |
| [F-KR-033](#f-kr-033) | feature | TLB shootdown |
| [F-KR-034](#f-kr-034) | feature | kernel synchronization |
| [F-KR-035](#f-kr-035) | feature | random number service |
| [F-KR-036](#f-kr-036) | feature | syscall ABI |
| [F-KR-037](#f-kr-037) | feature | stable userspace ABI |
| [F-KR-038](#f-kr-038) | feature | core dump |
| [F-KR-039](#f-kr-039) | feature | kernel live diagnostics |
| [F-KR-040](#f-kr-040) | feature | reboot/shutdown |
| [C-P3.1](#c-p3-1) | contract | syscall entry, kernel stack and return |
| [C-P3.2](#c-p3-2) | contract | process object and address space |
| [C-P3.3](#c-p3-3) | contract | central usercopy and fault containment |
| [C-P3.4](#c-p3-4) | contract | ELF admission and minimal zl process |
| [C-DA-17](#c-da-17) | contract | hostile user boundary |
| [C-DA-17L](#c-da-17l) | contract | hostile executable loader |
| [T-PLAT-011](#t-plat-011) | target | x86 topology/SMP provider |
| [T-PLAT-023](#t-plat-023) | target | TSC/deadline timer |

<a id="f-kr-001"></a>
## F-KR-001 — physical page allocator

**Original requirement:** typed ownership, reserved-map proof, exhaustion, zero/reuse, and double-free detection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** bounded to the first 1 GiB and pages at or above 320 MiB; production accounting covers only the fixed two-slot UEFI64 process diagnostic and its bounded anonymous window; no concurrent or interrupt-context locking contract; Multiboot has an exact golden boot gate but no dedicated allocator receipt; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-001.01 — Reconcile existing physical page allocator**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for physical page allocator. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed ownership, reserved-map proof, exhaustion, zero/reuse, and double-free detection
- [ ] **F-KR-001.02 — Freeze the exact contract for physical page allocator**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed ownership, reserved-map proof, exhaustion, zero/reuse, and double-free detection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-001.03 — Implement/prove: typed ownership**
  - Action: For physical page allocator, implement or reuse and verify this exact obligation: typed ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed ownership; retain observable state/resource expectations.
- [ ] **F-KR-001.04 — Implement/prove: reserved-map proof**
  - Action: For physical page allocator, implement or reuse and verify this exact obligation: reserved-map proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reserved-map proof; retain observable state/resource expectations.
- [ ] **F-KR-001.05 — Implement/prove: exhaustion**
  - Action: For physical page allocator, implement or reuse and verify this exact obligation: exhaustion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exhaustion; retain observable state/resource expectations.
- [ ] **F-KR-001.06 — Implement/prove: zero/reuse**
  - Action: For physical page allocator, implement or reuse and verify this exact obligation: zero/reuse. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for zero/reuse; retain observable state/resource expectations.
- [ ] **F-KR-001.07 — Implement/prove: double-free detection**
  - Action: For physical page allocator, implement or reuse and verify this exact obligation: double-free detection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for double-free detection; retain observable state/resource expectations.
- [ ] **F-KR-001.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to physical page allocator: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-001.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-001.09 — Integrate into the real consumer and runtime route**
  - Action: Wire physical page allocator into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-001.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-001.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for physical page allocator as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-001.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-002"></a>
## F-KR-002 — kernel heap

**Original requirement:** checked size/alignment, explicit OOM, tags, poisoning, and failure injection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** poison covers an 8-byte guard rather than every freed payload byte; tags are diagnostic integers rather than an ownership policy; failure injection covers allocation only; host execution does not prove target concurrency or interrupt safety; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-002.01 — Reconcile existing kernel heap**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kernel heap. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked size/alignment, explicit OOM, tags, poisoning, and failure injection
- [ ] **F-KR-002.02 — Freeze the exact contract for kernel heap**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked size/alignment, explicit OOM, tags, poisoning, and failure injection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-002.03 — Implement/prove: checked size/alignment**
  - Action: For kernel heap, implement or reuse and verify this exact obligation: checked size/alignment. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked size/alignment; retain observable state/resource expectations.
- [ ] **F-KR-002.04 — Implement/prove: explicit OOM**
  - Action: For kernel heap, implement or reuse and verify this exact obligation: explicit OOM. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit OOM; retain observable state/resource expectations.
- [ ] **F-KR-002.05 — Implement/prove: tags**
  - Action: For kernel heap, implement or reuse and verify this exact obligation: tags. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tags; retain observable state/resource expectations.
- [ ] **F-KR-002.06 — Implement/prove: poisoning**
  - Action: For kernel heap, implement or reuse and verify this exact obligation: poisoning. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-002.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for poisoning; retain observable state/resource expectations.
- [ ] **F-KR-002.07 — Implement/prove: failure injection**
  - Action: For kernel heap, implement or reuse and verify this exact obligation: failure injection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-002.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failure injection; retain observable state/resource expectations.
- [ ] **F-KR-002.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kernel heap: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-002.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-002.09 — Integrate into the real consumer and runtime route**
  - Action: Wire kernel heap into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-002.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-002.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kernel heap as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-002.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-003"></a>
## F-KR-003 — virtual address layout

**Original requirement:** generated non-overlap manifest for kernel, user, devices, stacks, heaps, and shared regions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** the zl-low shared reservation is an envelope rather than an internal-size manifest; dynamic user virtual bases are selected at runtime; the separate KR-004 transaction receipt covers the heap window, not every mapped region; source/build evidence does not prove QEMU or physical execution.

### Execution steps

- [ ] **F-KR-003.01 — Reconcile existing virtual address layout**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for virtual address layout. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated non-overlap manifest for kernel, user, devices, stacks, heaps, and shared regions
- [ ] **F-KR-003.02 — Freeze the exact contract for virtual address layout**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated non-overlap manifest for kernel, user, devices, stacks, heaps, and shared regions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-003.03 — Implement/prove: generated non-overlap manifest for kernel**
  - Action: For virtual address layout, implement or reuse and verify this exact obligation: generated non-overlap manifest for kernel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated non-overlap manifest for kernel; retain observable state/resource expectations.
- [ ] **F-KR-003.04 — Implement/prove: user**
  - Action: For virtual address layout, implement or reuse and verify this exact obligation: user. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user; retain observable state/resource expectations.
- [ ] **F-KR-003.05 — Implement/prove: devices**
  - Action: For virtual address layout, implement or reuse and verify this exact obligation: devices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices; retain observable state/resource expectations.
- [ ] **F-KR-003.06 — Implement/prove: stacks**
  - Action: For virtual address layout, implement or reuse and verify this exact obligation: stacks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stacks; retain observable state/resource expectations.
- [ ] **F-KR-003.07 — Implement/prove: heaps**
  - Action: For virtual address layout, implement or reuse and verify this exact obligation: heaps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-003.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for heaps; retain observable state/resource expectations.
- [ ] **F-KR-003.08 — Implement/prove: shared regions**
  - Action: For virtual address layout, implement or reuse and verify this exact obligation: shared regions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-003.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shared regions; retain observable state/resource expectations.
- [ ] **F-KR-003.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to virtual address layout: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-003.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-003.10 — Integrate into the real consumer and runtime route**
  - Action: Wire virtual address layout into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-003.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-003.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for virtual address layout as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-003.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-004"></a>
## F-KR-004 — page-table construction

**Original requirement:** reserve/validate/apply/flush/commit or exact rollback across full ranges

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** callers must provide exclusive page-table ownership; no general page-table allocator or teardown service; fixed Ring-3 private tables are built before CR3 publication but do not use the journal; future live page-table writers must adopt the transaction core; 32-bit paging remains disabled; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-004.01 — Reconcile existing page-table construction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for page-table construction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reserve/validate/apply/flush/commit or exact rollback across full ranges
- [ ] **F-KR-004.02 — Freeze the exact contract for page-table construction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reserve/validate/apply/flush/commit or exact rollback across full ranges. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-004.03 — Implement/prove: reserve/validate/apply/flush/commit or exact rollback across full ranges**
  - Action: For page-table construction, implement or reuse and verify this exact obligation: reserve/validate/apply/flush/commit or exact rollback across full ranges. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reserve/validate/apply/flush/commit or exact rollback across full ranges; retain observable state/resource expectations.
- [ ] **F-KR-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to page-table construction: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire page-table construction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for page-table construction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-005"></a>
## F-KR-005 — per-process address space

**Original requirement:** unique root, protected kernel half, owned mappings, teardown and switch receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** fixed two-process service; teardown is not exercised under concurrent service load; no userspace spawn/wait/process-handle ABI; no concurrent PID-reuse receipt; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-005.01 — Reconcile existing per-process address space**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for per-process address space. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: unique root, protected kernel half, owned mappings, teardown and switch receipts
- [ ] **F-KR-005.02 — Freeze the exact contract for per-process address space**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: unique root, protected kernel half, owned mappings, teardown and switch receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-005.03 — Implement/prove: unique root**
  - Action: For per-process address space, implement or reuse and verify this exact obligation: unique root. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unique root; retain observable state/resource expectations.
- [ ] **F-KR-005.04 — Implement/prove: protected kernel half**
  - Action: For per-process address space, implement or reuse and verify this exact obligation: protected kernel half. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for protected kernel half; retain observable state/resource expectations.
- [ ] **F-KR-005.05 — Implement/prove: owned mappings**
  - Action: For per-process address space, implement or reuse and verify this exact obligation: owned mappings. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owned mappings; retain observable state/resource expectations.
- [ ] **F-KR-005.06 — Implement/prove: teardown and switch receipts**
  - Action: For per-process address space, implement or reuse and verify this exact obligation: teardown and switch receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for teardown and switch receipts; retain observable state/resource expectations.
- [ ] **F-KR-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to per-process address space: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire per-process address space into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for per-process address space as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-006"></a>
## F-KR-006 — user/kernel privilege boundary

**Original requirement:** ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no SMEP or SMAP enablement receipt; x86-64 route only; no capability-mediated privileged-operation model; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-006.01 — Reconcile existing user/kernel privilege boundary**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for user/kernel privilege boundary. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present
- [ ] **F-KR-006.02 — Freeze the exact contract for user/kernel privilege boundary**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-006.03 — Implement/prove: ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present**
  - Action: For user/kernel privilege boundary, implement or reuse and verify this exact obligation: ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ring 3/EL0 plus SMEP/SMAP/PAN-like protections where present; retain observable state/resource expectations.
- [ ] **F-KR-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to user/kernel privilege boundary: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire user/kernel privilege boundary into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for user/kernel privilege boundary as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-007"></a>
## F-KR-007 — safe user copy

**Original requirement:** canonical, mapped, permission, full-span, bounded-string, and copyout-fault handling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** fixed code/stack ranges instead of arbitrary mapping walks; no demand-fault or copyout-fault recovery; no hostile copy corpus; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-007.01 — Reconcile existing safe user copy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for safe user copy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: canonical, mapped, permission, full-span, bounded-string, and copyout-fault handling
- [ ] **F-KR-007.02 — Freeze the exact contract for safe user copy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: canonical, mapped, permission, full-span, bounded-string, and copyout-fault handling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-007.03 — Implement/prove: canonical**
  - Action: For safe user copy, implement or reuse and verify this exact obligation: canonical. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for canonical; retain observable state/resource expectations.
- [ ] **F-KR-007.04 — Implement/prove: mapped**
  - Action: For safe user copy, implement or reuse and verify this exact obligation: mapped. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mapped; retain observable state/resource expectations.
- [ ] **F-KR-007.05 — Implement/prove: permission**
  - Action: For safe user copy, implement or reuse and verify this exact obligation: permission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission; retain observable state/resource expectations.
- [ ] **F-KR-007.06 — Implement/prove: full-span**
  - Action: For safe user copy, implement or reuse and verify this exact obligation: full-span. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for full-span; retain observable state/resource expectations.
- [ ] **F-KR-007.07 — Implement/prove: bounded-string**
  - Action: For safe user copy, implement or reuse and verify this exact obligation: bounded-string. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-007.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded-string; retain observable state/resource expectations.
- [ ] **F-KR-007.08 — Implement/prove: copyout-fault handling**
  - Action: For safe user copy, implement or reuse and verify this exact obligation: copyout-fault handling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-007.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for copyout-fault handling; retain observable state/resource expectations.
- [ ] **F-KR-007.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to safe user copy: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-007.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-007.10 — Integrate into the real consumer and runtime route**
  - Action: Wire safe user copy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-007.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-007.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for safe user copy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-007.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-008"></a>
## F-KR-008 — NX/W^X

**Original requirement:** no writable executable mapping; controlled authorized transition for JIT if ever supported

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no authorized writable-to-executable transition protocol; no system-wide audit of every dynamic mapping; no SMEP/SMAP enablement receipt; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-008.01 — Reconcile existing NX/W^X**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for NX/W^X. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: no writable executable mapping; controlled authorized transition for JIT if ever supported
- [ ] **F-KR-008.02 — Freeze the exact contract for NX/W^X**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: no writable executable mapping; controlled authorized transition for JIT if ever supported. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-008.03 — Implement/prove: no writable executable mapping**
  - Action: For NX/W^X, implement or reuse and verify this exact obligation: no writable executable mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no writable executable mapping; retain observable state/resource expectations.
- [ ] **F-KR-008.04 — Implement/prove: controlled authorized transition for JIT if ever supported**
  - Action: For NX/W^X, implement or reuse and verify this exact obligation: controlled authorized transition for JIT if ever supported. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controlled authorized transition for JIT if ever supported; retain observable state/resource expectations.
- [ ] **F-KR-008.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to NX/W^X: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-008.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-008.06 — Integrate into the real consumer and runtime route**
  - Action: Wire NX/W^X into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-008.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-008.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for NX/W^X as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-008.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-009"></a>
## F-KR-009 — guard pages

**Original requirement:** user/kernel stacks and critical arenas fault safely and kill only offender

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** fixed one-page lower user-stack guard is directly fault-observed; kernel-stack guards are source- and use-observed but not overflow-fault-injected; no emergency IST guard page; no general process lifecycle or recovery service; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-009.01 — Reconcile existing guard pages**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for guard pages. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user/kernel stacks and critical arenas fault safely and kill only offender
- [ ] **F-KR-009.02 — Freeze the exact contract for guard pages**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user/kernel stacks and critical arenas fault safely and kill only offender. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-009.03 — Implement/prove: user/kernel stacks and critical arenas fault safely and kill only offender**
  - Action: For guard pages, implement or reuse and verify this exact obligation: user/kernel stacks and critical arenas fault safely and kill only offender. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user/kernel stacks and critical arenas fault safely and kill only offender; retain observable state/resource expectations.
- [ ] **F-KR-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to guard pages: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire guard pages into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for guard pages as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-010"></a>
## F-KR-010 — anonymous memory

**Original requirement:** reserved vs committed, zero fill, limits, reclaim, collision, and OOM semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** fixed virtual window and caller-selected page indices; no virtual-area allocator or demand paging; no file-backed or shared mappings; no concurrent teardown protocol; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-010.01 — Reconcile existing anonymous memory**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for anonymous memory. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reserved vs committed, zero fill, limits, reclaim, collision, and OOM semantics
- [ ] **F-KR-010.02 — Freeze the exact contract for anonymous memory**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reserved vs committed, zero fill, limits, reclaim, collision, and OOM semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-010.03 — Implement/prove: reserved vs committed**
  - Action: For anonymous memory, implement or reuse and verify this exact obligation: reserved vs committed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reserved vs committed; retain observable state/resource expectations.
- [ ] **F-KR-010.04 — Implement/prove: zero fill**
  - Action: For anonymous memory, implement or reuse and verify this exact obligation: zero fill. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for zero fill; retain observable state/resource expectations.
- [ ] **F-KR-010.05 — Implement/prove: limits**
  - Action: For anonymous memory, implement or reuse and verify this exact obligation: limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for limits; retain observable state/resource expectations.
- [ ] **F-KR-010.06 — Implement/prove: reclaim**
  - Action: For anonymous memory, implement or reuse and verify this exact obligation: reclaim. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-010.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reclaim; retain observable state/resource expectations.
- [ ] **F-KR-010.07 — Implement/prove: collision**
  - Action: For anonymous memory, implement or reuse and verify this exact obligation: collision. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-010.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for collision; retain observable state/resource expectations.
- [ ] **F-KR-010.08 — Implement/prove: OOM semantics**
  - Action: For anonymous memory, implement or reuse and verify this exact obligation: OOM semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-010.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for OOM semantics; retain observable state/resource expectations.
- [ ] **F-KR-010.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to anonymous memory: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-010.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-010.10 — Integrate into the real consumer and runtime route**
  - Action: Wire anonymous memory into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-010.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-010.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for anonymous memory as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-010.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-011"></a>
## F-KR-011 — file-backed mapping

**Original requirement:** shared/private mappings integrated with cache, permissions, truncate, and revoke

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-011.01 — Reconcile existing file-backed mapping**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file-backed mapping. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: shared/private mappings integrated with cache, permissions, truncate, and revoke
- [ ] **F-KR-011.02 — Freeze the exact contract for file-backed mapping**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: shared/private mappings integrated with cache, permissions, truncate, and revoke. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-011.03 — Implement/prove: shared/private mappings integrated with cache**
  - Action: For file-backed mapping, implement or reuse and verify this exact obligation: shared/private mappings integrated with cache. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shared/private mappings integrated with cache; retain observable state/resource expectations.
- [ ] **F-KR-011.04 — Implement/prove: permissions**
  - Action: For file-backed mapping, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-KR-011.05 — Implement/prove: truncate**
  - Action: For file-backed mapping, implement or reuse and verify this exact obligation: truncate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for truncate; retain observable state/resource expectations.
- [ ] **F-KR-011.06 — Implement/prove: revoke**
  - Action: For file-backed mapping, implement or reuse and verify this exact obligation: revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revoke; retain observable state/resource expectations.
- [ ] **F-KR-011.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file-backed mapping: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-011.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-011.08 — Integrate into the real consumer and runtime route**
  - Action: Wire file-backed mapping into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-011.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-011.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file-backed mapping as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-011.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-012"></a>
## F-KR-012 — copy-on-write

**Original requirement:** atomic refcount/permission changes and concurrent-fault cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-012.01 — Reconcile existing copy-on-write**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for copy-on-write. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: atomic refcount/permission changes and concurrent-fault cleanup
- [ ] **F-KR-012.02 — Freeze the exact contract for copy-on-write**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: atomic refcount/permission changes and concurrent-fault cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-012.03 — Implement/prove: atomic refcount/permission changes and concurrent-fault cleanup**
  - Action: For copy-on-write, implement or reuse and verify this exact obligation: atomic refcount/permission changes and concurrent-fault cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic refcount/permission changes and concurrent-fault cleanup; retain observable state/resource expectations.
- [ ] **F-KR-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to copy-on-write: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire copy-on-write into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for copy-on-write as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-013"></a>
## F-KR-013 — shared-memory object

**Original requirement:** opaque owner/grantee handle, size/rights/generation/refcount/revoke; no global IDs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-013.01 — Reconcile existing shared-memory object**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shared-memory object. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: opaque owner/grantee handle, size/rights/generation/refcount/revoke; no global IDs
- [ ] **F-KR-013.02 — Freeze the exact contract for shared-memory object**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: opaque owner/grantee handle, size/rights/generation/refcount/revoke; no global IDs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-013.03 — Implement/prove: opaque owner/grantee handle**
  - Action: For shared-memory object, implement or reuse and verify this exact obligation: opaque owner/grantee handle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opaque owner/grantee handle; retain observable state/resource expectations.
- [ ] **F-KR-013.04 — Implement/prove: size/rights/generation/refcount/revoke**
  - Action: For shared-memory object, implement or reuse and verify this exact obligation: size/rights/generation/refcount/revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for size/rights/generation/refcount/revoke; retain observable state/resource expectations.
- [ ] **F-KR-013.05 — Implement/prove: no global IDs**
  - Action: For shared-memory object, implement or reuse and verify this exact obligation: no global IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no global IDs; retain observable state/resource expectations.
- [ ] **F-KR-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shared-memory object: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire shared-memory object into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shared-memory object as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-014"></a>
## F-KR-014 — page/file cache

**Original requirement:** bounded ownership, dirty accounting, async writeback, reclaim, flush, and pressure tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-014.01 — Reconcile existing page/file cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for page/file cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded ownership, dirty accounting, async writeback, reclaim, flush, and pressure tests
- [ ] **F-KR-014.02 — Freeze the exact contract for page/file cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded ownership, dirty accounting, async writeback, reclaim, flush, and pressure tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-014.03 — Implement/prove: bounded ownership**
  - Action: For page/file cache, implement or reuse and verify this exact obligation: bounded ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded ownership; retain observable state/resource expectations.
- [ ] **F-KR-014.04 — Implement/prove: dirty accounting**
  - Action: For page/file cache, implement or reuse and verify this exact obligation: dirty accounting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dirty accounting; retain observable state/resource expectations.
- [ ] **F-KR-014.05 — Implement/prove: async writeback**
  - Action: For page/file cache, implement or reuse and verify this exact obligation: async writeback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for async writeback; retain observable state/resource expectations.
- [ ] **F-KR-014.06 — Implement/prove: reclaim**
  - Action: For page/file cache, implement or reuse and verify this exact obligation: reclaim. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reclaim; retain observable state/resource expectations.
- [ ] **F-KR-014.07 — Implement/prove: flush**
  - Action: For page/file cache, implement or reuse and verify this exact obligation: flush. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-014.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flush; retain observable state/resource expectations.
- [ ] **F-KR-014.08 — Implement/prove: pressure tests**
  - Action: For page/file cache, implement or reuse and verify this exact obligation: pressure tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-014.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pressure tests; retain observable state/resource expectations.
- [ ] **F-KR-014.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to page/file cache: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-014.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-014.10 — Integrate into the real consumer and runtime route**
  - Action: Wire page/file cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-014.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-014.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for page/file cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-014.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-015"></a>
## F-KR-015 — memory accounting

**Original requirement:** process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no unified service accounting; no page/file-cache accounting; no pinned-memory accounting; no DMA accounting; no surface accounting; no complete kernel-total attribution; no allocation-origin provenance beyond the typed owner identifier; no pressure/reclaim policy; no concurrent or SMP ownership contract; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-015.01 — Reconcile existing memory accounting**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for memory accounting. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance
- [ ] **F-KR-015.02 — Freeze the exact contract for memory accounting**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-015.03 — Implement/prove: process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance**
  - Action: For memory accounting, implement or reuse and verify this exact obligation: process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for process/service/cache/pinned/DMA/surface/kernel totals with quotas and provenance; retain observable state/resource expectations.
- [ ] **F-KR-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to memory accounting: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire memory accounting into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for memory accounting as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-016"></a>
## F-KR-016 — memory pressure policy

**Original requirement:** priority-aware reclaim, cache eviction, app notification, OOM selection, recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-016.01 — Reconcile existing memory pressure policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for memory pressure policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-05, H-06, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: priority-aware reclaim, cache eviction, app notification, OOM selection, recovery
- [ ] **F-KR-016.02 — Freeze the exact contract for memory pressure policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: priority-aware reclaim, cache eviction, app notification, OOM selection, recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-016.03 — Implement/prove: priority-aware reclaim**
  - Action: For memory pressure policy, implement or reuse and verify this exact obligation: priority-aware reclaim. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for priority-aware reclaim; retain observable state/resource expectations.
- [ ] **F-KR-016.04 — Implement/prove: cache eviction**
  - Action: For memory pressure policy, implement or reuse and verify this exact obligation: cache eviction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache eviction; retain observable state/resource expectations.
- [ ] **F-KR-016.05 — Implement/prove: app notification**
  - Action: For memory pressure policy, implement or reuse and verify this exact obligation: app notification. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app notification; retain observable state/resource expectations.
- [ ] **F-KR-016.06 — Implement/prove: OOM selection**
  - Action: For memory pressure policy, implement or reuse and verify this exact obligation: OOM selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-016.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for OOM selection; retain observable state/resource expectations.
- [ ] **F-KR-016.07 — Implement/prove: recovery**
  - Action: For memory pressure policy, implement or reuse and verify this exact obligation: recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-016.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery; retain observable state/resource expectations.
- [ ] **F-KR-016.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to memory pressure policy: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-016.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-016.09 — Integrate into the real consumer and runtime route**
  - Action: Wire memory pressure policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-016.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-016.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for memory pressure policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-016.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-017"></a>
## F-KR-017 — kernel stack management

**Original requirement:** per-thread stacks, guards, high-water evidence, safe interrupt nesting

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** fixed two-process service rather than general per-thread stack ownership; no direct kernel-stack overflow fault injection; no guarded emergency IST stack; no scalable thread-stack allocator; no SMP interrupt-nesting or current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-017.01 — Reconcile existing kernel stack management**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kernel stack management. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-thread stacks, guards, high-water evidence, safe interrupt nesting
- [ ] **F-KR-017.02 — Freeze the exact contract for kernel stack management**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-thread stacks, guards, high-water evidence, safe interrupt nesting. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-017.03 — Implement/prove: per-thread stacks**
  - Action: For kernel stack management, implement or reuse and verify this exact obligation: per-thread stacks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-thread stacks; retain observable state/resource expectations.
- [ ] **F-KR-017.04 — Implement/prove: guards**
  - Action: For kernel stack management, implement or reuse and verify this exact obligation: guards. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for guards; retain observable state/resource expectations.
- [ ] **F-KR-017.05 — Implement/prove: high-water evidence**
  - Action: For kernel stack management, implement or reuse and verify this exact obligation: high-water evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for high-water evidence; retain observable state/resource expectations.
- [ ] **F-KR-017.06 — Implement/prove: safe interrupt nesting**
  - Action: For kernel stack management, implement or reuse and verify this exact obligation: safe interrupt nesting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe interrupt nesting; retain observable state/resource expectations.
- [ ] **F-KR-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kernel stack management: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire kernel stack management into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kernel stack management as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-018"></a>
## F-KR-018 — process creation

**Original requirement:** validated executable, credentials, handles, limits, namespace, atomic publication

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-018.01 — Reconcile existing process creation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for process creation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validated executable, credentials, handles, limits, namespace, atomic publication
- [ ] **F-KR-018.02 — Freeze the exact contract for process creation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validated executable, credentials, handles, limits, namespace, atomic publication. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-018.03 — Implement/prove: validated executable**
  - Action: For process creation, implement or reuse and verify this exact obligation: validated executable. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated executable; retain observable state/resource expectations.
- [ ] **F-KR-018.04 — Implement/prove: credentials**
  - Action: For process creation, implement or reuse and verify this exact obligation: credentials. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for credentials; retain observable state/resource expectations.
- [ ] **F-KR-018.05 — Implement/prove: handles**
  - Action: For process creation, implement or reuse and verify this exact obligation: handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handles; retain observable state/resource expectations.
- [ ] **F-KR-018.06 — Implement/prove: limits**
  - Action: For process creation, implement or reuse and verify this exact obligation: limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for limits; retain observable state/resource expectations.
- [ ] **F-KR-018.07 — Implement/prove: namespace**
  - Action: For process creation, implement or reuse and verify this exact obligation: namespace. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-018.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for namespace; retain observable state/resource expectations.
- [ ] **F-KR-018.08 — Implement/prove: atomic publication**
  - Action: For process creation, implement or reuse and verify this exact obligation: atomic publication. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-018.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic publication; retain observable state/resource expectations.
- [ ] **F-KR-018.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to process creation: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-018.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-018.10 — Integrate into the real consumer and runtime route**
  - Action: Wire process creation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-018.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-018.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for process creation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-018.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-019"></a>
## F-KR-019 — fork/clone semantics

**Original requirement:** exact credential/handle/address-space copy or sharing matrix with rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-019.01 — Reconcile existing fork/clone semantics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for fork/clone semantics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact credential/handle/address-space copy or sharing matrix with rollback
- [ ] **F-KR-019.02 — Freeze the exact contract for fork/clone semantics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact credential/handle/address-space copy or sharing matrix with rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-019.03 — Implement/prove: exact credential/handle/address-space copy or sharing matrix with rollback**
  - Action: For fork/clone semantics, implement or reuse and verify this exact obligation: exact credential/handle/address-space copy or sharing matrix with rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact credential/handle/address-space copy or sharing matrix with rollback; retain observable state/resource expectations.
- [ ] **F-KR-019.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to fork/clone semantics: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-019.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-019.05 — Integrate into the real consumer and runtime route**
  - Action: Wire fork/clone semantics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-019.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-019.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for fork/clone semantics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-019.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-020"></a>
## F-KR-020 — exec semantics

**Original requirement:** atomic image replacement preserving only declared process state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-020.01 — Reconcile existing exec semantics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for exec semantics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: atomic image replacement preserving only declared process state
- [ ] **F-KR-020.02 — Freeze the exact contract for exec semantics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: atomic image replacement preserving only declared process state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-020.03 — Implement/prove: atomic image replacement preserving only declared process state**
  - Action: For exec semantics, implement or reuse and verify this exact obligation: atomic image replacement preserving only declared process state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic image replacement preserving only declared process state; retain observable state/resource expectations.
- [ ] **F-KR-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to exec semantics: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire exec semantics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for exec semantics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-021"></a>
## F-KR-021 — thread lifecycle

**Original requirement:** create/join/exit/cancel, ownership, stack/TLS, peer/process death cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-021.01 — Reconcile existing thread lifecycle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for thread lifecycle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: create/join/exit/cancel, ownership, stack/TLS, peer/process death cleanup
- [ ] **F-KR-021.02 — Freeze the exact contract for thread lifecycle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: create/join/exit/cancel, ownership, stack/TLS, peer/process death cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-021.03 — Implement/prove: create/join/exit/cancel**
  - Action: For thread lifecycle, implement or reuse and verify this exact obligation: create/join/exit/cancel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for create/join/exit/cancel; retain observable state/resource expectations.
- [ ] **F-KR-021.04 — Implement/prove: ownership**
  - Action: For thread lifecycle, implement or reuse and verify this exact obligation: ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ownership; retain observable state/resource expectations.
- [ ] **F-KR-021.05 — Implement/prove: stack/TLS**
  - Action: For thread lifecycle, implement or reuse and verify this exact obligation: stack/TLS. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stack/TLS; retain observable state/resource expectations.
- [ ] **F-KR-021.06 — Implement/prove: peer/process death cleanup**
  - Action: For thread lifecycle, implement or reuse and verify this exact obligation: peer/process death cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for peer/process death cleanup; retain observable state/resource expectations.
- [ ] **F-KR-021.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to thread lifecycle: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-021.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-021.08 — Integrate into the real consumer and runtime route**
  - Action: Wire thread lifecycle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-021.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-021.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for thread lifecycle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-021.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-022"></a>
## F-KR-022 — scheduler

**Original requirement:** per-CPU ownership, priorities, fairness, bounded queues, accounting, migration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** the persistent Ring-3 service has exactly two fixed process slots; no general priority or deadline contract beyond bounded round robin; no per-CPU run-queue ownership or process migration; no userspace process-management or cancellation API; the separate eight-slot kernel task demo is cooperative and lacks FPU/SSE state; no current physical-hardware scheduler receipt.

### Execution steps

- [ ] **F-KR-022.01 — Reconcile existing scheduler**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scheduler. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-CPU ownership, priorities, fairness, bounded queues, accounting, migration
- [ ] **F-KR-022.02 — Freeze the exact contract for scheduler**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-CPU ownership, priorities, fairness, bounded queues, accounting, migration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-022.03 — Implement/prove: per-CPU ownership**
  - Action: For scheduler, implement or reuse and verify this exact obligation: per-CPU ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-CPU ownership; retain observable state/resource expectations.
- [ ] **F-KR-022.04 — Implement/prove: priorities**
  - Action: For scheduler, implement or reuse and verify this exact obligation: priorities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for priorities; retain observable state/resource expectations.
- [ ] **F-KR-022.05 — Implement/prove: fairness**
  - Action: For scheduler, implement or reuse and verify this exact obligation: fairness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fairness; retain observable state/resource expectations.
- [ ] **F-KR-022.06 — Implement/prove: bounded queues**
  - Action: For scheduler, implement or reuse and verify this exact obligation: bounded queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded queues; retain observable state/resource expectations.
- [ ] **F-KR-022.07 — Implement/prove: accounting**
  - Action: For scheduler, implement or reuse and verify this exact obligation: accounting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-022.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accounting; retain observable state/resource expectations.
- [ ] **F-KR-022.08 — Implement/prove: migration**
  - Action: For scheduler, implement or reuse and verify this exact obligation: migration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-022.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migration; retain observable state/resource expectations.
- [ ] **F-KR-022.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scheduler: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-022.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-022.10 — Integrate into the real consumer and runtime route**
  - Action: Wire scheduler into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-022.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-022.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scheduler as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-022.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-023"></a>
## F-KR-023 — real-time scheduling

**Original requirement:** admission, utilization bounds, priority inversion control, deadline miss evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-023.01 — Reconcile existing real-time scheduling**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for real-time scheduling. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: admission, utilization bounds, priority inversion control, deadline miss evidence
- [ ] **F-KR-023.02 — Freeze the exact contract for real-time scheduling**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: admission, utilization bounds, priority inversion control, deadline miss evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-023.03 — Implement/prove: admission**
  - Action: For real-time scheduling, implement or reuse and verify this exact obligation: admission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for admission; retain observable state/resource expectations.
- [ ] **F-KR-023.04 — Implement/prove: utilization bounds**
  - Action: For real-time scheduling, implement or reuse and verify this exact obligation: utilization bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for utilization bounds; retain observable state/resource expectations.
- [ ] **F-KR-023.05 — Implement/prove: priority inversion control**
  - Action: For real-time scheduling, implement or reuse and verify this exact obligation: priority inversion control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for priority inversion control; retain observable state/resource expectations.
- [ ] **F-KR-023.06 — Implement/prove: deadline miss evidence**
  - Action: For real-time scheduling, implement or reuse and verify this exact obligation: deadline miss evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadline miss evidence; retain observable state/resource expectations.
- [ ] **F-KR-023.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to real-time scheduling: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-023.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-023.08 — Integrate into the real consumer and runtime route**
  - Action: Wire real-time scheduling into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-023.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-023.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for real-time scheduling as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-023.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-024"></a>
## F-KR-024 — timer/deadline API

**Original requirement:** monotonic absolute deadlines, cancellation, coalescing, wrap and suspend behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no public absolute-deadline API; no cancellation or timer coalescing; no suspend/resume clock contract; no measured maximum wake latency; two fixed process slots and 32-bit 100 Hz tick domain; no current physical-hardware timer receipt.

### Execution steps

- [ ] **F-KR-024.01 — Reconcile existing timer/deadline API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for timer/deadline API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: monotonic absolute deadlines, cancellation, coalescing, wrap and suspend behavior
- [ ] **F-KR-024.02 — Freeze the exact contract for timer/deadline API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: monotonic absolute deadlines, cancellation, coalescing, wrap and suspend behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-024.03 — Implement/prove: monotonic absolute deadlines**
  - Action: For timer/deadline API, implement or reuse and verify this exact obligation: monotonic absolute deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for monotonic absolute deadlines; retain observable state/resource expectations.
- [ ] **F-KR-024.04 — Implement/prove: cancellation**
  - Action: For timer/deadline API, implement or reuse and verify this exact obligation: cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation; retain observable state/resource expectations.
- [ ] **F-KR-024.05 — Implement/prove: coalescing**
  - Action: For timer/deadline API, implement or reuse and verify this exact obligation: coalescing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for coalescing; retain observable state/resource expectations.
- [ ] **F-KR-024.06 — Implement/prove: wrap and suspend behavior**
  - Action: For timer/deadline API, implement or reuse and verify this exact obligation: wrap and suspend behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-024.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wrap and suspend behavior; retain observable state/resource expectations.
- [ ] **F-KR-024.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to timer/deadline API: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-024.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-024.08 — Integrate into the real consumer and runtime route**
  - Action: Wire timer/deadline API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-024.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-024.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for timer/deadline API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-024.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-025"></a>
## F-KR-025 — signals/events

**Original requirement:** permission-checked delivery including signal 0/groups, bounded queues, defined interruption

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-025.01 — Reconcile existing signals/events**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for signals/events. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permission-checked delivery including signal 0/groups, bounded queues, defined interruption
- [ ] **F-KR-025.02 — Freeze the exact contract for signals/events**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permission-checked delivery including signal 0/groups, bounded queues, defined interruption. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-025.03 — Implement/prove: permission-checked delivery including signal 0/groups**
  - Action: For signals/events, implement or reuse and verify this exact obligation: permission-checked delivery including signal 0/groups. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission-checked delivery including signal 0/groups; retain observable state/resource expectations.
- [ ] **F-KR-025.04 — Implement/prove: bounded queues**
  - Action: For signals/events, implement or reuse and verify this exact obligation: bounded queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded queues; retain observable state/resource expectations.
- [ ] **F-KR-025.05 — Implement/prove: defined interruption**
  - Action: For signals/events, implement or reuse and verify this exact obligation: defined interruption. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defined interruption; retain observable state/resource expectations.
- [ ] **F-KR-025.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to signals/events: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-025.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-025.07 — Integrate into the real consumer and runtime route**
  - Action: Wire signals/events into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-025.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-025.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for signals/events as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-025.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-026"></a>
## F-KR-026 — process groups/sessions/jobs

**Original requirement:** shell/terminal ownership, foreground control, hangup, stop/continue and cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-026.01 — Reconcile existing process groups/sessions/jobs**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for process groups/sessions/jobs. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: shell/terminal ownership, foreground control, hangup, stop/continue and cleanup
- [ ] **F-KR-026.02 — Freeze the exact contract for process groups/sessions/jobs**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: shell/terminal ownership, foreground control, hangup, stop/continue and cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-026.03 — Implement/prove: shell/terminal ownership**
  - Action: For process groups/sessions/jobs, implement or reuse and verify this exact obligation: shell/terminal ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shell/terminal ownership; retain observable state/resource expectations.
- [ ] **F-KR-026.04 — Implement/prove: foreground control**
  - Action: For process groups/sessions/jobs, implement or reuse and verify this exact obligation: foreground control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for foreground control; retain observable state/resource expectations.
- [ ] **F-KR-026.05 — Implement/prove: hangup**
  - Action: For process groups/sessions/jobs, implement or reuse and verify this exact obligation: hangup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hangup; retain observable state/resource expectations.
- [ ] **F-KR-026.06 — Implement/prove: stop/continue and cleanup**
  - Action: For process groups/sessions/jobs, implement or reuse and verify this exact obligation: stop/continue and cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-026.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stop/continue and cleanup; retain observable state/resource expectations.
- [ ] **F-KR-026.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to process groups/sessions/jobs: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-026.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-026.08 — Integrate into the real consumer and runtime route**
  - Action: Wire process groups/sessions/jobs into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-026.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-026.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for process groups/sessions/jobs as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-026.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-027"></a>
## F-KR-027 — wait/exit status

**Original requirement:** parent/child identity, exact termination reason, no PID reuse confusion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no userspace wait syscall or process-handle ABI; parent/child authority is host-proved but not exposed as a target service; no concurrent PID-reuse or cancellation receipt; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-027.01 — Reconcile existing wait/exit status**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for wait/exit status. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: parent/child identity, exact termination reason, no PID reuse confusion
- [ ] **F-KR-027.02 — Freeze the exact contract for wait/exit status**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: parent/child identity, exact termination reason, no PID reuse confusion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-027.03 — Implement/prove: parent/child identity**
  - Action: For wait/exit status, implement or reuse and verify this exact obligation: parent/child identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for parent/child identity; retain observable state/resource expectations.
- [ ] **F-KR-027.04 — Implement/prove: exact termination reason**
  - Action: For wait/exit status, implement or reuse and verify this exact obligation: exact termination reason. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact termination reason; retain observable state/resource expectations.
- [ ] **F-KR-027.05 — Implement/prove: no PID reuse confusion**
  - Action: For wait/exit status, implement or reuse and verify this exact obligation: no PID reuse confusion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no PID reuse confusion; retain observable state/resource expectations.
- [ ] **F-KR-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to wait/exit status: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire wait/exit status into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for wait/exit status as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-028"></a>
## F-KR-028 — process handles

**Original requirement:** generation-tagged opaque references replace ambient PIDs for privileged operations

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** handles are kernel-internal and not an opaque userspace ABI; PID remains the current IPC selector; no cross-process delegated authority or revocation model; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-028.01 — Reconcile existing process handles**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for process handles. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generation-tagged opaque references replace ambient PIDs for privileged operations
- [ ] **F-KR-028.02 — Freeze the exact contract for process handles**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generation-tagged opaque references replace ambient PIDs for privileged operations. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-028.03 — Implement/prove: generation-tagged opaque references replace ambient PIDs for privileged operations**
  - Action: For process handles, implement or reuse and verify this exact obligation: generation-tagged opaque references replace ambient PIDs for privileged operations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generation-tagged opaque references replace ambient PIDs for privileged operations; retain observable state/resource expectations.
- [ ] **F-KR-028.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to process handles: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-028.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-028.05 — Integrate into the real consumer and runtime route**
  - Action: Wire process handles into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-028.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-028.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for process handles as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-028.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-029"></a>
## F-KR-029 — resource limits

**Original requirement:** memory, CPU, handles, threads, IPC, files, sockets, surfaces, log and time budgets

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-029.01 — Reconcile existing resource limits**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for resource limits. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: memory, CPU, handles, threads, IPC, files, sockets, surfaces, log and time budgets
- [ ] **F-KR-029.02 — Freeze the exact contract for resource limits**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: memory, CPU, handles, threads, IPC, files, sockets, surfaces, log and time budgets. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-029.03 — Implement/prove: memory**
  - Action: For resource limits, implement or reuse and verify this exact obligation: memory. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for memory; retain observable state/resource expectations.
- [ ] **F-KR-029.04 — Implement/prove: CPU**
  - Action: For resource limits, implement or reuse and verify this exact obligation: CPU. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CPU; retain observable state/resource expectations.
- [ ] **F-KR-029.05 — Implement/prove: handles**
  - Action: For resource limits, implement or reuse and verify this exact obligation: handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handles; retain observable state/resource expectations.
- [ ] **F-KR-029.06 — Implement/prove: threads**
  - Action: For resource limits, implement or reuse and verify this exact obligation: threads. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for threads; retain observable state/resource expectations.
- [ ] **F-KR-029.07 — Implement/prove: IPC**
  - Action: For resource limits, implement or reuse and verify this exact obligation: IPC. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IPC; retain observable state/resource expectations.
- [ ] **F-KR-029.08 — Implement/prove: files**
  - Action: For resource limits, implement or reuse and verify this exact obligation: files. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files; retain observable state/resource expectations.
- [ ] **F-KR-029.09 — Implement/prove: sockets**
  - Action: For resource limits, implement or reuse and verify this exact obligation: sockets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sockets; retain observable state/resource expectations.
- [ ] **F-KR-029.10 — Implement/prove: surfaces**
  - Action: For resource limits, implement or reuse and verify this exact obligation: surfaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for surfaces; retain observable state/resource expectations.
- [ ] **F-KR-029.11 — Implement/prove: log and time budgets**
  - Action: For resource limits, implement or reuse and verify this exact obligation: log and time budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-029.10.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for log and time budgets; retain observable state/resource expectations.
- [ ] **F-KR-029.12 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to resource limits: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-029.11.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-029.13 — Integrate into the real consumer and runtime route**
  - Action: Wire resource limits into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-029.12.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-029.14 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for resource limits as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-029.13.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-030"></a>
## F-KR-030 — namespace/container boundary

**Original requirement:** mount, process, network, IPC, device and user namespaces with explicit joins

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-030.01 — Reconcile existing namespace/container boundary**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for namespace/container boundary. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04, H-06, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: mount, process, network, IPC, device and user namespaces with explicit joins
- [ ] **F-KR-030.02 — Freeze the exact contract for namespace/container boundary**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: mount, process, network, IPC, device and user namespaces with explicit joins. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-030.03 — Implement/prove: mount**
  - Action: For namespace/container boundary, implement or reuse and verify this exact obligation: mount. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mount; retain observable state/resource expectations.
- [ ] **F-KR-030.04 — Implement/prove: process**
  - Action: For namespace/container boundary, implement or reuse and verify this exact obligation: process. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for process; retain observable state/resource expectations.
- [ ] **F-KR-030.05 — Implement/prove: network**
  - Action: For namespace/container boundary, implement or reuse and verify this exact obligation: network. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for network; retain observable state/resource expectations.
- [ ] **F-KR-030.06 — Implement/prove: IPC**
  - Action: For namespace/container boundary, implement or reuse and verify this exact obligation: IPC. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IPC; retain observable state/resource expectations.
- [ ] **F-KR-030.07 — Implement/prove: device and user namespaces with explicit joins**
  - Action: For namespace/container boundary, implement or reuse and verify this exact obligation: device and user namespaces with explicit joins. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-030.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device and user namespaces with explicit joins; retain observable state/resource expectations.
- [ ] **F-KR-030.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to namespace/container boundary: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-030.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-030.09 — Integrate into the real consumer and runtime route**
  - Action: Wire namespace/container boundary into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-030.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-030.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for namespace/container boundary as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-030.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-031"></a>
## F-KR-031 — exception/fault containment

**Original requirement:** malformed user state kills current process only; desktop and system continue

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** only a bounded two-process service is covered; fault-vector and malformed-frame coverage is incomplete; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-031.01 — Reconcile existing exception/fault containment**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for exception/fault containment. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: malformed user state kills current process only; desktop and system continue
- [ ] **F-KR-031.02 — Freeze the exact contract for exception/fault containment**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: malformed user state kills current process only; desktop and system continue. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-031.03 — Implement/prove: malformed user state kills current process only**
  - Action: For exception/fault containment, implement or reuse and verify this exact obligation: malformed user state kills current process only. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for malformed user state kills current process only; retain observable state/resource expectations.
- [ ] **F-KR-031.04 — Implement/prove: desktop and system continue**
  - Action: For exception/fault containment, implement or reuse and verify this exact obligation: desktop and system continue. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for desktop and system continue; retain observable state/resource expectations.
- [ ] **F-KR-031.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to exception/fault containment: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-031.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-031.06 — Integrate into the real consumer and runtime route**
  - Action: Wire exception/fault containment into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-031.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-031.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for exception/fault containment as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-031.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-032"></a>
## F-KR-032 — interrupt/exception architecture

**Original requirement:** complete vector ownership, nesting, fault frames, spurious paths, and panic fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no broad kernel-fatal exception-vector or spurious-interrupt QEMU matrix; double fault stops safely but has no recovery or emergency-stack guard page; offender-only user-fault containment is limited to the native-UEFI64 two-slot harness; no general process lifecycle recovery.

### Execution steps

- [ ] **F-KR-032.01 — Reconcile existing interrupt/exception architecture**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for interrupt/exception architecture. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: complete vector ownership, nesting, fault frames, spurious paths, and panic fallback
- [ ] **F-KR-032.02 — Freeze the exact contract for interrupt/exception architecture**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: complete vector ownership, nesting, fault frames, spurious paths, and panic fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-032.03 — Implement/prove: complete vector ownership**
  - Action: For interrupt/exception architecture, implement or reuse and verify this exact obligation: complete vector ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete vector ownership; retain observable state/resource expectations.
- [ ] **F-KR-032.04 — Implement/prove: nesting**
  - Action: For interrupt/exception architecture, implement or reuse and verify this exact obligation: nesting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-032.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for nesting; retain observable state/resource expectations.
- [ ] **F-KR-032.05 — Implement/prove: fault frames**
  - Action: For interrupt/exception architecture, implement or reuse and verify this exact obligation: fault frames. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-032.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fault frames; retain observable state/resource expectations.
- [ ] **F-KR-032.06 — Implement/prove: spurious paths**
  - Action: For interrupt/exception architecture, implement or reuse and verify this exact obligation: spurious paths. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-032.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for spurious paths; retain observable state/resource expectations.
- [ ] **F-KR-032.07 — Implement/prove: panic fallback**
  - Action: For interrupt/exception architecture, implement or reuse and verify this exact obligation: panic fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-032.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for panic fallback; retain observable state/resource expectations.
- [ ] **F-KR-032.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to interrupt/exception architecture: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-032.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-032.09 — Integrate into the real consumer and runtime route**
  - Action: Wire interrupt/exception architecture into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-032.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-032.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for interrupt/exception architecture as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-032.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-033"></a>
## F-KR-033 — TLB shootdown

**Original requirement:** generation/ack protocol, offline CPUs, timeout/fail-stop, stress proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-033.01 — Reconcile existing TLB shootdown**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for TLB shootdown. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generation/ack protocol, offline CPUs, timeout/fail-stop, stress proof
- [ ] **F-KR-033.02 — Freeze the exact contract for TLB shootdown**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generation/ack protocol, offline CPUs, timeout/fail-stop, stress proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-033.03 — Implement/prove: generation/ack protocol**
  - Action: For TLB shootdown, implement or reuse and verify this exact obligation: generation/ack protocol. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generation/ack protocol; retain observable state/resource expectations.
- [ ] **F-KR-033.04 — Implement/prove: offline CPUs**
  - Action: For TLB shootdown, implement or reuse and verify this exact obligation: offline CPUs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for offline CPUs; retain observable state/resource expectations.
- [ ] **F-KR-033.05 — Implement/prove: timeout/fail-stop**
  - Action: For TLB shootdown, implement or reuse and verify this exact obligation: timeout/fail-stop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout/fail-stop; retain observable state/resource expectations.
- [ ] **F-KR-033.06 — Implement/prove: stress proof**
  - Action: For TLB shootdown, implement or reuse and verify this exact obligation: stress proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stress proof; retain observable state/resource expectations.
- [ ] **F-KR-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to TLB shootdown: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire TLB shootdown into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for TLB shootdown as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-034"></a>
## F-KR-034 — kernel synchronization

**Original requirement:** lock ordering, IRQ/preemption rules, wait queues, atomics, deadlock and race tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-034.01 — Reconcile existing kernel synchronization**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kernel synchronization. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: lock ordering, IRQ/preemption rules, wait queues, atomics, deadlock and race tests
- [ ] **F-KR-034.02 — Freeze the exact contract for kernel synchronization**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: lock ordering, IRQ/preemption rules, wait queues, atomics, deadlock and race tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-034.03 — Implement/prove: lock ordering**
  - Action: For kernel synchronization, implement or reuse and verify this exact obligation: lock ordering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lock ordering; retain observable state/resource expectations.
- [ ] **F-KR-034.04 — Implement/prove: IRQ/preemption rules**
  - Action: For kernel synchronization, implement or reuse and verify this exact obligation: IRQ/preemption rules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IRQ/preemption rules; retain observable state/resource expectations.
- [ ] **F-KR-034.05 — Implement/prove: wait queues**
  - Action: For kernel synchronization, implement or reuse and verify this exact obligation: wait queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-034.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wait queues; retain observable state/resource expectations.
- [ ] **F-KR-034.06 — Implement/prove: atomics**
  - Action: For kernel synchronization, implement or reuse and verify this exact obligation: atomics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-034.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomics; retain observable state/resource expectations.
- [ ] **F-KR-034.07 — Implement/prove: deadlock and race tests**
  - Action: For kernel synchronization, implement or reuse and verify this exact obligation: deadlock and race tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-034.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadlock and race tests; retain observable state/resource expectations.
- [ ] **F-KR-034.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kernel synchronization: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-034.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-034.09 — Integrate into the real consumer and runtime route**
  - Action: Wire kernel synchronization into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-034.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-034.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kernel synchronization as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-034.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-035"></a>
## F-KR-035 — random number service

**Original requirement:** boot entropy accounting, CSPRNG reseed/fork semantics, blocking/readiness policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-035.01 — Reconcile existing random number service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for random number service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: boot entropy accounting, CSPRNG reseed/fork semantics, blocking/readiness policy
- [ ] **F-KR-035.02 — Freeze the exact contract for random number service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: boot entropy accounting, CSPRNG reseed/fork semantics, blocking/readiness policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-035.03 — Implement/prove: boot entropy accounting**
  - Action: For random number service, implement or reuse and verify this exact obligation: boot entropy accounting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for boot entropy accounting; retain observable state/resource expectations.
- [ ] **F-KR-035.04 — Implement/prove: CSPRNG reseed/fork semantics**
  - Action: For random number service, implement or reuse and verify this exact obligation: CSPRNG reseed/fork semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for CSPRNG reseed/fork semantics; retain observable state/resource expectations.
- [ ] **F-KR-035.05 — Implement/prove: blocking/readiness policy**
  - Action: For random number service, implement or reuse and verify this exact obligation: blocking/readiness policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for blocking/readiness policy; retain observable state/resource expectations.
- [ ] **F-KR-035.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to random number service: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-035.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-035.07 — Integrate into the real consumer and runtime route**
  - Action: Wire random number service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-035.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-035.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for random number service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-035.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-036"></a>
## F-KR-036 — syscall ABI

**Original requirement:** generated table, unsigned bounds including negative/sign-bit indices, gaps, versioning

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** ABI table covers x86-64 only; dispatch bodies remain hand-written; no generated argument/type metadata; no compatibility translation layer; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-036.01 — Reconcile existing syscall ABI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for syscall ABI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated table, unsigned bounds including negative/sign-bit indices, gaps, versioning
- [ ] **F-KR-036.02 — Freeze the exact contract for syscall ABI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated table, unsigned bounds including negative/sign-bit indices, gaps, versioning. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-036.03 — Implement/prove: generated table**
  - Action: For syscall ABI, implement or reuse and verify this exact obligation: generated table. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated table; retain observable state/resource expectations.
- [ ] **F-KR-036.04 — Implement/prove: unsigned bounds including negative/sign-bit indices**
  - Action: For syscall ABI, implement or reuse and verify this exact obligation: unsigned bounds including negative/sign-bit indices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unsigned bounds including negative/sign-bit indices; retain observable state/resource expectations.
- [ ] **F-KR-036.05 — Implement/prove: gaps**
  - Action: For syscall ABI, implement or reuse and verify this exact obligation: gaps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for gaps; retain observable state/resource expectations.
- [ ] **F-KR-036.06 — Implement/prove: versioning**
  - Action: For syscall ABI, implement or reuse and verify this exact obligation: versioning. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-036.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioning; retain observable state/resource expectations.
- [ ] **F-KR-036.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to syscall ABI: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-036.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-036.08 — Integrate into the real consumer and runtime route**
  - Action: Wire syscall ABI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-036.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-036.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for syscall ABI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-036.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-037"></a>
## F-KR-037 — stable userspace ABI

**Original requirement:** calling convention, layout, errors, handles, time, signals, compatibility policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** ABI is x86-64-only and version 1 has no translation layer; no signal contract; no compatibility or deprecation tooling; no generated argument/layout/error/handle manifest; no current physical-hardware receipt.

### Execution steps

- [ ] **F-KR-037.01 — Reconcile existing stable userspace ABI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for stable userspace ABI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: calling convention, layout, errors, handles, time, signals, compatibility policy
- [ ] **F-KR-037.02 — Freeze the exact contract for stable userspace ABI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: calling convention, layout, errors, handles, time, signals, compatibility policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-037.03 — Implement/prove: calling convention**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: calling convention. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for calling convention; retain observable state/resource expectations.
- [ ] **F-KR-037.04 — Implement/prove: layout**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: layout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layout; retain observable state/resource expectations.
- [ ] **F-KR-037.05 — Implement/prove: errors**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors; retain observable state/resource expectations.
- [ ] **F-KR-037.06 — Implement/prove: handles**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handles; retain observable state/resource expectations.
- [ ] **F-KR-037.07 — Implement/prove: time**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time; retain observable state/resource expectations.
- [ ] **F-KR-037.08 — Implement/prove: signals**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: signals. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signals; retain observable state/resource expectations.
- [ ] **F-KR-037.09 — Implement/prove: compatibility policy**
  - Action: For stable userspace ABI, implement or reuse and verify this exact obligation: compatibility policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-037.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compatibility policy; retain observable state/resource expectations.
- [ ] **F-KR-037.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to stable userspace ABI: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-037.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-037.11 — Integrate into the real consumer and runtime route**
  - Action: Wire stable userspace ABI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-037.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-037.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for stable userspace ABI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-037.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-038"></a>
## F-KR-038 — core dump

**Original requirement:** permission/redaction-aware process image, registers, maps, build IDs and retrieval

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-038.01 — Reconcile existing core dump**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for core dump. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permission/redaction-aware process image, registers, maps, build IDs and retrieval
- [ ] **F-KR-038.02 — Freeze the exact contract for core dump**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permission/redaction-aware process image, registers, maps, build IDs and retrieval. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-038.03 — Implement/prove: permission/redaction-aware process image**
  - Action: For core dump, implement or reuse and verify this exact obligation: permission/redaction-aware process image. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permission/redaction-aware process image; retain observable state/resource expectations.
- [ ] **F-KR-038.04 — Implement/prove: registers**
  - Action: For core dump, implement or reuse and verify this exact obligation: registers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for registers; retain observable state/resource expectations.
- [ ] **F-KR-038.05 — Implement/prove: maps**
  - Action: For core dump, implement or reuse and verify this exact obligation: maps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for maps; retain observable state/resource expectations.
- [ ] **F-KR-038.06 — Implement/prove: build IDs and retrieval**
  - Action: For core dump, implement or reuse and verify this exact obligation: build IDs and retrieval. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-038.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for build IDs and retrieval; retain observable state/resource expectations.
- [ ] **F-KR-038.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to core dump: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-038.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-038.08 — Integrate into the real consumer and runtime route**
  - Action: Wire core dump into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-038.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-038.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for core dump as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-038.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-039"></a>
## F-KR-039 — kernel live diagnostics

**Original requirement:** read-only structured facts without exposing arbitrary memory or device control

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-039.01 — Reconcile existing kernel live diagnostics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kernel live diagnostics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-and-evidence comparison against the complete requirement: read-only structured facts without exposing arbitrary memory or device control
- [ ] **F-KR-039.02 — Freeze the exact contract for kernel live diagnostics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: read-only structured facts without exposing arbitrary memory or device control. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-039.03 — Implement/prove: read-only structured facts without exposing arbitrary memory or device control**
  - Action: For kernel live diagnostics, implement or reuse and verify this exact obligation: read-only structured facts without exposing arbitrary memory or device control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for read-only structured facts without exposing arbitrary memory or device control; retain observable state/resource expectations.
- [ ] **F-KR-039.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kernel live diagnostics: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-039.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-039.05 — Integrate into the real consumer and runtime route**
  - Action: Wire kernel live diagnostics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-039.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-039.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kernel live diagnostics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-039.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-kr-040"></a>
## F-KR-040 — reboot/shutdown

**Original requirement:** capability-gated orderly service/device/filesystem teardown with forced fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-KR-040.01 — Reconcile existing reboot/shutdown**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reboot/shutdown. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03, H-04, H-05, H-06.
  - Acceptance: A source-and-evidence comparison against the complete requirement: capability-gated orderly service/device/filesystem teardown with forced fallback
- [ ] **F-KR-040.02 — Freeze the exact contract for reboot/shutdown**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: capability-gated orderly service/device/filesystem teardown with forced fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-KR-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-KR-040.03 — Implement/prove: capability-gated orderly service/device/filesystem teardown with forced fallback**
  - Action: For reboot/shutdown, implement or reuse and verify this exact obligation: capability-gated orderly service/device/filesystem teardown with forced fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-KR-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capability-gated orderly service/device/filesystem teardown with forced fallback; retain observable state/resource expectations.
- [ ] **F-KR-040.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reboot/shutdown: nth allocation/page-write failure; bad user pointer spanning a boundary; stale full-width handle; parent/child death; hostile CPU state; IRQ/preemption races; exhaustion and teardown leaks. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-KR-040.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-KR-040.05 — Integrate into the real consumer and runtime route**
  - Action: Wire reboot/shutdown into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-KR-040.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-KR-040.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reboot/shutdown as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-KR-040.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p3-1"></a>
## C-P3.1 — syscall entry, kernel stack and return

**Original requirement:** syscall entry, kernel stack and return

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 184.

### Preserved original contract

- **Dependencies/current/provenance:** current 32-bit Ring-3 proof, GDT/TSS/IDT and x86-64 boot; Serenity/Brook process boundaries; reject Rings 1/2 and premature `sysret`.
- **I/O and state:** validated syscall number/register frame in; result/error and restored user frame out; `User -> EntryStub -> KernelStack -> Dispatch -> ReturnCheck -> User|Killed`.
- **Invariants/failure:** syscall index is unsigned and checked before scaling/indexing; interrupts/flags/canonical addresses checked; unknown/null slot returns `ENOSYS`; generated table length matches dispatch; kernel stack per running thread; no user stack trusted during dispatch.
- **Deterministic proof:** `-1`, sign-bit/`INT64_MIN`, maximum, every valid number and gap, null entry, malformed return RIP/RSP/RFLAGS, nested interrupt, generated table length/non-null assertion, TacOS negative-index and protOS NULL-unshare regressions.
- **Target proof:** QEMU UEFI64 round trip and malicious return corpus; hardware boot retains exact fault/return records.
- **Receipt/removal:** ABI version/register transcript; conservative `iretq` remains rollback; add `sysret` only as separately gated optimization.

### Execution steps

- [ ] **C-P3.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P3.1.02 — Resolve this contract's exact dependencies**
  - Action: current 32-bit Ring-3 proof, GDT/TSS/IDT and x86-64 boot; Serenity/Brook process boundaries; reject Rings 1/2 and premature `sysret`. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P3.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P3.1.03 — I/O and state — syscall entry, kernel stack and return**
  - Action: validated syscall number/register frame in; result/error and restored user frame out; `User -> EntryStub -> KernelStack -> Dispatch -> ReturnCheck -> User|Killed`.
  - Requires: C-P3.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P3.1.
- [ ] **C-P3.1.04 — Invariants/failure — syscall entry, kernel stack and return**
  - Action: syscall index is unsigned and checked before scaling/indexing; interrupts/flags/canonical addresses checked; unknown/null slot returns `ENOSYS`; generated table length matches dispatch; kernel stack per running thread; no user stack trusted during dispatch.
  - Requires: C-P3.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P3.1.
- [ ] **C-P3.1.05 — Deterministic proof — syscall entry, kernel stack and return**
  - Action: `-1`, sign-bit/`INT64_MIN`, maximum, every valid number and gap, null entry, malformed return RIP/RSP/RFLAGS, nested interrupt, generated table length/non-null assertion, TacOS negative-index and protOS NULL-unshare regressions.
  - Requires: C-P3.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P3.1.
- [ ] **C-P3.1.06 — Target proof — syscall entry, kernel stack and return**
  - Action: QEMU UEFI64 round trip and malicious return corpus; hardware boot retains exact fault/return records.
  - Requires: C-P3.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P3.1.
- [ ] **C-P3.1.07 — Receipt/removal — syscall entry, kernel stack and return**
  - Action: ABI version/register transcript; conservative `iretq` remains rollback; add `sysret` only as separately gated optimization.
  - Requires: C-P3.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P3.1.
- [ ] **C-P3.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P3.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P3.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p3-2"></a>
## C-P3.2 — process object and address space

**Original requirement:** process object and address space

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 193.

### Preserved original contract

- **Dependencies/current/provenance:** P3.1, PMM/VMM/scheduler; RustOS atomic mapping, NexiOS ownership; reject fixed shared user stacks and ambient mappings.
- **I/O and state:** image/manifest/limits/parent in; PID, PML4, stacks, handles and state out; `New -> Loading -> Runnable -> Running/Blocked -> Exiting -> Reaped`.
- **Invariants/failure:** one PML4 per process; supervisor-only kernel/device; W^X/NX; guarded unique stacks; full-span admission before commit; cleanup reverses partial load.
- **Deterministic proof:** overlap/overflow/OOM plus fail map/remap/unmap/flush/shootdown on every page and compare logical ranges/PTEs/object refs; W+X denial, stack collision, exact fork credential/mapping copy, exit during block, sanitizer lifetime tests including Skift/Zinnia partial-map and protOS exit-UAF regressions.
- **Target proof:** QEMU starts multiple processes and kills one without desktop impact; hardware receipt records CR3/process identity transitions.
- **Receipt/removal:** mapping manifest and leak counters; process feature flag retains current kernel apps; no removal until P3.4 and Phase 4 gates.

### Execution steps

- [ ] **C-P3.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P3.2.02 — Resolve this contract's exact dependencies**
  - Action: P3.1, PMM/VMM/scheduler; RustOS atomic mapping, NexiOS ownership; reject fixed shared user stacks and ambient mappings. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P3.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P3.2.03 — I/O and state — process object and address space**
  - Action: image/manifest/limits/parent in; PID, PML4, stacks, handles and state out; `New -> Loading -> Runnable -> Running/Blocked -> Exiting -> Reaped`.
  - Requires: C-P3.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P3.2.
- [ ] **C-P3.2.04 — Invariants/failure — process object and address space**
  - Action: one PML4 per process; supervisor-only kernel/device; W^X/NX; guarded unique stacks; full-span admission before commit; cleanup reverses partial load.
  - Requires: C-P3.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P3.2.
- [ ] **C-P3.2.05 — Deterministic proof — process object and address space**
  - Action: overlap/overflow/OOM plus fail map/remap/unmap/flush/shootdown on every page and compare logical ranges/PTEs/object refs; W+X denial, stack collision, exact fork credential/mapping copy, exit during block, sanitizer lifetime tests including Skift/Zinnia partial-map and protOS exit-UAF regressions.
  - Requires: C-P3.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P3.2.
- [ ] **C-P3.2.06 — Target proof — process object and address space**
  - Action: QEMU starts multiple processes and kills one without desktop impact; hardware receipt records CR3/process identity transitions.
  - Requires: C-P3.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P3.2.
- [ ] **C-P3.2.07 — Receipt/removal — process object and address space**
  - Action: mapping manifest and leak counters; process feature flag retains current kernel apps; no removal until P3.4 and Phase 4 gates.
  - Requires: C-P3.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P3.2.
- [ ] **C-P3.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P3.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P3.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p3-3"></a>
## C-P3.3 — central usercopy and fault containment

**Original requirement:** central usercopy and fault containment

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 202.

### Preserved original contract

- **Dependencies/current/provenance:** P3.2 and page-fault handler; Serenity fault outcomes and protOS negative evidence; reject raw `strlen/memcpy/VFS` on user pointers.
- **I/O and state:** process plus pointer/span/direction in; copied bytes or typed fault out; `ValidateEntireSpan -> Pin/Copy -> Unpin -> Complete`.
- **Invariants/failure:** checked arithmetic and every page/protection before mutation; bounded strings/slices; validation failure dominates access; copy faults are contained; only the offending process can die; partial output policy explicit and never reported as success.
- **Deterministic proof:** null/noncanonical/cross-page/first-byte-unmapped/last-byte-unmapped/read-only-destination/one-past-VMO/overflow/racing-unmap input and output buffers; fault during copyout; fuzz all syscall shapes; Cyjon/Brook/hhuOS/duckOS regressions.
- **Target proof:** QEMU hostile corpus proves desktop/system and unrelated processes continue after each offender; ThinkPad fault retained without system halt.
- **Receipt/removal:** per-case fault/result and no kernel panic; no unsafe compatibility bypass; direct pointer handlers removed when syscall inventory proves none.

### Execution steps

- [ ] **C-P3.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P3.3.02 — Resolve this contract's exact dependencies**
  - Action: P3.2 and page-fault handler; Serenity fault outcomes and protOS negative evidence; reject raw `strlen/memcpy/VFS` on user pointers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P3.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P3.3.03 — I/O and state — central usercopy and fault containment**
  - Action: process plus pointer/span/direction in; copied bytes or typed fault out; `ValidateEntireSpan -> Pin/Copy -> Unpin -> Complete`.
  - Requires: C-P3.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P3.3.
- [ ] **C-P3.3.04 — Invariants/failure — central usercopy and fault containment**
  - Action: checked arithmetic and every page/protection before mutation; bounded strings/slices; validation failure dominates access; copy faults are contained; only the offending process can die; partial output policy explicit and never reported as success.
  - Requires: C-P3.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P3.3.
- [ ] **C-P3.3.05 — Deterministic proof — central usercopy and fault containment**
  - Action: null/noncanonical/cross-page/first-byte-unmapped/last-byte-unmapped/read-only-destination/one-past-VMO/overflow/racing-unmap input and output buffers; fault during copyout; fuzz all syscall shapes; Cyjon/Brook/hhuOS/duckOS regressions.
  - Requires: C-P3.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P3.3.
- [ ] **C-P3.3.06 — Target proof — central usercopy and fault containment**
  - Action: QEMU hostile corpus proves desktop/system and unrelated processes continue after each offender; ThinkPad fault retained without system halt.
  - Requires: C-P3.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P3.3.
- [ ] **C-P3.3.07 — Receipt/removal — central usercopy and fault containment**
  - Action: per-case fault/result and no kernel panic; no unsafe compatibility bypass; direct pointer handlers removed when syscall inventory proves none.
  - Requires: C-P3.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P3.3.
- [ ] **C-P3.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P3.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P3.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p3-4"></a>
## C-P3.4 — ELF admission and minimal zl process

**Original requirement:** ELF admission and minimal zl process

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 211.

### Preserved original contract

- **Dependencies/current/provenance:** P3.1-P3.3 and compiler ELF output; Hyper assertion payload and Brook host shape; Zinnia/TacOS/Cyjon/Fudge/protOS are negative parser inputs; reject magic-only, short entry reads and unchecked offsets.
- **I/O and state:** byte slice plus ABI/manifest in; admitted segment plan/entry or precise rejection out; `Parse -> ValidateAll -> Reserve -> Map/Copy/Zero -> Commit`.
- **Invariants/failure:** magic/class/data/version/machine/header sizes; bounded tables; `filesz <= memsz`; checked offsets/addresses; no overlap/W+X; no mappings before total validation.
- **Deterministic proof:** mutation/fuzz corpus for undersized/zero/wrong entry sizes, count-by-size and offset overflow, `filesz > memsz`, overlap/noncanonical/W+X/bad entry, valid PIE/static fixtures, concurrent exec, OOM rollback, symbol data bounds.
- **Target proof:** assertion process runs/yields/exits; malformed image kills loader request, not desktop.
- **Receipt/removal:** image/plan/mapping digests and rejection code; current embedded/kernel apps remain rollback until process migrations complete.

### Execution steps

- [ ] **C-P3.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P3.4.02 — Resolve this contract's exact dependencies**
  - Action: P3.1-P3.3 and compiler ELF output; Hyper assertion payload and Brook host shape; Zinnia/TacOS/Cyjon/Fudge/protOS are negative parser inputs; reject magic-only, short entry reads and unchecked offsets. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P3.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P3.4.03 — I/O and state — ELF admission and minimal zl process**
  - Action: byte slice plus ABI/manifest in; admitted segment plan/entry or precise rejection out; `Parse -> ValidateAll -> Reserve -> Map/Copy/Zero -> Commit`.
  - Requires: C-P3.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P3.4.
- [ ] **C-P3.4.04 — Invariants/failure — ELF admission and minimal zl process**
  - Action: magic/class/data/version/machine/header sizes; bounded tables; `filesz <= memsz`; checked offsets/addresses; no overlap/W+X; no mappings before total validation.
  - Requires: C-P3.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P3.4.
- [ ] **C-P3.4.05 — Deterministic proof — ELF admission and minimal zl process**
  - Action: mutation/fuzz corpus for undersized/zero/wrong entry sizes, count-by-size and offset overflow, `filesz > memsz`, overlap/noncanonical/W+X/bad entry, valid PIE/static fixtures, concurrent exec, OOM rollback, symbol data bounds.
  - Requires: C-P3.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P3.4.
- [ ] **C-P3.4.06 — Target proof — ELF admission and minimal zl process**
  - Action: assertion process runs/yields/exits; malformed image kills loader request, not desktop.
  - Requires: C-P3.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P3.4.
- [ ] **C-P3.4.07 — Receipt/removal — ELF admission and minimal zl process**
  - Action: image/plan/mapping digests and rejection code; current embedded/kernel apps remain rollback until process migrations complete.
  - Requires: C-P3.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P3.4.
- [ ] **C-P3.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P3.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P3.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-17"></a>
## C-DA-17 — hostile user boundary

**Original requirement:** hostile user boundary

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 421.

### Preserved original contract

**Depends on:** UEFI64 processes, address spaces, usercopy, handles and scheduler.

**Deliver:** one checked string/slice/copy layer and generated syscall dispatch.

**Invariants:** negative/sign-bit/gap/null syscall indices return a typed error;
kernel never dereferences raw user pointers; validation failure dominates access;
only the offending process dies on a copy fault.

**Proof:** `-1`, minimum integer, maximum, gaps, null entry; first/last byte unmapped,
read-only output, one-past object, arithmetic wrap, copyout fault and desktop
survival.

### Execution steps

- [ ] **C-DA-17.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-17.02 — Resolve this contract's exact dependencies**
  - Action: UEFI64 processes, address spaces, usercopy, handles and scheduler. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-17.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-17.03 — Deliver — hostile user boundary**
  - Action: one checked string/slice/copy layer and generated syscall dispatch.
  - Requires: C-DA-17.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-17.
- [ ] **C-DA-17.04 — Invariants — hostile user boundary**
  - Action: negative/sign-bit/gap/null syscall indices return a typed error; kernel never dereferences raw user pointers; validation failure dominates access; only the offending process dies on a copy fault.
  - Requires: C-DA-17.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-17.
- [ ] **C-DA-17.05 — Proof — hostile user boundary**
  - Action: `-1`, minimum integer, maximum, gaps, null entry; first/last byte unmapped, read-only output, one-past object, arithmetic wrap, copyout fault and desktop survival.
  - Requires: C-DA-17.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-17.
- [ ] **C-DA-17.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-17. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-17.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-17l"></a>
## C-DA-17L — hostile executable loader

**Original requirement:** hostile executable loader

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 435.

### Preserved original contract

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

### Execution steps

- [ ] **C-DA-17L.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-17L.02 — Resolve this contract's exact dependencies**
  - Action: DA-17, File/VFS handles and transactional VM mapping. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-17L.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-17L.03 — Deliver — hostile executable loader**
  - Action: length-first ELF loader and, only if retained as a product goal, a separately scoped PE loader. Validate complete headers, architecture/ABI, entry sizes/counts, all offset+size arithmetic, segment/section/import/relocation spans, entrypoint and permissions before constructing a private image and atomically publishing the process.
  - Requires: C-DA-17L.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-17L.
- [ ] **C-DA-17L.04 — Invariants — hostile executable loader**
  - Action: `filesz <= memsz`; mapped spans are canonical, non-overlapping and within admitted image limits; W+X is rejected by policy; zero/nonadvancing entry sizes fail; duplicate module/import ownership is explicit; partial load leaves no PID, mappings, pages, files or handles.
  - Requires: C-DA-17L.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-17L.
- [ ] **C-DA-17L.05 — Proof — hostile executable loader**
  - Action: every wrong magic/class/data/machine/header/entry size, zero/wrong entry size, count*size and offset+size overflow, truncated table, overlap, W+X, noncanonical/bad entry, `filesz > memsz`, zero-step relocation, missing/duplicate import, concurrent exec and failure at every allocation/map/relocation/publish step.
  - Requires: C-DA-17L.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-17L.
- [ ] **C-DA-17L.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-17L. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-17L.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-plat-011"></a>
## T-PLAT-011 — x86 topology/SMP provider

**Original requirement:** staged online/offline, IPI/TLB/barrier proof

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 40.

### Execution steps

- [ ] **T-PLAT-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86 topology/SMP provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-011.02 — Specify the complete target boundary**
  - Action: x86 topology/SMP provider must supply: staged online/offline, IPI/TLB/barrier proof. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86 topology/SMP provider through the shared platform contract, delivering every part of: staged online/offline, IPI/TLB/barrier proof. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86 topology/SMP provider.
- [ ] **T-PLAT-011.05 — Qualify and retain this target's own result**
  - Action: Bind x86 topology/SMP provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-023"></a>
## T-PLAT-023 — TSC/deadline timer

**Original requirement:** invariant calibration, drift and fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 52.

### Execution steps

- [ ] **T-PLAT-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve TSC/deadline timer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-05, D-06, D-07, H-03.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-023.02 — Specify the complete target boundary**
  - Action: TSC/deadline timer must supply: invariant calibration, drift and fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse TSC/deadline timer through the shared platform contract, delivering every part of: invariant calibration, drift and fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-023.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for TSC/deadline timer.
- [ ] **T-PLAT-023.05 — Qualify and retain this target's own result**
  - Action: Bind TSC/deadline timer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
