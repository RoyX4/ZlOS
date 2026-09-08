# zlOS: complete system roadmap

Status: full-scope roadmap accepted for execution, 2026-09-08. The first process slice and its allocator rollback repair pass the bounded host, native-QEMU and desktop regressions. The full boot matrix, later-main integration and hosted closure remain pending. No feature maturity labels are promoted.

[Current execution receipt](../evidence/process-spawn-wait-2026-09-08.md) · [Implemented spawn/wait ABI](userspace-spawn-wait-abi.md).

## The approach

Map the whole destination first, then execute one bounded piece at a time. Keep every destination visible while making the next piece precise enough to build and refute. A roadmap should answer: what comes next, why it comes next, what it needs, what must work afterward, what breaks it, and what evidence closes it.

This suite expands the existing programme rather than replacing it with a smaller product. It retains all **906 features, 611 named targets and 174 detailed source contracts across 21 phases**. The current application inventory contains **63 named implementations including 24 games**, plus the All Applications surface. Those are overlapping views of the same system; do not add their counts together as independent features.

Start with this page. Use the phase chapters for implementation work, the decision register for unresolved choices, and the next-process checklist for the immediate bounded slice.

- [Decisions and research gates](FULL-ROADMAP-DECISIONS.md): 25 questions with required outputs and acceptance conditions.
- [Immediate process-management sequence](FULL-ROADMAP-NEXT-PROCESS.md): 32 ordered steps tied to the existing audited contract.
- [Coverage and validation](FULL-ROADMAP-COVERAGE.md): measured inventory, checks, limitations and source identities.
- [Machine-readable plan](FULL-SYSTEM-ROADMAP.json): stable IDs, source requirements, tasks, prerequisites and topological order.
- [Canonical programme](../program/README.md) and [product delivery order](../program/PRODUCT-IMPLEMENTATION-ORDER.md): retained scope and delivery rules.

## Baseline and where work happens

Current implementation checkout: `/home/roy/Documents/repos/zl-linux-spawn-wait-2026-09-08`, branch `codex/spawn-wait-reconcile-2026-09-08`, with a pending uncommitted merge of main `02180af`.

Original planning checkpoint: `/home/roy/Documents/repos/zl-linux-integration-2026-09-06`, branch `codex/integrate-sweep-process`, starting commit `5a33421361c447b0a42fea75f2f82682630de2ca`. The separate `/home/roy/Documents/repos/zl` checkout is the language repository and is not this work location.

The baseline receipt is [integration hosted closure](../evidence/integration-hosted-closure-2026-09-06.md). Its implementation commit `dd1871a82c544f38692fd286d14d4923b7898fa1` passed 126 complete hosted gate steps in [run 34014513857](https://github.com/RoyX4/ZlOS/actions/runs/34014513857). The subsequent `5a33421` checkpoint changed status/evidence documents. The imported full-run ledger records **8 PROVED_CURRENT, 55 PARTIAL_CURRENT and 843 PLANNED_UNPROVED**. These are dated, contract-bounded evidence labels; eight proved rows do not imply eight entire subsystems are finished.

The receipt retains the unresolved host build-performance budget and physical qualification. It also records the previously pushed draft integration PR and concurrent worktree ownership. Re-read live GitHub/worktree state before the next implementation or publication; this roadmap does not assert those remote states will remain unchanged.

Working policy: preserve other agents' changes; claim the actual files before editing; use a persistent isolated checkout for conflicting work. On this four-core development machine, use one active implementation lane and only lightweight independent reads alongside it. Run expensive checks in the project's established contained/hosted lanes. Do not turn planning into a background QEMU/build job.

## How to follow the plan exactly

The plan has five levels: destination → phase → subsystem milestone → acceptance package → execution step.

| ID form | What it means | How to use it |
|---|---|---|
| `MP-03` | One complete subsystem phase | Read its milestones, packages and full exit conditions. |
| `M-03.03` | A concrete subsystem milestone | Deliver its named outcome; reuse the matching packages. |
| `H-03` | A bounded development handoff | Export only the explicitly proved interface subset. |
| `F-KR-018` | One canonical feature requirement | Preserve every clause; inspect existing behavior before changing it. |
| `C-P3.1`, `C-DA-01`, `C-VX-01` | An original research contract | Preserve the original dependency, state, failure, proof and removal rules. |
| `T-SVC-001` | One named target | Qualify that particular provider, service or app through its actual consumer. |
| `N-PROCESS.01` | A detailed immediate execution step | Follow the next-process checklist in its stated order. |
| `D-05` | A decision/research obligation | Produce the specified answer and evidence before dependent implementation. |
| `CLOSE-03` | Complete acceptance for a phase | Require all its full feature, contract and target obligations. |
| `PROGRAM-CLOSE` | Completion of the entire retained programme | Require every full phase; a narrower release cannot satisfy this. |

The feature, contract and target packages deliberately overlap. Implement shared code once; check it against each applicable acceptance contract. Their checklist entries are not separate projects and must not generate duplicate drivers, protocols, service registries or tests that merely repeat the implementation.

For each actual work session:

1. Refresh the source/branch/worktree/evidence comparison. Classify the selected requirement as already satisfied, partially implemented, missing, unreachable or insufficiently proved.
2. Choose the next useful product outcome whose concrete prerequisites are available. Use the product waves and phase dependencies below; repairing a regression in an earlier working path takes precedence.
3. Read the complete relevant feature, named-target and research contracts, including the source document's global rules. Record the exact provider versions, authority, inputs, outputs, state, bounds and cleanup. A same-phase reading list is not an exact dependency map.
4. Resolve the decisions that affect this bounded change. Record their scope and remaining extensions. A decision about the initial two-slot process path cannot claim to settle future SMP, credentials or dynamic-linker semantics.
5. Bind narrative dependencies to real interfaces and add any missing edges to the plan. If the design creates a cycle, split the producer contract explicitly under `D-02`; do not silently replace a full prerequisite with a mock.
6. Reuse the repo, standard library, platform and installed dependencies before building a new component. For a real bug, reproduce the failure first. Otherwise write the contract's meaningful positive and negative scenarios before admitting the implementation.
7. Implement the bounded outcome through the real consumer. Validate hostile inputs and preserve resource/durable-state ownership at every failure boundary.
8. Run the affected checks: host logic, actual build/image reachability, QEMU, interaction, performance, accessibility and exact physical targets as required. A test adapter proves only its declared scope. Do not repeat unrelated broad gates for document edits.
9. Check rollback, cancellation, fault, restart and removal. Update the exact evidence and remaining limits. Keep the old path until its replacement meets the source contract's removal rule.
10. Review the intended diff and current worktree status. Commit, push, merge, publish, sign, expose remote services or operate destructive hardware only within the user's actual authorization. A checklist item is not that authorization.
11. Select the next outcome. Do not substitute another bookkeeping pass for product work unless the bookkeeping itself fixes a false pass, data-loss path or unsafe operation.

Every checklist entry starts as `PLANNED`. Existing maturity is a separate dated field. Future progress must be recorded against actual acceptance evidence, with a source identity, applicable profiles and outstanding gaps. Do not count checked boxes as a percentage of the whole OS: items differ radically in size and overlapping requirements share implementation.

## Ordering without circular instructions

The canonical [21-phase dependency graph](../program/PHASES.md) remains the full programme ordering. This proposal adds **bounded development handoffs** so work can proceed without falsely finishing an entire subsystem first. Each handoff needs a producer, consumer, version, ownership rule, failure behavior and passing proof.

For example, the first isolated parent/child process path supplies enough process ownership to build credentials and service authority. Finishing the full process feature later still requires credentials, file-backed mappings, richer executable loading and their real providers. Similarly, VFS can consume basic processes before file-backed memory is complete; file-backed memory then consumes the proved VFS interface. Neither half is permitted to certify the other through circular assertions.

The host compiler is another case. Host-side type/layout/code-generation work can begin after the baseline comparison. `H-16` supplies those host-tested primitives. Running that compiler inside zlOS remains `M-16.09`, after the process, authority, filesystem and development-tool handoffs. The canonical completed-toolchain dependencies are retained.

The JSON contains the original phase dependency graph, the proposed handoff graph and explicit later joins. Its acyclic task order is a structural check, **not a claim that every narrative dependency or future API is already resolved**. Resolving those is required work inside each package. Full closure still checks all original source obligations. The canonical graph remains authoritative until the relevant D-02 handoff resolution is recorded; this proposal cannot itself waive a source prerequisite.

Physical qualification is a separate obligation wherever the feature requires it. An unavailable device leaves that target open while safe host/simulator development continues. “Later” and “profile-specific” preserve the full destination. They do not remove rows from `PROGRAM-CLOSE`.

## The full route

These phases are a map, not an instruction to complete every row in one phase before starting any work in the next. Follow the named handoffs and the product waves. Preserve the current desktop throughout the migration.

| Phase | Product outcome | Features | Original contracts | Named targets |
|---|---|---:|---:|---:|
| [MP-00](FULL-ROADMAP-MP-00.md) | Preserve the baseline and make every claim traceable | 28 | 17 | 2 |
| [MP-01](FULL-ROADMAP-MP-01.md) | Prove and preserve the current desktop and hardware baseline | 0 | 9 | 1 |
| [MP-02](FULL-ROADMAP-MP-02.md) | Own the complete boot, firmware and recovery path | 30 | 2 | 12 |
| [MP-03](FULL-ROADMAP-MP-03.md) | Make independent programs a reliable kernel facility | 40 | 6 | 2 |
| [MP-04](FULL-ROADMAP-MP-04.md) | Give programs explicit authority and safe communication | 40 | 8 | 10 |
| [MP-05](FULL-ROADMAP-MP-05.md) | Make every device follow one lifecycle and resource contract | 55 | 10 | 28 |
| [MP-06](FULL-ROADMAP-MP-06.md) | Make storage, files and updates survive failure | 40 | 12 | 37 |
| [MP-07](FULL-ROADMAP-MP-07.md) | Make the desktop an owned, recoverable service | 120 | 9 | 27 |
| [MP-08](FULL-ROADMAP-MP-08.md) | Make networking reliable and explicitly authorized | 35 | 6 | 24 |
| [MP-09](FULL-ROADMAP-MP-09.md) | Build audio, capture and synchronized media streams | 25 | 2 | 17 |
| [MP-10](FULL-ROADMAP-MP-10.md) | Finish the shared visual, accessibility and language system | 75 | 9 | 15 |
| [MP-11](FULL-ROADMAP-MP-11.md) | Give applications a complete runtime and shared services | 40 | 12 | 14 |
| [MP-12](FULL-ROADMAP-MP-12.md) | Migrate every existing app and game without losing behavior | 20 | 12 | 64 |
| [MP-13](FULL-ROADMAP-MP-13.md) | Turn the browser into an isolated and capable web platform | 50 | 10 | 2 |
| [MP-14](FULL-ROADMAP-MP-14.md) | Deliver the full application families as real workflows | 113 | 17 | 67 |
| [MP-15](FULL-ROADMAP-MP-15.md) | Make development tools and compatibility usable | 60 | 2 | 69 |
| [MP-16](FULL-ROADMAP-MP-16.md) | Complete the zl systems language and toolchain | 55 | 6 | 45 |
| [MP-17](FULL-ROADMAP-MP-17.md) | Add agents and automation through ordinary system authority | 30 | 7 | 34 |
| [MP-18](FULL-ROADMAP-MP-18.md) | Make releases, recovery and operations trustworthy | 30 | 9 | 46 |
| [MP-19](FULL-ROADMAP-MP-19.md) | Deliver every broader hardware and architecture target | 20 | 5 | 95 |
| [MP-20](FULL-ROADMAP-MP-20.md) | Rebuild and qualify the complete system from inside zlOS | 0 | 4 | 0 |

The corresponding human delivery route is:

1. Keep the current desktop fast and dependable: input, drawing, files, save/reopen, diagnostics and honest performance measurements.
2. Complete owned boot and recovery paths while preserving every currently working boot route.
3. Build reliable independent programs: ownership, isolation, spawn/wait, executable admission, threads, scheduling and memory pressure.
4. Add explicit authority, typed communication, supervision, credentials and sessions.
5. Deepen driver lifecycles, storage durability, packages and real hardware providers.
6. Move the desktop, input and audio behind owned services; complete shared text, design, accessibility and localization.
7. Finish the app runtime and migrate every current app and game with its behavior intact.
8. Deliver the full browser, office/creative/communication/admin apps, development tools and compatibility workloads.
9. Complete the language, agents, operations, wider architectures/hardware and a system rebuilt from inside zlOS. Host-language preparation can run earlier through its explicit handoff.

The phase chapters contain the individual feature requirements and named targets for every line above; the word “apps” or “drivers” is never the only specification.

## First implementation slice after this planning pass

The next bounded feature is **a program starting a child program, staying isolated from it, receiving its exit or fault result, and reclaiming it safely**. The existing process and sleep work is the foundation. Start from the [audited process contract](userspace-process-management-next.md), then follow the [32-step execution sequence](FULL-ROADMAP-NEXT-PROCESS.md).

The sequence covers exact ABI/error/output layouts; deriving the caller's parent identity; a clean supervisor address-space template; preparing a child without changing the active parent; eight-byte handles; reservation and one publication point; failed allocation/admission/copyout rollback; normal exit and faults; parent-only wait; orphan custody; stale generations; repeated reuse; disk-loaded parent/child fixtures; QEMU fault isolation; and exact evidence updates.

Its first release remains the existing **two-slot, bounded raw-image** profile until an extension is separately implemented and proved. It does not close the complete process subsystem, ELF/dynamic linking, SMP scheduling or physical qualification. The later work remains in MP-03 and its provider phases.

Before coding, refresh live GitHub/worktree ownership and decide the actual integration baseline. Resolve the relevant portions of `D-05`, `D-06` and `D-07`; preserve extensions as open. Existing valid behavior and tests should be reused. Completing the entire catalogue is not a prerequisite for delivering this first slice.

## Decisions we must settle rather than guess

The [decision register](FULL-ROADMAP-DECISIONS.md) names 25 outputs across scope, dependency handoffs, evidence, boot profiles, processes, concurrency, memory, authority, IPC, cryptography, DMA, storage durability, display, accessibility, networking, media, app ABI, browser standards, app workflows, compatibility, language, bootstrap, agents, releases and future hardware.

Each affected package must bind those answers to its exact implementation scope before code. Research-dependent answers must retain pinned primary specifications and dependency/license evidence. Existing locked choices remain locked unless a concrete conflict requires review. Questions about hardware not yet available become acquisition/specification/fixture/qualification tasks; they are not fabricated device support.

A literal prediction of every future source edit would be dishonest. The suite fully inventories the retained destination and expands its current requirements into executable obligations. It does not claim all future architecture choices have already been made. Refining a package after research is part of following the roadmap, provided no requirement disappears and changed dependencies, acceptance conditions and reasons are recorded.

## Preserve the deeper plans and their constraints

The original contract bodies are retained in the phase chapters. Their source documents also contain shared rules and context, which remain required reading:

- [Systems implementation contracts](../program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md).
- [Driver and application contracts](../program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md).
- [Visual, browser and app-experience contracts](../program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md).

The following additional plans remain design inputs. Their inventories/headings were checked for this roadmap; they have not all been re-audited line by line against current code. Their old “current” claims and implementation recommendations require source comparison before reuse. A historical “not worth doing” or unavailable device cannot silently delete a current canonical target.

| Retained input | Where it feeds this roadmap | Required reconciliation |
|---|---|---|
| [Performance architecture](../../kernel/docs/plans/performance-architecture-roadmap.md) | MP-01/03/05/06/07/16 | Retained client/chrome, regions, cache types, fallback renderer, measured ownership and overhead budgets. Re-measure before adopting old numbers. |
| [Retained window surfaces](../../kernel/docs/plans/retained-window-surfaces.md) | MP-07 | Surface lifetime, memory budget, rendering boundary, invalidation and correctness. Reuse landed work. |
| [GPU next](../../kernel/docs/plans/gpu-next.md) | MP-07/19 | Compare software/SMP/render-engine alternatives against current measured needs and safe hardware contracts. |
| [Driver build order](../../kernel/docs/plans/driver-build-order.md) | MP-02/05/06/08/09/19 | Recheck old bugs, provider inventory, read-only versus physical tests and resource prerequisites. |
| [Wireless plan](../../kernel/docs/plans/wireless-plan.md) | MP-08/19 | Verify actual radios, firmware/licenses and primary specifications; retain separate Wi-Fi/Bluetooth qualification. No hardware purchase is made by this plan. |
| [Design-system clone plan](../../kernel/docs/plans/ds-clone-plan.md) | MP-07/10/12 | Preserve measurable visual comparison, ownership and machine limits while honoring the selected PRESSWORK direction. |
| [Design orientation](README.md) | MP-07/10/12/14 | Preserve current selected design and landed implementation; older prototypes are historical alternatives. |
| [Retained language research](../program/research/language/MASTER_PLAN.md) | MP-16/19/20, D-21/D-25 | Compare all retained language plans with live ownership and locked syntax. Explicitly map any historical full-stack ambition outside the canonical catalogue; do not silently import an unrelated project. |

## What closes the whole thing

A bounded release can be useful long before `PROGRAM-CLOSE`. Its manifest must say exactly which workflows, source generation, hardware/firmware, architectures and standards are qualified. It must preserve install/update/rollback and recovery.

The entire programme closes only when every retained feature, original contract and named target satisfies its full acceptance conditions; every current app and game preserves its required workflow; the language/toolchain and rebuilt system execute their actual workloads; and the required security, accessibility, visual, performance, operational and physical evidence exists. Availability of source, successful compilation, a screenshot or a passing emulator is insufficient for claims outside that evidence.

The weakest part of this plan is the **remaining semantic dependency and design resolution**, especially where boot, authority, storage, processes and the self-hosted toolchain depend on one another. The explicit decisions and package-level dependency reviews keep that work visible. The structural coverage check does not waive it.

## Keeping the roadmap usable

Retain stable catalogue/package IDs. Add detailed children when research justifies them, preserving the parent requirement. Record new or changed interface dependencies in the machine-readable plan and corresponding chapter, then recheck coverage, source drift, links and cycles. Do not silently overwrite this authored planning proposal with a newer generated feature-status snapshot.

Use the phase/package relevant to the current product outcome; do not read or execute all checklist entries at once. A follow-up can say “continue N-PROCESS.11” or name a feature/target ID, and the exact source, prerequisites and acceptance conditions remain recoverable.
