# MP-05: Make every device follow one lifecycle and resource contract

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/drivers/; kernel/src/arch/; kernel/tests/host/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-05` exports: Discovery, resource admission, lifecycle and common provider interfaces with honest DMA-isolation capabilities.

The handoff enables only its named subset. `CLOSE-05` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-05.01 — Normalize immutable device identity and discovery graphs for PCI/ACPI/FDT/USB/platform buses

Normalize immutable device identity and discovery graphs for PCI/ACPI/FDT/USB/platform buses.

**Requires:** `D-01`, `D-02`, `D-11`, `H-00`, `H-03`, `H-04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.02 — Define versioned provider descriptors, deterministic matching, required capabilities and unsupported-state reporting

Define versioned provider descriptors, deterministic matching, required capabilities and unsupported-state reporting.

**Requires:** `M-05.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.03 — Reserve BARs, IRQs, MSI/MSI-X vectors, DMA pages and IOMMU domains before driver start

Reserve BARs, IRQs, MSI/MSI-X vectors, DMA pages and IOMMU domains before driver start.

**Requires:** `M-05.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.04 — Implement admit/start/online/quiesce/recover/abort/offline transitions with one owner and bounded completion

Implement admit/start/online/quiesce/recover/abort/offline transitions with one owner and bounded completion.

**Requires:** `M-05.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.05 — Separate bus, controller, topology and class policy

Separate bus, controller, topology and class policy; preserve every current provider behind adapters while replacements mature.

**Requires:** `M-05.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-05 — Bounded development handoff: Make every device follow one lifecycle and resource contract

Discovery, resource admission, lifecycle and common provider interfaces with honest DMA-isolation capabilities.

**Requires:** `M-05.05`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-05.06 — Define hotplug, surprise removal, suspend, reset, rebind and generation changes before allowing resource reuse

Define hotplug, surprise removal, suspend, reset, rebind and generation changes before allowing resource reuse.

**Requires:** `M-05.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.07 — Implement default-deny DMA domains where available and explicitly label bounce/pinned no-IOMMU operation as non-isolated

Implement default-deny DMA domains where available and explicitly label bounce/pinned no-IOMMU operation as non-isolated.

**Requires:** `M-05.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.08 — Provide fake-device transcripts, simulator faults and independent resource accounting for every provider family

Provide fake-device transcripts, simulator faults and independent resource accounting for every provider family.

**Requires:** `M-05.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.09 — Admit modules and firmware only through named source, ABI, integrity, license and lifecycle policies

Admit modules and firmware only through named source, ABI, integrity, license and lifecycle policies.

**Requires:** `M-05.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-05.10 — Qualify target instances individually

Qualify target instances individually; a family interface or enumeration result never proves a working device.

**Requires:** `M-05.09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-DV-001](#f-dv-001) | feature | driver descriptor |
| [F-DV-002](#f-dv-002) | feature | driver matching |
| [F-DV-003](#f-dv-003) | feature | driver admission |
| [F-DV-004](#f-dv-004) | feature | resource plan/reservation |
| [F-DV-005](#f-dv-005) | feature | early abort/rollback |
| [F-DV-006](#f-dv-006) | feature | online/readiness |
| [F-DV-007](#f-dv-007) | feature | quiesce/stop |
| [F-DV-008](#f-dv-008) | feature | reset/recovery |
| [F-DV-009](#f-dv-009) | feature | hotplug/remove |
| [F-DV-010](#f-dv-010) | feature | driver supervisor |
| [F-DV-011](#f-dv-011) | feature | device manager UI |
| [F-DV-012](#f-dv-012) | feature | MMIO/port handle |
| [F-DV-013](#f-dv-013) | feature | IRQ handle |
| [F-DV-014](#f-dv-014) | feature | DMA object |
| [F-DV-015](#f-dv-015) | feature | IOMMU domain |
| [F-DV-016](#f-dv-016) | feature | no-IOMMU policy |
| [F-DV-017](#f-dv-017) | feature | PCI/PCIe enumeration |
| [F-DV-018](#f-dv-018) | feature | PCI BAR ownership |
| [F-DV-019](#f-dv-019) | feature | ACPI provider |
| [F-DV-020](#f-dv-020) | feature | device-tree provider |
| [F-DV-021](#f-dv-021) | feature | SMBIOS inventory |
| [F-DV-022](#f-dv-022) | feature | interrupt-controller provider |
| [F-DV-023](#f-dv-023) | feature | timer provider |
| [F-DV-024](#f-dv-024) | feature | serial/UART provider |
| [F-DV-025](#f-dv-025) | feature | PS/2 input provider |
| [F-DV-026](#f-dv-026) | feature | USB host core |
| [F-DV-027](#f-dv-027) | feature | xHCI provider |
| [F-DV-028](#f-dv-028) | feature | legacy USB controllers |
| [F-DV-029](#f-dv-029) | feature | USB HID keyboard |
| [F-DV-030](#f-dv-030) | feature | USB HID pointer |
| [F-DV-031](#f-dv-031) | feature | I2C-HID transport |
| [F-DV-032](#f-dv-032) | feature | touch/multitouch |
| [F-DV-033](#f-dv-033) | feature | stylus/tablet |
| [F-DV-034](#f-dv-034) | feature | game controller |
| [F-DV-035](#f-dv-035) | feature | NVMe storage provider |
| [F-DV-036](#f-dv-036) | feature | AHCI/SATA provider |
| [F-DV-037](#f-dv-037) | feature | ATA/IDE fallback |
| [F-DV-038](#f-dv-038) | feature | USB mass storage |
| [F-DV-039](#f-dv-039) | feature | virtio block |
| [F-DV-040](#f-dv-040) | feature | SD/eMMC |
| [F-DV-041](#f-dv-041) | feature | Ethernet provider |
| [F-DV-042](#f-dv-042) | feature | Wi-Fi provider |
| [F-DV-043](#f-dv-043) | feature | Bluetooth provider |
| [F-DV-044](#f-dv-044) | feature | framebuffer/GOP provider |
| [F-DV-045](#f-dv-045) | feature | Intel display provider |
| [F-DV-046](#f-dv-046) | feature | virtio-gpu provider |
| [F-DV-047](#f-dv-047) | feature | GPU command provider |
| [F-DV-048](#f-dv-048) | feature | HDA/audio provider |
| [F-DV-049](#f-dv-049) | feature | virtio-snd provider |
| [F-DV-050](#f-dv-050) | feature | USB Audio provider |
| [F-DV-051](#f-dv-051) | feature | camera provider |
| [F-DV-052](#f-dv-052) | feature | printer provider |
| [F-DV-053](#f-dv-053) | feature | scanner provider |
| [F-DV-054](#f-dv-054) | feature | battery/thermal/power provider |
| [F-DV-055](#f-dv-055) | feature | TPM/security device |
| [C-P8.1](#c-p8-1) | contract | declarative driver descriptor and lifecycle |
| [C-P8.2](#c-p8-2) | contract | USB core/class decomposition |
| [C-DA-04](#c-da-04) | contract | device identity and discovery graph |
| [C-DA-05](#c-da-05) | contract | driver descriptor and deterministic matching |
| [C-DA-06](#c-da-06) | contract | resource and DMA authority |
| [C-DA-07](#c-da-07) | contract | driver lifecycle supervisor |
| [C-DA-07M](#c-da-07m) | contract | kernel/driver module loader |
| [C-DA-08](#c-da-08) | contract | driver health and failure receipts |
| [C-DA-12](#c-da-12) | contract | USB controller/topology/class split |
| [C-DA-43](#c-da-43) | contract | unsupported and degraded-state gate |
| [T-PLAT-006](#t-plat-006) | target | AML/uACPI execution service |
| [T-PLAT-012](#t-plat-012) | target | x86 PIC |
| [T-PLAT-013](#t-plat-013) | target | x86 LAPIC/x2APIC |
| [T-PLAT-014](#t-plat-014) | target | x86 I/O APIC |
| [T-PLAT-015](#t-plat-015) | target | MSI/MSI-X allocator |
| [T-PLAT-021](#t-plat-021) | target | PIT |
| [T-PLAT-022](#t-plat-022) | target | HPET |
| [T-PLAT-024](#t-plat-024) | target | CMOS/RTC |
| [T-PLAT-032](#t-plat-032) | target | hardware RNG |
| [T-PLAT-036](#t-plat-036) | target | KVM paravirtual clock |
| [T-BUS-001](#t-bus-001) | target | PCI legacy configuration |
| [T-BUS-002](#t-bus-002) | target | PCIe ECAM/MCFG |
| [T-BUS-003](#t-bus-003) | target | PCI bridge/resource allocator |
| [T-BUS-004](#t-bus-004) | target | PCI hotplug/rebind |
| [T-BUS-006](#t-bus-006) | target | ISA/LPC platform bus |
| [T-BUS-007](#t-bus-007) | target | ACPI-enumerated platform bus |
| [T-BUS-009](#t-bus-009) | target | virtio PCI transport |
| [T-BUS-011](#t-bus-011) | target | USB device bus/core |
| [T-BUS-012](#t-bus-012) | target | USB hub/topology |
| [T-BUS-019](#t-bus-019) | target | DMA buffer allocator |
| [T-BUS-020](#t-bus-020) | target | Intel VT-d/DMAR IOMMU |
| [T-BUS-024](#t-bus-024) | target | no-IOMMU fallback |
| [T-USB-004](#t-usb-004) | target | xHCI host |
| [T-VM-002](#t-vm-002) | target | virtio RNG |
| [T-SVC-020](#t-svc-020) | target | Device Manager |
| [T-SVC-021](#t-svc-021) | target | Driver Supervisor |
| [T-SVC-023](#t-svc-023) | target | DMA/IOMMU Manager |
| [T-SVC-026](#t-svc-026) | target | Hotplug/Event Broker |

<a id="f-dv-001"></a>
## F-DV-001 — driver descriptor

**Original requirement:** schema/version, IDs, hardware revisions, ABI ranges, capabilities, resources, signature, origin

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-001.01 — Reconcile existing driver descriptor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for driver descriptor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: schema/version, IDs, hardware revisions, ABI ranges, capabilities, resources, signature, origin
- [ ] **F-DV-001.02 — Freeze the exact contract for driver descriptor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: schema/version, IDs, hardware revisions, ABI ranges, capabilities, resources, signature, origin. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-001.03 — Implement/prove: schema/version**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: schema/version. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for schema/version; retain observable state/resource expectations.
- [ ] **F-DV-001.04 — Implement/prove: IDs**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IDs; retain observable state/resource expectations.
- [ ] **F-DV-001.05 — Implement/prove: hardware revisions**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: hardware revisions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hardware revisions; retain observable state/resource expectations.
- [ ] **F-DV-001.06 — Implement/prove: ABI ranges**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: ABI ranges. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ABI ranges; retain observable state/resource expectations.
- [ ] **F-DV-001.07 — Implement/prove: capabilities**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: capabilities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capabilities; retain observable state/resource expectations.
- [ ] **F-DV-001.08 — Implement/prove: resources**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: resources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resources; retain observable state/resource expectations.
- [ ] **F-DV-001.09 — Implement/prove: signature**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: signature. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.08.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for signature; retain observable state/resource expectations.
- [ ] **F-DV-001.10 — Implement/prove: origin**
  - Action: For driver descriptor, implement or reuse and verify this exact obligation: origin. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-001.09.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for origin; retain observable state/resource expectations.
- [ ] **F-DV-001.11 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to driver descriptor: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-001.10.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-001.12 — Integrate into the real consumer and runtime route**
  - Action: Wire driver descriptor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-001.11.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-001.13 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for driver descriptor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-001.12.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-002"></a>
## F-DV-002 — driver matching

**Original requirement:** exact bus identifiers and required/optional capability negotiation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-002.01 — Reconcile existing driver matching**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for driver matching. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact bus identifiers and required/optional capability negotiation
- [ ] **F-DV-002.02 — Freeze the exact contract for driver matching**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact bus identifiers and required/optional capability negotiation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-002.03 — Implement/prove: exact bus identifiers and required/optional capability negotiation**
  - Action: For driver matching, implement or reuse and verify this exact obligation: exact bus identifiers and required/optional capability negotiation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact bus identifiers and required/optional capability negotiation; retain observable state/resource expectations.
- [ ] **F-DV-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to driver matching: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire driver matching into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for driver matching as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-003"></a>
## F-DV-003 — driver admission

**Original requirement:** authority, package trust, ABI, conflicts, resources, DMA width/coherency validated

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-003.01 — Reconcile existing driver admission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for driver admission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authority, package trust, ABI, conflicts, resources, DMA width/coherency validated
- [ ] **F-DV-003.02 — Freeze the exact contract for driver admission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authority, package trust, ABI, conflicts, resources, DMA width/coherency validated. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-003.03 — Implement/prove: authority**
  - Action: For driver admission, implement or reuse and verify this exact obligation: authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authority; retain observable state/resource expectations.
- [ ] **F-DV-003.04 — Implement/prove: package trust**
  - Action: For driver admission, implement or reuse and verify this exact obligation: package trust. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for package trust; retain observable state/resource expectations.
- [ ] **F-DV-003.05 — Implement/prove: ABI**
  - Action: For driver admission, implement or reuse and verify this exact obligation: ABI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ABI; retain observable state/resource expectations.
- [ ] **F-DV-003.06 — Implement/prove: conflicts**
  - Action: For driver admission, implement or reuse and verify this exact obligation: conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflicts; retain observable state/resource expectations.
- [ ] **F-DV-003.07 — Implement/prove: resources**
  - Action: For driver admission, implement or reuse and verify this exact obligation: resources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-003.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resources; retain observable state/resource expectations.
- [ ] **F-DV-003.08 — Implement/prove: DMA width/coherency validated**
  - Action: For driver admission, implement or reuse and verify this exact obligation: DMA width/coherency validated. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-003.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DMA width/coherency validated; retain observable state/resource expectations.
- [ ] **F-DV-003.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to driver admission: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-003.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-003.10 — Integrate into the real consumer and runtime route**
  - Action: Wire driver admission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-003.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-003.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for driver admission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-003.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-004"></a>
## F-DV-004 — resource plan/reservation

**Original requirement:** BAR, port, IRQ, DMA, clock, pin and dependency ownership reserved atomically

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-004.01 — Reconcile existing resource plan/reservation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for resource plan/reservation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: BAR, port, IRQ, DMA, clock, pin and dependency ownership reserved atomically
- [ ] **F-DV-004.02 — Freeze the exact contract for resource plan/reservation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: BAR, port, IRQ, DMA, clock, pin and dependency ownership reserved atomically. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-004.03 — Implement/prove: BAR**
  - Action: For resource plan/reservation, implement or reuse and verify this exact obligation: BAR. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for BAR; retain observable state/resource expectations.
- [ ] **F-DV-004.04 — Implement/prove: port**
  - Action: For resource plan/reservation, implement or reuse and verify this exact obligation: port. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for port; retain observable state/resource expectations.
- [ ] **F-DV-004.05 — Implement/prove: IRQ**
  - Action: For resource plan/reservation, implement or reuse and verify this exact obligation: IRQ. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IRQ; retain observable state/resource expectations.
- [ ] **F-DV-004.06 — Implement/prove: DMA**
  - Action: For resource plan/reservation, implement or reuse and verify this exact obligation: DMA. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-004.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DMA; retain observable state/resource expectations.
- [ ] **F-DV-004.07 — Implement/prove: clock**
  - Action: For resource plan/reservation, implement or reuse and verify this exact obligation: clock. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-004.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clock; retain observable state/resource expectations.
- [ ] **F-DV-004.08 — Implement/prove: pin and dependency ownership reserved atomically**
  - Action: For resource plan/reservation, implement or reuse and verify this exact obligation: pin and dependency ownership reserved atomically. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-004.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pin and dependency ownership reserved atomically; retain observable state/resource expectations.
- [ ] **F-DV-004.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to resource plan/reservation: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-004.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-004.10 — Integrate into the real consumer and runtime route**
  - Action: Wire resource plan/reservation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-004.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-004.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for resource plan/reservation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-004.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-005"></a>
## F-DV-005 — early abort/rollback

**Original requirement:** failures from match/admit/reserve/start release every object with receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-005.01 — Reconcile existing early abort/rollback**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for early abort/rollback. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: failures from match/admit/reserve/start release every object with receipts
- [ ] **F-DV-005.02 — Freeze the exact contract for early abort/rollback**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: failures from match/admit/reserve/start release every object with receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-005.03 — Implement/prove: failures from match/admit/reserve/start release every object with receipts**
  - Action: For early abort/rollback, implement or reuse and verify this exact obligation: failures from match/admit/reserve/start release every object with receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failures from match/admit/reserve/start release every object with receipts; retain observable state/resource expectations.
- [ ] **F-DV-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to early abort/rollback: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire early abort/rollback into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for early abort/rollback as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-006"></a>
## F-DV-006 — online/readiness

**Original requirement:** provider publishes only after health and protocol readiness

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-006.01 — Reconcile existing online/readiness**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for online/readiness. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider publishes only after health and protocol readiness
- [ ] **F-DV-006.02 — Freeze the exact contract for online/readiness**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider publishes only after health and protocol readiness. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-006.03 — Implement/prove: provider publishes only after health and protocol readiness**
  - Action: For online/readiness, implement or reuse and verify this exact obligation: provider publishes only after health and protocol readiness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider publishes only after health and protocol readiness; retain observable state/resource expectations.
- [ ] **F-DV-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to online/readiness: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire online/readiness into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for online/readiness as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-007"></a>
## F-DV-007 — quiesce/stop

**Original requirement:** new work refused, in-flight bounded, interrupts/DMA stopped before release

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-007.01 — Reconcile existing quiesce/stop**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for quiesce/stop. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: new work refused, in-flight bounded, interrupts/DMA stopped before release
- [ ] **F-DV-007.02 — Freeze the exact contract for quiesce/stop**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: new work refused, in-flight bounded, interrupts/DMA stopped before release. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-007.03 — Implement/prove: new work refused**
  - Action: For quiesce/stop, implement or reuse and verify this exact obligation: new work refused. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for new work refused; retain observable state/resource expectations.
- [ ] **F-DV-007.04 — Implement/prove: in-flight bounded**
  - Action: For quiesce/stop, implement or reuse and verify this exact obligation: in-flight bounded. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for in-flight bounded; retain observable state/resource expectations.
- [ ] **F-DV-007.05 — Implement/prove: interrupts/DMA stopped before release**
  - Action: For quiesce/stop, implement or reuse and verify this exact obligation: interrupts/DMA stopped before release. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for interrupts/DMA stopped before release; retain observable state/resource expectations.
- [ ] **F-DV-007.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to quiesce/stop: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-007.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-007.07 — Integrate into the real consumer and runtime route**
  - Action: Wire quiesce/stop into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-007.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-007.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for quiesce/stop as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-007.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-008"></a>
## F-DV-008 — reset/recovery

**Original requirement:** bounded staged reset, generation invalidation, state replay, quarantine

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-008.01 — Reconcile existing reset/recovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reset/recovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded staged reset, generation invalidation, state replay, quarantine
- [ ] **F-DV-008.02 — Freeze the exact contract for reset/recovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded staged reset, generation invalidation, state replay, quarantine. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-008.03 — Implement/prove: bounded staged reset**
  - Action: For reset/recovery, implement or reuse and verify this exact obligation: bounded staged reset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded staged reset; retain observable state/resource expectations.
- [ ] **F-DV-008.04 — Implement/prove: generation invalidation**
  - Action: For reset/recovery, implement or reuse and verify this exact obligation: generation invalidation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generation invalidation; retain observable state/resource expectations.
- [ ] **F-DV-008.05 — Implement/prove: state replay**
  - Action: For reset/recovery, implement or reuse and verify this exact obligation: state replay. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for state replay; retain observable state/resource expectations.
- [ ] **F-DV-008.06 — Implement/prove: quarantine**
  - Action: For reset/recovery, implement or reuse and verify this exact obligation: quarantine. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quarantine; retain observable state/resource expectations.
- [ ] **F-DV-008.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reset/recovery: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-008.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-008.08 — Integrate into the real consumer and runtime route**
  - Action: Wire reset/recovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-008.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-008.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reset/recovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-008.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-009"></a>
## F-DV-009 — hotplug/remove

**Original requirement:** dependency loss, user notice, safe unmount/eject, stale-handle failure

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-009.01 — Reconcile existing hotplug/remove**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hotplug/remove. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: dependency loss, user notice, safe unmount/eject, stale-handle failure
- [ ] **F-DV-009.02 — Freeze the exact contract for hotplug/remove**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: dependency loss, user notice, safe unmount/eject, stale-handle failure. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-009.03 — Implement/prove: dependency loss**
  - Action: For hotplug/remove, implement or reuse and verify this exact obligation: dependency loss. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependency loss; retain observable state/resource expectations.
- [ ] **F-DV-009.04 — Implement/prove: user notice**
  - Action: For hotplug/remove, implement or reuse and verify this exact obligation: user notice. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user notice; retain observable state/resource expectations.
- [ ] **F-DV-009.05 — Implement/prove: safe unmount/eject**
  - Action: For hotplug/remove, implement or reuse and verify this exact obligation: safe unmount/eject. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe unmount/eject; retain observable state/resource expectations.
- [ ] **F-DV-009.06 — Implement/prove: stale-handle failure**
  - Action: For hotplug/remove, implement or reuse and verify this exact obligation: stale-handle failure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stale-handle failure; retain observable state/resource expectations.
- [ ] **F-DV-009.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hotplug/remove: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-009.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-009.08 — Integrate into the real consumer and runtime route**
  - Action: Wire hotplug/remove into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-009.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-009.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hotplug/remove as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-009.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-010"></a>
## F-DV-010 — driver supervisor

**Original requirement:** crash budget/backoff, health, dependency graph, rebind and management authority

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-010.01 — Reconcile existing driver supervisor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for driver supervisor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: crash budget/backoff, health, dependency graph, rebind and management authority
- [ ] **F-DV-010.02 — Freeze the exact contract for driver supervisor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: crash budget/backoff, health, dependency graph, rebind and management authority. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-010.03 — Implement/prove: crash budget/backoff**
  - Action: For driver supervisor, implement or reuse and verify this exact obligation: crash budget/backoff. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for crash budget/backoff; retain observable state/resource expectations.
- [ ] **F-DV-010.04 — Implement/prove: health**
  - Action: For driver supervisor, implement or reuse and verify this exact obligation: health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for health; retain observable state/resource expectations.
- [ ] **F-DV-010.05 — Implement/prove: dependency graph**
  - Action: For driver supervisor, implement or reuse and verify this exact obligation: dependency graph. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependency graph; retain observable state/resource expectations.
- [ ] **F-DV-010.06 — Implement/prove: rebind and management authority**
  - Action: For driver supervisor, implement or reuse and verify this exact obligation: rebind and management authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-010.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rebind and management authority; retain observable state/resource expectations.
- [ ] **F-DV-010.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to driver supervisor: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-010.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-010.08 — Integrate into the real consumer and runtime route**
  - Action: Wire driver supervisor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-010.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-010.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for driver supervisor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-010.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-011"></a>
## F-DV-011 — device manager UI

**Original requirement:** truthful inventory, driver, resources, health, permissions, reset/eject and evidence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-011.01 — Reconcile existing device manager UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for device manager UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: truthful inventory, driver, resources, health, permissions, reset/eject and evidence
- [ ] **F-DV-011.02 — Freeze the exact contract for device manager UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: truthful inventory, driver, resources, health, permissions, reset/eject and evidence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-011.03 — Implement/prove: truthful inventory**
  - Action: For device manager UI, implement or reuse and verify this exact obligation: truthful inventory. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for truthful inventory; retain observable state/resource expectations.
- [ ] **F-DV-011.04 — Implement/prove: driver**
  - Action: For device manager UI, implement or reuse and verify this exact obligation: driver. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for driver; retain observable state/resource expectations.
- [ ] **F-DV-011.05 — Implement/prove: resources**
  - Action: For device manager UI, implement or reuse and verify this exact obligation: resources. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resources; retain observable state/resource expectations.
- [ ] **F-DV-011.06 — Implement/prove: health**
  - Action: For device manager UI, implement or reuse and verify this exact obligation: health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for health; retain observable state/resource expectations.
- [ ] **F-DV-011.07 — Implement/prove: permissions**
  - Action: For device manager UI, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-011.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-DV-011.08 — Implement/prove: reset/eject and evidence**
  - Action: For device manager UI, implement or reuse and verify this exact obligation: reset/eject and evidence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-011.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reset/eject and evidence; retain observable state/resource expectations.
- [ ] **F-DV-011.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to device manager UI: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-011.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-011.10 — Integrate into the real consumer and runtime route**
  - Action: Wire device manager UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-011.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-011.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for device manager UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-011.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-012"></a>
## F-DV-012 — MMIO/port handle

**Original requirement:** exact-width volatile access, bounds, barriers, ownership and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-012.01 — Reconcile existing MMIO/port handle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for MMIO/port handle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact-width volatile access, bounds, barriers, ownership and revocation
- [ ] **F-DV-012.02 — Freeze the exact contract for MMIO/port handle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact-width volatile access, bounds, barriers, ownership and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-012.03 — Implement/prove: exact-width volatile access**
  - Action: For MMIO/port handle, implement or reuse and verify this exact obligation: exact-width volatile access. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact-width volatile access; retain observable state/resource expectations.
- [ ] **F-DV-012.04 — Implement/prove: bounds**
  - Action: For MMIO/port handle, implement or reuse and verify this exact obligation: bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounds; retain observable state/resource expectations.
- [ ] **F-DV-012.05 — Implement/prove: barriers**
  - Action: For MMIO/port handle, implement or reuse and verify this exact obligation: barriers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for barriers; retain observable state/resource expectations.
- [ ] **F-DV-012.06 — Implement/prove: ownership and revocation**
  - Action: For MMIO/port handle, implement or reuse and verify this exact obligation: ownership and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ownership and revocation; retain observable state/resource expectations.
- [ ] **F-DV-012.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to MMIO/port handle: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-012.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-012.08 — Integrate into the real consumer and runtime route**
  - Action: Wire MMIO/port handle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-012.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-012.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for MMIO/port handle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-012.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-013"></a>
## F-DV-013 — IRQ handle

**Original requirement:** exclusive/shared policy, mask/ack ordering, generation, storm and teardown

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-013.01 — Reconcile existing IRQ handle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for IRQ handle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exclusive/shared policy, mask/ack ordering, generation, storm and teardown
- [ ] **F-DV-013.02 — Freeze the exact contract for IRQ handle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exclusive/shared policy, mask/ack ordering, generation, storm and teardown. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-013.03 — Implement/prove: exclusive/shared policy**
  - Action: For IRQ handle, implement or reuse and verify this exact obligation: exclusive/shared policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exclusive/shared policy; retain observable state/resource expectations.
- [ ] **F-DV-013.04 — Implement/prove: mask/ack ordering**
  - Action: For IRQ handle, implement or reuse and verify this exact obligation: mask/ack ordering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mask/ack ordering; retain observable state/resource expectations.
- [ ] **F-DV-013.05 — Implement/prove: generation**
  - Action: For IRQ handle, implement or reuse and verify this exact obligation: generation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generation; retain observable state/resource expectations.
- [ ] **F-DV-013.06 — Implement/prove: storm and teardown**
  - Action: For IRQ handle, implement or reuse and verify this exact obligation: storm and teardown. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-013.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for storm and teardown; retain observable state/resource expectations.
- [ ] **F-DV-013.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to IRQ handle: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-013.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-013.08 — Integrate into the real consumer and runtime route**
  - Action: Wire IRQ handle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-013.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-013.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for IRQ handle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-013.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-014"></a>
## F-DV-014 — DMA object

**Original requirement:** device-scoped pinned pages, direction, lifetime, sync, completion and revoke

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-014.01 — Reconcile existing DMA object**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for DMA object. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: device-scoped pinned pages, direction, lifetime, sync, completion and revoke
- [ ] **F-DV-014.02 — Freeze the exact contract for DMA object**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: device-scoped pinned pages, direction, lifetime, sync, completion and revoke. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-014.03 — Implement/prove: device-scoped pinned pages**
  - Action: For DMA object, implement or reuse and verify this exact obligation: device-scoped pinned pages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device-scoped pinned pages; retain observable state/resource expectations.
- [ ] **F-DV-014.04 — Implement/prove: direction**
  - Action: For DMA object, implement or reuse and verify this exact obligation: direction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for direction; retain observable state/resource expectations.
- [ ] **F-DV-014.05 — Implement/prove: lifetime**
  - Action: For DMA object, implement or reuse and verify this exact obligation: lifetime. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifetime; retain observable state/resource expectations.
- [ ] **F-DV-014.06 — Implement/prove: sync**
  - Action: For DMA object, implement or reuse and verify this exact obligation: sync. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sync; retain observable state/resource expectations.
- [ ] **F-DV-014.07 — Implement/prove: completion and revoke**
  - Action: For DMA object, implement or reuse and verify this exact obligation: completion and revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-014.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for completion and revoke; retain observable state/resource expectations.
- [ ] **F-DV-014.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to DMA object: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-014.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-014.09 — Integrate into the real consumer and runtime route**
  - Action: Wire DMA object into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-014.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-014.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for DMA object as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-014.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-015"></a>
## F-DV-015 — IOMMU domain

**Original requirement:** default-deny per device, scoped maps, out-of-domain fault proof, detach cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-015.01 — Reconcile existing IOMMU domain**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for IOMMU domain. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: default-deny per device, scoped maps, out-of-domain fault proof, detach cleanup
- [ ] **F-DV-015.02 — Freeze the exact contract for IOMMU domain**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: default-deny per device, scoped maps, out-of-domain fault proof, detach cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-015.03 — Implement/prove: default-deny per device**
  - Action: For IOMMU domain, implement or reuse and verify this exact obligation: default-deny per device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for default-deny per device; retain observable state/resource expectations.
- [ ] **F-DV-015.04 — Implement/prove: scoped maps**
  - Action: For IOMMU domain, implement or reuse and verify this exact obligation: scoped maps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scoped maps; retain observable state/resource expectations.
- [ ] **F-DV-015.05 — Implement/prove: out-of-domain fault proof**
  - Action: For IOMMU domain, implement or reuse and verify this exact obligation: out-of-domain fault proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for out-of-domain fault proof; retain observable state/resource expectations.
- [ ] **F-DV-015.06 — Implement/prove: detach cleanup**
  - Action: For IOMMU domain, implement or reuse and verify this exact obligation: detach cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-015.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for detach cleanup; retain observable state/resource expectations.
- [ ] **F-DV-015.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to IOMMU domain: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-015.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-015.08 — Integrate into the real consumer and runtime route**
  - Action: Wire IOMMU domain into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-015.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-015.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for IOMMU domain as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-015.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-016"></a>
## F-DV-016 — no-IOMMU policy

**Original requirement:** visibly not-isolated trusted/bounce/pinned fallback; never mislabeled isolated

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-016.01 — Reconcile existing no-IOMMU policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for no-IOMMU policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: visibly not-isolated trusted/bounce/pinned fallback; never mislabeled isolated
- [ ] **F-DV-016.02 — Freeze the exact contract for no-IOMMU policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: visibly not-isolated trusted/bounce/pinned fallback; never mislabeled isolated. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-016.03 — Implement/prove: visibly not-isolated trusted/bounce/pinned fallback**
  - Action: For no-IOMMU policy, implement or reuse and verify this exact obligation: visibly not-isolated trusted/bounce/pinned fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for visibly not-isolated trusted/bounce/pinned fallback; retain observable state/resource expectations.
- [ ] **F-DV-016.04 — Implement/prove: never mislabeled isolated**
  - Action: For no-IOMMU policy, implement or reuse and verify this exact obligation: never mislabeled isolated. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for never mislabeled isolated; retain observable state/resource expectations.
- [ ] **F-DV-016.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to no-IOMMU policy: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-016.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-016.06 — Integrate into the real consumer and runtime route**
  - Action: Wire no-IOMMU policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-016.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-016.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for no-IOMMU policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-016.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-017"></a>
## F-DV-017 — PCI/PCIe enumeration

**Original requirement:** typed BDF, multifunction/bridge recursion, checked config and unknown classes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-017.01 — Reconcile existing PCI/PCIe enumeration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for PCI/PCIe enumeration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed BDF, multifunction/bridge recursion, checked config and unknown classes
- [ ] **F-DV-017.02 — Freeze the exact contract for PCI/PCIe enumeration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed BDF, multifunction/bridge recursion, checked config and unknown classes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-017.03 — Implement/prove: typed BDF**
  - Action: For PCI/PCIe enumeration, implement or reuse and verify this exact obligation: typed BDF. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed BDF; retain observable state/resource expectations.
- [ ] **F-DV-017.04 — Implement/prove: multifunction/bridge recursion**
  - Action: For PCI/PCIe enumeration, implement or reuse and verify this exact obligation: multifunction/bridge recursion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multifunction/bridge recursion; retain observable state/resource expectations.
- [ ] **F-DV-017.05 — Implement/prove: checked config and unknown classes**
  - Action: For PCI/PCIe enumeration, implement or reuse and verify this exact obligation: checked config and unknown classes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked config and unknown classes; retain observable state/resource expectations.
- [ ] **F-DV-017.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to PCI/PCIe enumeration: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-017.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-017.07 — Integrate into the real consumer and runtime route**
  - Action: Wire PCI/PCIe enumeration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-017.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-017.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for PCI/PCIe enumeration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-017.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-018"></a>
## F-DV-018 — PCI BAR ownership

**Original requirement:** 64-bit sizing, type/prefetchability, collision, mapping, release and high-BAR tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-018.01 — Reconcile existing PCI BAR ownership**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for PCI BAR ownership. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: 64-bit sizing, type/prefetchability, collision, mapping, release and high-BAR tests
- [ ] **F-DV-018.02 — Freeze the exact contract for PCI BAR ownership**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: 64-bit sizing, type/prefetchability, collision, mapping, release and high-BAR tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-018.03 — Implement/prove: 64-bit sizing**
  - Action: For PCI BAR ownership, implement or reuse and verify this exact obligation: 64-bit sizing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for 64-bit sizing; retain observable state/resource expectations.
- [ ] **F-DV-018.04 — Implement/prove: type/prefetchability**
  - Action: For PCI BAR ownership, implement or reuse and verify this exact obligation: type/prefetchability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for type/prefetchability; retain observable state/resource expectations.
- [ ] **F-DV-018.05 — Implement/prove: collision**
  - Action: For PCI BAR ownership, implement or reuse and verify this exact obligation: collision. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for collision; retain observable state/resource expectations.
- [ ] **F-DV-018.06 — Implement/prove: mapping**
  - Action: For PCI BAR ownership, implement or reuse and verify this exact obligation: mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mapping; retain observable state/resource expectations.
- [ ] **F-DV-018.07 — Implement/prove: release and high-BAR tests**
  - Action: For PCI BAR ownership, implement or reuse and verify this exact obligation: release and high-BAR tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-018.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for release and high-BAR tests; retain observable state/resource expectations.
- [ ] **F-DV-018.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to PCI BAR ownership: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-018.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-018.09 — Integrate into the real consumer and runtime route**
  - Action: Wire PCI BAR ownership into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-018.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-018.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for PCI BAR ownership as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-018.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-019"></a>
## F-DV-019 — ACPI provider

**Original requirement:** checked tables/AML boundary, bounded EC/waits, lifecycle and provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-019.01 — Reconcile existing ACPI provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ACPI provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked tables/AML boundary, bounded EC/waits, lifecycle and provenance
- [ ] **F-DV-019.02 — Freeze the exact contract for ACPI provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked tables/AML boundary, bounded EC/waits, lifecycle and provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-019.03 — Implement/prove: checked tables/AML boundary**
  - Action: For ACPI provider, implement or reuse and verify this exact obligation: checked tables/AML boundary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked tables/AML boundary; retain observable state/resource expectations.
- [ ] **F-DV-019.04 — Implement/prove: bounded EC/waits**
  - Action: For ACPI provider, implement or reuse and verify this exact obligation: bounded EC/waits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded EC/waits; retain observable state/resource expectations.
- [ ] **F-DV-019.05 — Implement/prove: lifecycle and provenance**
  - Action: For ACPI provider, implement or reuse and verify this exact obligation: lifecycle and provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lifecycle and provenance; retain observable state/resource expectations.
- [ ] **F-DV-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ACPI provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire ACPI provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ACPI provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-020"></a>
## F-DV-020 — device-tree provider

**Original requirement:** checked FDT cells/ranges/compatible registry and malformed corpus

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-020.01 — Reconcile existing device-tree provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for device-tree provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked FDT cells/ranges/compatible registry and malformed corpus
- [ ] **F-DV-020.02 — Freeze the exact contract for device-tree provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked FDT cells/ranges/compatible registry and malformed corpus. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-020.03 — Implement/prove: checked FDT cells/ranges/compatible registry and malformed corpus**
  - Action: For device-tree provider, implement or reuse and verify this exact obligation: checked FDT cells/ranges/compatible registry and malformed corpus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked FDT cells/ranges/compatible registry and malformed corpus; retain observable state/resource expectations.
- [ ] **F-DV-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to device-tree provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire device-tree provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for device-tree provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-021"></a>
## F-DV-021 — SMBIOS inventory

**Original requirement:** checked versioned read-only facts; never used alone as security identity

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-021.01 — Reconcile existing SMBIOS inventory**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SMBIOS inventory. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked versioned read-only facts; never used alone as security identity
- [ ] **F-DV-021.02 — Freeze the exact contract for SMBIOS inventory**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked versioned read-only facts; never used alone as security identity. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-021.03 — Implement/prove: checked versioned read-only facts**
  - Action: For SMBIOS inventory, implement or reuse and verify this exact obligation: checked versioned read-only facts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked versioned read-only facts; retain observable state/resource expectations.
- [ ] **F-DV-021.04 — Implement/prove: never used alone as security identity**
  - Action: For SMBIOS inventory, implement or reuse and verify this exact obligation: never used alone as security identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for never used alone as security identity; retain observable state/resource expectations.
- [ ] **F-DV-021.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SMBIOS inventory: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-021.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-021.06 — Integrate into the real consumer and runtime route**
  - Action: Wire SMBIOS inventory into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-021.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-021.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SMBIOS inventory as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-021.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-022"></a>
## F-DV-022 — interrupt-controller provider

**Original requirement:** topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-022.01 — Reconcile existing interrupt-controller provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for interrupt-controller provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown
- [ ] **F-DV-022.02 — Freeze the exact contract for interrupt-controller provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-022.03 — Implement/prove: topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown**
  - Action: For interrupt-controller provider, implement or reuse and verify this exact obligation: topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for topology-derived PIC/APIC/IOAPIC/GIC/PLIC routing and teardown; retain observable state/resource expectations.
- [ ] **F-DV-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to interrupt-controller provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire interrupt-controller provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for interrupt-controller provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-023"></a>
## F-DV-023 — timer provider

**Original requirement:** PIT/HPET/APIC/TSC/architectural timer discovery, calibration and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-023.01 — Reconcile existing timer provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for timer provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: PIT/HPET/APIC/TSC/architectural timer discovery, calibration and fallback
- [ ] **F-DV-023.02 — Freeze the exact contract for timer provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: PIT/HPET/APIC/TSC/architectural timer discovery, calibration and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-023.03 — Implement/prove: PIT/HPET/APIC/TSC/architectural timer discovery**
  - Action: For timer provider, implement or reuse and verify this exact obligation: PIT/HPET/APIC/TSC/architectural timer discovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for PIT/HPET/APIC/TSC/architectural timer discovery; retain observable state/resource expectations.
- [ ] **F-DV-023.04 — Implement/prove: calibration and fallback**
  - Action: For timer provider, implement or reuse and verify this exact obligation: calibration and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for calibration and fallback; retain observable state/resource expectations.
- [ ] **F-DV-023.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to timer provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-023.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-023.06 — Integrate into the real consumer and runtime route**
  - Action: Wire timer provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-023.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-023.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for timer provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-023.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-024"></a>
## F-DV-024 — serial/UART provider

**Original requirement:** early polling plus bounded IRQ ring, errors and panic fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-024.01 — Reconcile existing serial/UART provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for serial/UART provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: early polling plus bounded IRQ ring, errors and panic fallback
- [ ] **F-DV-024.02 — Freeze the exact contract for serial/UART provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: early polling plus bounded IRQ ring, errors and panic fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-024.03 — Implement/prove: early polling plus bounded IRQ ring**
  - Action: For serial/UART provider, implement or reuse and verify this exact obligation: early polling plus bounded IRQ ring. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for early polling plus bounded IRQ ring; retain observable state/resource expectations.
- [ ] **F-DV-024.04 — Implement/prove: errors and panic fallback**
  - Action: For serial/UART provider, implement or reuse and verify this exact obligation: errors and panic fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors and panic fallback; retain observable state/resource expectations.
- [ ] **F-DV-024.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to serial/UART provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-024.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-024.06 — Integrate into the real consumer and runtime route**
  - Action: Wire serial/UART provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-024.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-024.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for serial/UART provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-024.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-025"></a>
## F-DV-025 — PS/2 input provider

**Original requirement:** bounded controller init, stateful decoder, overflow/loss/disconnect semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-025.01 — Reconcile existing PS/2 input provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for PS/2 input provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded controller init, stateful decoder, overflow/loss/disconnect semantics
- [ ] **F-DV-025.02 — Freeze the exact contract for PS/2 input provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded controller init, stateful decoder, overflow/loss/disconnect semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-025.03 — Implement/prove: bounded controller init**
  - Action: For PS/2 input provider, implement or reuse and verify this exact obligation: bounded controller init. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded controller init; retain observable state/resource expectations.
- [ ] **F-DV-025.04 — Implement/prove: stateful decoder**
  - Action: For PS/2 input provider, implement or reuse and verify this exact obligation: stateful decoder. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stateful decoder; retain observable state/resource expectations.
- [ ] **F-DV-025.05 — Implement/prove: overflow/loss/disconnect semantics**
  - Action: For PS/2 input provider, implement or reuse and verify this exact obligation: overflow/loss/disconnect semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow/loss/disconnect semantics; retain observable state/resource expectations.
- [ ] **F-DV-025.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to PS/2 input provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-025.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-025.07 — Integrate into the real consumer and runtime route**
  - Action: Wire PS/2 input provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-025.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-025.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for PS/2 input provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-025.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-026"></a>
## F-DV-026 — USB host core

**Original requirement:** controller-independent topology, descriptor parser, address/config/interface lifecycle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-026.01 — Reconcile existing USB host core**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for USB host core. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: controller-independent topology, descriptor parser, address/config/interface lifecycle
- [ ] **F-DV-026.02 — Freeze the exact contract for USB host core**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: controller-independent topology, descriptor parser, address/config/interface lifecycle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-026.03 — Implement/prove: controller-independent topology**
  - Action: For USB host core, implement or reuse and verify this exact obligation: controller-independent topology. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controller-independent topology; retain observable state/resource expectations.
- [ ] **F-DV-026.04 — Implement/prove: descriptor parser**
  - Action: For USB host core, implement or reuse and verify this exact obligation: descriptor parser. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for descriptor parser; retain observable state/resource expectations.
- [ ] **F-DV-026.05 — Implement/prove: address/config/interface lifecycle**
  - Action: For USB host core, implement or reuse and verify this exact obligation: address/config/interface lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for address/config/interface lifecycle; retain observable state/resource expectations.
- [ ] **F-DV-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to USB host core: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire USB host core into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for USB host core as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-027"></a>
## F-DV-027 — xHCI provider

**Original requirement:** rings/slots/endpoints/hubs, timeout/reset/hotplug/repeated reuse

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-027.01 — Reconcile existing xHCI provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for xHCI provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: rings/slots/endpoints/hubs, timeout/reset/hotplug/repeated reuse
- [ ] **F-DV-027.02 — Freeze the exact contract for xHCI provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: rings/slots/endpoints/hubs, timeout/reset/hotplug/repeated reuse. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-027.03 — Implement/prove: rings/slots/endpoints/hubs**
  - Action: For xHCI provider, implement or reuse and verify this exact obligation: rings/slots/endpoints/hubs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rings/slots/endpoints/hubs; retain observable state/resource expectations.
- [ ] **F-DV-027.04 — Implement/prove: timeout/reset/hotplug/repeated reuse**
  - Action: For xHCI provider, implement or reuse and verify this exact obligation: timeout/reset/hotplug/repeated reuse. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout/reset/hotplug/repeated reuse; retain observable state/resource expectations.
- [ ] **F-DV-027.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to xHCI provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-027.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-027.06 — Integrate into the real consumer and runtime route**
  - Action: Wire xHCI provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-027.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-027.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for xHCI provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-027.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-028"></a>
## F-DV-028 — legacy USB controllers

**Original requirement:** UHCI/OHCI/EHCI added only for target hardware under same core contract

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-028.01 — Reconcile existing legacy USB controllers**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for legacy USB controllers. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: UHCI/OHCI/EHCI added only for target hardware under same core contract
- [ ] **F-DV-028.02 — Freeze the exact contract for legacy USB controllers**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: UHCI/OHCI/EHCI added only for target hardware under same core contract. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-028.03 — Implement/prove: UHCI/OHCI/EHCI added only for target hardware under same core contract**
  - Action: For legacy USB controllers, implement or reuse and verify this exact obligation: UHCI/OHCI/EHCI added only for target hardware under same core contract. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for UHCI/OHCI/EHCI added only for target hardware under same core contract; retain observable state/resource expectations.
- [ ] **F-DV-028.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to legacy USB controllers: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-028.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-028.05 — Integrate into the real consumer and runtime route**
  - Action: Wire legacy USB controllers into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-028.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-028.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for legacy USB controllers as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-028.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-029"></a>
## F-DV-029 — USB HID keyboard

**Original requirement:** report descriptors, layouts, LEDs, repeat, multiple devices, disconnect and physical receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-029.01 — Reconcile existing USB HID keyboard**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for USB HID keyboard. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: report descriptors, layouts, LEDs, repeat, multiple devices, disconnect and physical receipt
- [ ] **F-DV-029.02 — Freeze the exact contract for USB HID keyboard**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: report descriptors, layouts, LEDs, repeat, multiple devices, disconnect and physical receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-029.03 — Implement/prove: report descriptors**
  - Action: For USB HID keyboard, implement or reuse and verify this exact obligation: report descriptors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for report descriptors; retain observable state/resource expectations.
- [ ] **F-DV-029.04 — Implement/prove: layouts**
  - Action: For USB HID keyboard, implement or reuse and verify this exact obligation: layouts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layouts; retain observable state/resource expectations.
- [ ] **F-DV-029.05 — Implement/prove: LEDs**
  - Action: For USB HID keyboard, implement or reuse and verify this exact obligation: LEDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for LEDs; retain observable state/resource expectations.
- [ ] **F-DV-029.06 — Implement/prove: repeat**
  - Action: For USB HID keyboard, implement or reuse and verify this exact obligation: repeat. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-029.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repeat; retain observable state/resource expectations.
- [ ] **F-DV-029.07 — Implement/prove: multiple devices**
  - Action: For USB HID keyboard, implement or reuse and verify this exact obligation: multiple devices. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-029.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multiple devices; retain observable state/resource expectations.
- [ ] **F-DV-029.08 — Implement/prove: disconnect and physical receipt**
  - Action: For USB HID keyboard, implement or reuse and verify this exact obligation: disconnect and physical receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-029.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for disconnect and physical receipt; retain observable state/resource expectations.
- [ ] **F-DV-029.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to USB HID keyboard: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-029.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-029.10 — Integrate into the real consumer and runtime route**
  - Action: Wire USB HID keyboard into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-029.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-029.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for USB HID keyboard as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-029.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-030"></a>
## F-DV-030 — USB HID pointer

**Original requirement:** relative/absolute axes, buttons, wheel, high-resolution input and physical receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-030.01 — Reconcile existing USB HID pointer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for USB HID pointer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: relative/absolute axes, buttons, wheel, high-resolution input and physical receipt
- [ ] **F-DV-030.02 — Freeze the exact contract for USB HID pointer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: relative/absolute axes, buttons, wheel, high-resolution input and physical receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-030.03 — Implement/prove: relative/absolute axes**
  - Action: For USB HID pointer, implement or reuse and verify this exact obligation: relative/absolute axes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for relative/absolute axes; retain observable state/resource expectations.
- [ ] **F-DV-030.04 — Implement/prove: buttons**
  - Action: For USB HID pointer, implement or reuse and verify this exact obligation: buttons. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for buttons; retain observable state/resource expectations.
- [ ] **F-DV-030.05 — Implement/prove: wheel**
  - Action: For USB HID pointer, implement or reuse and verify this exact obligation: wheel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wheel; retain observable state/resource expectations.
- [ ] **F-DV-030.06 — Implement/prove: high-resolution input and physical receipt**
  - Action: For USB HID pointer, implement or reuse and verify this exact obligation: high-resolution input and physical receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for high-resolution input and physical receipt; retain observable state/resource expectations.
- [ ] **F-DV-030.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to USB HID pointer: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-030.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-030.08 — Integrate into the real consumer and runtime route**
  - Action: Wire USB HID pointer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-030.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-030.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for USB HID pointer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-030.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-031"></a>
## F-DV-031 — I2C-HID transport

**Original requirement:** descriptor/report transport plus real x/y/button/touch decoder and session route

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-031.01 — Reconcile existing I2C-HID transport**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for I2C-HID transport. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: descriptor/report transport plus real x/y/button/touch decoder and session route
- [ ] **F-DV-031.02 — Freeze the exact contract for I2C-HID transport**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: descriptor/report transport plus real x/y/button/touch decoder and session route. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-031.03 — Implement/prove: descriptor/report transport plus real x/y/button/touch decoder and session route**
  - Action: For I2C-HID transport, implement or reuse and verify this exact obligation: descriptor/report transport plus real x/y/button/touch decoder and session route. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for descriptor/report transport plus real x/y/button/touch decoder and session route; retain observable state/resource expectations.
- [ ] **F-DV-031.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to I2C-HID transport: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-031.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-031.05 — Integrate into the real consumer and runtime route**
  - Action: Wire I2C-HID transport into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-031.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-031.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for I2C-HID transport as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-031.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-032"></a>
## F-DV-032 — touch/multitouch

**Original requirement:** contacts, slots, gestures, palm policy, calibration, orientation and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-032.01 — Reconcile existing touch/multitouch**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for touch/multitouch. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: contacts, slots, gestures, palm policy, calibration, orientation and a11y
- [ ] **F-DV-032.02 — Freeze the exact contract for touch/multitouch**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: contacts, slots, gestures, palm policy, calibration, orientation and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-032.03 — Implement/prove: contacts**
  - Action: For touch/multitouch, implement or reuse and verify this exact obligation: contacts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for contacts; retain observable state/resource expectations.
- [ ] **F-DV-032.04 — Implement/prove: slots**
  - Action: For touch/multitouch, implement or reuse and verify this exact obligation: slots. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-032.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for slots; retain observable state/resource expectations.
- [ ] **F-DV-032.05 — Implement/prove: gestures**
  - Action: For touch/multitouch, implement or reuse and verify this exact obligation: gestures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-032.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for gestures; retain observable state/resource expectations.
- [ ] **F-DV-032.06 — Implement/prove: palm policy**
  - Action: For touch/multitouch, implement or reuse and verify this exact obligation: palm policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-032.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for palm policy; retain observable state/resource expectations.
- [ ] **F-DV-032.07 — Implement/prove: calibration**
  - Action: For touch/multitouch, implement or reuse and verify this exact obligation: calibration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-032.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for calibration; retain observable state/resource expectations.
- [ ] **F-DV-032.08 — Implement/prove: orientation and a11y**
  - Action: For touch/multitouch, implement or reuse and verify this exact obligation: orientation and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-032.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for orientation and a11y; retain observable state/resource expectations.
- [ ] **F-DV-032.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to touch/multitouch: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-032.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-032.10 — Integrate into the real consumer and runtime route**
  - Action: Wire touch/multitouch into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-032.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-032.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for touch/multitouch as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-032.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-033"></a>
## F-DV-033 — stylus/tablet

**Original requirement:** pressure, tilt, buttons, eraser, proximity, mapping and app permissions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-033.01 — Reconcile existing stylus/tablet**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for stylus/tablet. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: pressure, tilt, buttons, eraser, proximity, mapping and app permissions
- [ ] **F-DV-033.02 — Freeze the exact contract for stylus/tablet**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: pressure, tilt, buttons, eraser, proximity, mapping and app permissions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-033.03 — Implement/prove: pressure**
  - Action: For stylus/tablet, implement or reuse and verify this exact obligation: pressure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pressure; retain observable state/resource expectations.
- [ ] **F-DV-033.04 — Implement/prove: tilt**
  - Action: For stylus/tablet, implement or reuse and verify this exact obligation: tilt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tilt; retain observable state/resource expectations.
- [ ] **F-DV-033.05 — Implement/prove: buttons**
  - Action: For stylus/tablet, implement or reuse and verify this exact obligation: buttons. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for buttons; retain observable state/resource expectations.
- [ ] **F-DV-033.06 — Implement/prove: eraser**
  - Action: For stylus/tablet, implement or reuse and verify this exact obligation: eraser. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for eraser; retain observable state/resource expectations.
- [ ] **F-DV-033.07 — Implement/prove: proximity**
  - Action: For stylus/tablet, implement or reuse and verify this exact obligation: proximity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-033.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for proximity; retain observable state/resource expectations.
- [ ] **F-DV-033.08 — Implement/prove: mapping and app permissions**
  - Action: For stylus/tablet, implement or reuse and verify this exact obligation: mapping and app permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-033.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mapping and app permissions; retain observable state/resource expectations.
- [ ] **F-DV-033.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to stylus/tablet: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-033.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-033.10 — Integrate into the real consumer and runtime route**
  - Action: Wire stylus/tablet into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-033.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-033.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for stylus/tablet as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-033.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-034"></a>
## F-DV-034 — game controller

**Original requirement:** standardized axes/buttons/haptics, hotplug, calibration, per-app mapping

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-034.01 — Reconcile existing game controller**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for game controller. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: standardized axes/buttons/haptics, hotplug, calibration, per-app mapping
- [ ] **F-DV-034.02 — Freeze the exact contract for game controller**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: standardized axes/buttons/haptics, hotplug, calibration, per-app mapping. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-034.03 — Implement/prove: standardized axes/buttons/haptics**
  - Action: For game controller, implement or reuse and verify this exact obligation: standardized axes/buttons/haptics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for standardized axes/buttons/haptics; retain observable state/resource expectations.
- [ ] **F-DV-034.04 — Implement/prove: hotplug**
  - Action: For game controller, implement or reuse and verify this exact obligation: hotplug. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug; retain observable state/resource expectations.
- [ ] **F-DV-034.05 — Implement/prove: calibration**
  - Action: For game controller, implement or reuse and verify this exact obligation: calibration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-034.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for calibration; retain observable state/resource expectations.
- [ ] **F-DV-034.06 — Implement/prove: per-app mapping**
  - Action: For game controller, implement or reuse and verify this exact obligation: per-app mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-034.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-app mapping; retain observable state/resource expectations.
- [ ] **F-DV-034.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to game controller: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-034.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-034.08 — Integrate into the real consumer and runtime route**
  - Action: Wire game controller into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-034.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-034.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for game controller as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-034.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-035"></a>
## F-DV-035 — NVMe storage provider

**Original requirement:** queue lifecycle, PRP bounds, timeout/reset, namespaces, flush and repeated reuse

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-035.01 — Reconcile existing NVMe storage provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for NVMe storage provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: queue lifecycle, PRP bounds, timeout/reset, namespaces, flush and repeated reuse
- [ ] **F-DV-035.02 — Freeze the exact contract for NVMe storage provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: queue lifecycle, PRP bounds, timeout/reset, namespaces, flush and repeated reuse. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-035.03 — Implement/prove: queue lifecycle**
  - Action: For NVMe storage provider, implement or reuse and verify this exact obligation: queue lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for queue lifecycle; retain observable state/resource expectations.
- [ ] **F-DV-035.04 — Implement/prove: PRP bounds**
  - Action: For NVMe storage provider, implement or reuse and verify this exact obligation: PRP bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for PRP bounds; retain observable state/resource expectations.
- [ ] **F-DV-035.05 — Implement/prove: timeout/reset**
  - Action: For NVMe storage provider, implement or reuse and verify this exact obligation: timeout/reset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout/reset; retain observable state/resource expectations.
- [ ] **F-DV-035.06 — Implement/prove: namespaces**
  - Action: For NVMe storage provider, implement or reuse and verify this exact obligation: namespaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-035.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for namespaces; retain observable state/resource expectations.
- [ ] **F-DV-035.07 — Implement/prove: flush and repeated reuse**
  - Action: For NVMe storage provider, implement or reuse and verify this exact obligation: flush and repeated reuse. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-035.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flush and repeated reuse; retain observable state/resource expectations.
- [ ] **F-DV-035.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to NVMe storage provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-035.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-035.09 — Integrate into the real consumer and runtime route**
  - Action: Wire NVMe storage provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-035.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-035.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for NVMe storage provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-035.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-036"></a>
## F-DV-036 — AHCI/SATA provider

**Original requirement:** ports, command tables, DMA, timeout/reset, hotplug and flush

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-036.01 — Reconcile existing AHCI/SATA provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for AHCI/SATA provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ports, command tables, DMA, timeout/reset, hotplug and flush
- [ ] **F-DV-036.02 — Freeze the exact contract for AHCI/SATA provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ports, command tables, DMA, timeout/reset, hotplug and flush. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-036.03 — Implement/prove: ports**
  - Action: For AHCI/SATA provider, implement or reuse and verify this exact obligation: ports. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ports; retain observable state/resource expectations.
- [ ] **F-DV-036.04 — Implement/prove: command tables**
  - Action: For AHCI/SATA provider, implement or reuse and verify this exact obligation: command tables. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for command tables; retain observable state/resource expectations.
- [ ] **F-DV-036.05 — Implement/prove: DMA**
  - Action: For AHCI/SATA provider, implement or reuse and verify this exact obligation: DMA. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DMA; retain observable state/resource expectations.
- [ ] **F-DV-036.06 — Implement/prove: timeout/reset**
  - Action: For AHCI/SATA provider, implement or reuse and verify this exact obligation: timeout/reset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-036.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout/reset; retain observable state/resource expectations.
- [ ] **F-DV-036.07 — Implement/prove: hotplug and flush**
  - Action: For AHCI/SATA provider, implement or reuse and verify this exact obligation: hotplug and flush. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-036.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug and flush; retain observable state/resource expectations.
- [ ] **F-DV-036.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to AHCI/SATA provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-036.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-036.09 — Integrate into the real consumer and runtime route**
  - Action: Wire AHCI/SATA provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-036.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-036.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for AHCI/SATA provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-036.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-037"></a>
## F-DV-037 — ATA/IDE fallback

**Original requirement:** bounded legacy reads/writes, identify, range, timeout and explicit scope

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-037.01 — Reconcile existing ATA/IDE fallback**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for ATA/IDE fallback. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded legacy reads/writes, identify, range, timeout and explicit scope
- [ ] **F-DV-037.02 — Freeze the exact contract for ATA/IDE fallback**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded legacy reads/writes, identify, range, timeout and explicit scope. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-037.03 — Implement/prove: bounded legacy reads/writes**
  - Action: For ATA/IDE fallback, implement or reuse and verify this exact obligation: bounded legacy reads/writes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded legacy reads/writes; retain observable state/resource expectations.
- [ ] **F-DV-037.04 — Implement/prove: identify**
  - Action: For ATA/IDE fallback, implement or reuse and verify this exact obligation: identify. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for identify; retain observable state/resource expectations.
- [ ] **F-DV-037.05 — Implement/prove: range**
  - Action: For ATA/IDE fallback, implement or reuse and verify this exact obligation: range. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-037.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for range; retain observable state/resource expectations.
- [ ] **F-DV-037.06 — Implement/prove: timeout and explicit scope**
  - Action: For ATA/IDE fallback, implement or reuse and verify this exact obligation: timeout and explicit scope. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-037.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout and explicit scope; retain observable state/resource expectations.
- [ ] **F-DV-037.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to ATA/IDE fallback: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-037.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-037.08 — Integrate into the real consumer and runtime route**
  - Action: Wire ATA/IDE fallback into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-037.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-037.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for ATA/IDE fallback as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-037.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-038"></a>
## F-DV-038 — USB mass storage

**Original requirement:** BOT/SCSI state, capacity, sense, stalls/reset/reconnect and durable physical proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-038.01 — Reconcile existing USB mass storage**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for USB mass storage. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: BOT/SCSI state, capacity, sense, stalls/reset/reconnect and durable physical proof
- [ ] **F-DV-038.02 — Freeze the exact contract for USB mass storage**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: BOT/SCSI state, capacity, sense, stalls/reset/reconnect and durable physical proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-038.03 — Implement/prove: BOT/SCSI state**
  - Action: For USB mass storage, implement or reuse and verify this exact obligation: BOT/SCSI state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for BOT/SCSI state; retain observable state/resource expectations.
- [ ] **F-DV-038.04 — Implement/prove: capacity**
  - Action: For USB mass storage, implement or reuse and verify this exact obligation: capacity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capacity; retain observable state/resource expectations.
- [ ] **F-DV-038.05 — Implement/prove: sense**
  - Action: For USB mass storage, implement or reuse and verify this exact obligation: sense. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sense; retain observable state/resource expectations.
- [ ] **F-DV-038.06 — Implement/prove: stalls/reset/reconnect and durable physical proof**
  - Action: For USB mass storage, implement or reuse and verify this exact obligation: stalls/reset/reconnect and durable physical proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-038.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stalls/reset/reconnect and durable physical proof; retain observable state/resource expectations.
- [ ] **F-DV-038.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to USB mass storage: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-038.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-038.08 — Integrate into the real consumer and runtime route**
  - Action: Wire USB mass storage into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-038.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-038.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for USB mass storage as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-038.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-039"></a>
## F-DV-039 — virtio block

**Original requirement:** negotiated queues, bounds, failure injection, flush/discard and QEMU oracle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-039.01 — Reconcile existing virtio block**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for virtio block. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: negotiated queues, bounds, failure injection, flush/discard and QEMU oracle
- [ ] **F-DV-039.02 — Freeze the exact contract for virtio block**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: negotiated queues, bounds, failure injection, flush/discard and QEMU oracle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-039.03 — Implement/prove: negotiated queues**
  - Action: For virtio block, implement or reuse and verify this exact obligation: negotiated queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for negotiated queues; retain observable state/resource expectations.
- [ ] **F-DV-039.04 — Implement/prove: bounds**
  - Action: For virtio block, implement or reuse and verify this exact obligation: bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounds; retain observable state/resource expectations.
- [ ] **F-DV-039.05 — Implement/prove: failure injection**
  - Action: For virtio block, implement or reuse and verify this exact obligation: failure injection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for failure injection; retain observable state/resource expectations.
- [ ] **F-DV-039.06 — Implement/prove: flush/discard and QEMU oracle**
  - Action: For virtio block, implement or reuse and verify this exact obligation: flush/discard and QEMU oracle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-039.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flush/discard and QEMU oracle; retain observable state/resource expectations.
- [ ] **F-DV-039.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to virtio block: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-039.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-039.08 — Integrate into the real consumer and runtime route**
  - Action: Wire virtio block into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-039.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-039.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for virtio block as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-039.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-040"></a>
## F-DV-040 — SD/eMMC

**Original requirement:** discovery, partitions, DMA, removal, flush and power-loss behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-040.01 — Reconcile existing SD/eMMC**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for SD/eMMC. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discovery, partitions, DMA, removal, flush and power-loss behavior
- [ ] **F-DV-040.02 — Freeze the exact contract for SD/eMMC**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discovery, partitions, DMA, removal, flush and power-loss behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-040.03 — Implement/prove: discovery**
  - Action: For SD/eMMC, implement or reuse and verify this exact obligation: discovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery; retain observable state/resource expectations.
- [ ] **F-DV-040.04 — Implement/prove: partitions**
  - Action: For SD/eMMC, implement or reuse and verify this exact obligation: partitions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for partitions; retain observable state/resource expectations.
- [ ] **F-DV-040.05 — Implement/prove: DMA**
  - Action: For SD/eMMC, implement or reuse and verify this exact obligation: DMA. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DMA; retain observable state/resource expectations.
- [ ] **F-DV-040.06 — Implement/prove: removal**
  - Action: For SD/eMMC, implement or reuse and verify this exact obligation: removal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-040.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for removal; retain observable state/resource expectations.
- [ ] **F-DV-040.07 — Implement/prove: flush and power-loss behavior**
  - Action: For SD/eMMC, implement or reuse and verify this exact obligation: flush and power-loss behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-040.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flush and power-loss behavior; retain observable state/resource expectations.
- [ ] **F-DV-040.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to SD/eMMC: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-040.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-040.09 — Integrate into the real consumer and runtime route**
  - Action: Wire SD/eMMC into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-040.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-040.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for SD/eMMC as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-040.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-041"></a>
## F-DV-041 — Ethernet provider

**Original requirement:** bounded packet queues, link, MTU, offloads, reset and per-driver receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-041.01 — Reconcile existing Ethernet provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Ethernet provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded packet queues, link, MTU, offloads, reset and per-driver receipts
- [ ] **F-DV-041.02 — Freeze the exact contract for Ethernet provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded packet queues, link, MTU, offloads, reset and per-driver receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-041.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-041.03 — Implement/prove: bounded packet queues**
  - Action: For Ethernet provider, implement or reuse and verify this exact obligation: bounded packet queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-041.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded packet queues; retain observable state/resource expectations.
- [ ] **F-DV-041.04 — Implement/prove: link**
  - Action: For Ethernet provider, implement or reuse and verify this exact obligation: link. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-041.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for link; retain observable state/resource expectations.
- [ ] **F-DV-041.05 — Implement/prove: MTU**
  - Action: For Ethernet provider, implement or reuse and verify this exact obligation: MTU. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-041.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MTU; retain observable state/resource expectations.
- [ ] **F-DV-041.06 — Implement/prove: offloads**
  - Action: For Ethernet provider, implement or reuse and verify this exact obligation: offloads. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-041.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for offloads; retain observable state/resource expectations.
- [ ] **F-DV-041.07 — Implement/prove: reset and per-driver receipts**
  - Action: For Ethernet provider, implement or reuse and verify this exact obligation: reset and per-driver receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-041.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reset and per-driver receipts; retain observable state/resource expectations.
- [ ] **F-DV-041.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Ethernet provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-041.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-041.09 — Integrate into the real consumer and runtime route**
  - Action: Wire Ethernet provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-041.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-041.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Ethernet provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-041.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-042"></a>
## F-DV-042 — Wi-Fi provider

**Original requirement:** radio/scan/auth/association/roam/power and firmware provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-042.01 — Reconcile existing Wi-Fi provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Wi-Fi provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: radio/scan/auth/association/roam/power and firmware provenance
- [ ] **F-DV-042.02 — Freeze the exact contract for Wi-Fi provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: radio/scan/auth/association/roam/power and firmware provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-042.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-042.03 — Implement/prove: radio/scan/auth/association/roam/power and firmware provenance**
  - Action: For Wi-Fi provider, implement or reuse and verify this exact obligation: radio/scan/auth/association/roam/power and firmware provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-042.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for radio/scan/auth/association/roam/power and firmware provenance; retain observable state/resource expectations.
- [ ] **F-DV-042.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Wi-Fi provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-042.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-042.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Wi-Fi provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-042.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-042.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Wi-Fi provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-042.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-043"></a>
## F-DV-043 — Bluetooth provider

**Original requirement:** controller, pairing/bonding, profiles, privacy, permissions and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-043.01 — Reconcile existing Bluetooth provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Bluetooth provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: controller, pairing/bonding, profiles, privacy, permissions and recovery
- [ ] **F-DV-043.02 — Freeze the exact contract for Bluetooth provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: controller, pairing/bonding, profiles, privacy, permissions and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-043.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-043.03 — Implement/prove: controller**
  - Action: For Bluetooth provider, implement or reuse and verify this exact obligation: controller. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-043.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controller; retain observable state/resource expectations.
- [ ] **F-DV-043.04 — Implement/prove: pairing/bonding**
  - Action: For Bluetooth provider, implement or reuse and verify this exact obligation: pairing/bonding. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-043.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pairing/bonding; retain observable state/resource expectations.
- [ ] **F-DV-043.05 — Implement/prove: profiles**
  - Action: For Bluetooth provider, implement or reuse and verify this exact obligation: profiles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-043.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for profiles; retain observable state/resource expectations.
- [ ] **F-DV-043.06 — Implement/prove: privacy**
  - Action: For Bluetooth provider, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-043.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-DV-043.07 — Implement/prove: permissions and recovery**
  - Action: For Bluetooth provider, implement or reuse and verify this exact obligation: permissions and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-043.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions and recovery; retain observable state/resource expectations.
- [ ] **F-DV-043.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Bluetooth provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-043.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-043.09 — Integrate into the real consumer and runtime route**
  - Action: Wire Bluetooth provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-043.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-043.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Bluetooth provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-043.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-044"></a>
## F-DV-044 — framebuffer/GOP provider

**Original requirement:** validated scanout mapping, format, pitch, write-combining and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-044.01 — Reconcile existing framebuffer/GOP provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for framebuffer/GOP provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validated scanout mapping, format, pitch, write-combining and fallback
- [ ] **F-DV-044.02 — Freeze the exact contract for framebuffer/GOP provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validated scanout mapping, format, pitch, write-combining and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-044.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-044.03 — Implement/prove: validated scanout mapping**
  - Action: For framebuffer/GOP provider, implement or reuse and verify this exact obligation: validated scanout mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-044.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated scanout mapping; retain observable state/resource expectations.
- [ ] **F-DV-044.04 — Implement/prove: format**
  - Action: For framebuffer/GOP provider, implement or reuse and verify this exact obligation: format. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-044.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format; retain observable state/resource expectations.
- [ ] **F-DV-044.05 — Implement/prove: pitch**
  - Action: For framebuffer/GOP provider, implement or reuse and verify this exact obligation: pitch. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-044.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pitch; retain observable state/resource expectations.
- [ ] **F-DV-044.06 — Implement/prove: write-combining and fallback**
  - Action: For framebuffer/GOP provider, implement or reuse and verify this exact obligation: write-combining and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-044.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for write-combining and fallback; retain observable state/resource expectations.
- [ ] **F-DV-044.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to framebuffer/GOP provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-044.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-044.08 — Integrate into the real consumer and runtime route**
  - Action: Wire framebuffer/GOP provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-044.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-044.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for framebuffer/GOP provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-044.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-045"></a>
## F-DV-045 — Intel display provider

**Original requirement:** native-boot probe/modeset/second/reset/fallback; host harness alone is not completion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-045.01 — Reconcile existing Intel display provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Intel display provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: native-boot probe/modeset/second/reset/fallback; host harness alone is not completion
- [ ] **F-DV-045.02 — Freeze the exact contract for Intel display provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: native-boot probe/modeset/second/reset/fallback; host harness alone is not completion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-045.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-045.03 — Implement/prove: native-boot probe/modeset/second/reset/fallback**
  - Action: For Intel display provider, implement or reuse and verify this exact obligation: native-boot probe/modeset/second/reset/fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-045.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for native-boot probe/modeset/second/reset/fallback; retain observable state/resource expectations.
- [ ] **F-DV-045.04 — Implement/prove: host harness alone is not completion**
  - Action: For Intel display provider, implement or reuse and verify this exact obligation: host harness alone is not completion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-045.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for host harness alone is not completion; retain observable state/resource expectations.
- [ ] **F-DV-045.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Intel display provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-045.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-045.06 — Integrate into the real consumer and runtime route**
  - Action: Wire Intel display provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-045.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-045.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Intel display provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-045.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-046"></a>
## F-DV-046 — virtio-gpu provider

**Original requirement:** negotiated 2D resources/scanout/fences; 3D remains separate

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-046.01 — Reconcile existing virtio-gpu provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for virtio-gpu provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: negotiated 2D resources/scanout/fences; 3D remains separate
- [ ] **F-DV-046.02 — Freeze the exact contract for virtio-gpu provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: negotiated 2D resources/scanout/fences; 3D remains separate. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-046.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-046.03 — Implement/prove: negotiated 2D resources/scanout/fences**
  - Action: For virtio-gpu provider, implement or reuse and verify this exact obligation: negotiated 2D resources/scanout/fences. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-046.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for negotiated 2D resources/scanout/fences; retain observable state/resource expectations.
- [ ] **F-DV-046.04 — Implement/prove: 3D remains separate**
  - Action: For virtio-gpu provider, implement or reuse and verify this exact obligation: 3D remains separate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-046.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for 3D remains separate; retain observable state/resource expectations.
- [ ] **F-DV-046.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to virtio-gpu provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-046.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-046.06 — Integrate into the real consumer and runtime route**
  - Action: Wire virtio-gpu provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-046.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-046.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for virtio-gpu provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-046.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-047"></a>
## F-DV-047 — GPU command provider

**Original requirement:** validated grammar, per-client contexts, fences, hang reset and software oracle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-047.01 — Reconcile existing GPU command provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for GPU command provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: validated grammar, per-client contexts, fences, hang reset and software oracle
- [ ] **F-DV-047.02 — Freeze the exact contract for GPU command provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: validated grammar, per-client contexts, fences, hang reset and software oracle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-047.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-047.03 — Implement/prove: validated grammar**
  - Action: For GPU command provider, implement or reuse and verify this exact obligation: validated grammar. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-047.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated grammar; retain observable state/resource expectations.
- [ ] **F-DV-047.04 — Implement/prove: per-client contexts**
  - Action: For GPU command provider, implement or reuse and verify this exact obligation: per-client contexts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-047.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-client contexts; retain observable state/resource expectations.
- [ ] **F-DV-047.05 — Implement/prove: fences**
  - Action: For GPU command provider, implement or reuse and verify this exact obligation: fences. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-047.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fences; retain observable state/resource expectations.
- [ ] **F-DV-047.06 — Implement/prove: hang reset and software oracle**
  - Action: For GPU command provider, implement or reuse and verify this exact obligation: hang reset and software oracle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-047.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hang reset and software oracle; retain observable state/resource expectations.
- [ ] **F-DV-047.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to GPU command provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-047.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-047.08 — Integrate into the real consumer and runtime route**
  - Action: Wire GPU command provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-047.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-047.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for GPU command provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-047.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-048"></a>
## F-DV-048 — HDA/audio provider

**Original requirement:** codec, DMA stream, IRQ, timing, volume and reset receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-048.01 — Reconcile existing HDA/audio provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for HDA/audio provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: codec, DMA stream, IRQ, timing, volume and reset receipts
- [ ] **F-DV-048.02 — Freeze the exact contract for HDA/audio provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: codec, DMA stream, IRQ, timing, volume and reset receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-048.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-048.03 — Implement/prove: codec**
  - Action: For HDA/audio provider, implement or reuse and verify this exact obligation: codec. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-048.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for codec; retain observable state/resource expectations.
- [ ] **F-DV-048.04 — Implement/prove: DMA stream**
  - Action: For HDA/audio provider, implement or reuse and verify this exact obligation: DMA stream. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-048.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DMA stream; retain observable state/resource expectations.
- [ ] **F-DV-048.05 — Implement/prove: IRQ**
  - Action: For HDA/audio provider, implement or reuse and verify this exact obligation: IRQ. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-048.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IRQ; retain observable state/resource expectations.
- [ ] **F-DV-048.06 — Implement/prove: timing**
  - Action: For HDA/audio provider, implement or reuse and verify this exact obligation: timing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-048.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timing; retain observable state/resource expectations.
- [ ] **F-DV-048.07 — Implement/prove: volume and reset receipts**
  - Action: For HDA/audio provider, implement or reuse and verify this exact obligation: volume and reset receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-048.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for volume and reset receipts; retain observable state/resource expectations.
- [ ] **F-DV-048.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to HDA/audio provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-048.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-048.09 — Integrate into the real consumer and runtime route**
  - Action: Wire HDA/audio provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-048.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-048.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for HDA/audio provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-048.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-049"></a>
## F-DV-049 — virtio-snd provider

**Original requirement:** negotiated streams and deterministic QEMU audio oracle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-049.01 — Reconcile existing virtio-snd provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for virtio-snd provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: negotiated streams and deterministic QEMU audio oracle
- [ ] **F-DV-049.02 — Freeze the exact contract for virtio-snd provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: negotiated streams and deterministic QEMU audio oracle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-049.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-049.03 — Implement/prove: negotiated streams and deterministic QEMU audio oracle**
  - Action: For virtio-snd provider, implement or reuse and verify this exact obligation: negotiated streams and deterministic QEMU audio oracle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-049.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for negotiated streams and deterministic QEMU audio oracle; retain observable state/resource expectations.
- [ ] **F-DV-049.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to virtio-snd provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-049.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-049.05 — Integrate into the real consumer and runtime route**
  - Action: Wire virtio-snd provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-049.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-049.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for virtio-snd provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-049.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-050"></a>
## F-DV-050 — USB Audio provider

**Original requirement:** descriptors, formats, clocking, feedback, hotplug and per-app service route

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-050.01 — Reconcile existing USB Audio provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for USB Audio provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: descriptors, formats, clocking, feedback, hotplug and per-app service route
- [ ] **F-DV-050.02 — Freeze the exact contract for USB Audio provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: descriptors, formats, clocking, feedback, hotplug and per-app service route. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-050.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-050.03 — Implement/prove: descriptors**
  - Action: For USB Audio provider, implement or reuse and verify this exact obligation: descriptors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-050.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for descriptors; retain observable state/resource expectations.
- [ ] **F-DV-050.04 — Implement/prove: formats**
  - Action: For USB Audio provider, implement or reuse and verify this exact obligation: formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-050.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats; retain observable state/resource expectations.
- [ ] **F-DV-050.05 — Implement/prove: clocking**
  - Action: For USB Audio provider, implement or reuse and verify this exact obligation: clocking. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-050.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clocking; retain observable state/resource expectations.
- [ ] **F-DV-050.06 — Implement/prove: feedback**
  - Action: For USB Audio provider, implement or reuse and verify this exact obligation: feedback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-050.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for feedback; retain observable state/resource expectations.
- [ ] **F-DV-050.07 — Implement/prove: hotplug and per-app service route**
  - Action: For USB Audio provider, implement or reuse and verify this exact obligation: hotplug and per-app service route. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-050.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug and per-app service route; retain observable state/resource expectations.
- [ ] **F-DV-050.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to USB Audio provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-050.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-050.09 — Integrate into the real consumer and runtime route**
  - Action: Wire USB Audio provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-050.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-050.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for USB Audio provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-050.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-051"></a>
## F-DV-051 — camera provider

**Original requirement:** formats, frames, privacy indicator, exclusive/shared policy and disconnect

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-051.01 — Reconcile existing camera provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for camera provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: formats, frames, privacy indicator, exclusive/shared policy and disconnect
- [ ] **F-DV-051.02 — Freeze the exact contract for camera provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: formats, frames, privacy indicator, exclusive/shared policy and disconnect. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-051.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-051.03 — Implement/prove: formats**
  - Action: For camera provider, implement or reuse and verify this exact obligation: formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-051.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats; retain observable state/resource expectations.
- [ ] **F-DV-051.04 — Implement/prove: frames**
  - Action: For camera provider, implement or reuse and verify this exact obligation: frames. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-051.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frames; retain observable state/resource expectations.
- [ ] **F-DV-051.05 — Implement/prove: privacy indicator**
  - Action: For camera provider, implement or reuse and verify this exact obligation: privacy indicator. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-051.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy indicator; retain observable state/resource expectations.
- [ ] **F-DV-051.06 — Implement/prove: exclusive/shared policy and disconnect**
  - Action: For camera provider, implement or reuse and verify this exact obligation: exclusive/shared policy and disconnect. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-051.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exclusive/shared policy and disconnect; retain observable state/resource expectations.
- [ ] **F-DV-051.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to camera provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-051.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-051.08 — Integrate into the real consumer and runtime route**
  - Action: Wire camera provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-051.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-051.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for camera provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-051.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-052"></a>
## F-DV-052 — printer provider

**Original requirement:** discovery, capabilities, job stream, status, cancellation and errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-052.01 — Reconcile existing printer provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for printer provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discovery, capabilities, job stream, status, cancellation and errors
- [ ] **F-DV-052.02 — Freeze the exact contract for printer provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discovery, capabilities, job stream, status, cancellation and errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-052.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-052.03 — Implement/prove: discovery**
  - Action: For printer provider, implement or reuse and verify this exact obligation: discovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-052.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery; retain observable state/resource expectations.
- [ ] **F-DV-052.04 — Implement/prove: capabilities**
  - Action: For printer provider, implement or reuse and verify this exact obligation: capabilities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-052.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capabilities; retain observable state/resource expectations.
- [ ] **F-DV-052.05 — Implement/prove: job stream**
  - Action: For printer provider, implement or reuse and verify this exact obligation: job stream. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-052.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for job stream; retain observable state/resource expectations.
- [ ] **F-DV-052.06 — Implement/prove: status**
  - Action: For printer provider, implement or reuse and verify this exact obligation: status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-052.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for status; retain observable state/resource expectations.
- [ ] **F-DV-052.07 — Implement/prove: cancellation and errors**
  - Action: For printer provider, implement or reuse and verify this exact obligation: cancellation and errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-052.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation and errors; retain observable state/resource expectations.
- [ ] **F-DV-052.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to printer provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-052.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-052.09 — Integrate into the real consumer and runtime route**
  - Action: Wire printer provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-052.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-052.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for printer provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-052.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-053"></a>
## F-DV-053 — scanner provider

**Original requirement:** source, resolution/color, page stream, privacy and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-053.01 — Reconcile existing scanner provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scanner provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: source, resolution/color, page stream, privacy and cancellation
- [ ] **F-DV-053.02 — Freeze the exact contract for scanner provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: source, resolution/color, page stream, privacy and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-053.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-053.03 — Implement/prove: source**
  - Action: For scanner provider, implement or reuse and verify this exact obligation: source. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-053.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source; retain observable state/resource expectations.
- [ ] **F-DV-053.04 — Implement/prove: resolution/color**
  - Action: For scanner provider, implement or reuse and verify this exact obligation: resolution/color. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-053.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resolution/color; retain observable state/resource expectations.
- [ ] **F-DV-053.05 — Implement/prove: page stream**
  - Action: For scanner provider, implement or reuse and verify this exact obligation: page stream. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-053.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for page stream; retain observable state/resource expectations.
- [ ] **F-DV-053.06 — Implement/prove: privacy and cancellation**
  - Action: For scanner provider, implement or reuse and verify this exact obligation: privacy and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-053.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and cancellation; retain observable state/resource expectations.
- [ ] **F-DV-053.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scanner provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-053.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-053.08 — Integrate into the real consumer and runtime route**
  - Action: Wire scanner provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-053.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-053.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scanner provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-053.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-054"></a>
## F-DV-054 — battery/thermal/power provider

**Original requirement:** units, unknown states, bounded firmware calls, events and policy separation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-054.01 — Reconcile existing battery/thermal/power provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for battery/thermal/power provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: units, unknown states, bounded firmware calls, events and policy separation
- [ ] **F-DV-054.02 — Freeze the exact contract for battery/thermal/power provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: units, unknown states, bounded firmware calls, events and policy separation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-054.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-054.03 — Implement/prove: units**
  - Action: For battery/thermal/power provider, implement or reuse and verify this exact obligation: units. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-054.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for units; retain observable state/resource expectations.
- [ ] **F-DV-054.04 — Implement/prove: unknown states**
  - Action: For battery/thermal/power provider, implement or reuse and verify this exact obligation: unknown states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-054.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unknown states; retain observable state/resource expectations.
- [ ] **F-DV-054.05 — Implement/prove: bounded firmware calls**
  - Action: For battery/thermal/power provider, implement or reuse and verify this exact obligation: bounded firmware calls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-054.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded firmware calls; retain observable state/resource expectations.
- [ ] **F-DV-054.06 — Implement/prove: events and policy separation**
  - Action: For battery/thermal/power provider, implement or reuse and verify this exact obligation: events and policy separation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-054.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for events and policy separation; retain observable state/resource expectations.
- [ ] **F-DV-054.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to battery/thermal/power provider: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-054.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-054.08 — Integrate into the real consumer and runtime route**
  - Action: Wire battery/thermal/power provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-054.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-054.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for battery/thermal/power provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-054.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-dv-055"></a>
## F-DV-055 — TPM/security device

**Original requirement:** measured identity/sealing/attestation with clear trust and recovery semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DV-055.01 — Reconcile existing TPM/security device**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for TPM/security device. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-and-evidence comparison against the complete requirement: measured identity/sealing/attestation with clear trust and recovery semantics
- [ ] **F-DV-055.02 — Freeze the exact contract for TPM/security device**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: measured identity/sealing/attestation with clear trust and recovery semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DV-055.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DV-055.03 — Implement/prove: measured identity/sealing/attestation with clear trust and recovery semantics**
  - Action: For TPM/security device, implement or reuse and verify this exact obligation: measured identity/sealing/attestation with clear trust and recovery semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DV-055.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for measured identity/sealing/attestation with clear trust and recovery semantics; retain observable state/resource expectations.
- [ ] **F-DV-055.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to TPM/security device: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DV-055.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DV-055.05 — Integrate into the real consumer and runtime route**
  - Action: Wire TPM/security device into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DV-055.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DV-055.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for TPM/security device as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DV-055.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p8-1"></a>
## C-P8.1 — declarative driver descriptor and lifecycle

**Original requirement:** declarative driver descriptor and lifecycle

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 429.

### Preserved original contract

- **Dependencies/current/provenance:** P4 caps/supervision and P2/P6 provider ABIs; Zinnia/Brook/Sapphire; reject probe-only feature claims.
- **I/O and state:** IDs/dependencies/resources/DMA/IOMMU/operations in; claimed handles/provider/counters out; driver lifecycle common state.
- **Invariants/failure:** resources cannot overlap without policy; each device has a default-deny DMA domain; mappings require a scoped DMA handle and detach/revoke removes translations; passthrough never satisfies isolation; cleanup idempotent; bounded transitions; recovery never publishes fake online.
- **Deterministic proof:** descriptor validation, resource conflict, DMA outside every granted edge, stale/revoked mapping, fail each transition, double stop, reset storm, stale IRQ/completion and IOMMU fault telemetry.
- **Target proof:** QEMU models plus exact physical device receipt including a denied out-of-domain DMA attempt where hardware safely permits.
- **Receipt/removal:** descriptor/resources/state/counters/artifact; old direct driver selectable until same gates; remove only at zero callers and successful fallback drill.

### Execution steps

- [ ] **C-P8.1.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P8.1.02 — Resolve this contract's exact dependencies**
  - Action: P4 caps/supervision and P2/P6 provider ABIs; Zinnia/Brook/Sapphire; reject probe-only feature claims. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P8.1.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P8.1.03 — I/O and state — declarative driver descriptor and lifecycle**
  - Action: IDs/dependencies/resources/DMA/IOMMU/operations in; claimed handles/provider/counters out; driver lifecycle common state.
  - Requires: C-P8.1.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P8.1.
- [ ] **C-P8.1.04 — Invariants/failure — declarative driver descriptor and lifecycle**
  - Action: resources cannot overlap without policy; each device has a default-deny DMA domain; mappings require a scoped DMA handle and detach/revoke removes translations; passthrough never satisfies isolation; cleanup idempotent; bounded transitions; recovery never publishes fake online.
  - Requires: C-P8.1.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P8.1.
- [ ] **C-P8.1.05 — Deterministic proof — declarative driver descriptor and lifecycle**
  - Action: descriptor validation, resource conflict, DMA outside every granted edge, stale/revoked mapping, fail each transition, double stop, reset storm, stale IRQ/completion and IOMMU fault telemetry.
  - Requires: C-P8.1.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P8.1.
- [ ] **C-P8.1.06 — Target proof — declarative driver descriptor and lifecycle**
  - Action: QEMU models plus exact physical device receipt including a denied out-of-domain DMA attempt where hardware safely permits.
  - Requires: C-P8.1.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P8.1.
- [ ] **C-P8.1.07 — Receipt/removal — declarative driver descriptor and lifecycle**
  - Action: descriptor/resources/state/counters/artifact; old direct driver selectable until same gates; remove only at zero callers and successful fallback drill.
  - Requires: C-P8.1.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P8.1.
- [ ] **C-P8.1.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P8.1. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P8.1.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p8-2"></a>
## C-P8.2 — USB core/class decomposition

**Original requirement:** USB core/class decomposition

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 438.

### Preserved original contract

- **Dependencies/current/provenance:** P8.1 and current xHCI/HID/MSC; Mollen HID split, Astral USB depth; reject parser/painting in IRQ and unchecked descriptors.
- **I/O and state:** topology/control endpoint and descriptor bytes in; bounded device/class provider handles out; `Detected -> Addressed -> Configured -> Bound -> Removed`.
- **Invariants/failure:** descriptor lengths/features validated; bounded retries/deadlines; class cannot access unrelated controller; unplug cancels and revokes.
- **Deterministic proof:** descriptor fuzz, hub topology, short/error transfers, reset/unplug during I/O, HID reports, MSC sense/residue.
- **Target proof:** QEMU keyboard/mouse/storage; ThinkPad exact xHCI devices and unplug/replug where safe.
- **Receipt/removal:** topology/class/error/lifecycle trace; monolithic path retained behind adapter; remove class logic from core after parity.

### Execution steps

- [ ] **C-P8.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P8.2.02 — Resolve this contract's exact dependencies**
  - Action: P8.1 and current xHCI/HID/MSC; Mollen HID split, Astral USB depth; reject parser/painting in IRQ and unchecked descriptors. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P8.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P8.2.03 — I/O and state — USB core/class decomposition**
  - Action: topology/control endpoint and descriptor bytes in; bounded device/class provider handles out; `Detected -> Addressed -> Configured -> Bound -> Removed`.
  - Requires: C-P8.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P8.2.
- [ ] **C-P8.2.04 — Invariants/failure — USB core/class decomposition**
  - Action: descriptor lengths/features validated; bounded retries/deadlines; class cannot access unrelated controller; unplug cancels and revokes.
  - Requires: C-P8.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P8.2.
- [ ] **C-P8.2.05 — Deterministic proof — USB core/class decomposition**
  - Action: descriptor fuzz, hub topology, short/error transfers, reset/unplug during I/O, HID reports, MSC sense/residue.
  - Requires: C-P8.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P8.2.
- [ ] **C-P8.2.06 — Target proof — USB core/class decomposition**
  - Action: QEMU keyboard/mouse/storage; ThinkPad exact xHCI devices and unplug/replug where safe.
  - Requires: C-P8.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P8.2.
- [ ] **C-P8.2.07 — Receipt/removal — USB core/class decomposition**
  - Action: topology/class/error/lifecycle trace; monolithic path retained behind adapter; remove class logic from core after parity.
  - Requires: C-P8.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P8.2.
- [ ] **C-P8.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P8.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P8.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-04"></a>
## C-DA-04 — device identity and discovery graph

**Original requirement:** device identity and discovery graph

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 128.

### Preserved original contract

**Depends on:** DA-03F and bus-specific enumeration.

**Deliver:** immutable device identities containing bus coordinates, generation,
IDs/class, firmware provenance, BAR/range sizes, interrupt options, DMA width,
coherency and parent topology.

**Invariants:** bus/slot/function order is canonical; multifunction devices are
enumerated; arithmetic is checked; unplug/re-enumeration changes generation;
drivers cannot invent global device IDs.

**Proof:** Nyaux-shaped `pci_map_bar` bus/function reversal, absent function zero,
64-bit/high BAR, malformed
capability list loop, overlapping resources, hot-remove/re-add and unknown firmware
node.

### Execution steps

- [ ] **C-DA-04.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-04.02 — Resolve this contract's exact dependencies**
  - Action: DA-03F and bus-specific enumeration. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-04.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-04.03 — Deliver — device identity and discovery graph**
  - Action: immutable device identities containing bus coordinates, generation, IDs/class, firmware provenance, BAR/range sizes, interrupt options, DMA width, coherency and parent topology.
  - Requires: C-DA-04.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-04.
- [ ] **C-DA-04.04 — Invariants — device identity and discovery graph**
  - Action: bus/slot/function order is canonical; multifunction devices are enumerated; arithmetic is checked; unplug/re-enumeration changes generation; drivers cannot invent global device IDs.
  - Requires: C-DA-04.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-04.
- [ ] **C-DA-04.05 — Proof — device identity and discovery graph**
  - Action: Nyaux-shaped `pci_map_bar` bus/function reversal, absent function zero, 64-bit/high BAR, malformed capability list loop, overlapping resources, hot-remove/re-add and unknown firmware node.
  - Requires: C-DA-04.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-04.
- [ ] **C-DA-04.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-04. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-04.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-05"></a>
## C-DA-05 — driver descriptor and deterministic matching

**Original requirement:** driver descriptor and deterministic matching

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 145.

### Preserved original contract

**Depends on:** DA-04.

**Deliver:** signed descriptor with driver/ABI version, architecture, match rules,
required/optional features, resource needs, limits, deadlines, lifecycle support,
firmware/license provenance and proof profiles.

**Invariants:** matching is pure; ties resolve deterministically; unknown mandatory
capabilities and incompatible ABI/architecture fail before resource mutation.

**Proof:** ambiguous matches, missing dependency, unknown mandatory bit, unsupported
DMA width, bad signature/hash, wrong architecture and downgrade/rollback policy.

### Execution steps

- [ ] **C-DA-05.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-05.02 — Resolve this contract's exact dependencies**
  - Action: DA-04. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-05.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-05.03 — Deliver — driver descriptor and deterministic matching**
  - Action: signed descriptor with driver/ABI version, architecture, match rules, required/optional features, resource needs, limits, deadlines, lifecycle support, firmware/license provenance and proof profiles.
  - Requires: C-DA-05.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-05.
- [ ] **C-DA-05.04 — Invariants — driver descriptor and deterministic matching**
  - Action: matching is pure; ties resolve deterministically; unknown mandatory capabilities and incompatible ABI/architecture fail before resource mutation.
  - Requires: C-DA-05.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-05.
- [ ] **C-DA-05.05 — Proof — driver descriptor and deterministic matching**
  - Action: ambiguous matches, missing dependency, unknown mandatory bit, unsupported DMA width, bad signature/hash, wrong architecture and downgrade/rollback policy.
  - Requires: C-DA-05.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-05.
- [ ] **C-DA-05.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-05. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-05.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-06"></a>
## C-DA-06 — resource and DMA authority

**Original requirement:** resource and DMA authority

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 159.

### Preserved original contract

**Depends on:** handles and VM; an IOMMU mechanism is optional but changes the
evidence and isolation state.

**Deliver:** typed MMIO, PIO, IRQ, DMA, firmware, clock and power handles granted
only for the admitted device.

**Invariants:** no raw physical-address API for ordinary processes. With an IOMMU,
DMA mappings belong to a per-device default-deny domain and detach/revoke removes
translations before page reuse. Without one, the ledger says `not isolated` and
permits only a reviewed in-kernel driver or bounded bounce/pinned-buffer policy;
passthrough or unrestricted DMA never earns isolation. Interrupt and memory
ownership cannot overlap silently.

**Proof:** out-of-range MMIO, arbitrary physical mapping, DMA outside granted pages,
stale generation, double map/unmap, device reset with in-flight DMA, detach before
late completion, passthrough isolation-claim rejection, and explicit no-IOMMU
bounce/pinned-buffer bounds and page-reuse tests.

### Execution steps

- [ ] **C-DA-06.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-06.02 — Resolve this contract's exact dependencies**
  - Action: handles and VM; an IOMMU mechanism is optional but changes the evidence and isolation state. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-06.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-06.03 — Deliver — resource and DMA authority**
  - Action: typed MMIO, PIO, IRQ, DMA, firmware, clock and power handles granted only for the admitted device.
  - Requires: C-DA-06.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-06.
- [ ] **C-DA-06.04 — Invariants — resource and DMA authority**
  - Action: no raw physical-address API for ordinary processes. With an IOMMU, DMA mappings belong to a per-device default-deny domain and detach/revoke removes translations before page reuse. Without one, the ledger says `not isolated` and permits only a reviewed in-kernel driver or bounded bounce/pinned-buffer policy; passthrough or unrestricted DMA never earns isolation. Interrupt and memory ownership cannot overlap silently.
  - Requires: C-DA-06.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-06.
- [ ] **C-DA-06.05 — Proof — resource and DMA authority**
  - Action: out-of-range MMIO, arbitrary physical mapping, DMA outside granted pages, stale generation, double map/unmap, device reset with in-flight DMA, detach before late completion, passthrough isolation-claim rejection, and explicit no-IOMMU bounce/pinned-buffer bounds and page-reuse tests.
  - Requires: C-DA-06.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-06.
- [ ] **C-DA-06.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-06. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-06.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-07"></a>
## C-DA-07 — driver lifecycle supervisor

**Original requirement:** driver lifecycle supervisor

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 179.

### Preserved original contract

**Depends on:** DA-04 through DA-06 plus process/service supervision.

**Deliver:** `Matched -> Admitted -> ResourcesReserved -> Starting -> Online ->
Degraded -> Quiescing -> Offline -> Removed`, with `Failed/Recovering/Quarantined`
branches. `Degraded` records lost functionality, user-visible effect, retry policy
and transition back to Online, Failed or Offline. Every pre-commit state can enter
`Aborting`; `abort(plan)` releases every dependency, BAR/port, IRQ, DMA map/domain,
firmware object, allocation and handle and returns a release receipt before the
device becomes Matched or Absent again.

**Invariants:** provider publication is atomic; every wait has a deadline; stop is
idempotent; remove masks interrupts and stops DMA before freeing resources; retry
budgets are finite; stale completions cannot mutate a new generation.

Every provider request carries provider epoch, device generation, request ID,
deadline, buffer provenance and cancellation state, and completes exactly once as:

```text
OK(result) | UNSUPPORTED(detail) | INVALID_INPUT(detail)
PERMISSION_DENIED(required_capability)
RESOURCE_EXHAUSTED(retryable, minimum_required) | TIMEOUT(stage)
CANCELLED(by) | DEVICE_GONE(generation) | PROVIDER_DIED(epoch)
PARTIAL(progress, continuation) | INTERNAL_ERROR(receipt_id)
```

Provider death stops admission, revokes DMA/IRQ/MMIO, settles every outstanding
request exactly once, resets or quarantines the device, and only then permits a
new provider epoch.

**Proof:** inject failure at every transition and allocation; repeated start/stop;
peer/service death; IRQ storm; timeout; reset failure; unplug with work in flight;
late IRQ/completion after resource reuse; abort after each reservation and compare
the complete resource/handle graph with pre-state.

### Execution steps

- [ ] **C-DA-07.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-07.02 — Resolve this contract's exact dependencies**
  - Action: DA-04 through DA-06 plus process/service supervision. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-07.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-07.03 — Deliver — driver lifecycle supervisor**
  - Action: `Matched -> Admitted -> ResourcesReserved -> Starting -> Online -> Degraded -> Quiescing -> Offline -> Removed`, with `Failed/Recovering/Quarantined` branches. `Degraded` records lost functionality, user-visible effect, retry policy and transition back to Online, Failed or Offline. Every pre-commit state can enter `Aborting`; `abort(plan)` releases every dependency, BAR/port, IRQ, DMA map/domain, firmware object, allocation and handle and returns a release receipt before the device becomes Matched or Absent again.
  - Requires: C-DA-07.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-07.
- [ ] **C-DA-07.04 — Invariants — driver lifecycle supervisor**
  - Action: provider publication is atomic; every wait has a deadline; stop is idempotent; remove masks interrupts and stops DMA before freeing resources; retry budgets are finite; stale completions cannot mutate a new generation.  Every provider request carries provider epoch, device generation, request ID, deadline, buffer provenance and cancellation state, and completes exactly once as:  ```text OK(result) | UNSUPPORTED(detail) | INVALID_INPUT(detail) PERMISSION_DENIED(required_capability) RESOURCE_EXHAUSTED(retryable, minimum_required) | TIMEOUT(stage) CANCELLED(by) | DEVICE_GONE(generation) | PROVIDER_DIED(epoch) PARTIAL(progress, continuation) | INTERNAL_ERROR(receipt_id) ```  Provider death stops admission, revokes DMA/IRQ/MMIO, settles every outstanding request exactly once, resets or quarantines the device, and only then permits a new provider epoch.
  - Requires: C-DA-07.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-07.
- [ ] **C-DA-07.05 — Proof — driver lifecycle supervisor**
  - Action: inject failure at every transition and allocation; repeated start/stop; peer/service death; IRQ storm; timeout; reset failure; unplug with work in flight; late IRQ/completion after resource reuse; abort after each reservation and compare the complete resource/handle graph with pre-state.
  - Requires: C-DA-07.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-07.
- [ ] **C-DA-07.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-07. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-07.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-07m"></a>
## C-DA-07M — kernel/driver module loader

**Original requirement:** kernel/driver module loader

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 215.

### Preserved original contract

**Depends on:** DA-05 through DA-07 and DA-24 package authenticity. Until DA-24
exists, boot-critical providers remain statically linked and dynamic loading is
unsupported.

**Deliver:** stage the complete dependency closure; validate module byte length,
architecture, kernel ABI, imports/exports, symbol versions/conflicts, relocations,
constructors/init arrays, descriptor and resource plan; initialize privately and
publish atomically. Support fini/unload only for modules declaring and proving it.

**Invariants:** ordinary apps cannot request ring-0 loading; no symbol or driver is
visible before the transaction commits; failure rolls back dependency refs, memory,
registries and resources; unload quiesces workers/IRQs/DMA and refuses while live
objects, callbacks, mappings or references remain.

**Proof:** malformed/truncated module, wrong machine/ABI, missing/duplicate symbol,
relocation overflow, dependency cycle/version conflict, constructor/init failure,
nth-module rollback, concurrent load, unload under traffic, live-reference refusal,
worker timeout and late IRQ/callback after unload.

### Execution steps

- [ ] **C-DA-07M.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-07M.02 — Resolve this contract's exact dependencies**
  - Action: DA-05 through DA-07 and DA-24 package authenticity. Until DA-24 exists, boot-critical providers remain statically linked and dynamic loading is unsupported. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-07M.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-07M.03 — Deliver — kernel/driver module loader**
  - Action: stage the complete dependency closure; validate module byte length, architecture, kernel ABI, imports/exports, symbol versions/conflicts, relocations, constructors/init arrays, descriptor and resource plan; initialize privately and publish atomically. Support fini/unload only for modules declaring and proving it.
  - Requires: C-DA-07M.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-07M.
- [ ] **C-DA-07M.04 — Invariants — kernel/driver module loader**
  - Action: ordinary apps cannot request ring-0 loading; no symbol or driver is visible before the transaction commits; failure rolls back dependency refs, memory, registries and resources; unload quiesces workers/IRQs/DMA and refuses while live objects, callbacks, mappings or references remain.
  - Requires: C-DA-07M.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-07M.
- [ ] **C-DA-07M.05 — Proof — kernel/driver module loader**
  - Action: malformed/truncated module, wrong machine/ABI, missing/duplicate symbol, relocation overflow, dependency cycle/version conflict, constructor/init failure, nth-module rollback, concurrent load, unload under traffic, live-reference refusal, worker timeout and late IRQ/callback after unload.
  - Requires: C-DA-07M.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-07M.
- [ ] **C-DA-07M.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-07M. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-07M.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-08"></a>
## C-DA-08 — driver health and failure receipts

**Original requirement:** driver health and failure receipts

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 236.

### Preserved original contract

**Depends on:** DA-07 and the DA-01 evidence/receipt sink.

**Deliver:** state, generation, resource ownership, queue occupancy, progress,
timeouts, resets, drops, last decoded hardware status and cleanup result as a
provider-side typed telemetry record. DA-08O consumes and aggregates it; DA-08 does
not depend on that service being alive.

**Invariants:** counters are monotonic or explicitly generation-scoped; health
reads cannot stall the driver; sensitive device data is redacted; dropped receipts
are themselves counted and anchored.

**Proof:** telemetry during wedge/reset/remove; counter saturation; log-full and
observer-death behavior; exact artifact/device identity on hardware.

### Execution steps

- [ ] **C-DA-08.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-08.02 — Resolve this contract's exact dependencies**
  - Action: DA-07 and the DA-01 evidence/receipt sink. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-08.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-08.03 — Deliver — driver health and failure receipts**
  - Action: state, generation, resource ownership, queue occupancy, progress, timeouts, resets, drops, last decoded hardware status and cleanup result as a provider-side typed telemetry record. DA-08O consumes and aggregates it; DA-08 does not depend on that service being alive.
  - Requires: C-DA-08.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-08.
- [ ] **C-DA-08.04 — Invariants — driver health and failure receipts**
  - Action: counters are monotonic or explicitly generation-scoped; health reads cannot stall the driver; sensitive device data is redacted; dropped receipts are themselves counted and anchored.
  - Requires: C-DA-08.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-08.
- [ ] **C-DA-08.05 — Proof — driver health and failure receipts**
  - Action: telemetry during wedge/reset/remove; counter saturation; log-full and observer-death behavior; exact artifact/device identity on hardware.
  - Requires: C-DA-08.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-08.
- [ ] **C-DA-08.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-08. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-08.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-12"></a>
## C-DA-12 — USB controller/topology/class split

**Original requirement:** USB controller/topology/class split

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 322.

### Preserved original contract

**Depends on:** DA-07, DA-10 and DA-11.

**Deliver:** controller owns rings/slots/interrupters; USB service owns descriptor
admission, topology, address/configuration and class binding; class drivers receive
endpoint transfer handles only.

**Invariants:** descriptor totals and topology depth are bounded; a class driver
cannot access unrelated controller/device memory; endpoint ownership survives
short/error completion; unplug cancels every transfer exactly once.

**Proof:** malformed/recursive descriptors, hub-depth and port churn, short packets,
endpoint halt, reset during control/bulk/interrupt I/O, slot reuse, unplug with work
in flight, HID and mass-storage reconnect.

### Execution steps

- [ ] **C-DA-12.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-12.02 — Resolve this contract's exact dependencies**
  - Action: DA-07, DA-10 and DA-11. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-12.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-12.03 — Deliver — USB controller/topology/class split**
  - Action: controller owns rings/slots/interrupters; USB service owns descriptor admission, topology, address/configuration and class binding; class drivers receive endpoint transfer handles only.
  - Requires: C-DA-12.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-12.
- [ ] **C-DA-12.04 — Invariants — USB controller/topology/class split**
  - Action: descriptor totals and topology depth are bounded; a class driver cannot access unrelated controller/device memory; endpoint ownership survives short/error completion; unplug cancels every transfer exactly once.
  - Requires: C-DA-12.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-12.
- [ ] **C-DA-12.05 — Proof — USB controller/topology/class split**
  - Action: malformed/recursive descriptors, hub-depth and port churn, short packets, endpoint halt, reset during control/bulk/interrupt I/O, slot reuse, unplug with work in flight, HID and mass-storage reconnect.
  - Requires: C-DA-12.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-12.
- [ ] **C-DA-12.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-12. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-12.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-43"></a>
## C-DA-43 — unsupported and degraded-state gate

**Original requirement:** unsupported and degraded-state gate

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 927.

### Preserved original contract

Every missing feature has a typed visible state: unsupported, external, simulated,
stub, source-only, built, unreachable, unavailable, degraded, failed or quarantined.
No fixed success, silent drop, generic blank tile or fake output is permitted.

### Execution steps

- [ ] **C-DA-43.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-43.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-43.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-43.03 — Execute the preserved contract requirements**
  - Action: Every missing feature has a typed visible state: unsupported, external, simulated, stub, source-only, built, unreachable, unavailable, degraded, failed or quarantined. No fixed success, silent drop, generic blank tile or fake output is permitted.
  - Requires: C-DA-43.02.
  - Acceptance: Every requirement in the original contract has an independent observable result.
- [ ] **C-DA-43.04 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-43. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-43.03.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-plat-006"></a>
## T-PLAT-006 — AML/uACPI execution service

**Original requirement:** bounded methods, OS hooks, policy outside parser

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 35.

### Execution steps

- [ ] **T-PLAT-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AML/uACPI execution service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-006.02 — Specify the complete target boundary**
  - Action: AML/uACPI execution service must supply: bounded methods, OS hooks, policy outside parser. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse AML/uACPI execution service through the shared platform contract, delivering every part of: bounded methods, OS hooks, policy outside parser. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AML/uACPI execution service.
- [ ] **T-PLAT-006.05 — Qualify and retain this target's own result**
  - Action: Bind AML/uACPI execution service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-012"></a>
## T-PLAT-012 — x86 PIC

**Original requirement:** legacy fallback, ownership and masking

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 41.

### Execution steps

- [ ] **T-PLAT-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86 PIC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-012.02 — Specify the complete target boundary**
  - Action: x86 PIC must supply: legacy fallback, ownership and masking. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86 PIC through the shared platform contract, delivering every part of: legacy fallback, ownership and masking. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86 PIC.
- [ ] **T-PLAT-012.05 — Qualify and retain this target's own result**
  - Action: Bind x86 PIC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-013"></a>
## T-PLAT-013 — x86 LAPIC/x2APIC

**Original requirement:** per-CPU timer/IPI/error lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 42.

### Execution steps

- [ ] **T-PLAT-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86 LAPIC/x2APIC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-013.02 — Specify the complete target boundary**
  - Action: x86 LAPIC/x2APIC must supply: per-CPU timer/IPI/error lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86 LAPIC/x2APIC through the shared platform contract, delivering every part of: per-CPU timer/IPI/error lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86 LAPIC/x2APIC.
- [ ] **T-PLAT-013.05 — Qualify and retain this target's own result**
  - Action: Bind x86 LAPIC/x2APIC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-014"></a>
## T-PLAT-014 — x86 I/O APIC

**Original requirement:** GSI/override/trigger/polarity ownership

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 43.

### Execution steps

- [ ] **T-PLAT-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86 I/O APIC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-014.02 — Specify the complete target boundary**
  - Action: x86 I/O APIC must supply: GSI/override/trigger/polarity ownership. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86 I/O APIC through the shared platform contract, delivering every part of: GSI/override/trigger/polarity ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86 I/O APIC.
- [ ] **T-PLAT-014.05 — Qualify and retain this target's own result**
  - Action: Bind x86 I/O APIC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-015"></a>
## T-PLAT-015 — MSI/MSI-X allocator

**Original requirement:** vector ownership, affinity, mask and teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 44.

### Execution steps

- [ ] **T-PLAT-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve MSI/MSI-X allocator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-015.02 — Specify the complete target boundary**
  - Action: MSI/MSI-X allocator must supply: vector ownership, affinity, mask and teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse MSI/MSI-X allocator through the shared platform contract, delivering every part of: vector ownership, affinity, mask and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-015.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for MSI/MSI-X allocator.
- [ ] **T-PLAT-015.05 — Qualify and retain this target's own result**
  - Action: Bind MSI/MSI-X allocator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-021"></a>
## T-PLAT-021 — PIT

**Original requirement:** calibration/fallback, bounded programming

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 50.

### Execution steps

- [ ] **T-PLAT-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PIT to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-021.02 — Specify the complete target boundary**
  - Action: PIT must supply: calibration/fallback, bounded programming. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse PIT through the shared platform contract, delivering every part of: calibration/fallback, bounded programming. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-021.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PIT.
- [ ] **T-PLAT-021.05 — Qualify and retain this target's own result**
  - Action: Bind PIT to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-022"></a>
## T-PLAT-022 — HPET

**Original requirement:** checked table/counters, wrap and routing

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 51.

### Execution steps

- [ ] **T-PLAT-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve HPET to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-022.02 — Specify the complete target boundary**
  - Action: HPET must supply: checked table/counters, wrap and routing. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse HPET through the shared platform contract, delivering every part of: checked table/counters, wrap and routing. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-022.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for HPET.
- [ ] **T-PLAT-022.05 — Qualify and retain this target's own result**
  - Action: Bind HPET to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-024"></a>
## T-PLAT-024 — CMOS/RTC

**Original requirement:** stable read, invalid time and wall-clock separation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 53.

### Execution steps

- [ ] **T-PLAT-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve CMOS/RTC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-024.02 — Specify the complete target boundary**
  - Action: CMOS/RTC must supply: stable read, invalid time and wall-clock separation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse CMOS/RTC through the shared platform contract, delivering every part of: stable read, invalid time and wall-clock separation. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-024.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for CMOS/RTC.
- [ ] **T-PLAT-024.05 — Qualify and retain this target's own result**
  - Action: Bind CMOS/RTC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-032"></a>
## T-PLAT-032 — hardware RNG

**Original requirement:** health checks, entropy accounting, fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 61.

### Execution steps

- [ ] **T-PLAT-032.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve hardware RNG to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-032.02 — Specify the complete target boundary**
  - Action: hardware RNG must supply: health checks, entropy accounting, fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-032.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-032.03 — Implement the exact target behavior**
  - Action: Implement or reuse hardware RNG through the shared platform contract, delivering every part of: health checks, entropy accounting, fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-032.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-032.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-032.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for hardware RNG.
- [ ] **T-PLAT-032.05 — Qualify and retain this target's own result**
  - Action: Bind hardware RNG to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-032.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-036"></a>
## T-PLAT-036 — KVM paravirtual clock

**Original requirement:** versioned stable reads, migration and clock fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 65.

### Execution steps

- [ ] **T-PLAT-036.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve KVM paravirtual clock to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-036.02 — Specify the complete target boundary**
  - Action: KVM paravirtual clock must supply: versioned stable reads, migration and clock fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-036.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-036.03 — Implement the exact target behavior**
  - Action: Implement or reuse KVM paravirtual clock through the shared platform contract, delivering every part of: versioned stable reads, migration and clock fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-036.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-036.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-036.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for KVM paravirtual clock.
- [ ] **T-PLAT-036.05 — Qualify and retain this target's own result**
  - Action: Bind KVM paravirtual clock to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-036.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-001"></a>
## T-BUS-001 — PCI legacy configuration

**Original requirement:** bridges/multifunction/ranges and tuple correctness

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 73.

### Execution steps

- [ ] **T-BUS-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PCI legacy configuration to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-001.02 — Specify the complete target boundary**
  - Action: PCI legacy configuration must supply: bridges/multifunction/ranges and tuple correctness. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse PCI legacy configuration through the shared platform contract, delivering every part of: bridges/multifunction/ranges and tuple correctness. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PCI legacy configuration.
- [ ] **T-BUS-001.05 — Qualify and retain this target's own result**
  - Action: Bind PCI legacy configuration to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-002"></a>
## T-BUS-002 — PCIe ECAM/MCFG

**Original requirement:** checked segments/bus windows and mappings

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 74.

### Execution steps

- [ ] **T-BUS-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PCIe ECAM/MCFG to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-002.02 — Specify the complete target boundary**
  - Action: PCIe ECAM/MCFG must supply: checked segments/bus windows and mappings. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse PCIe ECAM/MCFG through the shared platform contract, delivering every part of: checked segments/bus windows and mappings. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PCIe ECAM/MCFG.
- [ ] **T-BUS-002.05 — Qualify and retain this target's own result**
  - Action: Bind PCIe ECAM/MCFG to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-003"></a>
## T-BUS-003 — PCI bridge/resource allocator

**Original requirement:** BAR sizing, 64-bit BARs, windows and rollback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 75.

### Execution steps

- [ ] **T-BUS-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PCI bridge/resource allocator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-003.02 — Specify the complete target boundary**
  - Action: PCI bridge/resource allocator must supply: BAR sizing, 64-bit BARs, windows and rollback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse PCI bridge/resource allocator through the shared platform contract, delivering every part of: BAR sizing, 64-bit BARs, windows and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PCI bridge/resource allocator.
- [ ] **T-BUS-003.05 — Qualify and retain this target's own result**
  - Action: Bind PCI bridge/resource allocator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-004"></a>
## T-BUS-004 — PCI hotplug/rebind

**Original requirement:** surprise removal, generation and competition

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 76.

### Execution steps

- [ ] **T-BUS-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PCI hotplug/rebind to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-004.02 — Specify the complete target boundary**
  - Action: PCI hotplug/rebind must supply: surprise removal, generation and competition. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse PCI hotplug/rebind through the shared platform contract, delivering every part of: surprise removal, generation and competition. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PCI hotplug/rebind.
- [ ] **T-BUS-004.05 — Qualify and retain this target's own result**
  - Action: Bind PCI hotplug/rebind to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-006"></a>
## T-BUS-006 — ISA/LPC platform bus

**Original requirement:** decoded resource ownership and conflicts

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 78.

### Execution steps

- [ ] **T-BUS-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ISA/LPC platform bus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-006.02 — Specify the complete target boundary**
  - Action: ISA/LPC platform bus must supply: decoded resource ownership and conflicts. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse ISA/LPC platform bus through the shared platform contract, delivering every part of: decoded resource ownership and conflicts. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ISA/LPC platform bus.
- [ ] **T-BUS-006.05 — Qualify and retain this target's own result**
  - Action: Bind ISA/LPC platform bus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-007"></a>
## T-BUS-007 — ACPI-enumerated platform bus

**Original requirement:** _HID/_CID/_CRS/_PRS/_STA admission

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 79.

### Execution steps

- [ ] **T-BUS-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ACPI-enumerated platform bus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-007.02 — Specify the complete target boundary**
  - Action: ACPI-enumerated platform bus must supply: _HID/_CID/_CRS/_PRS/_STA admission. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse ACPI-enumerated platform bus through the shared platform contract, delivering every part of: _HID/_CID/_CRS/_PRS/_STA admission. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ACPI-enumerated platform bus.
- [ ] **T-BUS-007.05 — Qualify and retain this target's own result**
  - Action: Bind ACPI-enumerated platform bus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-009"></a>
## T-BUS-009 — virtio PCI transport

**Original requirement:** feature negotiation, queues, reset and generation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 81.

### Execution steps

- [ ] **T-BUS-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio PCI transport to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-009.02 — Specify the complete target boundary**
  - Action: virtio PCI transport must supply: feature negotiation, queues, reset and generation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio PCI transport through the shared platform contract, delivering every part of: feature negotiation, queues, reset and generation. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio PCI transport.
- [ ] **T-BUS-009.05 — Qualify and retain this target's own result**
  - Action: Bind virtio PCI transport to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-011"></a>
## T-BUS-011 — USB device bus/core

**Original requirement:** address/config/interface/endpoint ownership

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 83.

### Execution steps

- [ ] **T-BUS-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB device bus/core to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-011.02 — Specify the complete target boundary**
  - Action: USB device bus/core must supply: address/config/interface/endpoint ownership. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB device bus/core through the shared platform contract, delivering every part of: address/config/interface/endpoint ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB device bus/core.
- [ ] **T-BUS-011.05 — Qualify and retain this target's own result**
  - Action: Bind USB device bus/core to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-012"></a>
## T-BUS-012 — USB hub/topology

**Original requirement:** depth/power/change/removal and child teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 84.

### Execution steps

- [ ] **T-BUS-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB hub/topology to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-012.02 — Specify the complete target boundary**
  - Action: USB hub/topology must supply: depth/power/change/removal and child teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB hub/topology through the shared platform contract, delivering every part of: depth/power/change/removal and child teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB hub/topology.
- [ ] **T-BUS-012.05 — Qualify and retain this target's own result**
  - Action: Bind USB hub/topology to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-019"></a>
## T-BUS-019 — DMA buffer allocator

**Original requirement:** width/alignment/coherency/pin/unpin/zero

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 91.

### Execution steps

- [ ] **T-BUS-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve DMA buffer allocator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-019.02 — Specify the complete target boundary**
  - Action: DMA buffer allocator must supply: width/alignment/coherency/pin/unpin/zero. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse DMA buffer allocator through the shared platform contract, delivering every part of: width/alignment/coherency/pin/unpin/zero. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-019.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for DMA buffer allocator.
- [ ] **T-BUS-019.05 — Qualify and retain this target's own result**
  - Action: Bind DMA buffer allocator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-020"></a>
## T-BUS-020 — Intel VT-d/DMAR IOMMU

**Original requirement:** default-deny per-device domains and fault log

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 92.

### Execution steps

- [ ] **T-BUS-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel VT-d/DMAR IOMMU to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-020.02 — Specify the complete target boundary**
  - Action: Intel VT-d/DMAR IOMMU must supply: default-deny per-device domains and fault log. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel VT-d/DMAR IOMMU through the shared platform contract, delivering every part of: default-deny per-device domains and fault log. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-020.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel VT-d/DMAR IOMMU.
- [ ] **T-BUS-020.05 — Qualify and retain this target's own result**
  - Action: Bind Intel VT-d/DMAR IOMMU to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-024"></a>
## T-BUS-024 — no-IOMMU fallback

**Original requirement:** explicitly non-isolated bounce/pinned policy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 96.

### Execution steps

- [ ] **T-BUS-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve no-IOMMU fallback to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-024.02 — Specify the complete target boundary**
  - Action: no-IOMMU fallback must supply: explicitly non-isolated bounce/pinned policy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse no-IOMMU fallback through the shared platform contract, delivering every part of: explicitly non-isolated bounce/pinned policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-024.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for no-IOMMU fallback.
- [ ] **T-BUS-024.05 — Qualify and retain this target's own result**
  - Action: Bind no-IOMMU fallback to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-004"></a>
## T-USB-004 — xHCI host

**Original requirement:** rings/interrupters/ports/cancel/reset/remove

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 139.

### Execution steps

- [ ] **T-USB-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve xHCI host to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-004.02 — Specify the complete target boundary**
  - Action: xHCI host must supply: rings/interrupters/ports/cancel/reset/remove. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse xHCI host through the shared platform contract, delivering every part of: rings/interrupters/ports/cancel/reset/remove. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for xHCI host.
- [ ] **T-USB-004.05 — Qualify and retain this target's own result**
  - Action: Bind xHCI host to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-002"></a>
## T-VM-002 — virtio RNG

**Original requirement:** entropy health/accounting and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 259.

### Execution steps

- [ ] **T-VM-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio RNG to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-002.02 — Specify the complete target boundary**
  - Action: virtio RNG must supply: entropy health/accounting and reset. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio RNG through the shared platform contract, delivering every part of: entropy health/accounting and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-002.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio RNG.
- [ ] **T-VM-002.05 — Qualify and retain this target's own result**
  - Action: Bind virtio RNG to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-020"></a>
## T-SVC-020 — Device Manager

**Original requirement:** immutable inventory, match/bind/rebind/remove and generations

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 35.

### Execution steps

- [ ] **T-SVC-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Device Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-020.02 — Specify the complete target boundary**
  - Action: Device Manager must supply: immutable inventory, match/bind/rebind/remove and generations. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse Device Manager through the shared platform contract, delivering every part of: immutable inventory, match/bind/rebind/remove and generations. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-020.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-SVC-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Device Manager.
- [ ] **T-SVC-020.05 — Qualify and retain this target's own result**
  - Action: Bind Device Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-021"></a>
## T-SVC-021 — Driver Supervisor

**Original requirement:** provider process lifecycle, isolation and recovery

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 36.

### Execution steps

- [ ] **T-SVC-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Driver Supervisor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-021.02 — Specify the complete target boundary**
  - Action: Driver Supervisor must supply: provider process lifecycle, isolation and recovery. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse Driver Supervisor through the shared platform contract, delivering every part of: provider process lifecycle, isolation and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-021.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-SVC-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Driver Supervisor.
- [ ] **T-SVC-021.05 — Qualify and retain this target's own result**
  - Action: Bind Driver Supervisor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-023"></a>
## T-SVC-023 — DMA/IOMMU Manager

**Original requirement:** domains, buffers, revoke-before-reuse and fault telemetry

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 38.

### Execution steps

- [ ] **T-SVC-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve DMA/IOMMU Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-023.02 — Specify the complete target boundary**
  - Action: DMA/IOMMU Manager must supply: domains, buffers, revoke-before-reuse and fault telemetry. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse DMA/IOMMU Manager through the shared platform contract, delivering every part of: domains, buffers, revoke-before-reuse and fault telemetry. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-023.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-SVC-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for DMA/IOMMU Manager.
- [ ] **T-SVC-023.05 — Qualify and retain this target's own result**
  - Action: Bind DMA/IOMMU Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-026"></a>
## T-SVC-026 — Hotplug/Event Broker

**Original requirement:** device add/remove/change with bounded subscriber queues

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 41.

### Execution steps

- [ ] **T-SVC-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Hotplug/Event Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-11, H-05.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-026.02 — Specify the complete target boundary**
  - Action: Hotplug/Event Broker must supply: device add/remove/change with bounded subscriber queues. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse Hotplug/Event Broker through the shared platform contract, delivering every part of: device add/remove/change with bounded subscriber queues. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-026.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-SVC-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Hotplug/Event Broker.
- [ ] **T-SVC-026.05 — Qualify and retain this target's own result**
  - Action: Bind Hotplug/Event Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
