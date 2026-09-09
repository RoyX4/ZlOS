# MP-16: Complete the zl systems language and toolchain

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `src/frontend/; src/runtime/; src/backends/; src/selfhost/; stdlib/; editors/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-16` exports: Host-tested ABI/data/code-generation primitives and object emission for downstream work. Running the compiler inside zlOS belongs to M-16.09 and is not supplied by this early handoff.

The handoff enables only its named subset. `CLOSE-16` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-16.01 — Audit live language ownership against retained language plans

Audit live language ownership against retained language plans; preserve locked syntax and existing self-hosting proof; exclude archived unboxed experiments from active implementation.

**Requires:** `D-01`, `D-02`, `D-21`, `D-22`, `H-00`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.02 — Resolve imports, scopes, functions, records, variants and explicit error semantics before adding dependent conveniences

Resolve imports, scopes, functions, records, variants and explicit error semantics before adding dependent conveniences.

**Requires:** `M-16.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.03 — Implement exact integers, bytes, text, layouts, pointers, allocation and lifetime rules consistently across active engines

Implement exact integers, bytes, text, layouts, pointers, allocation and lifetime rules consistently across active engines.

**Requires:** `M-16.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.04 — Specify and implement atomics, volatile MMIO, barriers, inline assembly, FFI and callback lifetime at explicit unsafe boundaries

Specify and implement atomics, volatile MMIO, barriers, inline assembly, FFI and callback lifetime at explicit unsafe boundaries.

**Requires:** `M-16.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.05 — Add object/executable/relocation/section/debug/unwind emission with independent inspection and loader round trips

Add object/executable/relocation/section/debug/unwind emission with independent inspection and loader round trips.

**Requires:** `M-16.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-16 — Bounded development handoff: Complete the zl systems language and toolchain

Host-tested ABI/data/code-generation primitives and object emission for downstream work. Running the compiler inside zlOS belongs to M-16.09 and is not supplied by this early handoff.

**Requires:** `M-16.05`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-16.06 — Preserve interpreter/backend semantic parity and deterministic compiler fixpoints through every lowering/optimization change

Preserve interpreter/backend semantic parity and deterministic compiler fixpoints through every lowering/optimization change.

**Requires:** `M-16.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.07 — Build platform-split standard libraries for Linux, freestanding code and zlOS handles/services

Build platform-split standard libraries for Linux, freestanding code and zlOS handles/services.

**Requires:** `M-16.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.08 — Add package resolution, formatter/linter/docs/LSP/test tooling with immutable inputs and strict error propagation

Add package resolution, formatter/linter/docs/LSP/test tooling with immutable inputs and strict error propagation.

**Requires:** `M-16.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.09 — Port the compiler/build tools into isolated zlOS processes only after the native process/file/tool ABI handoffs exist

Port the compiler/build tools into isolated zlOS processes only after the native process/file/tool ABI handoffs exist.

**Requires:** `M-16.08`, `H-03`, `H-04`, `H-06`, `H-15`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-16.10 — Measure compile/runtime memory, size and time

Measure compile/runtime memory, size and time; retain a trusted seed and an explicit bootstrap chain for final self-hosting.

**Requires:** `M-16.09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-ZL-001](#f-zl-001) | feature | lexical grammar |
| [F-ZL-002](#f-zl-002) | feature | parser/AST |
| [F-ZL-003](#f-zl-003) | feature | module/import identity |
| [F-ZL-004](#f-zl-004) | feature | local/global scope |
| [F-ZL-005](#f-zl-005) | feature | functions/calls |
| [F-ZL-006](#f-zl-006) | feature | records/structs |
| [F-ZL-007](#f-zl-007) | feature | enums/sum types |
| [F-ZL-008](#f-zl-008) | feature | option/result/error values |
| [F-ZL-009](#f-zl-009) | feature | generics/templates |
| [F-ZL-010](#f-zl-010) | feature | traits/interfaces |
| [F-ZL-011](#f-zl-011) | feature | exact-width integers |
| [F-ZL-012](#f-zl-012) | feature | floating point |
| [F-ZL-013](#f-zl-013) | feature | booleans/chars |
| [F-ZL-014](#f-zl-014) | feature | byte slices |
| [F-ZL-015](#f-zl-015) | feature | UTF-8 strings |
| [F-ZL-016](#f-zl-016) | feature | arrays/vectors/maps |
| [F-ZL-017](#f-zl-017) | feature | pointers/references |
| [F-ZL-018](#f-zl-018) | feature | packed/aligned layouts |
| [F-ZL-019](#f-zl-019) | feature | volatile access |
| [F-ZL-020](#f-zl-020) | feature | atomics/memory ordering |
| [F-ZL-021](#f-zl-021) | feature | compiler/CPU barriers |
| [F-ZL-022](#f-zl-022) | feature | inline assembly |
| [F-ZL-023](#f-zl-023) | feature | extern/FFI |
| [F-ZL-024](#f-zl-024) | feature | named sections/linker symbols |
| [F-ZL-025](#f-zl-025) | feature | object emission |
| [F-ZL-026](#f-zl-026) | feature | executable emission |
| [F-ZL-027](#f-zl-027) | feature | relocation emission |
| [F-ZL-028](#f-zl-028) | feature | debug metadata |
| [F-ZL-029](#f-zl-029) | feature | unwind metadata |
| [F-ZL-030](#f-zl-030) | feature | interpreter |
| [F-ZL-031](#f-zl-031) | feature | native backend |
| [F-ZL-032](#f-zl-032) | feature | LLVM backend |
| [F-ZL-033](#f-zl-033) | feature | optimizer |
| [F-ZL-034](#f-zl-034) | feature | compile-time evaluation |
| [F-ZL-035](#f-zl-035) | feature | code generation/IDL |
| [F-ZL-036](#f-zl-036) | feature | memory allocation API |
| [F-ZL-037](#f-zl-037) | feature | ownership/lifetime helpers |
| [F-ZL-038](#f-zl-038) | feature | concurrency primitives |
| [F-ZL-039](#f-zl-039) | feature | async/deadline model |
| [F-ZL-040](#f-zl-040) | feature | standard library core |
| [F-ZL-041](#f-zl-041) | feature | freestanding standard library |
| [F-ZL-042](#f-zl-042) | feature | Linux platform library |
| [F-ZL-043](#f-zl-043) | feature | zlOS platform library |
| [F-ZL-044](#f-zl-044) | feature | package manifest/resolver |
| [F-ZL-045](#f-zl-045) | feature | formatter/linter |
| [F-ZL-046](#f-zl-046) | feature | documentation generator |
| [F-ZL-047](#f-zl-047) | feature | language server |
| [F-ZL-048](#f-zl-048) | feature | test framework |
| [F-ZL-049](#f-zl-049) | feature | compatibility/versioning |
| [F-ZL-050](#f-zl-050) | feature | bootstrap seed |
| [F-ZL-051](#f-zl-051) | feature | compiler fixpoint |
| [F-ZL-052](#f-zl-052) | feature | self-hosted system build |
| [F-ZL-053](#f-zl-053) | feature | multiarchitecture targets |
| [F-ZL-054](#f-zl-054) | feature | language security audit |
| [F-ZL-055](#f-zl-055) | feature | language performance lab |
| [C-P9.1](#c-p9-1) | contract | imports, scope/errors and data contracts |
| [C-P9.2](#c-p9-2) | contract | exact integers, bytes and arithmetic modes |
| [C-P9.3](#c-p9-3) | contract | native bitwise, volatile MMIO, ports and barriers |
| [C-P9.4](#c-p9-4) | contract | layouts, calling conventions and function values |
| [C-P9.5](#c-p9-5) | contract | ELF64 executable/relocatable and named sections |
| [C-P9.6](#c-p9-6) | contract | first zl provider and fixpoint preservation |
| [T-SVC-124](#t-svc-124) | target | Build Service |
| [T-APP-081](#t-app-081) | target | Compiler/Build Monitor |
| [T-APP-086](#t-app-086) | target | ABI/Provenance Inspector |
| [T-ZLP-001](#t-zlp-001) | target | module/import system |
| [T-ZLP-002](#t-zlp-002) | target | namespaces/scopes |
| [T-ZLP-003](#t-zlp-003) | target | source locations/diagnostics |
| [T-ZLP-004](#t-zlp-004) | target | error/result model |
| [T-ZLP-005](#t-zlp-005) | target | exact integers |
| [T-ZLP-006](#t-zlp-006) | target | byte and byte-string types |
| [T-ZLP-007](#t-zlp-007) | target | arithmetic modes |
| [T-ZLP-008](#t-zlp-008) | target | enums/tagged unions |
| [T-ZLP-009](#t-zlp-009) | target | generics/interfaces |
| [T-ZLP-010](#t-zlp-010) | target | function values/closures |
| [T-ZLP-011](#t-zlp-011) | target | compile-time evaluation |
| [T-ZLP-012](#t-zlp-012) | target | attributes/annotations |
| [T-ZLP-013](#t-zlp-013) | target | conditional compilation |
| [T-ZLP-014](#t-zlp-014) | target | package/module documentation |
| [T-ZLP-020](#t-zlp-020) | target | packed/aligned records |
| [T-ZLP-021](#t-zlp-021) | target | bitfields |
| [T-ZLP-022](#t-zlp-022) | target | typed raw pointers |
| [T-ZLP-023](#t-zlp-023) | target | volatile MMIO access |
| [T-ZLP-024](#t-zlp-024) | target | port I/O intrinsics |
| [T-ZLP-025](#t-zlp-025) | target | memory/compiler barriers |
| [T-ZLP-026](#t-zlp-026) | target | atomic operations |
| [T-ZLP-027](#t-zlp-027) | target | calling-convention declarations |
| [T-ZLP-028](#t-zlp-028) | target | external symbol/assembly declarations |
| [T-ZLP-029](#t-zlp-029) | target | TLS/per-thread data |
| [T-ZLP-030](#t-zlp-030) | target | endian/serialization primitives |
| [T-ZLP-031](#t-zlp-031) | target | ownership/lifetime helpers |
| [T-ZLP-040](#t-zlp-040) | target | ELF64 executable output |
| [T-ZLP-041](#t-zlp-041) | target | ELF64 relocatable output |
| [T-ZLP-042](#t-zlp-042) | target | shared-object/PIC output |
| [T-ZLP-043](#t-zlp-043) | target | named/custom sections |
| [T-ZLP-044](#t-zlp-044) | target | archive librarian |
| [T-ZLP-045](#t-zlp-045) | target | zl linker |
| [T-ZLP-046](#t-zlp-046) | target | debug information |
| [T-ZLP-047](#t-zlp-047) | target | object/binary inspection tools |
| [T-ZLP-048](#t-zlp-048) | target | dependency/build metadata |
| [T-ZLP-049](#t-zlp-049) | target | incremental build/cache |
| [T-ZLP-050](#t-zlp-050) | target | zl formatter/linter |
| [T-ZLP-051](#t-zlp-051) | target | language server |
| [T-ZLP-052](#t-zlp-052) | target | package manager integration |
| [T-ZLP-053](#t-zlp-053) | target | zlIDL generator |
| [T-ZLP-054](#t-zlp-054) | target | compiler hostile corpus |
| [T-ZLP-055](#t-zlp-055) | target | fixpoint and bootstrap ledger |

<a id="f-zl-001"></a>
## F-ZL-001 — lexical grammar

**Original requirement:** exact tokens/comments/literals/Unicode/error recovery shared across stages

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-001.01 — Reconcile existing lexical grammar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for lexical grammar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact tokens/comments/literals/Unicode/error recovery shared across stages
- [ ] **F-ZL-001.02 — Freeze the exact contract for lexical grammar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact tokens/comments/literals/Unicode/error recovery shared across stages. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-001.03 — Implement/prove: exact tokens/comments/literals/Unicode/error recovery shared across stages**
  - Action: For lexical grammar, implement or reuse and verify this exact obligation: exact tokens/comments/literals/Unicode/error recovery shared across stages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact tokens/comments/literals/Unicode/error recovery shared across stages; retain observable state/resource expectations.
- [ ] **F-ZL-001.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to lexical grammar: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-001.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-001.05 — Integrate into the real consumer and runtime route**
  - Action: Wire lexical grammar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-001.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-001.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for lexical grammar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-001.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-002"></a>
## F-ZL-002 — parser/AST

**Original requirement:** deterministic grammar, source spans, diagnostics, malformed corpus and no false recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-002.01 — Reconcile existing parser/AST**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for parser/AST. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic grammar, source spans, diagnostics, malformed corpus and no false recovery
- [ ] **F-ZL-002.02 — Freeze the exact contract for parser/AST**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic grammar, source spans, diagnostics, malformed corpus and no false recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-002.03 — Implement/prove: deterministic grammar**
  - Action: For parser/AST, implement or reuse and verify this exact obligation: deterministic grammar. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic grammar; retain observable state/resource expectations.
- [ ] **F-ZL-002.04 — Implement/prove: source spans**
  - Action: For parser/AST, implement or reuse and verify this exact obligation: source spans. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source spans; retain observable state/resource expectations.
- [ ] **F-ZL-002.05 — Implement/prove: diagnostics**
  - Action: For parser/AST, implement or reuse and verify this exact obligation: diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics; retain observable state/resource expectations.
- [ ] **F-ZL-002.06 — Implement/prove: malformed corpus and no false recovery**
  - Action: For parser/AST, implement or reuse and verify this exact obligation: malformed corpus and no false recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-002.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for malformed corpus and no false recovery; retain observable state/resource expectations.
- [ ] **F-ZL-002.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to parser/AST: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-002.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-002.08 — Integrate into the real consumer and runtime route**
  - Action: Wire parser/AST into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-002.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-002.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for parser/AST as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-002.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-003"></a>
## F-ZL-003 — module/import identity

**Original requirement:** canonical paths/namespaces, cycles, visibility, reproducible resolution and platform packages

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-003.01 — Reconcile existing module/import identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for module/import identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: canonical paths/namespaces, cycles, visibility, reproducible resolution and platform packages
- [ ] **F-ZL-003.02 — Freeze the exact contract for module/import identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: canonical paths/namespaces, cycles, visibility, reproducible resolution and platform packages. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-003.03 — Implement/prove: canonical paths/namespaces**
  - Action: For module/import identity, implement or reuse and verify this exact obligation: canonical paths/namespaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for canonical paths/namespaces; retain observable state/resource expectations.
- [ ] **F-ZL-003.04 — Implement/prove: cycles**
  - Action: For module/import identity, implement or reuse and verify this exact obligation: cycles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cycles; retain observable state/resource expectations.
- [ ] **F-ZL-003.05 — Implement/prove: visibility**
  - Action: For module/import identity, implement or reuse and verify this exact obligation: visibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for visibility; retain observable state/resource expectations.
- [ ] **F-ZL-003.06 — Implement/prove: reproducible resolution and platform packages**
  - Action: For module/import identity, implement or reuse and verify this exact obligation: reproducible resolution and platform packages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reproducible resolution and platform packages; retain observable state/resource expectations.
- [ ] **F-ZL-003.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to module/import identity: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-003.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-003.08 — Integrate into the real consumer and runtime route**
  - Action: Wire module/import identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-003.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-003.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for module/import identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-003.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-004"></a>
## F-ZL-004 — local/global scope

**Original requirement:** local by default, explicit global mutation, closures and shadowing semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-004.01 — Reconcile existing local/global scope**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for local/global scope. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: local by default, explicit global mutation, closures and shadowing semantics
- [ ] **F-ZL-004.02 — Freeze the exact contract for local/global scope**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: local by default, explicit global mutation, closures and shadowing semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-004.03 — Implement/prove: local by default**
  - Action: For local/global scope, implement or reuse and verify this exact obligation: local by default. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for local by default; retain observable state/resource expectations.
- [ ] **F-ZL-004.04 — Implement/prove: explicit global mutation**
  - Action: For local/global scope, implement or reuse and verify this exact obligation: explicit global mutation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit global mutation; retain observable state/resource expectations.
- [ ] **F-ZL-004.05 — Implement/prove: closures and shadowing semantics**
  - Action: For local/global scope, implement or reuse and verify this exact obligation: closures and shadowing semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for closures and shadowing semantics; retain observable state/resource expectations.
- [ ] **F-ZL-004.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to local/global scope: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-004.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-004.07 — Integrate into the real consumer and runtime route**
  - Action: Wire local/global scope into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-004.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-004.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for local/global scope as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-004.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-005"></a>
## F-ZL-005 — functions/calls

**Original requirement:** signatures, defaults/variadics if selected, recursion, errors and ABI lowering

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-005.01 — Reconcile existing functions/calls**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for functions/calls. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signatures, defaults/variadics if selected, recursion, errors and ABI lowering
- [ ] **F-ZL-005.02 — Freeze the exact contract for functions/calls**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signatures, defaults/variadics if selected, recursion, errors and ABI lowering. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-005.03 — Implement/prove: signatures**
  - Action: For functions/calls, implement or reuse and verify this exact obligation: signatures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signatures; retain observable state/resource expectations.
- [ ] **F-ZL-005.04 — Implement/prove: defaults/variadics if selected**
  - Action: For functions/calls, implement or reuse and verify this exact obligation: defaults/variadics if selected. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defaults/variadics if selected; retain observable state/resource expectations.
- [ ] **F-ZL-005.05 — Implement/prove: recursion**
  - Action: For functions/calls, implement or reuse and verify this exact obligation: recursion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recursion; retain observable state/resource expectations.
- [ ] **F-ZL-005.06 — Implement/prove: errors and ABI lowering**
  - Action: For functions/calls, implement or reuse and verify this exact obligation: errors and ABI lowering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors and ABI lowering; retain observable state/resource expectations.
- [ ] **F-ZL-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to functions/calls: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire functions/calls into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for functions/calls as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-006"></a>
## F-ZL-006 — records/structs

**Original requirement:** named fields, construction, access, layout-independent semantics and ABI representation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-006.01 — Reconcile existing records/structs**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for records/structs. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: named fields, construction, access, layout-independent semantics and ABI representation
- [ ] **F-ZL-006.02 — Freeze the exact contract for records/structs**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: named fields, construction, access, layout-independent semantics and ABI representation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-006.03 — Implement/prove: named fields**
  - Action: For records/structs, implement or reuse and verify this exact obligation: named fields. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for named fields; retain observable state/resource expectations.
- [ ] **F-ZL-006.04 — Implement/prove: construction**
  - Action: For records/structs, implement or reuse and verify this exact obligation: construction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for construction; retain observable state/resource expectations.
- [ ] **F-ZL-006.05 — Implement/prove: access**
  - Action: For records/structs, implement or reuse and verify this exact obligation: access. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for access; retain observable state/resource expectations.
- [ ] **F-ZL-006.06 — Implement/prove: layout-independent semantics and ABI representation**
  - Action: For records/structs, implement or reuse and verify this exact obligation: layout-independent semantics and ABI representation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layout-independent semantics and ABI representation; retain observable state/resource expectations.
- [ ] **F-ZL-006.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to records/structs: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-006.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-006.08 — Integrate into the real consumer and runtime route**
  - Action: Wire records/structs into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-006.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-006.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for records/structs as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-006.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-007"></a>
## F-ZL-007 — enums/sum types

**Original requirement:** exhaustive variants/payloads/matching and stable diagnostics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-007.01 — Reconcile existing enums/sum types**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for enums/sum types. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exhaustive variants/payloads/matching and stable diagnostics
- [ ] **F-ZL-007.02 — Freeze the exact contract for enums/sum types**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exhaustive variants/payloads/matching and stable diagnostics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-007.03 — Implement/prove: exhaustive variants/payloads/matching and stable diagnostics**
  - Action: For enums/sum types, implement or reuse and verify this exact obligation: exhaustive variants/payloads/matching and stable diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exhaustive variants/payloads/matching and stable diagnostics; retain observable state/resource expectations.
- [ ] **F-ZL-007.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to enums/sum types: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-007.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-007.05 — Integrate into the real consumer and runtime route**
  - Action: Wire enums/sum types into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-007.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-007.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for enums/sum types as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-007.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-008"></a>
## F-ZL-008 — option/result/error values

**Original requirement:** explicit absence/failure, propagation and FFI/syscall conversion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-008.01 — Reconcile existing option/result/error values**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for option/result/error values. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit absence/failure, propagation and FFI/syscall conversion
- [ ] **F-ZL-008.02 — Freeze the exact contract for option/result/error values**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit absence/failure, propagation and FFI/syscall conversion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-008.03 — Implement/prove: explicit absence/failure**
  - Action: For option/result/error values, implement or reuse and verify this exact obligation: explicit absence/failure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit absence/failure; retain observable state/resource expectations.
- [ ] **F-ZL-008.04 — Implement/prove: propagation and FFI/syscall conversion**
  - Action: For option/result/error values, implement or reuse and verify this exact obligation: propagation and FFI/syscall conversion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for propagation and FFI/syscall conversion; retain observable state/resource expectations.
- [ ] **F-ZL-008.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to option/result/error values: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-008.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-008.06 — Integrate into the real consumer and runtime route**
  - Action: Wire option/result/error values into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-008.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-008.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for option/result/error values as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-008.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-009"></a>
## F-ZL-009 — generics/templates

**Original requirement:** bounded compile-time instantiation, diagnostics, code size and ABI policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-009.01 — Reconcile existing generics/templates**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for generics/templates. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded compile-time instantiation, diagnostics, code size and ABI policy
- [ ] **F-ZL-009.02 — Freeze the exact contract for generics/templates**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded compile-time instantiation, diagnostics, code size and ABI policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-009.03 — Implement/prove: bounded compile-time instantiation**
  - Action: For generics/templates, implement or reuse and verify this exact obligation: bounded compile-time instantiation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded compile-time instantiation; retain observable state/resource expectations.
- [ ] **F-ZL-009.04 — Implement/prove: diagnostics**
  - Action: For generics/templates, implement or reuse and verify this exact obligation: diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics; retain observable state/resource expectations.
- [ ] **F-ZL-009.05 — Implement/prove: code size and ABI policy**
  - Action: For generics/templates, implement or reuse and verify this exact obligation: code size and ABI policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for code size and ABI policy; retain observable state/resource expectations.
- [ ] **F-ZL-009.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to generics/templates: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-009.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-009.07 — Integrate into the real consumer and runtime route**
  - Action: Wire generics/templates into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-009.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-009.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for generics/templates as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-009.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-010"></a>
## F-ZL-010 — traits/interfaces

**Original requirement:** explicit contracts, dispatch strategy, coherence and object representation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-010.01 — Reconcile existing traits/interfaces**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for traits/interfaces. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit contracts, dispatch strategy, coherence and object representation
- [ ] **F-ZL-010.02 — Freeze the exact contract for traits/interfaces**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit contracts, dispatch strategy, coherence and object representation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-010.03 — Implement/prove: explicit contracts**
  - Action: For traits/interfaces, implement or reuse and verify this exact obligation: explicit contracts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit contracts; retain observable state/resource expectations.
- [ ] **F-ZL-010.04 — Implement/prove: dispatch strategy**
  - Action: For traits/interfaces, implement or reuse and verify this exact obligation: dispatch strategy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dispatch strategy; retain observable state/resource expectations.
- [ ] **F-ZL-010.05 — Implement/prove: coherence and object representation**
  - Action: For traits/interfaces, implement or reuse and verify this exact obligation: coherence and object representation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for coherence and object representation; retain observable state/resource expectations.
- [ ] **F-ZL-010.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to traits/interfaces: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-010.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-010.07 — Integrate into the real consumer and runtime route**
  - Action: Wire traits/interfaces into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-010.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-010.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for traits/interfaces as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-010.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-011"></a>
## F-ZL-011 — exact-width integers

**Original requirement:** signed/unsigned widths and identical interpreter/native overflow modes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-011.01 — Reconcile existing exact-width integers**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for exact-width integers. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed/unsigned widths and identical interpreter/native overflow modes
- [ ] **F-ZL-011.02 — Freeze the exact contract for exact-width integers**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed/unsigned widths and identical interpreter/native overflow modes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-011.03 — Implement/prove: signed/unsigned widths and identical interpreter/native overflow modes**
  - Action: For exact-width integers, implement or reuse and verify this exact obligation: signed/unsigned widths and identical interpreter/native overflow modes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed/unsigned widths and identical interpreter/native overflow modes; retain observable state/resource expectations.
- [ ] **F-ZL-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to exact-width integers: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire exact-width integers into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for exact-width integers as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-012"></a>
## F-ZL-012 — floating point

**Original requirement:** format/rounding/NaN/conversion and deterministic target semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-012.01 — Reconcile existing floating point**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for floating point. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: format/rounding/NaN/conversion and deterministic target semantics
- [ ] **F-ZL-012.02 — Freeze the exact contract for floating point**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: format/rounding/NaN/conversion and deterministic target semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-012.03 — Implement/prove: format/rounding/NaN/conversion and deterministic target semantics**
  - Action: For floating point, implement or reuse and verify this exact obligation: format/rounding/NaN/conversion and deterministic target semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format/rounding/NaN/conversion and deterministic target semantics; retain observable state/resource expectations.
- [ ] **F-ZL-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to floating point: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire floating point into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for floating point as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-013"></a>
## F-ZL-013 — booleans/chars

**Original requirement:** distinct types, Unicode scalar policy and ABI representation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-013.01 — Reconcile existing booleans/chars**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for booleans/chars. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: distinct types, Unicode scalar policy and ABI representation
- [ ] **F-ZL-013.02 — Freeze the exact contract for booleans/chars**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: distinct types, Unicode scalar policy and ABI representation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-013.03 — Implement/prove: distinct types**
  - Action: For booleans/chars, implement or reuse and verify this exact obligation: distinct types. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for distinct types; retain observable state/resource expectations.
- [ ] **F-ZL-013.04 — Implement/prove: Unicode scalar policy and ABI representation**
  - Action: For booleans/chars, implement or reuse and verify this exact obligation: Unicode scalar policy and ABI representation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for Unicode scalar policy and ABI representation; retain observable state/resource expectations.
- [ ] **F-ZL-013.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to booleans/chars: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-013.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-013.06 — Integrate into the real consumer and runtime route**
  - Action: Wire booleans/chars into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-013.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-013.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for booleans/chars as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-013.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-014"></a>
## F-ZL-014 — byte slices

**Original requirement:** pointer+length ownership, checked indexing/arithmetic and no implicit C strings

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-014.01 — Reconcile existing byte slices**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for byte slices. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: pointer+length ownership, checked indexing/arithmetic and no implicit C strings
- [ ] **F-ZL-014.02 — Freeze the exact contract for byte slices**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: pointer+length ownership, checked indexing/arithmetic and no implicit C strings. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-014.03 — Implement/prove: pointer+length ownership**
  - Action: For byte slices, implement or reuse and verify this exact obligation: pointer+length ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pointer+length ownership; retain observable state/resource expectations.
- [ ] **F-ZL-014.04 — Implement/prove: checked indexing/arithmetic and no implicit C strings**
  - Action: For byte slices, implement or reuse and verify this exact obligation: checked indexing/arithmetic and no implicit C strings. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked indexing/arithmetic and no implicit C strings; retain observable state/resource expectations.
- [ ] **F-ZL-014.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to byte slices: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-014.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-014.06 — Integrate into the real consumer and runtime route**
  - Action: Wire byte slices into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-014.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-014.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for byte slices as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-014.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-015"></a>
## F-ZL-015 — UTF-8 strings

**Original requirement:** length-aware bytes/text boundary, validation, grapheme iteration and conversion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-015.01 — Reconcile existing UTF-8 strings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for UTF-8 strings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: length-aware bytes/text boundary, validation, grapheme iteration and conversion
- [ ] **F-ZL-015.02 — Freeze the exact contract for UTF-8 strings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: length-aware bytes/text boundary, validation, grapheme iteration and conversion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-015.03 — Implement/prove: length-aware bytes/text boundary**
  - Action: For UTF-8 strings, implement or reuse and verify this exact obligation: length-aware bytes/text boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for length-aware bytes/text boundary; retain observable state/resource expectations.
- [ ] **F-ZL-015.04 — Implement/prove: validation**
  - Action: For UTF-8 strings, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-ZL-015.05 — Implement/prove: grapheme iteration and conversion**
  - Action: For UTF-8 strings, implement or reuse and verify this exact obligation: grapheme iteration and conversion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grapheme iteration and conversion; retain observable state/resource expectations.
- [ ] **F-ZL-015.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to UTF-8 strings: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-015.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-015.07 — Integrate into the real consumer and runtime route**
  - Action: Wire UTF-8 strings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-015.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-015.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for UTF-8 strings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-015.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-016"></a>
## F-ZL-016 — arrays/vectors/maps

**Original requirement:** bounds, allocation/error, iteration, ownership and deterministic hashing policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-016.01 — Reconcile existing arrays/vectors/maps**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for arrays/vectors/maps. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounds, allocation/error, iteration, ownership and deterministic hashing policy
- [ ] **F-ZL-016.02 — Freeze the exact contract for arrays/vectors/maps**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounds, allocation/error, iteration, ownership and deterministic hashing policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-016.03 — Implement/prove: bounds**
  - Action: For arrays/vectors/maps, implement or reuse and verify this exact obligation: bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounds; retain observable state/resource expectations.
- [ ] **F-ZL-016.04 — Implement/prove: allocation/error**
  - Action: For arrays/vectors/maps, implement or reuse and verify this exact obligation: allocation/error. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allocation/error; retain observable state/resource expectations.
- [ ] **F-ZL-016.05 — Implement/prove: iteration**
  - Action: For arrays/vectors/maps, implement or reuse and verify this exact obligation: iteration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for iteration; retain observable state/resource expectations.
- [ ] **F-ZL-016.06 — Implement/prove: ownership and deterministic hashing policy**
  - Action: For arrays/vectors/maps, implement or reuse and verify this exact obligation: ownership and deterministic hashing policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-016.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ownership and deterministic hashing policy; retain observable state/resource expectations.
- [ ] **F-ZL-016.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to arrays/vectors/maps: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-016.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-016.08 — Integrate into the real consumer and runtime route**
  - Action: Wire arrays/vectors/maps into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-016.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-016.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for arrays/vectors/maps as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-016.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-017"></a>
## F-ZL-017 — pointers/references

**Original requirement:** explicit unsafe boundary, nullability, mutability, lifetime/ownership and casts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-017.01 — Reconcile existing pointers/references**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for pointers/references. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit unsafe boundary, nullability, mutability, lifetime/ownership and casts
- [ ] **F-ZL-017.02 — Freeze the exact contract for pointers/references**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit unsafe boundary, nullability, mutability, lifetime/ownership and casts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-017.03 — Implement/prove: explicit unsafe boundary**
  - Action: For pointers/references, implement or reuse and verify this exact obligation: explicit unsafe boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit unsafe boundary; retain observable state/resource expectations.
- [ ] **F-ZL-017.04 — Implement/prove: nullability**
  - Action: For pointers/references, implement or reuse and verify this exact obligation: nullability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for nullability; retain observable state/resource expectations.
- [ ] **F-ZL-017.05 — Implement/prove: mutability**
  - Action: For pointers/references, implement or reuse and verify this exact obligation: mutability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mutability; retain observable state/resource expectations.
- [ ] **F-ZL-017.06 — Implement/prove: lifetime/ownership and casts**
  - Action: For pointers/references, implement or reuse and verify this exact obligation: lifetime/ownership and casts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifetime/ownership and casts; retain observable state/resource expectations.
- [ ] **F-ZL-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to pointers/references: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire pointers/references into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for pointers/references as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-018"></a>
## F-ZL-018 — packed/aligned layouts

**Original requirement:** `sizeof`/`alignof`/`offsetof`, packed fields and C/firmware conformance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-018.01 — Reconcile existing packed/aligned layouts**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for packed/aligned layouts. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: `sizeof`/`alignof`/`offsetof`, packed fields and C/firmware conformance
- [ ] **F-ZL-018.02 — Freeze the exact contract for packed/aligned layouts**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: `sizeof`/`alignof`/`offsetof`, packed fields and C/firmware conformance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-018.03 — Implement/prove: `sizeof`/`alignof`/`offsetof`**
  - Action: For packed/aligned layouts, implement or reuse and verify this exact obligation: `sizeof`/`alignof`/`offsetof`. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for `sizeof`/`alignof`/`offsetof`; retain observable state/resource expectations.
- [ ] **F-ZL-018.04 — Implement/prove: packed fields and C/firmware conformance**
  - Action: For packed/aligned layouts, implement or reuse and verify this exact obligation: packed fields and C/firmware conformance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for packed fields and C/firmware conformance; retain observable state/resource expectations.
- [ ] **F-ZL-018.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to packed/aligned layouts: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-018.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-018.06 — Integrate into the real consumer and runtime route**
  - Action: Wire packed/aligned layouts into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-018.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-018.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for packed/aligned layouts as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-018.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-019"></a>
## F-ZL-019 — volatile access

**Original requirement:** exact-width device loads/stores separated from ordinary memory

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-019.01 — Reconcile existing volatile access**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for volatile access. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact-width device loads/stores separated from ordinary memory
- [ ] **F-ZL-019.02 — Freeze the exact contract for volatile access**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact-width device loads/stores separated from ordinary memory. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-019.03 — Implement/prove: exact-width device loads/stores separated from ordinary memory**
  - Action: For volatile access, implement or reuse and verify this exact obligation: exact-width device loads/stores separated from ordinary memory. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact-width device loads/stores separated from ordinary memory; retain observable state/resource expectations.
- [ ] **F-ZL-019.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to volatile access: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-019.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-019.05 — Integrate into the real consumer and runtime route**
  - Action: Wire volatile access into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-019.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-019.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for volatile access as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-019.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-020"></a>
## F-ZL-020 — atomics/memory ordering

**Original requirement:** typed operations/orderings and architecture litmus/conformance tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-020.01 — Reconcile existing atomics/memory ordering**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomics/memory ordering. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed operations/orderings and architecture litmus/conformance tests
- [ ] **F-ZL-020.02 — Freeze the exact contract for atomics/memory ordering**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed operations/orderings and architecture litmus/conformance tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-020.03 — Implement/prove: typed operations/orderings and architecture litmus/conformance tests**
  - Action: For atomics/memory ordering, implement or reuse and verify this exact obligation: typed operations/orderings and architecture litmus/conformance tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed operations/orderings and architecture litmus/conformance tests; retain observable state/resource expectations.
- [ ] **F-ZL-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomics/memory ordering: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire atomics/memory ordering into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomics/memory ordering as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-021"></a>
## F-ZL-021 — compiler/CPU barriers

**Original requirement:** named semantics with architecture implementation and mock transcripts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-021.01 — Reconcile existing compiler/CPU barriers**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compiler/CPU barriers. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: named semantics with architecture implementation and mock transcripts
- [ ] **F-ZL-021.02 — Freeze the exact contract for compiler/CPU barriers**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: named semantics with architecture implementation and mock transcripts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-021.03 — Implement/prove: named semantics with architecture implementation and mock transcripts**
  - Action: For compiler/CPU barriers, implement or reuse and verify this exact obligation: named semantics with architecture implementation and mock transcripts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for named semantics with architecture implementation and mock transcripts; retain observable state/resource expectations.
- [ ] **F-ZL-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compiler/CPU barriers: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire compiler/CPU barriers into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compiler/CPU barriers as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-022"></a>
## F-ZL-022 — inline assembly

**Original requirement:** explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-022.01 — Reconcile existing inline assembly**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for inline assembly. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation
- [ ] **F-ZL-022.02 — Freeze the exact contract for inline assembly**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-022.03 — Implement/prove: explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation**
  - Action: For inline assembly, implement or reuse and verify this exact obligation: explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit inputs/outputs/clobbers/options/unsafe scope and ABI preservation; retain observable state/resource expectations.
- [ ] **F-ZL-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to inline assembly: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire inline assembly into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for inline assembly as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-023"></a>
## F-ZL-023 — extern/FFI

**Original requirement:** generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-023.01 — Reconcile existing extern/FFI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for extern/FFI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes
- [ ] **F-ZL-023.02 — Freeze the exact contract for extern/FFI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-023.03 — Implement/prove: generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes**
  - Action: For extern/FFI, implement or reuse and verify this exact obligation: generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated signatures/layouts/calling conventions/ownership/errors/callback lifetimes; retain observable state/resource expectations.
- [ ] **F-ZL-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to extern/FFI: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire extern/FFI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for extern/FFI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-024"></a>
## F-ZL-024 — named sections/linker symbols

**Original requirement:** source attributes become checked object sections/flags/alignment/symbols

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-024.01 — Reconcile existing named sections/linker symbols**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for named sections/linker symbols. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source attributes become checked object sections/flags/alignment/symbols
- [ ] **F-ZL-024.02 — Freeze the exact contract for named sections/linker symbols**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source attributes become checked object sections/flags/alignment/symbols. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-024.03 — Implement/prove: source attributes become checked object sections/flags/alignment/symbols**
  - Action: For named sections/linker symbols, implement or reuse and verify this exact obligation: source attributes become checked object sections/flags/alignment/symbols. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source attributes become checked object sections/flags/alignment/symbols; retain observable state/resource expectations.
- [ ] **F-ZL-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to named sections/linker symbols: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire named sections/linker symbols into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for named sections/linker symbols as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-025"></a>
## F-ZL-025 — object emission

**Original requirement:** ELF relocatable sections/symbols/relocations and independent inspection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-025.01 — Reconcile existing object emission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for object emission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ELF relocatable sections/symbols/relocations and independent inspection
- [ ] **F-ZL-025.02 — Freeze the exact contract for object emission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ELF relocatable sections/symbols/relocations and independent inspection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-025.03 — Implement/prove: ELF relocatable sections/symbols/relocations and independent inspection**
  - Action: For object emission, implement or reuse and verify this exact obligation: ELF relocatable sections/symbols/relocations and independent inspection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ELF relocatable sections/symbols/relocations and independent inspection; retain observable state/resource expectations.
- [ ] **F-ZL-025.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to object emission: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-025.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-025.05 — Integrate into the real consumer and runtime route**
  - Action: Wire object emission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-025.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-025.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for object emission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-025.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-026"></a>
## F-ZL-026 — executable emission

**Original requirement:** freestanding/native startup, linker contract, no hidden host symbols and loader proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-026.01 — Reconcile existing executable emission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for executable emission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: freestanding/native startup, linker contract, no hidden host symbols and loader proof
- [ ] **F-ZL-026.02 — Freeze the exact contract for executable emission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: freestanding/native startup, linker contract, no hidden host symbols and loader proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-026.03 — Implement/prove: freestanding/native startup**
  - Action: For executable emission, implement or reuse and verify this exact obligation: freestanding/native startup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for freestanding/native startup; retain observable state/resource expectations.
- [ ] **F-ZL-026.04 — Implement/prove: linker contract**
  - Action: For executable emission, implement or reuse and verify this exact obligation: linker contract. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for linker contract; retain observable state/resource expectations.
- [ ] **F-ZL-026.05 — Implement/prove: no hidden host symbols and loader proof**
  - Action: For executable emission, implement or reuse and verify this exact obligation: no hidden host symbols and loader proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no hidden host symbols and loader proof; retain observable state/resource expectations.
- [ ] **F-ZL-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to executable emission: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire executable emission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for executable emission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-027"></a>
## F-ZL-027 — relocation emission

**Original requirement:** explicit supported set, checked addends/symbols/types and round-trip loader corpus

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-027.01 — Reconcile existing relocation emission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for relocation emission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit supported set, checked addends/symbols/types and round-trip loader corpus
- [ ] **F-ZL-027.02 — Freeze the exact contract for relocation emission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit supported set, checked addends/symbols/types and round-trip loader corpus. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-027.03 — Implement/prove: explicit supported set**
  - Action: For relocation emission, implement or reuse and verify this exact obligation: explicit supported set. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit supported set; retain observable state/resource expectations.
- [ ] **F-ZL-027.04 — Implement/prove: checked addends/symbols/types and round-trip loader corpus**
  - Action: For relocation emission, implement or reuse and verify this exact obligation: checked addends/symbols/types and round-trip loader corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked addends/symbols/types and round-trip loader corpus; retain observable state/resource expectations.
- [ ] **F-ZL-027.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to relocation emission: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-027.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-027.06 — Integrate into the real consumer and runtime route**
  - Action: Wire relocation emission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-027.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-027.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for relocation emission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-027.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-028"></a>
## F-ZL-028 — debug metadata

**Original requirement:** source files/lines/types/scopes/variables/build IDs and debugger compatibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-028.01 — Reconcile existing debug metadata**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for debug metadata. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source files/lines/types/scopes/variables/build IDs and debugger compatibility
- [ ] **F-ZL-028.02 — Freeze the exact contract for debug metadata**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source files/lines/types/scopes/variables/build IDs and debugger compatibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-028.03 — Implement/prove: source files/lines/types/scopes/variables/build IDs and debugger compatibility**
  - Action: For debug metadata, implement or reuse and verify this exact obligation: source files/lines/types/scopes/variables/build IDs and debugger compatibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source files/lines/types/scopes/variables/build IDs and debugger compatibility; retain observable state/resource expectations.
- [ ] **F-ZL-028.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to debug metadata: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-028.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-028.05 — Integrate into the real consumer and runtime route**
  - Action: Wire debug metadata into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-028.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-028.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for debug metadata as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-028.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-029"></a>
## F-ZL-029 — unwind metadata

**Original requirement:** optimized/unoptimized stack traversal and checked panic/debug consumers

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-029.01 — Reconcile existing unwind metadata**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for unwind metadata. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: optimized/unoptimized stack traversal and checked panic/debug consumers
- [ ] **F-ZL-029.02 — Freeze the exact contract for unwind metadata**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: optimized/unoptimized stack traversal and checked panic/debug consumers. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-029.03 — Implement/prove: optimized/unoptimized stack traversal and checked panic/debug consumers**
  - Action: For unwind metadata, implement or reuse and verify this exact obligation: optimized/unoptimized stack traversal and checked panic/debug consumers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for optimized/unoptimized stack traversal and checked panic/debug consumers; retain observable state/resource expectations.
- [ ] **F-ZL-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to unwind metadata: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire unwind metadata into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for unwind metadata as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-030"></a>
## F-ZL-030 — interpreter

**Original requirement:** reference semantics, sandbox/resource limits and differential tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-030.01 — Reconcile existing interpreter**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for interpreter. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reference semantics, sandbox/resource limits and differential tests
- [ ] **F-ZL-030.02 — Freeze the exact contract for interpreter**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reference semantics, sandbox/resource limits and differential tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-030.03 — Implement/prove: reference semantics**
  - Action: For interpreter, implement or reuse and verify this exact obligation: reference semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reference semantics; retain observable state/resource expectations.
- [ ] **F-ZL-030.04 — Implement/prove: sandbox/resource limits and differential tests**
  - Action: For interpreter, implement or reuse and verify this exact obligation: sandbox/resource limits and differential tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sandbox/resource limits and differential tests; retain observable state/resource expectations.
- [ ] **F-ZL-030.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to interpreter: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-030.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-030.06 — Integrate into the real consumer and runtime route**
  - Action: Wire interpreter into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-030.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-030.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for interpreter as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-030.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-031"></a>
## F-ZL-031 — native backend

**Original requirement:** semantics-equivalent lowering, optimization, ABI and code-generation receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-031.01 — Reconcile existing native backend**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for native backend. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantics-equivalent lowering, optimization, ABI and code-generation receipts
- [ ] **F-ZL-031.02 — Freeze the exact contract for native backend**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantics-equivalent lowering, optimization, ABI and code-generation receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-031.03 — Implement/prove: semantics-equivalent lowering**
  - Action: For native backend, implement or reuse and verify this exact obligation: semantics-equivalent lowering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantics-equivalent lowering; retain observable state/resource expectations.
- [ ] **F-ZL-031.04 — Implement/prove: optimization**
  - Action: For native backend, implement or reuse and verify this exact obligation: optimization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for optimization; retain observable state/resource expectations.
- [ ] **F-ZL-031.05 — Implement/prove: ABI and code-generation receipts**
  - Action: For native backend, implement or reuse and verify this exact obligation: ABI and code-generation receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ABI and code-generation receipts; retain observable state/resource expectations.
- [ ] **F-ZL-031.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to native backend: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-031.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-031.07 — Integrate into the real consumer and runtime route**
  - Action: Wire native backend into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-031.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-031.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for native backend as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-031.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-032"></a>
## F-ZL-032 — LLVM backend

**Original requirement:** differential semantics and explicit dependency/toolchain provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-032.01 — Reconcile existing LLVM backend**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for LLVM backend. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: differential semantics and explicit dependency/toolchain provenance
- [ ] **F-ZL-032.02 — Freeze the exact contract for LLVM backend**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: differential semantics and explicit dependency/toolchain provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-032.03 — Implement/prove: differential semantics and explicit dependency/toolchain provenance**
  - Action: For LLVM backend, implement or reuse and verify this exact obligation: differential semantics and explicit dependency/toolchain provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for differential semantics and explicit dependency/toolchain provenance; retain observable state/resource expectations.
- [ ] **F-ZL-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to LLVM backend: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire LLVM backend into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for LLVM backend as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-033"></a>
## F-ZL-033 — optimizer

**Original requirement:** behavior-preserving passes, debugability, size/speed budgets and mutation tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-033.01 — Reconcile existing optimizer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for optimizer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: behavior-preserving passes, debugability, size/speed budgets and mutation tests
- [ ] **F-ZL-033.02 — Freeze the exact contract for optimizer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: behavior-preserving passes, debugability, size/speed budgets and mutation tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-033.03 — Implement/prove: behavior-preserving passes**
  - Action: For optimizer, implement or reuse and verify this exact obligation: behavior-preserving passes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for behavior-preserving passes; retain observable state/resource expectations.
- [ ] **F-ZL-033.04 — Implement/prove: debugability**
  - Action: For optimizer, implement or reuse and verify this exact obligation: debugability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for debugability; retain observable state/resource expectations.
- [ ] **F-ZL-033.05 — Implement/prove: size/speed budgets and mutation tests**
  - Action: For optimizer, implement or reuse and verify this exact obligation: size/speed budgets and mutation tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for size/speed budgets and mutation tests; retain observable state/resource expectations.
- [ ] **F-ZL-033.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to optimizer: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-033.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-033.07 — Integrate into the real consumer and runtime route**
  - Action: Wire optimizer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-033.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-033.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for optimizer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-033.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-034"></a>
## F-ZL-034 — compile-time evaluation

**Original requirement:** deterministic declared inputs, termination/resources and reproducible output

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-034.01 — Reconcile existing compile-time evaluation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compile-time evaluation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic declared inputs, termination/resources and reproducible output
- [ ] **F-ZL-034.02 — Freeze the exact contract for compile-time evaluation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic declared inputs, termination/resources and reproducible output. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-034.03 — Implement/prove: deterministic declared inputs**
  - Action: For compile-time evaluation, implement or reuse and verify this exact obligation: deterministic declared inputs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic declared inputs; retain observable state/resource expectations.
- [ ] **F-ZL-034.04 — Implement/prove: termination/resources and reproducible output**
  - Action: For compile-time evaluation, implement or reuse and verify this exact obligation: termination/resources and reproducible output. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for termination/resources and reproducible output; retain observable state/resource expectations.
- [ ] **F-ZL-034.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compile-time evaluation: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-034.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-034.06 — Integrate into the real consumer and runtime route**
  - Action: Wire compile-time evaluation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-034.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-034.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compile-time evaluation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-034.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-035"></a>
## F-ZL-035 — code generation/IDL

**Original requirement:** schema validation and generated IPC/ABI/manifests with readable contracts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-035.01 — Reconcile existing code generation/IDL**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for code generation/IDL. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: schema validation and generated IPC/ABI/manifests with readable contracts
- [ ] **F-ZL-035.02 — Freeze the exact contract for code generation/IDL**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: schema validation and generated IPC/ABI/manifests with readable contracts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-035.03 — Implement/prove: schema validation and generated IPC/ABI/manifests with readable contracts**
  - Action: For code generation/IDL, implement or reuse and verify this exact obligation: schema validation and generated IPC/ABI/manifests with readable contracts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for schema validation and generated IPC/ABI/manifests with readable contracts; retain observable state/resource expectations.
- [ ] **F-ZL-035.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to code generation/IDL: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-035.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-035.05 — Integrate into the real consumer and runtime route**
  - Action: Wire code generation/IDL into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-035.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-035.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for code generation/IDL as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-035.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-036"></a>
## F-ZL-036 — memory allocation API

**Original requirement:** explicit allocator/failure/alignment/lifetime suitable for freestanding code

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-036.01 — Reconcile existing memory allocation API**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for memory allocation API. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit allocator/failure/alignment/lifetime suitable for freestanding code
- [ ] **F-ZL-036.02 — Freeze the exact contract for memory allocation API**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit allocator/failure/alignment/lifetime suitable for freestanding code. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-036.03 — Implement/prove: explicit allocator/failure/alignment/lifetime suitable for freestanding code**
  - Action: For memory allocation API, implement or reuse and verify this exact obligation: explicit allocator/failure/alignment/lifetime suitable for freestanding code. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit allocator/failure/alignment/lifetime suitable for freestanding code; retain observable state/resource expectations.
- [ ] **F-ZL-036.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to memory allocation API: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-036.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-036.05 — Integrate into the real consumer and runtime route**
  - Action: Wire memory allocation API into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-036.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-036.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for memory allocation API as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-036.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-037"></a>
## F-ZL-037 — ownership/lifetime helpers

**Original requirement:** deterministic cleanup without mandating one hidden runtime model

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-037.01 — Reconcile existing ownership/lifetime helpers**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ownership/lifetime helpers. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic cleanup without mandating one hidden runtime model
- [ ] **F-ZL-037.02 — Freeze the exact contract for ownership/lifetime helpers**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic cleanup without mandating one hidden runtime model. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-037.03 — Implement/prove: deterministic cleanup without mandating one hidden runtime model**
  - Action: For ownership/lifetime helpers, implement or reuse and verify this exact obligation: deterministic cleanup without mandating one hidden runtime model. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic cleanup without mandating one hidden runtime model; retain observable state/resource expectations.
- [ ] **F-ZL-037.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ownership/lifetime helpers: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-037.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-037.05 — Integrate into the real consumer and runtime route**
  - Action: Wire ownership/lifetime helpers into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-037.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-037.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ownership/lifetime helpers as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-037.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-038"></a>
## F-ZL-038 — concurrency primitives

**Original requirement:** threads/tasks/channels/locks/atomics with cancellation and bounded queues

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-038.01 — Reconcile existing concurrency primitives**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for concurrency primitives. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: threads/tasks/channels/locks/atomics with cancellation and bounded queues
- [ ] **F-ZL-038.02 — Freeze the exact contract for concurrency primitives**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: threads/tasks/channels/locks/atomics with cancellation and bounded queues. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-038.03 — Implement/prove: threads/tasks/channels/locks/atomics with cancellation and bounded queues**
  - Action: For concurrency primitives, implement or reuse and verify this exact obligation: threads/tasks/channels/locks/atomics with cancellation and bounded queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for threads/tasks/channels/locks/atomics with cancellation and bounded queues; retain observable state/resource expectations.
- [ ] **F-ZL-038.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to concurrency primitives: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-038.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-038.05 — Integrate into the real consumer and runtime route**
  - Action: Wire concurrency primitives into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-038.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-038.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for concurrency primitives as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-038.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-039"></a>
## F-ZL-039 — async/deadline model

**Original requirement:** explicit scheduler integration, cancellation, structured lifetime and backpressure

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-039.01 — Reconcile existing async/deadline model**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for async/deadline model. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit scheduler integration, cancellation, structured lifetime and backpressure
- [ ] **F-ZL-039.02 — Freeze the exact contract for async/deadline model**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit scheduler integration, cancellation, structured lifetime and backpressure. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-039.03 — Implement/prove: explicit scheduler integration**
  - Action: For async/deadline model, implement or reuse and verify this exact obligation: explicit scheduler integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit scheduler integration; retain observable state/resource expectations.
- [ ] **F-ZL-039.04 — Implement/prove: cancellation**
  - Action: For async/deadline model, implement or reuse and verify this exact obligation: cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation; retain observable state/resource expectations.
- [ ] **F-ZL-039.05 — Implement/prove: structured lifetime and backpressure**
  - Action: For async/deadline model, implement or reuse and verify this exact obligation: structured lifetime and backpressure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for structured lifetime and backpressure; retain observable state/resource expectations.
- [ ] **F-ZL-039.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to async/deadline model: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-039.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-039.07 — Integrate into the real consumer and runtime route**
  - Action: Wire async/deadline model into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-039.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-039.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for async/deadline model as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-039.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-040"></a>
## F-ZL-040 — standard library core

**Original requirement:** bytes/text/collections/math/time/errors/files abstractions split by platform authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-040.01 — Reconcile existing standard library core**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for standard library core. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bytes/text/collections/math/time/errors/files abstractions split by platform authority
- [ ] **F-ZL-040.02 — Freeze the exact contract for standard library core**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bytes/text/collections/math/time/errors/files abstractions split by platform authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-040.03 — Implement/prove: bytes/text/collections/math/time/errors/files abstractions split by platform authority**
  - Action: For standard library core, implement or reuse and verify this exact obligation: bytes/text/collections/math/time/errors/files abstractions split by platform authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bytes/text/collections/math/time/errors/files abstractions split by platform authority; retain observable state/resource expectations.
- [ ] **F-ZL-040.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to standard library core: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-040.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-040.05 — Integrate into the real consumer and runtime route**
  - Action: Wire standard library core into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-040.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-040.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for standard library core as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-040.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-041"></a>
## F-ZL-041 — freestanding standard library

**Original requirement:** no host assumptions, allocation choices, panics/errors and target tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-041.01 — Reconcile existing freestanding standard library**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for freestanding standard library. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: no host assumptions, allocation choices, panics/errors and target tests
- [ ] **F-ZL-041.02 — Freeze the exact contract for freestanding standard library**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: no host assumptions, allocation choices, panics/errors and target tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-041.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-041.03 — Implement/prove: no host assumptions**
  - Action: For freestanding standard library, implement or reuse and verify this exact obligation: no host assumptions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-041.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no host assumptions; retain observable state/resource expectations.
- [ ] **F-ZL-041.04 — Implement/prove: allocation choices**
  - Action: For freestanding standard library, implement or reuse and verify this exact obligation: allocation choices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-041.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allocation choices; retain observable state/resource expectations.
- [ ] **F-ZL-041.05 — Implement/prove: panics/errors and target tests**
  - Action: For freestanding standard library, implement or reuse and verify this exact obligation: panics/errors and target tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-041.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for panics/errors and target tests; retain observable state/resource expectations.
- [ ] **F-ZL-041.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to freestanding standard library: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-041.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-041.07 — Integrate into the real consumer and runtime route**
  - Action: Wire freestanding standard library into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-041.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-041.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for freestanding standard library as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-041.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-042"></a>
## F-ZL-042 — Linux platform library

**Original requirement:** files/process/network/time mappings with exact error and ABI semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-042.01 — Reconcile existing Linux platform library**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Linux platform library. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: files/process/network/time mappings with exact error and ABI semantics
- [ ] **F-ZL-042.02 — Freeze the exact contract for Linux platform library**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: files/process/network/time mappings with exact error and ABI semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-042.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-042.03 — Implement/prove: files/process/network/time mappings with exact error and ABI semantics**
  - Action: For Linux platform library, implement or reuse and verify this exact obligation: files/process/network/time mappings with exact error and ABI semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-042.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for files/process/network/time mappings with exact error and ABI semantics; retain observable state/resource expectations.
- [ ] **F-ZL-042.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Linux platform library: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-042.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-042.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Linux platform library into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-042.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-042.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Linux platform library as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-042.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-043"></a>
## F-ZL-043 — zlOS platform library

**Original requirement:** handles/IPC/process/files/window/input/audio/network/services over native ABI

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-043.01 — Reconcile existing zlOS platform library**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for zlOS platform library. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16, H-03, H-04, H-06, H-07, H-08, H-09, H-11.
  - Acceptance: A source-and-evidence comparison against the complete requirement: handles/IPC/process/files/window/input/audio/network/services over native ABI
- [ ] **F-ZL-043.02 — Freeze the exact contract for zlOS platform library**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: handles/IPC/process/files/window/input/audio/network/services over native ABI. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-043.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-043.03 — Implement/prove: handles/IPC/process/files/window/input/audio/network/services over native ABI**
  - Action: For zlOS platform library, implement or reuse and verify this exact obligation: handles/IPC/process/files/window/input/audio/network/services over native ABI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-043.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handles/IPC/process/files/window/input/audio/network/services over native ABI; retain observable state/resource expectations.
- [ ] **F-ZL-043.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to zlOS platform library: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-043.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-043.05 — Integrate into the real consumer and runtime route**
  - Action: Wire zlOS platform library into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-043.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-043.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for zlOS platform library as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-043.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-044"></a>
## F-ZL-044 — package manifest/resolver

**Original requirement:** immutable versions/dependencies/features/targets/licenses and reproducible resolution

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-044.01 — Reconcile existing package manifest/resolver**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for package manifest/resolver. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: immutable versions/dependencies/features/targets/licenses and reproducible resolution
- [ ] **F-ZL-044.02 — Freeze the exact contract for package manifest/resolver**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: immutable versions/dependencies/features/targets/licenses and reproducible resolution. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-044.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-044.03 — Implement/prove: immutable versions/dependencies/features/targets/licenses and reproducible resolution**
  - Action: For package manifest/resolver, implement or reuse and verify this exact obligation: immutable versions/dependencies/features/targets/licenses and reproducible resolution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-044.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for immutable versions/dependencies/features/targets/licenses and reproducible resolution; retain observable state/resource expectations.
- [ ] **F-ZL-044.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to package manifest/resolver: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-044.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-044.05 — Integrate into the real consumer and runtime route**
  - Action: Wire package manifest/resolver into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-044.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-044.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for package manifest/resolver as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-044.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-045"></a>
## F-ZL-045 — formatter/linter

**Original requirement:** stable style, machine fixes, diagnostics, configuration and strict exit status

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-045.01 — Reconcile existing formatter/linter**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for formatter/linter. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stable style, machine fixes, diagnostics, configuration and strict exit status
- [ ] **F-ZL-045.02 — Freeze the exact contract for formatter/linter**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stable style, machine fixes, diagnostics, configuration and strict exit status. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-045.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-045.03 — Implement/prove: stable style**
  - Action: For formatter/linter, implement or reuse and verify this exact obligation: stable style. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-045.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stable style; retain observable state/resource expectations.
- [ ] **F-ZL-045.04 — Implement/prove: machine fixes**
  - Action: For formatter/linter, implement or reuse and verify this exact obligation: machine fixes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-045.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for machine fixes; retain observable state/resource expectations.
- [ ] **F-ZL-045.05 — Implement/prove: diagnostics**
  - Action: For formatter/linter, implement or reuse and verify this exact obligation: diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-045.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics; retain observable state/resource expectations.
- [ ] **F-ZL-045.06 — Implement/prove: configuration and strict exit status**
  - Action: For formatter/linter, implement or reuse and verify this exact obligation: configuration and strict exit status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-045.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configuration and strict exit status; retain observable state/resource expectations.
- [ ] **F-ZL-045.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to formatter/linter: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-045.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-045.08 — Integrate into the real consumer and runtime route**
  - Action: Wire formatter/linter into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-045.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-045.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for formatter/linter as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-045.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-046"></a>
## F-ZL-046 — documentation generator

**Original requirement:** symbols/types/examples/links/version/search and source provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-046.01 — Reconcile existing documentation generator**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for documentation generator. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: symbols/types/examples/links/version/search and source provenance
- [ ] **F-ZL-046.02 — Freeze the exact contract for documentation generator**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: symbols/types/examples/links/version/search and source provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-046.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-046.03 — Implement/prove: symbols/types/examples/links/version/search and source provenance**
  - Action: For documentation generator, implement or reuse and verify this exact obligation: symbols/types/examples/links/version/search and source provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-046.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for symbols/types/examples/links/version/search and source provenance; retain observable state/resource expectations.
- [ ] **F-ZL-046.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to documentation generator: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-046.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-046.05 — Integrate into the real consumer and runtime route**
  - Action: Wire documentation generator into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-046.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-046.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for documentation generator as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-046.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-047"></a>
## F-ZL-047 — language server

**Original requirement:** parse/index/completion/diagnostics/refactor with bounded incremental work

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-047.01 — Reconcile existing language server**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for language server. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: parse/index/completion/diagnostics/refactor with bounded incremental work
- [ ] **F-ZL-047.02 — Freeze the exact contract for language server**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: parse/index/completion/diagnostics/refactor with bounded incremental work. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-047.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-047.03 — Implement/prove: parse/index/completion/diagnostics/refactor with bounded incremental work**
  - Action: For language server, implement or reuse and verify this exact obligation: parse/index/completion/diagnostics/refactor with bounded incremental work. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-047.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for parse/index/completion/diagnostics/refactor with bounded incremental work; retain observable state/resource expectations.
- [ ] **F-ZL-047.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to language server: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-047.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-047.05 — Integrate into the real consumer and runtime route**
  - Action: Wire language server into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-047.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-047.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for language server as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-047.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-048"></a>
## F-ZL-048 — test framework

**Original requirement:** host/target/property/fuzz/benchmark lanes, discovery parity and strict results

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-048.01 — Reconcile existing test framework**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for test framework. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: host/target/property/fuzz/benchmark lanes, discovery parity and strict results
- [ ] **F-ZL-048.02 — Freeze the exact contract for test framework**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: host/target/property/fuzz/benchmark lanes, discovery parity and strict results. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-048.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-048.03 — Implement/prove: host/target/property/fuzz/benchmark lanes**
  - Action: For test framework, implement or reuse and verify this exact obligation: host/target/property/fuzz/benchmark lanes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-048.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for host/target/property/fuzz/benchmark lanes; retain observable state/resource expectations.
- [ ] **F-ZL-048.04 — Implement/prove: discovery parity and strict results**
  - Action: For test framework, implement or reuse and verify this exact obligation: discovery parity and strict results. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-048.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery parity and strict results; retain observable state/resource expectations.
- [ ] **F-ZL-048.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to test framework: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-048.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-048.06 — Integrate into the real consumer and runtime route**
  - Action: Wire test framework into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-048.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-048.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for test framework as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-048.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-049"></a>
## F-ZL-049 — compatibility/versioning

**Original requirement:** language editions/features/deprecations/migrations and artifact metadata

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-049.01 — Reconcile existing compatibility/versioning**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compatibility/versioning. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: language editions/features/deprecations/migrations and artifact metadata
- [ ] **F-ZL-049.02 — Freeze the exact contract for compatibility/versioning**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: language editions/features/deprecations/migrations and artifact metadata. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-049.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-049.03 — Implement/prove: language editions/features/deprecations/migrations and artifact metadata**
  - Action: For compatibility/versioning, implement or reuse and verify this exact obligation: language editions/features/deprecations/migrations and artifact metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-049.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language editions/features/deprecations/migrations and artifact metadata; retain observable state/resource expectations.
- [ ] **F-ZL-049.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compatibility/versioning: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-049.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-049.05 — Integrate into the real consumer and runtime route**
  - Action: Wire compatibility/versioning into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-049.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-049.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compatibility/versioning as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-049.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-050"></a>
## F-ZL-050 — bootstrap seed

**Original requirement:** exact trusted seed retained and identified through stage chain

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-050.01 — Reconcile existing bootstrap seed**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for bootstrap seed. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact trusted seed retained and identified through stage chain
- [ ] **F-ZL-050.02 — Freeze the exact contract for bootstrap seed**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact trusted seed retained and identified through stage chain. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-050.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-050.03 — Implement/prove: exact trusted seed retained and identified through stage chain**
  - Action: For bootstrap seed, implement or reuse and verify this exact obligation: exact trusted seed retained and identified through stage chain. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-050.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact trusted seed retained and identified through stage chain; retain observable state/resource expectations.
- [ ] **F-ZL-050.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to bootstrap seed: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-050.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-050.05 — Integrate into the real consumer and runtime route**
  - Action: Wire bootstrap seed into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-050.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-050.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for bootstrap seed as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-050.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-051"></a>
## F-ZL-051 — compiler fixpoint

**Original requirement:** source-stage rebuild and semantic/artifact provenance, not stored hash alone

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-051.01 — Reconcile existing compiler fixpoint**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compiler fixpoint. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16, H-15.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source-stage rebuild and semantic/artifact provenance, not stored hash alone
- [ ] **F-ZL-051.02 — Freeze the exact contract for compiler fixpoint**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source-stage rebuild and semantic/artifact provenance, not stored hash alone. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-051.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-051.03 — Implement/prove: source-stage rebuild and semantic/artifact provenance**
  - Action: For compiler fixpoint, implement or reuse and verify this exact obligation: source-stage rebuild and semantic/artifact provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-051.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source-stage rebuild and semantic/artifact provenance; retain observable state/resource expectations.
- [ ] **F-ZL-051.04 — Implement/prove: not stored hash alone**
  - Action: For compiler fixpoint, implement or reuse and verify this exact obligation: not stored hash alone. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-051.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not stored hash alone; retain observable state/resource expectations.
- [ ] **F-ZL-051.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compiler fixpoint: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-051.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-051.06 — Integrate into the real consumer and runtime route**
  - Action: Wire compiler fixpoint into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-051.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-051.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compiler fixpoint as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-051.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-052"></a>
## F-ZL-052 — self-hosted system build

**Original requirement:** matching compiler, kernel, services, apps and packages rebuilt inside zlOS

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-052.01 — Reconcile existing self-hosted system build**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for self-hosted system build. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16, H-20.
  - Acceptance: A source-and-evidence comparison against the complete requirement: matching compiler, kernel, services, apps and packages rebuilt inside zlOS
- [ ] **F-ZL-052.02 — Freeze the exact contract for self-hosted system build**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: matching compiler, kernel, services, apps and packages rebuilt inside zlOS. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-052.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-052.03 — Implement/prove: matching compiler**
  - Action: For self-hosted system build, implement or reuse and verify this exact obligation: matching compiler. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-052.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for matching compiler; retain observable state/resource expectations.
- [ ] **F-ZL-052.04 — Implement/prove: kernel**
  - Action: For self-hosted system build, implement or reuse and verify this exact obligation: kernel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-052.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for kernel; retain observable state/resource expectations.
- [ ] **F-ZL-052.05 — Implement/prove: services**
  - Action: For self-hosted system build, implement or reuse and verify this exact obligation: services. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-052.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for services; retain observable state/resource expectations.
- [ ] **F-ZL-052.06 — Implement/prove: apps and packages rebuilt inside zlOS**
  - Action: For self-hosted system build, implement or reuse and verify this exact obligation: apps and packages rebuilt inside zlOS. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-052.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for apps and packages rebuilt inside zlOS; retain observable state/resource expectations.
- [ ] **F-ZL-052.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to self-hosted system build: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-052.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-052.08 — Integrate into the real consumer and runtime route**
  - Action: Wire self-hosted system build into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-052.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-052.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for self-hosted system build as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-052.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-053"></a>
## F-ZL-053 — multiarchitecture targets

**Original requirement:** x86-64/i686/AArch64/RISC-V target manifests and execution evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-053.01 — Reconcile existing multiarchitecture targets**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for multiarchitecture targets. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: x86-64/i686/AArch64/RISC-V target manifests and execution evidence
- [ ] **F-ZL-053.02 — Freeze the exact contract for multiarchitecture targets**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: x86-64/i686/AArch64/RISC-V target manifests and execution evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-053.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-053.03 — Implement/prove: x86-64/i686/AArch64/RISC-V target manifests and execution evidence**
  - Action: For multiarchitecture targets, implement or reuse and verify this exact obligation: x86-64/i686/AArch64/RISC-V target manifests and execution evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-053.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for x86-64/i686/AArch64/RISC-V target manifests and execution evidence; retain observable state/resource expectations.
- [ ] **F-ZL-053.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to multiarchitecture targets: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-053.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-053.05 — Integrate into the real consumer and runtime route**
  - Action: Wire multiarchitecture targets into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-053.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-053.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for multiarchitecture targets as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-053.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-054"></a>
## F-ZL-054 — language security audit

**Original requirement:** unsafe surface, parser/compiler attack inputs, supply chain and generated-code trust

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-054.01 — Reconcile existing language security audit**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for language security audit. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: unsafe surface, parser/compiler attack inputs, supply chain and generated-code trust
- [ ] **F-ZL-054.02 — Freeze the exact contract for language security audit**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: unsafe surface, parser/compiler attack inputs, supply chain and generated-code trust. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-054.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-054.03 — Implement/prove: unsafe surface**
  - Action: For language security audit, implement or reuse and verify this exact obligation: unsafe surface. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-054.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unsafe surface; retain observable state/resource expectations.
- [ ] **F-ZL-054.04 — Implement/prove: parser/compiler attack inputs**
  - Action: For language security audit, implement or reuse and verify this exact obligation: parser/compiler attack inputs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-054.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for parser/compiler attack inputs; retain observable state/resource expectations.
- [ ] **F-ZL-054.05 — Implement/prove: supply chain and generated-code trust**
  - Action: For language security audit, implement or reuse and verify this exact obligation: supply chain and generated-code trust. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-054.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for supply chain and generated-code trust; retain observable state/resource expectations.
- [ ] **F-ZL-054.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to language security audit: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-054.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-054.07 — Integrate into the real consumer and runtime route**
  - Action: Wire language security audit into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-054.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-054.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for language security audit as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-054.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-zl-055"></a>
## F-ZL-055 — language performance lab

**Original requirement:** compile time, memory, binary size and runtime benchmarks with exact baselines

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-ZL-055.01 — Reconcile existing language performance lab**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for language performance lab. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-and-evidence comparison against the complete requirement: compile time, memory, binary size and runtime benchmarks with exact baselines
- [ ] **F-ZL-055.02 — Freeze the exact contract for language performance lab**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: compile time, memory, binary size and runtime benchmarks with exact baselines. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-ZL-055.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-ZL-055.03 — Implement/prove: compile time**
  - Action: For language performance lab, implement or reuse and verify this exact obligation: compile time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-055.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compile time; retain observable state/resource expectations.
- [ ] **F-ZL-055.04 — Implement/prove: memory**
  - Action: For language performance lab, implement or reuse and verify this exact obligation: memory. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-055.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for memory; retain observable state/resource expectations.
- [ ] **F-ZL-055.05 — Implement/prove: binary size and runtime benchmarks with exact baselines**
  - Action: For language performance lab, implement or reuse and verify this exact obligation: binary size and runtime benchmarks with exact baselines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-ZL-055.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for binary size and runtime benchmarks with exact baselines; retain observable state/resource expectations.
- [ ] **F-ZL-055.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to language performance lab: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-ZL-055.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-ZL-055.07 — Integrate into the real consumer and runtime route**
  - Action: Wire language performance lab into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-ZL-055.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-ZL-055.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for language performance lab as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-ZL-055.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p9-1"></a>
## C-P9.1 — imports, scope/errors and data contracts

**Original requirement:** imports, scope/errors and data contracts

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 478.

### Preserved original contract

- **Dependencies/current/provenance:** active zl sequence; zl own roadmap, Lemon/Mollen schema needs; reject OS-only syntax fork.
- **I/O and state:** modules, local/global declarations, result/error values, records/maps/tuples/optionals/enums in; stable AST/semantics across engines out.
- **Invariants/failure:** import identity/cycle policy; local-by-default semantics; annotation neutrality later; errors never silently become sentinel success.
- **Deterministic proof:** interpreter/compiler/self-host differential, module cycles/duplicates, scoping corpus, error propagation, serialization of protocol records.
- **Target proof:** host is primary; QEMU runs generated assertion module when backend supports it.
- **Receipt/removal:** fixpoint and engine-output receipts; old copy-pasted modules remain until all imports resolve; no syntax fork.

### Execution steps

- [ ] **C-P9.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P9.1.02 — Resolve this contract's exact dependencies**
  - Action: active zl sequence; zl own roadmap, Lemon/Mollen schema needs; reject OS-only syntax fork. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P9.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P9.1.03 — I/O and state — imports, scope/errors and data contracts**
  - Action: modules, local/global declarations, result/error values, records/maps/tuples/optionals/enums in; stable AST/semantics across engines out.
  - Requires: C-P9.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P9.1.
- [ ] **C-P9.1.04 — Invariants/failure — imports, scope/errors and data contracts**
  - Action: import identity/cycle policy; local-by-default semantics; annotation neutrality later; errors never silently become sentinel success.
  - Requires: C-P9.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P9.1.
- [ ] **C-P9.1.05 — Deterministic proof — imports, scope/errors and data contracts**
  - Action: interpreter/compiler/self-host differential, module cycles/duplicates, scoping corpus, error propagation, serialization of protocol records.
  - Requires: C-P9.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P9.1.
- [ ] **C-P9.1.06 — Target proof — imports, scope/errors and data contracts**
  - Action: host is primary; QEMU runs generated assertion module when backend supports it.
  - Requires: C-P9.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P9.1.
- [ ] **C-P9.1.07 — Receipt/removal — imports, scope/errors and data contracts**
  - Action: fixpoint and engine-output receipts; old copy-pasted modules remain until all imports resolve; no syntax fork.
  - Requires: C-P9.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P9.1.
- [ ] **C-P9.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P9.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P9.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p9-2"></a>
## C-P9.2 — exact integers, bytes and arithmetic modes

**Original requirement:** exact integers, bytes and arithmetic modes

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 487.

### Preserved original contract

- **Dependencies/current/provenance:** P9.1; freestanding headers/Lunaris layouts; reject doubles for addresses and silent overflow.
- **I/O and state:** typed fixed-width values/slices and checked/wrapping operations in; exact bits or typed overflow out.
- **Invariants/failure:** widths/sign/shift semantics identical; 64-bit addresses preserve low bits; bounds checked; byte/string distinction explicit.
- **Deterministic proof:** exhaustive small widths, boundary/property corpus for all widths, interpreter/LLVM/native/C ABI comparison, endian/unaligned helpers.
- **Target proof:** QEMU passes addresses above 2^53 through zl unchanged and validates a descriptor/file/packet fixture.
- **Receipt/removal:** semantic version/corpus hashes and IR/assembly samples; boxed number mode stays compatible; remove double-only system bridges after caller migration.

### Execution steps

- [ ] **C-P9.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P9.2.02 — Resolve this contract's exact dependencies**
  - Action: P9.1; freestanding headers/Lunaris layouts; reject doubles for addresses and silent overflow. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P9.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P9.2.03 — I/O and state — exact integers, bytes and arithmetic modes**
  - Action: typed fixed-width values/slices and checked/wrapping operations in; exact bits or typed overflow out.
  - Requires: C-P9.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P9.2.
- [ ] **C-P9.2.04 — Invariants/failure — exact integers, bytes and arithmetic modes**
  - Action: widths/sign/shift semantics identical; 64-bit addresses preserve low bits; bounds checked; byte/string distinction explicit.
  - Requires: C-P9.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P9.2.
- [ ] **C-P9.2.05 — Deterministic proof — exact integers, bytes and arithmetic modes**
  - Action: exhaustive small widths, boundary/property corpus for all widths, interpreter/LLVM/native/C ABI comparison, endian/unaligned helpers.
  - Requires: C-P9.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P9.2.
- [ ] **C-P9.2.06 — Target proof — exact integers, bytes and arithmetic modes**
  - Action: QEMU passes addresses above 2^53 through zl unchanged and validates a descriptor/file/packet fixture.
  - Requires: C-P9.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P9.2.
- [ ] **C-P9.2.07 — Receipt/removal — exact integers, bytes and arithmetic modes**
  - Action: semantic version/corpus hashes and IR/assembly samples; boxed number mode stays compatible; remove double-only system bridges after caller migration.
  - Requires: C-P9.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P9.2.
- [ ] **C-P9.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P9.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P9.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p9-3"></a>
## C-P9.3 — native bitwise, volatile MMIO, ports and barriers

**Original requirement:** native bitwise, volatile MMIO, ports and barriers

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 496.

### Preserved original contract

- **Dependencies/current/provenance:** P9.2 and typed backend; existing `compilel` measurements, Sapphire protocol discipline; reject boxed name dispatch in drivers.
- **I/O and state:** typed bitwise/volatile/port/barrier operations in; direct IR/machine operations out.
- **Invariants/failure:** no conversion to double/boxed runtime; volatile order preserved; unsafe/danger marker required; target unsupported returns compile error.
- **Deterministic proof:** IR/assembly structural checks, semantic differential, host MMIO mock ordering, barrier litmus models, benchmark rerun.
- **Target proof:** QEMU small noncritical device operation; physical read-only probe before any write.
- **Receipt/removal:** source/IR/object/benchmark and hardware trace; C bridge rollback; remove boxed builtin route for typed calls only after parity.

### Execution steps

- [ ] **C-P9.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P9.3.02 — Resolve this contract's exact dependencies**
  - Action: P9.2 and typed backend; existing `compilel` measurements, Sapphire protocol discipline; reject boxed name dispatch in drivers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P9.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P9.3.03 — I/O and state — native bitwise, volatile MMIO, ports and barriers**
  - Action: typed bitwise/volatile/port/barrier operations in; direct IR/machine operations out.
  - Requires: C-P9.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P9.3.
- [ ] **C-P9.3.04 — Invariants/failure — native bitwise, volatile MMIO, ports and barriers**
  - Action: no conversion to double/boxed runtime; volatile order preserved; unsafe/danger marker required; target unsupported returns compile error.
  - Requires: C-P9.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P9.3.
- [ ] **C-P9.3.05 — Deterministic proof — native bitwise, volatile MMIO, ports and barriers**
  - Action: IR/assembly structural checks, semantic differential, host MMIO mock ordering, barrier litmus models, benchmark rerun.
  - Requires: C-P9.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P9.3.
- [ ] **C-P9.3.06 — Target proof — native bitwise, volatile MMIO, ports and barriers**
  - Action: QEMU small noncritical device operation; physical read-only probe before any write.
  - Requires: C-P9.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P9.3.
- [ ] **C-P9.3.07 — Receipt/removal — native bitwise, volatile MMIO, ports and barriers**
  - Action: source/IR/object/benchmark and hardware trace; C bridge rollback; remove boxed builtin route for typed calls only after parity.
  - Requires: C-P9.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P9.3.
- [ ] **C-P9.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P9.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P9.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p9-4"></a>
## C-P9.4 — layouts, calling conventions and function values

**Original requirement:** layouts, calling conventions and function values

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 505.

### Preserved original contract

- **Dependencies/current/provenance:** P9.2/types and scoping/closures; Lunaris concept, freestanding headers; reject implicit platform layout.
- **I/O and state:** packed/aligned records, extern/asm symbols, typed function values and ABI declarations in; layout/call manifest out.
- **Invariants/failure:** `size/align/offset` compile-time exact; calling convention explicit; invalid packed/reference combinations reject; callback lifetime owned.
- **Deterministic proof:** compare generated layouts/calls against C/compiler ABI fixtures on supported targets, register preservation, callback/interrupt-table corpus.
- **Target proof:** QEMU calls assembly stub and back; no general user callback executes in IRQ context.
- **Receipt/removal:** ABI manifest/object disassembly; C wrapper remains fallback; remove handwritten layout constants after generated equality.

### Execution steps

- [ ] **C-P9.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P9.4.02 — Resolve this contract's exact dependencies**
  - Action: P9.2/types and scoping/closures; Lunaris concept, freestanding headers; reject implicit platform layout. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P9.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P9.4.03 — I/O and state — layouts, calling conventions and function values**
  - Action: packed/aligned records, extern/asm symbols, typed function values and ABI declarations in; layout/call manifest out.
  - Requires: C-P9.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P9.4.
- [ ] **C-P9.4.04 — Invariants/failure — layouts, calling conventions and function values**
  - Action: `size/align/offset` compile-time exact; calling convention explicit; invalid packed/reference combinations reject; callback lifetime owned.
  - Requires: C-P9.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P9.4.
- [ ] **C-P9.4.05 — Deterministic proof — layouts, calling conventions and function values**
  - Action: compare generated layouts/calls against C/compiler ABI fixtures on supported targets, register preservation, callback/interrupt-table corpus.
  - Requires: C-P9.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P9.4.
- [ ] **C-P9.4.06 — Target proof — layouts, calling conventions and function values**
  - Action: QEMU calls assembly stub and back; no general user callback executes in IRQ context.
  - Requires: C-P9.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P9.4.
- [ ] **C-P9.4.07 — Receipt/removal — layouts, calling conventions and function values**
  - Action: ABI manifest/object disassembly; C wrapper remains fallback; remove handwritten layout constants after generated equality.
  - Requires: C-P9.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P9.4.
- [ ] **C-P9.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P9.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P9.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p9-5"></a>
## C-P9.5 — ELF64 executable/relocatable and named sections

**Original requirement:** ELF64 executable/relocatable and named sections

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 514.

### Preserved original contract

- **Dependencies/current/provenance:** P9.2-P9.4, current own/LLVM backends and P3 loader; Lunaris mechanism, Hyper assertion; reject file-validity by `file` alone.
- **I/O and state:** zl source, target/ABI/link manifest in; ELF executable/PIE or relocatable object with sections/symbols/relocations out.
- **Invariants/failure:** deterministic sections/symbols; relocations bounded/supported; no CRT when freestanding; undefined extern clear; loader and linker manifests agree.
- **Deterministic proof:** readelf/objdump structural assertions, relocate/link/load fixtures, malformed/unsupported relocation, deterministic rebuild, debug-symbol sanity.
- **Target proof:** QEMU loads minimal app then signed noncritical provider; hardware only after QEMU/host oracle.
- **Receipt/removal:** source/compiler/object/link/image digests; C-generated kernel remains primary; do not remove until Phase 12 rebuild proof.

### Execution steps

- [ ] **C-P9.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P9.5.02 — Resolve this contract's exact dependencies**
  - Action: P9.2-P9.4, current own/LLVM backends and P3 loader; Lunaris mechanism, Hyper assertion; reject file-validity by `file` alone. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P9.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P9.5.03 — I/O and state — ELF64 executable/relocatable and named sections**
  - Action: zl source, target/ABI/link manifest in; ELF executable/PIE or relocatable object with sections/symbols/relocations out.
  - Requires: C-P9.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P9.5.
- [ ] **C-P9.5.04 — Invariants/failure — ELF64 executable/relocatable and named sections**
  - Action: deterministic sections/symbols; relocations bounded/supported; no CRT when freestanding; undefined extern clear; loader and linker manifests agree.
  - Requires: C-P9.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P9.5.
- [ ] **C-P9.5.05 — Deterministic proof — ELF64 executable/relocatable and named sections**
  - Action: readelf/objdump structural assertions, relocate/link/load fixtures, malformed/unsupported relocation, deterministic rebuild, debug-symbol sanity.
  - Requires: C-P9.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P9.5.
- [ ] **C-P9.5.06 — Target proof — ELF64 executable/relocatable and named sections**
  - Action: QEMU loads minimal app then signed noncritical provider; hardware only after QEMU/host oracle.
  - Requires: C-P9.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P9.5.
- [ ] **C-P9.5.07 — Receipt/removal — ELF64 executable/relocatable and named sections**
  - Action: source/compiler/object/link/image digests; C-generated kernel remains primary; do not remove until Phase 12 rebuild proof.
  - Requires: C-P9.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P9.5.
- [ ] **C-P9.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P9.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P9.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p9-6"></a>
## C-P9.6 — first zl provider and fixpoint preservation

**Original requirement:** first zl provider and fixpoint preservation

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 523.

### Preserved original contract

- **Dependencies/current/provenance:** P8.1, P9.5 and signatures/caps; Brook load-boundary tests; reject translating Intel/xHCI first.
- **I/O and state:** small non-load-bearing provider plus manifest in; loaded provider handle and behavior out; driver lifecycle.
- **Invariants/failure:** whole image validated before map; only declared imports/resources; unload/revoke cleans all; compiler fixpoint unchanged.
- **Deterministic proof:** C-oracle differential, bad manifest/symbol/relocation, load/unload loops, resource denial, interpreter/LLVM/native compiler gates.
- **Target proof:** QEMU provider effect and recovery; hardware only if harmless/read-only.
- **Receipt/removal:** source/object/load/behavior/unload/fixpoint receipts; C provider remains selectable; remove only after multiple releases and exact parity.

### Execution steps

- [ ] **C-P9.6.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P9.6.02 — Resolve this contract's exact dependencies**
  - Action: P8.1, P9.5 and signatures/caps; Brook load-boundary tests; reject translating Intel/xHCI first. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P9.6.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P9.6.03 — I/O and state — first zl provider and fixpoint preservation**
  - Action: small non-load-bearing provider plus manifest in; loaded provider handle and behavior out; driver lifecycle.
  - Requires: C-P9.6.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P9.6.
- [ ] **C-P9.6.04 — Invariants/failure — first zl provider and fixpoint preservation**
  - Action: whole image validated before map; only declared imports/resources; unload/revoke cleans all; compiler fixpoint unchanged.
  - Requires: C-P9.6.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P9.6.
- [ ] **C-P9.6.05 — Deterministic proof — first zl provider and fixpoint preservation**
  - Action: C-oracle differential, bad manifest/symbol/relocation, load/unload loops, resource denial, interpreter/LLVM/native compiler gates.
  - Requires: C-P9.6.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P9.6.
- [ ] **C-P9.6.06 — Target proof — first zl provider and fixpoint preservation**
  - Action: QEMU provider effect and recovery; hardware only if harmless/read-only.
  - Requires: C-P9.6.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P9.6.
- [ ] **C-P9.6.07 — Receipt/removal — first zl provider and fixpoint preservation**
  - Action: source/object/load/behavior/unload/fixpoint receipts; C provider remains selectable; remove only after multiple releases and exact parity.
  - Requires: C-P9.6.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P9.6.
- [ ] **C-P9.6.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P9.6. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P9.6.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-svc-124"></a>
## T-SVC-124 — Build Service

**Original requirement:** hermetic graphs, cache, sandbox and receipts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 151.

### Execution steps

- [ ] **T-SVC-124.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Build Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-124.02 — Specify the complete target boundary**
  - Action: Build Service must supply: hermetic graphs, cache, sandbox and receipts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-124.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-124.03 — Implement the exact target behavior**
  - Action: Implement or reuse Build Service through the shared platform contract, delivering every part of: hermetic graphs, cache, sandbox and receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-124.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-124.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-124.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Build Service.
- [ ] **T-SVC-124.05 — Qualify and retain this target's own result**
  - Action: Bind Build Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-124.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-081"></a>
## T-APP-081 — Compiler/Build Monitor

**Original requirement:** graphs, diagnostics, artifacts, cache and reproducibility

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 210.

### Execution steps

- [ ] **T-APP-081.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Compiler/Build Monitor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-081.02 — Specify the complete target boundary**
  - Action: Compiler/Build Monitor must supply: graphs, diagnostics, artifacts, cache and reproducibility. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-081.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-081.03 — Implement the exact target behavior**
  - Action: Implement or reuse Compiler/Build Monitor through the shared platform contract, delivering every part of: graphs, diagnostics, artifacts, cache and reproducibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-081.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-081.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-081.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Compiler/Build Monitor.
- [ ] **T-APP-081.05 — Qualify and retain this target's own result**
  - Action: Bind Compiler/Build Monitor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-081.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-086"></a>
## T-APP-086 — ABI/Provenance Inspector

**Original requirement:** binaries, sections, symbols, dependencies and source identity

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 215.

### Execution steps

- [ ] **T-APP-086.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ABI/Provenance Inspector to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-086.02 — Specify the complete target boundary**
  - Action: ABI/Provenance Inspector must supply: binaries, sections, symbols, dependencies and source identity. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-086.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-086.03 — Implement the exact target behavior**
  - Action: Implement or reuse ABI/Provenance Inspector through the shared platform contract, delivering every part of: binaries, sections, symbols, dependencies and source identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-086.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-086.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: malformed/import-bomb input; encoding/format mismatch; huge document; undo after partial operation; disk full; cancelled export; crash before/after save; inaccessible error state.
  - Requires: T-APP-086.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ABI/Provenance Inspector.
- [ ] **T-APP-086.05 — Qualify and retain this target's own result**
  - Action: Bind ABI/Provenance Inspector to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-086.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-001"></a>
## T-ZLP-001 — module/import system

**Original requirement:** deterministic resolution, cycles, visibility and diagnostics

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 17.

### Execution steps

- [ ] **T-ZLP-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve module/import system to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-001.02 — Specify the complete target boundary**
  - Action: module/import system must supply: deterministic resolution, cycles, visibility and diagnostics. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse module/import system through the shared platform contract, delivering every part of: deterministic resolution, cycles, visibility and diagnostics. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-001.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for module/import system.
- [ ] **T-ZLP-001.05 — Qualify and retain this target's own result**
  - Action: Bind module/import system to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-002"></a>
## T-ZLP-002 — namespaces/scopes

**Original requirement:** exact shadowing, qualification, exports and errors

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 18.

### Execution steps

- [ ] **T-ZLP-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve namespaces/scopes to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-002.02 — Specify the complete target boundary**
  - Action: namespaces/scopes must supply: exact shadowing, qualification, exports and errors. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse namespaces/scopes through the shared platform contract, delivering every part of: exact shadowing, qualification, exports and errors. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-002.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for namespaces/scopes.
- [ ] **T-ZLP-002.05 — Qualify and retain this target's own result**
  - Action: Bind namespaces/scopes to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-003"></a>
## T-ZLP-003 — source locations/diagnostics

**Original requirement:** spans, notes, fix hints and stable machine output

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 19.

### Execution steps

- [ ] **T-ZLP-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve source locations/diagnostics to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-003.02 — Specify the complete target boundary**
  - Action: source locations/diagnostics must supply: spans, notes, fix hints and stable machine output. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse source locations/diagnostics through the shared platform contract, delivering every part of: spans, notes, fix hints and stable machine output. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-003.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for source locations/diagnostics.
- [ ] **T-ZLP-003.05 — Qualify and retain this target's own result**
  - Action: Bind source locations/diagnostics to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-004"></a>
## T-ZLP-004 — error/result model

**Original requirement:** explicit propagation, cleanup and no silent false success

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 20.

### Execution steps

- [ ] **T-ZLP-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve error/result model to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-004.02 — Specify the complete target boundary**
  - Action: error/result model must supply: explicit propagation, cleanup and no silent false success. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse error/result model through the shared platform contract, delivering every part of: explicit propagation, cleanup and no silent false success. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-004.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for error/result model.
- [ ] **T-ZLP-004.05 — Qualify and retain this target's own result**
  - Action: Bind error/result model to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-005"></a>
## T-ZLP-005 — exact integers

**Original requirement:** signed/unsigned widths, conversions and ABI agreement

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 21.

### Execution steps

- [ ] **T-ZLP-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve exact integers to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-005.02 — Specify the complete target boundary**
  - Action: exact integers must supply: signed/unsigned widths, conversions and ABI agreement. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse exact integers through the shared platform contract, delivering every part of: signed/unsigned widths, conversions and ABI agreement. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-005.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for exact integers.
- [ ] **T-ZLP-005.05 — Qualify and retain this target's own result**
  - Action: Bind exact integers to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-006"></a>
## T-ZLP-006 — byte and byte-string types

**Original requirement:** binary-safe indexing/slicing and no text conflation

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 22.

### Execution steps

- [ ] **T-ZLP-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve byte and byte-string types to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-006.02 — Specify the complete target boundary**
  - Action: byte and byte-string types must supply: binary-safe indexing/slicing and no text conflation. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse byte and byte-string types through the shared platform contract, delivering every part of: binary-safe indexing/slicing and no text conflation. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-006.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for byte and byte-string types.
- [ ] **T-ZLP-006.05 — Qualify and retain this target's own result**
  - Action: Bind byte and byte-string types to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-007"></a>
## T-ZLP-007 — arithmetic modes

**Original requirement:** checked/wrapping/saturating operations and overflow proof

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 23.

### Execution steps

- [ ] **T-ZLP-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve arithmetic modes to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-007.02 — Specify the complete target boundary**
  - Action: arithmetic modes must supply: checked/wrapping/saturating operations and overflow proof. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse arithmetic modes through the shared platform contract, delivering every part of: checked/wrapping/saturating operations and overflow proof. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-007.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for arithmetic modes.
- [ ] **T-ZLP-007.05 — Qualify and retain this target's own result**
  - Action: Bind arithmetic modes to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-008"></a>
## T-ZLP-008 — enums/tagged unions

**Original requirement:** stable layout, exhaustiveness and unknown-version behavior

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 24.

### Execution steps

- [ ] **T-ZLP-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve enums/tagged unions to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-008.02 — Specify the complete target boundary**
  - Action: enums/tagged unions must supply: stable layout, exhaustiveness and unknown-version behavior. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse enums/tagged unions through the shared platform contract, delivering every part of: stable layout, exhaustiveness and unknown-version behavior. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-008.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for enums/tagged unions.
- [ ] **T-ZLP-008.05 — Qualify and retain this target's own result**
  - Action: Bind enums/tagged unions to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-009"></a>
## T-ZLP-009 — generics/interfaces

**Original requirement:** bounded monomorphization or dictionaries with ABI rules

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 25.

### Execution steps

- [ ] **T-ZLP-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve generics/interfaces to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-009.02 — Specify the complete target boundary**
  - Action: generics/interfaces must supply: bounded monomorphization or dictionaries with ABI rules. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse generics/interfaces through the shared platform contract, delivering every part of: bounded monomorphization or dictionaries with ABI rules. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-009.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for generics/interfaces.
- [ ] **T-ZLP-009.05 — Qualify and retain this target's own result**
  - Action: Bind generics/interfaces to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-010"></a>
## T-ZLP-010 — function values/closures

**Original requirement:** calling convention, captures, lifetime and code/data authority

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 26.

### Execution steps

- [ ] **T-ZLP-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve function values/closures to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-010.02 — Specify the complete target boundary**
  - Action: function values/closures must supply: calling convention, captures, lifetime and code/data authority. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse function values/closures through the shared platform contract, delivering every part of: calling convention, captures, lifetime and code/data authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-010.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for function values/closures.
- [ ] **T-ZLP-010.05 — Qualify and retain this target's own result**
  - Action: Bind function values/closures to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-011"></a>
## T-ZLP-011 — compile-time evaluation

**Original requirement:** deterministic bounded execution and dependency tracking

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 27.

### Execution steps

- [ ] **T-ZLP-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve compile-time evaluation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-011.02 — Specify the complete target boundary**
  - Action: compile-time evaluation must supply: deterministic bounded execution and dependency tracking. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse compile-time evaluation through the shared platform contract, delivering every part of: deterministic bounded execution and dependency tracking. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-011.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for compile-time evaluation.
- [ ] **T-ZLP-011.05 — Qualify and retain this target's own result**
  - Action: Bind compile-time evaluation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-012"></a>
## T-ZLP-012 — attributes/annotations

**Original requirement:** versioned validated metadata for ABI/sections/generation

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 28.

### Execution steps

- [ ] **T-ZLP-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve attributes/annotations to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-012.02 — Specify the complete target boundary**
  - Action: attributes/annotations must supply: versioned validated metadata for ABI/sections/generation. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse attributes/annotations through the shared platform contract, delivering every part of: versioned validated metadata for ABI/sections/generation. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-012.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for attributes/annotations.
- [ ] **T-ZLP-012.05 — Qualify and retain this target's own result**
  - Action: Bind attributes/annotations to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-013"></a>
## T-ZLP-013 — conditional compilation

**Original requirement:** target/feature profiles included in artifact identity

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 29.

### Execution steps

- [ ] **T-ZLP-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve conditional compilation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-013.02 — Specify the complete target boundary**
  - Action: conditional compilation must supply: target/feature profiles included in artifact identity. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse conditional compilation through the shared platform contract, delivering every part of: target/feature profiles included in artifact identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-013.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for conditional compilation.
- [ ] **T-ZLP-013.05 — Qualify and retain this target's own result**
  - Action: Bind conditional compilation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-014"></a>
## T-ZLP-014 — package/module documentation

**Original requirement:** generated API docs with source/provenance links

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 30.

### Execution steps

- [ ] **T-ZLP-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve package/module documentation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-014.02 — Specify the complete target boundary**
  - Action: package/module documentation must supply: generated API docs with source/provenance links. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse package/module documentation through the shared platform contract, delivering every part of: generated API docs with source/provenance links. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-014.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for package/module documentation.
- [ ] **T-ZLP-014.05 — Qualify and retain this target's own result**
  - Action: Bind package/module documentation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-020"></a>
## T-ZLP-020 — packed/aligned records

**Original requirement:** explicit size/alignment/offset assertions

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 36.

### Execution steps

- [ ] **T-ZLP-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve packed/aligned records to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-020.02 — Specify the complete target boundary**
  - Action: packed/aligned records must supply: explicit size/alignment/offset assertions. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse packed/aligned records through the shared platform contract, delivering every part of: explicit size/alignment/offset assertions. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-020.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for packed/aligned records.
- [ ] **T-ZLP-020.05 — Qualify and retain this target's own result**
  - Action: Bind packed/aligned records to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-021"></a>
## T-ZLP-021 — bitfields

**Original requirement:** endian/width/range-defined hardware and protocol layout

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 37.

### Execution steps

- [ ] **T-ZLP-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve bitfields to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-021.02 — Specify the complete target boundary**
  - Action: bitfields must supply: endian/width/range-defined hardware and protocol layout. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse bitfields through the shared platform contract, delivering every part of: endian/width/range-defined hardware and protocol layout. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-021.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for bitfields.
- [ ] **T-ZLP-021.05 — Qualify and retain this target's own result**
  - Action: Bind bitfields to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-022"></a>
## T-ZLP-022 — typed raw pointers

**Original requirement:** address-space/const/nullable distinctions and explicit unsafe boundary

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 38.

### Execution steps

- [ ] **T-ZLP-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve typed raw pointers to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-022.02 — Specify the complete target boundary**
  - Action: typed raw pointers must supply: address-space/const/nullable distinctions and explicit unsafe boundary. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse typed raw pointers through the shared platform contract, delivering every part of: address-space/const/nullable distinctions and explicit unsafe boundary. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-022.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for typed raw pointers.
- [ ] **T-ZLP-022.05 — Qualify and retain this target's own result**
  - Action: Bind typed raw pointers to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-023"></a>
## T-ZLP-023 — volatile MMIO access

**Original requirement:** width/alignment/ordering semantics and target lowering

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 39.

### Execution steps

- [ ] **T-ZLP-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve volatile MMIO access to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-023.02 — Specify the complete target boundary**
  - Action: volatile MMIO access must supply: width/alignment/ordering semantics and target lowering. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse volatile MMIO access through the shared platform contract, delivering every part of: width/alignment/ordering semantics and target lowering. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-023.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for volatile MMIO access.
- [ ] **T-ZLP-023.05 — Qualify and retain this target's own result**
  - Action: Bind volatile MMIO access to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-024"></a>
## T-ZLP-024 — port I/O intrinsics

**Original requirement:** x86 width and privilege rules

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 40.

### Execution steps

- [ ] **T-ZLP-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve port I/O intrinsics to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-024.02 — Specify the complete target boundary**
  - Action: port I/O intrinsics must supply: x86 width and privilege rules. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse port I/O intrinsics through the shared platform contract, delivering every part of: x86 width and privilege rules. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-024.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for port I/O intrinsics.
- [ ] **T-ZLP-024.05 — Qualify and retain this target's own result**
  - Action: Bind port I/O intrinsics to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-025"></a>
## T-ZLP-025 — memory/compiler barriers

**Original requirement:** target-specific ordering with conformance tests

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 41.

### Execution steps

- [ ] **T-ZLP-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve memory/compiler barriers to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-025.02 — Specify the complete target boundary**
  - Action: memory/compiler barriers must supply: target-specific ordering with conformance tests. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse memory/compiler barriers through the shared platform contract, delivering every part of: target-specific ordering with conformance tests. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-025.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for memory/compiler barriers.
- [ ] **T-ZLP-025.05 — Qualify and retain this target's own result**
  - Action: Bind memory/compiler barriers to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-026"></a>
## T-ZLP-026 — atomic operations

**Original requirement:** memory orders, widths, lock-free facts and fallbacks

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 42.

### Execution steps

- [ ] **T-ZLP-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve atomic operations to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-026.02 — Specify the complete target boundary**
  - Action: atomic operations must supply: memory orders, widths, lock-free facts and fallbacks. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse atomic operations through the shared platform contract, delivering every part of: memory orders, widths, lock-free facts and fallbacks. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-026.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for atomic operations.
- [ ] **T-ZLP-026.05 — Qualify and retain this target's own result**
  - Action: Bind atomic operations to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-027"></a>
## T-ZLP-027 — calling-convention declarations

**Original requirement:** SysV, interrupt, firmware and generated zlIDL ABI

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 43.

### Execution steps

- [ ] **T-ZLP-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve calling-convention declarations to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-027.02 — Specify the complete target boundary**
  - Action: calling-convention declarations must supply: SysV, interrupt, firmware and generated zlIDL ABI. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse calling-convention declarations through the shared platform contract, delivering every part of: SysV, interrupt, firmware and generated zlIDL ABI. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-027.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for calling-convention declarations.
- [ ] **T-ZLP-027.05 — Qualify and retain this target's own result**
  - Action: Bind calling-convention declarations to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-028"></a>
## T-ZLP-028 — external symbol/assembly declarations

**Original requirement:** typed linkage, clobbers and diagnostics

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 44.

### Execution steps

- [ ] **T-ZLP-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve external symbol/assembly declarations to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-028.02 — Specify the complete target boundary**
  - Action: external symbol/assembly declarations must supply: typed linkage, clobbers and diagnostics. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse external symbol/assembly declarations through the shared platform contract, delivering every part of: typed linkage, clobbers and diagnostics. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-028.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for external symbol/assembly declarations.
- [ ] **T-ZLP-028.05 — Qualify and retain this target's own result**
  - Action: Bind external symbol/assembly declarations to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-029"></a>
## T-ZLP-029 — TLS/per-thread data

**Original requirement:** ABI, initialization, teardown and module interaction

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 45.

### Execution steps

- [ ] **T-ZLP-029.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve TLS/per-thread data to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-029.02 — Specify the complete target boundary**
  - Action: TLS/per-thread data must supply: ABI, initialization, teardown and module interaction. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-029.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-029.03 — Implement the exact target behavior**
  - Action: Implement or reuse TLS/per-thread data through the shared platform contract, delivering every part of: ABI, initialization, teardown and module interaction. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-029.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-029.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-029.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for TLS/per-thread data.
- [ ] **T-ZLP-029.05 — Qualify and retain this target's own result**
  - Action: Bind TLS/per-thread data to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-029.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-030"></a>
## T-ZLP-030 — endian/serialization primitives

**Original requirement:** checked length-first binary protocol support

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 46.

### Execution steps

- [ ] **T-ZLP-030.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve endian/serialization primitives to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-030.02 — Specify the complete target boundary**
  - Action: endian/serialization primitives must supply: checked length-first binary protocol support. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-030.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-030.03 — Implement the exact target behavior**
  - Action: Implement or reuse endian/serialization primitives through the shared platform contract, delivering every part of: checked length-first binary protocol support. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-030.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-030.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-030.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for endian/serialization primitives.
- [ ] **T-ZLP-030.05 — Qualify and retain this target's own result**
  - Action: Bind endian/serialization primitives to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-030.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-031"></a>
## T-ZLP-031 — ownership/lifetime helpers

**Original requirement:** explicit resource cleanup patterns without hiding authority

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 47.

### Execution steps

- [ ] **T-ZLP-031.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ownership/lifetime helpers to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-031.02 — Specify the complete target boundary**
  - Action: ownership/lifetime helpers must supply: explicit resource cleanup patterns without hiding authority. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-031.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-031.03 — Implement the exact target behavior**
  - Action: Implement or reuse ownership/lifetime helpers through the shared platform contract, delivering every part of: explicit resource cleanup patterns without hiding authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-031.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-031.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-031.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ownership/lifetime helpers.
- [ ] **T-ZLP-031.05 — Qualify and retain this target's own result**
  - Action: Bind ownership/lifetime helpers to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-031.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-040"></a>
## T-ZLP-040 — ELF64 executable output

**Original requirement:** correct headers/segments/permissions/relocations/entry

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 53.

### Execution steps

- [ ] **T-ZLP-040.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ELF64 executable output to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-040.02 — Specify the complete target boundary**
  - Action: ELF64 executable output must supply: correct headers/segments/permissions/relocations/entry. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-040.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-040.03 — Implement the exact target behavior**
  - Action: Implement or reuse ELF64 executable output through the shared platform contract, delivering every part of: correct headers/segments/permissions/relocations/entry. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-040.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-040.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-040.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ELF64 executable output.
- [ ] **T-ZLP-040.05 — Qualify and retain this target's own result**
  - Action: Bind ELF64 executable output to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-040.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-041"></a>
## T-ZLP-041 — ELF64 relocatable output

**Original requirement:** sections, symbols, RELA, COMDAT/weak policy and deterministic bytes

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 54.

### Execution steps

- [ ] **T-ZLP-041.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ELF64 relocatable output to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-041.02 — Specify the complete target boundary**
  - Action: ELF64 relocatable output must supply: sections, symbols, RELA, COMDAT/weak policy and deterministic bytes. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-041.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-041.03 — Implement the exact target behavior**
  - Action: Implement or reuse ELF64 relocatable output through the shared platform contract, delivering every part of: sections, symbols, RELA, COMDAT/weak policy and deterministic bytes. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-041.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-041.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-041.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ELF64 relocatable output.
- [ ] **T-ZLP-041.05 — Qualify and retain this target's own result**
  - Action: Bind ELF64 relocatable output to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-041.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-042"></a>
## T-ZLP-042 — shared-object/PIC output

**Original requirement:** GOT/PLT/TLS/relocations and versioned ABI

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 55.

### Execution steps

- [ ] **T-ZLP-042.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve shared-object/PIC output to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-042.02 — Specify the complete target boundary**
  - Action: shared-object/PIC output must supply: GOT/PLT/TLS/relocations and versioned ABI. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-042.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-042.03 — Implement the exact target behavior**
  - Action: Implement or reuse shared-object/PIC output through the shared platform contract, delivering every part of: GOT/PLT/TLS/relocations and versioned ABI. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-042.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-042.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-042.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for shared-object/PIC output.
- [ ] **T-ZLP-042.05 — Qualify and retain this target's own result**
  - Action: Bind shared-object/PIC output to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-042.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-043"></a>
## T-ZLP-043 — named/custom sections

**Original requirement:** boot requests, metadata, init/fini and linker retention

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 56.

### Execution steps

- [ ] **T-ZLP-043.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve named/custom sections to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-043.02 — Specify the complete target boundary**
  - Action: named/custom sections must supply: boot requests, metadata, init/fini and linker retention. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-043.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-043.03 — Implement the exact target behavior**
  - Action: Implement or reuse named/custom sections through the shared platform contract, delivering every part of: boot requests, metadata, init/fini and linker retention. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-043.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-043.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-043.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for named/custom sections.
- [ ] **T-ZLP-043.05 — Qualify and retain this target's own result**
  - Action: Bind named/custom sections to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-043.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-044"></a>
## T-ZLP-044 — archive librarian

**Original requirement:** deterministic static archives and indexes

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 57.

### Execution steps

- [ ] **T-ZLP-044.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve archive librarian to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-044.02 — Specify the complete target boundary**
  - Action: archive librarian must supply: deterministic static archives and indexes. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-044.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-044.03 — Implement the exact target behavior**
  - Action: Implement or reuse archive librarian through the shared platform contract, delivering every part of: deterministic static archives and indexes. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-044.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-044.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-044.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for archive librarian.
- [ ] **T-ZLP-044.05 — Qualify and retain this target's own result**
  - Action: Bind archive librarian to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-044.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-045"></a>
## T-ZLP-045 — zl linker

**Original requirement:** checked layout, scripts, GC, symbols, diagnostics and maps

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 58.

### Execution steps

- [ ] **T-ZLP-045.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zl linker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-045.02 — Specify the complete target boundary**
  - Action: zl linker must supply: checked layout, scripts, GC, symbols, diagnostics and maps. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-045.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-045.03 — Implement the exact target behavior**
  - Action: Implement or reuse zl linker through the shared platform contract, delivering every part of: checked layout, scripts, GC, symbols, diagnostics and maps. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-045.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-045.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-045.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zl linker.
- [ ] **T-ZLP-045.05 — Qualify and retain this target's own result**
  - Action: Bind zl linker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-045.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-046"></a>
## T-ZLP-046 — debug information

**Original requirement:** line/function/type data consumable by debugger/profiler

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 59.

### Execution steps

- [ ] **T-ZLP-046.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve debug information to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-046.02 — Specify the complete target boundary**
  - Action: debug information must supply: line/function/type data consumable by debugger/profiler. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-046.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-046.03 — Implement the exact target behavior**
  - Action: Implement or reuse debug information through the shared platform contract, delivering every part of: line/function/type data consumable by debugger/profiler. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-046.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-046.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-046.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for debug information.
- [ ] **T-ZLP-046.05 — Qualify and retain this target's own result**
  - Action: Bind debug information to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-046.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-047"></a>
## T-ZLP-047 — object/binary inspection tools

**Original requirement:** sections/symbols/relocations/dependencies/provenance

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 60.

### Execution steps

- [ ] **T-ZLP-047.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve object/binary inspection tools to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-047.02 — Specify the complete target boundary**
  - Action: object/binary inspection tools must supply: sections/symbols/relocations/dependencies/provenance. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-047.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-047.03 — Implement the exact target behavior**
  - Action: Implement or reuse object/binary inspection tools through the shared platform contract, delivering every part of: sections/symbols/relocations/dependencies/provenance. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-047.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-047.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-047.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for object/binary inspection tools.
- [ ] **T-ZLP-047.05 — Qualify and retain this target's own result**
  - Action: Bind object/binary inspection tools to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-047.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-048"></a>
## T-ZLP-048 — dependency/build metadata

**Original requirement:** exact source/generated/tool inputs and cache keys

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 61.

### Execution steps

- [ ] **T-ZLP-048.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve dependency/build metadata to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-048.02 — Specify the complete target boundary**
  - Action: dependency/build metadata must supply: exact source/generated/tool inputs and cache keys. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-048.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-048.03 — Implement the exact target behavior**
  - Action: Implement or reuse dependency/build metadata through the shared platform contract, delivering every part of: exact source/generated/tool inputs and cache keys. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-048.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-048.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-048.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for dependency/build metadata.
- [ ] **T-ZLP-048.05 — Qualify and retain this target's own result**
  - Action: Bind dependency/build metadata to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-048.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-049"></a>
## T-ZLP-049 — incremental build/cache

**Original requirement:** correctness-first invalidation and reproducible fallback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 62.

### Execution steps

- [ ] **T-ZLP-049.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve incremental build/cache to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-049.02 — Specify the complete target boundary**
  - Action: incremental build/cache must supply: correctness-first invalidation and reproducible fallback. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-049.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-049.03 — Implement the exact target behavior**
  - Action: Implement or reuse incremental build/cache through the shared platform contract, delivering every part of: correctness-first invalidation and reproducible fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-049.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-049.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-049.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for incremental build/cache.
- [ ] **T-ZLP-049.05 — Qualify and retain this target's own result**
  - Action: Bind incremental build/cache to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-049.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-050"></a>
## T-ZLP-050 — zl formatter/linter

**Original requirement:** stable formatting and semantic diagnostics

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 63.

### Execution steps

- [ ] **T-ZLP-050.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zl formatter/linter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-050.02 — Specify the complete target boundary**
  - Action: zl formatter/linter must supply: stable formatting and semantic diagnostics. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-050.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-050.03 — Implement the exact target behavior**
  - Action: Implement or reuse zl formatter/linter through the shared platform contract, delivering every part of: stable formatting and semantic diagnostics. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-050.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-050.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-050.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zl formatter/linter.
- [ ] **T-ZLP-050.05 — Qualify and retain this target's own result**
  - Action: Bind zl formatter/linter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-050.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-051"></a>
## T-ZLP-051 — language server

**Original requirement:** parse/type/navigation/completion over exact compiler semantics

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 64.

### Execution steps

- [ ] **T-ZLP-051.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve language server to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-051.02 — Specify the complete target boundary**
  - Action: language server must supply: parse/type/navigation/completion over exact compiler semantics. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-051.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-051.03 — Implement the exact target behavior**
  - Action: Implement or reuse language server through the shared platform contract, delivering every part of: parse/type/navigation/completion over exact compiler semantics. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-051.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-051.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-051.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for language server.
- [ ] **T-ZLP-051.05 — Qualify and retain this target's own result**
  - Action: Bind language server to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-051.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-052"></a>
## T-ZLP-052 — package manager integration

**Original requirement:** locked modules, signatures, licenses and build receipts

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 65.

### Execution steps

- [ ] **T-ZLP-052.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve package manager integration to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-052.02 — Specify the complete target boundary**
  - Action: package manager integration must supply: locked modules, signatures, licenses and build receipts. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-052.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-052.03 — Implement the exact target behavior**
  - Action: Implement or reuse package manager integration through the shared platform contract, delivering every part of: locked modules, signatures, licenses and build receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-052.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-052.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-052.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for package manager integration.
- [ ] **T-ZLP-052.05 — Qualify and retain this target's own result**
  - Action: Bind package manager integration to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-052.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-053"></a>
## T-ZLP-053 — zlIDL generator

**Original requirement:** schemas to zl/C bindings, limits and conformance tests

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 66.

### Execution steps

- [ ] **T-ZLP-053.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zlIDL generator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-053.02 — Specify the complete target boundary**
  - Action: zlIDL generator must supply: schemas to zl/C bindings, limits and conformance tests. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-053.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-053.03 — Implement the exact target behavior**
  - Action: Implement or reuse zlIDL generator through the shared platform contract, delivering every part of: schemas to zl/C bindings, limits and conformance tests. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-053.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-053.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-053.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zlIDL generator.
- [ ] **T-ZLP-053.05 — Qualify and retain this target's own result**
  - Action: Bind zlIDL generator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-053.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-054"></a>
## T-ZLP-054 — compiler hostile corpus

**Original requirement:** parser/type/codegen/link malformed and differential cases

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 67.

### Execution steps

- [ ] **T-ZLP-054.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve compiler hostile corpus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-054.02 — Specify the complete target boundary**
  - Action: compiler hostile corpus must supply: parser/type/codegen/link malformed and differential cases. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-054.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-054.03 — Implement the exact target behavior**
  - Action: Implement or reuse compiler hostile corpus through the shared platform contract, delivering every part of: parser/type/codegen/link malformed and differential cases. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-054.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-054.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-054.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for compiler hostile corpus.
- [ ] **T-ZLP-054.05 — Qualify and retain this target's own result**
  - Action: Bind compiler hostile corpus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-054.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-zlp-055"></a>
## T-ZLP-055 — fixpoint and bootstrap ledger

**Original requirement:** stage identity, semantic/hash comparison and rollback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 68.

### Execution steps

- [ ] **T-ZLP-055.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve fixpoint and bootstrap ledger to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-21, D-22, H-16.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-ZLP-055.02 — Specify the complete target boundary**
  - Action: fixpoint and bootstrap ledger must supply: stage identity, semantic/hash comparison and rollback. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-ZLP-055.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-ZLP-055.03 — Implement the exact target behavior**
  - Action: Implement or reuse fixpoint and bootstrap ledger through the shared platform contract, delivering every part of: stage identity, semantic/hash comparison and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-ZLP-055.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-ZLP-055.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: malformed source; backend semantic disagreement; overflow/conversion mismatch; ABI/layout mismatch; compile-time resource exhaustion; undefined lifetime; stale bootstrap seed; nondeterministic output.
  - Requires: T-ZLP-055.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for fixpoint and bootstrap ledger.
- [ ] **T-ZLP-055.05 — Qualify and retain this target's own result**
  - Action: Bind fixpoint and bootstrap ledger to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-ZLP-055.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
