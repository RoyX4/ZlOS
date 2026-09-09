# MP-02: Own the complete boot, firmware and recovery path

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/boot/; kernel/src/core/boot/; kernel/tools/images/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-02` exports: Validated boot origin and typed handover with a bootable fallback; later services consume this bounded contract.

The handoff enables only its named subset. `CLOSE-02` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-02.01 — Freeze BIOS/raw, Multiboot and native-UEFI acquisition interfaces and the ownership transfer to the kernel

Freeze BIOS/raw, Multiboot and native-UEFI acquisition interfaces and the ownership transfer to the kernel.

**Requires:** `D-01`, `D-02`, `D-04`, `H-00`, `H-01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.02 — Validate memory maps, modules, framebuffer format, firmware tables, CPU features, entropy and origin before publication

Validate memory maps, modules, framebuffer format, firmware tables, CPU features, entropy and origin before publication.

**Requires:** `M-02.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-02 — Bounded development handoff: Own the complete boot, firmware and recovery path

Validated boot origin and typed handover with a bootable fallback; later services consume this bounded contract.

**Requires:** `M-02.02`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-02.03 — Implement bounded kernel/config/archive parsing with range, overlap, permission, relocation and required-field rejection

Implement bounded kernel/config/archive parsing with range, overlap, permission, relocation and required-field rejection.

**Requires:** `M-02.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.04 — Separate boot selection policy from hardware acquisition

Separate boot selection policy from hardware acquisition; identify current, previous, recovery, debug and assertion generations.

**Requires:** `M-02.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.05 — Define signed generation custody, trust rotation and anti-rollback policy before activating enforcement

Define signed generation custody, trust rotation and anti-rollback policy before activating enforcement.

**Requires:** `M-02.04`, `H-04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.06 — Make update selection atomic and recoverable after every interrupted write

Make update selection atomic and recoverable after every interrupted write; keep an independently bootable previous generation.

**Requires:** `M-02.05`, `H-06`, `H-18`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.07 — Build the raw/ISO/disk/partition/filesystem/fragmentation/hostile-media matrix with exact inside-guest artifact identity

Build the raw/ISO/disk/partition/filesystem/fragmentation/hostile-media matrix with exact inside-guest artifact identity.

**Requires:** `M-02.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.08 — Bring up and qualify each architecture through its own CPU, MMU, interrupt, timer and process contract

Bring up and qualify each architecture through its own CPU, MMU, interrupt, timer and process contract; never copy x86 proof labels.

**Requires:** `M-02.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-02.09 — Keep firmware polls bounded and early diagnostics usable when the normal console, storage or scheduler is unavailable

Keep firmware polls bounded and early diagnostics usable when the normal console, storage or scheduler is unavailable.

**Requires:** `M-02.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-BT-001](#f-bt-001) | feature | BIOS boot |
| [F-BT-002](#f-bt-002) | feature | UEFI boot |
| [F-BT-003](#f-bt-003) | feature | raw-sector boot |
| [F-BT-004](#f-bt-004) | feature | Multiboot boot |
| [F-BT-005](#f-bt-005) | feature | network boot |
| [F-BT-006](#f-bt-006) | feature | boot-media matrix |
| [F-BT-007](#f-bt-007) | feature | boot-origin identity |
| [F-BT-008](#f-bt-008) | feature | typed boot handover |
| [F-BT-009](#f-bt-009) | feature | kernel ELF loader |
| [F-BT-010](#f-bt-010) | feature | relocatable kernel/KASLR |
| [F-BT-011](#f-bt-011) | feature | initramfs/module archive |
| [F-BT-012](#f-bt-012) | feature | boot configuration |
| [F-BT-013](#f-bt-013) | feature | boot selector |
| [F-BT-014](#f-bt-014) | feature | boot splash/progress |
| [F-BT-015](#f-bt-015) | feature | early serial console |
| [F-BT-016](#f-bt-016) | feature | early framebuffer console |
| [F-BT-017](#f-bt-017) | feature | initialization DAG |
| [F-BT-018](#f-bt-018) | feature | safe/recovery boot |
| [F-BT-019](#f-bt-019) | feature | previous-generation rollback |
| [F-BT-020](#f-bt-020) | feature | firmware inventory |
| [F-BT-021](#f-bt-021) | feature | CPU feature discovery |
| [F-BT-022](#f-bt-022) | feature | x86-64 platform |
| [F-BT-023](#f-bt-023) | feature | i686 compatibility lane |
| [F-BT-024](#f-bt-024) | feature | AArch64 port |
| [F-BT-025](#f-bt-025) | feature | RISC-V 64 port |
| [F-BT-026](#f-bt-026) | feature | architecture abstraction |
| [F-BT-027](#f-bt-027) | feature | SMP bring-up/offline |
| [F-BT-028](#f-bt-028) | feature | monotonic clock |
| [F-BT-029](#f-bt-029) | feature | wall clock/RTC |
| [F-BT-030](#f-bt-030) | feature | firmware wait policy |
| [C-DA-03F](#c-da-03f) | contract | firmware and platform service |
| [C-DA-36](#c-da-36) | contract | boot-contract matrix |
| [T-PLAT-001](#t-plat-001) | target | BIOS/raw-sector acquisition |
| [T-PLAT-002](#t-plat-002) | target | GRUB/Multiboot acquisition |
| [T-PLAT-003](#t-plat-003) | target | native x86-64 UEFI/GOP acquisition |
| [T-PLAT-004](#t-plat-004) | target | PXE/TFTP/network acquisition |
| [T-PLAT-005](#t-plat-005) | target | ACPI table admission |
| [T-PLAT-007](#t-plat-007) | target | SMBIOS inventory |
| [T-PLAT-009](#t-plat-009) | target | x86 CPUID/MSR/PAT feature provider |
| [T-PLAT-028](#t-plat-028) | target | 16550 UART |
| [T-PLAT-034](#t-plat-034) | target | secure-boot key provider |
| [T-PLAT-035](#t-plat-035) | target | QEMU fw_cfg |
| [T-FSP-011](#t-fsp-011) | target | initramfs/USTAR/TAR |
| [T-SVC-022](#t-svc-022) | target | Firmware Inventory |

<a id="f-bt-001"></a>
## F-BT-001 — BIOS boot

**Original requirement:** bounded, identity-checked boot from supported raw/disk/optical layouts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no current physical BIOS artifact receipt; network and recovery boot layouts are absent.

### Execution steps

- [ ] **F-BT-001.01 — Reconcile existing BIOS boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for BIOS boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded, identity-checked boot from supported raw/disk/optical layouts
- [ ] **F-BT-001.02 — Freeze the exact contract for BIOS boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded, identity-checked boot from supported raw/disk/optical layouts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-001.03 — Implement/prove: bounded**
  - Action: For BIOS boot, implement or reuse and verify this exact obligation: bounded. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded; retain observable state/resource expectations.
- [ ] **F-BT-001.04 — Implement/prove: identity-checked boot from supported raw/disk/optical layouts**
  - Action: For BIOS boot, implement or reuse and verify this exact obligation: identity-checked boot from supported raw/disk/optical layouts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identity-checked boot from supported raw/disk/optical layouts; retain observable state/resource expectations.
- [ ] **F-BT-001.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to BIOS boot: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-001.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-001.06 — Integrate into the real consumer and runtime route**
  - Action: Wire BIOS boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-001.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-001.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for BIOS boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-001.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-002"></a>
## F-BT-002 — UEFI boot

**Original requirement:** disciplined memory-map exit, GOP, exact image identity, and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no current physical UEFI/GOP receipt; firmware fallback and hostile memory-map coverage are incomplete.

### Execution steps

- [ ] **F-BT-002.01 — Reconcile existing UEFI boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for UEFI boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: disciplined memory-map exit, GOP, exact image identity, and fallback
- [ ] **F-BT-002.02 — Freeze the exact contract for UEFI boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: disciplined memory-map exit, GOP, exact image identity, and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-002.03 — Implement/prove: disciplined memory-map exit**
  - Action: For UEFI boot, implement or reuse and verify this exact obligation: disciplined memory-map exit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for disciplined memory-map exit; retain observable state/resource expectations.
- [ ] **F-BT-002.04 — Implement/prove: GOP**
  - Action: For UEFI boot, implement or reuse and verify this exact obligation: GOP. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for GOP; retain observable state/resource expectations.
- [ ] **F-BT-002.05 — Implement/prove: exact image identity**
  - Action: For UEFI boot, implement or reuse and verify this exact obligation: exact image identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact image identity; retain observable state/resource expectations.
- [ ] **F-BT-002.06 — Implement/prove: fallback**
  - Action: For UEFI boot, implement or reuse and verify this exact obligation: fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-002.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback; retain observable state/resource expectations.
- [ ] **F-BT-002.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to UEFI boot: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-002.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-002.08 — Integrate into the real consumer and runtime route**
  - Action: Wire UEFI boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-002.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-002.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for UEFI boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-002.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-003"></a>
## F-BT-003 — raw-sector boot

**Original requirement:** versioned bounded layout with capacity/range validation and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no versioned recovery layout; no current physical raw-media receipt.

### Execution steps

- [ ] **F-BT-003.01 — Reconcile existing raw-sector boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for raw-sector boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned bounded layout with capacity/range validation and recovery
- [ ] **F-BT-003.02 — Freeze the exact contract for raw-sector boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned bounded layout with capacity/range validation and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-003.03 — Implement/prove: versioned bounded layout with capacity/range validation and recovery**
  - Action: For raw-sector boot, implement or reuse and verify this exact obligation: versioned bounded layout with capacity/range validation and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned bounded layout with capacity/range validation and recovery; retain observable state/resource expectations.
- [ ] **F-BT-003.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to raw-sector boot: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-003.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-003.05 — Integrate into the real consumer and runtime route**
  - Action: Wire raw-sector boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-003.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-003.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for raw-sector boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-003.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-004"></a>
## F-BT-004 — Multiboot boot

**Original requirement:** exact protocol version, modules, framebuffer, map, and malformed-handover tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** module-list admission is incomplete; no current physical Multiboot receipt.

### Execution steps

- [ ] **F-BT-004.01 — Reconcile existing Multiboot boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Multiboot boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact protocol version, modules, framebuffer, map, and malformed-handover tests
- [ ] **F-BT-004.02 — Freeze the exact contract for Multiboot boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact protocol version, modules, framebuffer, map, and malformed-handover tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-004.03 — Implement/prove: exact protocol version**
  - Action: For Multiboot boot, implement or reuse and verify this exact obligation: exact protocol version. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact protocol version; retain observable state/resource expectations.
- [ ] **F-BT-004.04 — Implement/prove: modules**
  - Action: For Multiboot boot, implement or reuse and verify this exact obligation: modules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modules; retain observable state/resource expectations.
- [ ] **F-BT-004.05 — Implement/prove: framebuffer**
  - Action: For Multiboot boot, implement or reuse and verify this exact obligation: framebuffer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for framebuffer; retain observable state/resource expectations.
- [ ] **F-BT-004.06 — Implement/prove: map**
  - Action: For Multiboot boot, implement or reuse and verify this exact obligation: map. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-004.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for map; retain observable state/resource expectations.
- [ ] **F-BT-004.07 — Implement/prove: malformed-handover tests**
  - Action: For Multiboot boot, implement or reuse and verify this exact obligation: malformed-handover tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-004.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for malformed-handover tests; retain observable state/resource expectations.
- [ ] **F-BT-004.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Multiboot boot: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-004.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-004.09 — Integrate into the real consumer and runtime route**
  - Action: Wire Multiboot boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-004.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-004.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Multiboot boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-004.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-005"></a>
## F-BT-005 — network boot

**Original requirement:** authenticated bounded payload with origin/digest and failure recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-005.01 — Reconcile existing network boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for network boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02, H-08.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated bounded payload with origin/digest and failure recovery
- [ ] **F-BT-005.02 — Freeze the exact contract for network boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated bounded payload with origin/digest and failure recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-005.03 — Implement/prove: authenticated bounded payload with origin/digest and failure recovery**
  - Action: For network boot, implement or reuse and verify this exact obligation: authenticated bounded payload with origin/digest and failure recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated bounded payload with origin/digest and failure recovery; retain observable state/resource expectations.
- [ ] **F-BT-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to network boot: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire network boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for network boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-006"></a>
## F-BT-006 — boot-media matrix

**Original requirement:** BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** network boot is absent; partition/filesystem/config combinations are incomplete; all nine exact artifacts lack current physical proof.

### Execution steps

- [ ] **F-BT-006.01 — Reconcile existing boot-media matrix**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for boot-media matrix. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads
- [ ] **F-BT-006.02 — Freeze the exact contract for boot-media matrix**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-006.03 — Implement/prove: BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads**
  - Action: For boot-media matrix, implement or reuse and verify this exact obligation: BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for BIOS/UEFI × raw/ISO/disk/partition/filesystem/config combinations have assertion payloads; retain observable state/resource expectations.
- [ ] **F-BT-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to boot-media matrix: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire boot-media matrix into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for boot-media matrix as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-007"></a>
## F-BT-007 — boot-origin identity

**Original requirement:** firmware/loader-selected device and partition become immutable kernel handles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-007.01 — Reconcile existing boot-origin identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for boot-origin identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: firmware/loader-selected device and partition become immutable kernel handles
- [ ] **F-BT-007.02 — Freeze the exact contract for boot-origin identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: firmware/loader-selected device and partition become immutable kernel handles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-007.03 — Implement/prove: firmware/loader-selected device and partition become immutable kernel handles**
  - Action: For boot-origin identity, implement or reuse and verify this exact obligation: firmware/loader-selected device and partition become immutable kernel handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for firmware/loader-selected device and partition become immutable kernel handles; retain observable state/resource expectations.
- [ ] **F-BT-007.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to boot-origin identity: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-007.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-007.05 — Integrate into the real consumer and runtime route**
  - Action: Wire boot-origin identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-007.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-007.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for boot-origin identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-007.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-008"></a>
## F-BT-008 — typed boot handover

**Original requirement:** versioned length-delimited ranges, modules, framebuffer, ACPI/FDT, entropy, and origin

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** shipping loaders do not populate exact origin or kernel identity; no entropy field; foreign memory-map formats are not normalized; no physical ThinkPad receipt for the current artifacts.

### Execution steps

- [ ] **F-BT-008.01 — Reconcile existing typed boot handover**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for typed boot handover. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned length-delimited ranges, modules, framebuffer, ACPI/FDT, entropy, and origin
- [ ] **F-BT-008.02 — Freeze the exact contract for typed boot handover**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned length-delimited ranges, modules, framebuffer, ACPI/FDT, entropy, and origin. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-008.03 — Implement/prove: versioned length-delimited ranges**
  - Action: For typed boot handover, implement or reuse and verify this exact obligation: versioned length-delimited ranges. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned length-delimited ranges; retain observable state/resource expectations.
- [ ] **F-BT-008.04 — Implement/prove: modules**
  - Action: For typed boot handover, implement or reuse and verify this exact obligation: modules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modules; retain observable state/resource expectations.
- [ ] **F-BT-008.05 — Implement/prove: framebuffer**
  - Action: For typed boot handover, implement or reuse and verify this exact obligation: framebuffer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for framebuffer; retain observable state/resource expectations.
- [ ] **F-BT-008.06 — Implement/prove: ACPI/FDT**
  - Action: For typed boot handover, implement or reuse and verify this exact obligation: ACPI/FDT. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ACPI/FDT; retain observable state/resource expectations.
- [ ] **F-BT-008.07 — Implement/prove: entropy**
  - Action: For typed boot handover, implement or reuse and verify this exact obligation: entropy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-008.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for entropy; retain observable state/resource expectations.
- [ ] **F-BT-008.08 — Implement/prove: origin**
  - Action: For typed boot handover, implement or reuse and verify this exact obligation: origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-008.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin; retain observable state/resource expectations.
- [ ] **F-BT-008.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to typed boot handover: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-008.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-008.10 — Integrate into the real consumer and runtime route**
  - Action: Wire typed boot handover into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-008.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-008.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for typed boot handover as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-008.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-009"></a>
## F-BT-009 — kernel ELF loader

**Original requirement:** length-first checked headers/segments/entry/permissions and failure-atomic mapping

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-009.01 — Reconcile existing kernel ELF loader**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kernel ELF loader. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: length-first checked headers/segments/entry/permissions and failure-atomic mapping
- [ ] **F-BT-009.02 — Freeze the exact contract for kernel ELF loader**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: length-first checked headers/segments/entry/permissions and failure-atomic mapping. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-009.03 — Implement/prove: length-first checked headers/segments/entry/permissions and failure-atomic mapping**
  - Action: For kernel ELF loader, implement or reuse and verify this exact obligation: length-first checked headers/segments/entry/permissions and failure-atomic mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for length-first checked headers/segments/entry/permissions and failure-atomic mapping; retain observable state/resource expectations.
- [ ] **F-BT-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kernel ELF loader: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire kernel ELF loader into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kernel ELF loader as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-010"></a>
## F-BT-010 — relocatable kernel/KASLR

**Original requirement:** supported relocation manifest, trustworthy entropy, collision checks, and exact slide receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-010.01 — Reconcile existing relocatable kernel/KASLR**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for relocatable kernel/KASLR. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: supported relocation manifest, trustworthy entropy, collision checks, and exact slide receipt
- [ ] **F-BT-010.02 — Freeze the exact contract for relocatable kernel/KASLR**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: supported relocation manifest, trustworthy entropy, collision checks, and exact slide receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-010.03 — Implement/prove: supported relocation manifest**
  - Action: For relocatable kernel/KASLR, implement or reuse and verify this exact obligation: supported relocation manifest. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for supported relocation manifest; retain observable state/resource expectations.
- [ ] **F-BT-010.04 — Implement/prove: trustworthy entropy**
  - Action: For relocatable kernel/KASLR, implement or reuse and verify this exact obligation: trustworthy entropy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for trustworthy entropy; retain observable state/resource expectations.
- [ ] **F-BT-010.05 — Implement/prove: collision checks**
  - Action: For relocatable kernel/KASLR, implement or reuse and verify this exact obligation: collision checks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for collision checks; retain observable state/resource expectations.
- [ ] **F-BT-010.06 — Implement/prove: exact slide receipt**
  - Action: For relocatable kernel/KASLR, implement or reuse and verify this exact obligation: exact slide receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-010.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact slide receipt; retain observable state/resource expectations.
- [ ] **F-BT-010.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to relocatable kernel/KASLR: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-010.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-010.08 — Integrate into the real consumer and runtime route**
  - Action: Wire relocatable kernel/KASLR into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-010.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-010.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for relocatable kernel/KASLR as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-010.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-011"></a>
## F-BT-011 — initramfs/module archive

**Original requirement:** signed length-known archive with checksum/path/link/bomb bounds

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-011.01 — Reconcile existing initramfs/module archive**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for initramfs/module archive. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: signed length-known archive with checksum/path/link/bomb bounds
- [ ] **F-BT-011.02 — Freeze the exact contract for initramfs/module archive**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: signed length-known archive with checksum/path/link/bomb bounds. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-011.03 — Implement/prove: signed length-known archive with checksum/path/link/bomb bounds**
  - Action: For initramfs/module archive, implement or reuse and verify this exact obligation: signed length-known archive with checksum/path/link/bomb bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signed length-known archive with checksum/path/link/bomb bounds; retain observable state/resource expectations.
- [ ] **F-BT-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to initramfs/module archive: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire initramfs/module archive into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for initramfs/module archive as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-012"></a>
## F-BT-012 — boot configuration

**Original requirement:** parsed schema, required/optional fields, safe defaults, diagnostics, and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-012.01 — Reconcile existing boot configuration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for boot configuration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: parsed schema, required/optional fields, safe defaults, diagnostics, and rollback
- [ ] **F-BT-012.02 — Freeze the exact contract for boot configuration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: parsed schema, required/optional fields, safe defaults, diagnostics, and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-012.03 — Implement/prove: parsed schema**
  - Action: For boot configuration, implement or reuse and verify this exact obligation: parsed schema. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for parsed schema; retain observable state/resource expectations.
- [ ] **F-BT-012.04 — Implement/prove: required/optional fields**
  - Action: For boot configuration, implement or reuse and verify this exact obligation: required/optional fields. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for required/optional fields; retain observable state/resource expectations.
- [ ] **F-BT-012.05 — Implement/prove: safe defaults**
  - Action: For boot configuration, implement or reuse and verify this exact obligation: safe defaults. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe defaults; retain observable state/resource expectations.
- [ ] **F-BT-012.06 — Implement/prove: diagnostics**
  - Action: For boot configuration, implement or reuse and verify this exact obligation: diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics; retain observable state/resource expectations.
- [ ] **F-BT-012.07 — Implement/prove: rollback**
  - Action: For boot configuration, implement or reuse and verify this exact obligation: rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rollback; retain observable state/resource expectations.
- [ ] **F-BT-012.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to boot configuration: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-012.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-012.09 — Integrate into the real consumer and runtime route**
  - Action: Wire boot configuration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-012.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-012.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for boot configuration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-012.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-013"></a>
## F-BT-013 — boot selector

**Original requirement:** keyboard-accessible exact-origin choices, timeout/policy, recovery and previous generation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-013.01 — Reconcile existing boot selector**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for boot selector. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: keyboard-accessible exact-origin choices, timeout/policy, recovery and previous generation
- [ ] **F-BT-013.02 — Freeze the exact contract for boot selector**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: keyboard-accessible exact-origin choices, timeout/policy, recovery and previous generation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-013.03 — Implement/prove: keyboard-accessible exact-origin choices**
  - Action: For boot selector, implement or reuse and verify this exact obligation: keyboard-accessible exact-origin choices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard-accessible exact-origin choices; retain observable state/resource expectations.
- [ ] **F-BT-013.04 — Implement/prove: timeout/policy**
  - Action: For boot selector, implement or reuse and verify this exact obligation: timeout/policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout/policy; retain observable state/resource expectations.
- [ ] **F-BT-013.05 — Implement/prove: recovery and previous generation**
  - Action: For boot selector, implement or reuse and verify this exact obligation: recovery and previous generation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery and previous generation; retain observable state/resource expectations.
- [ ] **F-BT-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to boot selector: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire boot selector into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for boot selector as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-014"></a>
## F-BT-014 — boot splash/progress

**Original requirement:** honest stage/state, accessible text alternative, no fixed fake telemetry

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-014.01 — Reconcile existing boot splash/progress**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for boot splash/progress. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: honest stage/state, accessible text alternative, no fixed fake telemetry
- [ ] **F-BT-014.02 — Freeze the exact contract for boot splash/progress**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: honest stage/state, accessible text alternative, no fixed fake telemetry. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-014.03 — Implement/prove: honest stage/state**
  - Action: For boot splash/progress, implement or reuse and verify this exact obligation: honest stage/state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for honest stage/state; retain observable state/resource expectations.
- [ ] **F-BT-014.04 — Implement/prove: accessible text alternative**
  - Action: For boot splash/progress, implement or reuse and verify this exact obligation: accessible text alternative. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accessible text alternative; retain observable state/resource expectations.
- [ ] **F-BT-014.05 — Implement/prove: no fixed fake telemetry**
  - Action: For boot splash/progress, implement or reuse and verify this exact obligation: no fixed fake telemetry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no fixed fake telemetry; retain observable state/resource expectations.
- [ ] **F-BT-014.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to boot splash/progress: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-014.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-014.07 — Integrate into the real consumer and runtime route**
  - Action: Wire boot splash/progress into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-014.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-014.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for boot splash/progress as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-014.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-015"></a>
## F-BT-015 — early serial console

**Original requirement:** bounded polling fallback with framing/errors and panic independence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-015.01 — Reconcile existing early serial console**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for early serial console. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded polling fallback with framing/errors and panic independence
- [ ] **F-BT-015.02 — Freeze the exact contract for early serial console**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded polling fallback with framing/errors and panic independence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-015.03 — Implement/prove: bounded polling fallback with framing/errors and panic independence**
  - Action: For early serial console, implement or reuse and verify this exact obligation: bounded polling fallback with framing/errors and panic independence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded polling fallback with framing/errors and panic independence; retain observable state/resource expectations.
- [ ] **F-BT-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to early serial console: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire early serial console into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for early serial console as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-016"></a>
## F-BT-016 — early framebuffer console

**Original requirement:** validated format/pitch/geometry, bounded drawing, and handoff to display service

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-016.01 — Reconcile existing early framebuffer console**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for early framebuffer console. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validated format/pitch/geometry, bounded drawing, and handoff to display service
- [ ] **F-BT-016.02 — Freeze the exact contract for early framebuffer console**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validated format/pitch/geometry, bounded drawing, and handoff to display service. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-016.03 — Implement/prove: validated format/pitch/geometry**
  - Action: For early framebuffer console, implement or reuse and verify this exact obligation: validated format/pitch/geometry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated format/pitch/geometry; retain observable state/resource expectations.
- [ ] **F-BT-016.04 — Implement/prove: bounded drawing**
  - Action: For early framebuffer console, implement or reuse and verify this exact obligation: bounded drawing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded drawing; retain observable state/resource expectations.
- [ ] **F-BT-016.05 — Implement/prove: handoff to display service**
  - Action: For early framebuffer console, implement or reuse and verify this exact obligation: handoff to display service. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handoff to display service; retain observable state/resource expectations.
- [ ] **F-BT-016.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to early framebuffer console: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-016.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-016.07 — Integrate into the real consumer and runtime route**
  - Action: Wire early framebuffer console into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-016.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-016.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for early framebuffer console as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-016.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-017"></a>
## F-BT-017 — initialization DAG

**Original requirement:** cycle detection, readiness, deadlines, criticality, rollback, reverse teardown, receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no general deadline engine; no reverse teardown execution; provider rollback remains stage-specific; no physical route receipt.

### Execution steps

- [ ] **F-BT-017.01 — Reconcile existing initialization DAG**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for initialization DAG. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: cycle detection, readiness, deadlines, criticality, rollback, reverse teardown, receipts
- [ ] **F-BT-017.02 — Freeze the exact contract for initialization DAG**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: cycle detection, readiness, deadlines, criticality, rollback, reverse teardown, receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-017.03 — Implement/prove: cycle detection**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: cycle detection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cycle detection; retain observable state/resource expectations.
- [ ] **F-BT-017.04 — Implement/prove: readiness**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: readiness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for readiness; retain observable state/resource expectations.
- [ ] **F-BT-017.05 — Implement/prove: deadlines**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadlines; retain observable state/resource expectations.
- [ ] **F-BT-017.06 — Implement/prove: criticality**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: criticality. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for criticality; retain observable state/resource expectations.
- [ ] **F-BT-017.07 — Implement/prove: rollback**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rollback; retain observable state/resource expectations.
- [ ] **F-BT-017.08 — Implement/prove: reverse teardown**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: reverse teardown. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reverse teardown; retain observable state/resource expectations.
- [ ] **F-BT-017.09 — Implement/prove: receipts**
  - Action: For initialization DAG, implement or reuse and verify this exact obligation: receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-017.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for receipts; retain observable state/resource expectations.
- [ ] **F-BT-017.10 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to initialization DAG: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-017.09.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-017.11 — Integrate into the real consumer and runtime route**
  - Action: Wire initialization DAG into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-017.10.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-017.12 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for initialization DAG as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-017.11.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-018"></a>
## F-BT-018 — safe/recovery boot

**Original requirement:** separately signed read-only-first environment with explicit target and logged repair

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-018.01 — Reconcile existing safe/recovery boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for safe/recovery boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02, H-04, H-06, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: separately signed read-only-first environment with explicit target and logged repair
- [ ] **F-BT-018.02 — Freeze the exact contract for safe/recovery boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: separately signed read-only-first environment with explicit target and logged repair. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-018.03 — Implement/prove: separately signed read-only-first environment with explicit target and logged repair**
  - Action: For safe/recovery boot, implement or reuse and verify this exact obligation: separately signed read-only-first environment with explicit target and logged repair. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for separately signed read-only-first environment with explicit target and logged repair; retain observable state/resource expectations.
- [ ] **F-BT-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to safe/recovery boot: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire safe/recovery boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for safe/recovery boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-019"></a>
## F-BT-019 — previous-generation rollback

**Original requirement:** bootable known-good system and atomic selection after failed update

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-019.01 — Reconcile existing previous-generation rollback**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for previous-generation rollback. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02, H-06, H-18.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bootable known-good system and atomic selection after failed update
- [ ] **F-BT-019.02 — Freeze the exact contract for previous-generation rollback**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bootable known-good system and atomic selection after failed update. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-019.03 — Implement/prove: bootable known-good system and atomic selection after failed update**
  - Action: For previous-generation rollback, implement or reuse and verify this exact obligation: bootable known-good system and atomic selection after failed update. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bootable known-good system and atomic selection after failed update; retain observable state/resource expectations.
- [ ] **F-BT-019.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to previous-generation rollback: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-019.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-019.05 — Integrate into the real consumer and runtime route**
  - Action: Wire previous-generation rollback into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-019.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-019.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for previous-generation rollback as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-019.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-020"></a>
## F-BT-020 — firmware inventory

**Original requirement:** checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-020.01 — Reconcile existing firmware inventory**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for firmware inventory. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior
- [ ] **F-BT-020.02 — Freeze the exact contract for firmware inventory**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-020.03 — Implement/prove: checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior**
  - Action: For firmware inventory, implement or reuse and verify this exact obligation: checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked ACPI/SMBIOS/UEFI/FDT records with unknown-version behavior; retain observable state/resource expectations.
- [ ] **F-BT-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to firmware inventory: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire firmware inventory into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for firmware inventory as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-021"></a>
## F-BT-021 — CPU feature discovery

**Original requirement:** immutable required/optional capability record before feature use

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-021.01 — Reconcile existing CPU feature discovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for CPU feature discovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: immutable required/optional capability record before feature use
- [ ] **F-BT-021.02 — Freeze the exact contract for CPU feature discovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: immutable required/optional capability record before feature use. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-021.03 — Implement/prove: immutable required/optional capability record before feature use**
  - Action: For CPU feature discovery, implement or reuse and verify this exact obligation: immutable required/optional capability record before feature use. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for immutable required/optional capability record before feature use; retain observable state/resource expectations.
- [ ] **F-BT-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to CPU feature discovery: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire CPU feature discovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for CPU feature discovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-022"></a>
## F-BT-022 — x86-64 platform

**Original requirement:** long mode, exception, MMU, timer, SMP, process, service, and device gates

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-022.01 — Reconcile existing x86-64 platform**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for x86-64 platform. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: long mode, exception, MMU, timer, SMP, process, service, and device gates
- [ ] **F-BT-022.02 — Freeze the exact contract for x86-64 platform**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: long mode, exception, MMU, timer, SMP, process, service, and device gates. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-022.03 — Implement/prove: long mode**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: long mode. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for long mode; retain observable state/resource expectations.
- [ ] **F-BT-022.04 — Implement/prove: exception**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: exception. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exception; retain observable state/resource expectations.
- [ ] **F-BT-022.05 — Implement/prove: MMU**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: MMU. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MMU; retain observable state/resource expectations.
- [ ] **F-BT-022.06 — Implement/prove: timer**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: timer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timer; retain observable state/resource expectations.
- [ ] **F-BT-022.07 — Implement/prove: SMP**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: SMP. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for SMP; retain observable state/resource expectations.
- [ ] **F-BT-022.08 — Implement/prove: process**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: process. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for process; retain observable state/resource expectations.
- [ ] **F-BT-022.09 — Implement/prove: service**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: service. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for service; retain observable state/resource expectations.
- [ ] **F-BT-022.10 — Implement/prove: device gates**
  - Action: For x86-64 platform, implement or reuse and verify this exact obligation: device gates. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-022.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device gates; retain observable state/resource expectations.
- [ ] **F-BT-022.11 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to x86-64 platform: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-022.10.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-022.12 — Integrate into the real consumer and runtime route**
  - Action: Wire x86-64 platform into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-022.11.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-022.13 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for x86-64 platform as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-022.12.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-023"></a>
## F-BT-023 — i686 compatibility lane

**Original requirement:** deliberately scoped regression artifact, never confused with 64-bit completion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-023.01 — Reconcile existing i686 compatibility lane**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for i686 compatibility lane. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deliberately scoped regression artifact, never confused with 64-bit completion
- [ ] **F-BT-023.02 — Freeze the exact contract for i686 compatibility lane**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deliberately scoped regression artifact, never confused with 64-bit completion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-023.03 — Implement/prove: deliberately scoped regression artifact**
  - Action: For i686 compatibility lane, implement or reuse and verify this exact obligation: deliberately scoped regression artifact. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deliberately scoped regression artifact; retain observable state/resource expectations.
- [ ] **F-BT-023.04 — Implement/prove: never confused with 64-bit completion**
  - Action: For i686 compatibility lane, implement or reuse and verify this exact obligation: never confused with 64-bit completion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for never confused with 64-bit completion; retain observable state/resource expectations.
- [ ] **F-BT-023.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to i686 compatibility lane: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-023.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-023.06 — Integrate into the real consumer and runtime route**
  - Action: Wire i686 compatibility lane into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-023.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-023.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for i686 compatibility lane as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-023.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-024"></a>
## F-BT-024 — AArch64 port

**Original requirement:** real EL/MMU/GIC/timer/SMP/process/service boot receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-024.01 — Reconcile existing AArch64 port**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for AArch64 port. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: real EL/MMU/GIC/timer/SMP/process/service boot receipts
- [ ] **F-BT-024.02 — Freeze the exact contract for AArch64 port**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: real EL/MMU/GIC/timer/SMP/process/service boot receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-024.03 — Implement/prove: real EL/MMU/GIC/timer/SMP/process/service boot receipts**
  - Action: For AArch64 port, implement or reuse and verify this exact obligation: real EL/MMU/GIC/timer/SMP/process/service boot receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for real EL/MMU/GIC/timer/SMP/process/service boot receipts; retain observable state/resource expectations.
- [ ] **F-BT-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to AArch64 port: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire AArch64 port into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for AArch64 port as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-025"></a>
## F-BT-025 — RISC-V 64 port

**Original requirement:** SBI/Sv39/PLIC/boot/process/service matrix, not compile-only scaffolding

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-025.01 — Reconcile existing RISC-V 64 port**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for RISC-V 64 port. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: SBI/Sv39/PLIC/boot/process/service matrix, not compile-only scaffolding
- [ ] **F-BT-025.02 — Freeze the exact contract for RISC-V 64 port**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: SBI/Sv39/PLIC/boot/process/service matrix, not compile-only scaffolding. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-025.03 — Implement/prove: SBI/Sv39/PLIC/boot/process/service matrix**
  - Action: For RISC-V 64 port, implement or reuse and verify this exact obligation: SBI/Sv39/PLIC/boot/process/service matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for SBI/Sv39/PLIC/boot/process/service matrix; retain observable state/resource expectations.
- [ ] **F-BT-025.04 — Implement/prove: not compile-only scaffolding**
  - Action: For RISC-V 64 port, implement or reuse and verify this exact obligation: not compile-only scaffolding. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not compile-only scaffolding; retain observable state/resource expectations.
- [ ] **F-BT-025.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to RISC-V 64 port: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-025.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-025.06 — Integrate into the real consumer and runtime route**
  - Action: Wire RISC-V 64 port into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-025.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-025.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for RISC-V 64 port as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-025.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-026"></a>
## F-BT-026 — architecture abstraction

**Original requirement:** common contracts plus per-architecture conformance and reachability inventories

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-026.01 — Reconcile existing architecture abstraction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for architecture abstraction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: common contracts plus per-architecture conformance and reachability inventories
- [ ] **F-BT-026.02 — Freeze the exact contract for architecture abstraction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: common contracts plus per-architecture conformance and reachability inventories. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-026.03 — Implement/prove: common contracts plus per-architecture conformance and reachability inventories**
  - Action: For architecture abstraction, implement or reuse and verify this exact obligation: common contracts plus per-architecture conformance and reachability inventories. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for common contracts plus per-architecture conformance and reachability inventories; retain observable state/resource expectations.
- [ ] **F-BT-026.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to architecture abstraction: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-026.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-026.05 — Integrate into the real consumer and runtime route**
  - Action: Wire architecture abstraction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-026.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-026.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for architecture abstraction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-026.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-027"></a>
## F-BT-027 — SMP bring-up/offline

**Original requirement:** exact CPU inventory, staged online, barriers, IPIs, TLB, failure and teardown

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-027.01 — Reconcile existing SMP bring-up/offline**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SMP bring-up/offline. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact CPU inventory, staged online, barriers, IPIs, TLB, failure and teardown
- [ ] **F-BT-027.02 — Freeze the exact contract for SMP bring-up/offline**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact CPU inventory, staged online, barriers, IPIs, TLB, failure and teardown. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-027.03 — Implement/prove: exact CPU inventory**
  - Action: For SMP bring-up/offline, implement or reuse and verify this exact obligation: exact CPU inventory. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact CPU inventory; retain observable state/resource expectations.
- [ ] **F-BT-027.04 — Implement/prove: staged online**
  - Action: For SMP bring-up/offline, implement or reuse and verify this exact obligation: staged online. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for staged online; retain observable state/resource expectations.
- [ ] **F-BT-027.05 — Implement/prove: barriers**
  - Action: For SMP bring-up/offline, implement or reuse and verify this exact obligation: barriers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for barriers; retain observable state/resource expectations.
- [ ] **F-BT-027.06 — Implement/prove: IPIs**
  - Action: For SMP bring-up/offline, implement or reuse and verify this exact obligation: IPIs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-027.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IPIs; retain observable state/resource expectations.
- [ ] **F-BT-027.07 — Implement/prove: TLB**
  - Action: For SMP bring-up/offline, implement or reuse and verify this exact obligation: TLB. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-027.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for TLB; retain observable state/resource expectations.
- [ ] **F-BT-027.08 — Implement/prove: failure and teardown**
  - Action: For SMP bring-up/offline, implement or reuse and verify this exact obligation: failure and teardown. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-027.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failure and teardown; retain observable state/resource expectations.
- [ ] **F-BT-027.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SMP bring-up/offline: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-027.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-027.10 — Integrate into the real consumer and runtime route**
  - Action: Wire SMP bring-up/offline into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-027.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-027.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SMP bring-up/offline as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-027.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-028"></a>
## F-BT-028 — monotonic clock

**Original requirement:** one deadline clock with calibration, wrap, drift, suspend and fallback semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-028.01 — Reconcile existing monotonic clock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for monotonic clock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: one deadline clock with calibration, wrap, drift, suspend and fallback semantics
- [ ] **F-BT-028.02 — Freeze the exact contract for monotonic clock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: one deadline clock with calibration, wrap, drift, suspend and fallback semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-028.03 — Implement/prove: one deadline clock with calibration**
  - Action: For monotonic clock, implement or reuse and verify this exact obligation: one deadline clock with calibration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for one deadline clock with calibration; retain observable state/resource expectations.
- [ ] **F-BT-028.04 — Implement/prove: wrap**
  - Action: For monotonic clock, implement or reuse and verify this exact obligation: wrap. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wrap; retain observable state/resource expectations.
- [ ] **F-BT-028.05 — Implement/prove: drift**
  - Action: For monotonic clock, implement or reuse and verify this exact obligation: drift. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drift; retain observable state/resource expectations.
- [ ] **F-BT-028.06 — Implement/prove: suspend and fallback semantics**
  - Action: For monotonic clock, implement or reuse and verify this exact obligation: suspend and fallback semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for suspend and fallback semantics; retain observable state/resource expectations.
- [ ] **F-BT-028.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to monotonic clock: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-028.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-028.08 — Integrate into the real consumer and runtime route**
  - Action: Wire monotonic clock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-028.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-028.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for monotonic clock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-028.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-029"></a>
## F-BT-029 — wall clock/RTC

**Original requirement:** validated hardware time separated from deadlines; timezone/locale in user service

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PARTIAL_CURRENT` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** hardware clock timezone policy is absent; no synchronization service; no drift/accuracy or suspend-resume contract; no current physical RTC receipt; locale presentation remains in-process.

### Execution steps

- [ ] **F-BT-029.01 — Reconcile existing wall clock/RTC**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for wall clock/RTC. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PARTIAL_CURRENT.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validated hardware time separated from deadlines; timezone/locale in user service
- [ ] **F-BT-029.02 — Freeze the exact contract for wall clock/RTC**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validated hardware time separated from deadlines; timezone/locale in user service. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-029.03 — Implement/prove: validated hardware time separated from deadlines**
  - Action: For wall clock/RTC, implement or reuse and verify this exact obligation: validated hardware time separated from deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated hardware time separated from deadlines; retain observable state/resource expectations.
- [ ] **F-BT-029.04 — Implement/prove: timezone/locale in user service**
  - Action: For wall clock/RTC, implement or reuse and verify this exact obligation: timezone/locale in user service. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timezone/locale in user service; retain observable state/resource expectations.
- [ ] **F-BT-029.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to wall clock/RTC: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-029.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-029.06 — Integrate into the real consumer and runtime route**
  - Action: Wire wall clock/RTC into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-029.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-029.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for wall clock/RTC as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-029.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-bt-030"></a>
## F-BT-030 — firmware wait policy

**Original requirement:** every poll has deadline, cancel/reset, final status, and deterministic timeout proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-BT-030.01 — Reconcile existing firmware wait policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for firmware wait policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-and-evidence comparison against the complete requirement: every poll has deadline, cancel/reset, final status, and deterministic timeout proof
- [ ] **F-BT-030.02 — Freeze the exact contract for firmware wait policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: every poll has deadline, cancel/reset, final status, and deterministic timeout proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-BT-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-BT-030.03 — Implement/prove: every poll has deadline**
  - Action: For firmware wait policy, implement or reuse and verify this exact obligation: every poll has deadline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for every poll has deadline; retain observable state/resource expectations.
- [ ] **F-BT-030.04 — Implement/prove: cancel/reset**
  - Action: For firmware wait policy, implement or reuse and verify this exact obligation: cancel/reset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancel/reset; retain observable state/resource expectations.
- [ ] **F-BT-030.05 — Implement/prove: final status**
  - Action: For firmware wait policy, implement or reuse and verify this exact obligation: final status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for final status; retain observable state/resource expectations.
- [ ] **F-BT-030.06 — Implement/prove: deterministic timeout proof**
  - Action: For firmware wait policy, implement or reuse and verify this exact obligation: deterministic timeout proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-BT-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic timeout proof; retain observable state/resource expectations.
- [ ] **F-BT-030.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to firmware wait policy: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-BT-030.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-BT-030.08 — Integrate into the real consumer and runtime route**
  - Action: Wire firmware wait policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-BT-030.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-BT-030.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for firmware wait policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-BT-030.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-da-03f"></a>
## C-DA-03F — firmware and platform service

**Original requirement:** firmware and platform service

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 107.

### Preserved original contract

**Depends on:** DA-03, early boot memory ownership and a bounded byte parser.

**Deliver:** architecture-neutral typed discovery from EFI, ACPI/uACPI, device
tree/FDT and SMBIOS. Record table origin, revision, checksum, byte length, firmware
hook availability, parsed resources, reserved memory, power methods and exact
unsupported/degraded states.

**Invariants:** every table/header/subtable/resource span is bounds- and checksum-
admitted before use; uACPI/AML OS hooks return truthful failure; duplicate/conflicting
resources do not publish; architecture-specific sources map into the same typed
model; parser or firmware-method failure cannot corrupt the live device graph.

**Proof:** truncated/oversized/looping tables, bad checksum, unknown revision,
missing hook, malformed AML/resource descriptor, duplicate device/range, absent
ACPI with valid DT, unsupported architecture, shutdown refusal and exact parity
receipts for every claimed architecture.

### Execution steps

- [ ] **C-DA-03F.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-03F.02 — Resolve this contract's exact dependencies**
  - Action: DA-03, early boot memory ownership and a bounded byte parser. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-03F.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-03F.03 — Deliver — firmware and platform service**
  - Action: architecture-neutral typed discovery from EFI, ACPI/uACPI, device tree/FDT and SMBIOS. Record table origin, revision, checksum, byte length, firmware hook availability, parsed resources, reserved memory, power methods and exact unsupported/degraded states.
  - Requires: C-DA-03F.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-03F.
- [ ] **C-DA-03F.04 — Invariants — firmware and platform service**
  - Action: every table/header/subtable/resource span is bounds- and checksum- admitted before use; uACPI/AML OS hooks return truthful failure; duplicate/conflicting resources do not publish; architecture-specific sources map into the same typed model; parser or firmware-method failure cannot corrupt the live device graph.
  - Requires: C-DA-03F.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-03F.
- [ ] **C-DA-03F.05 — Proof — firmware and platform service**
  - Action: truncated/oversized/looping tables, bad checksum, unknown revision, missing hook, malformed AML/resource descriptor, duplicate device/range, absent ACPI with valid DT, unsupported architecture, shutdown refusal and exact parity receipts for every claimed architecture.
  - Requires: C-DA-03F.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-03F.
- [ ] **C-DA-03F.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-03F. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-03F.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-36"></a>
## C-DA-36 — boot-contract matrix

**Original requirement:** boot-contract matrix

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 857.

### Preserved original contract

**Depends on:** DA-03 and a tiny assertion kernel.

**Deliver:** BIOS/UEFI and supported architecture/media/origin cross-products for
partition, filesystem, fragmentation, modules, command line and framebuffer.

**Proof:** correct origin and metadata plus malformed configuration/header/size/
offset/partition inputs. Report collected, skipped, passed and failed separately;
never average an unrun architecture into success.

### Execution steps

- [ ] **C-DA-36.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-36.02 — Resolve this contract's exact dependencies**
  - Action: DA-03 and a tiny assertion kernel. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-36.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-36.03 — Deliver — boot-contract matrix**
  - Action: BIOS/UEFI and supported architecture/media/origin cross-products for partition, filesystem, fragmentation, modules, command line and framebuffer.
  - Requires: C-DA-36.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-36.
- [ ] **C-DA-36.04 — Proof — boot-contract matrix**
  - Action: correct origin and metadata plus malformed configuration/header/size/ offset/partition inputs. Report collected, skipped, passed and failed separately; never average an unrun architecture into success.
  - Requires: C-DA-36.03.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-36.
- [ ] **C-DA-36.05 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-36. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-36.04.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-plat-001"></a>
## T-PLAT-001 — BIOS/raw-sector acquisition

**Original requirement:** bounded layout, exact origin, recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 30.

### Execution steps

- [ ] **T-PLAT-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve BIOS/raw-sector acquisition to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-001.02 — Specify the complete target boundary**
  - Action: BIOS/raw-sector acquisition must supply: bounded layout, exact origin, recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse BIOS/raw-sector acquisition through the shared platform contract, delivering every part of: bounded layout, exact origin, recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for BIOS/raw-sector acquisition.
- [ ] **T-PLAT-001.05 — Qualify and retain this target's own result**
  - Action: Bind BIOS/raw-sector acquisition to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-002"></a>
## T-PLAT-002 — GRUB/Multiboot acquisition

**Original requirement:** protocol/modules/map/framebuffer validation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 31.

### Execution steps

- [ ] **T-PLAT-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve GRUB/Multiboot acquisition to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-002.02 — Specify the complete target boundary**
  - Action: GRUB/Multiboot acquisition must supply: protocol/modules/map/framebuffer validation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse GRUB/Multiboot acquisition through the shared platform contract, delivering every part of: protocol/modules/map/framebuffer validation. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for GRUB/Multiboot acquisition.
- [ ] **T-PLAT-002.05 — Qualify and retain this target's own result**
  - Action: Bind GRUB/Multiboot acquisition to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-003"></a>
## T-PLAT-003 — native x86-64 UEFI/GOP acquisition

**Original requirement:** disciplined memory-map exit and image identity

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 32.

### Execution steps

- [ ] **T-PLAT-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve native x86-64 UEFI/GOP acquisition to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-003.02 — Specify the complete target boundary**
  - Action: native x86-64 UEFI/GOP acquisition must supply: disciplined memory-map exit and image identity. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse native x86-64 UEFI/GOP acquisition through the shared platform contract, delivering every part of: disciplined memory-map exit and image identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for native x86-64 UEFI/GOP acquisition.
- [ ] **T-PLAT-003.05 — Qualify and retain this target's own result**
  - Action: Bind native x86-64 UEFI/GOP acquisition to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-004"></a>
## T-PLAT-004 — PXE/TFTP/network acquisition

**Original requirement:** authenticated payload, bound size/time/origin

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 33.

### Execution steps

- [ ] **T-PLAT-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PXE/TFTP/network acquisition to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-004.02 — Specify the complete target boundary**
  - Action: PXE/TFTP/network acquisition must supply: authenticated payload, bound size/time/origin. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse PXE/TFTP/network acquisition through the shared platform contract, delivering every part of: authenticated payload, bound size/time/origin. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PXE/TFTP/network acquisition.
- [ ] **T-PLAT-004.05 — Qualify and retain this target's own result**
  - Action: Bind PXE/TFTP/network acquisition to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-005"></a>
## T-PLAT-005 — ACPI table admission

**Original requirement:** RSDP/XSDT/RSDT checks and immutable inventory

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 34.

### Execution steps

- [ ] **T-PLAT-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ACPI table admission to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-005.02 — Specify the complete target boundary**
  - Action: ACPI table admission must supply: RSDP/XSDT/RSDT checks and immutable inventory. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse ACPI table admission through the shared platform contract, delivering every part of: RSDP/XSDT/RSDT checks and immutable inventory. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ACPI table admission.
- [ ] **T-PLAT-005.05 — Qualify and retain this target's own result**
  - Action: Bind ACPI table admission to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-007"></a>
## T-PLAT-007 — SMBIOS inventory

**Original requirement:** checked strings/records/version fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 36.

### Execution steps

- [ ] **T-PLAT-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SMBIOS inventory to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-007.02 — Specify the complete target boundary**
  - Action: SMBIOS inventory must supply: checked strings/records/version fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse SMBIOS inventory through the shared platform contract, delivering every part of: checked strings/records/version fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SMBIOS inventory.
- [ ] **T-PLAT-007.05 — Qualify and retain this target's own result**
  - Action: Bind SMBIOS inventory to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-009"></a>
## T-PLAT-009 — x86 CPUID/MSR/PAT feature provider

**Original requirement:** required/optional features before first use

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 38.

### Execution steps

- [ ] **T-PLAT-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86 CPUID/MSR/PAT feature provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-009.02 — Specify the complete target boundary**
  - Action: x86 CPUID/MSR/PAT feature provider must supply: required/optional features before first use. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86 CPUID/MSR/PAT feature provider through the shared platform contract, delivering every part of: required/optional features before first use. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86 CPUID/MSR/PAT feature provider.
- [ ] **T-PLAT-009.05 — Qualify and retain this target's own result**
  - Action: Bind x86 CPUID/MSR/PAT feature provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-028"></a>
## T-PLAT-028 — 16550 UART

**Original requirement:** early panic-safe polling then IRQ mode

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 57.

### Execution steps

- [ ] **T-PLAT-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 16550 UART to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-028.02 — Specify the complete target boundary**
  - Action: 16550 UART must supply: early panic-safe polling then IRQ mode. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse 16550 UART through the shared platform contract, delivering every part of: early panic-safe polling then IRQ mode. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-028.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 16550 UART.
- [ ] **T-PLAT-028.05 — Qualify and retain this target's own result**
  - Action: Bind 16550 UART to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-034"></a>
## T-PLAT-034 — secure-boot key provider

**Original requirement:** trust roots, rotation, revocation and recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 63.

### Execution steps

- [ ] **T-PLAT-034.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve secure-boot key provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-034.02 — Specify the complete target boundary**
  - Action: secure-boot key provider must supply: trust roots, rotation, revocation and recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-034.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-034.03 — Implement the exact target behavior**
  - Action: Implement or reuse secure-boot key provider through the shared platform contract, delivering every part of: trust roots, rotation, revocation and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-034.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-034.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-034.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for secure-boot key provider.
- [ ] **T-PLAT-034.05 — Qualify and retain this target's own result**
  - Action: Bind secure-boot key provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-034.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-035"></a>
## T-PLAT-035 — QEMU fw_cfg

**Original requirement:** bounded items, exact machine identity and no trust promotion

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 64.

### Execution steps

- [ ] **T-PLAT-035.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve QEMU fw_cfg to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-035.02 — Specify the complete target boundary**
  - Action: QEMU fw_cfg must supply: bounded items, exact machine identity and no trust promotion. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-035.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-035.03 — Implement the exact target behavior**
  - Action: Implement or reuse QEMU fw_cfg through the shared platform contract, delivering every part of: bounded items, exact machine identity and no trust promotion. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-035.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-035.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-035.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for QEMU fw_cfg.
- [ ] **T-PLAT-035.05 — Qualify and retain this target's own result**
  - Action: Bind QEMU fw_cfg to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-035.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-011"></a>
## T-FSP-011 — initramfs/USTAR/TAR

**Original requirement:** length-first archive admission and path safety

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 287.

### Execution steps

- [ ] **T-FSP-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve initramfs/USTAR/TAR to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-011.02 — Specify the complete target boundary**
  - Action: initramfs/USTAR/TAR must supply: length-first archive admission and path safety. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse initramfs/USTAR/TAR through the shared platform contract, delivering every part of: length-first archive admission and path safety. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-011.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for initramfs/USTAR/TAR.
- [ ] **T-FSP-011.05 — Qualify and retain this target's own result**
  - Action: Bind initramfs/USTAR/TAR to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-022"></a>
## T-SVC-022 — Firmware Inventory

**Original requirement:** checked ACPI/SMBIOS/FDT/UEFI facts and provenance

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 37.

### Execution steps

- [ ] **T-SVC-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Firmware Inventory to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, H-02.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-022.02 — Specify the complete target boundary**
  - Action: Firmware Inventory must supply: checked ACPI/SMBIOS/FDT/UEFI facts and provenance. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse Firmware Inventory through the shared platform contract, delivering every part of: checked ACPI/SMBIOS/FDT/UEFI facts and provenance. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-022.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: source-right and destination-right denial independently; stale generation; total revoke limit; partial transfer; peer death; failed credential drop; timeout; replay and secret disclosure.
  - Requires: T-SVC-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Firmware Inventory.
- [ ] **T-SVC-022.05 — Qualify and retain this target's own result**
  - Action: Bind Firmware Inventory to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
