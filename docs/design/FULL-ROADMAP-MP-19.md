# MP-19: Deliver every broader hardware and architecture target

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/boot/; kernel/src/arch/; kernel/src/drivers/; kernel/docs/plans/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-19` exports: Per-profile architecture/device contracts with independent simulation and exact physical qualification records.

The handoff enables only its named subset. `CLOSE-19` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-19.01 — Define concrete laptop/desktop/tablet/headless and architecture profiles without dropping the remaining registry targets

Define concrete laptop/desktop/tablet/headless and architecture profiles without dropping the remaining registry targets.

**Requires:** `D-01`, `D-02`, `D-04`, `D-06`, `D-11`, `D-13`, `D-25`, `H-00`, `H-02`, `H-03`, `H-04`, `H-05`, `H-08`, `H-09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.02 — Bring up AArch64/RISC-V64 memory, interrupts, timers, SMP, process, service and boot contracts independently

Bring up AArch64/RISC-V64 memory, interrupts, timers, SMP, process, service and boot contracts independently.

**Requires:** `M-19.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-19 — Bounded development handoff: Deliver every broader hardware and architecture target

Per-profile architecture/device contracts with independent simulation and exact physical qualification records.

**Requires:** `M-19.02`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-19.03 — Deepen suspend/resume, battery, thermal, radios, sensors, camera, cellular and secure-hardware providers

Deepen suspend/resume, battery, thermal, radios, sensors, camera, cellular and secure-hardware providers.

**Requires:** `M-19.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.04 — Add later GPU/video, heterogeneous compute and FPGA/accelerator targets through bounded resource and reset contracts

Add later GPU/video, heterogeneous compute and FPGA/accelerator targets through bounded resource and reset contracts.

**Requires:** `M-19.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.05 — For each provider instance, obtain primary specifications, firmware/license inputs, topology and an independent test oracle

For each provider instance, obtain primary specifications, firmware/license inputs, topology and an independent test oracle.

**Requires:** `M-19.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.06 — Implement host/simulation admission and failure tests before device writes or passthrough

Implement host/simulation admission and failure tests before device writes or passthrough.

**Requires:** `M-19.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.07 — Qualify exact hardware revisions, firmware, power states, reset, removal, sleep and failure recovery

Qualify exact hardware revisions, firmware, power states, reset, removal, sleep and failure recovery.

**Requires:** `M-19.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.08 — Keep unavailable hardware as a named unfinished task with acquisition/loan/emulation needs

Keep unavailable hardware as a named unfinished task with acquisition/loan/emulation needs; do not turn absence into a pass.

**Requires:** `M-19.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-19.09 — Reconcile older language full-stack ambitions with the current programme

Reconcile older language full-stack ambitions with the current programme; preserve any requested extra destination as an explicit scope decision rather than silently losing it.

**Requires:** `M-19.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-FH-001](#f-fh-001) | feature | laptop power/suspend |
| [F-FH-002](#f-fh-002) | feature | lid/buttons/hotkeys |
| [F-FH-003](#f-fh-003) | feature | brightness/backlight |
| [F-FH-004](#f-fh-004) | feature | external monitor/dock |
| [F-FH-005](#f-fh-005) | feature | HiDPI/4K |
| [F-FH-006](#f-fh-006) | feature | variable/high refresh |
| [F-FH-007](#f-fh-007) | feature | touchscreen convertible |
| [F-FH-008](#f-fh-008) | feature | camera/microphone hardware |
| [F-FH-009](#f-fh-009) | feature | wireless headset |
| [F-FH-010](#f-fh-010) | feature | printer/scanner hardware |
| [F-FH-011](#f-fh-011) | feature | removable SD media |
| [F-FH-012](#f-fh-012) | feature | biometric authentication |
| [F-FH-013](#f-fh-013) | feature | secure boot/measured boot |
| [F-FH-014](#f-fh-014) | feature | smart-glasses display |
| [F-FH-015](#f-fh-015) | feature | sensor framework |
| [F-FH-016](#f-fh-016) | feature | cellular modem |
| [F-FH-017](#f-fh-017) | feature | heterogeneous CPU/GPU |
| [F-FH-018](#f-fh-018) | feature | hardware video codec |
| [F-FH-019](#f-fh-019) | feature | FPGA/custom accelerator |
| [F-FH-020](#f-fh-020) | feature | headless/server profile |
| [C-P8.3](#c-p8-3) | contract | additional storage/NIC/audio providers |
| [C-P12.5](#c-p12-5) | contract | secondary architecture evidence lane |
| [C-DA-15](#c-da-15) | contract | clock, sensors and power provider |
| [C-DA-15S](#c-da-15s) | contract | system suspend and resume orchestration |
| [C-DA-16](#c-da-16) | contract | later device classes |
| [T-PLAT-008](#t-plat-008) | target | FDT/device-tree admission |
| [T-PLAT-010](#t-plat-010) | target | x86 microcode update provider |
| [T-PLAT-016](#t-plat-016) | target | AArch64 PSCI/EL platform |
| [T-PLAT-017](#t-plat-017) | target | ARM GICv2 |
| [T-PLAT-018](#t-plat-018) | target | ARM GICv3/ITS |
| [T-PLAT-019](#t-plat-019) | target | RISC-V SBI platform |
| [T-PLAT-020](#t-plat-020) | target | RISC-V PLIC |
| [T-PLAT-025](#t-plat-025) | target | ARM generic timer |
| [T-PLAT-026](#t-plat-026) | target | PL031/Goldfish RTC |
| [T-PLAT-027](#t-plat-027) | target | RISC-V timer |
| [T-PLAT-029](#t-plat-029) | target | ARM PL011 UART |
| [T-PLAT-030](#t-plat-030) | target | Raspberry Pi mini-UART/mailbox |
| [T-PLAT-033](#t-plat-033) | target | TPM 2.0 |
| [T-PLAT-038](#t-plat-038) | target | Apple SMC |
| [T-BUS-005](#t-bus-005) | target | Intel VMD discovery |
| [T-BUS-008](#t-bus-008) | target | FDT platform bus |
| [T-BUS-010](#t-bus-010) | target | virtio MMIO transport |
| [T-BUS-013](#t-bus-013) | target | I2C core |
| [T-BUS-014](#t-bus-014) | target | Intel LPSS/DesignWare I2C |
| [T-BUS-015](#t-bus-015) | target | SPI core/controller |
| [T-BUS-016](#t-bus-016) | target | GPIO core/controller |
| [T-BUS-017](#t-bus-017) | target | embedded-controller bus |
| [T-BUS-018](#t-bus-018) | target | Apple PCIe/DART platform |
| [T-BUS-021](#t-bus-021) | target | AMD-Vi IOMMU |
| [T-BUS-022](#t-bus-022) | target | ARM SMMU |
| [T-BUS-023](#t-bus-023) | target | Apple DART IOMMU |
| [T-BUS-025](#t-bus-025) | target | OpenCores I2C controller |
| [T-BLK-011](#t-blk-011) | target | SDHCI |
| [T-BLK-012](#t-blk-012) | target | SD memory card |
| [T-BLK-013](#t-blk-013) | target | eMMC |
| [T-BLK-014](#t-blk-014) | target | Raspberry Pi SD/mailbox |
| [T-BLK-019](#t-blk-019) | target | network block provider |
| [T-BLK-023](#t-blk-023) | target | storage multipath/provider failover |
| [T-BLK-025](#t-blk-025) | target | Apple ANS/NVMe controller |
| [T-USB-009](#t-usb-009) | target | CDC-ACM serial |
| [T-USB-012](#t-usb-012) | target | USB Bluetooth HCI |
| [T-USB-014](#t-usb-014) | target | USB smart-card/security token |
| [T-INPUT-007](#t-input-007) | target | USB touch/tablet/stylus |
| [T-INPUT-011](#t-input-011) | target | ARM PL050 keyboard/mouse |
| [T-INPUT-012](#t-input-012) | target | VMware absolute mouse |
| [T-INPUT-013](#t-input-013) | target | Bluetooth HID |
| [T-GPU-008](#t-gpu-008) | target | virtio-gpu virgl/3D |
| [T-GPU-009](#t-gpu-009) | target | VMware SVGA/VMSVGA |
| [T-GPU-013](#t-gpu-013) | target | older Intel display generations |
| [T-GPU-014](#t-gpu-014) | target | AMD display/GPU provider |
| [T-GPU-015](#t-gpu-015) | target | NVIDIA display/GPU provider |
| [T-GPU-016](#t-gpu-016) | target | Raspberry Pi framebuffer/display |
| [T-GPU-020](#t-gpu-020) | target | color management/HDR provider |
| [T-GPU-021](#t-gpu-021) | target | hardware video-codec provider |
| [T-GPU-022](#t-gpu-022) | target | FPGA/custom accelerator provider |
| [T-GPU-023](#t-gpu-023) | target | Apple AGX compute/display research provider |
| [T-GPU-024](#t-gpu-024) | target | SPI/ILI9341 display |
| [T-NIC-010](#t-nic-010) | target | Cadence GEM |
| [T-RADIO-001](#t-radio-001) | target | Intel AX201-class Wi-Fi |
| [T-RADIO-002](#t-radio-002) | target | Broadcom Wi-Fi |
| [T-RADIO-003](#t-radio-003) | target | generic 802.11 service/provider ABI |
| [T-RADIO-004](#t-radio-004) | target | Bluetooth HCI/core |
| [T-RADIO-005](#t-radio-005) | target | cellular modem |
| [T-PWR-001](#t-pwr-001) | target | ACPI power button/reboot/shutdown |
| [T-PWR-002](#t-pwr-002) | target | ACPI battery/AC adapter |
| [T-PWR-003](#t-pwr-003) | target | ACPI embedded controller |
| [T-PWR-004](#t-pwr-004) | target | thermal zones/fans |
| [T-PWR-005](#t-pwr-005) | target | display/backlight |
| [T-PWR-006](#t-pwr-006) | target | suspend/resume/wake |
| [T-PWR-007](#t-pwr-007) | target | Raspberry Pi watchdog/fan/PWM/clocks |
| [T-SENSOR-001](#t-sensor-001) | target | ambient light |
| [T-SENSOR-002](#t-sensor-002) | target | accelerometer/gyroscope |
| [T-SENSOR-003](#t-sensor-003) | target | GPS/location |
| [T-SENSOR-004](#t-sensor-004) | target | generic sensor fusion service |
| [T-PERIPH-003](#t-periph-003) | target | keyboard backlight/LED |
| [T-VM-003](#t-vm-003) | target | virtio balloon |
| [T-VM-010](#t-vm-010) | target | Linux Driver VM transport |
| [T-VM-011](#t-vm-011) | target | headless/server hardware profile |
| [T-FSP-014](#t-fsp-014) | target | network filesystem |
| [T-SVC-024](#t-svc-024) | target | Power Manager |
| [T-SVC-025](#t-svc-025) | target | Battery/Thermal Manager |
| [T-SVC-027](#t-svc-027) | target | Hardware Qualification Service |
| [T-SVC-028](#t-svc-028) | target | Firmware/Microcode Update Service |
| [T-SVC-057](#t-svc-057) | target | Wi-Fi Service |
| [T-SVC-058](#t-svc-058) | target | Bluetooth Service |
| [T-SVC-064](#t-svc-064) | target | VPN/Tunnel Service |
| [T-SVC-113](#t-svc-113) | target | Location Portal |
| [T-APP-106](#t-app-106) | target | Hardware Test Center |
| [T-FUT-001](#t-fut-001) | target | x86-64 laptop |
| [T-FUT-002](#t-fut-002) | target | x86-64 desktop/workstation |
| [T-FUT-003](#t-fut-003) | target | QEMU development VM |
| [T-FUT-004](#t-fut-004) | target | headless/server |
| [T-FUT-005](#t-fut-005) | target | AArch64 generic VM |
| [T-FUT-006](#t-fut-006) | target | Raspberry Pi |
| [T-FUT-007](#t-fut-007) | target | Apple Silicon research |
| [T-FUT-008](#t-fut-008) | target | RISC-V64 generic VM/board |
| [T-FUT-009](#t-fut-009) | target | tablet/touch product |
| [T-FUT-010](#t-fut-010) | target | secure/managed profile |
| [T-FUT-011](#t-fut-011) | target | heterogeneous compute |
| [T-FUT-012](#t-fut-012) | target | custom FPGA accelerator |

<a id="f-fh-001"></a>
## F-FH-001 — laptop power/suspend

**Original requirement:** ACPI sleep/wake, device quiesce/resume, clock/network/session and failure recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-001.01 — Reconcile existing laptop power/suspend**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for laptop power/suspend. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ACPI sleep/wake, device quiesce/resume, clock/network/session and failure recovery
- [ ] **F-FH-001.02 — Freeze the exact contract for laptop power/suspend**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ACPI sleep/wake, device quiesce/resume, clock/network/session and failure recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-001.03 — Implement/prove: ACPI sleep/wake**
  - Action: For laptop power/suspend, implement or reuse and verify this exact obligation: ACPI sleep/wake. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ACPI sleep/wake; retain observable state/resource expectations.
- [ ] **F-FH-001.04 — Implement/prove: device quiesce/resume**
  - Action: For laptop power/suspend, implement or reuse and verify this exact obligation: device quiesce/resume. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device quiesce/resume; retain observable state/resource expectations.
- [ ] **F-FH-001.05 — Implement/prove: clock/network/session and failure recovery**
  - Action: For laptop power/suspend, implement or reuse and verify this exact obligation: clock/network/session and failure recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clock/network/session and failure recovery; retain observable state/resource expectations.
- [ ] **F-FH-001.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to laptop power/suspend: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-001.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-001.07 — Integrate into the real consumer and runtime route**
  - Action: Wire laptop power/suspend into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-001.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-001.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for laptop power/suspend as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-001.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-002"></a>
## F-FH-002 — lid/buttons/hotkeys

**Original requirement:** platform events, policy, debounce and accessible configuration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-002.01 — Reconcile existing lid/buttons/hotkeys**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for lid/buttons/hotkeys. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: platform events, policy, debounce and accessible configuration
- [ ] **F-FH-002.02 — Freeze the exact contract for lid/buttons/hotkeys**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: platform events, policy, debounce and accessible configuration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-002.03 — Implement/prove: platform events**
  - Action: For lid/buttons/hotkeys, implement or reuse and verify this exact obligation: platform events. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for platform events; retain observable state/resource expectations.
- [ ] **F-FH-002.04 — Implement/prove: policy**
  - Action: For lid/buttons/hotkeys, implement or reuse and verify this exact obligation: policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for policy; retain observable state/resource expectations.
- [ ] **F-FH-002.05 — Implement/prove: debounce and accessible configuration**
  - Action: For lid/buttons/hotkeys, implement or reuse and verify this exact obligation: debounce and accessible configuration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for debounce and accessible configuration; retain observable state/resource expectations.
- [ ] **F-FH-002.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to lid/buttons/hotkeys: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-002.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-002.07 — Integrate into the real consumer and runtime route**
  - Action: Wire lid/buttons/hotkeys into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-002.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-002.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for lid/buttons/hotkeys as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-002.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-003"></a>
## F-FH-003 — brightness/backlight

**Original requirement:** provider capabilities, smooth policy, keys/settings and persistence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-003.01 — Reconcile existing brightness/backlight**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for brightness/backlight. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider capabilities, smooth policy, keys/settings and persistence
- [ ] **F-FH-003.02 — Freeze the exact contract for brightness/backlight**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider capabilities, smooth policy, keys/settings and persistence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-003.03 — Implement/prove: provider capabilities**
  - Action: For brightness/backlight, implement or reuse and verify this exact obligation: provider capabilities. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider capabilities; retain observable state/resource expectations.
- [ ] **F-FH-003.04 — Implement/prove: smooth policy**
  - Action: For brightness/backlight, implement or reuse and verify this exact obligation: smooth policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for smooth policy; retain observable state/resource expectations.
- [ ] **F-FH-003.05 — Implement/prove: keys/settings and persistence**
  - Action: For brightness/backlight, implement or reuse and verify this exact obligation: keys/settings and persistence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keys/settings and persistence; retain observable state/resource expectations.
- [ ] **F-FH-003.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to brightness/backlight: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-003.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-003.07 — Integrate into the real consumer and runtime route**
  - Action: Wire brightness/backlight into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-003.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-003.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for brightness/backlight as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-003.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-004"></a>
## F-FH-004 — external monitor/dock

**Original requirement:** hotplug topology, mode/scale/audio/input/network and safe detach

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-004.01 — Reconcile existing external monitor/dock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for external monitor/dock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hotplug topology, mode/scale/audio/input/network and safe detach
- [ ] **F-FH-004.02 — Freeze the exact contract for external monitor/dock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hotplug topology, mode/scale/audio/input/network and safe detach. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-004.03 — Implement/prove: hotplug topology**
  - Action: For external monitor/dock, implement or reuse and verify this exact obligation: hotplug topology. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug topology; retain observable state/resource expectations.
- [ ] **F-FH-004.04 — Implement/prove: mode/scale/audio/input/network and safe detach**
  - Action: For external monitor/dock, implement or reuse and verify this exact obligation: mode/scale/audio/input/network and safe detach. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mode/scale/audio/input/network and safe detach; retain observable state/resource expectations.
- [ ] **F-FH-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to external monitor/dock: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire external monitor/dock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for external monitor/dock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-005"></a>
## F-FH-005 — HiDPI/4K

**Original requirement:** per-display scale, asset/text/layout quality and performance/memory budgets

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-005.01 — Reconcile existing HiDPI/4K**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for HiDPI/4K. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-display scale, asset/text/layout quality and performance/memory budgets
- [ ] **F-FH-005.02 — Freeze the exact contract for HiDPI/4K**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-display scale, asset/text/layout quality and performance/memory budgets. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-005.03 — Implement/prove: per-display scale**
  - Action: For HiDPI/4K, implement or reuse and verify this exact obligation: per-display scale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-display scale; retain observable state/resource expectations.
- [ ] **F-FH-005.04 — Implement/prove: asset/text/layout quality and performance/memory budgets**
  - Action: For HiDPI/4K, implement or reuse and verify this exact obligation: asset/text/layout quality and performance/memory budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for asset/text/layout quality and performance/memory budgets; retain observable state/resource expectations.
- [ ] **F-FH-005.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to HiDPI/4K: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-005.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-005.06 — Integrate into the real consumer and runtime route**
  - Action: Wire HiDPI/4K into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-005.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-005.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for HiDPI/4K as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-005.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-006"></a>
## F-FH-006 — variable/high refresh

**Original requirement:** negotiated deadlines, pacing, input latency and power policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-006.01 — Reconcile existing variable/high refresh**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for variable/high refresh. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: negotiated deadlines, pacing, input latency and power policy
- [ ] **F-FH-006.02 — Freeze the exact contract for variable/high refresh**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: negotiated deadlines, pacing, input latency and power policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-006.03 — Implement/prove: negotiated deadlines**
  - Action: For variable/high refresh, implement or reuse and verify this exact obligation: negotiated deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for negotiated deadlines; retain observable state/resource expectations.
- [ ] **F-FH-006.04 — Implement/prove: pacing**
  - Action: For variable/high refresh, implement or reuse and verify this exact obligation: pacing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pacing; retain observable state/resource expectations.
- [ ] **F-FH-006.05 — Implement/prove: input latency and power policy**
  - Action: For variable/high refresh, implement or reuse and verify this exact obligation: input latency and power policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input latency and power policy; retain observable state/resource expectations.
- [ ] **F-FH-006.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to variable/high refresh: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-006.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-006.07 — Integrate into the real consumer and runtime route**
  - Action: Wire variable/high refresh into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-006.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-006.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for variable/high refresh as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-006.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-007"></a>
## F-FH-007 — touchscreen convertible

**Original requirement:** orientation, touch/pen, keyboard transition, layout and gestures

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-007.01 — Reconcile existing touchscreen convertible**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for touchscreen convertible. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: orientation, touch/pen, keyboard transition, layout and gestures
- [ ] **F-FH-007.02 — Freeze the exact contract for touchscreen convertible**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: orientation, touch/pen, keyboard transition, layout and gestures. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-007.03 — Implement/prove: orientation**
  - Action: For touchscreen convertible, implement or reuse and verify this exact obligation: orientation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for orientation; retain observable state/resource expectations.
- [ ] **F-FH-007.04 — Implement/prove: touch/pen**
  - Action: For touchscreen convertible, implement or reuse and verify this exact obligation: touch/pen. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for touch/pen; retain observable state/resource expectations.
- [ ] **F-FH-007.05 — Implement/prove: keyboard transition**
  - Action: For touchscreen convertible, implement or reuse and verify this exact obligation: keyboard transition. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard transition; retain observable state/resource expectations.
- [ ] **F-FH-007.06 — Implement/prove: layout and gestures**
  - Action: For touchscreen convertible, implement or reuse and verify this exact obligation: layout and gestures. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for layout and gestures; retain observable state/resource expectations.
- [ ] **F-FH-007.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to touchscreen convertible: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-007.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-007.08 — Integrate into the real consumer and runtime route**
  - Action: Wire touchscreen convertible into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-007.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-007.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for touchscreen convertible as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-007.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-008"></a>
## F-FH-008 — camera/microphone hardware

**Original requirement:** privacy-first providers, indicators, permissions, apps and diagnostics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-008.01 — Reconcile existing camera/microphone hardware**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for camera/microphone hardware. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: privacy-first providers, indicators, permissions, apps and diagnostics
- [ ] **F-FH-008.02 — Freeze the exact contract for camera/microphone hardware**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: privacy-first providers, indicators, permissions, apps and diagnostics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-008.03 — Implement/prove: privacy-first providers**
  - Action: For camera/microphone hardware, implement or reuse and verify this exact obligation: privacy-first providers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy-first providers; retain observable state/resource expectations.
- [ ] **F-FH-008.04 — Implement/prove: indicators**
  - Action: For camera/microphone hardware, implement or reuse and verify this exact obligation: indicators. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for indicators; retain observable state/resource expectations.
- [ ] **F-FH-008.05 — Implement/prove: permissions**
  - Action: For camera/microphone hardware, implement or reuse and verify this exact obligation: permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissions; retain observable state/resource expectations.
- [ ] **F-FH-008.06 — Implement/prove: apps and diagnostics**
  - Action: For camera/microphone hardware, implement or reuse and verify this exact obligation: apps and diagnostics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for apps and diagnostics; retain observable state/resource expectations.
- [ ] **F-FH-008.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to camera/microphone hardware: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-008.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-008.08 — Integrate into the real consumer and runtime route**
  - Action: Wire camera/microphone hardware into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-008.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-008.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for camera/microphone hardware as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-008.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-009"></a>
## F-FH-009 — wireless headset

**Original requirement:** Bluetooth audio/control, codec/latency/battery, handoff and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-009.01 — Reconcile existing wireless headset**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for wireless headset. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: Bluetooth audio/control, codec/latency/battery, handoff and recovery
- [ ] **F-FH-009.02 — Freeze the exact contract for wireless headset**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: Bluetooth audio/control, codec/latency/battery, handoff and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-009.03 — Implement/prove: Bluetooth audio/control**
  - Action: For wireless headset, implement or reuse and verify this exact obligation: Bluetooth audio/control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for Bluetooth audio/control; retain observable state/resource expectations.
- [ ] **F-FH-009.04 — Implement/prove: codec/latency/battery**
  - Action: For wireless headset, implement or reuse and verify this exact obligation: codec/latency/battery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for codec/latency/battery; retain observable state/resource expectations.
- [ ] **F-FH-009.05 — Implement/prove: handoff and recovery**
  - Action: For wireless headset, implement or reuse and verify this exact obligation: handoff and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for handoff and recovery; retain observable state/resource expectations.
- [ ] **F-FH-009.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to wireless headset: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-009.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-009.07 — Integrate into the real consumer and runtime route**
  - Action: Wire wireless headset into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-009.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-009.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for wireless headset as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-009.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-010"></a>
## F-FH-010 — printer/scanner hardware

**Original requirement:** discovery/provider/spool/scan lifecycle, drivers, UI and error proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-010.01 — Reconcile existing printer/scanner hardware**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for printer/scanner hardware. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discovery/provider/spool/scan lifecycle, drivers, UI and error proof
- [ ] **F-FH-010.02 — Freeze the exact contract for printer/scanner hardware**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discovery/provider/spool/scan lifecycle, drivers, UI and error proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-010.03 — Implement/prove: discovery/provider/spool/scan lifecycle**
  - Action: For printer/scanner hardware, implement or reuse and verify this exact obligation: discovery/provider/spool/scan lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discovery/provider/spool/scan lifecycle; retain observable state/resource expectations.
- [ ] **F-FH-010.04 — Implement/prove: drivers**
  - Action: For printer/scanner hardware, implement or reuse and verify this exact obligation: drivers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drivers; retain observable state/resource expectations.
- [ ] **F-FH-010.05 — Implement/prove: UI and error proof**
  - Action: For printer/scanner hardware, implement or reuse and verify this exact obligation: UI and error proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for UI and error proof; retain observable state/resource expectations.
- [ ] **F-FH-010.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to printer/scanner hardware: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-010.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-010.07 — Integrate into the real consumer and runtime route**
  - Action: Wire printer/scanner hardware into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-010.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-010.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for printer/scanner hardware as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-010.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-011"></a>
## F-FH-011 — removable SD media

**Original requirement:** hotplug/storage/mount/eject/recovery and device policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-011.01 — Reconcile existing removable SD media**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for removable SD media. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hotplug/storage/mount/eject/recovery and device policy
- [ ] **F-FH-011.02 — Freeze the exact contract for removable SD media**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hotplug/storage/mount/eject/recovery and device policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-011.03 — Implement/prove: hotplug/storage/mount/eject/recovery and device policy**
  - Action: For removable SD media, implement or reuse and verify this exact obligation: hotplug/storage/mount/eject/recovery and device policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug/storage/mount/eject/recovery and device policy; retain observable state/resource expectations.
- [ ] **F-FH-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to removable SD media: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire removable SD media into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for removable SD media as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-012"></a>
## F-FH-012 — biometric authentication

**Original requirement:** protected template/device, fallback, consent, liveness claims and revocation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-012.01 — Reconcile existing biometric authentication**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for biometric authentication. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: protected template/device, fallback, consent, liveness claims and revocation
- [ ] **F-FH-012.02 — Freeze the exact contract for biometric authentication**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: protected template/device, fallback, consent, liveness claims and revocation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-012.03 — Implement/prove: protected template/device**
  - Action: For biometric authentication, implement or reuse and verify this exact obligation: protected template/device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for protected template/device; retain observable state/resource expectations.
- [ ] **F-FH-012.04 — Implement/prove: fallback**
  - Action: For biometric authentication, implement or reuse and verify this exact obligation: fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback; retain observable state/resource expectations.
- [ ] **F-FH-012.05 — Implement/prove: consent**
  - Action: For biometric authentication, implement or reuse and verify this exact obligation: consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consent; retain observable state/resource expectations.
- [ ] **F-FH-012.06 — Implement/prove: liveness claims and revocation**
  - Action: For biometric authentication, implement or reuse and verify this exact obligation: liveness claims and revocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for liveness claims and revocation; retain observable state/resource expectations.
- [ ] **F-FH-012.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to biometric authentication: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-012.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-012.08 — Integrate into the real consumer and runtime route**
  - Action: Wire biometric authentication into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-012.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-012.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for biometric authentication as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-012.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-013"></a>
## F-FH-013 — secure boot/measured boot

**Original requirement:** key policy, measurement log, recovery and user-owned trust roots

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-013.01 — Reconcile existing secure boot/measured boot**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for secure boot/measured boot. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: key policy, measurement log, recovery and user-owned trust roots
- [ ] **F-FH-013.02 — Freeze the exact contract for secure boot/measured boot**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: key policy, measurement log, recovery and user-owned trust roots. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-013.03 — Implement/prove: key policy**
  - Action: For secure boot/measured boot, implement or reuse and verify this exact obligation: key policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for key policy; retain observable state/resource expectations.
- [ ] **F-FH-013.04 — Implement/prove: measurement log**
  - Action: For secure boot/measured boot, implement or reuse and verify this exact obligation: measurement log. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for measurement log; retain observable state/resource expectations.
- [ ] **F-FH-013.05 — Implement/prove: recovery and user-owned trust roots**
  - Action: For secure boot/measured boot, implement or reuse and verify this exact obligation: recovery and user-owned trust roots. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery and user-owned trust roots; retain observable state/resource expectations.
- [ ] **F-FH-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to secure boot/measured boot: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire secure boot/measured boot into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for secure boot/measured boot as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-014"></a>
## F-FH-014 — smart-glasses display

**Original requirement:** low-latency HUD composition, power, privacy, glanceable UI and remote companion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-014.01 — Reconcile existing smart-glasses display**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for smart-glasses display. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: low-latency HUD composition, power, privacy, glanceable UI and remote companion
- [ ] **F-FH-014.02 — Freeze the exact contract for smart-glasses display**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: low-latency HUD composition, power, privacy, glanceable UI and remote companion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-014.03 — Implement/prove: low-latency HUD composition**
  - Action: For smart-glasses display, implement or reuse and verify this exact obligation: low-latency HUD composition. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for low-latency HUD composition; retain observable state/resource expectations.
- [ ] **F-FH-014.04 — Implement/prove: power**
  - Action: For smart-glasses display, implement or reuse and verify this exact obligation: power. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for power; retain observable state/resource expectations.
- [ ] **F-FH-014.05 — Implement/prove: privacy**
  - Action: For smart-glasses display, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-FH-014.06 — Implement/prove: glanceable UI and remote companion**
  - Action: For smart-glasses display, implement or reuse and verify this exact obligation: glanceable UI and remote companion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for glanceable UI and remote companion; retain observable state/resource expectations.
- [ ] **F-FH-014.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to smart-glasses display: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-014.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-014.08 — Integrate into the real consumer and runtime route**
  - Action: Wire smart-glasses display into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-014.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-014.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for smart-glasses display as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-014.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-015"></a>
## F-FH-015 — sensor framework

**Original requirement:** accelerometer/gyro/GPS/ambient sensors, units/rate/permissions and fusion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-015.01 — Reconcile existing sensor framework**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sensor framework. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: accelerometer/gyro/GPS/ambient sensors, units/rate/permissions and fusion
- [ ] **F-FH-015.02 — Freeze the exact contract for sensor framework**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: accelerometer/gyro/GPS/ambient sensors, units/rate/permissions and fusion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-015.03 — Implement/prove: accelerometer/gyro/GPS/ambient sensors**
  - Action: For sensor framework, implement or reuse and verify this exact obligation: accelerometer/gyro/GPS/ambient sensors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accelerometer/gyro/GPS/ambient sensors; retain observable state/resource expectations.
- [ ] **F-FH-015.04 — Implement/prove: units/rate/permissions and fusion**
  - Action: For sensor framework, implement or reuse and verify this exact obligation: units/rate/permissions and fusion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for units/rate/permissions and fusion; retain observable state/resource expectations.
- [ ] **F-FH-015.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sensor framework: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-015.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-015.06 — Integrate into the real consumer and runtime route**
  - Action: Wire sensor framework into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-015.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-015.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sensor framework as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-015.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-016"></a>
## F-FH-016 — cellular modem

**Original requirement:** modem lifecycle, SIM/eSIM, data/SMS/calls, privacy and carrier state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-016.01 — Reconcile existing cellular modem**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cellular modem. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: modem lifecycle, SIM/eSIM, data/SMS/calls, privacy and carrier state
- [ ] **F-FH-016.02 — Freeze the exact contract for cellular modem**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: modem lifecycle, SIM/eSIM, data/SMS/calls, privacy and carrier state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-016.03 — Implement/prove: modem lifecycle**
  - Action: For cellular modem, implement or reuse and verify this exact obligation: modem lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modem lifecycle; retain observable state/resource expectations.
- [ ] **F-FH-016.04 — Implement/prove: SIM/eSIM**
  - Action: For cellular modem, implement or reuse and verify this exact obligation: SIM/eSIM. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for SIM/eSIM; retain observable state/resource expectations.
- [ ] **F-FH-016.05 — Implement/prove: data/SMS/calls**
  - Action: For cellular modem, implement or reuse and verify this exact obligation: data/SMS/calls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for data/SMS/calls; retain observable state/resource expectations.
- [ ] **F-FH-016.06 — Implement/prove: privacy and carrier state**
  - Action: For cellular modem, implement or reuse and verify this exact obligation: privacy and carrier state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-016.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and carrier state; retain observable state/resource expectations.
- [ ] **F-FH-016.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cellular modem: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-016.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-016.08 — Integrate into the real consumer and runtime route**
  - Action: Wire cellular modem into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-016.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-016.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cellular modem as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-016.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-017"></a>
## F-FH-017 — heterogeneous CPU/GPU

**Original requirement:** topology/capabilities/scheduling/memory/coherency and backend selection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-017.01 — Reconcile existing heterogeneous CPU/GPU**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for heterogeneous CPU/GPU. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: topology/capabilities/scheduling/memory/coherency and backend selection
- [ ] **F-FH-017.02 — Freeze the exact contract for heterogeneous CPU/GPU**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: topology/capabilities/scheduling/memory/coherency and backend selection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-017.03 — Implement/prove: topology/capabilities/scheduling/memory/coherency and backend selection**
  - Action: For heterogeneous CPU/GPU, implement or reuse and verify this exact obligation: topology/capabilities/scheduling/memory/coherency and backend selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for topology/capabilities/scheduling/memory/coherency and backend selection; retain observable state/resource expectations.
- [ ] **F-FH-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to heterogeneous CPU/GPU: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire heterogeneous CPU/GPU into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for heterogeneous CPU/GPU as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-018"></a>
## F-FH-018 — hardware video codec

**Original requirement:** negotiated formats/buffers/fences/reset and software fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-018.01 — Reconcile existing hardware video codec**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hardware video codec. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: negotiated formats/buffers/fences/reset and software fallback
- [ ] **F-FH-018.02 — Freeze the exact contract for hardware video codec**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: negotiated formats/buffers/fences/reset and software fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-018.03 — Implement/prove: negotiated formats/buffers/fences/reset and software fallback**
  - Action: For hardware video codec, implement or reuse and verify this exact obligation: negotiated formats/buffers/fences/reset and software fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for negotiated formats/buffers/fences/reset and software fallback; retain observable state/resource expectations.
- [ ] **F-FH-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hardware video codec: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire hardware video codec into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hardware video codec as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-019"></a>
## F-FH-019 — FPGA/custom accelerator

**Original requirement:** versioned capability/stream protocol, isolation, simulation and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-019.01 — Reconcile existing FPGA/custom accelerator**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for FPGA/custom accelerator. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned capability/stream protocol, isolation, simulation and recovery
- [ ] **F-FH-019.02 — Freeze the exact contract for FPGA/custom accelerator**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned capability/stream protocol, isolation, simulation and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-019.03 — Implement/prove: versioned capability/stream protocol**
  - Action: For FPGA/custom accelerator, implement or reuse and verify this exact obligation: versioned capability/stream protocol. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned capability/stream protocol; retain observable state/resource expectations.
- [ ] **F-FH-019.04 — Implement/prove: isolation**
  - Action: For FPGA/custom accelerator, implement or reuse and verify this exact obligation: isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolation; retain observable state/resource expectations.
- [ ] **F-FH-019.05 — Implement/prove: simulation and recovery**
  - Action: For FPGA/custom accelerator, implement or reuse and verify this exact obligation: simulation and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for simulation and recovery; retain observable state/resource expectations.
- [ ] **F-FH-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to FPGA/custom accelerator: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire FPGA/custom accelerator into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for FPGA/custom accelerator as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-fh-020"></a>
## F-FH-020 — headless/server profile

**Original requirement:** serial/network management, no-display services, resource/security and update policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-FH-020.01 — Reconcile existing headless/server profile**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for headless/server profile. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-and-evidence comparison against the complete requirement: serial/network management, no-display services, resource/security and update policy
- [ ] **F-FH-020.02 — Freeze the exact contract for headless/server profile**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: serial/network management, no-display services, resource/security and update policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-FH-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-FH-020.03 — Implement/prove: serial/network management**
  - Action: For headless/server profile, implement or reuse and verify this exact obligation: serial/network management. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for serial/network management; retain observable state/resource expectations.
- [ ] **F-FH-020.04 — Implement/prove: no-display services**
  - Action: For headless/server profile, implement or reuse and verify this exact obligation: no-display services. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no-display services; retain observable state/resource expectations.
- [ ] **F-FH-020.05 — Implement/prove: resource/security and update policy**
  - Action: For headless/server profile, implement or reuse and verify this exact obligation: resource/security and update policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-FH-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resource/security and update policy; retain observable state/resource expectations.
- [ ] **F-FH-020.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to headless/server profile: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-FH-020.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-FH-020.07 — Integrate into the real consumer and runtime route**
  - Action: Wire headless/server profile into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-FH-020.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-FH-020.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for headless/server profile as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-FH-020.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p8-3"></a>
## C-P8.3 — additional storage/NIC/audio providers

**Original requirement:** additional storage/NIC/audio providers

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 447.

### Preserved original contract

- **Dependencies/current/provenance:** P8.1 and block/net/audio contracts; Astral/banan/Serenity; reject breadth before recovery.
- **I/O and state:** AHCI/VirtIO/E1000/I219/HDA resources in; standard provider handles out.
- **Invariants/failure:** each provider passes generic contract plus controller-specific reset/flush/IRQ/DMA rules; unsupported hardware refuses.
- **Deterministic proof:** register/descriptor model, queue wrap, forced timeout/reset, DMA bounds, repeated lifecycle, concurrency.
- **Target proof:** QEMU models; corresponding physical device transcript with carrier/audio/storage effect.
- **Receipt/removal:** generic and device-specific receipts; existing provider fallback retained; no provider removed for coverage vanity.

### Execution steps

- [ ] **C-P8.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P8.3.02 — Resolve this contract's exact dependencies**
  - Action: P8.1 and block/net/audio contracts; Astral/banan/Serenity; reject breadth before recovery. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P8.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P8.3.03 — I/O and state — additional storage/NIC/audio providers**
  - Action: AHCI/VirtIO/E1000/I219/HDA resources in; standard provider handles out.
  - Requires: C-P8.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P8.3.
- [ ] **C-P8.3.04 — Invariants/failure — additional storage/NIC/audio providers**
  - Action: each provider passes generic contract plus controller-specific reset/flush/IRQ/DMA rules; unsupported hardware refuses.
  - Requires: C-P8.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P8.3.
- [ ] **C-P8.3.05 — Deterministic proof — additional storage/NIC/audio providers**
  - Action: register/descriptor model, queue wrap, forced timeout/reset, DMA bounds, repeated lifecycle, concurrency.
  - Requires: C-P8.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P8.3.
- [ ] **C-P8.3.06 — Target proof — additional storage/NIC/audio providers**
  - Action: QEMU models; corresponding physical device transcript with carrier/audio/storage effect.
  - Requires: C-P8.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P8.3.
- [ ] **C-P8.3.07 — Receipt/removal — additional storage/NIC/audio providers**
  - Action: generic and device-specific receipts; existing provider fallback retained; no provider removed for coverage vanity.
  - Requires: C-P8.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P8.3.
- [ ] **C-P8.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P8.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P8.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p12-5"></a>
## C-P12.5 — secondary architecture evidence lane

**Original requirement:** secondary architecture evidence lane

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 670.

### Preserved original contract

- **Dependencies/current/provenance:** stable kernel/service/ABI contracts and P12.4; Serenity multiarchitecture build/QEMU-boot CI with x86-64-only target tests, Zinnia honesty, Nyaux/vib claim failures; reject target JSON/linker script as support.
- **I/O and state:** architecture ABI/boot/platform manifest in; separately versioned artifact and evidence lane out.
- **Invariants/failure:** no claim inheritance from x86; unsupported subsystems report precise gaps; source/build/test/runtime/hardware states independent.
- **Deterministic proof:** cross-ABI layouts/calling/endianness, compile inventory, emulator boot/process/service corpus.
- **Target proof:** emulator first, then exact physical machine if support is claimed.
- **Receipt/removal:** architecture-specific toolchain/artifact/gate/hardware receipt; x86 remains primary; no x86 path removed for parity aesthetics.

### Execution steps

- [ ] **C-P12.5.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P12.5.02 — Resolve this contract's exact dependencies**
  - Action: stable kernel/service/ABI contracts and P12.4; Serenity multiarchitecture build/QEMU-boot CI with x86-64-only target tests, Zinnia honesty, Nyaux/vib claim failures; reject target JSON/linker script as support. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P12.5.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P12.5.03 — I/O and state — secondary architecture evidence lane**
  - Action: architecture ABI/boot/platform manifest in; separately versioned artifact and evidence lane out.
  - Requires: C-P12.5.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P12.5.
- [ ] **C-P12.5.04 — Invariants/failure — secondary architecture evidence lane**
  - Action: no claim inheritance from x86; unsupported subsystems report precise gaps; source/build/test/runtime/hardware states independent.
  - Requires: C-P12.5.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P12.5.
- [ ] **C-P12.5.05 — Deterministic proof — secondary architecture evidence lane**
  - Action: cross-ABI layouts/calling/endianness, compile inventory, emulator boot/process/service corpus.
  - Requires: C-P12.5.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P12.5.
- [ ] **C-P12.5.06 — Target proof — secondary architecture evidence lane**
  - Action: emulator first, then exact physical machine if support is claimed.
  - Requires: C-P12.5.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P12.5.
- [ ] **C-P12.5.07 — Receipt/removal — secondary architecture evidence lane**
  - Action: architecture-specific toolchain/artifact/gate/hardware receipt; x86 remains primary; no x86 path removed for parity aesthetics.
  - Requires: C-P12.5.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P12.5.
- [ ] **C-P12.5.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P12.5. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P12.5.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-15"></a>
## C-DA-15 — clock, sensors and power provider

**Original requirement:** clock, sensors and power provider

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 370.

### Preserved original contract

**Depends on:** DA-03F and DA-07.

**Deliver:** monotonic/wall clocks; read-only battery, AC, temperature, fan,
brightness, lid and power-button records with units, provenance and unknown state.
Power policy is a service, not a driver decision.

**Invariants:** time never moves backward within a monotonic generation; unavailable
sensor data is not zero; shutdown/reboot requires explicit power authority; suspend
cannot be advertised until every active provider supports quiesce/resume/fallback.

**Proof:** clock wrap/drift, RTC invalid fields, absent sensor, unit mismatch,
ordinary-process shutdown denial, provider refusing suspend, resume rebind failure
and fallback boot.

### Execution steps

- [ ] **C-DA-15.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-15.02 — Resolve this contract's exact dependencies**
  - Action: DA-03F and DA-07. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-15.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-15.03 — Deliver — clock, sensors and power provider**
  - Action: monotonic/wall clocks; read-only battery, AC, temperature, fan, brightness, lid and power-button records with units, provenance and unknown state. Power policy is a service, not a driver decision.
  - Requires: C-DA-15.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-15.
- [ ] **C-DA-15.04 — Invariants — clock, sensors and power provider**
  - Action: time never moves backward within a monotonic generation; unavailable sensor data is not zero; shutdown/reboot requires explicit power authority; suspend cannot be advertised until every active provider supports quiesce/resume/fallback.
  - Requires: C-DA-15.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-15.
- [ ] **C-DA-15.05 — Proof — clock, sensors and power provider**
  - Action: clock wrap/drift, RTC invalid fields, absent sensor, unit mismatch, ordinary-process shutdown denial, provider refusing suspend, resume rebind failure and fallback boot.
  - Requires: C-DA-15.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-15.
- [ ] **C-DA-15.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-15. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-15.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-15s"></a>
## C-DA-15S — system suspend and resume orchestration

**Original requirement:** system suspend and resume orchestration

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 386.

### Preserved original contract

**Depends on:** DA-03F, DA-07, DA-15 and every active provider declaring support.

**Deliver:** system `Online -> Suspending -> Suspended -> Resuming -> Online`
states, driven by a dependency DAG. Quiesce occurs in dependency order, resume in
the inverse required order, with per-provider state save/restore, deadlines,
rollback and explicit degraded/fallback convergence.

**Invariants:** no suspend claim while any active provider lacks the transition;
new work is stopped before state capture; DMA/IRQs are quiesced; resumed devices
use new generations; stale IRQs, mappings, handles and completions cannot reach
restored or reused state; partial resume never reports ordinary success.

**Proof:** dependency cycle, refusal/timeout at every suspend and resume position,
partial-provider failure, device missing after wake, clock discontinuity, stale
IRQ/DMA/completion, fallback display/storage/input, and repeated suspend/resume
cycles under I/O and app load.

### Execution steps

- [ ] **C-DA-15S.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-15S.02 — Resolve this contract's exact dependencies**
  - Action: DA-03F, DA-07, DA-15 and every active provider declaring support. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-15S.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-15S.03 — Deliver — system suspend and resume orchestration**
  - Action: system `Online -> Suspending -> Suspended -> Resuming -> Online` states, driven by a dependency DAG. Quiesce occurs in dependency order, resume in the inverse required order, with per-provider state save/restore, deadlines, rollback and explicit degraded/fallback convergence.
  - Requires: C-DA-15S.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-15S.
- [ ] **C-DA-15S.04 — Invariants — system suspend and resume orchestration**
  - Action: no suspend claim while any active provider lacks the transition; new work is stopped before state capture; DMA/IRQs are quiesced; resumed devices use new generations; stale IRQs, mappings, handles and completions cannot reach restored or reused state; partial resume never reports ordinary success.
  - Requires: C-DA-15S.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-15S.
- [ ] **C-DA-15S.05 — Proof — system suspend and resume orchestration**
  - Action: dependency cycle, refusal/timeout at every suspend and resume position, partial-provider failure, device missing after wake, clock discontinuity, stale IRQ/DMA/completion, fallback display/storage/input, and repeated suspend/resume cycles under I/O and app load.
  - Requires: C-DA-15S.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-15S.
- [ ] **C-DA-15S.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-15S. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-15S.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-16"></a>
## C-DA-16 — later device classes

**Original requirement:** later device classes

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 405.

### Preserved original contract

**Depends on:** stable DA-05 through DA-08 and a real product need.

**Deliver:** common-contract providers for AHCI/virtio block, Wi-Fi, Bluetooth,
camera/UVC, touch/gamepad, printers, serial/GPIO/I2C/SPI and virtualization devices.

**Invariants:** none bypass the common authority, lifecycle, receipt or evidence
model; radio/camera discoverability and capture are user-visible policy; firmware
is pinned and attributed.

**Proof:** class-specific malformed corpora, hotplug, denial, reset and hardware
profiles. A source file or enumerated device remains below supported status.

### Execution steps

- [ ] **C-DA-16.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-16.02 — Resolve this contract's exact dependencies**
  - Action: stable DA-05 through DA-08 and a real product need. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-16.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-16.03 — Deliver — later device classes**
  - Action: common-contract providers for AHCI/virtio block, Wi-Fi, Bluetooth, camera/UVC, touch/gamepad, printers, serial/GPIO/I2C/SPI and virtualization devices.
  - Requires: C-DA-16.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-16.
- [ ] **C-DA-16.04 — Invariants — later device classes**
  - Action: none bypass the common authority, lifecycle, receipt or evidence model; radio/camera discoverability and capture are user-visible policy; firmware is pinned and attributed.
  - Requires: C-DA-16.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-16.
- [ ] **C-DA-16.05 — Proof — later device classes**
  - Action: class-specific malformed corpora, hotplug, denial, reset and hardware profiles. A source file or enumerated device remains below supported status.
  - Requires: C-DA-16.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-16.
- [ ] **C-DA-16.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-16. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-16.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-plat-008"></a>
## T-PLAT-008 — FDT/device-tree admission

**Original requirement:** checked nodes/ranges/phandles and version behavior

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 37.

### Execution steps

- [ ] **T-PLAT-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve FDT/device-tree admission to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-008.02 — Specify the complete target boundary**
  - Action: FDT/device-tree admission must supply: checked nodes/ranges/phandles and version behavior. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse FDT/device-tree admission through the shared platform contract, delivering every part of: checked nodes/ranges/phandles and version behavior. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for FDT/device-tree admission.
- [ ] **T-PLAT-008.05 — Qualify and retain this target's own result**
  - Action: Bind FDT/device-tree admission to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-010"></a>
## T-PLAT-010 — x86 microcode update provider

**Original requirement:** signed revision policy and per-CPU receipt

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 39.

### Execution steps

- [ ] **T-PLAT-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86 microcode update provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-010.02 — Specify the complete target boundary**
  - Action: x86 microcode update provider must supply: signed revision policy and per-CPU receipt. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86 microcode update provider through the shared platform contract, delivering every part of: signed revision policy and per-CPU receipt. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86 microcode update provider.
- [ ] **T-PLAT-010.05 — Qualify and retain this target's own result**
  - Action: Bind x86 microcode update provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-016"></a>
## T-PLAT-016 — AArch64 PSCI/EL platform

**Original requirement:** EL transition, CPU online/offline and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 45.

### Execution steps

- [ ] **T-PLAT-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AArch64 PSCI/EL platform to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-016.02 — Specify the complete target boundary**
  - Action: AArch64 PSCI/EL platform must supply: EL transition, CPU online/offline and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse AArch64 PSCI/EL platform through the shared platform contract, delivering every part of: EL transition, CPU online/offline and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-016.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AArch64 PSCI/EL platform.
- [ ] **T-PLAT-016.05 — Qualify and retain this target's own result**
  - Action: Bind AArch64 PSCI/EL platform to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-017"></a>
## T-PLAT-017 — ARM GICv2

**Original requirement:** distributor/CPU interface and lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 46.

### Execution steps

- [ ] **T-PLAT-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ARM GICv2 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-017.02 — Specify the complete target boundary**
  - Action: ARM GICv2 must supply: distributor/CPU interface and lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse ARM GICv2 through the shared platform contract, delivering every part of: distributor/CPU interface and lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-017.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ARM GICv2.
- [ ] **T-PLAT-017.05 — Qualify and retain this target's own result**
  - Action: Bind ARM GICv2 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-018"></a>
## T-PLAT-018 — ARM GICv3/ITS

**Original requirement:** redistributors, LPIs/MSI and affinity

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 47.

### Execution steps

- [ ] **T-PLAT-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ARM GICv3/ITS to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-018.02 — Specify the complete target boundary**
  - Action: ARM GICv3/ITS must supply: redistributors, LPIs/MSI and affinity. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse ARM GICv3/ITS through the shared platform contract, delivering every part of: redistributors, LPIs/MSI and affinity. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-018.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ARM GICv3/ITS.
- [ ] **T-PLAT-018.05 — Qualify and retain this target's own result**
  - Action: Bind ARM GICv3/ITS to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-019"></a>
## T-PLAT-019 — RISC-V SBI platform

**Original requirement:** versioned SBI calls and hart lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 48.

### Execution steps

- [ ] **T-PLAT-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RISC-V SBI platform to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-019.02 — Specify the complete target boundary**
  - Action: RISC-V SBI platform must supply: versioned SBI calls and hart lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse RISC-V SBI platform through the shared platform contract, delivering every part of: versioned SBI calls and hart lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-019.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RISC-V SBI platform.
- [ ] **T-PLAT-019.05 — Qualify and retain this target's own result**
  - Action: Bind RISC-V SBI platform to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-020"></a>
## T-PLAT-020 — RISC-V PLIC

**Original requirement:** source/context priority, ownership, teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 49.

### Execution steps

- [ ] **T-PLAT-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RISC-V PLIC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-020.02 — Specify the complete target boundary**
  - Action: RISC-V PLIC must supply: source/context priority, ownership, teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse RISC-V PLIC through the shared platform contract, delivering every part of: source/context priority, ownership, teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-020.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RISC-V PLIC.
- [ ] **T-PLAT-020.05 — Qualify and retain this target's own result**
  - Action: Bind RISC-V PLIC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-025"></a>
## T-PLAT-025 — ARM generic timer

**Original requirement:** monotonic deadline and suspend continuity

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 54.

### Execution steps

- [ ] **T-PLAT-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ARM generic timer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-025.02 — Specify the complete target boundary**
  - Action: ARM generic timer must supply: monotonic deadline and suspend continuity. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse ARM generic timer through the shared platform contract, delivering every part of: monotonic deadline and suspend continuity. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-025.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ARM generic timer.
- [ ] **T-PLAT-025.05 — Qualify and retain this target's own result**
  - Action: Bind ARM generic timer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-026"></a>
## T-PLAT-026 — PL031/Goldfish RTC

**Original requirement:** validated wall clock for ARM/VM profiles

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 55.

### Execution steps

- [ ] **T-PLAT-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PL031/Goldfish RTC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-026.02 — Specify the complete target boundary**
  - Action: PL031/Goldfish RTC must supply: validated wall clock for ARM/VM profiles. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse PL031/Goldfish RTC through the shared platform contract, delivering every part of: validated wall clock for ARM/VM profiles. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-026.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PL031/Goldfish RTC.
- [ ] **T-PLAT-026.05 — Qualify and retain this target's own result**
  - Action: Bind PL031/Goldfish RTC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-027"></a>
## T-PLAT-027 — RISC-V timer

**Original requirement:** deadline interrupt and wrap behavior

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 56.

### Execution steps

- [ ] **T-PLAT-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RISC-V timer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-027.02 — Specify the complete target boundary**
  - Action: RISC-V timer must supply: deadline interrupt and wrap behavior. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse RISC-V timer through the shared platform contract, delivering every part of: deadline interrupt and wrap behavior. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-027.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RISC-V timer.
- [ ] **T-PLAT-027.05 — Qualify and retain this target's own result**
  - Action: Bind RISC-V timer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-029"></a>
## T-PLAT-029 — ARM PL011 UART

**Original requirement:** early/normal console transition

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 58.

### Execution steps

- [ ] **T-PLAT-029.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ARM PL011 UART to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-029.02 — Specify the complete target boundary**
  - Action: ARM PL011 UART must supply: early/normal console transition. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-029.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-029.03 — Implement the exact target behavior**
  - Action: Implement or reuse ARM PL011 UART through the shared platform contract, delivering every part of: early/normal console transition. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-029.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-029.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-029.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ARM PL011 UART.
- [ ] **T-PLAT-029.05 — Qualify and retain this target's own result**
  - Action: Bind ARM PL011 UART to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-029.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-030"></a>
## T-PLAT-030 — Raspberry Pi mini-UART/mailbox

**Original requirement:** clock/mailbox bounds and platform identity

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 59.

### Execution steps

- [ ] **T-PLAT-030.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Raspberry Pi mini-UART/mailbox to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-030.02 — Specify the complete target boundary**
  - Action: Raspberry Pi mini-UART/mailbox must supply: clock/mailbox bounds and platform identity. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-030.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-030.03 — Implement the exact target behavior**
  - Action: Implement or reuse Raspberry Pi mini-UART/mailbox through the shared platform contract, delivering every part of: clock/mailbox bounds and platform identity. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-030.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-030.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-030.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Raspberry Pi mini-UART/mailbox.
- [ ] **T-PLAT-030.05 — Qualify and retain this target's own result**
  - Action: Bind Raspberry Pi mini-UART/mailbox to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-030.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-033"></a>
## T-PLAT-033 — TPM 2.0

**Original requirement:** measured boot, sealed keys, quote and reset policy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 62.

### Execution steps

- [ ] **T-PLAT-033.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve TPM 2.0 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-033.02 — Specify the complete target boundary**
  - Action: TPM 2.0 must supply: measured boot, sealed keys, quote and reset policy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-033.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-033.03 — Implement the exact target behavior**
  - Action: Implement or reuse TPM 2.0 through the shared platform contract, delivering every part of: measured boot, sealed keys, quote and reset policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-033.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-033.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-033.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for TPM 2.0.
- [ ] **T-PLAT-033.05 — Qualify and retain this target's own result**
  - Action: Bind TPM 2.0 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-033.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-plat-038"></a>
## T-PLAT-038 — Apple SMC

**Original requirement:** keys, sensors, power events and bounded platform access

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 67.

### Execution steps

- [ ] **T-PLAT-038.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Apple SMC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PLAT-038.02 — Specify the complete target boundary**
  - Action: Apple SMC must supply: keys, sensors, power events and bounded platform access. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PLAT-038.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PLAT-038.03 — Implement the exact target behavior**
  - Action: Implement or reuse Apple SMC through the shared platform contract, delivering every part of: keys, sensors, power events and bounded platform access. Do not fork a duplicate subsystem for this row.
  - Requires: T-PLAT-038.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PLAT-038.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: truncation; arithmetic overflow; overlapping or executable-writable segments; wrong origin; unknown required fields; firmware timeout; power loss during generation selection.
  - Requires: T-PLAT-038.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Apple SMC.
- [ ] **T-PLAT-038.05 — Qualify and retain this target's own result**
  - Action: Bind Apple SMC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PLAT-038.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-005"></a>
## T-BUS-005 — Intel VMD discovery

**Original requirement:** nested NVMe ownership and reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 77.

### Execution steps

- [ ] **T-BUS-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel VMD discovery to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-005.02 — Specify the complete target boundary**
  - Action: Intel VMD discovery must supply: nested NVMe ownership and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel VMD discovery through the shared platform contract, delivering every part of: nested NVMe ownership and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel VMD discovery.
- [ ] **T-BUS-005.05 — Qualify and retain this target's own result**
  - Action: Bind Intel VMD discovery to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-008"></a>
## T-BUS-008 — FDT platform bus

**Original requirement:** ranges/interrupts/clocks/resets/dma-ranges

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 80.

### Execution steps

- [ ] **T-BUS-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve FDT platform bus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-008.02 — Specify the complete target boundary**
  - Action: FDT platform bus must supply: ranges/interrupts/clocks/resets/dma-ranges. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse FDT platform bus through the shared platform contract, delivering every part of: ranges/interrupts/clocks/resets/dma-ranges. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for FDT platform bus.
- [ ] **T-BUS-008.05 — Qualify and retain this target's own result**
  - Action: Bind FDT platform bus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-010"></a>
## T-BUS-010 — virtio MMIO transport

**Original requirement:** version/features/IRQ/queue lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 82.

### Execution steps

- [ ] **T-BUS-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio MMIO transport to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-010.02 — Specify the complete target boundary**
  - Action: virtio MMIO transport must supply: version/features/IRQ/queue lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio MMIO transport through the shared platform contract, delivering every part of: version/features/IRQ/queue lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio MMIO transport.
- [ ] **T-BUS-010.05 — Qualify and retain this target's own result**
  - Action: Bind virtio MMIO transport to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-013"></a>
## T-BUS-013 — I2C core

**Original requirement:** controller arbitration, timeouts and ownership

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 85.

### Execution steps

- [ ] **T-BUS-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve I2C core to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-013.02 — Specify the complete target boundary**
  - Action: I2C core must supply: controller arbitration, timeouts and ownership. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse I2C core through the shared platform contract, delivering every part of: controller arbitration, timeouts and ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for I2C core.
- [ ] **T-BUS-013.05 — Qualify and retain this target's own result**
  - Action: Bind I2C core to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-014"></a>
## T-BUS-014 — Intel LPSS/DesignWare I2C

**Original requirement:** ACPI resources, reset and transfer errors

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 86.

### Execution steps

- [ ] **T-BUS-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel LPSS/DesignWare I2C to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-014.02 — Specify the complete target boundary**
  - Action: Intel LPSS/DesignWare I2C must supply: ACPI resources, reset and transfer errors. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel LPSS/DesignWare I2C through the shared platform contract, delivering every part of: ACPI resources, reset and transfer errors. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel LPSS/DesignWare I2C.
- [ ] **T-BUS-014.05 — Qualify and retain this target's own result**
  - Action: Bind Intel LPSS/DesignWare I2C to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-015"></a>
## T-BUS-015 — SPI core/controller

**Original requirement:** chip-select/mode/rate/bounds and arbitration

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 87.

### Execution steps

- [ ] **T-BUS-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SPI core/controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-015.02 — Specify the complete target boundary**
  - Action: SPI core/controller must supply: chip-select/mode/rate/bounds and arbitration. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse SPI core/controller through the shared platform contract, delivering every part of: chip-select/mode/rate/bounds and arbitration. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-015.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SPI core/controller.
- [ ] **T-BUS-015.05 — Qualify and retain this target's own result**
  - Action: Bind SPI core/controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-016"></a>
## T-BUS-016 — GPIO core/controller

**Original requirement:** pin ownership, direction, IRQ, suspend

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 88.

### Execution steps

- [ ] **T-BUS-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve GPIO core/controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-016.02 — Specify the complete target boundary**
  - Action: GPIO core/controller must supply: pin ownership, direction, IRQ, suspend. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse GPIO core/controller through the shared platform contract, delivering every part of: pin ownership, direction, IRQ, suspend. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-016.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for GPIO core/controller.
- [ ] **T-BUS-016.05 — Qualify and retain this target's own result**
  - Action: Bind GPIO core/controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-017"></a>
## T-BUS-017 — embedded-controller bus

**Original requirement:** ACPI EC deadlines, burst mode and event queue

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 89.

### Execution steps

- [ ] **T-BUS-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve embedded-controller bus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-017.02 — Specify the complete target boundary**
  - Action: embedded-controller bus must supply: ACPI EC deadlines, burst mode and event queue. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse embedded-controller bus through the shared platform contract, delivering every part of: ACPI EC deadlines, burst mode and event queue. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-017.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for embedded-controller bus.
- [ ] **T-BUS-017.05 — Qualify and retain this target's own result**
  - Action: Bind embedded-controller bus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-018"></a>
## T-BUS-018 — Apple PCIe/DART platform

**Original requirement:** m1n1 handover, resources and device isolation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 90.

### Execution steps

- [ ] **T-BUS-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Apple PCIe/DART platform to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-018.02 — Specify the complete target boundary**
  - Action: Apple PCIe/DART platform must supply: m1n1 handover, resources and device isolation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse Apple PCIe/DART platform through the shared platform contract, delivering every part of: m1n1 handover, resources and device isolation. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-018.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Apple PCIe/DART platform.
- [ ] **T-BUS-018.05 — Qualify and retain this target's own result**
  - Action: Bind Apple PCIe/DART platform to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-021"></a>
## T-BUS-021 — AMD-Vi IOMMU

**Original requirement:** per-device domains and invalidation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 93.

### Execution steps

- [ ] **T-BUS-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AMD-Vi IOMMU to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-021.02 — Specify the complete target boundary**
  - Action: AMD-Vi IOMMU must supply: per-device domains and invalidation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse AMD-Vi IOMMU through the shared platform contract, delivering every part of: per-device domains and invalidation. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-021.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AMD-Vi IOMMU.
- [ ] **T-BUS-021.05 — Qualify and retain this target's own result**
  - Action: Bind AMD-Vi IOMMU to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-022"></a>
## T-BUS-022 — ARM SMMU

**Original requirement:** streams/domains/invalidation/faults

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 94.

### Execution steps

- [ ] **T-BUS-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ARM SMMU to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-022.02 — Specify the complete target boundary**
  - Action: ARM SMMU must supply: streams/domains/invalidation/faults. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse ARM SMMU through the shared platform contract, delivering every part of: streams/domains/invalidation/faults. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-022.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ARM SMMU.
- [ ] **T-BUS-022.05 — Qualify and retain this target's own result**
  - Action: Bind ARM SMMU to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-023"></a>
## T-BUS-023 — Apple DART IOMMU

**Original requirement:** stream ownership and translation teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 95.

### Execution steps

- [ ] **T-BUS-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Apple DART IOMMU to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-023.02 — Specify the complete target boundary**
  - Action: Apple DART IOMMU must supply: stream ownership and translation teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse Apple DART IOMMU through the shared platform contract, delivering every part of: stream ownership and translation teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-023.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Apple DART IOMMU.
- [ ] **T-BUS-023.05 — Qualify and retain this target's own result**
  - Action: Bind Apple DART IOMMU to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-bus-025"></a>
## T-BUS-025 — OpenCores I2C controller

**Original requirement:** controller IRQ/transfer/reset and FDT resources

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 97.

### Execution steps

- [ ] **T-BUS-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve OpenCores I2C controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BUS-025.02 — Specify the complete target boundary**
  - Action: OpenCores I2C controller must supply: controller IRQ/transfer/reset and FDT resources. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BUS-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BUS-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse OpenCores I2C controller through the shared platform contract, delivering every part of: controller IRQ/transfer/reset and FDT resources. Do not fork a duplicate subsystem for this row.
  - Requires: T-BUS-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BUS-025.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-BUS-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for OpenCores I2C controller.
- [ ] **T-BUS-025.05 — Qualify and retain this target's own result**
  - Action: Bind OpenCores I2C controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BUS-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-011"></a>
## T-BLK-011 — SDHCI

**Original requirement:** voltage/clock/tuning/ADMA/card removal

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 116.

### Execution steps

- [ ] **T-BLK-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SDHCI to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-011.02 — Specify the complete target boundary**
  - Action: SDHCI must supply: voltage/clock/tuning/ADMA/card removal. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse SDHCI through the shared platform contract, delivering every part of: voltage/clock/tuning/ADMA/card removal. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SDHCI.
- [ ] **T-BLK-011.05 — Qualify and retain this target's own result**
  - Action: Bind SDHCI to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-012"></a>
## T-BLK-012 — SD memory card

**Original requirement:** identify/capacity/bus width/error recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 117.

### Execution steps

- [ ] **T-BLK-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SD memory card to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-012.02 — Specify the complete target boundary**
  - Action: SD memory card must supply: identify/capacity/bus width/error recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse SD memory card through the shared platform contract, delivering every part of: identify/capacity/bus width/error recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SD memory card.
- [ ] **T-BLK-012.05 — Qualify and retain this target's own result**
  - Action: Bind SD memory card to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-013"></a>
## T-BLK-013 — eMMC

**Original requirement:** partitions, reliable write and lifetime data

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 118.

### Execution steps

- [ ] **T-BLK-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve eMMC to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-013.02 — Specify the complete target boundary**
  - Action: eMMC must supply: partitions, reliable write and lifetime data. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse eMMC through the shared platform contract, delivering every part of: partitions, reliable write and lifetime data. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for eMMC.
- [ ] **T-BLK-013.05 — Qualify and retain this target's own result**
  - Action: Bind eMMC to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-014"></a>
## T-BLK-014 — Raspberry Pi SD/mailbox

**Original requirement:** platform DMA/mailbox and card lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 119.

### Execution steps

- [ ] **T-BLK-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Raspberry Pi SD/mailbox to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-014.02 — Specify the complete target boundary**
  - Action: Raspberry Pi SD/mailbox must supply: platform DMA/mailbox and card lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse Raspberry Pi SD/mailbox through the shared platform contract, delivering every part of: platform DMA/mailbox and card lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Raspberry Pi SD/mailbox.
- [ ] **T-BLK-014.05 — Qualify and retain this target's own result**
  - Action: Bind Raspberry Pi SD/mailbox to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-019"></a>
## T-BLK-019 — network block provider

**Original requirement:** authenticated loss/reconnect/flush semantics

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 124.

### Execution steps

- [ ] **T-BLK-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve network block provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-019.02 — Specify the complete target boundary**
  - Action: network block provider must supply: authenticated loss/reconnect/flush semantics. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse network block provider through the shared platform contract, delivering every part of: authenticated loss/reconnect/flush semantics. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-019.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for network block provider.
- [ ] **T-BLK-019.05 — Qualify and retain this target's own result**
  - Action: Bind network block provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-023"></a>
## T-BLK-023 — storage multipath/provider failover

**Original requirement:** identity, ordering and no split brain

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 128.

### Execution steps

- [ ] **T-BLK-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve storage multipath/provider failover to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-023.02 — Specify the complete target boundary**
  - Action: storage multipath/provider failover must supply: identity, ordering and no split brain. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse storage multipath/provider failover through the shared platform contract, delivering every part of: identity, ordering and no split brain. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-023.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for storage multipath/provider failover.
- [ ] **T-BLK-023.05 — Qualify and retain this target's own result**
  - Action: Bind storage multipath/provider failover to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-blk-025"></a>
## T-BLK-025 — Apple ANS/NVMe controller

**Original requirement:** platform queues, DMA, reset and honest stub rejection

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 130.

### Execution steps

- [ ] **T-BLK-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Apple ANS/NVMe controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-025.02 — Specify the complete target boundary**
  - Action: Apple ANS/NVMe controller must supply: platform queues, DMA, reset and honest stub rejection. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse Apple ANS/NVMe controller through the shared platform contract, delivering every part of: platform queues, DMA, reset and honest stub rejection. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-025.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Apple ANS/NVMe controller.
- [ ] **T-BLK-025.05 — Qualify and retain this target's own result**
  - Action: Bind Apple ANS/NVMe controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-009"></a>
## T-USB-009 — CDC-ACM serial

**Original requirement:** line state, endpoints, disconnect

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 144.

### Execution steps

- [ ] **T-USB-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve CDC-ACM serial to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-009.02 — Specify the complete target boundary**
  - Action: CDC-ACM serial must supply: line state, endpoints, disconnect. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse CDC-ACM serial through the shared platform contract, delivering every part of: line state, endpoints, disconnect. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for CDC-ACM serial.
- [ ] **T-USB-009.05 — Qualify and retain this target's own result**
  - Action: Bind CDC-ACM serial to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-012"></a>
## T-USB-012 — USB Bluetooth HCI

**Original requirement:** transport ownership and radio service binding

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 147.

### Execution steps

- [ ] **T-USB-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB Bluetooth HCI to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-012.02 — Specify the complete target boundary**
  - Action: USB Bluetooth HCI must supply: transport ownership and radio service binding. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB Bluetooth HCI through the shared platform contract, delivering every part of: transport ownership and radio service binding. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB Bluetooth HCI.
- [ ] **T-USB-012.05 — Qualify and retain this target's own result**
  - Action: Bind USB Bluetooth HCI to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-014"></a>
## T-USB-014 — USB smart-card/security token

**Original requirement:** APDU/token authority and removal

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 149.

### Execution steps

- [ ] **T-USB-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB smart-card/security token to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-014.02 — Specify the complete target boundary**
  - Action: USB smart-card/security token must supply: APDU/token authority and removal. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB smart-card/security token through the shared platform contract, delivering every part of: APDU/token authority and removal. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB smart-card/security token.
- [ ] **T-USB-014.05 — Qualify and retain this target's own result**
  - Action: Bind USB smart-card/security token to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-007"></a>
## T-INPUT-007 — USB touch/tablet/stylus

**Original requirement:** contacts/pressure/tilt/calibration

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 161.

### Execution steps

- [ ] **T-INPUT-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB touch/tablet/stylus to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-007.02 — Specify the complete target boundary**
  - Action: USB touch/tablet/stylus must supply: contacts/pressure/tilt/calibration. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB touch/tablet/stylus through the shared platform contract, delivering every part of: contacts/pressure/tilt/calibration. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB touch/tablet/stylus.
- [ ] **T-INPUT-007.05 — Qualify and retain this target's own result**
  - Action: Bind USB touch/tablet/stylus to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-011"></a>
## T-INPUT-011 — ARM PL050 keyboard/mouse

**Original requirement:** controller lifecycle and input binding

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 165.

### Execution steps

- [ ] **T-INPUT-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ARM PL050 keyboard/mouse to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-011.02 — Specify the complete target boundary**
  - Action: ARM PL050 keyboard/mouse must supply: controller lifecycle and input binding. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse ARM PL050 keyboard/mouse through the shared platform contract, delivering every part of: controller lifecycle and input binding. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ARM PL050 keyboard/mouse.
- [ ] **T-INPUT-011.05 — Qualify and retain this target's own result**
  - Action: Bind ARM PL050 keyboard/mouse to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-012"></a>
## T-INPUT-012 — VMware absolute mouse

**Original requirement:** guest coordinate/focus/disconnect

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 166.

### Execution steps

- [ ] **T-INPUT-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VMware absolute mouse to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-012.02 — Specify the complete target boundary**
  - Action: VMware absolute mouse must supply: guest coordinate/focus/disconnect. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse VMware absolute mouse through the shared platform contract, delivering every part of: guest coordinate/focus/disconnect. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VMware absolute mouse.
- [ ] **T-INPUT-012.05 — Qualify and retain this target's own result**
  - Action: Bind VMware absolute mouse to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-013"></a>
## T-INPUT-013 — Bluetooth HID

**Original requirement:** pairing, encryption, reconnect and revoke

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 167.

### Execution steps

- [ ] **T-INPUT-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Bluetooth HID to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-013.02 — Specify the complete target boundary**
  - Action: Bluetooth HID must supply: pairing, encryption, reconnect and revoke. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse Bluetooth HID through the shared platform contract, delivering every part of: pairing, encryption, reconnect and revoke. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Bluetooth HID.
- [ ] **T-INPUT-013.05 — Qualify and retain this target's own result**
  - Action: Bind Bluetooth HID to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-008"></a>
## T-GPU-008 — virtio-gpu virgl/3D

**Original requirement:** contexts/execbuffer/capabilities/isolation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 181.

### Execution steps

- [ ] **T-GPU-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio-gpu virgl/3D to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-008.02 — Specify the complete target boundary**
  - Action: virtio-gpu virgl/3D must supply: contexts/execbuffer/capabilities/isolation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio-gpu virgl/3D through the shared platform contract, delivering every part of: contexts/execbuffer/capabilities/isolation. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio-gpu virgl/3D.
- [ ] **T-GPU-008.05 — Qualify and retain this target's own result**
  - Action: Bind virtio-gpu virgl/3D to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-009"></a>
## T-GPU-009 — VMware SVGA/VMSVGA

**Original requirement:** modes/FIFO/cursor/fence/reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 182.

### Execution steps

- [ ] **T-GPU-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VMware SVGA/VMSVGA to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-009.02 — Specify the complete target boundary**
  - Action: VMware SVGA/VMSVGA must supply: modes/FIFO/cursor/fence/reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse VMware SVGA/VMSVGA through the shared platform contract, delivering every part of: modes/FIFO/cursor/fence/reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VMware SVGA/VMSVGA.
- [ ] **T-GPU-009.05 — Qualify and retain this target's own result**
  - Action: Bind VMware SVGA/VMSVGA to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-013"></a>
## T-GPU-013 — older Intel display generations

**Original requirement:** generation-specific register providers

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 186.

### Execution steps

- [ ] **T-GPU-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve older Intel display generations to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-013.02 — Specify the complete target boundary**
  - Action: older Intel display generations must supply: generation-specific register providers. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse older Intel display generations through the shared platform contract, delivering every part of: generation-specific register providers. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-013.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for older Intel display generations.
- [ ] **T-GPU-013.05 — Qualify and retain this target's own result**
  - Action: Bind older Intel display generations to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-014"></a>
## T-GPU-014 — AMD display/GPU provider

**Original requirement:** firmware/modesetting/queues/reset/isolation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 187.

### Execution steps

- [ ] **T-GPU-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AMD display/GPU provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-014.02 — Specify the complete target boundary**
  - Action: AMD display/GPU provider must supply: firmware/modesetting/queues/reset/isolation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse AMD display/GPU provider through the shared platform contract, delivering every part of: firmware/modesetting/queues/reset/isolation. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AMD display/GPU provider.
- [ ] **T-GPU-014.05 — Qualify and retain this target's own result**
  - Action: Bind AMD display/GPU provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-015"></a>
## T-GPU-015 — NVIDIA display/GPU provider

**Original requirement:** firmware/modesetting/queues/reset/isolation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 188.

### Execution steps

- [ ] **T-GPU-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve NVIDIA display/GPU provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-015.02 — Specify the complete target boundary**
  - Action: NVIDIA display/GPU provider must supply: firmware/modesetting/queues/reset/isolation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse NVIDIA display/GPU provider through the shared platform contract, delivering every part of: firmware/modesetting/queues/reset/isolation. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-015.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for NVIDIA display/GPU provider.
- [ ] **T-GPU-015.05 — Qualify and retain this target's own result**
  - Action: Bind NVIDIA display/GPU provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-016"></a>
## T-GPU-016 — Raspberry Pi framebuffer/display

**Original requirement:** mailbox/scanout/blanking and ownership

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 189.

### Execution steps

- [ ] **T-GPU-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Raspberry Pi framebuffer/display to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-016.02 — Specify the complete target boundary**
  - Action: Raspberry Pi framebuffer/display must supply: mailbox/scanout/blanking and ownership. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse Raspberry Pi framebuffer/display through the shared platform contract, delivering every part of: mailbox/scanout/blanking and ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-016.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Raspberry Pi framebuffer/display.
- [ ] **T-GPU-016.05 — Qualify and retain this target's own result**
  - Action: Bind Raspberry Pi framebuffer/display to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-020"></a>
## T-GPU-020 — color management/HDR provider

**Original requirement:** profiles, transfer, precision and fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 193.

### Execution steps

- [ ] **T-GPU-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve color management/HDR provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-020.02 — Specify the complete target boundary**
  - Action: color management/HDR provider must supply: profiles, transfer, precision and fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse color management/HDR provider through the shared platform contract, delivering every part of: profiles, transfer, precision and fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-020.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for color management/HDR provider.
- [ ] **T-GPU-020.05 — Qualify and retain this target's own result**
  - Action: Bind color management/HDR provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-021"></a>
## T-GPU-021 — hardware video-codec provider

**Original requirement:** formats/buffers/fences/reset/software fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 194.

### Execution steps

- [ ] **T-GPU-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve hardware video-codec provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-021.02 — Specify the complete target boundary**
  - Action: hardware video-codec provider must supply: formats/buffers/fences/reset/software fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse hardware video-codec provider through the shared platform contract, delivering every part of: formats/buffers/fences/reset/software fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-021.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for hardware video-codec provider.
- [ ] **T-GPU-021.05 — Qualify and retain this target's own result**
  - Action: Bind hardware video-codec provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-022"></a>
## T-GPU-022 — FPGA/custom accelerator provider

**Original requirement:** required/optional capabilities and simulation

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 195.

### Execution steps

- [ ] **T-GPU-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve FPGA/custom accelerator provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-022.02 — Specify the complete target boundary**
  - Action: FPGA/custom accelerator provider must supply: required/optional capabilities and simulation. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse FPGA/custom accelerator provider through the shared platform contract, delivering every part of: required/optional capabilities and simulation. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-022.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for FPGA/custom accelerator provider.
- [ ] **T-GPU-022.05 — Qualify and retain this target's own result**
  - Action: Bind FPGA/custom accelerator provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-023"></a>
## T-GPU-023 — Apple AGX compute/display research provider

**Original requirement:** firmware/queues/memory/fences/reset; no desktop claim until wired

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 196.

### Execution steps

- [ ] **T-GPU-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Apple AGX compute/display research provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-023.02 — Specify the complete target boundary**
  - Action: Apple AGX compute/display research provider must supply: firmware/queues/memory/fences/reset; no desktop claim until wired. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse Apple AGX compute/display research provider through the shared platform contract, delivering every part of: firmware/queues/memory/fences/reset; no desktop claim until wired. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-023.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Apple AGX compute/display research provider.
- [ ] **T-GPU-023.05 — Qualify and retain this target's own result**
  - Action: Bind Apple AGX compute/display research provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-024"></a>
## T-GPU-024 — SPI/ILI9341 display

**Original requirement:** panel init, pixel transfer, rotation, damage and recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 197.

### Execution steps

- [ ] **T-GPU-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve SPI/ILI9341 display to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-024.02 — Specify the complete target boundary**
  - Action: SPI/ILI9341 display must supply: panel init, pixel transfer, rotation, damage and recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse SPI/ILI9341 display through the shared platform contract, delivering every part of: panel init, pixel transfer, rotation, damage and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-024.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for SPI/ILI9341 display.
- [ ] **T-GPU-024.05 — Qualify and retain this target's own result**
  - Action: Bind SPI/ILI9341 display to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-nic-010"></a>
## T-NIC-010 — Cadence GEM

**Original requirement:** platform DMA/PHY/link

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 213.

### Execution steps

- [ ] **T-NIC-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Cadence GEM to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-NIC-010.02 — Specify the complete target boundary**
  - Action: Cadence GEM must supply: platform DMA/PHY/link. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-NIC-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-NIC-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Cadence GEM through the shared platform contract, delivering every part of: platform DMA/PHY/link. Do not fork a duplicate subsystem for this row.
  - Requires: T-NIC-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-NIC-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-NIC-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Cadence GEM.
- [ ] **T-NIC-010.05 — Qualify and retain this target's own result**
  - Action: Bind Cadence GEM to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-NIC-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-radio-001"></a>
## T-RADIO-001 — Intel AX201-class Wi-Fi

**Original requirement:** firmware, scan, association, WPA, regulatory

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 220.

### Execution steps

- [ ] **T-RADIO-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel AX201-class Wi-Fi to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-RADIO-001.02 — Specify the complete target boundary**
  - Action: Intel AX201-class Wi-Fi must supply: firmware, scan, association, WPA, regulatory. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-RADIO-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-RADIO-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel AX201-class Wi-Fi through the shared platform contract, delivering every part of: firmware, scan, association, WPA, regulatory. Do not fork a duplicate subsystem for this row.
  - Requires: T-RADIO-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-RADIO-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-RADIO-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel AX201-class Wi-Fi.
- [ ] **T-RADIO-001.05 — Qualify and retain this target's own result**
  - Action: Bind Intel AX201-class Wi-Fi to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-RADIO-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-radio-002"></a>
## T-RADIO-002 — Broadcom Wi-Fi

**Original requirement:** selected chipset/firmware and regulatory proof

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 221.

### Execution steps

- [ ] **T-RADIO-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Broadcom Wi-Fi to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-RADIO-002.02 — Specify the complete target boundary**
  - Action: Broadcom Wi-Fi must supply: selected chipset/firmware and regulatory proof. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-RADIO-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-RADIO-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse Broadcom Wi-Fi through the shared platform contract, delivering every part of: selected chipset/firmware and regulatory proof. Do not fork a duplicate subsystem for this row.
  - Requires: T-RADIO-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-RADIO-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-RADIO-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Broadcom Wi-Fi.
- [ ] **T-RADIO-002.05 — Qualify and retain this target's own result**
  - Action: Bind Broadcom Wi-Fi to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-RADIO-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-radio-003"></a>
## T-RADIO-003 — generic 802.11 service/provider ABI

**Original requirement:** scan/auth/roam/power/privacy boundary

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 222.

### Execution steps

- [ ] **T-RADIO-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve generic 802.11 service/provider ABI to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-RADIO-003.02 — Specify the complete target boundary**
  - Action: generic 802.11 service/provider ABI must supply: scan/auth/roam/power/privacy boundary. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-RADIO-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-RADIO-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse generic 802.11 service/provider ABI through the shared platform contract, delivering every part of: scan/auth/roam/power/privacy boundary. Do not fork a duplicate subsystem for this row.
  - Requires: T-RADIO-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-RADIO-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-RADIO-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for generic 802.11 service/provider ABI.
- [ ] **T-RADIO-003.05 — Qualify and retain this target's own result**
  - Action: Bind generic 802.11 service/provider ABI to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-RADIO-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-radio-004"></a>
## T-RADIO-004 — Bluetooth HCI/core

**Original requirement:** controller, keys, pairing and profiles

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 223.

### Execution steps

- [ ] **T-RADIO-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Bluetooth HCI/core to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-RADIO-004.02 — Specify the complete target boundary**
  - Action: Bluetooth HCI/core must supply: controller, keys, pairing and profiles. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-RADIO-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-RADIO-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse Bluetooth HCI/core through the shared platform contract, delivering every part of: controller, keys, pairing and profiles. Do not fork a duplicate subsystem for this row.
  - Requires: T-RADIO-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-RADIO-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-RADIO-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Bluetooth HCI/core.
- [ ] **T-RADIO-004.05 — Qualify and retain this target's own result**
  - Action: Bind Bluetooth HCI/core to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-RADIO-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-radio-005"></a>
## T-RADIO-005 — cellular modem

**Original requirement:** SIM/eSIM, data/SMS/calls and carrier lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 224.

### Execution steps

- [ ] **T-RADIO-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve cellular modem to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-RADIO-005.02 — Specify the complete target boundary**
  - Action: cellular modem must supply: SIM/eSIM, data/SMS/calls and carrier lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-RADIO-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-RADIO-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse cellular modem through the shared platform contract, delivering every part of: SIM/eSIM, data/SMS/calls and carrier lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-RADIO-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-RADIO-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: loss/reorder/duplication; malformed lengths/checksums/fragments; window exhaustion; DNS/TLS/clock failure; critical-extension/name/usage rejection; link loss; ungranted remote access.
  - Requires: T-RADIO-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for cellular modem.
- [ ] **T-RADIO-005.05 — Qualify and retain this target's own result**
  - Action: Bind cellular modem to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-RADIO-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-001"></a>
## T-PWR-001 — ACPI power button/reboot/shutdown

**Original requirement:** policy capability and final state receipt

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 238.

### Execution steps

- [ ] **T-PWR-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ACPI power button/reboot/shutdown to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-001.02 — Specify the complete target boundary**
  - Action: ACPI power button/reboot/shutdown must supply: policy capability and final state receipt. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse ACPI power button/reboot/shutdown through the shared platform contract, delivering every part of: policy capability and final state receipt. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ACPI power button/reboot/shutdown.
- [ ] **T-PWR-001.05 — Qualify and retain this target's own result**
  - Action: Bind ACPI power button/reboot/shutdown to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-002"></a>
## T-PWR-002 — ACPI battery/AC adapter

**Original requirement:** units/rates/invalid data/event lifecycle

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 239.

### Execution steps

- [ ] **T-PWR-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ACPI battery/AC adapter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-002.02 — Specify the complete target boundary**
  - Action: ACPI battery/AC adapter must supply: units/rates/invalid data/event lifecycle. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse ACPI battery/AC adapter through the shared platform contract, delivering every part of: units/rates/invalid data/event lifecycle. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ACPI battery/AC adapter.
- [ ] **T-PWR-002.05 — Qualify and retain this target's own result**
  - Action: Bind ACPI battery/AC adapter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-003"></a>
## T-PWR-003 — ACPI embedded controller

**Original requirement:** bounded transactions and event handling

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 240.

### Execution steps

- [ ] **T-PWR-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ACPI embedded controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-003.02 — Specify the complete target boundary**
  - Action: ACPI embedded controller must supply: bounded transactions and event handling. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse ACPI embedded controller through the shared platform contract, delivering every part of: bounded transactions and event handling. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ACPI embedded controller.
- [ ] **T-PWR-003.05 — Qualify and retain this target's own result**
  - Action: Bind ACPI embedded controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-004"></a>
## T-PWR-004 — thermal zones/fans

**Original requirement:** trip points, failsafe and policy service

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 241.

### Execution steps

- [ ] **T-PWR-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve thermal zones/fans to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-004.02 — Specify the complete target boundary**
  - Action: thermal zones/fans must supply: trip points, failsafe and policy service. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse thermal zones/fans through the shared platform contract, delivering every part of: trip points, failsafe and policy service. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for thermal zones/fans.
- [ ] **T-PWR-004.05 — Qualify and retain this target's own result**
  - Action: Bind thermal zones/fans to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-005"></a>
## T-PWR-005 — display/backlight

**Original requirement:** safe ranges, restore and user policy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 242.

### Execution steps

- [ ] **T-PWR-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve display/backlight to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-005.02 — Specify the complete target boundary**
  - Action: display/backlight must supply: safe ranges, restore and user policy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse display/backlight through the shared platform contract, delivering every part of: safe ranges, restore and user policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for display/backlight.
- [ ] **T-PWR-005.05 — Qualify and retain this target's own result**
  - Action: Bind display/backlight to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-006"></a>
## T-PWR-006 — suspend/resume/wake

**Original requirement:** ordered quiesce, device state and recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 243.

### Execution steps

- [ ] **T-PWR-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve suspend/resume/wake to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-006.02 — Specify the complete target boundary**
  - Action: suspend/resume/wake must supply: ordered quiesce, device state and recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse suspend/resume/wake through the shared platform contract, delivering every part of: ordered quiesce, device state and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for suspend/resume/wake.
- [ ] **T-PWR-006.05 — Qualify and retain this target's own result**
  - Action: Bind suspend/resume/wake to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-pwr-007"></a>
## T-PWR-007 — Raspberry Pi watchdog/fan/PWM/clocks

**Original requirement:** mailbox/GPIO lifetime and failsafe

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 244.

### Execution steps

- [ ] **T-PWR-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Raspberry Pi watchdog/fan/PWM/clocks to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PWR-007.02 — Specify the complete target boundary**
  - Action: Raspberry Pi watchdog/fan/PWM/clocks must supply: mailbox/GPIO lifetime and failsafe. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PWR-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PWR-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse Raspberry Pi watchdog/fan/PWM/clocks through the shared platform contract, delivering every part of: mailbox/GPIO lifetime and failsafe. Do not fork a duplicate subsystem for this row.
  - Requires: T-PWR-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PWR-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PWR-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Raspberry Pi watchdog/fan/PWM/clocks.
- [ ] **T-PWR-007.05 — Qualify and retain this target's own result**
  - Action: Bind Raspberry Pi watchdog/fan/PWM/clocks to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PWR-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-sensor-001"></a>
## T-SENSOR-001 — ambient light

**Original requirement:** units/rate/calibration/privacy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 245.

### Execution steps

- [ ] **T-SENSOR-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve ambient light to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SENSOR-001.02 — Specify the complete target boundary**
  - Action: ambient light must supply: units/rate/calibration/privacy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SENSOR-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SENSOR-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse ambient light through the shared platform contract, delivering every part of: units/rate/calibration/privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SENSOR-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SENSOR-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SENSOR-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for ambient light.
- [ ] **T-SENSOR-001.05 — Qualify and retain this target's own result**
  - Action: Bind ambient light to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SENSOR-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-sensor-002"></a>
## T-SENSOR-002 — accelerometer/gyroscope

**Original requirement:** units/rate/orientation/permission

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 246.

### Execution steps

- [ ] **T-SENSOR-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve accelerometer/gyroscope to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SENSOR-002.02 — Specify the complete target boundary**
  - Action: accelerometer/gyroscope must supply: units/rate/orientation/permission. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SENSOR-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SENSOR-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse accelerometer/gyroscope through the shared platform contract, delivering every part of: units/rate/orientation/permission. Do not fork a duplicate subsystem for this row.
  - Requires: T-SENSOR-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SENSOR-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SENSOR-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for accelerometer/gyroscope.
- [ ] **T-SENSOR-002.05 — Qualify and retain this target's own result**
  - Action: Bind accelerometer/gyroscope to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SENSOR-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-sensor-003"></a>
## T-SENSOR-003 — GPS/location

**Original requirement:** permission, accuracy, retention and revoke

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 247.

### Execution steps

- [ ] **T-SENSOR-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve GPS/location to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SENSOR-003.02 — Specify the complete target boundary**
  - Action: GPS/location must supply: permission, accuracy, retention and revoke. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SENSOR-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SENSOR-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse GPS/location through the shared platform contract, delivering every part of: permission, accuracy, retention and revoke. Do not fork a duplicate subsystem for this row.
  - Requires: T-SENSOR-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SENSOR-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SENSOR-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for GPS/location.
- [ ] **T-SENSOR-003.05 — Qualify and retain this target's own result**
  - Action: Bind GPS/location to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SENSOR-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-sensor-004"></a>
## T-SENSOR-004 — generic sensor fusion service

**Original requirement:** timestamped streams and provenance

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 248.

### Execution steps

- [ ] **T-SENSOR-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve generic sensor fusion service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SENSOR-004.02 — Specify the complete target boundary**
  - Action: generic sensor fusion service must supply: timestamped streams and provenance. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SENSOR-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SENSOR-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse generic sensor fusion service through the shared platform contract, delivering every part of: timestamped streams and provenance. Do not fork a duplicate subsystem for this row.
  - Requires: T-SENSOR-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SENSOR-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SENSOR-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for generic sensor fusion service.
- [ ] **T-SENSOR-004.05 — Qualify and retain this target's own result**
  - Action: Bind generic sensor fusion service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SENSOR-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-periph-003"></a>
## T-PERIPH-003 — keyboard backlight/LED

**Original requirement:** ownership, ranges and suspend restore

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 251.

### Execution steps

- [ ] **T-PERIPH-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve keyboard backlight/LED to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-PERIPH-003.02 — Specify the complete target boundary**
  - Action: keyboard backlight/LED must supply: ownership, ranges and suspend restore. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-PERIPH-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-PERIPH-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse keyboard backlight/LED through the shared platform contract, delivering every part of: ownership, ranges and suspend restore. Do not fork a duplicate subsystem for this row.
  - Requires: T-PERIPH-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-PERIPH-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-PERIPH-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for keyboard backlight/LED.
- [ ] **T-PERIPH-003.05 — Qualify and retain this target's own result**
  - Action: Bind keyboard backlight/LED to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-PERIPH-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-003"></a>
## T-VM-003 — virtio balloon

**Original requirement:** pressure/accounting and reclaim safety

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 260.

### Execution steps

- [ ] **T-VM-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio balloon to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-003.02 — Specify the complete target boundary**
  - Action: virtio balloon must supply: pressure/accounting and reclaim safety. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio balloon through the shared platform contract, delivering every part of: pressure/accounting and reclaim safety. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-003.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio balloon.
- [ ] **T-VM-003.05 — Qualify and retain this target's own result**
  - Action: Bind virtio balloon to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-010"></a>
## T-VM-010 — Linux Driver VM transport

**Original requirement:** attested provider, IOMMU, epochs and recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 267.

### Execution steps

- [ ] **T-VM-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Linux Driver VM transport to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-010.02 — Specify the complete target boundary**
  - Action: Linux Driver VM transport must supply: attested provider, IOMMU, epochs and recovery. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Linux Driver VM transport through the shared platform contract, delivering every part of: attested provider, IOMMU, epochs and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-010.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Linux Driver VM transport.
- [ ] **T-VM-010.05 — Qualify and retain this target's own result**
  - Action: Bind Linux Driver VM transport to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-vm-011"></a>
## T-VM-011 — headless/server hardware profile

**Original requirement:** serial/network management and no-display boot

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 268.

### Execution steps

- [ ] **T-VM-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve headless/server hardware profile to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-VM-011.02 — Specify the complete target boundary**
  - Action: headless/server hardware profile must supply: serial/network management and no-display boot. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-VM-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-VM-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse headless/server hardware profile through the shared platform contract, delivering every part of: serial/network management and no-display boot. Do not fork a duplicate subsystem for this row.
  - Requires: T-VM-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-VM-011.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: unsupported syscall/ABI; wrong architecture/version; hostile executable; broken dynamic dependency; foreign process crash; shared-folder/clipboard revoke; guest escape; interrupted upgrade.
  - Requires: T-VM-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for headless/server hardware profile.
- [ ] **T-VM-011.05 — Qualify and retain this target's own result**
  - Action: Bind headless/server hardware profile to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-VM-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fsp-014"></a>
## T-FSP-014 — network filesystem

**Original requirement:** authentication/offline/cache/conflict policy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 290.

### Execution steps

- [ ] **T-FSP-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve network filesystem to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FSP-014.02 — Specify the complete target boundary**
  - Action: network filesystem must supply: authentication/offline/cache/conflict policy. Define declared input/version/admission/start/work/cancel/complete or fail/cleanup/update/recovery with explicit authority and resources. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FSP-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FSP-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse network filesystem through the shared platform contract, delivering every part of: authentication/offline/cache/conflict policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-FSP-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FSP-014.04 — Prove target-specific failure and recovery**
  - Action: Require exact versioned workload and negative corpus, independent output verification, bounded failure and rollback; target execution and physical qualification where the target requires them. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-FSP-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for network filesystem.
- [ ] **T-FSP-014.05 — Qualify and retain this target's own result**
  - Action: Bind network filesystem to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FSP-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-024"></a>
## T-SVC-024 — Power Manager

**Original requirement:** shutdown/reboot/suspend, policy and ordered quiesce

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 39.

### Execution steps

- [ ] **T-SVC-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Power Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-024.02 — Specify the complete target boundary**
  - Action: Power Manager must supply: shutdown/reboot/suspend, policy and ordered quiesce. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse Power Manager through the shared platform contract, delivering every part of: shutdown/reboot/suspend, policy and ordered quiesce. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-024.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Power Manager.
- [ ] **T-SVC-024.05 — Qualify and retain this target's own result**
  - Action: Bind Power Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-025"></a>
## T-SVC-025 — Battery/Thermal Manager

**Original requirement:** status, thresholds, fan policy and failsafe

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 40.

### Execution steps

- [ ] **T-SVC-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Battery/Thermal Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-025.02 — Specify the complete target boundary**
  - Action: Battery/Thermal Manager must supply: status, thresholds, fan policy and failsafe. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse Battery/Thermal Manager through the shared platform contract, delivering every part of: status, thresholds, fan policy and failsafe. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-025.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Battery/Thermal Manager.
- [ ] **T-SVC-025.05 — Qualify and retain this target's own result**
  - Action: Bind Battery/Thermal Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-027"></a>
## T-SVC-027 — Hardware Qualification Service

**Original requirement:** topology-specific tests and physical receipts

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 42.

### Execution steps

- [ ] **T-SVC-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Hardware Qualification Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-027.02 — Specify the complete target boundary**
  - Action: Hardware Qualification Service must supply: topology-specific tests and physical receipts. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse Hardware Qualification Service through the shared platform contract, delivering every part of: topology-specific tests and physical receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-027.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Hardware Qualification Service.
- [ ] **T-SVC-027.05 — Qualify and retain this target's own result**
  - Action: Bind Hardware Qualification Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-028"></a>
## T-SVC-028 — Firmware/Microcode Update Service

**Original requirement:** signed staged updates, rollback and compatibility

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 43.

### Execution steps

- [ ] **T-SVC-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Firmware/Microcode Update Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-028.02 — Specify the complete target boundary**
  - Action: Firmware/Microcode Update Service must supply: signed staged updates, rollback and compatibility. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse Firmware/Microcode Update Service through the shared platform contract, delivering every part of: signed staged updates, rollback and compatibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-028.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Firmware/Microcode Update Service.
- [ ] **T-SVC-028.05 — Qualify and retain this target's own result**
  - Action: Bind Firmware/Microcode Update Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-057"></a>
## T-SVC-057 — Wi-Fi Service

**Original requirement:** scan/associate/WPA/roam/regulatory/credentials

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 77.

### Execution steps

- [ ] **T-SVC-057.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Wi-Fi Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-057.02 — Specify the complete target boundary**
  - Action: Wi-Fi Service must supply: scan/associate/WPA/roam/regulatory/credentials. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-057.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-057.03 — Implement the exact target behavior**
  - Action: Implement or reuse Wi-Fi Service through the shared platform contract, delivering every part of: scan/associate/WPA/roam/regulatory/credentials. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-057.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-057.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-057.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Wi-Fi Service.
- [ ] **T-SVC-057.05 — Qualify and retain this target's own result**
  - Action: Bind Wi-Fi Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-057.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-058"></a>
## T-SVC-058 — Bluetooth Service

**Original requirement:** HCI, pairing, keys, discovery and profile brokers

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 78.

### Execution steps

- [ ] **T-SVC-058.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Bluetooth Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-058.02 — Specify the complete target boundary**
  - Action: Bluetooth Service must supply: HCI, pairing, keys, discovery and profile brokers. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-058.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-058.03 — Implement the exact target behavior**
  - Action: Implement or reuse Bluetooth Service through the shared platform contract, delivering every part of: HCI, pairing, keys, discovery and profile brokers. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-058.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-058.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-058.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Bluetooth Service.
- [ ] **T-SVC-058.05 — Qualify and retain this target's own result**
  - Action: Bind Bluetooth Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-058.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-064"></a>
## T-SVC-064 — VPN/Tunnel Service

**Original requirement:** keys, routes, kill switch, reconnect and policy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 84.

### Execution steps

- [ ] **T-SVC-064.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VPN/Tunnel Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-064.02 — Specify the complete target boundary**
  - Action: VPN/Tunnel Service must supply: keys, routes, kill switch, reconnect and policy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-064.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-064.03 — Implement the exact target behavior**
  - Action: Implement or reuse VPN/Tunnel Service through the shared platform contract, delivering every part of: keys, routes, kill switch, reconnect and policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-064.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-064.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-064.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VPN/Tunnel Service.
- [ ] **T-SVC-064.05 — Qualify and retain this target's own result**
  - Action: Bind VPN/Tunnel Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-064.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-113"></a>
## T-SVC-113 — Location Portal

**Original requirement:** consent, precision, lifetime and audit

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 139.

### Execution steps

- [ ] **T-SVC-113.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Location Portal to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-113.02 — Specify the complete target boundary**
  - Action: Location Portal must supply: consent, precision, lifetime and audit. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-113.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-113.03 — Implement the exact target behavior**
  - Action: Implement or reuse Location Portal through the shared platform contract, delivering every part of: consent, precision, lifetime and audit. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-113.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-113.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-SVC-113.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Location Portal.
- [ ] **T-SVC-113.05 — Qualify and retain this target's own result**
  - Action: Bind Location Portal to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-113.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-106"></a>
## T-APP-106 — Hardware Test Center

**Original requirement:** exact device qualification, failures and receipts

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 234.

### Execution steps

- [ ] **T-APP-106.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Hardware Test Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-106.02 — Specify the complete target boundary**
  - Action: Hardware Test Center must supply: exact device qualification, failures and receipts. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-106.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-106.03 — Implement the exact target behavior**
  - Action: Implement or reuse Hardware Test Center through the shared platform contract, delivering every part of: exact device qualification, failures and receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-106.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-106.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-APP-106.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Hardware Test Center.
- [ ] **T-APP-106.05 — Qualify and retain this target's own result**
  - Action: Bind Hardware Test Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-106.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-001"></a>
## T-FUT-001 — x86-64 laptop

**Original requirement:** battery/thermal/suspend, Intel display/audio/network, Wi-Fi/BT, touchpad

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 176.

### Execution steps

- [ ] **T-FUT-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86-64 laptop to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-001.02 — Specify the complete target boundary**
  - Action: x86-64 laptop must supply: battery/thermal/suspend, Intel display/audio/network, Wi-Fi/BT, touchpad. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86-64 laptop through the shared platform contract, delivering every part of: battery/thermal/suspend, Intel display/audio/network, Wi-Fi/BT, touchpad. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86-64 laptop.
- [ ] **T-FUT-001.05 — Qualify and retain this target's own result**
  - Action: Bind x86-64 laptop to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-002"></a>
## T-FUT-002 — x86-64 desktop/workstation

**Original requirement:** discrete GPU, multimonitor, wired/audio/USB and power

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 177.

### Execution steps

- [ ] **T-FUT-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve x86-64 desktop/workstation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-002.02 — Specify the complete target boundary**
  - Action: x86-64 desktop/workstation must supply: discrete GPU, multimonitor, wired/audio/USB and power. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse x86-64 desktop/workstation through the shared platform contract, delivering every part of: discrete GPU, multimonitor, wired/audio/USB and power. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for x86-64 desktop/workstation.
- [ ] **T-FUT-002.05 — Qualify and retain this target's own result**
  - Action: Bind x86-64 desktop/workstation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-003"></a>
## T-FUT-003 — QEMU development VM

**Original requirement:** complete virtio profile and deterministic gates

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 178.

### Execution steps

- [ ] **T-FUT-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve QEMU development VM to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-003.02 — Specify the complete target boundary**
  - Action: QEMU development VM must supply: complete virtio profile and deterministic gates. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse QEMU development VM through the shared platform contract, delivering every part of: complete virtio profile and deterministic gates. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for QEMU development VM.
- [ ] **T-FUT-003.05 — Qualify and retain this target's own result**
  - Action: Bind QEMU development VM to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-004"></a>
## T-FUT-004 — headless/server

**Original requirement:** serial/network management, storage, no-display boot and updates

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 179.

### Execution steps

- [ ] **T-FUT-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve headless/server to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-004.02 — Specify the complete target boundary**
  - Action: headless/server must supply: serial/network management, storage, no-display boot and updates. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse headless/server through the shared platform contract, delivering every part of: serial/network management, storage, no-display boot and updates. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for headless/server.
- [ ] **T-FUT-004.05 — Qualify and retain this target's own result**
  - Action: Bind headless/server to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-005"></a>
## T-FUT-005 — AArch64 generic VM

**Original requirement:** UEFI/FDT, EL/MMU/GIC/timer/SMP/process/services

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 180.

### Execution steps

- [ ] **T-FUT-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AArch64 generic VM to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-005.02 — Specify the complete target boundary**
  - Action: AArch64 generic VM must supply: UEFI/FDT, EL/MMU/GIC/timer/SMP/process/services. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse AArch64 generic VM through the shared platform contract, delivering every part of: UEFI/FDT, EL/MMU/GIC/timer/SMP/process/services. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AArch64 generic VM.
- [ ] **T-FUT-005.05 — Qualify and retain this target's own result**
  - Action: Bind AArch64 generic VM to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-006"></a>
## T-FUT-006 — Raspberry Pi

**Original requirement:** firmware/mailbox/GPIO/SD/display/input/network/power

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 181.

### Execution steps

- [ ] **T-FUT-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Raspberry Pi to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-006.02 — Specify the complete target boundary**
  - Action: Raspberry Pi must supply: firmware/mailbox/GPIO/SD/display/input/network/power. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse Raspberry Pi through the shared platform contract, delivering every part of: firmware/mailbox/GPIO/SD/display/input/network/power. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Raspberry Pi.
- [ ] **T-FUT-006.05 — Qualify and retain this target's own result**
  - Action: Bind Raspberry Pi to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-007"></a>
## T-FUT-007 — Apple Silicon research

**Original requirement:** m1n1/FDT/PCIe/DART/display/input/storage platform contracts

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 182.

### Execution steps

- [ ] **T-FUT-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Apple Silicon research to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-007.02 — Specify the complete target boundary**
  - Action: Apple Silicon research must supply: m1n1/FDT/PCIe/DART/display/input/storage platform contracts. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse Apple Silicon research through the shared platform contract, delivering every part of: m1n1/FDT/PCIe/DART/display/input/storage platform contracts. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Apple Silicon research.
- [ ] **T-FUT-007.05 — Qualify and retain this target's own result**
  - Action: Bind Apple Silicon research to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-008"></a>
## T-FUT-008 — RISC-V64 generic VM/board

**Original requirement:** SBI/Sv39/PLIC/timer/SMP/process/services

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 183.

### Execution steps

- [ ] **T-FUT-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve RISC-V64 generic VM/board to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-008.02 — Specify the complete target boundary**
  - Action: RISC-V64 generic VM/board must supply: SBI/Sv39/PLIC/timer/SMP/process/services. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse RISC-V64 generic VM/board through the shared platform contract, delivering every part of: SBI/Sv39/PLIC/timer/SMP/process/services. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for RISC-V64 generic VM/board.
- [ ] **T-FUT-008.05 — Qualify and retain this target's own result**
  - Action: Bind RISC-V64 generic VM/board to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-009"></a>
## T-FUT-009 — tablet/touch product

**Original requirement:** touch/stylus/rotation/keyboard/a11y/power/camera

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 184.

### Execution steps

- [ ] **T-FUT-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve tablet/touch product to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-009.02 — Specify the complete target boundary**
  - Action: tablet/touch product must supply: touch/stylus/rotation/keyboard/a11y/power/camera. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse tablet/touch product through the shared platform contract, delivering every part of: touch/stylus/rotation/keyboard/a11y/power/camera. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for tablet/touch product.
- [ ] **T-FUT-009.05 — Qualify and retain this target's own result**
  - Action: Bind tablet/touch product to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-010"></a>
## T-FUT-010 — secure/managed profile

**Original requirement:** TPM/measured boot/policy/update/attestation and recovery

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 185.

### Execution steps

- [ ] **T-FUT-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve secure/managed profile to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-010.02 — Specify the complete target boundary**
  - Action: secure/managed profile must supply: TPM/measured boot/policy/update/attestation and recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse secure/managed profile through the shared platform contract, delivering every part of: TPM/measured boot/policy/update/attestation and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for secure/managed profile.
- [ ] **T-FUT-010.05 — Qualify and retain this target's own result**
  - Action: Bind secure/managed profile to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-011"></a>
## T-FUT-011 — heterogeneous compute

**Original requirement:** CPU/GPU topology, memory/coherency, scheduling and fallback

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 186.

### Execution steps

- [ ] **T-FUT-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve heterogeneous compute to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-011.02 — Specify the complete target boundary**
  - Action: heterogeneous compute must supply: CPU/GPU topology, memory/coherency, scheduling and fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse heterogeneous compute through the shared platform contract, delivering every part of: CPU/GPU topology, memory/coherency, scheduling and fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for heterogeneous compute.
- [ ] **T-FUT-011.05 — Qualify and retain this target's own result**
  - Action: Bind heterogeneous compute to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-fut-012"></a>
## T-FUT-012 — custom FPGA accelerator

**Original requirement:** capability descriptors, DMA/isolation, simulation and reset

**Source:** [docs/program/LANGUAGE-AGENTS-OPERATIONS.md](../../docs/program/LANGUAGE-AGENTS-OPERATIONS.md), line 187.

### Execution steps

- [ ] **T-FUT-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve custom FPGA accelerator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-04, D-06, D-11, D-13, D-25, H-19.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-FUT-012.02 — Specify the complete target boundary**
  - Action: custom FPGA accelerator must supply: capability descriptors, DMA/isolation, simulation and reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-FUT-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-FUT-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse custom FPGA accelerator through the shared platform contract, delivering every part of: capability descriptors, DMA/isolation, simulation and reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-FUT-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-FUT-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: wrong architecture/device revision; absent required capability; incompatible firmware; DMA/MMU mismatch; reset/hotplug/suspend failure; unavailable hardware; unsafe power sequence.
  - Requires: T-FUT-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for custom FPGA accelerator.
- [ ] **T-FUT-012.05 — Qualify and retain this target's own result**
  - Action: Bind custom FPGA accelerator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-FUT-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
