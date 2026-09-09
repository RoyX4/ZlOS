# MP-09: Build audio, capture and synchronized media streams

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/drivers/; kernel/src/graphics/; kernel/apps/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-09` exports: Audio/capture ownership, bounded rings and permission-aware streams usable by accessibility and media clients.

The handoff enables only its named subset. `CLOSE-09` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-09.01 — Specify device format/rate/channel/buffer negotiation, clock ownership and stream lifecycle

Specify device format/rate/channel/buffer negotiation, clock ownership and stream lifecycle.

**Requires:** `D-01`, `D-02`, `D-16`, `H-00`, `H-04`, `H-05`, `H-07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-09.02 — Wrap existing audio behavior and add selected HDA/AC97/virtio/USB/legacy providers behind the common driver contract

Wrap existing audio behavior and add selected HDA/AC97/virtio/USB/legacy providers behind the common driver contract.

**Requires:** `M-09.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-09.03 — Build AudioServer mixing, device selection, volume and bounded shared rings without granting apps device DMA

Build AudioServer mixing, device selection, volume and bounded shared rings without granting apps device DMA.

**Requires:** `M-09.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-09.04 — Handle underrun, overrun, partial buffers, clock drift, disconnect, suspend and restart explicitly

Handle underrun, overrun, partial buffers, clock drift, disconnect, suspend and restart explicitly.

**Requires:** `M-09.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-09 — Bounded development handoff: Build audio, capture and synchronized media streams

Audio/capture ownership, bounded rings and permission-aware streams usable by accessibility and media clients.

**Requires:** `M-09.04`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-09.05 — Define microphone/camera capture grants, visible indicators and immediate revocation semantics

Define microphone/camera capture grants, visible indicators and immediate revocation semantics.

**Requires:** `M-09.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-09.06 — Implement UVC/capture and isolated software codec workers with bounded output and hostile input admission

Implement UVC/capture and isolated software codec workers with bounded output and hostile input admission.

**Requires:** `M-09.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-09.07 — Join media clocks and synchronization to monotonic timing rather than wall-clock adjustments

Join media clocks and synchronization to monotonic timing rather than wall-clock adjustments.

**Requires:** `M-09.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-09.08 — Measure real stream latency and physical capture/playback where claimed

Measure real stream latency and physical capture/playback where claimed; simulated buffers alone do not close hardware targets.

**Requires:** `M-09.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-AD-001](#f-ad-001) | feature | AudioServer |
| [F-AD-002](#f-ad-002) | feature | audio stream contract |
| [F-AD-003](#f-ad-003) | feature | mixer |
| [F-AD-004](#f-ad-004) | feature | resampler |
| [F-AD-005](#f-ad-005) | feature | channel conversion |
| [F-AD-006](#f-ad-006) | feature | audio clock |
| [F-AD-007](#f-ad-007) | feature | playback routing |
| [F-AD-008](#f-ad-008) | feature | recording routing |
| [F-AD-009](#f-ad-009) | feature | system sounds |
| [F-AD-010](#f-ad-010) | feature | media session |
| [F-AD-011](#f-ad-011) | feature | WAV/PCM |
| [F-AD-012](#f-ad-012) | feature | compressed audio codecs |
| [F-AD-013](#f-ad-013) | feature | image decode service |
| [F-AD-014](#f-ad-014) | feature | image encode/export |
| [F-AD-015](#f-ad-015) | feature | video demux/decode |
| [F-AD-016](#f-ad-016) | feature | audio/video synchronization |
| [F-AD-017](#f-ad-017) | feature | subtitle/caption pipeline |
| [F-AD-018](#f-ad-018) | feature | camera service |
| [F-AD-019](#f-ad-019) | feature | microphone service |
| [F-AD-020](#f-ad-020) | feature | MIDI |
| [F-AD-021](#f-ad-021) | feature | synthesizer/DSP library |
| [F-AD-022](#f-ad-022) | feature | media metadata |
| [F-AD-023](#f-ad-023) | feature | media library/index |
| [F-AD-024](#f-ad-024) | feature | media hardware acceleration |
| [F-AD-025](#f-ad-025) | feature | audio diagnostics |
| [C-P5.4](#c-p5-4) | contract | Audio service and provider ABI |
| [C-DA-14](#c-da-14) | contract | audio provider and AudioServer |
| [T-BLK-010](#t-blk-010) | target | USB MSC UAS |
| [T-USB-001](#t-usb-001) | target | UHCI host |
| [T-USB-002](#t-usb-002) | target | OHCI host |
| [T-USB-003](#t-usb-003) | target | EHCI host |
| [T-USB-007](#t-usb-007) | target | USB Audio class 1/2 |
| [T-USB-008](#t-usb-008) | target | USB Video class |
| [T-MEDIA-001](#t-media-001) | target | Intel/PCI HDA controller |
| [T-MEDIA-002](#t-media-002) | target | AC97 |
| [T-MEDIA-003](#t-media-003) | target | virtio-snd |
| [T-MEDIA-005](#t-media-005) | target | PC speaker |
| [T-MEDIA-006](#t-media-006) | target | USB Audio |
| [T-MEDIA-007](#t-media-007) | target | microphone/capture provider |
| [T-MEDIA-008](#t-media-008) | target | UVC camera |
| [T-SVC-090](#t-svc-090) | target | Audio Server |
| [T-SVC-091](#t-svc-091) | target | Media Clock Service |
| [T-SVC-092](#t-svc-092) | target | Codec/Decoder Worker Pool |
| [T-SVC-093](#t-svc-093) | target | Camera/Capture Service |

<a id="f-ad-001"></a>
## F-AD-001 — AudioServer

**Original requirement:** exclusive hardware owner, authenticated clients, streams, routing, volume and health

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-001.01 — Reconcile existing AudioServer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for AudioServer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exclusive hardware owner, authenticated clients, streams, routing, volume and health
- [ ] **F-AD-001.02 — Freeze the exact contract for AudioServer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exclusive hardware owner, authenticated clients, streams, routing, volume and health. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-001.03 — Implement/prove: exclusive hardware owner**
  - Action: For AudioServer, implement or reuse and verify this exact obligation: exclusive hardware owner. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exclusive hardware owner; retain observable state/resource expectations.
- [ ] **F-AD-001.04 — Implement/prove: authenticated clients**
  - Action: For AudioServer, implement or reuse and verify this exact obligation: authenticated clients. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated clients; retain observable state/resource expectations.
- [ ] **F-AD-001.05 — Implement/prove: streams**
  - Action: For AudioServer, implement or reuse and verify this exact obligation: streams. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for streams; retain observable state/resource expectations.
- [ ] **F-AD-001.06 — Implement/prove: routing**
  - Action: For AudioServer, implement or reuse and verify this exact obligation: routing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for routing; retain observable state/resource expectations.
- [ ] **F-AD-001.07 — Implement/prove: volume and health**
  - Action: For AudioServer, implement or reuse and verify this exact obligation: volume and health. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for volume and health; retain observable state/resource expectations.
- [ ] **F-AD-001.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to AudioServer: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-001.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-001.09 — Integrate into the real consumer and runtime route**
  - Action: Wire AudioServer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-001.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-001.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for AudioServer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-001.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-002"></a>
## F-AD-002 — audio stream contract

**Original requirement:** rate/channels/format/buffer/latency negotiation and bounded shared rings

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-002.01 — Reconcile existing audio stream contract**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio stream contract. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: rate/channels/format/buffer/latency negotiation and bounded shared rings
- [ ] **F-AD-002.02 — Freeze the exact contract for audio stream contract**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: rate/channels/format/buffer/latency negotiation and bounded shared rings. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-002.03 — Implement/prove: rate/channels/format/buffer/latency negotiation and bounded shared rings**
  - Action: For audio stream contract, implement or reuse and verify this exact obligation: rate/channels/format/buffer/latency negotiation and bounded shared rings. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rate/channels/format/buffer/latency negotiation and bounded shared rings; retain observable state/resource expectations.
- [ ] **F-AD-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio stream contract: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio stream contract into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio stream contract as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-003"></a>
## F-AD-003 — mixer

**Original requirement:** per-client gain/mute/pan, saturation, deterministic mix and resource budgets

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-003.01 — Reconcile existing mixer**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mixer. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-client gain/mute/pan, saturation, deterministic mix and resource budgets
- [ ] **F-AD-003.02 — Freeze the exact contract for mixer**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-client gain/mute/pan, saturation, deterministic mix and resource budgets. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-003.03 — Implement/prove: per-client gain/mute/pan**
  - Action: For mixer, implement or reuse and verify this exact obligation: per-client gain/mute/pan. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-client gain/mute/pan; retain observable state/resource expectations.
- [ ] **F-AD-003.04 — Implement/prove: saturation**
  - Action: For mixer, implement or reuse and verify this exact obligation: saturation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for saturation; retain observable state/resource expectations.
- [ ] **F-AD-003.05 — Implement/prove: deterministic mix and resource budgets**
  - Action: For mixer, implement or reuse and verify this exact obligation: deterministic mix and resource budgets. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic mix and resource budgets; retain observable state/resource expectations.
- [ ] **F-AD-003.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mixer: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-003.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-003.07 — Integrate into the real consumer and runtime route**
  - Action: Wire mixer into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-003.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-003.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mixer as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-003.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-004"></a>
## F-AD-004 — resampler

**Original requirement:** declared quality/latency, rate changes and golden/property tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-004.01 — Reconcile existing resampler**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for resampler. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: declared quality/latency, rate changes and golden/property tests
- [ ] **F-AD-004.02 — Freeze the exact contract for resampler**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: declared quality/latency, rate changes and golden/property tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-004.03 — Implement/prove: declared quality/latency**
  - Action: For resampler, implement or reuse and verify this exact obligation: declared quality/latency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared quality/latency; retain observable state/resource expectations.
- [ ] **F-AD-004.04 — Implement/prove: rate changes and golden/property tests**
  - Action: For resampler, implement or reuse and verify this exact obligation: rate changes and golden/property tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rate changes and golden/property tests; retain observable state/resource expectations.
- [ ] **F-AD-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to resampler: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire resampler into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for resampler as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-005"></a>
## F-AD-005 — channel conversion

**Original requirement:** mono/stereo/multichannel mapping with explicit layout and clipping

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-005.01 — Reconcile existing channel conversion**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for channel conversion. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: mono/stereo/multichannel mapping with explicit layout and clipping
- [ ] **F-AD-005.02 — Freeze the exact contract for channel conversion**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: mono/stereo/multichannel mapping with explicit layout and clipping. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-005.03 — Implement/prove: mono/stereo/multichannel mapping with explicit layout and clipping**
  - Action: For channel conversion, implement or reuse and verify this exact obligation: mono/stereo/multichannel mapping with explicit layout and clipping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mono/stereo/multichannel mapping with explicit layout and clipping; retain observable state/resource expectations.
- [ ] **F-AD-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to channel conversion: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire channel conversion into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for channel conversion as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-006"></a>
## F-AD-006 — audio clock

**Original requirement:** hardware/monotonic relation, drift, timestamp, underrun and sync semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-006.01 — Reconcile existing audio clock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio clock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hardware/monotonic relation, drift, timestamp, underrun and sync semantics
- [ ] **F-AD-006.02 — Freeze the exact contract for audio clock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hardware/monotonic relation, drift, timestamp, underrun and sync semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-006.03 — Implement/prove: hardware/monotonic relation**
  - Action: For audio clock, implement or reuse and verify this exact obligation: hardware/monotonic relation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hardware/monotonic relation; retain observable state/resource expectations.
- [ ] **F-AD-006.04 — Implement/prove: drift**
  - Action: For audio clock, implement or reuse and verify this exact obligation: drift. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drift; retain observable state/resource expectations.
- [ ] **F-AD-006.05 — Implement/prove: timestamp**
  - Action: For audio clock, implement or reuse and verify this exact obligation: timestamp. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timestamp; retain observable state/resource expectations.
- [ ] **F-AD-006.06 — Implement/prove: underrun and sync semantics**
  - Action: For audio clock, implement or reuse and verify this exact obligation: underrun and sync semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for underrun and sync semantics; retain observable state/resource expectations.
- [ ] **F-AD-006.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio clock: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-006.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-006.08 — Integrate into the real consumer and runtime route**
  - Action: Wire audio clock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-006.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-006.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio clock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-006.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-007"></a>
## F-AD-007 — playback routing

**Original requirement:** output selection, hotplug, default/fallback and app/session permissions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-007.01 — Reconcile existing playback routing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for playback routing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: output selection, hotplug, default/fallback and app/session permissions
- [ ] **F-AD-007.02 — Freeze the exact contract for playback routing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: output selection, hotplug, default/fallback and app/session permissions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-007.03 — Implement/prove: output selection**
  - Action: For playback routing, implement or reuse and verify this exact obligation: output selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for output selection; retain observable state/resource expectations.
- [ ] **F-AD-007.04 — Implement/prove: hotplug**
  - Action: For playback routing, implement or reuse and verify this exact obligation: hotplug. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug; retain observable state/resource expectations.
- [ ] **F-AD-007.05 — Implement/prove: default/fallback and app/session permissions**
  - Action: For playback routing, implement or reuse and verify this exact obligation: default/fallback and app/session permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for default/fallback and app/session permissions; retain observable state/resource expectations.
- [ ] **F-AD-007.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to playback routing: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-007.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-007.07 — Integrate into the real consumer and runtime route**
  - Action: Wire playback routing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-007.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-007.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for playback routing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-007.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-008"></a>
## F-AD-008 — recording routing

**Original requirement:** input selection, privacy indicator, grant, gain, echo policy and revoke

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-008.01 — Reconcile existing recording routing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for recording routing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: input selection, privacy indicator, grant, gain, echo policy and revoke
- [ ] **F-AD-008.02 — Freeze the exact contract for recording routing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: input selection, privacy indicator, grant, gain, echo policy and revoke. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-008.03 — Implement/prove: input selection**
  - Action: For recording routing, implement or reuse and verify this exact obligation: input selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input selection; retain observable state/resource expectations.
- [ ] **F-AD-008.04 — Implement/prove: privacy indicator**
  - Action: For recording routing, implement or reuse and verify this exact obligation: privacy indicator. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy indicator; retain observable state/resource expectations.
- [ ] **F-AD-008.05 — Implement/prove: grant**
  - Action: For recording routing, implement or reuse and verify this exact obligation: grant. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grant; retain observable state/resource expectations.
- [ ] **F-AD-008.06 — Implement/prove: gain**
  - Action: For recording routing, implement or reuse and verify this exact obligation: gain. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for gain; retain observable state/resource expectations.
- [ ] **F-AD-008.07 — Implement/prove: echo policy and revoke**
  - Action: For recording routing, implement or reuse and verify this exact obligation: echo policy and revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-008.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for echo policy and revoke; retain observable state/resource expectations.
- [ ] **F-AD-008.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to recording routing: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-008.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-008.09 — Integrate into the real consumer and runtime route**
  - Action: Wire recording routing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-008.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-008.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for recording routing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-008.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-009"></a>
## F-AD-009 — system sounds

**Original requirement:** themed accessible cues, rate limiting, quiet mode and no direct device access

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-009.01 — Reconcile existing system sounds**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for system sounds. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: themed accessible cues, rate limiting, quiet mode and no direct device access
- [ ] **F-AD-009.02 — Freeze the exact contract for system sounds**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: themed accessible cues, rate limiting, quiet mode and no direct device access. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-009.03 — Implement/prove: themed accessible cues**
  - Action: For system sounds, implement or reuse and verify this exact obligation: themed accessible cues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for themed accessible cues; retain observable state/resource expectations.
- [ ] **F-AD-009.04 — Implement/prove: rate limiting**
  - Action: For system sounds, implement or reuse and verify this exact obligation: rate limiting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rate limiting; retain observable state/resource expectations.
- [ ] **F-AD-009.05 — Implement/prove: quiet mode and no direct device access**
  - Action: For system sounds, implement or reuse and verify this exact obligation: quiet mode and no direct device access. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for quiet mode and no direct device access; retain observable state/resource expectations.
- [ ] **F-AD-009.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to system sounds: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-009.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-009.07 — Integrate into the real consumer and runtime route**
  - Action: Wire system sounds into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-009.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-009.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for system sounds as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-009.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-010"></a>
## F-AD-010 — media session

**Original requirement:** play/pause/seek/metadata/position/rate and global controls across apps

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-010.01 — Reconcile existing media session**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for media session. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: play/pause/seek/metadata/position/rate and global controls across apps
- [ ] **F-AD-010.02 — Freeze the exact contract for media session**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: play/pause/seek/metadata/position/rate and global controls across apps. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-010.03 — Implement/prove: play/pause/seek/metadata/position/rate and global controls across apps**
  - Action: For media session, implement or reuse and verify this exact obligation: play/pause/seek/metadata/position/rate and global controls across apps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for play/pause/seek/metadata/position/rate and global controls across apps; retain observable state/resource expectations.
- [ ] **F-AD-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to media session: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire media session into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for media session as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-011"></a>
## F-AD-011 — WAV/PCM

**Original requirement:** checked headers/chunks/formats, streaming and golden output

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-011.01 — Reconcile existing WAV/PCM**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for WAV/PCM. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked headers/chunks/formats, streaming and golden output
- [ ] **F-AD-011.02 — Freeze the exact contract for WAV/PCM**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked headers/chunks/formats, streaming and golden output. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-011.03 — Implement/prove: checked headers/chunks/formats**
  - Action: For WAV/PCM, implement or reuse and verify this exact obligation: checked headers/chunks/formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked headers/chunks/formats; retain observable state/resource expectations.
- [ ] **F-AD-011.04 — Implement/prove: streaming and golden output**
  - Action: For WAV/PCM, implement or reuse and verify this exact obligation: streaming and golden output. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for streaming and golden output; retain observable state/resource expectations.
- [ ] **F-AD-011.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to WAV/PCM: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-011.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-011.06 — Integrate into the real consumer and runtime route**
  - Action: Wire WAV/PCM into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-011.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-011.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for WAV/PCM as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-011.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-012"></a>
## F-AD-012 — compressed audio codecs

**Original requirement:** isolated decoders, format matrix, output/time/memory limits and failure UX

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-012.01 — Reconcile existing compressed audio codecs**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compressed audio codecs. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated decoders, format matrix, output/time/memory limits and failure UX
- [ ] **F-AD-012.02 — Freeze the exact contract for compressed audio codecs**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated decoders, format matrix, output/time/memory limits and failure UX. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-012.03 — Implement/prove: isolated decoders**
  - Action: For compressed audio codecs, implement or reuse and verify this exact obligation: isolated decoders. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated decoders; retain observable state/resource expectations.
- [ ] **F-AD-012.04 — Implement/prove: format matrix**
  - Action: For compressed audio codecs, implement or reuse and verify this exact obligation: format matrix. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format matrix; retain observable state/resource expectations.
- [ ] **F-AD-012.05 — Implement/prove: output/time/memory limits and failure UX**
  - Action: For compressed audio codecs, implement or reuse and verify this exact obligation: output/time/memory limits and failure UX. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for output/time/memory limits and failure UX; retain observable state/resource expectations.
- [ ] **F-AD-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compressed audio codecs: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire compressed audio codecs into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compressed audio codecs as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-013"></a>
## F-AD-013 — image decode service

**Original requirement:** isolated PNG/JPEG/etc. parser, dimensions/output budget, metadata and color profile

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-013.01 — Reconcile existing image decode service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for image decode service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated PNG/JPEG/etc. parser, dimensions/output budget, metadata and color profile
- [ ] **F-AD-013.02 — Freeze the exact contract for image decode service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated PNG/JPEG/etc. parser, dimensions/output budget, metadata and color profile. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-013.03 — Implement/prove: isolated PNG/JPEG/etc. parser**
  - Action: For image decode service, implement or reuse and verify this exact obligation: isolated PNG/JPEG/etc. parser. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated PNG/JPEG/etc. parser; retain observable state/resource expectations.
- [ ] **F-AD-013.04 — Implement/prove: dimensions/output budget**
  - Action: For image decode service, implement or reuse and verify this exact obligation: dimensions/output budget. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dimensions/output budget; retain observable state/resource expectations.
- [ ] **F-AD-013.05 — Implement/prove: metadata and color profile**
  - Action: For image decode service, implement or reuse and verify this exact obligation: metadata and color profile. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for metadata and color profile; retain observable state/resource expectations.
- [ ] **F-AD-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to image decode service: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire image decode service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for image decode service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-014"></a>
## F-AD-014 — image encode/export

**Original requirement:** explicit format/quality/profile/metadata/privacy and atomic file save

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-014.01 — Reconcile existing image encode/export**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for image encode/export. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit format/quality/profile/metadata/privacy and atomic file save
- [ ] **F-AD-014.02 — Freeze the exact contract for image encode/export**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit format/quality/profile/metadata/privacy and atomic file save. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-014.03 — Implement/prove: explicit format/quality/profile/metadata/privacy and atomic file save**
  - Action: For image encode/export, implement or reuse and verify this exact obligation: explicit format/quality/profile/metadata/privacy and atomic file save. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit format/quality/profile/metadata/privacy and atomic file save; retain observable state/resource expectations.
- [ ] **F-AD-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to image encode/export: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire image encode/export into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for image encode/export as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-015"></a>
## F-AD-015 — video demux/decode

**Original requirement:** isolated parsers, bounded queues, seek/error/recovery and codec receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-015.01 — Reconcile existing video demux/decode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for video demux/decode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated parsers, bounded queues, seek/error/recovery and codec receipts
- [ ] **F-AD-015.02 — Freeze the exact contract for video demux/decode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated parsers, bounded queues, seek/error/recovery and codec receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-015.03 — Implement/prove: isolated parsers**
  - Action: For video demux/decode, implement or reuse and verify this exact obligation: isolated parsers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated parsers; retain observable state/resource expectations.
- [ ] **F-AD-015.04 — Implement/prove: bounded queues**
  - Action: For video demux/decode, implement or reuse and verify this exact obligation: bounded queues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded queues; retain observable state/resource expectations.
- [ ] **F-AD-015.05 — Implement/prove: seek/error/recovery and codec receipts**
  - Action: For video demux/decode, implement or reuse and verify this exact obligation: seek/error/recovery and codec receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for seek/error/recovery and codec receipts; retain observable state/resource expectations.
- [ ] **F-AD-015.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to video demux/decode: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-015.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-015.07 — Integrate into the real consumer and runtime route**
  - Action: Wire video demux/decode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-015.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-015.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for video demux/decode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-015.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-016"></a>
## F-AD-016 — audio/video synchronization

**Original requirement:** master clock, drift correction, frame drop/repeat and seek reset

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-016.01 — Reconcile existing audio/video synchronization**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio/video synchronization. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: master clock, drift correction, frame drop/repeat and seek reset
- [ ] **F-AD-016.02 — Freeze the exact contract for audio/video synchronization**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: master clock, drift correction, frame drop/repeat and seek reset. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-016.03 — Implement/prove: master clock**
  - Action: For audio/video synchronization, implement or reuse and verify this exact obligation: master clock. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for master clock; retain observable state/resource expectations.
- [ ] **F-AD-016.04 — Implement/prove: drift correction**
  - Action: For audio/video synchronization, implement or reuse and verify this exact obligation: drift correction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drift correction; retain observable state/resource expectations.
- [ ] **F-AD-016.05 — Implement/prove: frame drop/repeat and seek reset**
  - Action: For audio/video synchronization, implement or reuse and verify this exact obligation: frame drop/repeat and seek reset. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-016.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frame drop/repeat and seek reset; retain observable state/resource expectations.
- [ ] **F-AD-016.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio/video synchronization: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-016.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-016.07 — Integrate into the real consumer and runtime route**
  - Action: Wire audio/video synchronization into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-016.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-016.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio/video synchronization as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-016.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-017"></a>
## F-AD-017 — subtitle/caption pipeline

**Original requirement:** timed text, styling, language, accessibility, live captions and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-017.01 — Reconcile existing subtitle/caption pipeline**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for subtitle/caption pipeline. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: timed text, styling, language, accessibility, live captions and privacy
- [ ] **F-AD-017.02 — Freeze the exact contract for subtitle/caption pipeline**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: timed text, styling, language, accessibility, live captions and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-017.03 — Implement/prove: timed text**
  - Action: For subtitle/caption pipeline, implement or reuse and verify this exact obligation: timed text. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timed text; retain observable state/resource expectations.
- [ ] **F-AD-017.04 — Implement/prove: styling**
  - Action: For subtitle/caption pipeline, implement or reuse and verify this exact obligation: styling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for styling; retain observable state/resource expectations.
- [ ] **F-AD-017.05 — Implement/prove: language**
  - Action: For subtitle/caption pipeline, implement or reuse and verify this exact obligation: language. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language; retain observable state/resource expectations.
- [ ] **F-AD-017.06 — Implement/prove: accessibility**
  - Action: For subtitle/caption pipeline, implement or reuse and verify this exact obligation: accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accessibility; retain observable state/resource expectations.
- [ ] **F-AD-017.07 — Implement/prove: live captions and privacy**
  - Action: For subtitle/caption pipeline, implement or reuse and verify this exact obligation: live captions and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-017.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for live captions and privacy; retain observable state/resource expectations.
- [ ] **F-AD-017.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to subtitle/caption pipeline: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-017.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-017.09 — Integrate into the real consumer and runtime route**
  - Action: Wire subtitle/caption pipeline into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-017.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-017.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for subtitle/caption pipeline as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-017.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-018"></a>
## F-AD-018 — camera service

**Original requirement:** device ownership, format negotiation, privacy, frame buffers and peer death

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-018.01 — Reconcile existing camera service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for camera service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: device ownership, format negotiation, privacy, frame buffers and peer death
- [ ] **F-AD-018.02 — Freeze the exact contract for camera service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: device ownership, format negotiation, privacy, frame buffers and peer death. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-018.03 — Implement/prove: device ownership**
  - Action: For camera service, implement or reuse and verify this exact obligation: device ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device ownership; retain observable state/resource expectations.
- [ ] **F-AD-018.04 — Implement/prove: format negotiation**
  - Action: For camera service, implement or reuse and verify this exact obligation: format negotiation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format negotiation; retain observable state/resource expectations.
- [ ] **F-AD-018.05 — Implement/prove: privacy**
  - Action: For camera service, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-AD-018.06 — Implement/prove: frame buffers and peer death**
  - Action: For camera service, implement or reuse and verify this exact obligation: frame buffers and peer death. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frame buffers and peer death; retain observable state/resource expectations.
- [ ] **F-AD-018.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to camera service: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-018.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-018.08 — Integrate into the real consumer and runtime route**
  - Action: Wire camera service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-018.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-018.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for camera service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-018.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-019"></a>
## F-AD-019 — microphone service

**Original requirement:** explicit recording state, device/source, privacy, level and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-019.01 — Reconcile existing microphone service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for microphone service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit recording state, device/source, privacy, level and cancellation
- [ ] **F-AD-019.02 — Freeze the exact contract for microphone service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit recording state, device/source, privacy, level and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-019.03 — Implement/prove: explicit recording state**
  - Action: For microphone service, implement or reuse and verify this exact obligation: explicit recording state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit recording state; retain observable state/resource expectations.
- [ ] **F-AD-019.04 — Implement/prove: device/source**
  - Action: For microphone service, implement or reuse and verify this exact obligation: device/source. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device/source; retain observable state/resource expectations.
- [ ] **F-AD-019.05 — Implement/prove: privacy**
  - Action: For microphone service, implement or reuse and verify this exact obligation: privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy; retain observable state/resource expectations.
- [ ] **F-AD-019.06 — Implement/prove: level and cancellation**
  - Action: For microphone service, implement or reuse and verify this exact obligation: level and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for level and cancellation; retain observable state/resource expectations.
- [ ] **F-AD-019.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to microphone service: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-019.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-019.08 — Integrate into the real consumer and runtime route**
  - Action: Wire microphone service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-019.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-019.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for microphone service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-019.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-020"></a>
## F-AD-020 — MIDI

**Original requirement:** device/events/clock/routing, hotplug and synthesizer integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-020.01 — Reconcile existing MIDI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for MIDI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: device/events/clock/routing, hotplug and synthesizer integration
- [ ] **F-AD-020.02 — Freeze the exact contract for MIDI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: device/events/clock/routing, hotplug and synthesizer integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-020.03 — Implement/prove: device/events/clock/routing**
  - Action: For MIDI, implement or reuse and verify this exact obligation: device/events/clock/routing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device/events/clock/routing; retain observable state/resource expectations.
- [ ] **F-AD-020.04 — Implement/prove: hotplug and synthesizer integration**
  - Action: For MIDI, implement or reuse and verify this exact obligation: hotplug and synthesizer integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug and synthesizer integration; retain observable state/resource expectations.
- [ ] **F-AD-020.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to MIDI: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-020.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-020.06 — Integrate into the real consumer and runtime route**
  - Action: Wire MIDI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-020.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-020.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for MIDI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-020.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-021"></a>
## F-AD-021 — synthesizer/DSP library

**Original requirement:** reusable bounded realtime buffer API derived from deterministic golden DSP

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-021.01 — Reconcile existing synthesizer/DSP library**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for synthesizer/DSP library. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reusable bounded realtime buffer API derived from deterministic golden DSP
- [ ] **F-AD-021.02 — Freeze the exact contract for synthesizer/DSP library**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reusable bounded realtime buffer API derived from deterministic golden DSP. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-021.03 — Implement/prove: reusable bounded realtime buffer API derived from deterministic golden DSP**
  - Action: For synthesizer/DSP library, implement or reuse and verify this exact obligation: reusable bounded realtime buffer API derived from deterministic golden DSP. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reusable bounded realtime buffer API derived from deterministic golden DSP; retain observable state/resource expectations.
- [ ] **F-AD-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to synthesizer/DSP library: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire synthesizer/DSP library into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for synthesizer/DSP library as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-022"></a>
## F-AD-022 — media metadata

**Original requirement:** safe parsing, tags/art, provenance, privacy and indexing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-022.01 — Reconcile existing media metadata**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for media metadata. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: safe parsing, tags/art, provenance, privacy and indexing
- [ ] **F-AD-022.02 — Freeze the exact contract for media metadata**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: safe parsing, tags/art, provenance, privacy and indexing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-022.03 — Implement/prove: safe parsing**
  - Action: For media metadata, implement or reuse and verify this exact obligation: safe parsing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe parsing; retain observable state/resource expectations.
- [ ] **F-AD-022.04 — Implement/prove: tags/art**
  - Action: For media metadata, implement or reuse and verify this exact obligation: tags/art. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tags/art; retain observable state/resource expectations.
- [ ] **F-AD-022.05 — Implement/prove: provenance**
  - Action: For media metadata, implement or reuse and verify this exact obligation: provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provenance; retain observable state/resource expectations.
- [ ] **F-AD-022.06 — Implement/prove: privacy and indexing**
  - Action: For media metadata, implement or reuse and verify this exact obligation: privacy and indexing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and indexing; retain observable state/resource expectations.
- [ ] **F-AD-022.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to media metadata: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-022.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-022.08 — Integrate into the real consumer and runtime route**
  - Action: Wire media metadata into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-022.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-022.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for media metadata as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-022.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-023"></a>
## F-AD-023 — media library/index

**Original requirement:** per-user catalog, removable media, duplicates, search and permission-aware thumbnails

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-023.01 — Reconcile existing media library/index**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for media library/index. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-user catalog, removable media, duplicates, search and permission-aware thumbnails
- [ ] **F-AD-023.02 — Freeze the exact contract for media library/index**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-user catalog, removable media, duplicates, search and permission-aware thumbnails. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-023.03 — Implement/prove: per-user catalog**
  - Action: For media library/index, implement or reuse and verify this exact obligation: per-user catalog. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user catalog; retain observable state/resource expectations.
- [ ] **F-AD-023.04 — Implement/prove: removable media**
  - Action: For media library/index, implement or reuse and verify this exact obligation: removable media. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for removable media; retain observable state/resource expectations.
- [ ] **F-AD-023.05 — Implement/prove: duplicates**
  - Action: For media library/index, implement or reuse and verify this exact obligation: duplicates. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for duplicates; retain observable state/resource expectations.
- [ ] **F-AD-023.06 — Implement/prove: search and permission-aware thumbnails**
  - Action: For media library/index, implement or reuse and verify this exact obligation: search and permission-aware thumbnails. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search and permission-aware thumbnails; retain observable state/resource expectations.
- [ ] **F-AD-023.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to media library/index: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-023.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-023.08 — Integrate into the real consumer and runtime route**
  - Action: Wire media library/index into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-023.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-023.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for media library/index as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-023.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-024"></a>
## F-AD-024 — media hardware acceleration

**Original requirement:** optional capability-negotiated decode/encode with software fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-024.01 — Reconcile existing media hardware acceleration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for media hardware acceleration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: optional capability-negotiated decode/encode with software fallback
- [ ] **F-AD-024.02 — Freeze the exact contract for media hardware acceleration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: optional capability-negotiated decode/encode with software fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-024.03 — Implement/prove: optional capability-negotiated decode/encode with software fallback**
  - Action: For media hardware acceleration, implement or reuse and verify this exact obligation: optional capability-negotiated decode/encode with software fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for optional capability-negotiated decode/encode with software fallback; retain observable state/resource expectations.
- [ ] **F-AD-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to media hardware acceleration: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire media hardware acceleration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for media hardware acceleration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ad-025"></a>
## F-AD-025 — audio diagnostics

**Original requirement:** devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AD-025.01 — Reconcile existing audio diagnostics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio diagnostics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-and-evidence comparison against the complete requirement: devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt
- [ ] **F-AD-025.02 — Freeze the exact contract for audio diagnostics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AD-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AD-025.03 — Implement/prove: devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt**
  - Action: For audio diagnostics, implement or reuse and verify this exact obligation: devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AD-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for devices/routes/formats/latency/xruns/clocks and deterministic loopback receipt; retain observable state/resource expectations.
- [ ] **F-AD-025.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio diagnostics: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AD-025.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AD-025.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio diagnostics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AD-025.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AD-025.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio diagnostics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AD-025.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p5-4"></a>
## C-P5.4 — Audio service and provider ABI

**Original requirement:** Audio service and provider ABI

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 300.

### Preserved original contract

- **Dependencies/current/provenance:** P4 and driver DMA primitives; Serenity/duckOS audio servers, asm.fm probes; reject app-direct hardware and offline WAV as architecture.
- **I/O and state:** stream handle, negotiated format/rate/channels and bounded ring in; mixed provider frames and counters out; stream `Created -> Configured -> Running -> Draining -> Closed/Error`.
- **Invariants/failure:** bounded queue; disconnect cleanup; clipping/resampling defined; underrun/overrun counted; provider failure degrades without blocking clients forever.
- **Deterministic proof:** integer tone/golden properties, format negotiation, mix/clipping, ring wrap, underrun/overrun, client/provider crash, controlled clock latency.
- **Target proof:** QEMU/null/file provider then physical HDA stream/reset/recovery with audio/device receipt.
- **Receipt/removal:** input/output hashes, timing/counters/provider state; silent/null provider rollback; remove direct audio calls after all clients migrate.

### Execution steps

- [ ] **C-P5.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.4.02 — Resolve this contract's exact dependencies**
  - Action: P4 and driver DMA primitives; Serenity/duckOS audio servers, asm.fm probes; reject app-direct hardware and offline WAV as architecture. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.4.03 — I/O and state — Audio service and provider ABI**
  - Action: stream handle, negotiated format/rate/channels and bounded ring in; mixed provider frames and counters out; stream `Created -> Configured -> Running -> Draining -> Closed/Error`.
  - Requires: C-P5.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.4.
- [ ] **C-P5.4.04 — Invariants/failure — Audio service and provider ABI**
  - Action: bounded queue; disconnect cleanup; clipping/resampling defined; underrun/overrun counted; provider failure degrades without blocking clients forever.
  - Requires: C-P5.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.4.
- [ ] **C-P5.4.05 — Deterministic proof — Audio service and provider ABI**
  - Action: integer tone/golden properties, format negotiation, mix/clipping, ring wrap, underrun/overrun, client/provider crash, controlled clock latency.
  - Requires: C-P5.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.4.
- [ ] **C-P5.4.06 — Target proof — Audio service and provider ABI**
  - Action: QEMU/null/file provider then physical HDA stream/reset/recovery with audio/device receipt.
  - Requires: C-P5.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.4.
- [ ] **C-P5.4.07 — Receipt/removal — Audio service and provider ABI**
  - Action: input/output hashes, timing/counters/provider state; silent/null provider rollback; remove direct audio calls after all clients migrate.
  - Requires: C-P5.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.4.
- [ ] **C-P5.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-14"></a>
## C-DA-14 — audio provider and AudioServer

**Original requirement:** audio provider and AudioServer

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 354.

### Preserved original contract

**Depends on:** DA-07 and session authority.

**Deliver:** fake provider first, then HDA or `virtio-snd`; negotiated PCM formats,
bounded per-client rings, monotonic presentation clock, mixer, volume/mute, route,
underrun/overrun counters and explicit microphone permission/indicator.

**Invariants:** clients never access hardware; sample arithmetic clips by declared
policy; chunking does not alter deterministic integer output; capture cannot start
without a visible permission state; device reset preserves ownership.

**Proof:** asm.fm-derived original golden vectors, silence/max/mixed formats,
chunk-size invariance, ring under/overrun, client death, route change, clock drift,
reset, simultaneous clients and denied microphone access.

### Execution steps

- [ ] **C-DA-14.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-14.02 — Resolve this contract's exact dependencies**
  - Action: DA-07 and session authority. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-14.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-14.03 — Deliver — audio provider and AudioServer**
  - Action: fake provider first, then HDA or `virtio-snd`; negotiated PCM formats, bounded per-client rings, monotonic presentation clock, mixer, volume/mute, route, underrun/overrun counters and explicit microphone permission/indicator.
  - Requires: C-DA-14.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-14.
- [ ] **C-DA-14.04 — Invariants — audio provider and AudioServer**
  - Action: clients never access hardware; sample arithmetic clips by declared policy; chunking does not alter deterministic integer output; capture cannot start without a visible permission state; device reset preserves ownership.
  - Requires: C-DA-14.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-14.
- [ ] **C-DA-14.05 — Proof — audio provider and AudioServer**
  - Action: asm.fm-derived original golden vectors, silence/max/mixed formats, chunk-size invariance, ring under/overrun, client death, route change, clock drift, reset, simultaneous clients and denied microphone access.
  - Requires: C-DA-14.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-14.
- [ ] **C-DA-14.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-14. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-14.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-blk-010"></a>
## T-BLK-010 — USB MSC UAS

**Original requirement:** streams/tags/task management and fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 115.

### Execution steps

- [ ] **T-BLK-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB MSC UAS to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-BLK-010.02 — Specify the complete target boundary**
  - Action: USB MSC UAS must supply: streams/tags/task management and fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-BLK-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-BLK-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB MSC UAS through the shared platform contract, delivering every part of: streams/tags/task management and fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-BLK-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-BLK-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: torn/reordered writes; power loss at every commit boundary; full disk; corrupt length/path/link metadata; media removal; denied ownership; cancel/crash during update; old/new recovery.
  - Requires: T-BLK-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB MSC UAS.
- [ ] **T-BLK-010.05 — Qualify and retain this target's own result**
  - Action: Bind USB MSC UAS to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-BLK-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-001"></a>
## T-USB-001 — UHCI host

**Original requirement:** schedule/control/bulk/interrupt and teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 136.

### Execution steps

- [ ] **T-USB-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve UHCI host to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-001.02 — Specify the complete target boundary**
  - Action: UHCI host must supply: schedule/control/bulk/interrupt and teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse UHCI host through the shared platform contract, delivering every part of: schedule/control/bulk/interrupt and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for UHCI host.
- [ ] **T-USB-001.05 — Qualify and retain this target's own result**
  - Action: Bind UHCI host to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-002"></a>
## T-USB-002 — OHCI host

**Original requirement:** descriptors, root hub and recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 137.

### Execution steps

- [ ] **T-USB-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve OHCI host to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-002.02 — Specify the complete target boundary**
  - Action: OHCI host must supply: descriptors, root hub and recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse OHCI host through the shared platform contract, delivering every part of: descriptors, root hub and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for OHCI host.
- [ ] **T-USB-002.05 — Qualify and retain this target's own result**
  - Action: Bind OHCI host to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-003"></a>
## T-USB-003 — EHCI host

**Original requirement:** async/periodic schedules and companion handoff

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 138.

### Execution steps

- [ ] **T-USB-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve EHCI host to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-003.02 — Specify the complete target boundary**
  - Action: EHCI host must supply: async/periodic schedules and companion handoff. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse EHCI host through the shared platform contract, delivering every part of: async/periodic schedules and companion handoff. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for EHCI host.
- [ ] **T-USB-003.05 — Qualify and retain this target's own result**
  - Action: Bind EHCI host to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-007"></a>
## T-USB-007 — USB Audio class 1/2

**Original requirement:** descriptors, rates, isochronous recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 142.

### Execution steps

- [ ] **T-USB-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB Audio class 1/2 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-007.02 — Specify the complete target boundary**
  - Action: USB Audio class 1/2 must supply: descriptors, rates, isochronous recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB Audio class 1/2 through the shared platform contract, delivering every part of: descriptors, rates, isochronous recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB Audio class 1/2.
- [ ] **T-USB-007.05 — Qualify and retain this target's own result**
  - Action: Bind USB Audio class 1/2 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-usb-008"></a>
## T-USB-008 — USB Video class

**Original requirement:** probe/commit, formats, isochronous frames

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 143.

### Execution steps

- [ ] **T-USB-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB Video class to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-008.02 — Specify the complete target boundary**
  - Action: USB Video class must supply: probe/commit, formats, isochronous frames. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB Video class through the shared platform contract, delivering every part of: probe/commit, formats, isochronous frames. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB Video class.
- [ ] **T-USB-008.05 — Qualify and retain this target's own result**
  - Action: Bind USB Video class to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-001"></a>
## T-MEDIA-001 — Intel/PCI HDA controller

**Original requirement:** codecs/widgets/routes/DMA/IRQ/reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 230.

### Execution steps

- [ ] **T-MEDIA-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel/PCI HDA controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-001.02 — Specify the complete target boundary**
  - Action: Intel/PCI HDA controller must supply: codecs/widgets/routes/DMA/IRQ/reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel/PCI HDA controller through the shared platform contract, delivering every part of: codecs/widgets/routes/DMA/IRQ/reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel/PCI HDA controller.
- [ ] **T-MEDIA-001.05 — Qualify and retain this target's own result**
  - Action: Bind Intel/PCI HDA controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-002"></a>
## T-MEDIA-002 — AC97

**Original requirement:** codecs/mixer/DMA/underrun

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 231.

### Execution steps

- [ ] **T-MEDIA-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve AC97 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-002.02 — Specify the complete target boundary**
  - Action: AC97 must supply: codecs/mixer/DMA/underrun. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse AC97 through the shared platform contract, delivering every part of: codecs/mixer/DMA/underrun. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for AC97.
- [ ] **T-MEDIA-002.05 — Qualify and retain this target's own result**
  - Action: Bind AC97 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-003"></a>
## T-MEDIA-003 — virtio-snd

**Original requirement:** negotiation/streams/queues/reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 232.

### Execution steps

- [ ] **T-MEDIA-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio-snd to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-003.02 — Specify the complete target boundary**
  - Action: virtio-snd must supply: negotiation/streams/queues/reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio-snd through the shared platform contract, delivering every part of: negotiation/streams/queues/reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio-snd.
- [ ] **T-MEDIA-003.05 — Qualify and retain this target's own result**
  - Action: Bind virtio-snd to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-005"></a>
## T-MEDIA-005 — PC speaker

**Original requirement:** bounded tone service and no global blocking

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 234.

### Execution steps

- [ ] **T-MEDIA-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PC speaker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-005.02 — Specify the complete target boundary**
  - Action: PC speaker must supply: bounded tone service and no global blocking. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse PC speaker through the shared platform contract, delivering every part of: bounded tone service and no global blocking. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PC speaker.
- [ ] **T-MEDIA-005.05 — Qualify and retain this target's own result**
  - Action: Bind PC speaker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-006"></a>
## T-MEDIA-006 — USB Audio

**Original requirement:** UAC formats/clock/isochronous recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 235.

### Execution steps

- [ ] **T-MEDIA-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB Audio to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-006.02 — Specify the complete target boundary**
  - Action: USB Audio must supply: UAC formats/clock/isochronous recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB Audio through the shared platform contract, delivering every part of: UAC formats/clock/isochronous recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB Audio.
- [ ] **T-MEDIA-006.05 — Qualify and retain this target's own result**
  - Action: Bind USB Audio to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-007"></a>
## T-MEDIA-007 — microphone/capture provider

**Original requirement:** privacy indicator, permission and revoke

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 236.

### Execution steps

- [ ] **T-MEDIA-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve microphone/capture provider to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-007.02 — Specify the complete target boundary**
  - Action: microphone/capture provider must supply: privacy indicator, permission and revoke. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse microphone/capture provider through the shared platform contract, delivering every part of: privacy indicator, permission and revoke. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for microphone/capture provider.
- [ ] **T-MEDIA-007.05 — Qualify and retain this target's own result**
  - Action: Bind microphone/capture provider to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-media-008"></a>
## T-MEDIA-008 — UVC camera

**Original requirement:** formats/frame buffers/timestamps/unplug

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 237.

### Execution steps

- [ ] **T-MEDIA-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve UVC camera to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-MEDIA-008.02 — Specify the complete target boundary**
  - Action: UVC camera must supply: formats/frame buffers/timestamps/unplug. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-MEDIA-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-MEDIA-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse UVC camera through the shared platform contract, delivering every part of: formats/frame buffers/timestamps/unplug. Do not fork a duplicate subsystem for this row.
  - Requires: T-MEDIA-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-MEDIA-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-MEDIA-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for UVC camera.
- [ ] **T-MEDIA-008.05 — Qualify and retain this target's own result**
  - Action: Bind UVC camera to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-MEDIA-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-090"></a>
## T-SVC-090 — Audio Server

**Original requirement:** device ownership, mixing, shared rings, routing and recovery

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 112.

### Execution steps

- [ ] **T-SVC-090.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Audio Server to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-090.02 — Specify the complete target boundary**
  - Action: Audio Server must supply: device ownership, mixing, shared rings, routing and recovery. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-090.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-090.03 — Implement the exact target behavior**
  - Action: Implement or reuse Audio Server through the shared platform contract, delivering every part of: device ownership, mixing, shared rings, routing and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-090.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-090.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-SVC-090.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Audio Server.
- [ ] **T-SVC-090.05 — Qualify and retain this target's own result**
  - Action: Bind Audio Server to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-090.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-091"></a>
## T-SVC-091 — Media Clock Service

**Original requirement:** stream timestamps, sync, drift and pause/seek

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 113.

### Execution steps

- [ ] **T-SVC-091.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Media Clock Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-091.02 — Specify the complete target boundary**
  - Action: Media Clock Service must supply: stream timestamps, sync, drift and pause/seek. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-091.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-091.03 — Implement the exact target behavior**
  - Action: Implement or reuse Media Clock Service through the shared platform contract, delivering every part of: stream timestamps, sync, drift and pause/seek. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-091.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-091.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-SVC-091.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Media Clock Service.
- [ ] **T-SVC-091.05 — Qualify and retain this target's own result**
  - Action: Bind Media Clock Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-091.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-092"></a>
## T-SVC-092 — Codec/Decoder Worker Pool

**Original requirement:** isolated image/audio/video/font/document parsers

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 114.

### Execution steps

- [ ] **T-SVC-092.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Codec/Decoder Worker Pool to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-092.02 — Specify the complete target boundary**
  - Action: Codec/Decoder Worker Pool must supply: isolated image/audio/video/font/document parsers. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-092.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-092.03 — Implement the exact target behavior**
  - Action: Implement or reuse Codec/Decoder Worker Pool through the shared platform contract, delivering every part of: isolated image/audio/video/font/document parsers. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-092.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-092.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-SVC-092.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Codec/Decoder Worker Pool.
- [ ] **T-SVC-092.05 — Qualify and retain this target's own result**
  - Action: Bind Codec/Decoder Worker Pool to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-092.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-093"></a>
## T-SVC-093 — Camera/Capture Service

**Original requirement:** frames, formats, permission, indicator and revoke

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 115.

### Execution steps

- [ ] **T-SVC-093.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Camera/Capture Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-16, H-09.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-093.02 — Specify the complete target boundary**
  - Action: Camera/Capture Service must supply: frames, formats, permission, indicator and revoke. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-093.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-093.03 — Implement the exact target behavior**
  - Action: Implement or reuse Camera/Capture Service through the shared platform contract, delivering every part of: frames, formats, permission, indicator and revoke. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-093.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-093.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: underrun/overrun; incompatible format/rate; clock drift; hot unplug; capture denial/revocation; crashed client with mapped ring; suspend and reconnect.
  - Requires: T-SVC-093.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Camera/Capture Service.
- [ ] **T-SVC-093.05 — Qualify and retain this target's own result**
  - Action: Bind Camera/Capture Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-093.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
