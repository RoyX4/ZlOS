# MP-20: Rebuild and qualify the complete system from inside zlOS

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `src/selfhost/; kernel/tools/; docs/program/; .github/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-20` exports: A matching bootable system rebuilt inside zlOS; full programme completion additionally requires every retained feature/target acceptance gate.

The handoff enables only its named subset. `CLOSE-20` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-20.01 — Freeze the trusted seed, source generation, toolchain, package graph and exact build configuration

Freeze the trusted seed, source generation, toolchain, package graph and exact build configuration.

**Requires:** `D-01`, `D-02`, `D-03`, `D-22`, `D-25`, `H-00`, `H-01`, `H-02`, `H-03`, `H-04`, `H-05`, `H-06`, `H-07`, `H-08`, `H-09`, `H-10`, `H-11`, `H-12`, `H-13`, `H-14`, `H-15`, `H-16`, `H-17`, `H-18`, `H-19`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.02 — Run compiler/build/package tools as isolated zlOS processes with bounded resources and declared inputs

Run compiler/build/package tools as isolated zlOS processes with bounded resources and declared inputs.

**Requires:** `M-20.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.03 — Build the compiler through its source-stage fixpoint and compare both artifact identities and semantic test behavior

Build the compiler through its source-stage fixpoint and compare both artifact identities and semantic test behavior.

**Requires:** `M-20.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.04 — Build bootloader, kernel, services, apps and packages into a new identified bootable generation from inside zlOS

Build bootloader, kernel, services, apps and packages into a new identified bootable generation from inside zlOS.

**Requires:** `M-20.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.05 — Boot that generated system and prove its inside-guest artifact identity matches the generated release

Boot that generated system and prove its inside-guest artifact identity matches the generated release.

**Requires:** `M-20.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-20 — Bounded development handoff: Rebuild and qualify the complete system from inside zlOS

A matching bootable system rebuilt inside zlOS; full programme completion additionally requires every retained feature/target acceptance gate.

**Requires:** `M-20.05`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-20.06 — Run update/rollback, corruption/power-cut, fault isolation, performance, accessibility, localization, security and visual suites

Run update/rollback, corruption/power-cut, fault isolation, performance, accessibility, localization, security and visual suites.

**Requires:** `M-20.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.07 — Qualify each advertised physical/architecture profile independently and retain every unqualified catalogue target as open

Qualify each advertised physical/architecture profile independently and retain every unqualified catalogue target as open.

**Requires:** `M-20.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.08 — Audit the complete 906-feature, 611-target, 174-contract and current-app/game matrices

Audit the complete 906-feature, 611-target, 174-contract and current-app/game matrices; no missing item is hidden by a release-profile choice.

**Requires:** `M-20.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-20.09 — Require independent refutation of completion claims, reproducible recovery and explicit release approval before publication

Require independent refutation of completion claims, reproducible recovery and explicit release approval before publication.

**Requires:** `M-20.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [C-P12.1](#c-p12-1) | contract | compiler/build tools as isolated zlOS processes |
| [C-P12.2](#c-p12-2) | contract | source/package build graph and cache |
| [C-P12.3](#c-p12-3) | contract | compiler fixpoint inside zlOS |
| [C-P12.4](#c-p12-4) | contract | rebuild bootable zlOS and pass assertion matrix |

<a id="c-p12-1"></a>
## C-P12.1 — compiler/build tools as isolated zlOS processes

**Original requirement:** compiler/build tools as isolated zlOS processes

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 634.

### Preserved original contract

- **Dependencies/current/provenance:** Phase 7 packages, Phase 9 toolchain, processes/files; zl self-host proof and banan build recipes; reject hidden host compiler requirement.
- **I/O and state:** source tree/toolchain manifest/build request in; compiler/object/package artifacts out; build request common state.
- **Invariants/failure:** explicit inputs/environment; bounded resources; no undeclared host files/network; build failure preserves diagnostics and prior artifacts.
- **Deterministic proof:** clean/offline build, missing input/tool, resource exhaustion, parallel dependency order, hermeticity probe.
- **Target proof:** QEMU then hardware zlOS runs compiler/build without host assistance.
- **Receipt/removal:** complete dependency/toolchain/artifact chain; imported bootstrap binary retained; no deletion yet.

### Execution steps

- [ ] **C-P12.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-22, D-25, H-20.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P12.1.02 — Resolve this contract's exact dependencies**
  - Action: Phase 7 packages, Phase 9 toolchain, processes/files; zl self-host proof and banan build recipes; reject hidden host compiler requirement. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P12.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P12.1.03 — I/O and state — compiler/build tools as isolated zlOS processes**
  - Action: source tree/toolchain manifest/build request in; compiler/object/package artifacts out; build request common state.
  - Requires: C-P12.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P12.1.
- [ ] **C-P12.1.04 — Invariants/failure — compiler/build tools as isolated zlOS processes**
  - Action: explicit inputs/environment; bounded resources; no undeclared host files/network; build failure preserves diagnostics and prior artifacts.
  - Requires: C-P12.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P12.1.
- [ ] **C-P12.1.05 — Deterministic proof — compiler/build tools as isolated zlOS processes**
  - Action: clean/offline build, missing input/tool, resource exhaustion, parallel dependency order, hermeticity probe.
  - Requires: C-P12.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P12.1.
- [ ] **C-P12.1.06 — Target proof — compiler/build tools as isolated zlOS processes**
  - Action: QEMU then hardware zlOS runs compiler/build without host assistance.
  - Requires: C-P12.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P12.1.
- [ ] **C-P12.1.07 — Receipt/removal — compiler/build tools as isolated zlOS processes**
  - Action: complete dependency/toolchain/artifact chain; imported bootstrap binary retained; no deletion yet.
  - Requires: C-P12.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P12.1.
- [ ] **C-P12.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P12.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P12.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p12-2"></a>
## C-P12.2 — source/package build graph and cache

**Original requirement:** source/package build graph and cache

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 643.

### Preserved original contract

- **Dependencies/current/provenance:** P12.1 and package/ports; banan dependency hashes, freestanding header generator; reject timestamps/mutable network as hidden input.
- **I/O and state:** target/dependency/source/toolchain digests in; ordered jobs and content-addressed results out; node `Unknown -> Cached|Building -> Passed|Failed`.
- **Invariants/failure:** cycle detection; cache key covers every input; failed node cannot publish; reproducible log ordering/identity.
- **Deterministic proof:** cache hit/miss, changed transitive input, cycle, interrupted build, corrupt cache, parallel determinism.
- **Target proof:** two clean zlOS builds from same inputs produce matching declared artifacts.
- **Receipt/removal:** graph/cache/artifact digests; cache bypass rebuild; no legacy build removal until P12.4.

### Execution steps

- [ ] **C-P12.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-22, D-25, H-20.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P12.2.02 — Resolve this contract's exact dependencies**
  - Action: P12.1 and package/ports; banan dependency hashes, freestanding header generator; reject timestamps/mutable network as hidden input. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P12.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P12.2.03 — I/O and state — source/package build graph and cache**
  - Action: target/dependency/source/toolchain digests in; ordered jobs and content-addressed results out; node `Unknown -> Cached|Building -> Passed|Failed`.
  - Requires: C-P12.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P12.2.
- [ ] **C-P12.2.04 — Invariants/failure — source/package build graph and cache**
  - Action: cycle detection; cache key covers every input; failed node cannot publish; reproducible log ordering/identity.
  - Requires: C-P12.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P12.2.
- [ ] **C-P12.2.05 — Deterministic proof — source/package build graph and cache**
  - Action: cache hit/miss, changed transitive input, cycle, interrupted build, corrupt cache, parallel determinism.
  - Requires: C-P12.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P12.2.
- [ ] **C-P12.2.06 — Target proof — source/package build graph and cache**
  - Action: two clean zlOS builds from same inputs produce matching declared artifacts.
  - Requires: C-P12.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P12.2.
- [ ] **C-P12.2.07 — Receipt/removal — source/package build graph and cache**
  - Action: graph/cache/artifact digests; cache bypass rebuild; no legacy build removal until P12.4.
  - Requires: C-P12.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P12.2.
- [ ] **C-P12.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P12.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P12.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p12-3"></a>
## C-P12.3 — compiler fixpoint inside zlOS

**Original requirement:** compiler fixpoint inside zlOS

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 652.

### Preserved original contract

- **Dependencies/current/provenance:** P12.1/P12.2 and zl self-host compiler; unix-history provenance; reject fixed historical hash as fixpoint.
- **I/O and state:** compiler source and stage-0 compiler in; gen1/gen2 compiler artifacts plus outputs out.
- **Invariants/failure:** fixpoint means same compiler function reaches byte/semantic equality under declared nondeterminism policy; source changes may change final hash.
- **Deterministic proof:** clean two-generation build, intentional compiler/source perturbation, stage identity, interpreter/LLVM/native semantic corpus.
- **Target proof:** QEMU/hardware in-zlOS fixpoint with no host compilation step.
- **Receipt/removal:** source/stage/gen/toolchain/artifact digests and comparisons; bootstrap compiler retained until independent recovery path.

### Execution steps

- [ ] **C-P12.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-22, D-25, H-20.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P12.3.02 — Resolve this contract's exact dependencies**
  - Action: P12.1/P12.2 and zl self-host compiler; unix-history provenance; reject fixed historical hash as fixpoint. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P12.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P12.3.03 — I/O and state — compiler fixpoint inside zlOS**
  - Action: compiler source and stage-0 compiler in; gen1/gen2 compiler artifacts plus outputs out.
  - Requires: C-P12.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P12.3.
- [ ] **C-P12.3.04 — Invariants/failure — compiler fixpoint inside zlOS**
  - Action: fixpoint means same compiler function reaches byte/semantic equality under declared nondeterminism policy; source changes may change final hash.
  - Requires: C-P12.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P12.3.
- [ ] **C-P12.3.05 — Deterministic proof — compiler fixpoint inside zlOS**
  - Action: clean two-generation build, intentional compiler/source perturbation, stage identity, interpreter/LLVM/native semantic corpus.
  - Requires: C-P12.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P12.3.
- [ ] **C-P12.3.06 — Target proof — compiler fixpoint inside zlOS**
  - Action: QEMU/hardware in-zlOS fixpoint with no host compilation step.
  - Requires: C-P12.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P12.3.
- [ ] **C-P12.3.07 — Receipt/removal — compiler fixpoint inside zlOS**
  - Action: source/stage/gen/toolchain/artifact digests and comparisons; bootstrap compiler retained until independent recovery path.
  - Requires: C-P12.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P12.3.
- [ ] **C-P12.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P12.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P12.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p12-4"></a>
## C-P12.4 — rebuild bootable zlOS and pass assertion matrix

**Original requirement:** rebuild bootable zlOS and pass assertion matrix

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 661.

### Preserved original contract

- **Dependencies/current/provenance:** P12.2/P12.3, P9 ELF, Phase 0 boot matrix; Hyper assertion boot; reject “build succeeded” as self-host OS proof.
- **I/O and state:** declared system source/package graph in; signed boot image out.
- **Invariants/failure:** shipped inventory matches graph; boot artifact self-identifies; firmware/media/filesystem/failure matrix uses rebuilt artifact; no stale image can pass.
- **Deterministic proof:** image structure, source-to-object manifest, reproducible regions, intentional stale/wrong module/hash failures.
- **Target proof:** QEMU full matrix then ThinkPad boot/ZLLOG/desktop/file/network subset with rebuilt artifact.
- **Receipt/removal:** end-to-end source-to-hardware chain; known-good previous image remains boot fallback; bootstrap C becomes optional only now.

### Execution steps

- [ ] **C-P12.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-03, D-22, D-25, H-20.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P12.4.02 — Resolve this contract's exact dependencies**
  - Action: P12.2/P12.3, P9 ELF, Phase 0 boot matrix; Hyper assertion boot; reject “build succeeded” as self-host OS proof. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P12.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P12.4.03 — I/O and state — rebuild bootable zlOS and pass assertion matrix**
  - Action: declared system source/package graph in; signed boot image out.
  - Requires: C-P12.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P12.4.
- [ ] **C-P12.4.04 — Invariants/failure — rebuild bootable zlOS and pass assertion matrix**
  - Action: shipped inventory matches graph; boot artifact self-identifies; firmware/media/filesystem/failure matrix uses rebuilt artifact; no stale image can pass.
  - Requires: C-P12.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P12.4.
- [ ] **C-P12.4.05 — Deterministic proof — rebuild bootable zlOS and pass assertion matrix**
  - Action: image structure, source-to-object manifest, reproducible regions, intentional stale/wrong module/hash failures.
  - Requires: C-P12.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P12.4.
- [ ] **C-P12.4.06 — Target proof — rebuild bootable zlOS and pass assertion matrix**
  - Action: QEMU full matrix then ThinkPad boot/ZLLOG/desktop/file/network subset with rebuilt artifact.
  - Requires: C-P12.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P12.4.
- [ ] **C-P12.4.07 — Receipt/removal — rebuild bootable zlOS and pass assertion matrix**
  - Action: end-to-end source-to-hardware chain; known-good previous image remains boot fallback; bootstrap C becomes optional only now.
  - Requires: C-P12.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P12.4.
- [ ] **C-P12.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P12.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P12.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.
