# MP-07: Make the desktop an owned, recoverable service

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/graphics/; kernel/src/drivers/display/; kernel/src/drivers/input/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-07` exports: Owned surfaces, input/focus and a bounded compositor protocol with a working software fallback.

The handoff enables only its named subset. `CLOSE-07` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-07.01 — Preserve the selected PRESSWORK behavior and measured software renderer

Preserve the selected PRESSWORK behavior and measured software renderer; reconcile older visual proposals before adopting any change.

**Requires:** `D-01`, `D-02`, `D-13`, `D-14`, `H-00`, `H-01`, `H-03`, `H-04`, `H-05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.02 — Define display provider format, pitch, mode, color, scale, fence and failure contracts

Define display provider format, pitch, mode, color, scale, fence and failure contracts.

**Requires:** `M-07.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.03 — Implement opaque surface ownership, bounded damage/regions, clipping, occlusion and atomic commits

Implement opaque surface ownership, bounded damage/regions, clipping, occlusion and atomic commits.

**Requires:** `M-07.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.04 — Retain client content, shell/chrome, glyphs and icons with explicit invalidation and memory pressure behavior

Retain client content, shell/chrome, glyphs and icons with explicit invalidation and memory pressure behavior.

**Requires:** `M-07.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.05 — Separate global input, focus, grabs, secure attention and scanout authority from untrusted application processes

Separate global input, focus, grabs, secure attention and scanout authority from untrusted application processes.

**Requires:** `M-07.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.06 — Normalize multi-device input, key state, repeat, touch/stylus/gamepad, IME and queue overflow/recovery

Normalize multi-device input, key state, repeat, touch/stylus/gamepad, IME and queue overflow/recovery.

**Requires:** `M-07.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-07 — Bounded development handoff: Make the desktop an owned, recoverable service

Owned surfaces, input/focus and a bounded compositor protocol with a working software fallback.

**Requires:** `M-07.06`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-07.07 — Move Window/Input/Session/shell owners behind the service boundary in dependency order while preserving a recovery console

Move Window/Input/Session/shell owners behind the service boundary in dependency order while preserving a recovery console.

**Requires:** `M-07.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.08 — Implement multi-display/workspaces, lock/login/logout, task switching, notifications and restart recovery

Implement multi-display/workspaces, lock/login/logout, task switching, notifications and restart recovery.

**Requires:** `M-07.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.09 — Compare software and accelerated providers with semantic pixel/interaction oracles and measured frame/input distributions

Compare software and accelerated providers with semantic pixel/interaction oracles and measured frame/input distributions.

**Requires:** `M-07.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-07.10 — For Intel and other physical display paths, define panel power, timing, recovery and explicit hardware-operation approval before writes

For Intel and other physical display paths, define panel power, timing, recovery and explicit hardware-operation approval before writes.

**Requires:** `M-07.09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-GR-001](#f-gr-001) | feature | display-provider contract |
| [F-GR-002](#f-gr-002) | feature | mode discovery/selection |
| [F-GR-003](#f-gr-003) | feature | multi-display topology |
| [F-GR-004](#f-gr-004) | feature | color format conversion |
| [F-GR-005](#f-gr-005) | feature | color management |
| [F-GR-006](#f-gr-006) | feature | compositor-exclusive scanout |
| [F-GR-007](#f-gr-007) | feature | opaque surface/window handles |
| [F-GR-008](#f-gr-008) | feature | surface admission |
| [F-GR-009](#f-gr-009) | feature | double-buffer protocol |
| [F-GR-010](#f-gr-010) | feature | retained client surfaces |
| [F-GR-011](#f-gr-011) | feature | retained shell/chrome |
| [F-GR-012](#f-gr-012) | feature | retained scene graph |
| [F-GR-013](#f-gr-013) | feature | damage regions |
| [F-GR-014](#f-gr-014) | feature | occlusion calculation |
| [F-GR-015](#f-gr-015) | feature | hierarchical clipping |
| [F-GR-016](#f-gr-016) | feature | atomic scene commit |
| [F-GR-017](#f-gr-017) | feature | one-present frame |
| [F-GR-018](#f-gr-018) | feature | frame scheduler |
| [F-GR-019](#f-gr-019) | feature | vblank/present timing |
| [F-GR-020](#f-gr-020) | feature | frame attribution |
| [F-GR-021](#f-gr-021) | feature | input-to-present tracing |
| [F-GR-022](#f-gr-022) | feature | worst-frame gate |
| [F-GR-023](#f-gr-023) | feature | framebuffer mapping policy |
| [F-GR-024](#f-gr-024) | feature | software renderer oracle |
| [F-GR-025](#f-gr-025) | feature | GPU renderer backend |
| [F-GR-026](#f-gr-026) | feature | alpha composition |
| [F-GR-027](#f-gr-027) | feature | rounded geometry |
| [F-GR-028](#f-gr-028) | feature | shadows/elevation |
| [F-GR-029](#f-gr-029) | feature | gradients/decorative fields |
| [F-GR-030](#f-gr-030) | feature | backdrop blur |
| [F-GR-031](#f-gr-031) | feature | wallpaper cache |
| [F-GR-032](#f-gr-032) | feature | icon/image cache |
| [F-GR-033](#f-gr-033) | feature | glyph/shaping cache |
| [F-GR-034](#f-gr-034) | feature | layout cache |
| [F-GR-035](#f-gr-035) | feature | virtualized rendering |
| [F-GR-036](#f-gr-036) | feature | resize interaction |
| [F-GR-037](#f-gr-037) | feature | move/drag interaction |
| [F-GR-038](#f-gr-038) | feature | window stacking/focus |
| [F-GR-039](#f-gr-039) | feature | workspaces |
| [F-GR-040](#f-gr-040) | feature | fullscreen |
| [F-GR-041](#f-gr-041) | feature | direct scanout |
| [F-GR-042](#f-gr-042) | feature | hardware cursor |
| [F-GR-043](#f-gr-043) | feature | screenshot/capture portal |
| [F-GR-044](#f-gr-044) | feature | screen recording |
| [F-GR-045](#f-gr-045) | feature | remote composition mode |
| [F-GR-046](#f-gr-046) | feature | graphics diagnostics |
| [F-GR-047](#f-gr-047) | feature | graphics recovery |
| [F-GR-048](#f-gr-048) | feature | memory-pressure graphics policy |
| [F-GR-049](#f-gr-049) | feature | reduced-quality mode |
| [F-GR-050](#f-gr-050) | feature | visual/frame regression lab |
| [F-IN-001](#f-in-001) | feature | normalized input event |
| [F-IN-002](#f-in-002) | feature | raw input stream |
| [F-IN-003](#f-in-003) | feature | compositor hit testing |
| [F-IN-004](#f-in-004) | feature | keyboard focus |
| [F-IN-005](#f-in-005) | feature | pointer focus |
| [F-IN-006](#f-in-006) | feature | input grabs/capture |
| [F-IN-007](#f-in-007) | feature | global shortcut policy |
| [F-IN-008](#f-in-008) | feature | key repeat |
| [F-IN-009](#f-in-009) | feature | keyboard layouts |
| [F-IN-010](#f-in-010) | feature | input methods |
| [F-IN-011](#f-in-011) | feature | pointer acceleration |
| [F-IN-012](#f-in-012) | feature | pointer confinement/lock |
| [F-IN-013](#f-in-013) | feature | wheel/high-resolution scroll |
| [F-IN-014](#f-in-014) | feature | drag and drop |
| [F-IN-015](#f-in-015) | feature | touch gestures |
| [F-IN-016](#f-in-016) | feature | stylus interaction |
| [F-IN-017](#f-in-017) | feature | gamepad mapping |
| [F-IN-018](#f-in-018) | feature | text input/selection |
| [F-IN-019](#f-in-019) | feature | context menu |
| [F-IN-020](#f-in-020) | feature | tooltip/help affordance |
| [F-IN-021](#f-in-021) | feature | coarse-pointer mode |
| [F-IN-022](#f-in-022) | feature | device settings |
| [F-IN-023](#f-in-023) | feature | input privacy |
| [F-IN-024](#f-in-024) | feature | input diagnostics |
| [F-IN-025](#f-in-025) | feature | input latency receipt |
| [F-SH-001](#f-sh-001) | feature | login experience |
| [F-SH-002](#f-sh-002) | feature | session startup |
| [F-SH-003](#f-sh-003) | feature | lock/unlock |
| [F-SH-004](#f-sh-004) | feature | logout/shutdown/restart UX |
| [F-SH-005](#f-sh-005) | feature | desktop background |
| [F-SH-006](#f-sh-006) | feature | top/status bar |
| [F-SH-007](#f-sh-007) | feature | dock/taskbar |
| [F-SH-008](#f-sh-008) | feature | application menu |
| [F-SH-009](#f-sh-009) | feature | launcher/search |
| [F-SH-010](#f-sh-010) | feature | Run command |
| [F-SH-011](#f-sh-011) | feature | All Applications catalog |
| [F-SH-012](#f-sh-012) | feature | window create/close |
| [F-SH-013](#f-sh-013) | feature | window move |
| [F-SH-014](#f-sh-014) | feature | window resize |
| [F-SH-015](#f-sh-015) | feature | minimize/restore |
| [F-SH-016](#f-sh-016) | feature | maximize/restore |
| [F-SH-017](#f-sh-017) | feature | snap/tile |
| [F-SH-018](#f-sh-018) | feature | fullscreen |
| [F-SH-019](#f-sh-019) | feature | window switcher |
| [F-SH-020](#f-sh-020) | feature | workspace overview |
| [F-SH-021](#f-sh-021) | feature | multiple workspaces |
| [F-SH-022](#f-sh-022) | feature | modal/transient windows |
| [F-SH-023](#f-sh-023) | feature | notification service |
| [F-SH-024](#f-sh-024) | feature | notification center |
| [F-SH-025](#f-sh-025) | feature | clipboard service |
| [F-SH-026](#f-sh-026) | feature | clipboard history UI |
| [F-SH-027](#f-sh-027) | feature | drag-and-drop broker |
| [F-SH-028](#f-sh-028) | feature | file chooser portal |
| [F-SH-029](#f-sh-029) | feature | open-with/share portal |
| [F-SH-030](#f-sh-030) | feature | screen-capture portal |
| [F-SH-031](#f-sh-031) | feature | permission prompt |
| [F-SH-032](#f-sh-032) | feature | quick settings |
| [F-SH-033](#f-sh-033) | feature | system tray/status items |
| [F-SH-034](#f-sh-034) | feature | clock/calendar panel |
| [F-SH-035](#f-sh-035) | feature | power/session menu |
| [F-SH-036](#f-sh-036) | feature | onboarding |
| [F-SH-037](#f-sh-037) | feature | help system |
| [F-SH-038](#f-sh-038) | feature | crash UI |
| [F-SH-039](#f-sh-039) | feature | busy/unresponsive UI |
| [F-SH-040](#f-sh-040) | feature | session restore |
| [F-SH-041](#f-sh-041) | feature | multi-user switching |
| [F-SH-042](#f-sh-042) | feature | guest session |
| [F-SH-043](#f-sh-043) | feature | kiosk/public-demo session |
| [F-SH-044](#f-sh-044) | feature | remote session |
| [F-SH-045](#f-sh-045) | feature | shell performance telemetry |
| [C-P5.3](#c-p5-3) | contract | Window and Input services |
| [C-P8.4](#c-p8-4) | contract | Intel render/composition provider |
| [C-DA-09](#c-da-09) | contract | display provider |
| [C-DA-10](#c-da-10) | contract | normalized input provider |
| [C-DA-20](#c-da-20) | contract | session, compositor and surface authority |
| [C-VX-11](#c-vx-11) | contract | Window/surface protocol |
| [C-VX-12](#c-vx-12) | contract | Compositor scene and recovery |
| [C-VX-13](#c-vx-13) | contract | Session shell and lifecycle |
| [C-VX-14](#c-vx-14) | contract | Window interaction and multi-monitor |
| [T-USB-005](#t-usb-005) | target | USB HID class |
| [T-INPUT-001](#t-input-001) | target | PS/2 controller |
| [T-INPUT-002](#t-input-002) | target | PS/2 keyboard |
| [T-INPUT-003](#t-input-003) | target | PS/2 mouse |
| [T-INPUT-004](#t-input-004) | target | USB HID keyboard |
| [T-INPUT-005](#t-input-005) | target | USB HID mouse |
| [T-INPUT-006](#t-input-006) | target | generic HID report parser |
| [T-INPUT-009](#t-input-009) | target | I2C-HID transport and decoder |
| [T-INPUT-010](#t-input-010) | target | virtio-input/tablet |
| [T-GPU-001](#t-gpu-001) | target | software raster/compositor oracle |
| [T-GPU-002](#t-gpu-002) | target | UEFI GOP framebuffer |
| [T-GPU-004](#t-gpu-004) | target | VGA text/planar fallback |
| [T-GPU-005](#t-gpu-005) | target | Bochs/BGA display |
| [T-GPU-006](#t-gpu-006) | target | QEMU ramfb/simplefb |
| [T-GPU-007](#t-gpu-007) | target | virtio-gpu 2D/KMS |
| [T-GPU-011](#t-gpu-011) | target | Intel Gen9/9.5 display |
| [T-GPU-012](#t-gpu-012) | target | Intel blitter/render rings |
| [T-GPU-018](#t-gpu-018) | target | hardware cursor contract |
| [T-GPU-019](#t-gpu-019) | target | display connector/EDID/DP/HDMI core |
| [T-SVC-070](#t-svc-070) | target | Display Server/Compositor |
| [T-SVC-071](#t-svc-071) | target | Window Manager Policy |
| [T-SVC-072](#t-svc-072) | target | Input Service |
| [T-SVC-073](#t-svc-073) | target | Seat/Focus/Grab Service |
| [T-SVC-074](#t-svc-074) | target | Login/Lock/Greeter Service |
| [T-SVC-075](#t-svc-075) | target | Shell/Desktop Service |
| [T-APP-002](#t-app-002) | target | Login/Greeter |
| [T-APP-003](#t-app-003) | target | Lock Screen |

<a id="f-gr-001"></a>
## F-GR-001 — display-provider contract

**Original requirement:** mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-001.01 — Reconcile existing display-provider contract**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for display-provider contract. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API
- [ ] **F-GR-001.02 — Freeze the exact contract for display-provider contract**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-001.03 — Implement/prove: mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API**
  - Action: For display-provider contract, implement or reuse and verify this exact obligation: mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mode/format/refresh/scanout/fence/cursor/capture/recovery behind one provider API; retain observable state/resource expectations.
- [ ] **F-GR-001.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to display-provider contract: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-001.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-001.05 — Integrate into the real consumer and runtime route**
  - Action: Wire display-provider contract into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-001.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-001.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for display-provider contract as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-001.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-002"></a>
## F-GR-002 — mode discovery/selection

**Original requirement:** connector/EDID/mode validity, preferred/fallback, safe apply and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-002.01 — Reconcile existing mode discovery/selection**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mode discovery/selection. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: connector/EDID/mode validity, preferred/fallback, safe apply and rollback
- [ ] **F-GR-002.02 — Freeze the exact contract for mode discovery/selection**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: connector/EDID/mode validity, preferred/fallback, safe apply and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-002.03 — Implement/prove: connector/EDID/mode validity**
  - Action: For mode discovery/selection, implement or reuse and verify this exact obligation: connector/EDID/mode validity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for connector/EDID/mode validity; retain observable state/resource expectations.
- [ ] **F-GR-002.04 — Implement/prove: preferred/fallback**
  - Action: For mode discovery/selection, implement or reuse and verify this exact obligation: preferred/fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preferred/fallback; retain observable state/resource expectations.
- [ ] **F-GR-002.05 — Implement/prove: safe apply and rollback**
  - Action: For mode discovery/selection, implement or reuse and verify this exact obligation: safe apply and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-002.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe apply and rollback; retain observable state/resource expectations.
- [ ] **F-GR-002.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mode discovery/selection: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-002.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-002.07 — Integrate into the real consumer and runtime route**
  - Action: Wire mode discovery/selection into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-002.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-002.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mode discovery/selection as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-002.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-003"></a>
## F-GR-003 — multi-display topology

**Original requirement:** position, primary, scale, rotation, mirror/extend, hotplug and per-display refresh

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-003.01 — Reconcile existing multi-display topology**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for multi-display topology. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: position, primary, scale, rotation, mirror/extend, hotplug and per-display refresh
- [ ] **F-GR-003.02 — Freeze the exact contract for multi-display topology**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: position, primary, scale, rotation, mirror/extend, hotplug and per-display refresh. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-003.03 — Implement/prove: position**
  - Action: For multi-display topology, implement or reuse and verify this exact obligation: position. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for position; retain observable state/resource expectations.
- [ ] **F-GR-003.04 — Implement/prove: primary**
  - Action: For multi-display topology, implement or reuse and verify this exact obligation: primary. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for primary; retain observable state/resource expectations.
- [ ] **F-GR-003.05 — Implement/prove: scale**
  - Action: For multi-display topology, implement or reuse and verify this exact obligation: scale. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scale; retain observable state/resource expectations.
- [ ] **F-GR-003.06 — Implement/prove: rotation**
  - Action: For multi-display topology, implement or reuse and verify this exact obligation: rotation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-003.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rotation; retain observable state/resource expectations.
- [ ] **F-GR-003.07 — Implement/prove: mirror/extend**
  - Action: For multi-display topology, implement or reuse and verify this exact obligation: mirror/extend. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-003.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mirror/extend; retain observable state/resource expectations.
- [ ] **F-GR-003.08 — Implement/prove: hotplug and per-display refresh**
  - Action: For multi-display topology, implement or reuse and verify this exact obligation: hotplug and per-display refresh. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-003.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug and per-display refresh; retain observable state/resource expectations.
- [ ] **F-GR-003.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to multi-display topology: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-003.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-003.10 — Integrate into the real consumer and runtime route**
  - Action: Wire multi-display topology into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-003.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-003.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for multi-display topology as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-003.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-004"></a>
## F-GR-004 — color format conversion

**Original requirement:** checked RGB/BGR/565/8888 conversions, alpha rules and differential tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-004.01 — Reconcile existing color format conversion**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for color format conversion. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked RGB/BGR/565/8888 conversions, alpha rules and differential tests
- [ ] **F-GR-004.02 — Freeze the exact contract for color format conversion**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked RGB/BGR/565/8888 conversions, alpha rules and differential tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-004.03 — Implement/prove: checked RGB/BGR/565/8888 conversions**
  - Action: For color format conversion, implement or reuse and verify this exact obligation: checked RGB/BGR/565/8888 conversions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked RGB/BGR/565/8888 conversions; retain observable state/resource expectations.
- [ ] **F-GR-004.04 — Implement/prove: alpha rules and differential tests**
  - Action: For color format conversion, implement or reuse and verify this exact obligation: alpha rules and differential tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for alpha rules and differential tests; retain observable state/resource expectations.
- [ ] **F-GR-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to color format conversion: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire color format conversion into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for color format conversion as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-005"></a>
## F-GR-005 — color management

**Original requirement:** profiles, transfer functions, gamut, HDR policy and screenshot/export consistency

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-005.01 — Reconcile existing color management**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for color management. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: profiles, transfer functions, gamut, HDR policy and screenshot/export consistency
- [ ] **F-GR-005.02 — Freeze the exact contract for color management**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: profiles, transfer functions, gamut, HDR policy and screenshot/export consistency. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-005.03 — Implement/prove: profiles**
  - Action: For color management, implement or reuse and verify this exact obligation: profiles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for profiles; retain observable state/resource expectations.
- [ ] **F-GR-005.04 — Implement/prove: transfer functions**
  - Action: For color management, implement or reuse and verify this exact obligation: transfer functions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for transfer functions; retain observable state/resource expectations.
- [ ] **F-GR-005.05 — Implement/prove: gamut**
  - Action: For color management, implement or reuse and verify this exact obligation: gamut. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for gamut; retain observable state/resource expectations.
- [ ] **F-GR-005.06 — Implement/prove: HDR policy and screenshot/export consistency**
  - Action: For color management, implement or reuse and verify this exact obligation: HDR policy and screenshot/export consistency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for HDR policy and screenshot/export consistency; retain observable state/resource expectations.
- [ ] **F-GR-005.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to color management: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-005.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-005.08 — Integrate into the real consumer and runtime route**
  - Action: Wire color management into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-005.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-005.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for color management as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-005.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-006"></a>
## F-GR-006 — compositor-exclusive scanout

**Original requirement:** apps never map global scanout; only compositor/provider presents

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-006.01 — Reconcile existing compositor-exclusive scanout**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compositor-exclusive scanout. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: apps never map global scanout; only compositor/provider presents
- [ ] **F-GR-006.02 — Freeze the exact contract for compositor-exclusive scanout**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: apps never map global scanout; only compositor/provider presents. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-006.03 — Implement/prove: apps never map global scanout**
  - Action: For compositor-exclusive scanout, implement or reuse and verify this exact obligation: apps never map global scanout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for apps never map global scanout; retain observable state/resource expectations.
- [ ] **F-GR-006.04 — Implement/prove: only compositor/provider presents**
  - Action: For compositor-exclusive scanout, implement or reuse and verify this exact obligation: only compositor/provider presents. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for only compositor/provider presents; retain observable state/resource expectations.
- [ ] **F-GR-006.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compositor-exclusive scanout: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-006.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-006.06 — Integrate into the real consumer and runtime route**
  - Action: Wire compositor-exclusive scanout into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-006.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-006.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compositor-exclusive scanout as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-006.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-007"></a>
## F-GR-007 — opaque surface/window handles

**Original requirement:** owner/grantee rights, generations, mapping references, revoke and peer-death cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-007.01 — Reconcile existing opaque surface/window handles**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for opaque surface/window handles. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: owner/grantee rights, generations, mapping references, revoke and peer-death cleanup
- [ ] **F-GR-007.02 — Freeze the exact contract for opaque surface/window handles**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: owner/grantee rights, generations, mapping references, revoke and peer-death cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-007.03 — Implement/prove: owner/grantee rights**
  - Action: For opaque surface/window handles, implement or reuse and verify this exact obligation: owner/grantee rights. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owner/grantee rights; retain observable state/resource expectations.
- [ ] **F-GR-007.04 — Implement/prove: generations**
  - Action: For opaque surface/window handles, implement or reuse and verify this exact obligation: generations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generations; retain observable state/resource expectations.
- [ ] **F-GR-007.05 — Implement/prove: mapping references**
  - Action: For opaque surface/window handles, implement or reuse and verify this exact obligation: mapping references. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mapping references; retain observable state/resource expectations.
- [ ] **F-GR-007.06 — Implement/prove: revoke and peer-death cleanup**
  - Action: For opaque surface/window handles, implement or reuse and verify this exact obligation: revoke and peer-death cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for revoke and peer-death cleanup; retain observable state/resource expectations.
- [ ] **F-GR-007.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to opaque surface/window handles: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-007.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-007.08 — Integrate into the real consumer and runtime route**
  - Action: Wire opaque surface/window handles into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-007.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-007.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for opaque surface/window handles as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-007.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-008"></a>
## F-GR-008 — surface admission

**Original requirement:** checked geometry, stride, format, bytes, overflow, limits and memory accounting

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-008.01 — Reconcile existing surface admission**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for surface admission. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checked geometry, stride, format, bytes, overflow, limits and memory accounting
- [ ] **F-GR-008.02 — Freeze the exact contract for surface admission**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checked geometry, stride, format, bytes, overflow, limits and memory accounting. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-008.03 — Implement/prove: checked geometry**
  - Action: For surface admission, implement or reuse and verify this exact obligation: checked geometry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checked geometry; retain observable state/resource expectations.
- [ ] **F-GR-008.04 — Implement/prove: stride**
  - Action: For surface admission, implement or reuse and verify this exact obligation: stride. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stride; retain observable state/resource expectations.
- [ ] **F-GR-008.05 — Implement/prove: format**
  - Action: For surface admission, implement or reuse and verify this exact obligation: format. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for format; retain observable state/resource expectations.
- [ ] **F-GR-008.06 — Implement/prove: bytes**
  - Action: For surface admission, implement or reuse and verify this exact obligation: bytes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-008.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bytes; retain observable state/resource expectations.
- [ ] **F-GR-008.07 — Implement/prove: overflow**
  - Action: For surface admission, implement or reuse and verify this exact obligation: overflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-008.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow; retain observable state/resource expectations.
- [ ] **F-GR-008.08 — Implement/prove: limits and memory accounting**
  - Action: For surface admission, implement or reuse and verify this exact obligation: limits and memory accounting. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-008.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for limits and memory accounting; retain observable state/resource expectations.
- [ ] **F-GR-008.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to surface admission: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-008.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-008.10 — Integrate into the real consumer and runtime route**
  - Action: Wire surface admission into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-008.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-008.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for surface admission as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-008.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-009"></a>
## F-GR-009 — double-buffer protocol

**Original requirement:** configure/ack, attach, damage, commit, release, no reuse before release

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-009.01 — Reconcile existing double-buffer protocol**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for double-buffer protocol. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: configure/ack, attach, damage, commit, release, no reuse before release
- [ ] **F-GR-009.02 — Freeze the exact contract for double-buffer protocol**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: configure/ack, attach, damage, commit, release, no reuse before release. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-009.03 — Implement/prove: configure/ack**
  - Action: For double-buffer protocol, implement or reuse and verify this exact obligation: configure/ack. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configure/ack; retain observable state/resource expectations.
- [ ] **F-GR-009.04 — Implement/prove: attach**
  - Action: For double-buffer protocol, implement or reuse and verify this exact obligation: attach. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for attach; retain observable state/resource expectations.
- [ ] **F-GR-009.05 — Implement/prove: damage**
  - Action: For double-buffer protocol, implement or reuse and verify this exact obligation: damage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for damage; retain observable state/resource expectations.
- [ ] **F-GR-009.06 — Implement/prove: commit**
  - Action: For double-buffer protocol, implement or reuse and verify this exact obligation: commit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for commit; retain observable state/resource expectations.
- [ ] **F-GR-009.07 — Implement/prove: release**
  - Action: For double-buffer protocol, implement or reuse and verify this exact obligation: release. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-009.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for release; retain observable state/resource expectations.
- [ ] **F-GR-009.08 — Implement/prove: no reuse before release**
  - Action: For double-buffer protocol, implement or reuse and verify this exact obligation: no reuse before release. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-009.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no reuse before release; retain observable state/resource expectations.
- [ ] **F-GR-009.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to double-buffer protocol: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-009.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-009.10 — Integrate into the real consumer and runtime route**
  - Action: Wire double-buffer protocol into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-009.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-009.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for double-buffer protocol as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-009.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-010"></a>
## F-GR-010 — retained client surfaces

**Original requirement:** unchanged app content survives move/expose/focus without app redraw

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-010.01 — Reconcile existing retained client surfaces**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for retained client surfaces. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: unchanged app content survives move/expose/focus without app redraw
- [ ] **F-GR-010.02 — Freeze the exact contract for retained client surfaces**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: unchanged app content survives move/expose/focus without app redraw. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-010.03 — Implement/prove: unchanged app content survives move/expose/focus without app redraw**
  - Action: For retained client surfaces, implement or reuse and verify this exact obligation: unchanged app content survives move/expose/focus without app redraw. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unchanged app content survives move/expose/focus without app redraw; retain observable state/resource expectations.
- [ ] **F-GR-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to retained client surfaces: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire retained client surfaces into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for retained client surfaces as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-011"></a>
## F-GR-011 — retained shell/chrome

**Original requirement:** borders, shadows, dock, menu and desktop layers invalidate independently

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-011.01 — Reconcile existing retained shell/chrome**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for retained shell/chrome. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: borders, shadows, dock, menu and desktop layers invalidate independently
- [ ] **F-GR-011.02 — Freeze the exact contract for retained shell/chrome**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: borders, shadows, dock, menu and desktop layers invalidate independently. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-011.03 — Implement/prove: borders**
  - Action: For retained shell/chrome, implement or reuse and verify this exact obligation: borders. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for borders; retain observable state/resource expectations.
- [ ] **F-GR-011.04 — Implement/prove: shadows**
  - Action: For retained shell/chrome, implement or reuse and verify this exact obligation: shadows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shadows; retain observable state/resource expectations.
- [ ] **F-GR-011.05 — Implement/prove: dock**
  - Action: For retained shell/chrome, implement or reuse and verify this exact obligation: dock. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dock; retain observable state/resource expectations.
- [ ] **F-GR-011.06 — Implement/prove: menu and desktop layers invalidate independently**
  - Action: For retained shell/chrome, implement or reuse and verify this exact obligation: menu and desktop layers invalidate independently. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for menu and desktop layers invalidate independently; retain observable state/resource expectations.
- [ ] **F-GR-011.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to retained shell/chrome: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-011.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-011.08 — Integrate into the real consumer and runtime route**
  - Action: Wire retained shell/chrome into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-011.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-011.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for retained shell/chrome as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-011.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-012"></a>
## F-GR-012 — retained scene graph

**Original requirement:** immutable/generation scene state, transforms, clips, opacity, resource ownership

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-012.01 — Reconcile existing retained scene graph**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for retained scene graph. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: immutable/generation scene state, transforms, clips, opacity, resource ownership
- [ ] **F-GR-012.02 — Freeze the exact contract for retained scene graph**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: immutable/generation scene state, transforms, clips, opacity, resource ownership. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-012.03 — Implement/prove: immutable/generation scene state**
  - Action: For retained scene graph, implement or reuse and verify this exact obligation: immutable/generation scene state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for immutable/generation scene state; retain observable state/resource expectations.
- [ ] **F-GR-012.04 — Implement/prove: transforms**
  - Action: For retained scene graph, implement or reuse and verify this exact obligation: transforms. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for transforms; retain observable state/resource expectations.
- [ ] **F-GR-012.05 — Implement/prove: clips**
  - Action: For retained scene graph, implement or reuse and verify this exact obligation: clips. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clips; retain observable state/resource expectations.
- [ ] **F-GR-012.06 — Implement/prove: opacity**
  - Action: For retained scene graph, implement or reuse and verify this exact obligation: opacity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-012.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opacity; retain observable state/resource expectations.
- [ ] **F-GR-012.07 — Implement/prove: resource ownership**
  - Action: For retained scene graph, implement or reuse and verify this exact obligation: resource ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-012.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resource ownership; retain observable state/resource expectations.
- [ ] **F-GR-012.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to retained scene graph: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-012.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-012.09 — Integrate into the real consumer and runtime route**
  - Action: Wire retained scene graph into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-012.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-012.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for retained scene graph as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-012.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-013"></a>
## F-GR-013 — damage regions

**Original requirement:** bounded area-aware rectangles, merge policy, cause, full fallback and properties

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-013.01 — Reconcile existing damage regions**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for damage regions. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bounded area-aware rectangles, merge policy, cause, full fallback and properties
- [ ] **F-GR-013.02 — Freeze the exact contract for damage regions**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bounded area-aware rectangles, merge policy, cause, full fallback and properties. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-013.03 — Implement/prove: bounded area-aware rectangles**
  - Action: For damage regions, implement or reuse and verify this exact obligation: bounded area-aware rectangles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded area-aware rectangles; retain observable state/resource expectations.
- [ ] **F-GR-013.04 — Implement/prove: merge policy**
  - Action: For damage regions, implement or reuse and verify this exact obligation: merge policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for merge policy; retain observable state/resource expectations.
- [ ] **F-GR-013.05 — Implement/prove: cause**
  - Action: For damage regions, implement or reuse and verify this exact obligation: cause. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cause; retain observable state/resource expectations.
- [ ] **F-GR-013.06 — Implement/prove: full fallback and properties**
  - Action: For damage regions, implement or reuse and verify this exact obligation: full fallback and properties. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-013.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for full fallback and properties; retain observable state/resource expectations.
- [ ] **F-GR-013.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to damage regions: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-013.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-013.08 — Integrate into the real consumer and runtime route**
  - Action: Wire damage regions into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-013.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-013.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for damage regions as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-013.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-014"></a>
## F-GR-014 — occlusion calculation

**Original requirement:** opaque coverage suppresses hidden work without breaking translucency

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-014.01 — Reconcile existing occlusion calculation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for occlusion calculation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: opaque coverage suppresses hidden work without breaking translucency
- [ ] **F-GR-014.02 — Freeze the exact contract for occlusion calculation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: opaque coverage suppresses hidden work without breaking translucency. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-014.03 — Implement/prove: opaque coverage suppresses hidden work without breaking translucency**
  - Action: For occlusion calculation, implement or reuse and verify this exact obligation: opaque coverage suppresses hidden work without breaking translucency. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opaque coverage suppresses hidden work without breaking translucency; retain observable state/resource expectations.
- [ ] **F-GR-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to occlusion calculation: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire occlusion calculation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for occlusion calculation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-015"></a>
## F-GR-015 — hierarchical clipping

**Original requirement:** screen/window/widget/scroll/rounded clips constrain work before pixel loops

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-015.01 — Reconcile existing hierarchical clipping**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hierarchical clipping. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: screen/window/widget/scroll/rounded clips constrain work before pixel loops
- [ ] **F-GR-015.02 — Freeze the exact contract for hierarchical clipping**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: screen/window/widget/scroll/rounded clips constrain work before pixel loops. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-015.03 — Implement/prove: screen/window/widget/scroll/rounded clips constrain work before pixel loops**
  - Action: For hierarchical clipping, implement or reuse and verify this exact obligation: screen/window/widget/scroll/rounded clips constrain work before pixel loops. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for screen/window/widget/scroll/rounded clips constrain work before pixel loops; retain observable state/resource expectations.
- [ ] **F-GR-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hierarchical clipping: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire hierarchical clipping into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hierarchical clipping as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-016"></a>
## F-GR-016 — atomic scene commit

**Original requirement:** chrome/content/focus/z-order from one coherent generation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-016.01 — Reconcile existing atomic scene commit**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for atomic scene commit. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: chrome/content/focus/z-order from one coherent generation
- [ ] **F-GR-016.02 — Freeze the exact contract for atomic scene commit**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: chrome/content/focus/z-order from one coherent generation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-016.03 — Implement/prove: chrome/content/focus/z-order from one coherent generation**
  - Action: For atomic scene commit, implement or reuse and verify this exact obligation: chrome/content/focus/z-order from one coherent generation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for chrome/content/focus/z-order from one coherent generation; retain observable state/resource expectations.
- [ ] **F-GR-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to atomic scene commit: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire atomic scene commit into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for atomic scene commit as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-017"></a>
## F-GR-017 — one-present frame

**Original requirement:** one newest committed scene per deadline, no half-frame or redundant present

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-017.01 — Reconcile existing one-present frame**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for one-present frame. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: one newest committed scene per deadline, no half-frame or redundant present
- [ ] **F-GR-017.02 — Freeze the exact contract for one-present frame**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: one newest committed scene per deadline, no half-frame or redundant present. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-017.03 — Implement/prove: one newest committed scene per deadline**
  - Action: For one-present frame, implement or reuse and verify this exact obligation: one newest committed scene per deadline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for one newest committed scene per deadline; retain observable state/resource expectations.
- [ ] **F-GR-017.04 — Implement/prove: no half-frame or redundant present**
  - Action: For one-present frame, implement or reuse and verify this exact obligation: no half-frame or redundant present. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no half-frame or redundant present; retain observable state/resource expectations.
- [ ] **F-GR-017.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to one-present frame: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-017.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-017.06 — Integrate into the real consumer and runtime route**
  - Action: Wire one-present frame into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-017.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-017.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for one-present frame as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-017.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-018"></a>
## F-GR-018 — frame scheduler

**Original requirement:** refresh-aware deadline, event-driven wake, cost estimate, stale-state drop

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-018.01 — Reconcile existing frame scheduler**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for frame scheduler. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: refresh-aware deadline, event-driven wake, cost estimate, stale-state drop
- [ ] **F-GR-018.02 — Freeze the exact contract for frame scheduler**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: refresh-aware deadline, event-driven wake, cost estimate, stale-state drop. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-018.03 — Implement/prove: refresh-aware deadline**
  - Action: For frame scheduler, implement or reuse and verify this exact obligation: refresh-aware deadline. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for refresh-aware deadline; retain observable state/resource expectations.
- [ ] **F-GR-018.04 — Implement/prove: event-driven wake**
  - Action: For frame scheduler, implement or reuse and verify this exact obligation: event-driven wake. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for event-driven wake; retain observable state/resource expectations.
- [ ] **F-GR-018.05 — Implement/prove: cost estimate**
  - Action: For frame scheduler, implement or reuse and verify this exact obligation: cost estimate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cost estimate; retain observable state/resource expectations.
- [ ] **F-GR-018.06 — Implement/prove: stale-state drop**
  - Action: For frame scheduler, implement or reuse and verify this exact obligation: stale-state drop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stale-state drop; retain observable state/resource expectations.
- [ ] **F-GR-018.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to frame scheduler: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-018.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-018.08 — Integrate into the real consumer and runtime route**
  - Action: Wire frame scheduler into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-018.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-018.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for frame scheduler as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-018.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-019"></a>
## F-GR-019 — vblank/present timing

**Original requirement:** negotiated refresh, bounded waits, missed-vblank accounting and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-019.01 — Reconcile existing vblank/present timing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for vblank/present timing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: negotiated refresh, bounded waits, missed-vblank accounting and fallback
- [ ] **F-GR-019.02 — Freeze the exact contract for vblank/present timing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: negotiated refresh, bounded waits, missed-vblank accounting and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-019.03 — Implement/prove: negotiated refresh**
  - Action: For vblank/present timing, implement or reuse and verify this exact obligation: negotiated refresh. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for negotiated refresh; retain observable state/resource expectations.
- [ ] **F-GR-019.04 — Implement/prove: bounded waits**
  - Action: For vblank/present timing, implement or reuse and verify this exact obligation: bounded waits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounded waits; retain observable state/resource expectations.
- [ ] **F-GR-019.05 — Implement/prove: missed-vblank accounting and fallback**
  - Action: For vblank/present timing, implement or reuse and verify this exact obligation: missed-vblank accounting and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for missed-vblank accounting and fallback; retain observable state/resource expectations.
- [ ] **F-GR-019.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to vblank/present timing: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-019.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-019.07 — Integrate into the real consumer and runtime route**
  - Action: Wire vblank/present timing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-019.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-019.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for vblank/present timing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-019.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-020"></a>
## F-GR-020 — frame attribution

**Original requirement:** input/app/compositor/wait/present time plus damage/windows/bytes/backend

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-020.01 — Reconcile existing frame attribution**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for frame attribution. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: input/app/compositor/wait/present time plus damage/windows/bytes/backend
- [ ] **F-GR-020.02 — Freeze the exact contract for frame attribution**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: input/app/compositor/wait/present time plus damage/windows/bytes/backend. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-020.03 — Implement/prove: input/app/compositor/wait/present time plus damage/windows/bytes/backend**
  - Action: For frame attribution, implement or reuse and verify this exact obligation: input/app/compositor/wait/present time plus damage/windows/bytes/backend. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input/app/compositor/wait/present time plus damage/windows/bytes/backend; retain observable state/resource expectations.
- [ ] **F-GR-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to frame attribution: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire frame attribution into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for frame attribution as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-021"></a>
## F-GR-021 — input-to-present tracing

**Original requirement:** input sequence/timestamp survives through first affected present

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-021.01 — Reconcile existing input-to-present tracing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input-to-present tracing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: input sequence/timestamp survives through first affected present
- [ ] **F-GR-021.02 — Freeze the exact contract for input-to-present tracing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: input sequence/timestamp survives through first affected present. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-021.03 — Implement/prove: input sequence/timestamp survives through first affected present**
  - Action: For input-to-present tracing, implement or reuse and verify this exact obligation: input sequence/timestamp survives through first affected present. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input sequence/timestamp survives through first affected present; retain observable state/resource expectations.
- [ ] **F-GR-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input-to-present tracing: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire input-to-present tracing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input-to-present tracing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-022"></a>
## F-GR-022 — worst-frame gate

**Original requirement:** peak and percentiles plus zero unexplained late frames; average FPS insufficient

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-022.01 — Reconcile existing worst-frame gate**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for worst-frame gate. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: peak and percentiles plus zero unexplained late frames; average FPS insufficient
- [ ] **F-GR-022.02 — Freeze the exact contract for worst-frame gate**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: peak and percentiles plus zero unexplained late frames; average FPS insufficient. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-022.03 — Implement/prove: peak and percentiles plus zero unexplained late frames**
  - Action: For worst-frame gate, implement or reuse and verify this exact obligation: peak and percentiles plus zero unexplained late frames. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for peak and percentiles plus zero unexplained late frames; retain observable state/resource expectations.
- [ ] **F-GR-022.04 — Implement/prove: average FPS insufficient**
  - Action: For worst-frame gate, implement or reuse and verify this exact obligation: average FPS insufficient. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for average FPS insufficient; retain observable state/resource expectations.
- [ ] **F-GR-022.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to worst-frame gate: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-022.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-022.06 — Integrate into the real consumer and runtime route**
  - Action: Wire worst-frame gate into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-022.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-022.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for worst-frame gate as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-022.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-023"></a>
## F-GR-023 — framebuffer mapping policy

**Original requirement:** correct cacheability/write-combining with architecture and physical receipts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-023.01 — Reconcile existing framebuffer mapping policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for framebuffer mapping policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: correct cacheability/write-combining with architecture and physical receipts
- [ ] **F-GR-023.02 — Freeze the exact contract for framebuffer mapping policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: correct cacheability/write-combining with architecture and physical receipts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-023.03 — Implement/prove: correct cacheability/write-combining with architecture and physical receipts**
  - Action: For framebuffer mapping policy, implement or reuse and verify this exact obligation: correct cacheability/write-combining with architecture and physical receipts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for correct cacheability/write-combining with architecture and physical receipts; retain observable state/resource expectations.
- [ ] **F-GR-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to framebuffer mapping policy: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire framebuffer mapping policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for framebuffer mapping policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-024"></a>
## F-GR-024 — software renderer oracle

**Original requirement:** deterministic blend/clip/shape/text/image output remains fallback and GPU comparator

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-024.01 — Reconcile existing software renderer oracle**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for software renderer oracle. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic blend/clip/shape/text/image output remains fallback and GPU comparator
- [ ] **F-GR-024.02 — Freeze the exact contract for software renderer oracle**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic blend/clip/shape/text/image output remains fallback and GPU comparator. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-024.03 — Implement/prove: deterministic blend/clip/shape/text/image output remains fallback and GPU comparator**
  - Action: For software renderer oracle, implement or reuse and verify this exact obligation: deterministic blend/clip/shape/text/image output remains fallback and GPU comparator. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic blend/clip/shape/text/image output remains fallback and GPU comparator; retain observable state/resource expectations.
- [ ] **F-GR-024.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to software renderer oracle: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-024.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-024.05 — Integrate into the real consumer and runtime route**
  - Action: Wire software renderer oracle into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-024.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-024.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for software renderer oracle as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-024.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-025"></a>
## F-GR-025 — GPU renderer backend

**Original requirement:** same scene semantics, validated resources/commands, fences, reset and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-025.01 — Reconcile existing GPU renderer backend**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for GPU renderer backend. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: same scene semantics, validated resources/commands, fences, reset and fallback
- [ ] **F-GR-025.02 — Freeze the exact contract for GPU renderer backend**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: same scene semantics, validated resources/commands, fences, reset and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-025.03 — Implement/prove: same scene semantics**
  - Action: For GPU renderer backend, implement or reuse and verify this exact obligation: same scene semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for same scene semantics; retain observable state/resource expectations.
- [ ] **F-GR-025.04 — Implement/prove: validated resources/commands**
  - Action: For GPU renderer backend, implement or reuse and verify this exact obligation: validated resources/commands. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validated resources/commands; retain observable state/resource expectations.
- [ ] **F-GR-025.05 — Implement/prove: fences**
  - Action: For GPU renderer backend, implement or reuse and verify this exact obligation: fences. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fences; retain observable state/resource expectations.
- [ ] **F-GR-025.06 — Implement/prove: reset and fallback**
  - Action: For GPU renderer backend, implement or reuse and verify this exact obligation: reset and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-025.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reset and fallback; retain observable state/resource expectations.
- [ ] **F-GR-025.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to GPU renderer backend: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-025.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-025.08 — Integrate into the real consumer and runtime route**
  - Action: Wire GPU renderer backend into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-025.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-025.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for GPU renderer backend as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-025.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-026"></a>
## F-GR-026 — alpha composition

**Original requirement:** defined straight/premultiplied model, rounding, color space and property tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-026.01 — Reconcile existing alpha composition**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for alpha composition. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: defined straight/premultiplied model, rounding, color space and property tests
- [ ] **F-GR-026.02 — Freeze the exact contract for alpha composition**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: defined straight/premultiplied model, rounding, color space and property tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-026.03 — Implement/prove: defined straight/premultiplied model**
  - Action: For alpha composition, implement or reuse and verify this exact obligation: defined straight/premultiplied model. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for defined straight/premultiplied model; retain observable state/resource expectations.
- [ ] **F-GR-026.04 — Implement/prove: rounding**
  - Action: For alpha composition, implement or reuse and verify this exact obligation: rounding. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rounding; retain observable state/resource expectations.
- [ ] **F-GR-026.05 — Implement/prove: color space and property tests**
  - Action: For alpha composition, implement or reuse and verify this exact obligation: color space and property tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for color space and property tests; retain observable state/resource expectations.
- [ ] **F-GR-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to alpha composition: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire alpha composition into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for alpha composition as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-027"></a>
## F-GR-027 — rounded geometry

**Original requirement:** supersampled/analytic coverage, consistent radii, clipping and hit testing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-027.01 — Reconcile existing rounded geometry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for rounded geometry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: supersampled/analytic coverage, consistent radii, clipping and hit testing
- [ ] **F-GR-027.02 — Freeze the exact contract for rounded geometry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: supersampled/analytic coverage, consistent radii, clipping and hit testing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-027.03 — Implement/prove: supersampled/analytic coverage**
  - Action: For rounded geometry, implement or reuse and verify this exact obligation: supersampled/analytic coverage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for supersampled/analytic coverage; retain observable state/resource expectations.
- [ ] **F-GR-027.04 — Implement/prove: consistent radii**
  - Action: For rounded geometry, implement or reuse and verify this exact obligation: consistent radii. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consistent radii; retain observable state/resource expectations.
- [ ] **F-GR-027.05 — Implement/prove: clipping and hit testing**
  - Action: For rounded geometry, implement or reuse and verify this exact obligation: clipping and hit testing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clipping and hit testing; retain observable state/resource expectations.
- [ ] **F-GR-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to rounded geometry: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire rounded geometry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for rounded geometry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-028"></a>
## F-GR-028 — shadows/elevation

**Original requirement:** semantic depth, scissor-aware bounded implementation, cache and opaque fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-028.01 — Reconcile existing shadows/elevation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shadows/elevation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic depth, scissor-aware bounded implementation, cache and opaque fallback
- [ ] **F-GR-028.02 — Freeze the exact contract for shadows/elevation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic depth, scissor-aware bounded implementation, cache and opaque fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-028.03 — Implement/prove: semantic depth**
  - Action: For shadows/elevation, implement or reuse and verify this exact obligation: semantic depth. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic depth; retain observable state/resource expectations.
- [ ] **F-GR-028.04 — Implement/prove: scissor-aware bounded implementation**
  - Action: For shadows/elevation, implement or reuse and verify this exact obligation: scissor-aware bounded implementation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scissor-aware bounded implementation; retain observable state/resource expectations.
- [ ] **F-GR-028.05 — Implement/prove: cache and opaque fallback**
  - Action: For shadows/elevation, implement or reuse and verify this exact obligation: cache and opaque fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache and opaque fallback; retain observable state/resource expectations.
- [ ] **F-GR-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shadows/elevation: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire shadows/elevation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shadows/elevation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-029"></a>
## F-GR-029 — gradients/decorative fields

**Original requirement:** deterministic cached rendering with bounded setup and degradation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-029.01 — Reconcile existing gradients/decorative fields**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for gradients/decorative fields. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic cached rendering with bounded setup and degradation
- [ ] **F-GR-029.02 — Freeze the exact contract for gradients/decorative fields**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic cached rendering with bounded setup and degradation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-029.03 — Implement/prove: deterministic cached rendering with bounded setup and degradation**
  - Action: For gradients/decorative fields, implement or reuse and verify this exact obligation: deterministic cached rendering with bounded setup and degradation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic cached rendering with bounded setup and degradation; retain observable state/resource expectations.
- [ ] **F-GR-029.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to gradients/decorative fields: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-029.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-029.05 — Integrate into the real consumer and runtime route**
  - Action: Wire gradients/decorative fields into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-029.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-029.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for gradients/decorative fields as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-029.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-030"></a>
## F-GR-030 — backdrop blur

**Original requirement:** stationary/cacheable only unless backend budget proves live update; invalidation exact

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-030.01 — Reconcile existing backdrop blur**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for backdrop blur. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stationary/cacheable only unless backend budget proves live update; invalidation exact
- [ ] **F-GR-030.02 — Freeze the exact contract for backdrop blur**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stationary/cacheable only unless backend budget proves live update; invalidation exact. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-030.03 — Implement/prove: stationary/cacheable only unless backend budget proves live update**
  - Action: For backdrop blur, implement or reuse and verify this exact obligation: stationary/cacheable only unless backend budget proves live update. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stationary/cacheable only unless backend budget proves live update; retain observable state/resource expectations.
- [ ] **F-GR-030.04 — Implement/prove: invalidation exact**
  - Action: For backdrop blur, implement or reuse and verify this exact obligation: invalidation exact. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for invalidation exact; retain observable state/resource expectations.
- [ ] **F-GR-030.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to backdrop blur: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-030.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-030.06 — Integrate into the real consumer and runtime route**
  - Action: Wire backdrop blur into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-030.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-030.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for backdrop blur as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-030.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-031"></a>
## F-GR-031 — wallpaper cache

**Original requirement:** full-resolution bounded cache, memory admission, invalidation and honest fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-031.01 — Reconcile existing wallpaper cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for wallpaper cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: full-resolution bounded cache, memory admission, invalidation and honest fallback
- [ ] **F-GR-031.02 — Freeze the exact contract for wallpaper cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: full-resolution bounded cache, memory admission, invalidation and honest fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-031.03 — Implement/prove: full-resolution bounded cache**
  - Action: For wallpaper cache, implement or reuse and verify this exact obligation: full-resolution bounded cache. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for full-resolution bounded cache; retain observable state/resource expectations.
- [ ] **F-GR-031.04 — Implement/prove: memory admission**
  - Action: For wallpaper cache, implement or reuse and verify this exact obligation: memory admission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for memory admission; retain observable state/resource expectations.
- [ ] **F-GR-031.05 — Implement/prove: invalidation and honest fallback**
  - Action: For wallpaper cache, implement or reuse and verify this exact obligation: invalidation and honest fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for invalidation and honest fallback; retain observable state/resource expectations.
- [ ] **F-GR-031.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to wallpaper cache: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-031.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-031.07 — Integrate into the real consumer and runtime route**
  - Action: Wire wallpaper cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-031.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-031.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for wallpaper cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-031.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-032"></a>
## F-GR-032 — icon/image cache

**Original requirement:** key by origin/scale/theme/profile, byte budget, generation and decoder isolation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-032.01 — Reconcile existing icon/image cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for icon/image cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: key by origin/scale/theme/profile, byte budget, generation and decoder isolation
- [ ] **F-GR-032.02 — Freeze the exact contract for icon/image cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: key by origin/scale/theme/profile, byte budget, generation and decoder isolation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-032.03 — Implement/prove: key by origin/scale/theme/profile**
  - Action: For icon/image cache, implement or reuse and verify this exact obligation: key by origin/scale/theme/profile. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for key by origin/scale/theme/profile; retain observable state/resource expectations.
- [ ] **F-GR-032.04 — Implement/prove: byte budget**
  - Action: For icon/image cache, implement or reuse and verify this exact obligation: byte budget. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-032.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for byte budget; retain observable state/resource expectations.
- [ ] **F-GR-032.05 — Implement/prove: generation and decoder isolation**
  - Action: For icon/image cache, implement or reuse and verify this exact obligation: generation and decoder isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-032.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generation and decoder isolation; retain observable state/resource expectations.
- [ ] **F-GR-032.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to icon/image cache: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-032.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-032.07 — Integrate into the real consumer and runtime route**
  - Action: Wire icon/image cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-032.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-032.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for icon/image cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-032.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-033"></a>
## F-GR-033 — glyph/shaping cache

**Original requirement:** font/size/scale/script/features key, atlas budget, eviction and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-033.01 — Reconcile existing glyph/shaping cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for glyph/shaping cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: font/size/scale/script/features key, atlas budget, eviction and fallback
- [ ] **F-GR-033.02 — Freeze the exact contract for glyph/shaping cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: font/size/scale/script/features key, atlas budget, eviction and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-033.03 — Implement/prove: font/size/scale/script/features key**
  - Action: For glyph/shaping cache, implement or reuse and verify this exact obligation: font/size/scale/script/features key. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for font/size/scale/script/features key; retain observable state/resource expectations.
- [ ] **F-GR-033.04 — Implement/prove: atlas budget**
  - Action: For glyph/shaping cache, implement or reuse and verify this exact obligation: atlas budget. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atlas budget; retain observable state/resource expectations.
- [ ] **F-GR-033.05 — Implement/prove: eviction and fallback**
  - Action: For glyph/shaping cache, implement or reuse and verify this exact obligation: eviction and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for eviction and fallback; retain observable state/resource expectations.
- [ ] **F-GR-033.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to glyph/shaping cache: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-033.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-033.07 — Integrate into the real consumer and runtime route**
  - Action: Wire glyph/shaping cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-033.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-033.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for glyph/shaping cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-033.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-034"></a>
## F-GR-034 — layout cache

**Original requirement:** dependency-aware invalidation for text/widget/browser trees

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-034.01 — Reconcile existing layout cache**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for layout cache. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: dependency-aware invalidation for text/widget/browser trees
- [ ] **F-GR-034.02 — Freeze the exact contract for layout cache**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: dependency-aware invalidation for text/widget/browser trees. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-034.03 — Implement/prove: dependency-aware invalidation for text/widget/browser trees**
  - Action: For layout cache, implement or reuse and verify this exact obligation: dependency-aware invalidation for text/widget/browser trees. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dependency-aware invalidation for text/widget/browser trees; retain observable state/resource expectations.
- [ ] **F-GR-034.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to layout cache: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-034.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-034.05 — Integrate into the real consumer and runtime route**
  - Action: Wire layout cache into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-034.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-034.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for layout cache as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-034.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-035"></a>
## F-GR-035 — virtualized rendering

**Original requirement:** only visible list/text/table/directory/document ranges are created and painted

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-035.01 — Reconcile existing virtualized rendering**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for virtualized rendering. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: only visible list/text/table/directory/document ranges are created and painted
- [ ] **F-GR-035.02 — Freeze the exact contract for virtualized rendering**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: only visible list/text/table/directory/document ranges are created and painted. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-035.03 — Implement/prove: only visible list/text/table/directory/document ranges are created and painted**
  - Action: For virtualized rendering, implement or reuse and verify this exact obligation: only visible list/text/table/directory/document ranges are created and painted. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for only visible list/text/table/directory/document ranges are created and painted; retain observable state/resource expectations.
- [ ] **F-GR-035.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to virtualized rendering: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-035.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-035.05 — Integrate into the real consumer and runtime route**
  - Action: Wire virtualized rendering into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-035.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-035.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for virtualized rendering as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-035.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-036"></a>
## F-GR-036 — resize interaction

**Original requirement:** configure/ack, minimums, aspect policy, retained preview, settled redraw

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-036.01 — Reconcile existing resize interaction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for resize interaction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: configure/ack, minimums, aspect policy, retained preview, settled redraw
- [ ] **F-GR-036.02 — Freeze the exact contract for resize interaction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: configure/ack, minimums, aspect policy, retained preview, settled redraw. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-036.03 — Implement/prove: configure/ack**
  - Action: For resize interaction, implement or reuse and verify this exact obligation: configure/ack. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configure/ack; retain observable state/resource expectations.
- [ ] **F-GR-036.04 — Implement/prove: minimums**
  - Action: For resize interaction, implement or reuse and verify this exact obligation: minimums. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimums; retain observable state/resource expectations.
- [ ] **F-GR-036.05 — Implement/prove: aspect policy**
  - Action: For resize interaction, implement or reuse and verify this exact obligation: aspect policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for aspect policy; retain observable state/resource expectations.
- [ ] **F-GR-036.06 — Implement/prove: retained preview**
  - Action: For resize interaction, implement or reuse and verify this exact obligation: retained preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-036.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retained preview; retain observable state/resource expectations.
- [ ] **F-GR-036.07 — Implement/prove: settled redraw**
  - Action: For resize interaction, implement or reuse and verify this exact obligation: settled redraw. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-036.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for settled redraw; retain observable state/resource expectations.
- [ ] **F-GR-036.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to resize interaction: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-036.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-036.09 — Integrate into the real consumer and runtime route**
  - Action: Wire resize interaction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-036.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-036.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for resize interaction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-036.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-037"></a>
## F-GR-037 — move/drag interaction

**Original requirement:** time-stable grab identity, old/new damage, retained node move and snap preview

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-037.01 — Reconcile existing move/drag interaction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for move/drag interaction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: time-stable grab identity, old/new damage, retained node move and snap preview
- [ ] **F-GR-037.02 — Freeze the exact contract for move/drag interaction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: time-stable grab identity, old/new damage, retained node move and snap preview. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-037.03 — Implement/prove: time-stable grab identity**
  - Action: For move/drag interaction, implement or reuse and verify this exact obligation: time-stable grab identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time-stable grab identity; retain observable state/resource expectations.
- [ ] **F-GR-037.04 — Implement/prove: old/new damage**
  - Action: For move/drag interaction, implement or reuse and verify this exact obligation: old/new damage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for old/new damage; retain observable state/resource expectations.
- [ ] **F-GR-037.05 — Implement/prove: retained node move and snap preview**
  - Action: For move/drag interaction, implement or reuse and verify this exact obligation: retained node move and snap preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-037.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retained node move and snap preview; retain observable state/resource expectations.
- [ ] **F-GR-037.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to move/drag interaction: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-037.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-037.07 — Integrate into the real consumer and runtime route**
  - Action: Wire move/drag interaction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-037.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-037.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for move/drag interaction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-037.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-038"></a>
## F-GR-038 — window stacking/focus

**Original requirement:** deterministic z-order, modal/transient relationships, activation policy and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-038.01 — Reconcile existing window stacking/focus**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for window stacking/focus. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic z-order, modal/transient relationships, activation policy and a11y
- [ ] **F-GR-038.02 — Freeze the exact contract for window stacking/focus**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic z-order, modal/transient relationships, activation policy and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-038.03 — Implement/prove: deterministic z-order**
  - Action: For window stacking/focus, implement or reuse and verify this exact obligation: deterministic z-order. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic z-order; retain observable state/resource expectations.
- [ ] **F-GR-038.04 — Implement/prove: modal/transient relationships**
  - Action: For window stacking/focus, implement or reuse and verify this exact obligation: modal/transient relationships. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modal/transient relationships; retain observable state/resource expectations.
- [ ] **F-GR-038.05 — Implement/prove: activation policy and a11y**
  - Action: For window stacking/focus, implement or reuse and verify this exact obligation: activation policy and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for activation policy and a11y; retain observable state/resource expectations.
- [ ] **F-GR-038.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to window stacking/focus: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-038.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-038.07 — Integrate into the real consumer and runtime route**
  - Action: Wire window stacking/focus into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-038.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-038.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for window stacking/focus as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-038.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-039"></a>
## F-GR-039 — workspaces

**Original requirement:** create/switch/move/overview, keyboard route, persistence and notifications

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-039.01 — Reconcile existing workspaces**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for workspaces. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: create/switch/move/overview, keyboard route, persistence and notifications
- [ ] **F-GR-039.02 — Freeze the exact contract for workspaces**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: create/switch/move/overview, keyboard route, persistence and notifications. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-039.03 — Implement/prove: create/switch/move/overview**
  - Action: For workspaces, implement or reuse and verify this exact obligation: create/switch/move/overview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for create/switch/move/overview; retain observable state/resource expectations.
- [ ] **F-GR-039.04 — Implement/prove: keyboard route**
  - Action: For workspaces, implement or reuse and verify this exact obligation: keyboard route. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard route; retain observable state/resource expectations.
- [ ] **F-GR-039.05 — Implement/prove: persistence and notifications**
  - Action: For workspaces, implement or reuse and verify this exact obligation: persistence and notifications. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistence and notifications; retain observable state/resource expectations.
- [ ] **F-GR-039.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to workspaces: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-039.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-039.07 — Integrate into the real consumer and runtime route**
  - Action: Wire workspaces into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-039.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-039.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for workspaces as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-039.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-040"></a>
## F-GR-040 — fullscreen

**Original requirement:** explicit user/app policy, overlays, escape route, multi-display and permission

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-040.01 — Reconcile existing fullscreen**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for fullscreen. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit user/app policy, overlays, escape route, multi-display and permission
- [ ] **F-GR-040.02 — Freeze the exact contract for fullscreen**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit user/app policy, overlays, escape route, multi-display and permission. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-040.03 — Implement/prove: explicit user/app policy**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: explicit user/app policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit user/app policy; retain observable state/resource expectations.
- [ ] **F-GR-040.04 — Implement/prove: overlays**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: overlays. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overlays; retain observable state/resource expectations.
- [ ] **F-GR-040.05 — Implement/prove: escape route**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: escape route. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for escape route; retain observable state/resource expectations.
- [ ] **F-GR-040.06 — Implement/prove: multi-display and permission**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: multi-display and permission. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-040.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multi-display and permission; retain observable state/resource expectations.
- [ ] **F-GR-040.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to fullscreen: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-040.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-040.08 — Integrate into the real consumer and runtime route**
  - Action: Wire fullscreen into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-040.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-040.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for fullscreen as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-040.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-041"></a>
## F-GR-041 — direct scanout

**Original requirement:** safe eligible fullscreen optimization with compositor revocation and fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-041.01 — Reconcile existing direct scanout**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for direct scanout. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: safe eligible fullscreen optimization with compositor revocation and fallback
- [ ] **F-GR-041.02 — Freeze the exact contract for direct scanout**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: safe eligible fullscreen optimization with compositor revocation and fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-041.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-041.03 — Implement/prove: safe eligible fullscreen optimization with compositor revocation and fallback**
  - Action: For direct scanout, implement or reuse and verify this exact obligation: safe eligible fullscreen optimization with compositor revocation and fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-041.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for safe eligible fullscreen optimization with compositor revocation and fallback; retain observable state/resource expectations.
- [ ] **F-GR-041.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to direct scanout: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-041.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-041.05 — Integrate into the real consumer and runtime route**
  - Action: Wire direct scanout into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-041.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-041.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for direct scanout as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-041.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-042"></a>
## F-GR-042 — hardware cursor

**Original requirement:** provider cursor plane with software equivalence and capture behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-042.01 — Reconcile existing hardware cursor**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for hardware cursor. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider cursor plane with software equivalence and capture behavior
- [ ] **F-GR-042.02 — Freeze the exact contract for hardware cursor**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider cursor plane with software equivalence and capture behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-042.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-042.03 — Implement/prove: provider cursor plane with software equivalence and capture behavior**
  - Action: For hardware cursor, implement or reuse and verify this exact obligation: provider cursor plane with software equivalence and capture behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-042.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider cursor plane with software equivalence and capture behavior; retain observable state/resource expectations.
- [ ] **F-GR-042.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to hardware cursor: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-042.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-042.05 — Integrate into the real consumer and runtime route**
  - Action: Wire hardware cursor into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-042.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-042.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for hardware cursor as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-042.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-043"></a>
## F-GR-043 — screenshot/capture portal

**Original requirement:** user-mediated target, privacy indicator, protected surfaces and typed result

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-043.01 — Reconcile existing screenshot/capture portal**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screenshot/capture portal. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user-mediated target, privacy indicator, protected surfaces and typed result
- [ ] **F-GR-043.02 — Freeze the exact contract for screenshot/capture portal**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user-mediated target, privacy indicator, protected surfaces and typed result. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-043.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-043.03 — Implement/prove: user-mediated target**
  - Action: For screenshot/capture portal, implement or reuse and verify this exact obligation: user-mediated target. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-043.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user-mediated target; retain observable state/resource expectations.
- [ ] **F-GR-043.04 — Implement/prove: privacy indicator**
  - Action: For screenshot/capture portal, implement or reuse and verify this exact obligation: privacy indicator. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-043.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy indicator; retain observable state/resource expectations.
- [ ] **F-GR-043.05 — Implement/prove: protected surfaces and typed result**
  - Action: For screenshot/capture portal, implement or reuse and verify this exact obligation: protected surfaces and typed result. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-043.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for protected surfaces and typed result; retain observable state/resource expectations.
- [ ] **F-GR-043.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screenshot/capture portal: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-043.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-043.07 — Integrate into the real consumer and runtime route**
  - Action: Wire screenshot/capture portal into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-043.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-043.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screenshot/capture portal as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-043.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-044"></a>
## F-GR-044 — screen recording

**Original requirement:** consent, region/window/display, audio, frame pacing, storage and protected content

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-044.01 — Reconcile existing screen recording**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screen recording. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: consent, region/window/display, audio, frame pacing, storage and protected content
- [ ] **F-GR-044.02 — Freeze the exact contract for screen recording**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: consent, region/window/display, audio, frame pacing, storage and protected content. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-044.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-044.03 — Implement/prove: consent**
  - Action: For screen recording, implement or reuse and verify this exact obligation: consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-044.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consent; retain observable state/resource expectations.
- [ ] **F-GR-044.04 — Implement/prove: region/window/display**
  - Action: For screen recording, implement or reuse and verify this exact obligation: region/window/display. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-044.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for region/window/display; retain observable state/resource expectations.
- [ ] **F-GR-044.05 — Implement/prove: audio**
  - Action: For screen recording, implement or reuse and verify this exact obligation: audio. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-044.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for audio; retain observable state/resource expectations.
- [ ] **F-GR-044.06 — Implement/prove: frame pacing**
  - Action: For screen recording, implement or reuse and verify this exact obligation: frame pacing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-044.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frame pacing; retain observable state/resource expectations.
- [ ] **F-GR-044.07 — Implement/prove: storage and protected content**
  - Action: For screen recording, implement or reuse and verify this exact obligation: storage and protected content. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-044.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for storage and protected content; retain observable state/resource expectations.
- [ ] **F-GR-044.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screen recording: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-044.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-044.09 — Integrate into the real consumer and runtime route**
  - Action: Wire screen recording into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-044.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-044.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screen recording as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-044.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-045"></a>
## F-GR-045 — remote composition mode

**Original requirement:** damage/video transport, congestion, input clock, scaling and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-045.01 — Reconcile existing remote composition mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for remote composition mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: damage/video transport, congestion, input clock, scaling and privacy
- [ ] **F-GR-045.02 — Freeze the exact contract for remote composition mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: damage/video transport, congestion, input clock, scaling and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-045.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-045.03 — Implement/prove: damage/video transport**
  - Action: For remote composition mode, implement or reuse and verify this exact obligation: damage/video transport. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-045.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for damage/video transport; retain observable state/resource expectations.
- [ ] **F-GR-045.04 — Implement/prove: congestion**
  - Action: For remote composition mode, implement or reuse and verify this exact obligation: congestion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-045.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for congestion; retain observable state/resource expectations.
- [ ] **F-GR-045.05 — Implement/prove: input clock**
  - Action: For remote composition mode, implement or reuse and verify this exact obligation: input clock. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-045.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input clock; retain observable state/resource expectations.
- [ ] **F-GR-045.06 — Implement/prove: scaling and privacy**
  - Action: For remote composition mode, implement or reuse and verify this exact obligation: scaling and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-045.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scaling and privacy; retain observable state/resource expectations.
- [ ] **F-GR-045.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to remote composition mode: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-045.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-045.08 — Integrate into the real consumer and runtime route**
  - Action: Wire remote composition mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-045.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-045.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for remote composition mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-045.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-046"></a>
## F-GR-046 — graphics diagnostics

**Original requirement:** live backend/mode/cache/damage/frame/fence/drop information and export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-046.01 — Reconcile existing graphics diagnostics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for graphics diagnostics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: live backend/mode/cache/damage/frame/fence/drop information and export
- [ ] **F-GR-046.02 — Freeze the exact contract for graphics diagnostics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: live backend/mode/cache/damage/frame/fence/drop information and export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-046.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-046.03 — Implement/prove: live backend/mode/cache/damage/frame/fence/drop information and export**
  - Action: For graphics diagnostics, implement or reuse and verify this exact obligation: live backend/mode/cache/damage/frame/fence/drop information and export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-046.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for live backend/mode/cache/damage/frame/fence/drop information and export; retain observable state/resource expectations.
- [ ] **F-GR-046.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to graphics diagnostics: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-046.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-046.05 — Integrate into the real consumer and runtime route**
  - Action: Wire graphics diagnostics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-046.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-046.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for graphics diagnostics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-046.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-047"></a>
## F-GR-047 — graphics recovery

**Original requirement:** provider hang/loss resets safely and returns to software/GOP usable shell

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-047.01 — Reconcile existing graphics recovery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for graphics recovery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: provider hang/loss resets safely and returns to software/GOP usable shell
- [ ] **F-GR-047.02 — Freeze the exact contract for graphics recovery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: provider hang/loss resets safely and returns to software/GOP usable shell. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-047.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-047.03 — Implement/prove: provider hang/loss resets safely and returns to software/GOP usable shell**
  - Action: For graphics recovery, implement or reuse and verify this exact obligation: provider hang/loss resets safely and returns to software/GOP usable shell. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-047.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for provider hang/loss resets safely and returns to software/GOP usable shell; retain observable state/resource expectations.
- [ ] **F-GR-047.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to graphics recovery: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-047.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-047.05 — Integrate into the real consumer and runtime route**
  - Action: Wire graphics recovery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-047.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-047.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for graphics recovery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-047.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-048"></a>
## F-GR-048 — memory-pressure graphics policy

**Original requirement:** deterministic cache/surface eviction without stale pixels or semantic loss

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-048.01 — Reconcile existing memory-pressure graphics policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for memory-pressure graphics policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic cache/surface eviction without stale pixels or semantic loss
- [ ] **F-GR-048.02 — Freeze the exact contract for memory-pressure graphics policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic cache/surface eviction without stale pixels or semantic loss. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-048.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-048.03 — Implement/prove: deterministic cache/surface eviction without stale pixels or semantic loss**
  - Action: For memory-pressure graphics policy, implement or reuse and verify this exact obligation: deterministic cache/surface eviction without stale pixels or semantic loss. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-048.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic cache/surface eviction without stale pixels or semantic loss; retain observable state/resource expectations.
- [ ] **F-GR-048.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to memory-pressure graphics policy: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-048.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-048.05 — Integrate into the real consumer and runtime route**
  - Action: Wire memory-pressure graphics policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-048.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-048.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for memory-pressure graphics policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-048.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-049"></a>
## F-GR-049 — reduced-quality mode

**Original requirement:** declared effect degradation while preserving content, contrast, focus and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-049.01 — Reconcile existing reduced-quality mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reduced-quality mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: declared effect degradation while preserving content, contrast, focus and a11y
- [ ] **F-GR-049.02 — Freeze the exact contract for reduced-quality mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: declared effect degradation while preserving content, contrast, focus and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-049.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-049.03 — Implement/prove: declared effect degradation while preserving content**
  - Action: For reduced-quality mode, implement or reuse and verify this exact obligation: declared effect degradation while preserving content. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-049.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for declared effect degradation while preserving content; retain observable state/resource expectations.
- [ ] **F-GR-049.04 — Implement/prove: contrast**
  - Action: For reduced-quality mode, implement or reuse and verify this exact obligation: contrast. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-049.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for contrast; retain observable state/resource expectations.
- [ ] **F-GR-049.05 — Implement/prove: focus and a11y**
  - Action: For reduced-quality mode, implement or reuse and verify this exact obligation: focus and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-049.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus and a11y; retain observable state/resource expectations.
- [ ] **F-GR-049.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reduced-quality mode: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-049.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-049.07 — Integrate into the real consumer and runtime route**
  - Action: Wire reduced-quality mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-049.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-049.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reduced-quality mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-049.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gr-050"></a>
## F-GR-050 — visual/frame regression lab

**Original requirement:** shipping-code benchmark, screenshot oracles and mutation tests across modes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GR-050.01 — Reconcile existing visual/frame regression lab**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for visual/frame regression lab. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: shipping-code benchmark, screenshot oracles and mutation tests across modes
- [ ] **F-GR-050.02 — Freeze the exact contract for visual/frame regression lab**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: shipping-code benchmark, screenshot oracles and mutation tests across modes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GR-050.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GR-050.03 — Implement/prove: shipping-code benchmark**
  - Action: For visual/frame regression lab, implement or reuse and verify this exact obligation: shipping-code benchmark. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-050.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shipping-code benchmark; retain observable state/resource expectations.
- [ ] **F-GR-050.04 — Implement/prove: screenshot oracles and mutation tests across modes**
  - Action: For visual/frame regression lab, implement or reuse and verify this exact obligation: screenshot oracles and mutation tests across modes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GR-050.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for screenshot oracles and mutation tests across modes; retain observable state/resource expectations.
- [ ] **F-GR-050.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to visual/frame regression lab: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GR-050.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GR-050.06 — Integrate into the real consumer and runtime route**
  - Action: Wire visual/frame regression lab into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GR-050.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GR-050.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for visual/frame regression lab as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GR-050.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-001"></a>
## F-IN-001 — normalized input event

**Original requirement:** device, sequence, monotonic time, physical/logical code, value, flags and loss

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-001.01 — Reconcile existing normalized input event**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for normalized input event. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: device, sequence, monotonic time, physical/logical code, value, flags and loss
- [ ] **F-IN-001.02 — Freeze the exact contract for normalized input event**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: device, sequence, monotonic time, physical/logical code, value, flags and loss. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-001.03 — Implement/prove: device**
  - Action: For normalized input event, implement or reuse and verify this exact obligation: device. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device; retain observable state/resource expectations.
- [ ] **F-IN-001.04 — Implement/prove: sequence**
  - Action: For normalized input event, implement or reuse and verify this exact obligation: sequence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sequence; retain observable state/resource expectations.
- [ ] **F-IN-001.05 — Implement/prove: monotonic time**
  - Action: For normalized input event, implement or reuse and verify this exact obligation: monotonic time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for monotonic time; retain observable state/resource expectations.
- [ ] **F-IN-001.06 — Implement/prove: physical/logical code**
  - Action: For normalized input event, implement or reuse and verify this exact obligation: physical/logical code. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for physical/logical code; retain observable state/resource expectations.
- [ ] **F-IN-001.07 — Implement/prove: value**
  - Action: For normalized input event, implement or reuse and verify this exact obligation: value. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-001.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for value; retain observable state/resource expectations.
- [ ] **F-IN-001.08 — Implement/prove: flags and loss**
  - Action: For normalized input event, implement or reuse and verify this exact obligation: flags and loss. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-001.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for flags and loss; retain observable state/resource expectations.
- [ ] **F-IN-001.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to normalized input event: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-001.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-001.10 — Integrate into the real consumer and runtime route**
  - Action: Wire normalized input event into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-001.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-001.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for normalized input event as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-001.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-002"></a>
## F-IN-002 — raw input stream

**Original requirement:** permissioned unaccelerated device events for games, diagnostics and assistive tools

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-002.01 — Reconcile existing raw input stream**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for raw input stream. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: permissioned unaccelerated device events for games, diagnostics and assistive tools
- [ ] **F-IN-002.02 — Freeze the exact contract for raw input stream**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: permissioned unaccelerated device events for games, diagnostics and assistive tools. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-002.03 — Implement/prove: permissioned unaccelerated device events for games**
  - Action: For raw input stream, implement or reuse and verify this exact obligation: permissioned unaccelerated device events for games. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for permissioned unaccelerated device events for games; retain observable state/resource expectations.
- [ ] **F-IN-002.04 — Implement/prove: diagnostics and assistive tools**
  - Action: For raw input stream, implement or reuse and verify this exact obligation: diagnostics and assistive tools. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for diagnostics and assistive tools; retain observable state/resource expectations.
- [ ] **F-IN-002.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to raw input stream: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-002.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-002.06 — Integrate into the real consumer and runtime route**
  - Action: Wire raw input stream into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-002.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-002.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for raw input stream as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-002.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-003"></a>
## F-IN-003 — compositor hit testing

**Original requirement:** z-order/clip/alpha-aware target selection with authenticated ownership

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-003.01 — Reconcile existing compositor hit testing**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for compositor hit testing. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: z-order/clip/alpha-aware target selection with authenticated ownership
- [ ] **F-IN-003.02 — Freeze the exact contract for compositor hit testing**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: z-order/clip/alpha-aware target selection with authenticated ownership. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-003.03 — Implement/prove: z-order/clip/alpha-aware target selection with authenticated ownership**
  - Action: For compositor hit testing, implement or reuse and verify this exact obligation: z-order/clip/alpha-aware target selection with authenticated ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for z-order/clip/alpha-aware target selection with authenticated ownership; retain observable state/resource expectations.
- [ ] **F-IN-003.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to compositor hit testing: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-003.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-003.05 — Integrate into the real consumer and runtime route**
  - Action: Wire compositor hit testing into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-003.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-003.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for compositor hit testing as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-003.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-004"></a>
## F-IN-004 — keyboard focus

**Original requirement:** explicit transfer, visual/semantic indication, modal policy and peer-death fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-004.01 — Reconcile existing keyboard focus**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for keyboard focus. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit transfer, visual/semantic indication, modal policy and peer-death fallback
- [ ] **F-IN-004.02 — Freeze the exact contract for keyboard focus**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit transfer, visual/semantic indication, modal policy and peer-death fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-004.03 — Implement/prove: explicit transfer**
  - Action: For keyboard focus, implement or reuse and verify this exact obligation: explicit transfer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit transfer; retain observable state/resource expectations.
- [ ] **F-IN-004.04 — Implement/prove: visual/semantic indication**
  - Action: For keyboard focus, implement or reuse and verify this exact obligation: visual/semantic indication. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for visual/semantic indication; retain observable state/resource expectations.
- [ ] **F-IN-004.05 — Implement/prove: modal policy and peer-death fallback**
  - Action: For keyboard focus, implement or reuse and verify this exact obligation: modal policy and peer-death fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modal policy and peer-death fallback; retain observable state/resource expectations.
- [ ] **F-IN-004.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to keyboard focus: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-004.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-004.07 — Integrate into the real consumer and runtime route**
  - Action: Wire keyboard focus into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-004.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-004.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for keyboard focus as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-004.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-005"></a>
## F-IN-005 — pointer focus

**Original requirement:** enter/leave/motion/buttons/wheel with stable surface generation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-005.01 — Reconcile existing pointer focus**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for pointer focus. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: enter/leave/motion/buttons/wheel with stable surface generation
- [ ] **F-IN-005.02 — Freeze the exact contract for pointer focus**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: enter/leave/motion/buttons/wheel with stable surface generation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-005.03 — Implement/prove: enter/leave/motion/buttons/wheel with stable surface generation**
  - Action: For pointer focus, implement or reuse and verify this exact obligation: enter/leave/motion/buttons/wheel with stable surface generation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for enter/leave/motion/buttons/wheel with stable surface generation; retain observable state/resource expectations.
- [ ] **F-IN-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to pointer focus: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire pointer focus into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for pointer focus as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-006"></a>
## F-IN-006 — input grabs/capture

**Original requirement:** owner, scope, cancellation, secure shortcuts and stuck-key recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-006.01 — Reconcile existing input grabs/capture**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input grabs/capture. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: owner, scope, cancellation, secure shortcuts and stuck-key recovery
- [ ] **F-IN-006.02 — Freeze the exact contract for input grabs/capture**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: owner, scope, cancellation, secure shortcuts and stuck-key recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-006.03 — Implement/prove: owner**
  - Action: For input grabs/capture, implement or reuse and verify this exact obligation: owner. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owner; retain observable state/resource expectations.
- [ ] **F-IN-006.04 — Implement/prove: scope**
  - Action: For input grabs/capture, implement or reuse and verify this exact obligation: scope. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scope; retain observable state/resource expectations.
- [ ] **F-IN-006.05 — Implement/prove: cancellation**
  - Action: For input grabs/capture, implement or reuse and verify this exact obligation: cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancellation; retain observable state/resource expectations.
- [ ] **F-IN-006.06 — Implement/prove: secure shortcuts and stuck-key recovery**
  - Action: For input grabs/capture, implement or reuse and verify this exact obligation: secure shortcuts and stuck-key recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-006.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secure shortcuts and stuck-key recovery; retain observable state/resource expectations.
- [ ] **F-IN-006.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input grabs/capture: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-006.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-006.08 — Integrate into the real consumer and runtime route**
  - Action: Wire input grabs/capture into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-006.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-006.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input grabs/capture as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-006.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-007"></a>
## F-IN-007 — global shortcut policy

**Original requirement:** reserved/session/app shortcuts, conflicts, remapping and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-007.01 — Reconcile existing global shortcut policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for global shortcut policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reserved/session/app shortcuts, conflicts, remapping and accessibility
- [ ] **F-IN-007.02 — Freeze the exact contract for global shortcut policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reserved/session/app shortcuts, conflicts, remapping and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-007.03 — Implement/prove: reserved/session/app shortcuts**
  - Action: For global shortcut policy, implement or reuse and verify this exact obligation: reserved/session/app shortcuts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reserved/session/app shortcuts; retain observable state/resource expectations.
- [ ] **F-IN-007.04 — Implement/prove: conflicts**
  - Action: For global shortcut policy, implement or reuse and verify this exact obligation: conflicts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflicts; retain observable state/resource expectations.
- [ ] **F-IN-007.05 — Implement/prove: remapping and accessibility**
  - Action: For global shortcut policy, implement or reuse and verify this exact obligation: remapping and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for remapping and accessibility; retain observable state/resource expectations.
- [ ] **F-IN-007.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to global shortcut policy: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-007.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-007.07 — Integrate into the real consumer and runtime route**
  - Action: Wire global shortcut policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-007.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-007.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for global shortcut policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-007.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-008"></a>
## F-IN-008 — key repeat

**Original requirement:** configurable delay/rate, per-device state, focus transitions and no stuck repeats

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-008.01 — Reconcile existing key repeat**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for key repeat. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: configurable delay/rate, per-device state, focus transitions and no stuck repeats
- [ ] **F-IN-008.02 — Freeze the exact contract for key repeat**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: configurable delay/rate, per-device state, focus transitions and no stuck repeats. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-008.03 — Implement/prove: configurable delay/rate**
  - Action: For key repeat, implement or reuse and verify this exact obligation: configurable delay/rate. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configurable delay/rate; retain observable state/resource expectations.
- [ ] **F-IN-008.04 — Implement/prove: per-device state**
  - Action: For key repeat, implement or reuse and verify this exact obligation: per-device state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-device state; retain observable state/resource expectations.
- [ ] **F-IN-008.05 — Implement/prove: focus transitions and no stuck repeats**
  - Action: For key repeat, implement or reuse and verify this exact obligation: focus transitions and no stuck repeats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus transitions and no stuck repeats; retain observable state/resource expectations.
- [ ] **F-IN-008.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to key repeat: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-008.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-008.07 — Integrate into the real consumer and runtime route**
  - Action: Wire key repeat into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-008.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-008.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for key repeat as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-008.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-009"></a>
## F-IN-009 — keyboard layouts

**Original requirement:** physical keys to locale layout, dead keys, compose, persistence and switching

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-009.01 — Reconcile existing keyboard layouts**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for keyboard layouts. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: physical keys to locale layout, dead keys, compose, persistence and switching
- [ ] **F-IN-009.02 — Freeze the exact contract for keyboard layouts**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: physical keys to locale layout, dead keys, compose, persistence and switching. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-009.03 — Implement/prove: physical keys to locale layout**
  - Action: For keyboard layouts, implement or reuse and verify this exact obligation: physical keys to locale layout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for physical keys to locale layout; retain observable state/resource expectations.
- [ ] **F-IN-009.04 — Implement/prove: dead keys**
  - Action: For keyboard layouts, implement or reuse and verify this exact obligation: dead keys. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dead keys; retain observable state/resource expectations.
- [ ] **F-IN-009.05 — Implement/prove: compose**
  - Action: For keyboard layouts, implement or reuse and verify this exact obligation: compose. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compose; retain observable state/resource expectations.
- [ ] **F-IN-009.06 — Implement/prove: persistence and switching**
  - Action: For keyboard layouts, implement or reuse and verify this exact obligation: persistence and switching. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistence and switching; retain observable state/resource expectations.
- [ ] **F-IN-009.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to keyboard layouts: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-009.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-009.08 — Integrate into the real consumer and runtime route**
  - Action: Wire keyboard layouts into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-009.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-009.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for keyboard layouts as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-009.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-010"></a>
## F-IN-010 — input methods

**Original requirement:** IME composition, candidate UI, script support, focus/security and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-010.01 — Reconcile existing input methods**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input methods. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: IME composition, candidate UI, script support, focus/security and a11y
- [ ] **F-IN-010.02 — Freeze the exact contract for input methods**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: IME composition, candidate UI, script support, focus/security and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-010.03 — Implement/prove: IME composition**
  - Action: For input methods, implement or reuse and verify this exact obligation: IME composition. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IME composition; retain observable state/resource expectations.
- [ ] **F-IN-010.04 — Implement/prove: candidate UI**
  - Action: For input methods, implement or reuse and verify this exact obligation: candidate UI. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for candidate UI; retain observable state/resource expectations.
- [ ] **F-IN-010.05 — Implement/prove: script support**
  - Action: For input methods, implement or reuse and verify this exact obligation: script support. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for script support; retain observable state/resource expectations.
- [ ] **F-IN-010.06 — Implement/prove: focus/security and a11y**
  - Action: For input methods, implement or reuse and verify this exact obligation: focus/security and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-010.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus/security and a11y; retain observable state/resource expectations.
- [ ] **F-IN-010.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input methods: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-010.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-010.08 — Integrate into the real consumer and runtime route**
  - Action: Wire input methods into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-010.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-010.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input methods as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-010.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-011"></a>
## F-IN-011 — pointer acceleration

**Original requirement:** time/velocity-normalized curve, speed, raw toggle and reproducible aiming tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-011.01 — Reconcile existing pointer acceleration**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for pointer acceleration. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: time/velocity-normalized curve, speed, raw toggle and reproducible aiming tests
- [ ] **F-IN-011.02 — Freeze the exact contract for pointer acceleration**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: time/velocity-normalized curve, speed, raw toggle and reproducible aiming tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-011.03 — Implement/prove: time/velocity-normalized curve**
  - Action: For pointer acceleration, implement or reuse and verify this exact obligation: time/velocity-normalized curve. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time/velocity-normalized curve; retain observable state/resource expectations.
- [ ] **F-IN-011.04 — Implement/prove: speed**
  - Action: For pointer acceleration, implement or reuse and verify this exact obligation: speed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for speed; retain observable state/resource expectations.
- [ ] **F-IN-011.05 — Implement/prove: raw toggle and reproducible aiming tests**
  - Action: For pointer acceleration, implement or reuse and verify this exact obligation: raw toggle and reproducible aiming tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for raw toggle and reproducible aiming tests; retain observable state/resource expectations.
- [ ] **F-IN-011.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to pointer acceleration: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-011.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-011.07 — Integrate into the real consumer and runtime route**
  - Action: Wire pointer acceleration into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-011.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-011.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for pointer acceleration as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-011.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-012"></a>
## F-IN-012 — pointer confinement/lock

**Original requirement:** explicit app/user grant, escape route, focus loss and remote-session behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-012.01 — Reconcile existing pointer confinement/lock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for pointer confinement/lock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit app/user grant, escape route, focus loss and remote-session behavior
- [ ] **F-IN-012.02 — Freeze the exact contract for pointer confinement/lock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit app/user grant, escape route, focus loss and remote-session behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-012.03 — Implement/prove: explicit app/user grant**
  - Action: For pointer confinement/lock, implement or reuse and verify this exact obligation: explicit app/user grant. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit app/user grant; retain observable state/resource expectations.
- [ ] **F-IN-012.04 — Implement/prove: escape route**
  - Action: For pointer confinement/lock, implement or reuse and verify this exact obligation: escape route. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for escape route; retain observable state/resource expectations.
- [ ] **F-IN-012.05 — Implement/prove: focus loss and remote-session behavior**
  - Action: For pointer confinement/lock, implement or reuse and verify this exact obligation: focus loss and remote-session behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus loss and remote-session behavior; retain observable state/resource expectations.
- [ ] **F-IN-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to pointer confinement/lock: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire pointer confinement/lock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for pointer confinement/lock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-013"></a>
## F-IN-013 — wheel/high-resolution scroll

**Original requirement:** lines/pixels/phases/inertia, user settings and reduced-motion policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-013.01 — Reconcile existing wheel/high-resolution scroll**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for wheel/high-resolution scroll. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: lines/pixels/phases/inertia, user settings and reduced-motion policy
- [ ] **F-IN-013.02 — Freeze the exact contract for wheel/high-resolution scroll**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: lines/pixels/phases/inertia, user settings and reduced-motion policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-013.03 — Implement/prove: lines/pixels/phases/inertia**
  - Action: For wheel/high-resolution scroll, implement or reuse and verify this exact obligation: lines/pixels/phases/inertia. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lines/pixels/phases/inertia; retain observable state/resource expectations.
- [ ] **F-IN-013.04 — Implement/prove: user settings and reduced-motion policy**
  - Action: For wheel/high-resolution scroll, implement or reuse and verify this exact obligation: user settings and reduced-motion policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user settings and reduced-motion policy; retain observable state/resource expectations.
- [ ] **F-IN-013.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to wheel/high-resolution scroll: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-013.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-013.06 — Integrate into the real consumer and runtime route**
  - Action: Wire wheel/high-resolution scroll into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-013.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-013.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for wheel/high-resolution scroll as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-013.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-014"></a>
## F-IN-014 — drag and drop

**Original requirement:** typed offers, target consent, actions, progress/cancel and source death

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-014.01 — Reconcile existing drag and drop**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for drag and drop. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: typed offers, target consent, actions, progress/cancel and source death
- [ ] **F-IN-014.02 — Freeze the exact contract for drag and drop**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: typed offers, target consent, actions, progress/cancel and source death. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-014.03 — Implement/prove: typed offers**
  - Action: For drag and drop, implement or reuse and verify this exact obligation: typed offers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for typed offers; retain observable state/resource expectations.
- [ ] **F-IN-014.04 — Implement/prove: target consent**
  - Action: For drag and drop, implement or reuse and verify this exact obligation: target consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target consent; retain observable state/resource expectations.
- [ ] **F-IN-014.05 — Implement/prove: actions**
  - Action: For drag and drop, implement or reuse and verify this exact obligation: actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for actions; retain observable state/resource expectations.
- [ ] **F-IN-014.06 — Implement/prove: progress/cancel and source death**
  - Action: For drag and drop, implement or reuse and verify this exact obligation: progress/cancel and source death. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-014.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress/cancel and source death; retain observable state/resource expectations.
- [ ] **F-IN-014.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to drag and drop: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-014.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-014.08 — Integrate into the real consumer and runtime route**
  - Action: Wire drag and drop into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-014.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-014.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for drag and drop as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-014.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-015"></a>
## F-IN-015 — touch gestures

**Original requirement:** tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-015.01 — Reconcile existing touch gestures**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for touch gestures. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation
- [ ] **F-IN-015.02 — Freeze the exact contract for touch gestures**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-015.03 — Implement/prove: tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation**
  - Action: For touch gestures, implement or reuse and verify this exact obligation: tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tap/scroll/pinch/rotate/system edge gestures with arbitration and cancellation; retain observable state/resource expectations.
- [ ] **F-IN-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to touch gestures: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire touch gestures into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for touch gestures as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-016"></a>
## F-IN-016 — stylus interaction

**Original requirement:** pressure/tilt/eraser/palm behavior and app semantic events

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-016.01 — Reconcile existing stylus interaction**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for stylus interaction. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: pressure/tilt/eraser/palm behavior and app semantic events
- [ ] **F-IN-016.02 — Freeze the exact contract for stylus interaction**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: pressure/tilt/eraser/palm behavior and app semantic events. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-016.03 — Implement/prove: pressure/tilt/eraser/palm behavior and app semantic events**
  - Action: For stylus interaction, implement or reuse and verify this exact obligation: pressure/tilt/eraser/palm behavior and app semantic events. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pressure/tilt/eraser/palm behavior and app semantic events; retain observable state/resource expectations.
- [ ] **F-IN-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to stylus interaction: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire stylus interaction into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for stylus interaction as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-017"></a>
## F-IN-017 — gamepad mapping

**Original requirement:** device profiles, dead zones, remap, hotplug, haptics and accessible alternatives

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-017.01 — Reconcile existing gamepad mapping**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for gamepad mapping. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: device profiles, dead zones, remap, hotplug, haptics and accessible alternatives
- [ ] **F-IN-017.02 — Freeze the exact contract for gamepad mapping**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: device profiles, dead zones, remap, hotplug, haptics and accessible alternatives. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-017.03 — Implement/prove: device profiles**
  - Action: For gamepad mapping, implement or reuse and verify this exact obligation: device profiles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device profiles; retain observable state/resource expectations.
- [ ] **F-IN-017.04 — Implement/prove: dead zones**
  - Action: For gamepad mapping, implement or reuse and verify this exact obligation: dead zones. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dead zones; retain observable state/resource expectations.
- [ ] **F-IN-017.05 — Implement/prove: remap**
  - Action: For gamepad mapping, implement or reuse and verify this exact obligation: remap. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for remap; retain observable state/resource expectations.
- [ ] **F-IN-017.06 — Implement/prove: hotplug**
  - Action: For gamepad mapping, implement or reuse and verify this exact obligation: hotplug. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hotplug; retain observable state/resource expectations.
- [ ] **F-IN-017.07 — Implement/prove: haptics and accessible alternatives**
  - Action: For gamepad mapping, implement or reuse and verify this exact obligation: haptics and accessible alternatives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-017.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for haptics and accessible alternatives; retain observable state/resource expectations.
- [ ] **F-IN-017.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to gamepad mapping: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-017.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-017.09 — Integrate into the real consumer and runtime route**
  - Action: Wire gamepad mapping into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-017.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-017.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for gamepad mapping as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-017.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-018"></a>
## F-IN-018 — text input/selection

**Original requirement:** cursor, range, word/line, bidi/grapheme, clipboard, composition and undo

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-018.01 — Reconcile existing text input/selection**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for text input/selection. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: cursor, range, word/line, bidi/grapheme, clipboard, composition and undo
- [ ] **F-IN-018.02 — Freeze the exact contract for text input/selection**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: cursor, range, word/line, bidi/grapheme, clipboard, composition and undo. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-018.03 — Implement/prove: cursor**
  - Action: For text input/selection, implement or reuse and verify this exact obligation: cursor. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cursor; retain observable state/resource expectations.
- [ ] **F-IN-018.04 — Implement/prove: range**
  - Action: For text input/selection, implement or reuse and verify this exact obligation: range. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for range; retain observable state/resource expectations.
- [ ] **F-IN-018.05 — Implement/prove: word/line**
  - Action: For text input/selection, implement or reuse and verify this exact obligation: word/line. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for word/line; retain observable state/resource expectations.
- [ ] **F-IN-018.06 — Implement/prove: bidi/grapheme**
  - Action: For text input/selection, implement or reuse and verify this exact obligation: bidi/grapheme. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-018.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bidi/grapheme; retain observable state/resource expectations.
- [ ] **F-IN-018.07 — Implement/prove: clipboard**
  - Action: For text input/selection, implement or reuse and verify this exact obligation: clipboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-018.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clipboard; retain observable state/resource expectations.
- [ ] **F-IN-018.08 — Implement/prove: composition and undo**
  - Action: For text input/selection, implement or reuse and verify this exact obligation: composition and undo. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-018.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for composition and undo; retain observable state/resource expectations.
- [ ] **F-IN-018.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to text input/selection: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-018.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-018.10 — Integrate into the real consumer and runtime route**
  - Action: Wire text input/selection into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-018.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-018.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for text input/selection as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-018.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-019"></a>
## F-IN-019 — context menu

**Original requirement:** target-relative placement, keyboard invocation, bounds, focus and dismissal

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-019.01 — Reconcile existing context menu**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for context menu. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: target-relative placement, keyboard invocation, bounds, focus and dismissal
- [ ] **F-IN-019.02 — Freeze the exact contract for context menu**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: target-relative placement, keyboard invocation, bounds, focus and dismissal. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-019.03 — Implement/prove: target-relative placement**
  - Action: For context menu, implement or reuse and verify this exact obligation: target-relative placement. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target-relative placement; retain observable state/resource expectations.
- [ ] **F-IN-019.04 — Implement/prove: keyboard invocation**
  - Action: For context menu, implement or reuse and verify this exact obligation: keyboard invocation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard invocation; retain observable state/resource expectations.
- [ ] **F-IN-019.05 — Implement/prove: bounds**
  - Action: For context menu, implement or reuse and verify this exact obligation: bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounds; retain observable state/resource expectations.
- [ ] **F-IN-019.06 — Implement/prove: focus and dismissal**
  - Action: For context menu, implement or reuse and verify this exact obligation: focus and dismissal. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus and dismissal; retain observable state/resource expectations.
- [ ] **F-IN-019.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to context menu: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-019.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-019.08 — Integrate into the real consumer and runtime route**
  - Action: Wire context menu into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-019.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-019.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for context menu as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-019.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-020"></a>
## F-IN-020 — tooltip/help affordance

**Original requirement:** delayed/keyboard-accessible explanation that never blocks interaction

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-020.01 — Reconcile existing tooltip/help affordance**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for tooltip/help affordance. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: delayed/keyboard-accessible explanation that never blocks interaction
- [ ] **F-IN-020.02 — Freeze the exact contract for tooltip/help affordance**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: delayed/keyboard-accessible explanation that never blocks interaction. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-020.03 — Implement/prove: delayed/keyboard-accessible explanation that never blocks interaction**
  - Action: For tooltip/help affordance, implement or reuse and verify this exact obligation: delayed/keyboard-accessible explanation that never blocks interaction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for delayed/keyboard-accessible explanation that never blocks interaction; retain observable state/resource expectations.
- [ ] **F-IN-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to tooltip/help affordance: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire tooltip/help affordance into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for tooltip/help affordance as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-021"></a>
## F-IN-021 — coarse-pointer mode

**Original requirement:** hit targets/layout/hover alternatives for touch and remote use

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-021.01 — Reconcile existing coarse-pointer mode**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for coarse-pointer mode. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hit targets/layout/hover alternatives for touch and remote use
- [ ] **F-IN-021.02 — Freeze the exact contract for coarse-pointer mode**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hit targets/layout/hover alternatives for touch and remote use. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-021.03 — Implement/prove: hit targets/layout/hover alternatives for touch and remote use**
  - Action: For coarse-pointer mode, implement or reuse and verify this exact obligation: hit targets/layout/hover alternatives for touch and remote use. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hit targets/layout/hover alternatives for touch and remote use; retain observable state/resource expectations.
- [ ] **F-IN-021.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to coarse-pointer mode: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-021.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-021.05 — Integrate into the real consumer and runtime route**
  - Action: Wire coarse-pointer mode into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-021.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-021.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for coarse-pointer mode as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-021.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-022"></a>
## F-IN-022 — device settings

**Original requirement:** per-device speed/layout/buttons/calibration profile with rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-022.01 — Reconcile existing device settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for device settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-device speed/layout/buttons/calibration profile with rollback
- [ ] **F-IN-022.02 — Freeze the exact contract for device settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-device speed/layout/buttons/calibration profile with rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-022.03 — Implement/prove: per-device speed/layout/buttons/calibration profile with rollback**
  - Action: For device settings, implement or reuse and verify this exact obligation: per-device speed/layout/buttons/calibration profile with rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-device speed/layout/buttons/calibration profile with rollback; retain observable state/resource expectations.
- [ ] **F-IN-022.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to device settings: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-022.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-022.05 — Integrate into the real consumer and runtime route**
  - Action: Wire device settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-022.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-022.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for device settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-022.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-023"></a>
## F-IN-023 — input privacy

**Original requirement:** password/secure surfaces, event isolation, recording indicator and permissions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-023.01 — Reconcile existing input privacy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input privacy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: password/secure surfaces, event isolation, recording indicator and permissions
- [ ] **F-IN-023.02 — Freeze the exact contract for input privacy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: password/secure surfaces, event isolation, recording indicator and permissions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-023.03 — Implement/prove: password/secure surfaces**
  - Action: For input privacy, implement or reuse and verify this exact obligation: password/secure surfaces. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for password/secure surfaces; retain observable state/resource expectations.
- [ ] **F-IN-023.04 — Implement/prove: event isolation**
  - Action: For input privacy, implement or reuse and verify this exact obligation: event isolation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for event isolation; retain observable state/resource expectations.
- [ ] **F-IN-023.05 — Implement/prove: recording indicator and permissions**
  - Action: For input privacy, implement or reuse and verify this exact obligation: recording indicator and permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recording indicator and permissions; retain observable state/resource expectations.
- [ ] **F-IN-023.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input privacy: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-023.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-023.07 — Integrate into the real consumer and runtime route**
  - Action: Wire input privacy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-023.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-023.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input privacy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-023.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-024"></a>
## F-IN-024 — input diagnostics

**Original requirement:** physical/logical events, modifiers, repeat, loss, device identity and export

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-024.01 — Reconcile existing input diagnostics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input diagnostics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: physical/logical events, modifiers, repeat, loss, device identity and export
- [ ] **F-IN-024.02 — Freeze the exact contract for input diagnostics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: physical/logical events, modifiers, repeat, loss, device identity and export. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-024.03 — Implement/prove: physical/logical events**
  - Action: For input diagnostics, implement or reuse and verify this exact obligation: physical/logical events. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for physical/logical events; retain observable state/resource expectations.
- [ ] **F-IN-024.04 — Implement/prove: modifiers**
  - Action: For input diagnostics, implement or reuse and verify this exact obligation: modifiers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modifiers; retain observable state/resource expectations.
- [ ] **F-IN-024.05 — Implement/prove: repeat**
  - Action: For input diagnostics, implement or reuse and verify this exact obligation: repeat. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for repeat; retain observable state/resource expectations.
- [ ] **F-IN-024.06 — Implement/prove: loss**
  - Action: For input diagnostics, implement or reuse and verify this exact obligation: loss. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-024.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for loss; retain observable state/resource expectations.
- [ ] **F-IN-024.07 — Implement/prove: device identity and export**
  - Action: For input diagnostics, implement or reuse and verify this exact obligation: device identity and export. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-024.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device identity and export; retain observable state/resource expectations.
- [ ] **F-IN-024.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input diagnostics: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-024.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-024.09 — Integrate into the real consumer and runtime route**
  - Action: Wire input diagnostics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-024.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-024.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input diagnostics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-024.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-in-025"></a>
## F-IN-025 — input latency receipt

**Original requirement:** per-class physical/QEMU path to first affected present, not controller-wide inference

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-IN-025.01 — Reconcile existing input latency receipt**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for input latency receipt. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-class physical/QEMU path to first affected present, not controller-wide inference
- [ ] **F-IN-025.02 — Freeze the exact contract for input latency receipt**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-class physical/QEMU path to first affected present, not controller-wide inference. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-IN-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-IN-025.03 — Implement/prove: per-class physical/QEMU path to first affected present**
  - Action: For input latency receipt, implement or reuse and verify this exact obligation: per-class physical/QEMU path to first affected present. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-class physical/QEMU path to first affected present; retain observable state/resource expectations.
- [ ] **F-IN-025.04 — Implement/prove: not controller-wide inference**
  - Action: For input latency receipt, implement or reuse and verify this exact obligation: not controller-wide inference. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-IN-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not controller-wide inference; retain observable state/resource expectations.
- [ ] **F-IN-025.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to input latency receipt: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-IN-025.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-IN-025.06 — Integrate into the real consumer and runtime route**
  - Action: Wire input latency receipt into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-IN-025.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-IN-025.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for input latency receipt as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-IN-025.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-001"></a>
## F-SH-001 — login experience

**Original requirement:** user selection, secure auth, recovery, accessibility and session start errors

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-001.01 — Reconcile existing login experience**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for login experience. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: user selection, secure auth, recovery, accessibility and session start errors
- [ ] **F-SH-001.02 — Freeze the exact contract for login experience**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: user selection, secure auth, recovery, accessibility and session start errors. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-001.03 — Implement/prove: user selection**
  - Action: For login experience, implement or reuse and verify this exact obligation: user selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user selection; retain observable state/resource expectations.
- [ ] **F-SH-001.04 — Implement/prove: secure auth**
  - Action: For login experience, implement or reuse and verify this exact obligation: secure auth. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for secure auth; retain observable state/resource expectations.
- [ ] **F-SH-001.05 — Implement/prove: recovery**
  - Action: For login experience, implement or reuse and verify this exact obligation: recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery; retain observable state/resource expectations.
- [ ] **F-SH-001.06 — Implement/prove: accessibility and session start errors**
  - Action: For login experience, implement or reuse and verify this exact obligation: accessibility and session start errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for accessibility and session start errors; retain observable state/resource expectations.
- [ ] **F-SH-001.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to login experience: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-001.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-001.08 — Integrate into the real consumer and runtime route**
  - Action: Wire login experience into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-001.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-001.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for login experience as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-001.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-002"></a>
## F-SH-002 — session startup

**Original requirement:** authenticated profile/settings/services/apps with readiness and rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-002.01 — Reconcile existing session startup**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for session startup. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated profile/settings/services/apps with readiness and rollback
- [ ] **F-SH-002.02 — Freeze the exact contract for session startup**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated profile/settings/services/apps with readiness and rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-002.03 — Implement/prove: authenticated profile/settings/services/apps with readiness and rollback**
  - Action: For session startup, implement or reuse and verify this exact obligation: authenticated profile/settings/services/apps with readiness and rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated profile/settings/services/apps with readiness and rollback; retain observable state/resource expectations.
- [ ] **F-SH-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to session startup: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire session startup into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for session startup as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-003"></a>
## F-SH-003 — lock/unlock

**Original requirement:** compositor-owned secure surface, notifications/privacy policy and recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-003.01 — Reconcile existing lock/unlock**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for lock/unlock. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: compositor-owned secure surface, notifications/privacy policy and recovery
- [ ] **F-SH-003.02 — Freeze the exact contract for lock/unlock**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: compositor-owned secure surface, notifications/privacy policy and recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-003.03 — Implement/prove: compositor-owned secure surface**
  - Action: For lock/unlock, implement or reuse and verify this exact obligation: compositor-owned secure surface. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compositor-owned secure surface; retain observable state/resource expectations.
- [ ] **F-SH-003.04 — Implement/prove: notifications/privacy policy and recovery**
  - Action: For lock/unlock, implement or reuse and verify this exact obligation: notifications/privacy policy and recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for notifications/privacy policy and recovery; retain observable state/resource expectations.
- [ ] **F-SH-003.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to lock/unlock: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-003.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-003.06 — Integrate into the real consumer and runtime route**
  - Action: Wire lock/unlock into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-003.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-003.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for lock/unlock as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-003.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-004"></a>
## F-SH-004 — logout/shutdown/restart UX

**Original requirement:** unsaved-work negotiation, deadlines, progress, errors and forced path

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-004.01 — Reconcile existing logout/shutdown/restart UX**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for logout/shutdown/restart UX. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: unsaved-work negotiation, deadlines, progress, errors and forced path
- [ ] **F-SH-004.02 — Freeze the exact contract for logout/shutdown/restart UX**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: unsaved-work negotiation, deadlines, progress, errors and forced path. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-004.03 — Implement/prove: unsaved-work negotiation**
  - Action: For logout/shutdown/restart UX, implement or reuse and verify this exact obligation: unsaved-work negotiation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for unsaved-work negotiation; retain observable state/resource expectations.
- [ ] **F-SH-004.04 — Implement/prove: deadlines**
  - Action: For logout/shutdown/restart UX, implement or reuse and verify this exact obligation: deadlines. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deadlines; retain observable state/resource expectations.
- [ ] **F-SH-004.05 — Implement/prove: progress**
  - Action: For logout/shutdown/restart UX, implement or reuse and verify this exact obligation: progress. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress; retain observable state/resource expectations.
- [ ] **F-SH-004.06 — Implement/prove: errors and forced path**
  - Action: For logout/shutdown/restart UX, implement or reuse and verify this exact obligation: errors and forced path. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-004.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors and forced path; retain observable state/resource expectations.
- [ ] **F-SH-004.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to logout/shutdown/restart UX: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-004.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-004.08 — Integrate into the real consumer and runtime route**
  - Action: Wire logout/shutdown/restart UX into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-004.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-004.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for logout/shutdown/restart UX as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-004.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-005"></a>
## F-SH-005 — desktop background

**Original requirement:** wallpaper/color/theme, scaling, preview, cache, per-user persistence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-005.01 — Reconcile existing desktop background**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for desktop background. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: wallpaper/color/theme, scaling, preview, cache, per-user persistence
- [ ] **F-SH-005.02 — Freeze the exact contract for desktop background**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: wallpaper/color/theme, scaling, preview, cache, per-user persistence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-005.03 — Implement/prove: wallpaper/color/theme**
  - Action: For desktop background, implement or reuse and verify this exact obligation: wallpaper/color/theme. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wallpaper/color/theme; retain observable state/resource expectations.
- [ ] **F-SH-005.04 — Implement/prove: scaling**
  - Action: For desktop background, implement or reuse and verify this exact obligation: scaling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scaling; retain observable state/resource expectations.
- [ ] **F-SH-005.05 — Implement/prove: preview**
  - Action: For desktop background, implement or reuse and verify this exact obligation: preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preview; retain observable state/resource expectations.
- [ ] **F-SH-005.06 — Implement/prove: cache**
  - Action: For desktop background, implement or reuse and verify this exact obligation: cache. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache; retain observable state/resource expectations.
- [ ] **F-SH-005.07 — Implement/prove: per-user persistence**
  - Action: For desktop background, implement or reuse and verify this exact obligation: per-user persistence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-005.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user persistence; retain observable state/resource expectations.
- [ ] **F-SH-005.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to desktop background: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-005.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-005.09 — Integrate into the real consumer and runtime route**
  - Action: Wire desktop background into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-005.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-005.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for desktop background as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-005.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-006"></a>
## F-SH-006 — top/status bar

**Original requirement:** truthful clock/network/power/audio/session indicators and overflow

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-006.01 — Reconcile existing top/status bar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for top/status bar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: truthful clock/network/power/audio/session indicators and overflow
- [ ] **F-SH-006.02 — Freeze the exact contract for top/status bar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: truthful clock/network/power/audio/session indicators and overflow. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-006.03 — Implement/prove: truthful clock/network/power/audio/session indicators and overflow**
  - Action: For top/status bar, implement or reuse and verify this exact obligation: truthful clock/network/power/audio/session indicators and overflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for truthful clock/network/power/audio/session indicators and overflow; retain observable state/resource expectations.
- [ ] **F-SH-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to top/status bar: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire top/status bar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for top/status bar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-007"></a>
## F-SH-007 — dock/taskbar

**Original requirement:** pinned/running/active/minimized states, grouping, reorder and multi-display policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-007.01 — Reconcile existing dock/taskbar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for dock/taskbar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: pinned/running/active/minimized states, grouping, reorder and multi-display policy
- [ ] **F-SH-007.02 — Freeze the exact contract for dock/taskbar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: pinned/running/active/minimized states, grouping, reorder and multi-display policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-007.03 — Implement/prove: pinned/running/active/minimized states**
  - Action: For dock/taskbar, implement or reuse and verify this exact obligation: pinned/running/active/minimized states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pinned/running/active/minimized states; retain observable state/resource expectations.
- [ ] **F-SH-007.04 — Implement/prove: grouping**
  - Action: For dock/taskbar, implement or reuse and verify this exact obligation: grouping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grouping; retain observable state/resource expectations.
- [ ] **F-SH-007.05 — Implement/prove: reorder and multi-display policy**
  - Action: For dock/taskbar, implement or reuse and verify this exact obligation: reorder and multi-display policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reorder and multi-display policy; retain observable state/resource expectations.
- [ ] **F-SH-007.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to dock/taskbar: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-007.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-007.07 — Integrate into the real consumer and runtime route**
  - Action: Wire dock/taskbar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-007.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-007.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for dock/taskbar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-007.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-008"></a>
## F-SH-008 — application menu

**Original requirement:** generated from admitted packages; search/category/recent and no dead routes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-008.01 — Reconcile existing application menu**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for application menu. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated from admitted packages; search/category/recent and no dead routes
- [ ] **F-SH-008.02 — Freeze the exact contract for application menu**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated from admitted packages; search/category/recent and no dead routes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-008.03 — Implement/prove: generated from admitted packages**
  - Action: For application menu, implement or reuse and verify this exact obligation: generated from admitted packages. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated from admitted packages; retain observable state/resource expectations.
- [ ] **F-SH-008.04 — Implement/prove: search/category/recent and no dead routes**
  - Action: For application menu, implement or reuse and verify this exact obligation: search/category/recent and no dead routes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search/category/recent and no dead routes; retain observable state/resource expectations.
- [ ] **F-SH-008.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to application menu: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-008.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-008.06 — Integrate into the real consumer and runtime route**
  - Action: Wire application menu into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-008.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-008.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for application menu as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-008.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-009"></a>
## F-SH-009 — launcher/search

**Original requirement:** app/file/setting/action search with ranking, keyboard and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-009.01 — Reconcile existing launcher/search**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for launcher/search. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: app/file/setting/action search with ranking, keyboard and privacy
- [ ] **F-SH-009.02 — Freeze the exact contract for launcher/search**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: app/file/setting/action search with ranking, keyboard and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-009.03 — Implement/prove: app/file/setting/action search with ranking**
  - Action: For launcher/search, implement or reuse and verify this exact obligation: app/file/setting/action search with ranking. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/file/setting/action search with ranking; retain observable state/resource expectations.
- [ ] **F-SH-009.04 — Implement/prove: keyboard and privacy**
  - Action: For launcher/search, implement or reuse and verify this exact obligation: keyboard and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard and privacy; retain observable state/resource expectations.
- [ ] **F-SH-009.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to launcher/search: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-009.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-009.06 — Integrate into the real consumer and runtime route**
  - Action: Wire launcher/search into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-009.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-009.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for launcher/search as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-009.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-010"></a>
## F-SH-010 — Run command

**Original requirement:** explicit parsing/authority/errors/history and app metadata resolution

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-010.01 — Reconcile existing Run command**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Run command. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit parsing/authority/errors/history and app metadata resolution
- [ ] **F-SH-010.02 — Freeze the exact contract for Run command**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit parsing/authority/errors/history and app metadata resolution. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-010.03 — Implement/prove: explicit parsing/authority/errors/history and app metadata resolution**
  - Action: For Run command, implement or reuse and verify this exact obligation: explicit parsing/authority/errors/history and app metadata resolution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit parsing/authority/errors/history and app metadata resolution; retain observable state/resource expectations.
- [ ] **F-SH-010.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Run command: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-010.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-010.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Run command into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-010.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-010.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Run command as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-010.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-011"></a>
## F-SH-011 — All Applications catalog

**Original requirement:** exact registry parity, metadata, install state, evidence and launchability

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-011.01 — Reconcile existing All Applications catalog**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for All Applications catalog. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact registry parity, metadata, install state, evidence and launchability
- [ ] **F-SH-011.02 — Freeze the exact contract for All Applications catalog**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact registry parity, metadata, install state, evidence and launchability. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-011.03 — Implement/prove: exact registry parity**
  - Action: For All Applications catalog, implement or reuse and verify this exact obligation: exact registry parity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact registry parity; retain observable state/resource expectations.
- [ ] **F-SH-011.04 — Implement/prove: metadata**
  - Action: For All Applications catalog, implement or reuse and verify this exact obligation: metadata. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for metadata; retain observable state/resource expectations.
- [ ] **F-SH-011.05 — Implement/prove: install state**
  - Action: For All Applications catalog, implement or reuse and verify this exact obligation: install state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for install state; retain observable state/resource expectations.
- [ ] **F-SH-011.06 — Implement/prove: evidence and launchability**
  - Action: For All Applications catalog, implement or reuse and verify this exact obligation: evidence and launchability. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-011.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for evidence and launchability; retain observable state/resource expectations.
- [ ] **F-SH-011.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to All Applications catalog: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-011.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-011.08 — Integrate into the real consumer and runtime route**
  - Action: Wire All Applications catalog into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-011.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-011.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for All Applications catalog as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-011.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-012"></a>
## F-SH-012 — window create/close

**Original requirement:** owner authority, ready nonce, graceful close/deadline/force and cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-012.01 — Reconcile existing window create/close**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for window create/close. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: owner authority, ready nonce, graceful close/deadline/force and cleanup
- [ ] **F-SH-012.02 — Freeze the exact contract for window create/close**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: owner authority, ready nonce, graceful close/deadline/force and cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-012.03 — Implement/prove: owner authority**
  - Action: For window create/close, implement or reuse and verify this exact obligation: owner authority. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for owner authority; retain observable state/resource expectations.
- [ ] **F-SH-012.04 — Implement/prove: ready nonce**
  - Action: For window create/close, implement or reuse and verify this exact obligation: ready nonce. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ready nonce; retain observable state/resource expectations.
- [ ] **F-SH-012.05 — Implement/prove: graceful close/deadline/force and cleanup**
  - Action: For window create/close, implement or reuse and verify this exact obligation: graceful close/deadline/force and cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for graceful close/deadline/force and cleanup; retain observable state/resource expectations.
- [ ] **F-SH-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to window create/close: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire window create/close into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for window create/close as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-013"></a>
## F-SH-013 — window move

**Original requirement:** drag handles/Alt path, multi-display bounds, retained composition

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-013.01 — Reconcile existing window move**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for window move. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: drag handles/Alt path, multi-display bounds, retained composition
- [ ] **F-SH-013.02 — Freeze the exact contract for window move**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: drag handles/Alt path, multi-display bounds, retained composition. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-013.03 — Implement/prove: drag handles/Alt path**
  - Action: For window move, implement or reuse and verify this exact obligation: drag handles/Alt path. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drag handles/Alt path; retain observable state/resource expectations.
- [ ] **F-SH-013.04 — Implement/prove: multi-display bounds**
  - Action: For window move, implement or reuse and verify this exact obligation: multi-display bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multi-display bounds; retain observable state/resource expectations.
- [ ] **F-SH-013.05 — Implement/prove: retained composition**
  - Action: For window move, implement or reuse and verify this exact obligation: retained composition. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for retained composition; retain observable state/resource expectations.
- [ ] **F-SH-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to window move: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire window move into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for window move as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-014"></a>
## F-SH-014 — window resize

**Original requirement:** edge/corner/keyboard, minimum/maximum, configure/ack and preview

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-014.01 — Reconcile existing window resize**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for window resize. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: edge/corner/keyboard, minimum/maximum, configure/ack and preview
- [ ] **F-SH-014.02 — Freeze the exact contract for window resize**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: edge/corner/keyboard, minimum/maximum, configure/ack and preview. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-014.03 — Implement/prove: edge/corner/keyboard**
  - Action: For window resize, implement or reuse and verify this exact obligation: edge/corner/keyboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for edge/corner/keyboard; retain observable state/resource expectations.
- [ ] **F-SH-014.04 — Implement/prove: minimum/maximum**
  - Action: For window resize, implement or reuse and verify this exact obligation: minimum/maximum. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-014.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimum/maximum; retain observable state/resource expectations.
- [ ] **F-SH-014.05 — Implement/prove: configure/ack and preview**
  - Action: For window resize, implement or reuse and verify this exact obligation: configure/ack and preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-014.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configure/ack and preview; retain observable state/resource expectations.
- [ ] **F-SH-014.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to window resize: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-014.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-014.07 — Integrate into the real consumer and runtime route**
  - Action: Wire window resize into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-014.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-014.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for window resize as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-014.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-015"></a>
## F-SH-015 — minimize/restore

**Original requirement:** taskbar state, focus fallback, animation/reduced motion and app notification

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-015.01 — Reconcile existing minimize/restore**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for minimize/restore. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: taskbar state, focus fallback, animation/reduced motion and app notification
- [ ] **F-SH-015.02 — Freeze the exact contract for minimize/restore**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: taskbar state, focus fallback, animation/reduced motion and app notification. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-015.03 — Implement/prove: taskbar state**
  - Action: For minimize/restore, implement or reuse and verify this exact obligation: taskbar state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for taskbar state; retain observable state/resource expectations.
- [ ] **F-SH-015.04 — Implement/prove: focus fallback**
  - Action: For minimize/restore, implement or reuse and verify this exact obligation: focus fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus fallback; retain observable state/resource expectations.
- [ ] **F-SH-015.05 — Implement/prove: animation/reduced motion and app notification**
  - Action: For minimize/restore, implement or reuse and verify this exact obligation: animation/reduced motion and app notification. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-015.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for animation/reduced motion and app notification; retain observable state/resource expectations.
- [ ] **F-SH-015.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to minimize/restore: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-015.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-015.07 — Integrate into the real consumer and runtime route**
  - Action: Wire minimize/restore into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-015.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-015.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for minimize/restore as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-015.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-016"></a>
## F-SH-016 — maximize/restore

**Original requirement:** work-area geometry, multi-display and correct hit regions

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-016.01 — Reconcile existing maximize/restore**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for maximize/restore. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: work-area geometry, multi-display and correct hit regions
- [ ] **F-SH-016.02 — Freeze the exact contract for maximize/restore**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: work-area geometry, multi-display and correct hit regions. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-016.03 — Implement/prove: work-area geometry**
  - Action: For maximize/restore, implement or reuse and verify this exact obligation: work-area geometry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for work-area geometry; retain observable state/resource expectations.
- [ ] **F-SH-016.04 — Implement/prove: multi-display and correct hit regions**
  - Action: For maximize/restore, implement or reuse and verify this exact obligation: multi-display and correct hit regions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-016.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for multi-display and correct hit regions; retain observable state/resource expectations.
- [ ] **F-SH-016.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to maximize/restore: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-016.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-016.06 — Integrate into the real consumer and runtime route**
  - Action: Wire maximize/restore into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-016.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-016.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for maximize/restore as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-016.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-017"></a>
## F-SH-017 — snap/tile

**Original requirement:** edge/keyboard layouts, preview, gaps, restore and accessibility

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-017.01 — Reconcile existing snap/tile**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for snap/tile. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: edge/keyboard layouts, preview, gaps, restore and accessibility
- [ ] **F-SH-017.02 — Freeze the exact contract for snap/tile**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: edge/keyboard layouts, preview, gaps, restore and accessibility. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-017.03 — Implement/prove: edge/keyboard layouts**
  - Action: For snap/tile, implement or reuse and verify this exact obligation: edge/keyboard layouts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for edge/keyboard layouts; retain observable state/resource expectations.
- [ ] **F-SH-017.04 — Implement/prove: preview**
  - Action: For snap/tile, implement or reuse and verify this exact obligation: preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preview; retain observable state/resource expectations.
- [ ] **F-SH-017.05 — Implement/prove: gaps**
  - Action: For snap/tile, implement or reuse and verify this exact obligation: gaps. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for gaps; retain observable state/resource expectations.
- [ ] **F-SH-017.06 — Implement/prove: restore and accessibility**
  - Action: For snap/tile, implement or reuse and verify this exact obligation: restore and accessibility. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-017.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for restore and accessibility; retain observable state/resource expectations.
- [ ] **F-SH-017.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to snap/tile: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-017.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-017.08 — Integrate into the real consumer and runtime route**
  - Action: Wire snap/tile into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-017.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-017.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for snap/tile as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-017.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-018"></a>
## F-SH-018 — fullscreen

**Original requirement:** explicit entry/exit, overlays, shortcuts and focus/input policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-018.01 — Reconcile existing fullscreen**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for fullscreen. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: explicit entry/exit, overlays, shortcuts and focus/input policy
- [ ] **F-SH-018.02 — Freeze the exact contract for fullscreen**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: explicit entry/exit, overlays, shortcuts and focus/input policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-018.03 — Implement/prove: explicit entry/exit**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: explicit entry/exit. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for explicit entry/exit; retain observable state/resource expectations.
- [ ] **F-SH-018.04 — Implement/prove: overlays**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: overlays. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overlays; retain observable state/resource expectations.
- [ ] **F-SH-018.05 — Implement/prove: shortcuts and focus/input policy**
  - Action: For fullscreen, implement or reuse and verify this exact obligation: shortcuts and focus/input policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shortcuts and focus/input policy; retain observable state/resource expectations.
- [ ] **F-SH-018.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to fullscreen: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-018.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-018.07 — Integrate into the real consumer and runtime route**
  - Action: Wire fullscreen into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-018.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-018.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for fullscreen as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-018.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-019"></a>
## F-SH-019 — window switcher

**Original requirement:** keyboard ordering, preview, titles/icons, minimized apps and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-019.01 — Reconcile existing window switcher**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for window switcher. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: keyboard ordering, preview, titles/icons, minimized apps and a11y
- [ ] **F-SH-019.02 — Freeze the exact contract for window switcher**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: keyboard ordering, preview, titles/icons, minimized apps and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-019.03 — Implement/prove: keyboard ordering**
  - Action: For window switcher, implement or reuse and verify this exact obligation: keyboard ordering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard ordering; retain observable state/resource expectations.
- [ ] **F-SH-019.04 — Implement/prove: preview**
  - Action: For window switcher, implement or reuse and verify this exact obligation: preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for preview; retain observable state/resource expectations.
- [ ] **F-SH-019.05 — Implement/prove: titles/icons**
  - Action: For window switcher, implement or reuse and verify this exact obligation: titles/icons. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for titles/icons; retain observable state/resource expectations.
- [ ] **F-SH-019.06 — Implement/prove: minimized apps and a11y**
  - Action: For window switcher, implement or reuse and verify this exact obligation: minimized apps and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimized apps and a11y; retain observable state/resource expectations.
- [ ] **F-SH-019.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to window switcher: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-019.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-019.08 — Integrate into the real consumer and runtime route**
  - Action: Wire window switcher into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-019.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-019.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for window switcher as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-019.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-020"></a>
## F-SH-020 — workspace overview

**Original requirement:** thumbnails, drag/move, keyboard, search and reduced-motion path

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-020.01 — Reconcile existing workspace overview**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for workspace overview. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: thumbnails, drag/move, keyboard, search and reduced-motion path
- [ ] **F-SH-020.02 — Freeze the exact contract for workspace overview**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: thumbnails, drag/move, keyboard, search and reduced-motion path. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-020.03 — Implement/prove: thumbnails**
  - Action: For workspace overview, implement or reuse and verify this exact obligation: thumbnails. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for thumbnails; retain observable state/resource expectations.
- [ ] **F-SH-020.04 — Implement/prove: drag/move**
  - Action: For workspace overview, implement or reuse and verify this exact obligation: drag/move. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drag/move; retain observable state/resource expectations.
- [ ] **F-SH-020.05 — Implement/prove: keyboard**
  - Action: For workspace overview, implement or reuse and verify this exact obligation: keyboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard; retain observable state/resource expectations.
- [ ] **F-SH-020.06 — Implement/prove: search and reduced-motion path**
  - Action: For workspace overview, implement or reuse and verify this exact obligation: search and reduced-motion path. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-020.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search and reduced-motion path; retain observable state/resource expectations.
- [ ] **F-SH-020.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to workspace overview: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-020.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-020.08 — Integrate into the real consumer and runtime route**
  - Action: Wire workspace overview into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-020.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-020.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for workspace overview as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-020.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-021"></a>
## F-SH-021 — multiple workspaces

**Original requirement:** persistent names/order, app/window association and shortcuts

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-021.01 — Reconcile existing multiple workspaces**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for multiple workspaces. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: persistent names/order, app/window association and shortcuts
- [ ] **F-SH-021.02 — Freeze the exact contract for multiple workspaces**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: persistent names/order, app/window association and shortcuts. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-021.03 — Implement/prove: persistent names/order**
  - Action: For multiple workspaces, implement or reuse and verify this exact obligation: persistent names/order. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistent names/order; retain observable state/resource expectations.
- [ ] **F-SH-021.04 — Implement/prove: app/window association and shortcuts**
  - Action: For multiple workspaces, implement or reuse and verify this exact obligation: app/window association and shortcuts. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/window association and shortcuts; retain observable state/resource expectations.
- [ ] **F-SH-021.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to multiple workspaces: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-021.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-021.06 — Integrate into the real consumer and runtime route**
  - Action: Wire multiple workspaces into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-021.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-021.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for multiple workspaces as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-021.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-022"></a>
## F-SH-022 — modal/transient windows

**Original requirement:** parent ownership, stacking, focus, block scope and peer death

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-022.01 — Reconcile existing modal/transient windows**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for modal/transient windows. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: parent ownership, stacking, focus, block scope and peer death
- [ ] **F-SH-022.02 — Freeze the exact contract for modal/transient windows**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: parent ownership, stacking, focus, block scope and peer death. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-022.03 — Implement/prove: parent ownership**
  - Action: For modal/transient windows, implement or reuse and verify this exact obligation: parent ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for parent ownership; retain observable state/resource expectations.
- [ ] **F-SH-022.04 — Implement/prove: stacking**
  - Action: For modal/transient windows, implement or reuse and verify this exact obligation: stacking. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stacking; retain observable state/resource expectations.
- [ ] **F-SH-022.05 — Implement/prove: focus**
  - Action: For modal/transient windows, implement or reuse and verify this exact obligation: focus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus; retain observable state/resource expectations.
- [ ] **F-SH-022.06 — Implement/prove: block scope and peer death**
  - Action: For modal/transient windows, implement or reuse and verify this exact obligation: block scope and peer death. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for block scope and peer death; retain observable state/resource expectations.
- [ ] **F-SH-022.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to modal/transient windows: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-022.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-022.08 — Integrate into the real consumer and runtime route**
  - Action: Wire modal/transient windows into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-022.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-022.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for modal/transient windows as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-022.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-023"></a>
## F-SH-023 — notification service

**Original requirement:** app permissions, banners, actions, grouping, do-not-disturb and history

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-023.01 — Reconcile existing notification service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for notification service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: app permissions, banners, actions, grouping, do-not-disturb and history
- [ ] **F-SH-023.02 — Freeze the exact contract for notification service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: app permissions, banners, actions, grouping, do-not-disturb and history. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-023.03 — Implement/prove: app permissions**
  - Action: For notification service, implement or reuse and verify this exact obligation: app permissions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app permissions; retain observable state/resource expectations.
- [ ] **F-SH-023.04 — Implement/prove: banners**
  - Action: For notification service, implement or reuse and verify this exact obligation: banners. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for banners; retain observable state/resource expectations.
- [ ] **F-SH-023.05 — Implement/prove: actions**
  - Action: For notification service, implement or reuse and verify this exact obligation: actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for actions; retain observable state/resource expectations.
- [ ] **F-SH-023.06 — Implement/prove: grouping**
  - Action: For notification service, implement or reuse and verify this exact obligation: grouping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grouping; retain observable state/resource expectations.
- [ ] **F-SH-023.07 — Implement/prove: do-not-disturb and history**
  - Action: For notification service, implement or reuse and verify this exact obligation: do-not-disturb and history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-023.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for do-not-disturb and history; retain observable state/resource expectations.
- [ ] **F-SH-023.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to notification service: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-023.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-023.09 — Integrate into the real consumer and runtime route**
  - Action: Wire notification service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-023.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-023.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for notification service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-023.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-024"></a>
## F-SH-024 — notification center

**Original requirement:** searchable history, clear/settings, privacy and screen-reader announcements

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-024.01 — Reconcile existing notification center**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for notification center. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: searchable history, clear/settings, privacy and screen-reader announcements
- [ ] **F-SH-024.02 — Freeze the exact contract for notification center**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: searchable history, clear/settings, privacy and screen-reader announcements. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-024.03 — Implement/prove: searchable history**
  - Action: For notification center, implement or reuse and verify this exact obligation: searchable history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for searchable history; retain observable state/resource expectations.
- [ ] **F-SH-024.04 — Implement/prove: clear/settings**
  - Action: For notification center, implement or reuse and verify this exact obligation: clear/settings. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clear/settings; retain observable state/resource expectations.
- [ ] **F-SH-024.05 — Implement/prove: privacy and screen-reader announcements**
  - Action: For notification center, implement or reuse and verify this exact obligation: privacy and screen-reader announcements. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and screen-reader announcements; retain observable state/resource expectations.
- [ ] **F-SH-024.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to notification center: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-024.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-024.07 — Integrate into the real consumer and runtime route**
  - Action: Wire notification center into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-024.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-024.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for notification center as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-024.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-025"></a>
## F-SH-025 — clipboard service

**Original requirement:** MIME offers, source/recipient, size, expiry, history, privacy and live revoke

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-025.01 — Reconcile existing clipboard service**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for clipboard service. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: MIME offers, source/recipient, size, expiry, history, privacy and live revoke
- [ ] **F-SH-025.02 — Freeze the exact contract for clipboard service**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: MIME offers, source/recipient, size, expiry, history, privacy and live revoke. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-025.03 — Implement/prove: MIME offers**
  - Action: For clipboard service, implement or reuse and verify this exact obligation: MIME offers. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MIME offers; retain observable state/resource expectations.
- [ ] **F-SH-025.04 — Implement/prove: source/recipient**
  - Action: For clipboard service, implement or reuse and verify this exact obligation: source/recipient. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for source/recipient; retain observable state/resource expectations.
- [ ] **F-SH-025.05 — Implement/prove: size**
  - Action: For clipboard service, implement or reuse and verify this exact obligation: size. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for size; retain observable state/resource expectations.
- [ ] **F-SH-025.06 — Implement/prove: expiry**
  - Action: For clipboard service, implement or reuse and verify this exact obligation: expiry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-025.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for expiry; retain observable state/resource expectations.
- [ ] **F-SH-025.07 — Implement/prove: history**
  - Action: For clipboard service, implement or reuse and verify this exact obligation: history. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-025.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for history; retain observable state/resource expectations.
- [ ] **F-SH-025.08 — Implement/prove: privacy and live revoke**
  - Action: For clipboard service, implement or reuse and verify this exact obligation: privacy and live revoke. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-025.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for privacy and live revoke; retain observable state/resource expectations.
- [ ] **F-SH-025.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to clipboard service: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-025.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-025.10 — Integrate into the real consumer and runtime route**
  - Action: Wire clipboard service into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-025.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-025.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for clipboard service as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-025.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-026"></a>
## F-SH-026 — clipboard history UI

**Original requirement:** opt-in sensitive filtering, search, delete and per-app attribution

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-026.01 — Reconcile existing clipboard history UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for clipboard history UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: opt-in sensitive filtering, search, delete and per-app attribution
- [ ] **F-SH-026.02 — Freeze the exact contract for clipboard history UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: opt-in sensitive filtering, search, delete and per-app attribution. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-026.03 — Implement/prove: opt-in sensitive filtering**
  - Action: For clipboard history UI, implement or reuse and verify this exact obligation: opt-in sensitive filtering. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for opt-in sensitive filtering; retain observable state/resource expectations.
- [ ] **F-SH-026.04 — Implement/prove: search**
  - Action: For clipboard history UI, implement or reuse and verify this exact obligation: search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for search; retain observable state/resource expectations.
- [ ] **F-SH-026.05 — Implement/prove: delete and per-app attribution**
  - Action: For clipboard history UI, implement or reuse and verify this exact obligation: delete and per-app attribution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for delete and per-app attribution; retain observable state/resource expectations.
- [ ] **F-SH-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to clipboard history UI: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire clipboard history UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for clipboard history UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-027"></a>
## F-SH-027 — drag-and-drop broker

**Original requirement:** capability transfer, MIME/actions, progress/cancel and failure rollback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-027.01 — Reconcile existing drag-and-drop broker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for drag-and-drop broker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: capability transfer, MIME/actions, progress/cancel and failure rollback
- [ ] **F-SH-027.02 — Freeze the exact contract for drag-and-drop broker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: capability transfer, MIME/actions, progress/cancel and failure rollback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-027.03 — Implement/prove: capability transfer**
  - Action: For drag-and-drop broker, implement or reuse and verify this exact obligation: capability transfer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for capability transfer; retain observable state/resource expectations.
- [ ] **F-SH-027.04 — Implement/prove: MIME/actions**
  - Action: For drag-and-drop broker, implement or reuse and verify this exact obligation: MIME/actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for MIME/actions; retain observable state/resource expectations.
- [ ] **F-SH-027.05 — Implement/prove: progress/cancel and failure rollback**
  - Action: For drag-and-drop broker, implement or reuse and verify this exact obligation: progress/cancel and failure rollback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress/cancel and failure rollback; retain observable state/resource expectations.
- [ ] **F-SH-027.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to drag-and-drop broker: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-027.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-027.07 — Integrate into the real consumer and runtime route**
  - Action: Wire drag-and-drop broker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-027.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-027.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for drag-and-drop broker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-027.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-028"></a>
## F-SH-028 — file chooser portal

**Original requirement:** open/save/folder modes, recent/places/search, scoped result and atomic save

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-028.01 — Reconcile existing file chooser portal**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for file chooser portal. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: open/save/folder modes, recent/places/search, scoped result and atomic save
- [ ] **F-SH-028.02 — Freeze the exact contract for file chooser portal**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: open/save/folder modes, recent/places/search, scoped result and atomic save. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-028.03 — Implement/prove: open/save/folder modes**
  - Action: For file chooser portal, implement or reuse and verify this exact obligation: open/save/folder modes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for open/save/folder modes; retain observable state/resource expectations.
- [ ] **F-SH-028.04 — Implement/prove: recent/places/search**
  - Action: For file chooser portal, implement or reuse and verify this exact obligation: recent/places/search. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recent/places/search; retain observable state/resource expectations.
- [ ] **F-SH-028.05 — Implement/prove: scoped result and atomic save**
  - Action: For file chooser portal, implement or reuse and verify this exact obligation: scoped result and atomic save. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scoped result and atomic save; retain observable state/resource expectations.
- [ ] **F-SH-028.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to file chooser portal: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-028.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-028.07 — Integrate into the real consumer and runtime route**
  - Action: Wire file chooser portal into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-028.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-028.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for file chooser portal as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-028.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-029"></a>
## F-SH-029 — open-with/share portal

**Original requirement:** compatible apps/actions, user choice, remembered defaults and scoped handles

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-029.01 — Reconcile existing open-with/share portal**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for open-with/share portal. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: compatible apps/actions, user choice, remembered defaults and scoped handles
- [ ] **F-SH-029.02 — Freeze the exact contract for open-with/share portal**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: compatible apps/actions, user choice, remembered defaults and scoped handles. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-029.03 — Implement/prove: compatible apps/actions**
  - Action: For open-with/share portal, implement or reuse and verify this exact obligation: compatible apps/actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compatible apps/actions; retain observable state/resource expectations.
- [ ] **F-SH-029.04 — Implement/prove: user choice**
  - Action: For open-with/share portal, implement or reuse and verify this exact obligation: user choice. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for user choice; retain observable state/resource expectations.
- [ ] **F-SH-029.05 — Implement/prove: remembered defaults and scoped handles**
  - Action: For open-with/share portal, implement or reuse and verify this exact obligation: remembered defaults and scoped handles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for remembered defaults and scoped handles; retain observable state/resource expectations.
- [ ] **F-SH-029.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to open-with/share portal: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-029.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-029.07 — Integrate into the real consumer and runtime route**
  - Action: Wire open-with/share portal into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-029.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-029.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for open-with/share portal as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-029.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-030"></a>
## F-SH-030 — screen-capture portal

**Original requirement:** target preview, consent, protected content, indicator and result handle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-030.01 — Reconcile existing screen-capture portal**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screen-capture portal. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: target preview, consent, protected content, indicator and result handle
- [ ] **F-SH-030.02 — Freeze the exact contract for screen-capture portal**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: target preview, consent, protected content, indicator and result handle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-030.03 — Implement/prove: target preview**
  - Action: For screen-capture portal, implement or reuse and verify this exact obligation: target preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for target preview; retain observable state/resource expectations.
- [ ] **F-SH-030.04 — Implement/prove: consent**
  - Action: For screen-capture portal, implement or reuse and verify this exact obligation: consent. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consent; retain observable state/resource expectations.
- [ ] **F-SH-030.05 — Implement/prove: protected content**
  - Action: For screen-capture portal, implement or reuse and verify this exact obligation: protected content. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for protected content; retain observable state/resource expectations.
- [ ] **F-SH-030.06 — Implement/prove: indicator and result handle**
  - Action: For screen-capture portal, implement or reuse and verify this exact obligation: indicator and result handle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for indicator and result handle; retain observable state/resource expectations.
- [ ] **F-SH-030.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screen-capture portal: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-030.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-030.08 — Integrate into the real consumer and runtime route**
  - Action: Wire screen-capture portal into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-030.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-030.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screen-capture portal as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-030.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-031"></a>
## F-SH-031 — permission prompt

**Original requirement:** exact action/resource/duration, understandable risk, deny/default and settings link

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-031.01 — Reconcile existing permission prompt**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for permission prompt. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: exact action/resource/duration, understandable risk, deny/default and settings link
- [ ] **F-SH-031.02 — Freeze the exact contract for permission prompt**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: exact action/resource/duration, understandable risk, deny/default and settings link. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-031.03 — Implement/prove: exact action/resource/duration**
  - Action: For permission prompt, implement or reuse and verify this exact obligation: exact action/resource/duration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact action/resource/duration; retain observable state/resource expectations.
- [ ] **F-SH-031.04 — Implement/prove: understandable risk**
  - Action: For permission prompt, implement or reuse and verify this exact obligation: understandable risk. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for understandable risk; retain observable state/resource expectations.
- [ ] **F-SH-031.05 — Implement/prove: deny/default and settings link**
  - Action: For permission prompt, implement or reuse and verify this exact obligation: deny/default and settings link. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deny/default and settings link; retain observable state/resource expectations.
- [ ] **F-SH-031.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to permission prompt: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-031.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-031.07 — Integrate into the real consumer and runtime route**
  - Action: Wire permission prompt into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-031.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-031.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for permission prompt as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-031.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-032"></a>
## F-SH-032 — quick settings

**Original requirement:** network/audio/brightness/power/a11y controls with effective-state feedback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-032.01 — Reconcile existing quick settings**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for quick settings. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: network/audio/brightness/power/a11y controls with effective-state feedback
- [ ] **F-SH-032.02 — Freeze the exact contract for quick settings**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: network/audio/brightness/power/a11y controls with effective-state feedback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-032.03 — Implement/prove: network/audio/brightness/power/a11y controls with effective-state feedback**
  - Action: For quick settings, implement or reuse and verify this exact obligation: network/audio/brightness/power/a11y controls with effective-state feedback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for network/audio/brightness/power/a11y controls with effective-state feedback; retain observable state/resource expectations.
- [ ] **F-SH-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to quick settings: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire quick settings into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for quick settings as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-033"></a>
## F-SH-033 — system tray/status items

**Original requirement:** authenticated bounded app indicators/actions, overflow and no arbitrary draw

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-033.01 — Reconcile existing system tray/status items**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for system tray/status items. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated bounded app indicators/actions, overflow and no arbitrary draw
- [ ] **F-SH-033.02 — Freeze the exact contract for system tray/status items**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated bounded app indicators/actions, overflow and no arbitrary draw. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-033.03 — Implement/prove: authenticated bounded app indicators/actions**
  - Action: For system tray/status items, implement or reuse and verify this exact obligation: authenticated bounded app indicators/actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated bounded app indicators/actions; retain observable state/resource expectations.
- [ ] **F-SH-033.04 — Implement/prove: overflow and no arbitrary draw**
  - Action: For system tray/status items, implement or reuse and verify this exact obligation: overflow and no arbitrary draw. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow and no arbitrary draw; retain observable state/resource expectations.
- [ ] **F-SH-033.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to system tray/status items: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-033.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-033.06 — Integrate into the real consumer and runtime route**
  - Action: Wire system tray/status items into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-033.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-033.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for system tray/status items as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-033.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-034"></a>
## F-SH-034 — clock/calendar panel

**Original requirement:** locale/timezone, events, timers and notification integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-034.01 — Reconcile existing clock/calendar panel**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for clock/calendar panel. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: locale/timezone, events, timers and notification integration
- [ ] **F-SH-034.02 — Freeze the exact contract for clock/calendar panel**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: locale/timezone, events, timers and notification integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-034.03 — Implement/prove: locale/timezone**
  - Action: For clock/calendar panel, implement or reuse and verify this exact obligation: locale/timezone. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale/timezone; retain observable state/resource expectations.
- [ ] **F-SH-034.04 — Implement/prove: events**
  - Action: For clock/calendar panel, implement or reuse and verify this exact obligation: events. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for events; retain observable state/resource expectations.
- [ ] **F-SH-034.05 — Implement/prove: timers and notification integration**
  - Action: For clock/calendar panel, implement or reuse and verify this exact obligation: timers and notification integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-034.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timers and notification integration; retain observable state/resource expectations.
- [ ] **F-SH-034.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to clock/calendar panel: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-034.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-034.07 — Integrate into the real consumer and runtime route**
  - Action: Wire clock/calendar panel into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-034.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-034.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for clock/calendar panel as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-034.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-035"></a>
## F-SH-035 — power/session menu

**Original requirement:** lock/logout/suspend/restart/shutdown with permissions and unsaved-work state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-035.01 — Reconcile existing power/session menu**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for power/session menu. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: lock/logout/suspend/restart/shutdown with permissions and unsaved-work state
- [ ] **F-SH-035.02 — Freeze the exact contract for power/session menu**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: lock/logout/suspend/restart/shutdown with permissions and unsaved-work state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-035.03 — Implement/prove: lock/logout/suspend/restart/shutdown with permissions and unsaved-work state**
  - Action: For power/session menu, implement or reuse and verify this exact obligation: lock/logout/suspend/restart/shutdown with permissions and unsaved-work state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lock/logout/suspend/restart/shutdown with permissions and unsaved-work state; retain observable state/resource expectations.
- [ ] **F-SH-035.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to power/session menu: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-035.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-035.05 — Integrate into the real consumer and runtime route**
  - Action: Wire power/session menu into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-035.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-035.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for power/session menu as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-035.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-036"></a>
## F-SH-036 — onboarding

**Original requirement:** theme/network/account/accessibility/privacy tour with skip/resume and truth

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-036.01 — Reconcile existing onboarding**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for onboarding. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: theme/network/account/accessibility/privacy tour with skip/resume and truth
- [ ] **F-SH-036.02 — Freeze the exact contract for onboarding**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: theme/network/account/accessibility/privacy tour with skip/resume and truth. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-036.03 — Implement/prove: theme/network/account/accessibility/privacy tour with skip/resume and truth**
  - Action: For onboarding, implement or reuse and verify this exact obligation: theme/network/account/accessibility/privacy tour with skip/resume and truth. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for theme/network/account/accessibility/privacy tour with skip/resume and truth; retain observable state/resource expectations.
- [ ] **F-SH-036.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to onboarding: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-036.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-036.05 — Integrate into the real consumer and runtime route**
  - Action: Wire onboarding into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-036.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-036.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for onboarding as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-036.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-037"></a>
## F-SH-037 — help system

**Original requirement:** searchable contextual offline docs, shortcuts and version/provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-037.01 — Reconcile existing help system**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for help system. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: searchable contextual offline docs, shortcuts and version/provenance
- [ ] **F-SH-037.02 — Freeze the exact contract for help system**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: searchable contextual offline docs, shortcuts and version/provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-037.03 — Implement/prove: searchable contextual offline docs**
  - Action: For help system, implement or reuse and verify this exact obligation: searchable contextual offline docs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for searchable contextual offline docs; retain observable state/resource expectations.
- [ ] **F-SH-037.04 — Implement/prove: shortcuts and version/provenance**
  - Action: For help system, implement or reuse and verify this exact obligation: shortcuts and version/provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shortcuts and version/provenance; retain observable state/resource expectations.
- [ ] **F-SH-037.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to help system: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-037.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-037.06 — Integrate into the real consumer and runtime route**
  - Action: Wire help system into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-037.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-037.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for help system as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-037.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-038"></a>
## F-SH-038 — crash UI

**Original requirement:** app/service identity, recovery/relaunch/report details and no crash loop

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-038.01 — Reconcile existing crash UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for crash UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: app/service identity, recovery/relaunch/report details and no crash loop
- [ ] **F-SH-038.02 — Freeze the exact contract for crash UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: app/service identity, recovery/relaunch/report details and no crash loop. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-038.03 — Implement/prove: app/service identity**
  - Action: For crash UI, implement or reuse and verify this exact obligation: app/service identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/service identity; retain observable state/resource expectations.
- [ ] **F-SH-038.04 — Implement/prove: recovery/relaunch/report details and no crash loop**
  - Action: For crash UI, implement or reuse and verify this exact obligation: recovery/relaunch/report details and no crash loop. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for recovery/relaunch/report details and no crash loop; retain observable state/resource expectations.
- [ ] **F-SH-038.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to crash UI: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-038.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-038.06 — Integrate into the real consumer and runtime route**
  - Action: Wire crash UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-038.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-038.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for crash UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-038.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-039"></a>
## F-SH-039 — busy/unresponsive UI

**Original requirement:** progress, cancel, wait, terminate and preserved desktop input

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-039.01 — Reconcile existing busy/unresponsive UI**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for busy/unresponsive UI. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: progress, cancel, wait, terminate and preserved desktop input
- [ ] **F-SH-039.02 — Freeze the exact contract for busy/unresponsive UI**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: progress, cancel, wait, terminate and preserved desktop input. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-039.03 — Implement/prove: progress**
  - Action: For busy/unresponsive UI, implement or reuse and verify this exact obligation: progress. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for progress; retain observable state/resource expectations.
- [ ] **F-SH-039.04 — Implement/prove: cancel**
  - Action: For busy/unresponsive UI, implement or reuse and verify this exact obligation: cancel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancel; retain observable state/resource expectations.
- [ ] **F-SH-039.05 — Implement/prove: wait**
  - Action: For busy/unresponsive UI, implement or reuse and verify this exact obligation: wait. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wait; retain observable state/resource expectations.
- [ ] **F-SH-039.06 — Implement/prove: terminate and preserved desktop input**
  - Action: For busy/unresponsive UI, implement or reuse and verify this exact obligation: terminate and preserved desktop input. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-039.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for terminate and preserved desktop input; retain observable state/resource expectations.
- [ ] **F-SH-039.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to busy/unresponsive UI: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-039.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-039.08 — Integrate into the real consumer and runtime route**
  - Action: Wire busy/unresponsive UI into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-039.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-039.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for busy/unresponsive UI as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-039.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-040"></a>
## F-SH-040 — session restore

**Original requirement:** allowed apps/documents/windows restored with crash/privacy/user control

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-040.01 — Reconcile existing session restore**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for session restore. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: allowed apps/documents/windows restored with crash/privacy/user control
- [ ] **F-SH-040.02 — Freeze the exact contract for session restore**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: allowed apps/documents/windows restored with crash/privacy/user control. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-040.03 — Implement/prove: allowed apps/documents/windows restored with crash/privacy/user control**
  - Action: For session restore, implement or reuse and verify this exact obligation: allowed apps/documents/windows restored with crash/privacy/user control. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for allowed apps/documents/windows restored with crash/privacy/user control; retain observable state/resource expectations.
- [ ] **F-SH-040.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to session restore: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-040.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-040.05 — Integrate into the real consumer and runtime route**
  - Action: Wire session restore into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-040.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-040.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for session restore as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-040.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-041"></a>
## F-SH-041 — multi-user switching

**Original requirement:** isolated sessions, device/display ownership, lock and resource policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-041.01 — Reconcile existing multi-user switching**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for multi-user switching. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated sessions, device/display ownership, lock and resource policy
- [ ] **F-SH-041.02 — Freeze the exact contract for multi-user switching**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated sessions, device/display ownership, lock and resource policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-041.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-041.03 — Implement/prove: isolated sessions**
  - Action: For multi-user switching, implement or reuse and verify this exact obligation: isolated sessions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-041.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated sessions; retain observable state/resource expectations.
- [ ] **F-SH-041.04 — Implement/prove: device/display ownership**
  - Action: For multi-user switching, implement or reuse and verify this exact obligation: device/display ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-041.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device/display ownership; retain observable state/resource expectations.
- [ ] **F-SH-041.05 — Implement/prove: lock and resource policy**
  - Action: For multi-user switching, implement or reuse and verify this exact obligation: lock and resource policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-041.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for lock and resource policy; retain observable state/resource expectations.
- [ ] **F-SH-041.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to multi-user switching: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-041.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-041.07 — Integrate into the real consumer and runtime route**
  - Action: Wire multi-user switching into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-041.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-041.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for multi-user switching as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-041.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-042"></a>
## F-SH-042 — guest session

**Original requirement:** ephemeral restricted state with verified teardown

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-042.01 — Reconcile existing guest session**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for guest session. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: ephemeral restricted state with verified teardown
- [ ] **F-SH-042.02 — Freeze the exact contract for guest session**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: ephemeral restricted state with verified teardown. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-042.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-042.03 — Implement/prove: ephemeral restricted state with verified teardown**
  - Action: For guest session, implement or reuse and verify this exact obligation: ephemeral restricted state with verified teardown. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-042.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for ephemeral restricted state with verified teardown; retain observable state/resource expectations.
- [ ] **F-SH-042.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to guest session: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-042.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-042.05 — Integrate into the real consumer and runtime route**
  - Action: Wire guest session into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-042.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-042.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for guest session as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-042.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-043"></a>
## F-SH-043 — kiosk/public-demo session

**Original requirement:** fixed app/lease/limits/input/egress/reset and escape/admin policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-043.01 — Reconcile existing kiosk/public-demo session**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for kiosk/public-demo session. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: fixed app/lease/limits/input/egress/reset and escape/admin policy
- [ ] **F-SH-043.02 — Freeze the exact contract for kiosk/public-demo session**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: fixed app/lease/limits/input/egress/reset and escape/admin policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-043.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-043.03 — Implement/prove: fixed app/lease/limits/input/egress/reset and escape/admin policy**
  - Action: For kiosk/public-demo session, implement or reuse and verify this exact obligation: fixed app/lease/limits/input/egress/reset and escape/admin policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-043.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fixed app/lease/limits/input/egress/reset and escape/admin policy; retain observable state/resource expectations.
- [ ] **F-SH-043.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to kiosk/public-demo session: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-043.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-043.05 — Integrate into the real consumer and runtime route**
  - Action: Wire kiosk/public-demo session into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-043.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-043.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for kiosk/public-demo session as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-043.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-044"></a>
## F-SH-044 — remote session

**Original requirement:** authenticated lifecycle, resize/input/clipboard, reconnect and local privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-044.01 — Reconcile existing remote session**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for remote session. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: authenticated lifecycle, resize/input/clipboard, reconnect and local privacy
- [ ] **F-SH-044.02 — Freeze the exact contract for remote session**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: authenticated lifecycle, resize/input/clipboard, reconnect and local privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-044.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-044.03 — Implement/prove: authenticated lifecycle**
  - Action: For remote session, implement or reuse and verify this exact obligation: authenticated lifecycle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-044.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for authenticated lifecycle; retain observable state/resource expectations.
- [ ] **F-SH-044.04 — Implement/prove: resize/input/clipboard**
  - Action: For remote session, implement or reuse and verify this exact obligation: resize/input/clipboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-044.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize/input/clipboard; retain observable state/resource expectations.
- [ ] **F-SH-044.05 — Implement/prove: reconnect and local privacy**
  - Action: For remote session, implement or reuse and verify this exact obligation: reconnect and local privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-044.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reconnect and local privacy; retain observable state/resource expectations.
- [ ] **F-SH-044.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to remote session: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-044.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-044.07 — Integrate into the real consumer and runtime route**
  - Action: Wire remote session into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-044.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-044.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for remote session as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-044.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-sh-045"></a>
## F-SH-045 — shell performance telemetry

**Original requirement:** launch/frame/input/service failure data surfaced without fake values

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-SH-045.01 — Reconcile existing shell performance telemetry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shell performance telemetry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-and-evidence comparison against the complete requirement: launch/frame/input/service failure data surfaced without fake values
- [ ] **F-SH-045.02 — Freeze the exact contract for shell performance telemetry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: launch/frame/input/service failure data surfaced without fake values. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-SH-045.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-SH-045.03 — Implement/prove: launch/frame/input/service failure data surfaced without fake values**
  - Action: For shell performance telemetry, implement or reuse and verify this exact obligation: launch/frame/input/service failure data surfaced without fake values. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-SH-045.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for launch/frame/input/service failure data surfaced without fake values; retain observable state/resource expectations.
- [ ] **F-SH-045.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shell performance telemetry: cross-session focus/input; lock spoof or bypass; stale app/window identity; crash during restore; denied launch; dependency restart; clipboard/capture grant revocation. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-SH-045.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-SH-045.05 — Integrate into the real consumer and runtime route**
  - Action: Wire shell performance telemetry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-SH-045.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-SH-045.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shell performance telemetry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-SH-045.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p5-3"></a>
## C-P5.3 — Window and Input services

**Original requirement:** Window and Input services

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 291.

### Preserved original contract

- **Dependencies/current/provenance:** Phase 1, P4, P3 app processes; Serenity/Lemon/duckOS/protOS user WM; reject client scanout access and unchecked geometry.
- **I/O and state:** surface/window/input handles, validated geometry/damage and events in/out; window `Created -> Mapped -> Visible/Hidden -> Closing -> Destroyed`; surface generations retire by fence.
- **Invariants/failure:** compositor exclusively owns scanout; client owns only granted surface; geometry/stride/format overflow checked; focus/window ownership enforced; bounded events.
- **Deterministic proof:** malicious dimensions/damage, stale surface, sender ownership, focus/workspace/modal matrix, client crash, compositor restart policy.
- **Target proof:** QEMU process apps render/input; ThinkPad trace preserves Phase 1 latency/visual contracts.
- **Receipt/removal:** scene hashes, event ownership and timing; old hook bridge per app; remove app hook only after workflow parity.

### Execution steps

- [ ] **C-P5.3.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P5.3.02 — Resolve this contract's exact dependencies**
  - Action: Phase 1, P4, P3 app processes; Serenity/Lemon/duckOS/protOS user WM; reject client scanout access and unchecked geometry. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P5.3.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P5.3.03 — I/O and state — Window and Input services**
  - Action: surface/window/input handles, validated geometry/damage and events in/out; window `Created -> Mapped -> Visible/Hidden -> Closing -> Destroyed`; surface generations retire by fence.
  - Requires: C-P5.3.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P5.3.
- [ ] **C-P5.3.04 — Invariants/failure — Window and Input services**
  - Action: compositor exclusively owns scanout; client owns only granted surface; geometry/stride/format overflow checked; focus/window ownership enforced; bounded events.
  - Requires: C-P5.3.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P5.3.
- [ ] **C-P5.3.05 — Deterministic proof — Window and Input services**
  - Action: malicious dimensions/damage, stale surface, sender ownership, focus/workspace/modal matrix, client crash, compositor restart policy.
  - Requires: C-P5.3.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P5.3.
- [ ] **C-P5.3.06 — Target proof — Window and Input services**
  - Action: QEMU process apps render/input; ThinkPad trace preserves Phase 1 latency/visual contracts.
  - Requires: C-P5.3.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P5.3.
- [ ] **C-P5.3.07 — Receipt/removal — Window and Input services**
  - Action: scene hashes, event ownership and timing; old hook bridge per app; remove app hook only after workflow parity.
  - Requires: C-P5.3.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P5.3.
- [ ] **C-P5.3.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P5.3. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P5.3.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-p8-4"></a>
## C-P8.4 — Intel render/composition provider

**Original requirement:** Intel render/composition provider

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 456.

### Preserved original contract

- **Dependencies/current/provenance:** Phase 1 stable scene/lifetimes, P8.1, current Intel modeset/ring and software renderer; RustOS failed GPU receipts and mainstream atomic composition; reject mandatory GPU path.
- **I/O and state:** validated scene commands/surfaces/fences in; completion or reset/fallback out; `Software -> GPUStarting -> GPUReady -> Submitted -> Completed|Hung -> Recovering -> Software`.
- **Invariants/failure:** GOP/software always available; bounded command/ring memory; exact fence ownership; hang triggers recovery; development scene agreement required.
- **Deterministic proof:** command encoder/model, fence wrap, forced hang/reset, stale surface, scene hash corpus, fallback transition.
- **Target proof:** ThinkPad second modeset/teardown and composition timing; QEMU uses software/compatible model only and cannot stand in for Gen9.
- **Receipt/removal:** software/GPU hashes, fences/reset and physical timing; one boot switch disables GPU; software/GOP never removed.

### Execution steps

- [ ] **C-P8.4.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P8.4.02 — Resolve this contract's exact dependencies**
  - Action: Phase 1 stable scene/lifetimes, P8.1, current Intel modeset/ring and software renderer; RustOS failed GPU receipts and mainstream atomic composition; reject mandatory GPU path. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P8.4.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P8.4.03 — I/O and state — Intel render/composition provider**
  - Action: validated scene commands/surfaces/fences in; completion or reset/fallback out; `Software -> GPUStarting -> GPUReady -> Submitted -> Completed|Hung -> Recovering -> Software`.
  - Requires: C-P8.4.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P8.4.
- [ ] **C-P8.4.04 — Invariants/failure — Intel render/composition provider**
  - Action: GOP/software always available; bounded command/ring memory; exact fence ownership; hang triggers recovery; development scene agreement required.
  - Requires: C-P8.4.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P8.4.
- [ ] **C-P8.4.05 — Deterministic proof — Intel render/composition provider**
  - Action: command encoder/model, fence wrap, forced hang/reset, stale surface, scene hash corpus, fallback transition.
  - Requires: C-P8.4.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P8.4.
- [ ] **C-P8.4.06 — Target proof — Intel render/composition provider**
  - Action: ThinkPad second modeset/teardown and composition timing; QEMU uses software/compatible model only and cannot stand in for Gen9.
  - Requires: C-P8.4.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P8.4.
- [ ] **C-P8.4.07 — Receipt/removal — Intel render/composition provider**
  - Action: software/GPU hashes, fences/reset and physical timing; one boot switch disables GPU; software/GOP never removed.
  - Requires: C-P8.4.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P8.4.
- [ ] **C-P8.4.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P8.4. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P8.4.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-09"></a>
## C-DA-09 — display provider

**Original requirement:** display provider

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 272.

### Preserved original contract

**Depends on:** DA-04 through DA-08; preserve GOP/software.

**Deliver:** connector/mode/plane/cursor discovery, bounded surfaces, validated
atomic scene updates, present fences, vblank/hotplug, modeset, diagnostic capture,
reset and fallback. Wrap BGA/virtio-gpu first; bring the Intel investigation into
native boot without treating its host-harness modeset as a provider receipt.

**Invariants:** compositor alone owns scanout; stride/format/range and scene geometry
are checked; invalid work never reaches hardware; fence waits expire; failure falls
back without losing the session.

**Proof:** invalid mode/stride/plane, noncanonical address, ring full, fence timeout,
underrun, second modeset, repeated reset, software/accelerated scene equivalence,
QEMU BGA/virtio, separate Intel host-harness evidence, then a named native-boot
Intel panel/artifact/display-effect receipt.

### Execution steps

- [ ] **C-DA-09.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-09.02 — Resolve this contract's exact dependencies**
  - Action: DA-04 through DA-08; preserve GOP/software. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-09.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-09.03 — Deliver — display provider**
  - Action: connector/mode/plane/cursor discovery, bounded surfaces, validated atomic scene updates, present fences, vblank/hotplug, modeset, diagnostic capture, reset and fallback. Wrap BGA/virtio-gpu first; bring the Intel investigation into native boot without treating its host-harness modeset as a provider receipt.
  - Requires: C-DA-09.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-09.
- [ ] **C-DA-09.04 — Invariants — display provider**
  - Action: compositor alone owns scanout; stride/format/range and scene geometry are checked; invalid work never reaches hardware; fence waits expire; failure falls back without losing the session.
  - Requires: C-DA-09.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-09.
- [ ] **C-DA-09.05 — Proof — display provider**
  - Action: invalid mode/stride/plane, noncanonical address, ring full, fence timeout, underrun, second modeset, repeated reset, software/accelerated scene equivalence, QEMU BGA/virtio, separate Intel host-harness evidence, then a named native-boot Intel panel/artifact/display-effect receipt.
  - Requires: C-DA-09.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-09.
- [ ] **C-DA-09.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-09. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-09.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-10"></a>
## C-DA-10 — normalized input provider

**Original requirement:** normalized input provider

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 290.

### Preserved original contract

**Depends on:** DA-04 through DA-08.

**Deliver:** device/generation/sequence/time plus normalized key, pointer, wheel,
touch and loss events. Move layout, composition, acceleration, gestures, focus and
accessibility to session policy.

**Invariants:** descriptor parsing is length-bounded; disconnect synthesizes safe
release; queue capacity/coalescing/drop is explicit; reconnect cannot revive stale
state; input grab requires the session/compositor right.

**Proof:** malformed HID descriptors, short reports, multiple keyboards/pointers,
hub churn, stuck key/button, sequence wrap, queue exhaustion, disconnect during
drag, focus theft and ordinary-process input-grab denial.

### Execution steps

- [ ] **C-DA-10.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-10.02 — Resolve this contract's exact dependencies**
  - Action: DA-04 through DA-08. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-10.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-10.03 — Deliver — normalized input provider**
  - Action: device/generation/sequence/time plus normalized key, pointer, wheel, touch and loss events. Move layout, composition, acceleration, gestures, focus and accessibility to session policy.
  - Requires: C-DA-10.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-10.
- [ ] **C-DA-10.04 — Invariants — normalized input provider**
  - Action: descriptor parsing is length-bounded; disconnect synthesizes safe release; queue capacity/coalescing/drop is explicit; reconnect cannot revive stale state; input grab requires the session/compositor right.
  - Requires: C-DA-10.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-10.
- [ ] **C-DA-10.05 — Proof — normalized input provider**
  - Action: malformed HID descriptors, short reports, multiple keyboards/pointers, hub churn, stuck key/button, sequence wrap, queue exhaustion, disconnect during drag, focus theft and ordinary-process input-grab denial.
  - Requires: C-DA-10.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-10.
- [ ] **C-DA-10.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-10. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-10.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-20"></a>
## C-DA-20 — session, compositor and surface authority

**Original requirement:** session, compositor and surface authority

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 496.

### Preserved original contract

**Depends on:** DA-09, DA-10, DA-18 and DA-19.

**Deliver:** login/session capability root, user-space window service, compositor-
exclusive scanout/input routing, opaque bounded surfaces, clipboard, notifications,
focus, grabs, drag-and-drop, per-seat state and accessibility semantics. Lock/logout
tears down or suspends session authority explicitly; restoration is versioned and
crash-safe.

**Invariants:** client geometry/stride/format/name are checked; only compositor can
present or globally grab input; clipboard reads require current policy/gesture;
revoked or closed surface cannot refresh; one client cannot starve the desktop;
focus, grab and drag objects are generation-bound; compositor restart reconstructs
only admitted live windows and cannot preserve stale authority.

**Proof:** oversized/negative geometry, stale surface, fake window ID, focus theft,
clipboard snoop, input grab denial, event flood, client crash during present and
compositor restart with session recovery; cross-session drag/drop, seat disconnect,
lock/logout with background work and stale focus/grab/drag after restart.

### Execution steps

- [ ] **C-DA-20.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-20.02 — Resolve this contract's exact dependencies**
  - Action: DA-09, DA-10, DA-18 and DA-19. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-20.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-20.03 — Deliver — session, compositor and surface authority**
  - Action: login/session capability root, user-space window service, compositor- exclusive scanout/input routing, opaque bounded surfaces, clipboard, notifications, focus, grabs, drag-and-drop, per-seat state and accessibility semantics. Lock/logout tears down or suspends session authority explicitly; restoration is versioned and crash-safe.
  - Requires: C-DA-20.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-20.
- [ ] **C-DA-20.04 — Invariants — session, compositor and surface authority**
  - Action: client geometry/stride/format/name are checked; only compositor can present or globally grab input; clipboard reads require current policy/gesture; revoked or closed surface cannot refresh; one client cannot starve the desktop; focus, grab and drag objects are generation-bound; compositor restart reconstructs only admitted live windows and cannot preserve stale authority.
  - Requires: C-DA-20.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for DA-20.
- [ ] **C-DA-20.05 — Proof — session, compositor and surface authority**
  - Action: oversized/negative geometry, stale surface, fake window ID, focus theft, clipboard snoop, input grab denial, event flood, client crash during present and compositor restart with session recovery; cross-session drag/drop, seat disconnect, lock/logout with background work and stale focus/grab/drag after restart.
  - Requires: C-DA-20.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-20.
- [ ] **C-DA-20.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-20. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-20.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-11"></a>
## C-VX-11 — Window/surface protocol

**Original requirement:** Window/surface protocol

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 261.

### Preserved original contract

**Depends on:** process/IPC handles, VX-02, VX-10.

**Deliver:** generation-tagged window/surface handles; map/draw/present/resize/
embed/capture/share/destroy rights; bounded buffers, damage and frame callbacks.

**Invariants:** compositor-exclusive scanout; checked width*height*stride;
destination capacity reserved before transfer; resize commits atomically;
revocation precedes memory reuse.

**Proof:** overflow/noncanonical formats, quota exhaustion, nth-buffer failure,
stale handle, owner death, capture denial, resize rollback and forged present.

### Execution steps

- [ ] **C-VX-11.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-11.02 — Resolve this contract's exact dependencies**
  - Action: process/IPC handles, VX-02, VX-10. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-11.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-11.03 — Deliver — Window/surface protocol**
  - Action: generation-tagged window/surface handles; map/draw/present/resize/ embed/capture/share/destroy rights; bounded buffers, damage and frame callbacks.
  - Requires: C-VX-11.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-11.
- [ ] **C-VX-11.04 — Invariants — Window/surface protocol**
  - Action: compositor-exclusive scanout; checked width*height*stride; destination capacity reserved before transfer; resize commits atomically; revocation precedes memory reuse.
  - Requires: C-VX-11.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-11.
- [ ] **C-VX-11.05 — Proof — Window/surface protocol**
  - Action: overflow/noncanonical formats, quota exhaustion, nth-buffer failure, stale handle, owner death, capture denial, resize rollback and forged present.
  - Requires: C-VX-11.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-11.
- [ ] **C-VX-11.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-11. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-11.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-12"></a>
## C-VX-12 — Compositor scene and recovery

**Original requirement:** Compositor scene and recovery

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 275.

### Preserved original contract

**Depends on:** VX-05, VX-06, VX-11.

**Deliver:** scene ownership, z-order, clipping, damage, occlusion, software
oracle, backend interface, cursor, scanout, fence/deadline and reset fallback.

**Invariants:** client cannot draw outside surface; protected surfaces cannot be
captured; late frame/fence cannot target reused surface; backend failure returns
to software/GOP without losing session control.

**Proof:** randomized scene differential, overlapping/transparent/rotated cases,
backend hang/reset, stale fence, cursor edge, capture policy, sustained frame
budgets at supported resolutions.

### Execution steps

- [ ] **C-VX-12.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-12.02 — Resolve this contract's exact dependencies**
  - Action: VX-05, VX-06, VX-11. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-12.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-12.03 — Deliver — Compositor scene and recovery**
  - Action: scene ownership, z-order, clipping, damage, occlusion, software oracle, backend interface, cursor, scanout, fence/deadline and reset fallback.
  - Requires: C-VX-12.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-12.
- [ ] **C-VX-12.04 — Invariants — Compositor scene and recovery**
  - Action: client cannot draw outside surface; protected surfaces cannot be captured; late frame/fence cannot target reused surface; backend failure returns to software/GOP without losing session control.
  - Requires: C-VX-12.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-12.
- [ ] **C-VX-12.05 — Proof — Compositor scene and recovery**
  - Action: randomized scene differential, overlapping/transparent/rotated cases, backend hang/reset, stale fence, cursor edge, capture policy, sustained frame budgets at supported resolutions.
  - Requires: C-VX-12.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-12.
- [ ] **C-VX-12.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-12. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-12.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-13"></a>
## C-VX-13 — Session shell and lifecycle

**Original requirement:** Session shell and lifecycle

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 290.

### Preserved original contract

**Depends on:** VX-03, VX-07..12, session/auth service.

**Deliver:** login/lock/unlock/logout, top island, dock, Activities/overview,
catalogue, Run/command palette, workspaces, status, quick settings and power UI.

**Invariants:** lock hides/revokes prior content; readiness waits for dependencies;
degraded providers are visible; logout reverses startup and proves process death;
global actions require exact authority.

**Proof:** dependency timeout/crash, lock during capture/drag/modal, failed login,
session restart, orphan-window check, ordinary-app shutdown denial and full
keyboard route.

### Execution steps

- [ ] **C-VX-13.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-13.02 — Resolve this contract's exact dependencies**
  - Action: VX-03, VX-07..12, session/auth service. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-13.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-13.03 — Deliver — Session shell and lifecycle**
  - Action: login/lock/unlock/logout, top island, dock, Activities/overview, catalogue, Run/command palette, workspaces, status, quick settings and power UI.
  - Requires: C-VX-13.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-13.
- [ ] **C-VX-13.04 — Invariants — Session shell and lifecycle**
  - Action: lock hides/revokes prior content; readiness waits for dependencies; degraded providers are visible; logout reverses startup and proves process death; global actions require exact authority.
  - Requires: C-VX-13.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-13.
- [ ] **C-VX-13.05 — Proof — Session shell and lifecycle**
  - Action: dependency timeout/crash, lock during capture/drag/modal, failed login, session restart, orphan-window check, ordinary-app shutdown denial and full keyboard route.
  - Requires: C-VX-13.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-13.
- [ ] **C-VX-13.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-13. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-13.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-14"></a>
## C-VX-14 — Window interaction and multi-monitor

**Original requirement:** Window interaction and multi-monitor

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 305.

### Preserved original contract

**Depends on:** VX-12, VX-13.

**Deliver:** move/resize/min/max/snap, tabs, Alt-Tab, workspaces, overview,
multi-monitor placement, per-monitor scale, hotplug and restore.

**Invariants:** visible recovery point remains; snap/restore geometry survives
scale changes; window never becomes irretrievable; modal/focus ownership stays
within workspace/session policy.

**Proof:** geometry boundaries, rapid hotplug, monitor removal mid-drag, scale
transition, workspace switching with modal, app death and persisted restore.

### Execution steps

- [ ] **C-VX-14.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-14.02 — Resolve this contract's exact dependencies**
  - Action: VX-12, VX-13. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-14.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-14.03 — Deliver — Window interaction and multi-monitor**
  - Action: move/resize/min/max/snap, tabs, Alt-Tab, workspaces, overview, multi-monitor placement, per-monitor scale, hotplug and restore.
  - Requires: C-VX-14.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-14.
- [ ] **C-VX-14.04 — Invariants — Window interaction and multi-monitor**
  - Action: visible recovery point remains; snap/restore geometry survives scale changes; window never becomes irretrievable; modal/focus ownership stays within workspace/session policy.
  - Requires: C-VX-14.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-14.
- [ ] **C-VX-14.05 — Proof — Window interaction and multi-monitor**
  - Action: geometry boundaries, rapid hotplug, monitor removal mid-drag, scale transition, workspace switching with modal, app death and persisted restore.
  - Requires: C-VX-14.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-14.
- [ ] **C-VX-14.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-14. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-14.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-usb-005"></a>
## T-USB-005 — USB HID class

**Original requirement:** descriptors/reports/usages/multiple interfaces

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 140.

### Execution steps

- [ ] **T-USB-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB HID class to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-USB-005.02 — Specify the complete target boundary**
  - Action: USB HID class must supply: descriptors/reports/usages/multiple interfaces. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-USB-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-USB-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB HID class through the shared platform contract, delivering every part of: descriptors/reports/usages/multiple interfaces. Do not fork a duplicate subsystem for this row.
  - Requires: T-USB-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-USB-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: failure at every start/recover/stop stage; partial DMA/IRQ/BAR reservation; late completion after detach; wrong device generation; reset timeout; unsupported firmware; missing IOMMU isolation.
  - Requires: T-USB-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB HID class.
- [ ] **T-USB-005.05 — Qualify and retain this target's own result**
  - Action: Bind USB HID class to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-USB-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-001"></a>
## T-INPUT-001 — PS/2 controller

**Original requirement:** dual-channel detection, reset and errors

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 155.

### Execution steps

- [ ] **T-INPUT-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PS/2 controller to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-001.02 — Specify the complete target boundary**
  - Action: PS/2 controller must supply: dual-channel detection, reset and errors. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse PS/2 controller through the shared platform contract, delivering every part of: dual-channel detection, reset and errors. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PS/2 controller.
- [ ] **T-INPUT-001.05 — Qualify and retain this target's own result**
  - Action: Bind PS/2 controller to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-002"></a>
## T-INPUT-002 — PS/2 keyboard

**Original requirement:** scan sets, LEDs, layouts and repeats

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 156.

### Execution steps

- [ ] **T-INPUT-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PS/2 keyboard to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-002.02 — Specify the complete target boundary**
  - Action: PS/2 keyboard must supply: scan sets, LEDs, layouts and repeats. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse PS/2 keyboard through the shared platform contract, delivering every part of: scan sets, LEDs, layouts and repeats. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PS/2 keyboard.
- [ ] **T-INPUT-002.05 — Qualify and retain this target's own result**
  - Action: Bind PS/2 keyboard to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-003"></a>
## T-INPUT-003 — PS/2 mouse

**Original requirement:** packets, wheel/buttons, resync and acceleration

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 157.

### Execution steps

- [ ] **T-INPUT-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve PS/2 mouse to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-003.02 — Specify the complete target boundary**
  - Action: PS/2 mouse must supply: packets, wheel/buttons, resync and acceleration. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse PS/2 mouse through the shared platform contract, delivering every part of: packets, wheel/buttons, resync and acceleration. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-003.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for PS/2 mouse.
- [ ] **T-INPUT-003.05 — Qualify and retain this target's own result**
  - Action: Bind PS/2 mouse to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-004"></a>
## T-INPUT-004 — USB HID keyboard

**Original requirement:** boot/report modes and rollover

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 158.

### Execution steps

- [ ] **T-INPUT-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB HID keyboard to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-004.02 — Specify the complete target boundary**
  - Action: USB HID keyboard must supply: boot/report modes and rollover. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB HID keyboard through the shared platform contract, delivering every part of: boot/report modes and rollover. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB HID keyboard.
- [ ] **T-INPUT-004.05 — Qualify and retain this target's own result**
  - Action: Bind USB HID keyboard to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-005"></a>
## T-INPUT-005 — USB HID mouse

**Original requirement:** buttons/wheel/high-resolution motion

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 159.

### Execution steps

- [ ] **T-INPUT-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB HID mouse to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-005.02 — Specify the complete target boundary**
  - Action: USB HID mouse must supply: buttons/wheel/high-resolution motion. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB HID mouse through the shared platform contract, delivering every part of: buttons/wheel/high-resolution motion. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB HID mouse.
- [ ] **T-INPUT-005.05 — Qualify and retain this target's own result**
  - Action: Bind USB HID mouse to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-006"></a>
## T-INPUT-006 — generic HID report parser

**Original requirement:** bounded descriptors, usages and collections

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 160.

### Execution steps

- [ ] **T-INPUT-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve generic HID report parser to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-006.02 — Specify the complete target boundary**
  - Action: generic HID report parser must supply: bounded descriptors, usages and collections. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse generic HID report parser through the shared platform contract, delivering every part of: bounded descriptors, usages and collections. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for generic HID report parser.
- [ ] **T-INPUT-006.05 — Qualify and retain this target's own result**
  - Action: Bind generic HID report parser to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-009"></a>
## T-INPUT-009 — I2C-HID transport and decoder

**Original requirement:** HID descriptor, full reports, no diagnostic-only claim

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 163.

### Execution steps

- [ ] **T-INPUT-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve I2C-HID transport and decoder to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-009.02 — Specify the complete target boundary**
  - Action: I2C-HID transport and decoder must supply: HID descriptor, full reports, no diagnostic-only claim. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse I2C-HID transport and decoder through the shared platform contract, delivering every part of: HID descriptor, full reports, no diagnostic-only claim. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-009.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for I2C-HID transport and decoder.
- [ ] **T-INPUT-009.05 — Qualify and retain this target's own result**
  - Action: Bind I2C-HID transport and decoder to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-input-010"></a>
## T-INPUT-010 — virtio-input/tablet

**Original requirement:** negotiated events, absolute/relative modes

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 164.

### Execution steps

- [ ] **T-INPUT-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio-input/tablet to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-010.02 — Specify the complete target boundary**
  - Action: virtio-input/tablet must supply: negotiated events, absolute/relative modes. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio-input/tablet through the shared platform contract, delivering every part of: negotiated events, absolute/relative modes. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-010.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio-input/tablet.
- [ ] **T-INPUT-010.05 — Qualify and retain this target's own result**
  - Action: Bind virtio-input/tablet to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-001"></a>
## T-GPU-001 — software raster/compositor oracle

**Original requirement:** pixel-correct bounded fallback and differential hash

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 174.

### Execution steps

- [ ] **T-GPU-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve software raster/compositor oracle to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-001.02 — Specify the complete target boundary**
  - Action: software raster/compositor oracle must supply: pixel-correct bounded fallback and differential hash. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse software raster/compositor oracle through the shared platform contract, delivering every part of: pixel-correct bounded fallback and differential hash. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-001.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for software raster/compositor oracle.
- [ ] **T-GPU-001.05 — Qualify and retain this target's own result**
  - Action: Bind software raster/compositor oracle to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-002"></a>
## T-GPU-002 — UEFI GOP framebuffer

**Original requirement:** validated mode/pitch/format and handoff

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 175.

### Execution steps

- [ ] **T-GPU-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve UEFI GOP framebuffer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-002.02 — Specify the complete target boundary**
  - Action: UEFI GOP framebuffer must supply: validated mode/pitch/format and handoff. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse UEFI GOP framebuffer through the shared platform contract, delivering every part of: validated mode/pitch/format and handoff. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-002.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for UEFI GOP framebuffer.
- [ ] **T-GPU-002.05 — Qualify and retain this target's own result**
  - Action: Bind UEFI GOP framebuffer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-004"></a>
## T-GPU-004 — VGA text/planar fallback

**Original requirement:** panic/legacy console ownership

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 177.

### Execution steps

- [ ] **T-GPU-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve VGA text/planar fallback to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-004.02 — Specify the complete target boundary**
  - Action: VGA text/planar fallback must supply: panic/legacy console ownership. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse VGA text/planar fallback through the shared platform contract, delivering every part of: panic/legacy console ownership. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-004.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for VGA text/planar fallback.
- [ ] **T-GPU-004.05 — Qualify and retain this target's own result**
  - Action: Bind VGA text/planar fallback to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-005"></a>
## T-GPU-005 — Bochs/BGA display

**Original requirement:** mode validation, framebuffer and teardown

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 178.

### Execution steps

- [ ] **T-GPU-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Bochs/BGA display to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-005.02 — Specify the complete target boundary**
  - Action: Bochs/BGA display must supply: mode validation, framebuffer and teardown. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Bochs/BGA display through the shared platform contract, delivering every part of: mode validation, framebuffer and teardown. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-005.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Bochs/BGA display.
- [ ] **T-GPU-005.05 — Qualify and retain this target's own result**
  - Action: Bind Bochs/BGA display to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-006"></a>
## T-GPU-006 — QEMU ramfb/simplefb

**Original requirement:** immutable mode and safe scanout

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 179.

### Execution steps

- [ ] **T-GPU-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve QEMU ramfb/simplefb to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-006.02 — Specify the complete target boundary**
  - Action: QEMU ramfb/simplefb must supply: immutable mode and safe scanout. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse QEMU ramfb/simplefb through the shared platform contract, delivering every part of: immutable mode and safe scanout. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-006.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for QEMU ramfb/simplefb.
- [ ] **T-GPU-006.05 — Qualify and retain this target's own result**
  - Action: Bind QEMU ramfb/simplefb to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-007"></a>
## T-GPU-007 — virtio-gpu 2D/KMS

**Original requirement:** resources/backing/scanout/flush/fences/reset

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 180.

### Execution steps

- [ ] **T-GPU-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve virtio-gpu 2D/KMS to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-007.02 — Specify the complete target boundary**
  - Action: virtio-gpu 2D/KMS must supply: resources/backing/scanout/flush/fences/reset. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse virtio-gpu 2D/KMS through the shared platform contract, delivering every part of: resources/backing/scanout/flush/fences/reset. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-007.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for virtio-gpu 2D/KMS.
- [ ] **T-GPU-007.05 — Qualify and retain this target's own result**
  - Action: Bind virtio-gpu 2D/KMS to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-011"></a>
## T-GPU-011 — Intel Gen9/9.5 display

**Original requirement:** VBT/EDID/pipes/planes/panel/hotplug/recovery

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 184.

### Execution steps

- [ ] **T-GPU-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel Gen9/9.5 display to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-011.02 — Specify the complete target boundary**
  - Action: Intel Gen9/9.5 display must supply: VBT/EDID/pipes/planes/panel/hotplug/recovery. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel Gen9/9.5 display through the shared platform contract, delivering every part of: VBT/EDID/pipes/planes/panel/hotplug/recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-011.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel Gen9/9.5 display.
- [ ] **T-GPU-011.05 — Qualify and retain this target's own result**
  - Action: Bind Intel Gen9/9.5 display to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-012"></a>
## T-GPU-012 — Intel blitter/render rings

**Original requirement:** commands/fences/hang/reset/software agreement

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 185.

### Execution steps

- [ ] **T-GPU-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Intel blitter/render rings to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-012.02 — Specify the complete target boundary**
  - Action: Intel blitter/render rings must supply: commands/fences/hang/reset/software agreement. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse Intel blitter/render rings through the shared platform contract, delivering every part of: commands/fences/hang/reset/software agreement. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-012.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Intel blitter/render rings.
- [ ] **T-GPU-012.05 — Qualify and retain this target's own result**
  - Action: Bind Intel blitter/render rings to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-018"></a>
## T-GPU-018 — hardware cursor contract

**Original requirement:** ownership, clipping, atomic update and fallback

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 191.

### Execution steps

- [ ] **T-GPU-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve hardware cursor contract to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-018.02 — Specify the complete target boundary**
  - Action: hardware cursor contract must supply: ownership, clipping, atomic update and fallback. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse hardware cursor contract through the shared platform contract, delivering every part of: ownership, clipping, atomic update and fallback. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-018.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for hardware cursor contract.
- [ ] **T-GPU-018.05 — Qualify and retain this target's own result**
  - Action: Bind hardware cursor contract to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-gpu-019"></a>
## T-GPU-019 — display connector/EDID/DP/HDMI core

**Original requirement:** bounded EDID, hotplug, link and mode policy

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 192.

### Execution steps

- [ ] **T-GPU-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve display connector/EDID/DP/HDMI core to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GPU-019.02 — Specify the complete target boundary**
  - Action: display connector/EDID/DP/HDMI core must supply: bounded EDID, hotplug, link and mode policy. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GPU-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GPU-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse display connector/EDID/DP/HDMI core through the shared platform contract, delivering every part of: bounded EDID, hotplug, link and mode policy. Do not fork a duplicate subsystem for this row.
  - Requires: T-GPU-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GPU-019.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-GPU-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for display connector/EDID/DP/HDMI core.
- [ ] **T-GPU-019.05 — Qualify and retain this target's own result**
  - Action: Bind display connector/EDID/DP/HDMI core to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GPU-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-070"></a>
## T-SVC-070 — Display Server/Compositor

**Original requirement:** current WM/FB -> isolated surfaces, scanout and atomic commit

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 90.

### Execution steps

- [ ] **T-SVC-070.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Display Server/Compositor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-070.02 — Specify the complete target boundary**
  - Action: Display Server/Compositor must supply: current WM/FB -> isolated surfaces, scanout and atomic commit. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-070.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-070.03 — Implement the exact target behavior**
  - Action: Implement or reuse Display Server/Compositor through the shared platform contract, delivering every part of: current WM/FB -> isolated surfaces, scanout and atomic commit. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-070.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-070.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-SVC-070.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Display Server/Compositor.
- [ ] **T-SVC-070.05 — Qualify and retain this target's own result**
  - Action: Bind Display Server/Compositor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-070.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-071"></a>
## T-SVC-071 — Window Manager Policy

**Original requirement:** placement, focus, stacking, workspaces and recovery

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 91.

### Execution steps

- [ ] **T-SVC-071.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Window Manager Policy to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-071.02 — Specify the complete target boundary**
  - Action: Window Manager Policy must supply: placement, focus, stacking, workspaces and recovery. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-071.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-071.03 — Implement the exact target behavior**
  - Action: Implement or reuse Window Manager Policy through the shared platform contract, delivering every part of: placement, focus, stacking, workspaces and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-071.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-071.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-SVC-071.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Window Manager Policy.
- [ ] **T-SVC-071.05 — Qualify and retain this target's own result**
  - Action: Bind Window Manager Policy to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-071.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-072"></a>
## T-SVC-072 — Input Service

**Original requirement:** devices/events/remap/repeat/acceleration and routing

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 92.

### Execution steps

- [ ] **T-SVC-072.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Input Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-072.02 — Specify the complete target boundary**
  - Action: Input Service must supply: devices/events/remap/repeat/acceleration and routing. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-072.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-072.03 — Implement the exact target behavior**
  - Action: Implement or reuse Input Service through the shared platform contract, delivering every part of: devices/events/remap/repeat/acceleration and routing. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-072.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-072.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-SVC-072.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Input Service.
- [ ] **T-SVC-072.05 — Qualify and retain this target's own result**
  - Action: Bind Input Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-072.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-073"></a>
## T-SVC-073 — Seat/Focus/Grab Service

**Original requirement:** secure ownership of keyboard/pointer/touch and grabs

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 93.

### Execution steps

- [ ] **T-SVC-073.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Seat/Focus/Grab Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-073.02 — Specify the complete target boundary**
  - Action: Seat/Focus/Grab Service must supply: secure ownership of keyboard/pointer/touch and grabs. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-073.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-073.03 — Implement the exact target behavior**
  - Action: Implement or reuse Seat/Focus/Grab Service through the shared platform contract, delivering every part of: secure ownership of keyboard/pointer/touch and grabs. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-073.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-073.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-SVC-073.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Seat/Focus/Grab Service.
- [ ] **T-SVC-073.05 — Qualify and retain this target's own result**
  - Action: Bind Seat/Focus/Grab Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-073.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-074"></a>
## T-SVC-074 — Login/Lock/Greeter Service

**Original requirement:** authenticated accessible session entry and secure attention

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 94.

### Execution steps

- [ ] **T-SVC-074.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Login/Lock/Greeter Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-074.02 — Specify the complete target boundary**
  - Action: Login/Lock/Greeter Service must supply: authenticated accessible session entry and secure attention. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-074.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-074.03 — Implement the exact target behavior**
  - Action: Implement or reuse Login/Lock/Greeter Service through the shared platform contract, delivering every part of: authenticated accessible session entry and secure attention. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-074.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-074.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-SVC-074.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Login/Lock/Greeter Service.
- [ ] **T-SVC-074.05 — Qualify and retain this target's own result**
  - Action: Bind Login/Lock/Greeter Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-074.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-075"></a>
## T-SVC-075 — Shell/Desktop Service

**Original requirement:** desktop, dock/panel, menu, notifications and workspaces

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 95.

### Execution steps

- [ ] **T-SVC-075.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Shell/Desktop Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-075.02 — Specify the complete target boundary**
  - Action: Shell/Desktop Service must supply: desktop, dock/panel, menu, notifications and workspaces. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-075.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-075.03 — Implement the exact target behavior**
  - Action: Implement or reuse Shell/Desktop Service through the shared platform contract, delivering every part of: desktop, dock/panel, menu, notifications and workspaces. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-075.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-075.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-SVC-075.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Shell/Desktop Service.
- [ ] **T-SVC-075.05 — Qualify and retain this target's own result**
  - Action: Bind Shell/Desktop Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-075.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-002"></a>
## T-APP-002 — Login/Greeter

**Original requirement:** accessible authenticated session selection

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 129.

### Execution steps

- [ ] **T-APP-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Login/Greeter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-002.02 — Specify the complete target boundary**
  - Action: Login/Greeter must supply: accessible authenticated session selection. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse Login/Greeter through the shared platform contract, delivering every part of: accessible authenticated session selection. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-002.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-APP-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Login/Greeter.
- [ ] **T-APP-002.05 — Qualify and retain this target's own result**
  - Action: Bind Login/Greeter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-003"></a>
## T-APP-003 — Lock Screen

**Original requirement:** secure attention, notification privacy and unlock

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 130.

### Execution steps

- [ ] **T-APP-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Lock Screen to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-13, D-14, H-07.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-003.02 — Specify the complete target boundary**
  - Action: Lock Screen must supply: secure attention, notification privacy and unlock. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse Lock Screen through the shared platform contract, delivering every part of: secure attention, notification privacy and unlock. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-003.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: overflowing geometry/pitch; offscreen/occluded damage; stale surface generation; frame-budget overflow; resize/close during draw; provider failure; invalid mode/power transition.
  - Requires: T-APP-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Lock Screen.
- [ ] **T-APP-003.05 — Qualify and retain this target's own result**
  - Action: Bind Lock Screen to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
