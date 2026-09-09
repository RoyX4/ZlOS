# Roadmap decisions and research gates

Status: decision register. Initial process portions of D-02/D-05/D-06/D-07 now have a [bounded implementation decision](userspace-spawn-wait-abi.md); their wider system obligations and all other decisions remain open. The register itself does not authorize publication.

[Master roadmap](FULL-SYSTEM-ROADMAP.md)

Each decision has a question, concrete output and rejection condition. Consult current primary specifications and pinned dependency documentation where the answer depends on an external standard, implementation or hardware revision. Preserve source/version/licensing evidence. Record selected/rejected alternatives and downstream task changes; do not silently guess a missing policy.

## D-01 — Baseline and scope

**Applies to:** All.

**Resolve:** Which exact committed input and pending work belong to the next implementation generation?

**Required output:** A source/worktree custody ledger and explicit disposition of all 906 features, 611 targets, 174 contracts, 63 named implementations and the catalogue surface; Later/Profile labels retain scope.

**Acceptance:** No missing or duplicate IDs; no dirty work overwritten; no historical done claim adopted without matching evidence.

## D-02 — Dependency and handoff semantics

**Applies to:** All.

**Resolve:** Which bounded interfaces can be consumed before the owning phase is fully complete?

**Required output:** A named producer/consumer/ABI/error/ownership/proof contract per handoff, plus the distinction between development readiness and final phase promotion.

**Acceptance:** Resolve process/credentials, file-backed memory/VFS, host-language/target-toolchain and browser/app-platform bootstrap cycles explicitly; all executable edges must be acyclic.

## D-03 — Evidence and promotion

**Applies to:** 00,18,20.

**Resolve:** Which evidence is sufficient for each claim, and which implementation changes invalidate it?

**Required output:** Exact source/build/host/QEMU/physical/visual/security/a11y/performance rules, receipt schema and affected-check selection.

**Acceptance:** A stale artifact, missing required negative case, hardware skip or unsupported target cannot satisfy completion.

## D-04 — Boot generations and platform profiles

**Applies to:** 02,19.

**Resolve:** What are the supported media, firmware, architecture and recovery combinations?

**Required output:** A full matrix retaining every canonical target, bounded handover schema, selected initial rehearsal profile and explicit later qualification tasks.

**Acceptance:** Malformed/unknown-required input rejects; each selected/previous/recovery generation is identifiable; no profile selection silently shrinks the whole programme.

## D-05 — Process construction and orphan custody

**Applies to:** 03.

**Resolve:** How are a child image, full-width identity, scheduler admission and parent ownership published atomically?

**Required output:** The detailed next-process contract, syscall layouts/errors, orphan transition table and allocation/admission/copyout failure matrix.

**Acceptance:** Parent state survives every failed spawn; terminal results survive failed wait; high-bit handle generations remain valid; no child loses its owner.

## D-06 — Concurrency and CPU state

**Applies to:** 03,19.

**Resolve:** Which state belongs to a process, thread, CPU and interrupt frame, and how is ownership synchronized?

**Required output:** FP/vector/flags/TLS/stack state matrix, scheduler policy, atomics/lock ordering, TLB acknowledgement and CPU offline contracts.

**Acceptance:** Nested interrupts, preemption, migration, faults and offline paths preserve ownership or fail-stop with a bounded diagnostic.

## D-07 — Memory and lifetime

**Applies to:** 03,04,06.

**Resolve:** How do allocation, shared mappings, COW, file cache, quotas and revocation interact?

**Required output:** Typed ownership and reference rules, complete accounting classes, pressure order, failure-atomic mapping/cleanup and ABI semantics.

**Acceptance:** Every allocation/transfer/revoke failure has a deterministic expected pre/post-state and a leak oracle.

## D-08 — Authority, identity and consent

**Applies to:** 04.

**Resolve:** What rights, credentials and explicit consent does each operation require?

**Required output:** Handle/type/right matrix, derivation/revoke limits, credential transition matrix and operation-bound approval records.

**Acceptance:** Source and destination rights are independent; failed credential drop prevents launch; a stale or foreign token never grants authority.

## D-09 — IPC and service lifecycle

**Applies to:** 04,11.

**Resolve:** How are request versions, bytes, handles, budgets, deadlines, cancellation and peer death represented?

**Required output:** Versioned IDL, descriptor/readiness dependencies, request state machines, crash budgets and rollback/teardown order.

**Acceptance:** Unknown-required data, late replies, full queues and service loss have bounded typed outcomes and leave no leaked grants.

## D-10 — Cryptography, keys and trust

**Applies to:** 04,06,08,18.

**Resolve:** Which reviewed algorithms/libraries, keys and trust policies serve TLS, accounts, packages, storage and releases?

**Required output:** Version-locked primary specifications, library/license review, key custody/rotation/recovery, time policy and rejection vectors; no ad hoc cryptographic construction.

**Acceptance:** Separate integrity from publisher authentication; prove chain/name/usage/critical-extension policy; signing and deployment still require their own authorization.

## D-11 — Device and DMA isolation

**Applies to:** 05,19.

**Resolve:** Which hardware can actually enforce isolation, and what happens where it cannot?

**Required output:** Provider resource/lifecycle contracts, IOMMU and no-IOMMU profiles, firmware custody and removal/reset state machines.

**Acceptance:** No DMA-isolation claim on a non-isolated profile; every BAR/IRQ/DMA/endpoint reservation has a tested release path.

## D-12 — Storage durability and migration

**Applies to:** 06.

**Resolve:** What does committed mean for each block provider, filesystem and package transaction?

**Required output:** Flush/barrier model, on-disk versions, migration/repair/recovery policy, encrypted-volume ownership and old/new commit points.

**Acceptance:** Power loss, reordered/torn writes, disk full and media removal preserve a defined recoverable state.

## D-13 — Display safety and presentation

**Applies to:** 07,19.

**Resolve:** Which owner controls scanout, surfaces, fences, cache types, panel power and fallback?

**Required output:** Display/surface protocol and renderer oracle, frame/input budgets, Intel power/timing/recovery sheet and approved physical procedure.

**Acceptance:** A failed provider cannot remove the last usable display; physical timing rules come from pinned primary specifications and measured hardware.

## D-14 — Design, text and accessibility

**Applies to:** 07,10,11.

**Resolve:** Which current design rules and semantic interfaces must all apps share?

**Required output:** Selected PRESSWORK tokens/states, shaping/font/license policy, accessibility tree/actions, focus, IME/bidi/localization and reduced-motion/high-contrast behavior.

**Acceptance:** Keyboard and assistive workflows operate the real effects; screenshots alone do not satisfy accessibility.

## D-15 — Network and wireless profiles

**Applies to:** 08.

**Resolve:** Which physical providers, firmware, protocol versions and remote-exposure rules are required?

**Required output:** Per-device/provider matrix, DNS/TLS/socket policy, fault simulator and Wi-Fi/Bluetooth association/authentication/recovery contracts.

**Acceptance:** Virtual networking does not promote physical radios; unsupported hardware and unavailable firmware stay explicit.

## D-16 — Audio, camera and media

**Applies to:** 09,14.

**Resolve:** What are the stream, codec, format, timing, privacy and licensing contracts?

**Required output:** Negotiation/ring/clock schemas, capture grant lifetime, codec worker limits, formats, latency budgets and licensing decisions.

**Acceptance:** Revocation stops capture; malformed content cannot own device memory; hotplug and drift have tested recovery.

## D-17 — Application and package ABI

**Applies to:** 11,12.

**Resolve:** What can a native app depend on across launch, update, crash, restore and removal?

**Required output:** Versioned ABI/SDK, manifest admission, portal contracts, document state and package compatibility/migration policy.

**Acceptance:** A real app completes its durable workflow outside the kernel and survives denied grants and service restart.

## D-18 — Web platform and browser containment

**Applies to:** 13.

**Resolve:** Which web capabilities are implemented internally, which approved dependencies are used, and what is unsupported?

**Required output:** Per-capability standards/version/corpus map, process authority split, origin/storage/download/certificate policy and resource budgets.

**Acceptance:** Every advertised web behavior has a deterministic oracle and restricted-worker failure proof; dependency choices retain licenses and provenance.

## D-19 — Application formats and workflow depth

**Applies to:** 14.

**Resolve:** What does complete mean for each office, creative, media, communication and personal app?

**Required output:** Every target gets explicit create/import/edit/action/save/reopen/export, offline/conflict, recovery, accessibility and performance scenarios.

**Acceptance:** No target closes on a painted screen or a menu entry; verify durable/external effects with independent inspection.

## D-20 — Compatibility and external workloads

**Applies to:** 15.

**Resolve:** Which exact Linux, Windows, POSIX and VM workloads define the compatibility contracts?

**Required output:** Pinned programme/version/architecture/API matrices, port/build/dependency recipes and explicit unsupported results; all canonical compatibility features remain queued.

**Acceptance:** Successful compilation or one demo does not imply blanket compatibility; test the full selected workflows and containment.

## D-21 — Language semantics and active engine ownership

**Applies to:** 16.

**Resolve:** Which language choices are locked, which remain design decisions, and which backend is responsible for each target?

**Required output:** A live-source comparison with retained language plans, explicit types/ownership/errors/ABI rules, active/archived backend map and feature parity matrix.

**Acceptance:** Do not reopen syntax without a reason or revive archived implementations; all active engines agree on the declared semantics.

## D-22 — Bootstrap and self-hosting trust

**Applies to:** 16,20.

**Resolve:** How does the trusted host seed lead to a matching system rebuilt inside zlOS?

**Required output:** Seed, stage, compiler, linker, sysroot, source/package graph and artifact identities plus semantic and byte-level comparison rules.

**Acceptance:** A stored hash alone is insufficient; execute the rebuilt compiler and boot the resulting exact system.

## D-23 — Agent authority and model-independent execution

**Applies to:** 17.

**Resolve:** How can agents plan and automate without becoming a new privileged authority?

**Required output:** Typed deterministic broker, attenuated child grants, model/tool boundaries, secret handling, approval binding, budgets and independent completion oracle.

**Acceptance:** Hostile tool output, parent death, revoke-before-spawn and forged success cannot bypass authority or conceal unfinished effects.

## D-24 — Release, demo and support operations

**Applies to:** 18.

**Resolve:** Who may publish, sign, deploy, repair, retain support data and operate remote demos?

**Required output:** Exact-artifact release/runbook, lease/transport/egress model, recovery rehearsal, consent/redaction and incident/revocation procedure.

**Acceptance:** No orphan resources, unsigned promotion, unapproved external exposure or destructive target ambiguity; publish only after explicit approval.

## D-25 — Full programme scope and unavailable hardware

**Applies to:** 19,20.

**Resolve:** How are later devices, architectures and historical full-stack ambitions retained without claiming unavailable proof?

**Required output:** A target-by-target hardware/specification/fixture/acquisition plan and explicit mapping or scope-decision record for every legacy ambition.

**Acceptance:** No missing target is erased by choosing an initial profile; distinguish full programme completion from one qualified release.
