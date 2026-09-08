# MP-12: Migrate every existing app and game without losing behavior

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/apps/; kernel/src/graphics/; kernel/metadata/app-manifest.json`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-12` exports: At least one fully migrated app journey; the phase stays open until all current entries and games are accounted for.

The handoff enables only its named subset. `CLOSE-12` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-12.01 — Map every current name and launch surface to one immutable manifest identity and destination package

Map every current name and launch surface to one immutable manifest identity and destination package.

**Requires:** `D-01`, `D-02`, `D-17`, `H-00`, `H-11`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-12.02 — Migrate Terminal, Files, Editor, Settings and diagnostic clients using real process/service/file endpoints

Migrate Terminal, Files, Editor, Settings and diagnostic clients using real process/service/file endpoints.

**Requires:** `M-12.01`, `H-15`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-12 — Bounded development handoff: Migrate every existing app and game without losing behavior

At least one fully migrated app journey; the phase stays open until all current entries and games are accounted for.

**Requires:** `M-12.02`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-12.03 — Migrate creation, image, text, conversion, clipboard and personal-state utilities with save/reopen and recovery proof

Migrate creation, image, text, conversion, clipboard and personal-state utilities with save/reopen and recovery proof.

**Requires:** `M-12.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-12.04 — Migrate shell surfaces separately from ordinary apps so trusted session authority is not accidentally delegated

Migrate shell surfaces separately from ordinary apps so trusted session authority is not accidentally delegated.

**Requires:** `M-12.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-12.05 — Give every one of the 24 games a deterministic replay, input, rendering, pause/resume, close and state test

Give every one of the 24 games a deterministic replay, input, rendering, pause/resume, close and state test.

**Requires:** `M-12.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-12.06 — Prove catalogue/menu/dock/run routes reach the same app

Prove catalogue/menu/dock/run routes reach the same app; seed missing-field, wrong-ID and blank-fallback cases.

**Requires:** `M-12.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-12.07 — Compare each replacement with its retained behavior oracle, including accessibility and dependency failure

Compare each replacement with its retained behavior oracle, including accessibility and dependency failure.

**Requires:** `M-12.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-12.08 — Retire an old route only after exact parity, fallback/recovery and removal conditions are satisfied

Retire an old route only after exact parity, fallback/recovery and removal conditions are satisfied.

**Requires:** `M-12.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-GM-001](#f-gm-001) | feature | deterministic game runtime |
| [F-GM-002](#f-gm-002) | feature | game input |
| [F-GM-003](#f-gm-003) | feature | game audio |
| [F-GM-004](#f-gm-004) | feature | save/high scores |
| [F-GM-005](#f-gm-005) | feature | game frame/latency gate |
| [F-GM-006](#f-gm-006) | feature | color/motion/timing accessibility |
| [F-GM-007](#f-gm-007) | feature | board-game surface |
| [F-GM-008](#f-gm-008) | feature | arcade/collision surface |
| [F-GM-009](#f-gm-009) | feature | cellular simulation |
| [F-GM-010](#f-gm-010) | feature | rendering benchmark demo |
| [F-GM-011](#f-gm-011) | feature | 3D graphics demo |
| [F-GM-012](#f-gm-012) | feature | audio/DSP demos |
| [F-GM-013](#f-gm-013) | feature | driver/input test apps |
| [F-GM-014](#f-gm-014) | feature | UI component gallery |
| [F-GM-015](#f-gm-015) | feature | onboarding/tutorial games |
| [F-GM-016](#f-gm-016) | feature | classic game ports |
| [F-GM-017](#f-gm-017) | feature | game catalog |
| [F-GM-018](#f-gm-018) | feature | replay/export |
| [F-GM-019](#f-gm-019) | feature | game crash containment |
| [F-GM-020](#f-gm-020) | feature | benchmark anti-cheat/truth |
| [C-P7.2](#c-p7-2) | contract | ordered application migration |
| [C-DA-27](#c-da-27) | contract | system-truth applications |
| [C-DA-28](#c-da-28) | contract | file and state applications |
| [C-DA-29](#c-da-29) | contract | desktop and session applications |
| [C-DA-31](#c-da-31) | contract | deterministic tools and creation apps |
| [C-DA-32](#c-da-32) | contract | all games as runtime probes |
| [C-VX-20](#c-vx-20) | contract | System-observability app family |
| [C-VX-21](#c-vx-21) | contract | Files, editor and data-inspection family |
| [C-VX-22](#c-vx-22) | contract | Terminal, console and launch family |
| [C-VX-23](#c-vx-23) | contract | Settings and personal-state family |
| [C-VX-24](#c-vx-24) | contract | Creative/rendering family |
| [C-VX-25](#c-vx-25) | contract | Games conformance suite |
| [T-INPUT-008](#t-input-008) | target | USB gamepad/joystick |
| [T-CUR-001](#t-cur-001) | target | Terminal |
| [T-CUR-002](#t-cur-002) | target | System Monitor |
| [T-CUR-003](#t-cur-003) | target | About |
| [T-CUR-004](#t-cur-004) | target | Menu |
| [T-CUR-006](#t-cur-006) | target | Settings |
| [T-CUR-007](#t-cur-007) | target | Run |
| [T-CUR-008](#t-cur-008) | target | All Applications |
| [T-CUR-009](#t-cur-009) | target | Files |
| [T-CUR-010](#t-cur-010) | target | Text Editor |
| [T-CUR-039](#t-cur-039) | target | System |
| [T-CUR-040](#t-cur-040) | target | Type |
| [T-CUR-011](#t-cur-011) | target | Paint |
| [T-CUR-012](#t-cur-012) | target | 3D |
| [T-CUR-013](#t-cur-013) | target | zlOS animation |
| [T-CUR-014](#t-cur-014) | target | Pointer |
| [T-CUR-015](#t-cur-015) | target | Renderer |
| [T-CUR-016](#t-cur-016) | target | Framebuffer |
| [T-CUR-017](#t-cur-017) | target | Font Atlas |
| [T-CUR-018](#t-cur-018) | target | Image Viewer |
| [T-CUR-019](#t-cur-019) | target | Colour Picker |
| [T-CUR-020](#t-cur-020) | target | Clipboard |
| [T-CUR-021](#t-cur-021) | target | Regex Tester |
| [T-CUR-022](#t-cur-022) | target | Base Converter |
| [T-CUR-023](#t-cur-023) | target | Text Diff |
| [T-CUR-024](#t-cur-024) | target | Checksum |
| [T-CUR-025](#t-cur-025) | target | Unit Converter |
| [T-CUR-026](#t-cur-026) | target | Sticky Notes |
| [T-CUR-027](#t-cur-027) | target | Keyboard Tester |
| [T-CUR-028](#t-cur-028) | target | Benchmark |
| [T-CUR-029](#t-cur-029) | target | Calculator |
| [T-CUR-030](#t-cur-030) | target | Clocks & Timers |
| [T-CUR-031](#t-cur-031) | target | System Info |
| [T-CUR-032](#t-cur-032) | target | Kernel Log |
| [T-CUR-033](#t-cur-033) | target | Hex Viewer |
| [T-CUR-034](#t-cur-034) | target | Console (tty1) |
| [T-CUR-035](#t-cur-035) | target | Disk Usage |
| [T-CUR-036](#t-cur-036) | target | Services |
| [T-CUR-037](#t-cur-037) | target | Archive Manager |
| [T-CUR-038](#t-cur-038) | target | Network |
| [T-GAME-001](#t-game-001) | target | Snake |
| [T-GAME-002](#t-game-002) | target | Word Guess |
| [T-GAME-003](#t-game-003) | target | Tic-Tac-Toe |
| [T-GAME-004](#t-game-004) | target | Nim |
| [T-GAME-005](#t-game-005) | target | Tower of Hanoi |
| [T-GAME-006](#t-game-006) | target | Lights Out |
| [T-GAME-007](#t-game-007) | target | Connect Four |
| [T-GAME-008](#t-game-008) | target | Maze |
| [T-GAME-009](#t-game-009) | target | Tetris |
| [T-GAME-010](#t-game-010) | target | Pong |
| [T-GAME-011](#t-game-011) | target | Breakout |
| [T-GAME-012](#t-game-012) | target | Minesweeper |
| [T-GAME-013](#t-game-013) | target | 2048 |
| [T-GAME-014](#t-game-014) | target | Conway's Life |
| [T-GAME-015](#t-game-015) | target | Asteroids |
| [T-GAME-016](#t-game-016) | target | Invaders |
| [T-GAME-017](#t-game-017) | target | 15 Puzzle |
| [T-GAME-018](#t-game-018) | target | Reversi |
| [T-GAME-019](#t-game-019) | target | Simon |
| [T-GAME-020](#t-game-020) | target | Sokoban |
| [T-GAME-021](#t-game-021) | target | Flappy |
| [T-GAME-022](#t-game-022) | target | Missile Command |
| [T-GAME-023](#t-game-023) | target | Blackjack |
| [T-GAME-024](#t-game-024) | target | Frogger |

<a id="f-gm-001"></a>
## F-GM-001 — deterministic game runtime

**Original requirement:** monotonic tick, seeded randomness, replay, pause/background and resource limits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-001.01 — Reconcile existing deterministic game runtime**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for deterministic game runtime. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: monotonic tick, seeded randomness, replay, pause/background and resource limits
- [ ] **F-GM-001.02 — Freeze the exact contract for deterministic game runtime**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: monotonic tick, seeded randomness, replay, pause/background and resource limits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-001.03 — Implement/prove: monotonic tick**
  - Action: For deterministic game runtime, implement or reuse and verify this exact obligation: monotonic tick. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for monotonic tick; retain observable state/resource expectations.
- [ ] **F-GM-001.04 — Implement/prove: seeded randomness**
  - Action: For deterministic game runtime, implement or reuse and verify this exact obligation: seeded randomness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-001.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for seeded randomness; retain observable state/resource expectations.
- [ ] **F-GM-001.05 — Implement/prove: replay**
  - Action: For deterministic game runtime, implement or reuse and verify this exact obligation: replay. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-001.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for replay; retain observable state/resource expectations.
- [ ] **F-GM-001.06 — Implement/prove: pause/background and resource limits**
  - Action: For deterministic game runtime, implement or reuse and verify this exact obligation: pause/background and resource limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-001.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pause/background and resource limits; retain observable state/resource expectations.
- [ ] **F-GM-001.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to deterministic game runtime: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-001.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-001.08 — Integrate into the real consumer and runtime route**
  - Action: Wire deterministic game runtime into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-001.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-001.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for deterministic game runtime as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-001.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-002"></a>
## F-GM-002 — game input

**Original requirement:** keyboard/pointer/gamepad mapping, focus/grab and accessible remapping

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-002.01 — Reconcile existing game input**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for game input. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: keyboard/pointer/gamepad mapping, focus/grab and accessible remapping
- [ ] **F-GM-002.02 — Freeze the exact contract for game input**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: keyboard/pointer/gamepad mapping, focus/grab and accessible remapping. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-002.03 — Implement/prove: keyboard/pointer/gamepad mapping**
  - Action: For game input, implement or reuse and verify this exact obligation: keyboard/pointer/gamepad mapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard/pointer/gamepad mapping; retain observable state/resource expectations.
- [ ] **F-GM-002.04 — Implement/prove: focus/grab and accessible remapping**
  - Action: For game input, implement or reuse and verify this exact obligation: focus/grab and accessible remapping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus/grab and accessible remapping; retain observable state/resource expectations.
- [ ] **F-GM-002.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to game input: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-002.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-002.06 — Integrate into the real consumer and runtime route**
  - Action: Wire game input into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-002.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-002.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for game input as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-002.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-003"></a>
## F-GM-003 — game audio

**Original requirement:** AudioServer cues/music, per-app volume and device recovery

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-003.01 — Reconcile existing game audio**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for game audio. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: AudioServer cues/music, per-app volume and device recovery
- [ ] **F-GM-003.02 — Freeze the exact contract for game audio**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: AudioServer cues/music, per-app volume and device recovery. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-003.03 — Implement/prove: AudioServer cues/music**
  - Action: For game audio, implement or reuse and verify this exact obligation: AudioServer cues/music. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for AudioServer cues/music; retain observable state/resource expectations.
- [ ] **F-GM-003.04 — Implement/prove: per-app volume and device recovery**
  - Action: For game audio, implement or reuse and verify this exact obligation: per-app volume and device recovery. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-app volume and device recovery; retain observable state/resource expectations.
- [ ] **F-GM-003.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to game audio: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-003.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-003.06 — Integrate into the real consumer and runtime route**
  - Action: Wire game audio into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-003.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-003.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for game audio as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-003.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-004"></a>
## F-GM-004 — save/high scores

**Original requirement:** per-user durable data, versioning, atomic save and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-004.01 — Reconcile existing save/high scores**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for save/high scores. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: per-user durable data, versioning, atomic save and privacy
- [ ] **F-GM-004.02 — Freeze the exact contract for save/high scores**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: per-user durable data, versioning, atomic save and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-004.03 — Implement/prove: per-user durable data**
  - Action: For save/high scores, implement or reuse and verify this exact obligation: per-user durable data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for per-user durable data; retain observable state/resource expectations.
- [ ] **F-GM-004.04 — Implement/prove: versioning**
  - Action: For save/high scores, implement or reuse and verify this exact obligation: versioning. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioning; retain observable state/resource expectations.
- [ ] **F-GM-004.05 — Implement/prove: atomic save and privacy**
  - Action: For save/high scores, implement or reuse and verify this exact obligation: atomic save and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-004.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for atomic save and privacy; retain observable state/resource expectations.
- [ ] **F-GM-004.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to save/high scores: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-004.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-004.07 — Integrate into the real consumer and runtime route**
  - Action: Wire save/high scores into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-004.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-004.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for save/high scores as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-004.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-005"></a>
## F-GM-005 — game frame/latency gate

**Original requirement:** input-to-present, peak frame, dropped updates and exact scenario receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-005.01 — Reconcile existing game frame/latency gate**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for game frame/latency gate. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: input-to-present, peak frame, dropped updates and exact scenario receipt
- [ ] **F-GM-005.02 — Freeze the exact contract for game frame/latency gate**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: input-to-present, peak frame, dropped updates and exact scenario receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-005.03 — Implement/prove: input-to-present**
  - Action: For game frame/latency gate, implement or reuse and verify this exact obligation: input-to-present. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for input-to-present; retain observable state/resource expectations.
- [ ] **F-GM-005.04 — Implement/prove: peak frame**
  - Action: For game frame/latency gate, implement or reuse and verify this exact obligation: peak frame. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for peak frame; retain observable state/resource expectations.
- [ ] **F-GM-005.05 — Implement/prove: dropped updates and exact scenario receipt**
  - Action: For game frame/latency gate, implement or reuse and verify this exact obligation: dropped updates and exact scenario receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dropped updates and exact scenario receipt; retain observable state/resource expectations.
- [ ] **F-GM-005.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to game frame/latency gate: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-005.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-005.07 — Integrate into the real consumer and runtime route**
  - Action: Wire game frame/latency gate into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-005.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-005.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for game frame/latency gate as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-005.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-006"></a>
## F-GM-006 — color/motion/timing accessibility

**Original requirement:** non-color cues, reduced motion, adjustable timing and keyboard alternatives

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-006.01 — Reconcile existing color/motion/timing accessibility**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for color/motion/timing accessibility. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: non-color cues, reduced motion, adjustable timing and keyboard alternatives
- [ ] **F-GM-006.02 — Freeze the exact contract for color/motion/timing accessibility**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: non-color cues, reduced motion, adjustable timing and keyboard alternatives. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-006.03 — Implement/prove: non-color cues**
  - Action: For color/motion/timing accessibility, implement or reuse and verify this exact obligation: non-color cues. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for non-color cues; retain observable state/resource expectations.
- [ ] **F-GM-006.04 — Implement/prove: reduced motion**
  - Action: For color/motion/timing accessibility, implement or reuse and verify this exact obligation: reduced motion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reduced motion; retain observable state/resource expectations.
- [ ] **F-GM-006.05 — Implement/prove: adjustable timing and keyboard alternatives**
  - Action: For color/motion/timing accessibility, implement or reuse and verify this exact obligation: adjustable timing and keyboard alternatives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for adjustable timing and keyboard alternatives; retain observable state/resource expectations.
- [ ] **F-GM-006.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to color/motion/timing accessibility: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-006.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-006.07 — Integrate into the real consumer and runtime route**
  - Action: Wire color/motion/timing accessibility into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-006.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-006.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for color/motion/timing accessibility as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-006.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-007"></a>
## F-GM-007 — board-game surface

**Original requirement:** deterministic rules, keyboard/pointer semantics, status, reset and replay

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-007.01 — Reconcile existing board-game surface**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for board-game surface. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deterministic rules, keyboard/pointer semantics, status, reset and replay
- [ ] **F-GM-007.02 — Freeze the exact contract for board-game surface**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deterministic rules, keyboard/pointer semantics, status, reset and replay. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-007.03 — Implement/prove: deterministic rules**
  - Action: For board-game surface, implement or reuse and verify this exact obligation: deterministic rules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deterministic rules; retain observable state/resource expectations.
- [ ] **F-GM-007.04 — Implement/prove: keyboard/pointer semantics**
  - Action: For board-game surface, implement or reuse and verify this exact obligation: keyboard/pointer semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard/pointer semantics; retain observable state/resource expectations.
- [ ] **F-GM-007.05 — Implement/prove: status**
  - Action: For board-game surface, implement or reuse and verify this exact obligation: status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-007.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for status; retain observable state/resource expectations.
- [ ] **F-GM-007.06 — Implement/prove: reset and replay**
  - Action: For board-game surface, implement or reuse and verify this exact obligation: reset and replay. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-007.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reset and replay; retain observable state/resource expectations.
- [ ] **F-GM-007.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to board-game surface: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-007.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-007.08 — Integrate into the real consumer and runtime route**
  - Action: Wire board-game surface into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-007.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-007.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for board-game surface as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-007.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-008"></a>
## F-GM-008 — arcade/collision surface

**Original requirement:** movement/collision/score/lives/timing and bounded frame behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-008.01 — Reconcile existing arcade/collision surface**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for arcade/collision surface. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: movement/collision/score/lives/timing and bounded frame behavior
- [ ] **F-GM-008.02 — Freeze the exact contract for arcade/collision surface**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: movement/collision/score/lives/timing and bounded frame behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-008.03 — Implement/prove: movement/collision/score/lives/timing and bounded frame behavior**
  - Action: For arcade/collision surface, implement or reuse and verify this exact obligation: movement/collision/score/lives/timing and bounded frame behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for movement/collision/score/lives/timing and bounded frame behavior; retain observable state/resource expectations.
- [ ] **F-GM-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to arcade/collision surface: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire arcade/collision surface into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for arcade/collision surface as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-009"></a>
## F-GM-009 — cellular simulation

**Original requirement:** grid editing, stepping/running, speed, reset and deterministic patterns

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-009.01 — Reconcile existing cellular simulation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cellular simulation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: grid editing, stepping/running, speed, reset and deterministic patterns
- [ ] **F-GM-009.02 — Freeze the exact contract for cellular simulation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: grid editing, stepping/running, speed, reset and deterministic patterns. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-009.03 — Implement/prove: grid editing**
  - Action: For cellular simulation, implement or reuse and verify this exact obligation: grid editing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grid editing; retain observable state/resource expectations.
- [ ] **F-GM-009.04 — Implement/prove: stepping/running**
  - Action: For cellular simulation, implement or reuse and verify this exact obligation: stepping/running. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stepping/running; retain observable state/resource expectations.
- [ ] **F-GM-009.05 — Implement/prove: speed**
  - Action: For cellular simulation, implement or reuse and verify this exact obligation: speed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-009.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for speed; retain observable state/resource expectations.
- [ ] **F-GM-009.06 — Implement/prove: reset and deterministic patterns**
  - Action: For cellular simulation, implement or reuse and verify this exact obligation: reset and deterministic patterns. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-009.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reset and deterministic patterns; retain observable state/resource expectations.
- [ ] **F-GM-009.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cellular simulation: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-009.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-009.08 — Integrate into the real consumer and runtime route**
  - Action: Wire cellular simulation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-009.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-009.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cellular simulation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-009.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-010"></a>
## F-GM-010 — rendering benchmark demo

**Original requirement:** workload identity, backend comparison, controls and honest results

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-010.01 — Reconcile existing rendering benchmark demo**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for rendering benchmark demo. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: workload identity, backend comparison, controls and honest results
- [ ] **F-GM-010.02 — Freeze the exact contract for rendering benchmark demo**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: workload identity, backend comparison, controls and honest results. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-010.03 — Implement/prove: workload identity**
  - Action: For rendering benchmark demo, implement or reuse and verify this exact obligation: workload identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for workload identity; retain observable state/resource expectations.
- [ ] **F-GM-010.04 — Implement/prove: backend comparison**
  - Action: For rendering benchmark demo, implement or reuse and verify this exact obligation: backend comparison. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for backend comparison; retain observable state/resource expectations.
- [ ] **F-GM-010.05 — Implement/prove: controls and honest results**
  - Action: For rendering benchmark demo, implement or reuse and verify this exact obligation: controls and honest results. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controls and honest results; retain observable state/resource expectations.
- [ ] **F-GM-010.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to rendering benchmark demo: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-010.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-010.07 — Integrate into the real consumer and runtime route**
  - Action: Wire rendering benchmark demo into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-010.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-010.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for rendering benchmark demo as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-010.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-011"></a>
## F-GM-011 — 3D graphics demo

**Original requirement:** reusable scene/camera primitives and backend/performance oracle

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-011.01 — Reconcile existing 3D graphics demo**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for 3D graphics demo. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: reusable scene/camera primitives and backend/performance oracle
- [ ] **F-GM-011.02 — Freeze the exact contract for 3D graphics demo**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: reusable scene/camera primitives and backend/performance oracle. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-011.03 — Implement/prove: reusable scene/camera primitives and backend/performance oracle**
  - Action: For 3D graphics demo, implement or reuse and verify this exact obligation: reusable scene/camera primitives and backend/performance oracle. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reusable scene/camera primitives and backend/performance oracle; retain observable state/resource expectations.
- [ ] **F-GM-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to 3D graphics demo: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire 3D graphics demo into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for 3D graphics demo as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-012"></a>
## F-GM-012 — audio/DSP demos

**Original requirement:** golden outputs plus realtime-service integration when claimed

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-012.01 — Reconcile existing audio/DSP demos**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio/DSP demos. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: golden outputs plus realtime-service integration when claimed
- [ ] **F-GM-012.02 — Freeze the exact contract for audio/DSP demos**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: golden outputs plus realtime-service integration when claimed. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-012.03 — Implement/prove: golden outputs plus realtime-service integration when claimed**
  - Action: For audio/DSP demos, implement or reuse and verify this exact obligation: golden outputs plus realtime-service integration when claimed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for golden outputs plus realtime-service integration when claimed; retain observable state/resource expectations.
- [ ] **F-GM-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio/DSP demos: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio/DSP demos into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio/DSP demos as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-013"></a>
## F-GM-013 — driver/input test apps

**Original requirement:** class-specific event visualization and physical evidence capture

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-013.01 — Reconcile existing driver/input test apps**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for driver/input test apps. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: class-specific event visualization and physical evidence capture
- [ ] **F-GM-013.02 — Freeze the exact contract for driver/input test apps**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: class-specific event visualization and physical evidence capture. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-013.03 — Implement/prove: class-specific event visualization and physical evidence capture**
  - Action: For driver/input test apps, implement or reuse and verify this exact obligation: class-specific event visualization and physical evidence capture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for class-specific event visualization and physical evidence capture; retain observable state/resource expectations.
- [ ] **F-GM-013.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to driver/input test apps: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-013.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-013.05 — Integrate into the real consumer and runtime route**
  - Action: Wire driver/input test apps into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-013.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-013.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for driver/input test apps as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-013.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-014"></a>
## F-GM-014 — UI component gallery

**Original requirement:** all states/themes/scales/locales/a11y and visual-performance regression

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-014.01 — Reconcile existing UI component gallery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for UI component gallery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: all states/themes/scales/locales/a11y and visual-performance regression
- [ ] **F-GM-014.02 — Freeze the exact contract for UI component gallery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: all states/themes/scales/locales/a11y and visual-performance regression. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-014.03 — Implement/prove: all states/themes/scales/locales/a11y and visual-performance regression**
  - Action: For UI component gallery, implement or reuse and verify this exact obligation: all states/themes/scales/locales/a11y and visual-performance regression. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for all states/themes/scales/locales/a11y and visual-performance regression; retain observable state/resource expectations.
- [ ] **F-GM-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to UI component gallery: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire UI component gallery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for UI component gallery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-015"></a>
## F-GM-015 — onboarding/tutorial games

**Original requirement:** teach input/window/accessibility without hiding product state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-015.01 — Reconcile existing onboarding/tutorial games**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for onboarding/tutorial games. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: teach input/window/accessibility without hiding product state
- [ ] **F-GM-015.02 — Freeze the exact contract for onboarding/tutorial games**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: teach input/window/accessibility without hiding product state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-015.03 — Implement/prove: teach input/window/accessibility without hiding product state**
  - Action: For onboarding/tutorial games, implement or reuse and verify this exact obligation: teach input/window/accessibility without hiding product state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for teach input/window/accessibility without hiding product state; retain observable state/resource expectations.
- [ ] **F-GM-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to onboarding/tutorial games: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire onboarding/tutorial games into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for onboarding/tutorial games as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-016"></a>
## F-GM-016 — classic game ports

**Original requirement:** packaged sandboxed external content with license/provenance and cleanup

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-016.01 — Reconcile existing classic game ports**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for classic game ports. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: packaged sandboxed external content with license/provenance and cleanup
- [ ] **F-GM-016.02 — Freeze the exact contract for classic game ports**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: packaged sandboxed external content with license/provenance and cleanup. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-016.03 — Implement/prove: packaged sandboxed external content with license/provenance and cleanup**
  - Action: For classic game ports, implement or reuse and verify this exact obligation: packaged sandboxed external content with license/provenance and cleanup. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for packaged sandboxed external content with license/provenance and cleanup; retain observable state/resource expectations.
- [ ] **F-GM-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to classic game ports: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire classic game ports into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for classic game ports as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-017"></a>
## F-GM-017 — game catalog

**Original requirement:** generated exact route/ID/icon/package metadata and launch proof

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-017.01 — Reconcile existing game catalog**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for game catalog. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated exact route/ID/icon/package metadata and launch proof
- [ ] **F-GM-017.02 — Freeze the exact contract for game catalog**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated exact route/ID/icon/package metadata and launch proof. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-017.03 — Implement/prove: generated exact route/ID/icon/package metadata and launch proof**
  - Action: For game catalog, implement or reuse and verify this exact obligation: generated exact route/ID/icon/package metadata and launch proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated exact route/ID/icon/package metadata and launch proof; retain observable state/resource expectations.
- [ ] **F-GM-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to game catalog: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire game catalog into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for game catalog as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-018"></a>
## F-GM-018 — replay/export

**Original requirement:** portable versioned input/state trace for bugs and deterministic review

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-018.01 — Reconcile existing replay/export**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for replay/export. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: portable versioned input/state trace for bugs and deterministic review
- [ ] **F-GM-018.02 — Freeze the exact contract for replay/export**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: portable versioned input/state trace for bugs and deterministic review. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-018.03 — Implement/prove: portable versioned input/state trace for bugs and deterministic review**
  - Action: For replay/export, implement or reuse and verify this exact obligation: portable versioned input/state trace for bugs and deterministic review. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for portable versioned input/state trace for bugs and deterministic review; retain observable state/resource expectations.
- [ ] **F-GM-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to replay/export: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire replay/export into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for replay/export as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-019"></a>
## F-GM-019 — game crash containment

**Original requirement:** process failure removes handles/surfaces/audio/grabs without harming session

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-019.01 — Reconcile existing game crash containment**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for game crash containment. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: process failure removes handles/surfaces/audio/grabs without harming session
- [ ] **F-GM-019.02 — Freeze the exact contract for game crash containment**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: process failure removes handles/surfaces/audio/grabs without harming session. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-019.03 — Implement/prove: process failure removes handles/surfaces/audio/grabs without harming session**
  - Action: For game crash containment, implement or reuse and verify this exact obligation: process failure removes handles/surfaces/audio/grabs without harming session. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for process failure removes handles/surfaces/audio/grabs without harming session; retain observable state/resource expectations.
- [ ] **F-GM-019.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to game crash containment: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-019.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-019.05 — Integrate into the real consumer and runtime route**
  - Action: Wire game crash containment into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-019.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-019.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for game crash containment as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-019.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-gm-020"></a>
## F-GM-020 — benchmark anti-cheat/truth

**Original requirement:** no simulated score/result; exact code/artifact/backend/workload receipt

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-GM-020.01 — Reconcile existing benchmark anti-cheat/truth**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for benchmark anti-cheat/truth. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-and-evidence comparison against the complete requirement: no simulated score/result; exact code/artifact/backend/workload receipt
- [ ] **F-GM-020.02 — Freeze the exact contract for benchmark anti-cheat/truth**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: no simulated score/result; exact code/artifact/backend/workload receipt. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-GM-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-GM-020.03 — Implement/prove: no simulated score/result**
  - Action: For benchmark anti-cheat/truth, implement or reuse and verify this exact obligation: no simulated score/result. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for no simulated score/result; retain observable state/resource expectations.
- [ ] **F-GM-020.04 — Implement/prove: exact code/artifact/backend/workload receipt**
  - Action: For benchmark anti-cheat/truth, implement or reuse and verify this exact obligation: exact code/artifact/backend/workload receipt. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-GM-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for exact code/artifact/backend/workload receipt; retain observable state/resource expectations.
- [ ] **F-GM-020.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to benchmark anti-cheat/truth: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-GM-020.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-GM-020.06 — Integrate into the real consumer and runtime route**
  - Action: Wire benchmark anti-cheat/truth into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-GM-020.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-GM-020.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for benchmark anti-cheat/truth as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-GM-020.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-p7-2"></a>
## C-P7.2 — ordered application migration

**Original requirement:** ordered application migration

**Source:** [docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 389.

### Preserved original contract

- **Dependencies/current/provenance:** P7.1 and service protocols; TacOS/protOS vertical slices, MaslOS conversations; reject big-bang app rewrite.
- **I/O and state:** current app workflow/oracle in; process app package out; order is assertion -> terminal -> editor/Files -> System Monitor -> Settings -> browser -> rest.
- **Invariants/failure:** visual/workflow parity; save authority explicit; one app crash isolated; no direct kernel/global state after migration.
- **Deterministic proof:** per-app action script, scene hash, input/focus, persistence, crash, resource leak and denied-capability tests.
- **Target proof:** QEMU all apps; ThinkPad primary workflows/latency after each migration.
- **Receipt/removal:** old/new workflow diff and resource/cap map; per-app feature switch; remove hook/global only after exact caller inventory zero.

### Execution steps

- [ ] **C-P7.2.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-P7.2.02 — Resolve this contract's exact dependencies**
  - Action: P7.1 and service protocols; TacOS/protOS vertical slices, MaslOS conversations; reject big-bang app rewrite. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-P7.2.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-P7.2.03 — I/O and state — ordered application migration**
  - Action: current app workflow/oracle in; process app package out; order is assertion -> terminal -> editor/Files -> System Monitor -> Settings -> browser -> rest.
  - Requires: C-P7.2.02.
  - Acceptance: Satisfy every obligation in the preserved I/O and state field for P7.2.
- [ ] **C-P7.2.04 — Invariants/failure — ordered application migration**
  - Action: visual/workflow parity; save authority explicit; one app crash isolated; no direct kernel/global state after migration.
  - Requires: C-P7.2.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants/failure field for P7.2.
- [ ] **C-P7.2.05 — Deterministic proof — ordered application migration**
  - Action: per-app action script, scene hash, input/focus, persistence, crash, resource leak and denied-capability tests.
  - Requires: C-P7.2.04.
  - Acceptance: Satisfy every obligation in the preserved Deterministic proof field for P7.2.
- [ ] **C-P7.2.06 — Target proof — ordered application migration**
  - Action: QEMU all apps; ThinkPad primary workflows/latency after each migration.
  - Requires: C-P7.2.05.
  - Acceptance: Satisfy every obligation in the preserved Target proof field for P7.2.
- [ ] **C-P7.2.07 — Receipt/removal — ordered application migration**
  - Action: old/new workflow diff and resource/cap map; per-app feature switch; remove hook/global only after exact caller inventory zero.
  - Requires: C-P7.2.06.
  - Acceptance: Satisfy every obligation in the preserved Receipt/removal field for P7.2.
- [ ] **C-P7.2.08 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for P7.2. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-P7.2.07.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-27"></a>
## C-DA-27 — system-truth applications

**Original requirement:** system-truth applications

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 697.

### Preserved original contract

**Depends on:** DA-08O, DA-19 and the relevant provider/service schemas.

**Apps:** System Monitor, About, Clocks & Timers, Kernel Log, System Info, Services,
Disk Usage, Network, Framebuffer, Renderer, Font Atlas, Keyboard Tester and
Benchmark.

**Deliver:** read-only clients of typed telemetry/provider/supervisor interfaces.

**Proof:** no arbitrary kernel-global access; permission/redaction tests; provider
restart and unavailable states; displayed values match recorded fixtures and never
silently render unknown as zero.

### Execution steps

- [ ] **C-DA-27.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-27.02 — Resolve this contract's exact dependencies**
  - Action: DA-08O, DA-19 and the relevant provider/service schemas. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-27.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-27.03 — Apps — system-truth applications**
  - Action: System Monitor, About, Clocks & Timers, Kernel Log, System Info, Services, Disk Usage, Network, Framebuffer, Renderer, Font Atlas, Keyboard Tester and Benchmark.
  - Requires: C-DA-27.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-27.
- [ ] **C-DA-27.04 — Deliver — system-truth applications**
  - Action: read-only clients of typed telemetry/provider/supervisor interfaces.
  - Requires: C-DA-27.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-27.
- [ ] **C-DA-27.05 — Proof — system-truth applications**
  - Action: no arbitrary kernel-global access; permission/redaction tests; provider restart and unavailable states; displayed values match recorded fixtures and never silently render unknown as zero.
  - Requires: C-DA-27.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-27.
- [ ] **C-DA-27.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-27. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-27.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-28"></a>
## C-DA-28 — file and state applications

**Original requirement:** file and state applications

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 711.

### Preserved original contract

**Depends on:** DA-20C, DA-21/DA-21F, DA-23L and DA-25.

**Apps:** Files, Text Editor, Sticky Notes, Hex Viewer, Archive Manager, Image
Viewer, Text Diff and Checksum.

**Deliver:** directory/file handles, file picker, safe-save/rename, autosave/recovery,
restricted parsers and persistent schema migration.

**Proof:** current zlfs differential fixtures, disk full, rename collision, crash
during save, corrupt input, huge input budget, revoked handle, cold boot and recovery.

### Execution steps

- [ ] **C-DA-28.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-28.02 — Resolve this contract's exact dependencies**
  - Action: DA-20C, DA-21/DA-21F, DA-23L and DA-25. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-28.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-28.03 — Apps — file and state applications**
  - Action: Files, Text Editor, Sticky Notes, Hex Viewer, Archive Manager, Image Viewer, Text Diff and Checksum.
  - Requires: C-DA-28.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-28.
- [ ] **C-DA-28.04 — Deliver — file and state applications**
  - Action: directory/file handles, file picker, safe-save/rename, autosave/recovery, restricted parsers and persistent schema migration.
  - Requires: C-DA-28.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-28.
- [ ] **C-DA-28.05 — Proof — file and state applications**
  - Action: current zlfs differential fixtures, disk full, rename collision, crash during save, corrupt input, huge input budget, revoked handle, cold boot and recovery.
  - Requires: C-DA-28.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-28.
- [ ] **C-DA-28.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-28. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-28.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-29"></a>
## C-DA-29 — desktop and session applications

**Original requirement:** desktop and session applications

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 724.

### Preserved original contract

**Depends on:** DA-20, DA-20C, DA-23L, DA-25 and DA-25N.

**Apps:** Terminal, Console (`tty1`), Run, Settings, Menu/catalogue/dock, Clipboard,
notifications, login/lock and accessibility tools. Console consolidates into the
Terminal/PTY service while its stable ID is preserved as a migration alias or
explicitly retired; it cannot disappear as an untracked gap.

**Deliver:** shell/PTY process tree; clients of DA-23L launch/handler routing,
DA-20C settings and DA-25N notification/crash services; clipboard privacy, focus
navigation and semantic names.

**Proof:** hostile command arguments, child/orphan cleanup, missing executable,
catalogue inverse mapping, clipboard snoop denial, lock-session isolation, keyboard-
only operation and screen-reader metadata inspection.

### Execution steps

- [ ] **C-DA-29.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-29.02 — Resolve this contract's exact dependencies**
  - Action: DA-20, DA-20C, DA-23L, DA-25 and DA-25N. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-29.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-29.03 — Apps — desktop and session applications**
  - Action: Terminal, Console (`tty1`), Run, Settings, Menu/catalogue/dock, Clipboard, notifications, login/lock and accessibility tools. Console consolidates into the Terminal/PTY service while its stable ID is preserved as a migration alias or explicitly retired; it cannot disappear as an untracked gap.
  - Requires: C-DA-29.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-29.
- [ ] **C-DA-29.04 — Deliver — desktop and session applications**
  - Action: shell/PTY process tree; clients of DA-23L launch/handler routing, DA-20C settings and DA-25N notification/crash services; clipboard privacy, focus navigation and semantic names.
  - Requires: C-DA-29.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-29.
- [ ] **C-DA-29.05 — Proof — desktop and session applications**
  - Action: hostile command arguments, child/orphan cleanup, missing executable, catalogue inverse mapping, clipboard snoop denial, lock-session isolation, keyboard- only operation and screen-reader metadata inspection.
  - Requires: C-DA-29.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-29.
- [ ] **C-DA-29.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-29. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-29.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-31"></a>
## C-DA-31 — deterministic tools and creation apps

**Original requirement:** deterministic tools and creation apps

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 753.

### Preserved original contract

**Apps:** Calculator, Colour Picker, converters, regex, diff/checksum, Paint, 3D,
animation, pointer demos and later editor/compiler/build/debug/profile/source-
control tools.

**Deliver:** pure cores separated from UI and privileged effects; versioned project
formats; reproducible compiler/build receipts after self-hosting.

**Proof:** host golden/property tests, locale/overflow/large input, malformed project,
undo/redo and safe-save, app crash/restart, QEMU launch/input/render/close.

### Execution steps

- [ ] **C-DA-31.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-31.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-31.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-31.03 — Apps — deterministic tools and creation apps**
  - Action: Calculator, Colour Picker, converters, regex, diff/checksum, Paint, 3D, animation, pointer demos and later editor/compiler/build/debug/profile/source- control tools.
  - Requires: C-DA-31.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-31.
- [ ] **C-DA-31.04 — Deliver — deterministic tools and creation apps**
  - Action: pure cores separated from UI and privileged effects; versioned project formats; reproducible compiler/build receipts after self-hosting.
  - Requires: C-DA-31.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-31.
- [ ] **C-DA-31.05 — Proof — deterministic tools and creation apps**
  - Action: host golden/property tests, locale/overflow/large input, malformed project, undo/redo and safe-save, app crash/restart, QEMU launch/input/render/close.
  - Requires: C-DA-31.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-31.
- [ ] **C-DA-31.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-31. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-31.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-da-32"></a>
## C-DA-32 — all games as runtime probes

**Original requirement:** all games as runtime probes

**Source:** [docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/DRIVER_AND_APP_IMPLEMENTATION_CONTRACT_BACKLOG_2026-08-21.md), line 785.

### Preserved original contract

**Apps:** the current 24 named games, including Maze only after its route exists.

**Deliver:** deterministic seeded rules, event/timer/render separation, save state,
resource budgets and AudioServer use. Preserve each current game's behavior while
moving it into an ordinary process.

**Proof:** direct rule suites plus registry/launch/ready/input/render/close/restart;
seed replay, pause/time wrap, malformed save, quota exceed, audio loss and one-game
crash while desktop and other apps continue.

### Execution steps

- [ ] **C-DA-32.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-DA-32.02 — Resolve this contract's exact dependencies**
  - Action: Inherited phase and source-document dependency rules; inspect the complete source contract below. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-DA-32.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-DA-32.03 — Apps — all games as runtime probes**
  - Action: the current 24 named games, including Maze only after its route exists.
  - Requires: C-DA-32.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for DA-32.
- [ ] **C-DA-32.04 — Deliver — all games as runtime probes**
  - Action: deterministic seeded rules, event/timer/render separation, save state, resource budgets and AudioServer use. Preserve each current game's behavior while moving it into an ordinary process.
  - Requires: C-DA-32.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for DA-32.
- [ ] **C-DA-32.05 — Proof — all games as runtime probes**
  - Action: direct rule suites plus registry/launch/ready/input/render/close/restart; seed replay, pause/time wrap, malformed save, quota exceed, audio loss and one-game crash while desktop and other apps continue.
  - Requires: C-DA-32.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for DA-32.
- [ ] **C-DA-32.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for DA-32. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-DA-32.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-20"></a>
## C-VX-20 — System-observability app family

**Original requirement:** System-observability app family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 395.

### Preserved original contract

**Apps:** About, System Monitor, System Info, Kernel Log, Services, Disk Usage,
Network, Benchmark, Framebuffer, Font Atlas, Pointer.

**Depends on:** VX-18, typed telemetry/logger/device services.

**Deliver:** read-only snapshots/streams, filters, copy/export, exact source/
backend/time, degraded state and separately authorized actions.

**Proof:** provider restart, counter wrap, missing sensor, redaction, huge log,
stale action, ordinary-user privileged denial and receipt verification.

### Execution steps

- [ ] **C-VX-20.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-20.02 — Resolve this contract's exact dependencies**
  - Action: VX-18, typed telemetry/logger/device services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-20.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-20.03 — Apps — System-observability app family**
  - Action: About, System Monitor, System Info, Kernel Log, Services, Disk Usage, Network, Benchmark, Framebuffer, Font Atlas, Pointer.
  - Requires: C-VX-20.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for VX-20.
- [ ] **C-VX-20.04 — Deliver — System-observability app family**
  - Action: read-only snapshots/streams, filters, copy/export, exact source/ backend/time, degraded state and separately authorized actions.
  - Requires: C-VX-20.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-20.
- [ ] **C-VX-20.05 — Proof — System-observability app family**
  - Action: provider restart, counter wrap, missing sensor, redaction, huge log, stale action, ordinary-user privileged denial and receipt verification.
  - Requires: C-VX-20.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-20.
- [ ] **C-VX-20.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-20. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-20.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-21"></a>
## C-VX-21 — Files, editor and data-inspection family

**Original requirement:** Files, editor and data-inspection family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 408.

### Preserved original contract

**Apps:** Files, Text Editor, Sticky Notes, Regex Tester, Text Diff, Checksum,
Hex Viewer, Archive Manager, Image Viewer.

**Depends on:** VX-16, File service, decoder workers.

**Deliver:** handle-based open/save, safe-save transaction, dirty/undo/recovery,
huge-file paging, search, hostile archive/image isolation and recent items.

**Proof:** disk full, rename/flush failure, app/decoder crash, malformed input,
symlink/traversal/bomb, concurrent change, revoke mid-save and reboot recovery.

### Execution steps

- [ ] **C-VX-21.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-21.02 — Resolve this contract's exact dependencies**
  - Action: VX-16, File service, decoder workers. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-21.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-21.03 — Apps — Files, editor and data-inspection family**
  - Action: Files, Text Editor, Sticky Notes, Regex Tester, Text Diff, Checksum, Hex Viewer, Archive Manager, Image Viewer.
  - Requires: C-VX-21.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for VX-21.
- [ ] **C-VX-21.04 — Deliver — Files, editor and data-inspection family**
  - Action: handle-based open/save, safe-save transaction, dirty/undo/recovery, huge-file paging, search, hostile archive/image isolation and recent items.
  - Requires: C-VX-21.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-21.
- [ ] **C-VX-21.05 — Proof — Files, editor and data-inspection family**
  - Action: disk full, rename/flush failure, app/decoder crash, malformed input, symlink/traversal/bomb, concurrent change, revoke mid-save and reboot recovery.
  - Requires: C-VX-21.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-21.
- [ ] **C-VX-21.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-21. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-21.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-22"></a>
## C-VX-22 — Terminal, console and launch family

**Original requirement:** Terminal, console and launch family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 421.

### Preserved original contract

**Apps:** Terminal, Console (`tty1`), Run.

**Depends on:** VX-18, PTY/process/session services.

**Deliver:** PTY tabs, process tree, resize/signals/exit, scroll/search/copy,
command parsing and explicit recovery-console role or retirement migration.

**Proof:** hostile arguments/environment, child fork/exec failure, terminal
death, signal permissions including signal 0, huge output/backpressure, resize,
logout and no orphan children.

### Execution steps

- [ ] **C-VX-22.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-22.02 — Resolve this contract's exact dependencies**
  - Action: VX-18, PTY/process/session services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-22.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-22.03 — Apps — Terminal, console and launch family**
  - Action: Terminal, Console (`tty1`), Run.
  - Requires: C-VX-22.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for VX-22.
- [ ] **C-VX-22.04 — Deliver — Terminal, console and launch family**
  - Action: PTY tabs, process tree, resize/signals/exit, scroll/search/copy, command parsing and explicit recovery-console role or retirement migration.
  - Requires: C-VX-22.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-22.
- [ ] **C-VX-22.05 — Proof — Terminal, console and launch family**
  - Action: hostile arguments/environment, child fork/exec failure, terminal death, signal permissions including signal 0, huge output/backpressure, resize, logout and no orphan children.
  - Requires: C-VX-22.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-22.
- [ ] **C-VX-22.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-22. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-22.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-23"></a>
## C-VX-23 — Settings and personal-state family

**Original requirement:** Settings and personal-state family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 434.

### Preserved original contract

**Apps:** Settings, Clipboard, Clocks & Timers, Calculator, Base Converter, Unit
Converter, Colour Picker, Keyboard Tester.

**Depends on:** VX-03, VX-15, VX-16, per-user settings service.

**Deliver:** staged preferences, typed clipboard, alarms, history/precision,
screen-pick permission, physical/logical key inspection and schema migration.

**Proof:** failed persistence, reboot, locale/timezone, suspend timer, clipboard
owner death/privacy, capture denial, layout change and invalid numeric input.

### Execution steps

- [ ] **C-VX-23.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-23.02 — Resolve this contract's exact dependencies**
  - Action: VX-03, VX-15, VX-16, per-user settings service. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-23.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-23.03 — Apps — Settings and personal-state family**
  - Action: Settings, Clipboard, Clocks & Timers, Calculator, Base Converter, Unit Converter, Colour Picker, Keyboard Tester.
  - Requires: C-VX-23.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for VX-23.
- [ ] **C-VX-23.04 — Deliver — Settings and personal-state family**
  - Action: staged preferences, typed clipboard, alarms, history/precision, screen-pick permission, physical/logical key inspection and schema migration.
  - Requires: C-VX-23.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-23.
- [ ] **C-VX-23.05 — Proof — Settings and personal-state family**
  - Action: failed persistence, reboot, locale/timezone, suspend timer, clipboard owner death/privacy, capture denial, layout change and invalid numeric input.
  - Requires: C-VX-23.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-23.
- [ ] **C-VX-23.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-23. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-23.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-24"></a>
## C-VX-24 — Creative/rendering family

**Original requirement:** Creative/rendering family

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 447.

### Preserved original contract

**Apps:** Paint, Renderer, 3D, zlOS animation.

**Depends on:** VX-11, VX-12, VX-16, File/Media services.

**Deliver:** versioned project state, undo, tools/selection/layers as appropriate,
safe import/export, reusable scene/render interface and deterministic demo mode.

**Proof:** huge canvas/mesh, decoder failure, save crash, backend reset, undo
round-trip, software/GPU differential and project migration.

### Execution steps

- [ ] **C-VX-24.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-24.02 — Resolve this contract's exact dependencies**
  - Action: VX-11, VX-12, VX-16, File/Media services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-24.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-24.03 — Apps — Creative/rendering family**
  - Action: Paint, Renderer, 3D, zlOS animation.
  - Requires: C-VX-24.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for VX-24.
- [ ] **C-VX-24.04 — Deliver — Creative/rendering family**
  - Action: versioned project state, undo, tools/selection/layers as appropriate, safe import/export, reusable scene/render interface and deterministic demo mode.
  - Requires: C-VX-24.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-24.
- [ ] **C-VX-24.05 — Proof — Creative/rendering family**
  - Action: huge canvas/mesh, decoder failure, save crash, backend reset, undo round-trip, software/GPU differential and project migration.
  - Requires: C-VX-24.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-24.
- [ ] **C-VX-24.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-24. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-24.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-25"></a>
## C-VX-25 — Games conformance suite

**Original requirement:** Games conformance suite

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 459.

### Preserved original contract

**Apps:** all 24 named games.

**Depends on:** VX-18, input, audio and user-data services.

**Deliver:** deterministic seed/replay, pause/resume, save/high score, audio cues,
resource budgets, accessibility alternatives and one shared game-runtime seam.

**Proof:** rule goldens, launch every shipped game, replay hash, background
throttle, input loss, audio loss, crash/restore, quota, close teardown and the
repaired Maze route.

### Execution steps

- [ ] **C-VX-25.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-25.02 — Resolve this contract's exact dependencies**
  - Action: VX-18, input, audio and user-data services. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-25.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-25.03 — Apps — Games conformance suite**
  - Action: all 24 named games.
  - Requires: C-VX-25.02.
  - Acceptance: Satisfy every obligation in the preserved Apps field for VX-25.
- [ ] **C-VX-25.04 — Deliver — Games conformance suite**
  - Action: deterministic seed/replay, pause/resume, save/high score, audio cues, resource budgets, accessibility alternatives and one shared game-runtime seam.
  - Requires: C-VX-25.03.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-25.
- [ ] **C-VX-25.05 — Proof — Games conformance suite**
  - Action: rule goldens, launch every shipped game, replay hash, background throttle, input loss, audio loss, crash/restore, quota, close teardown and the repaired Maze route.
  - Requires: C-VX-25.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-25.
- [ ] **C-VX-25.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-25. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-25.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-input-008"></a>
## T-INPUT-008 — USB gamepad/joystick

**Original requirement:** axes/buttons/hats/deadzones/remap

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 162.

### Execution steps

- [ ] **T-INPUT-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve USB gamepad/joystick to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-008.02 — Specify the complete target boundary**
  - Action: USB gamepad/joystick must supply: axes/buttons/hats/deadzones/remap. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse USB gamepad/joystick through the shared platform contract, delivering every part of: axes/buttons/hats/deadzones/remap. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-008.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for USB gamepad/joystick.
- [ ] **T-INPUT-008.05 — Qualify and retain this target's own result**
  - Action: Bind USB gamepad/joystick to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-001"></a>
## T-CUR-001 — Terminal

**Original requirement:** PTY-backed process terminal and shell client

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 30.

### Execution steps

- [ ] **T-CUR-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Terminal to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-001.02 — Specify the complete target boundary**
  - Action: Terminal must supply: PTY-backed process terminal and shell client. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse Terminal through the shared platform contract, delivering every part of: PTY-backed process terminal and shell client. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-001.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Terminal.
- [ ] **T-CUR-001.05 — Qualify and retain this target's own result**
  - Action: Bind Terminal to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-002"></a>
## T-CUR-002 — System Monitor

**Original requirement:** typed telemetry, process/service/device controls

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 31.

### Execution steps

- [ ] **T-CUR-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve System Monitor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-002.02 — Specify the complete target boundary**
  - Action: System Monitor must supply: typed telemetry, process/service/device controls. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse System Monitor through the shared platform contract, delivering every part of: typed telemetry, process/service/device controls. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-002.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for System Monitor.
- [ ] **T-CUR-002.05 — Qualify and retain this target's own result**
  - Action: Bind System Monitor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-003"></a>
## T-CUR-003 — About

**Original requirement:** immutable build, provenance, licenses and health

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 32.

### Execution steps

- [ ] **T-CUR-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve About to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-003.02 — Specify the complete target boundary**
  - Action: About must supply: immutable build, provenance, licenses and health. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse About through the shared platform contract, delivering every part of: immutable build, provenance, licenses and health. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-003.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for About.
- [ ] **T-CUR-003.05 — Qualify and retain this target's own result**
  - Action: Bind About to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-004"></a>
## T-CUR-004 — Menu

**Original requirement:** shell-owned generated admitted-app menu

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 33.

### Execution steps

- [ ] **T-CUR-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Menu to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-004.02 — Specify the complete target boundary**
  - Action: Menu must supply: shell-owned generated admitted-app menu. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse Menu through the shared platform contract, delivering every part of: shell-owned generated admitted-app menu. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-004.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Menu.
- [ ] **T-CUR-004.05 — Qualify and retain this target's own result**
  - Action: Bind Menu to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-006"></a>
## T-CUR-006 — Settings

**Original requirement:** schema-driven per-user/system settings and rollback

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 35.

### Execution steps

- [ ] **T-CUR-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Settings to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-006.02 — Specify the complete target boundary**
  - Action: Settings must supply: schema-driven per-user/system settings and rollback. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse Settings through the shared platform contract, delivering every part of: schema-driven per-user/system settings and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-006.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Settings.
- [ ] **T-CUR-006.05 — Qualify and retain this target's own result**
  - Action: Bind Settings to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-007"></a>
## T-CUR-007 — Run

**Original requirement:** bounded parser/resolver with explicit launch authority

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 36.

### Execution steps

- [ ] **T-CUR-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Run to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-007.02 — Specify the complete target boundary**
  - Action: Run must supply: bounded parser/resolver with explicit launch authority. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse Run through the shared platform contract, delivering every part of: bounded parser/resolver with explicit launch authority. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-007.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Run.
- [ ] **T-CUR-007.05 — Qualify and retain this target's own result**
  - Action: Bind Run to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-008"></a>
## T-CUR-008 — All Applications

**Original requirement:** exact generated catalogue; fix Maze/blank-ID false green

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 37.

### Execution steps

- [ ] **T-CUR-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve All Applications to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-008.02 — Specify the complete target boundary**
  - Action: All Applications must supply: exact generated catalogue; fix Maze/blank-ID false green. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse All Applications through the shared platform contract, delivering every part of: exact generated catalogue; fix Maze/blank-ID false green. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-008.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for All Applications.
- [ ] **T-CUR-008.05 — Qualify and retain this target's own result**
  - Action: Bind All Applications to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-009"></a>
## T-CUR-009 — Files

**Original requirement:** VFS/file-portal manager with search/removable/trash

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 38.

### Execution steps

- [ ] **T-CUR-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Files to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-009.02 — Specify the complete target boundary**
  - Action: Files must supply: VFS/file-portal manager with search/removable/trash. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse Files through the shared platform contract, delivering every part of: VFS/file-portal manager with search/removable/trash. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-009.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Files.
- [ ] **T-CUR-009.05 — Qualify and retain this target's own result**
  - Action: Bind Files to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-010"></a>
## T-CUR-010 — Text Editor

**Original requirement:** multi-document text editor with undo/recovery/encoding

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 39.

### Execution steps

- [ ] **T-CUR-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Text Editor to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-010.02 — Specify the complete target boundary**
  - Action: Text Editor must supply: multi-document text editor with undo/recovery/encoding. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Text Editor through the shared platform contract, delivering every part of: multi-document text editor with undo/recovery/encoding. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-010.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Text Editor.
- [ ] **T-CUR-010.05 — Qualify and retain this target's own result**
  - Action: Bind Text Editor to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-039"></a>
## T-CUR-039 — System

**Original requirement:** PRESSWORK system pane backed by typed providers

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 40.

### Execution steps

- [ ] **T-CUR-039.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve System to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-039.02 — Specify the complete target boundary**
  - Action: System must supply: PRESSWORK system pane backed by typed providers. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-039.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-039.03 — Implement the exact target behavior**
  - Action: Implement or reuse System through the shared platform contract, delivering every part of: PRESSWORK system pane backed by typed providers. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-039.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-039.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-039.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for System.
- [ ] **T-CUR-039.05 — Qualify and retain this target's own result**
  - Action: Bind System to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-039.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-040"></a>
## T-CUR-040 — Type

**Original requirement:** PRESSWORK typography pane with persistent accessible settings

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 41.

### Execution steps

- [ ] **T-CUR-040.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Type to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-040.02 — Specify the complete target boundary**
  - Action: Type must supply: PRESSWORK typography pane with persistent accessible settings. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-040.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-040.03 — Implement the exact target behavior**
  - Action: Implement or reuse Type through the shared platform contract, delivering every part of: PRESSWORK typography pane with persistent accessible settings. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-040.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-040.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-040.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Type.
- [ ] **T-CUR-040.05 — Qualify and retain this target's own result**
  - Action: Bind Type to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-040.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-011"></a>
## T-CUR-011 — Paint

**Original requirement:** tools, selection, layers, history, formats and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 47.

### Execution steps

- [ ] **T-CUR-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Paint to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-011.02 — Specify the complete target boundary**
  - Action: Paint must supply: tools, selection, layers, history, formats and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse Paint through the shared platform contract, delivering every part of: tools, selection, layers, history, formats and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-011.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Paint.
- [ ] **T-CUR-011.05 — Qualify and retain this target's own result**
  - Action: Bind Paint to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-012"></a>
## T-CUR-012 — 3D

**Original requirement:** renderer/scene/camera/asset fixture and creation seed

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 48.

### Execution steps

- [ ] **T-CUR-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 3D to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-012.02 — Specify the complete target boundary**
  - Action: 3D must supply: renderer/scene/camera/asset fixture and creation seed. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse 3D through the shared platform contract, delivering every part of: renderer/scene/camera/asset fixture and creation seed. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-012.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 3D.
- [ ] **T-CUR-012.05 — Qualify and retain this target's own result**
  - Action: Bind 3D to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-013"></a>
## T-CUR-013 — zlOS animation

**Original requirement:** motion, branding and visual-regression fixture

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 49.

### Execution steps

- [ ] **T-CUR-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve zlOS animation to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-013.02 — Specify the complete target boundary**
  - Action: zlOS animation must supply: motion, branding and visual-regression fixture. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse zlOS animation through the shared platform contract, delivering every part of: motion, branding and visual-regression fixture. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-013.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for zlOS animation.
- [ ] **T-CUR-013.05 — Qualify and retain this target's own result**
  - Action: Bind zlOS animation to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-014"></a>
## T-CUR-014 — Pointer

**Original requirement:** per-device input/acceleration/latency inspector

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 50.

### Execution steps

- [ ] **T-CUR-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Pointer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-014.02 — Specify the complete target boundary**
  - Action: Pointer must supply: per-device input/acceleration/latency inspector. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse Pointer through the shared platform contract, delivering every part of: per-device input/acceleration/latency inspector. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-014.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Pointer.
- [ ] **T-CUR-014.05 — Qualify and retain this target's own result**
  - Action: Bind Pointer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-015"></a>
## T-CUR-015 — Renderer

**Original requirement:** software/hardware correctness and performance inspector

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 51.

### Execution steps

- [ ] **T-CUR-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Renderer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-015.02 — Specify the complete target boundary**
  - Action: Renderer must supply: software/hardware correctness and performance inspector. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse Renderer through the shared platform contract, delivering every part of: software/hardware correctness and performance inspector. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-015.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Renderer.
- [ ] **T-CUR-015.05 — Qualify and retain this target's own result**
  - Action: Bind Renderer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-016"></a>
## T-CUR-016 — Framebuffer

**Original requirement:** authority-mediated capture/scanout inspector

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 52.

### Execution steps

- [ ] **T-CUR-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Framebuffer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-016.02 — Specify the complete target boundary**
  - Action: Framebuffer must supply: authority-mediated capture/scanout inspector. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse Framebuffer through the shared platform contract, delivering every part of: authority-mediated capture/scanout inspector. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-016.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Framebuffer.
- [ ] **T-CUR-016.05 — Qualify and retain this target's own result**
  - Action: Bind Framebuffer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-017"></a>
## T-CUR-017 — Font Atlas

**Original requirement:** glyph coverage/shaping/fallback/metrics inspector

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 53.

### Execution steps

- [ ] **T-CUR-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Font Atlas to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-017.02 — Specify the complete target boundary**
  - Action: Font Atlas must supply: glyph coverage/shaping/fallback/metrics inspector. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse Font Atlas through the shared platform contract, delivering every part of: glyph coverage/shaping/fallback/metrics inspector. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-017.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Font Atlas.
- [ ] **T-CUR-017.05 — Qualify and retain this target's own result**
  - Action: Bind Font Atlas to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-018"></a>
## T-CUR-018 — Image Viewer

**Original requirement:** isolated decode, zoom/pan/rotate/metadata/color

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 54.

### Execution steps

- [ ] **T-CUR-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Image Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-018.02 — Specify the complete target boundary**
  - Action: Image Viewer must supply: isolated decode, zoom/pan/rotate/metadata/color. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse Image Viewer through the shared platform contract, delivering every part of: isolated decode, zoom/pan/rotate/metadata/color. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-018.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Image Viewer.
- [ ] **T-CUR-018.05 — Qualify and retain this target's own result**
  - Action: Bind Image Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-019"></a>
## T-CUR-019 — Colour Picker

**Original requirement:** screen-pick portal, palettes, formats and contrast

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 55.

### Execution steps

- [ ] **T-CUR-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Colour Picker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-019.02 — Specify the complete target boundary**
  - Action: Colour Picker must supply: screen-pick portal, palettes, formats and contrast. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse Colour Picker through the shared platform contract, delivering every part of: screen-pick portal, palettes, formats and contrast. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-019.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Colour Picker.
- [ ] **T-CUR-019.05 — Qualify and retain this target's own result**
  - Action: Bind Colour Picker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-020"></a>
## T-CUR-020 — Clipboard

**Original requirement:** session MIME offers/history/privacy broker client

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 61.

### Execution steps

- [ ] **T-CUR-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Clipboard to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-020.02 — Specify the complete target boundary**
  - Action: Clipboard must supply: session MIME offers/history/privacy broker client. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse Clipboard through the shared platform contract, delivering every part of: session MIME offers/history/privacy broker client. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-020.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Clipboard.
- [ ] **T-CUR-020.05 — Qualify and retain this target's own result**
  - Action: Bind Clipboard to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-021"></a>
## T-CUR-021 — Regex Tester

**Original requirement:** bounded dialect-labelled pattern workbench

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 62.

### Execution steps

- [ ] **T-CUR-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Regex Tester to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-021.02 — Specify the complete target boundary**
  - Action: Regex Tester must supply: bounded dialect-labelled pattern workbench. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse Regex Tester through the shared platform contract, delivering every part of: bounded dialect-labelled pattern workbench. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-021.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Regex Tester.
- [ ] **T-CUR-021.05 — Qualify and retain this target's own result**
  - Action: Bind Regex Tester to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-022"></a>
## T-CUR-022 — Base Converter

**Original requirement:** exact-width/arbitrary-precision conversion and errors

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 63.

### Execution steps

- [ ] **T-CUR-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Base Converter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-022.02 — Specify the complete target boundary**
  - Action: Base Converter must supply: exact-width/arbitrary-precision conversion and errors. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse Base Converter through the shared platform contract, delivering every part of: exact-width/arbitrary-precision conversion and errors. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-022.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Base Converter.
- [ ] **T-CUR-022.05 — Qualify and retain this target's own result**
  - Action: Bind Base Converter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-023"></a>
## T-CUR-023 — Text Diff

**Original requirement:** scalable text/file compare and merge modes

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 64.

### Execution steps

- [ ] **T-CUR-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Text Diff to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-023.02 — Specify the complete target boundary**
  - Action: Text Diff must supply: scalable text/file compare and merge modes. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse Text Diff through the shared platform contract, delivering every part of: scalable text/file compare and merge modes. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-023.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Text Diff.
- [ ] **T-CUR-023.05 — Qualify and retain this target's own result**
  - Action: Bind Text Diff to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-024"></a>
## T-CUR-024 — Checksum

**Original requirement:** streaming hash and verification workflow

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 65.

### Execution steps

- [ ] **T-CUR-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Checksum to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-024.02 — Specify the complete target boundary**
  - Action: Checksum must supply: streaming hash and verification workflow. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse Checksum through the shared platform contract, delivering every part of: streaming hash and verification workflow. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-024.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Checksum.
- [ ] **T-CUR-024.05 — Qualify and retain this target's own result**
  - Action: Bind Checksum to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-025"></a>
## T-CUR-025 — Unit Converter

**Original requirement:** typed units, precision, locale, favorites/history

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 66.

### Execution steps

- [ ] **T-CUR-025.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Unit Converter to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-025.02 — Specify the complete target boundary**
  - Action: Unit Converter must supply: typed units, precision, locale, favorites/history. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-025.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-025.03 — Implement the exact target behavior**
  - Action: Implement or reuse Unit Converter through the shared platform contract, delivering every part of: typed units, precision, locale, favorites/history. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-025.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-025.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-025.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Unit Converter.
- [ ] **T-CUR-025.05 — Qualify and retain this target's own result**
  - Action: Bind Unit Converter to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-025.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-026"></a>
## T-CUR-026 — Sticky Notes

**Original requirement:** per-user durable private notes and recovery

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 67.

### Execution steps

- [ ] **T-CUR-026.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Sticky Notes to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-026.02 — Specify the complete target boundary**
  - Action: Sticky Notes must supply: per-user durable private notes and recovery. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-026.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-026.03 — Implement the exact target behavior**
  - Action: Implement or reuse Sticky Notes through the shared platform contract, delivering every part of: per-user durable private notes and recovery. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-026.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-026.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-026.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Sticky Notes.
- [ ] **T-CUR-026.05 — Qualify and retain this target's own result**
  - Action: Bind Sticky Notes to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-026.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-027"></a>
## T-CUR-027 — Keyboard Tester

**Original requirement:** physical/logical keys, layouts/modifiers/repeat

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 68.

### Execution steps

- [ ] **T-CUR-027.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Keyboard Tester to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-027.02 — Specify the complete target boundary**
  - Action: Keyboard Tester must supply: physical/logical keys, layouts/modifiers/repeat. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-027.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-027.03 — Implement the exact target behavior**
  - Action: Implement or reuse Keyboard Tester through the shared platform contract, delivering every part of: physical/logical keys, layouts/modifiers/repeat. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-027.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-027.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-027.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Keyboard Tester.
- [ ] **T-CUR-027.05 — Qualify and retain this target's own result**
  - Action: Bind Keyboard Tester to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-027.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-028"></a>
## T-CUR-028 — Benchmark

**Original requirement:** artifact/workload/backend-bound comparable receipts

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 69.

### Execution steps

- [ ] **T-CUR-028.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Benchmark to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-028.02 — Specify the complete target boundary**
  - Action: Benchmark must supply: artifact/workload/backend-bound comparable receipts. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-028.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-028.03 — Implement the exact target behavior**
  - Action: Implement or reuse Benchmark through the shared platform contract, delivering every part of: artifact/workload/backend-bound comparable receipts. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-028.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-028.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-028.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Benchmark.
- [ ] **T-CUR-028.05 — Qualify and retain this target's own result**
  - Action: Bind Benchmark to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-028.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-029"></a>
## T-CUR-029 — Calculator

**Original requirement:** expression history, precision, keyboard and modes

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 70.

### Execution steps

- [ ] **T-CUR-029.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Calculator to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-029.02 — Specify the complete target boundary**
  - Action: Calculator must supply: expression history, precision, keyboard and modes. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-029.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-029.03 — Implement the exact target behavior**
  - Action: Implement or reuse Calculator through the shared platform contract, delivering every part of: expression history, precision, keyboard and modes. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-029.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-029.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-029.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Calculator.
- [ ] **T-CUR-029.05 — Qualify and retain this target's own result**
  - Action: Bind Calculator to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-029.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-030"></a>
## T-CUR-030 — Clocks & Timers

**Original requirement:** clocks, stopwatch, countdown and background alarms

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 71.

### Execution steps

- [ ] **T-CUR-030.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Clocks & Timers to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-030.02 — Specify the complete target boundary**
  - Action: Clocks & Timers must supply: clocks, stopwatch, countdown and background alarms. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-030.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-030.03 — Implement the exact target behavior**
  - Action: Implement or reuse Clocks & Timers through the shared platform contract, delivering every part of: clocks, stopwatch, countdown and background alarms. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-030.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-030.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-030.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Clocks & Timers.
- [ ] **T-CUR-030.05 — Qualify and retain this target's own result**
  - Action: Bind Clocks & Timers to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-030.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-031"></a>
## T-CUR-031 — System Info

**Original requirement:** consolidated read-only hardware/software inventory

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 77.

### Execution steps

- [ ] **T-CUR-031.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve System Info to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-031.02 — Specify the complete target boundary**
  - Action: System Info must supply: consolidated read-only hardware/software inventory. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-031.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-031.03 — Implement the exact target behavior**
  - Action: Implement or reuse System Info through the shared platform contract, delivering every part of: consolidated read-only hardware/software inventory. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-031.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-031.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-031.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for System Info.
- [ ] **T-CUR-031.05 — Qualify and retain this target's own result**
  - Action: Bind System Info to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-031.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-032"></a>
## T-CUR-032 — Kernel Log

**Original requirement:** filtered/redacted persistent event stream

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 78.

### Execution steps

- [ ] **T-CUR-032.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Kernel Log to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-032.02 — Specify the complete target boundary**
  - Action: Kernel Log must supply: filtered/redacted persistent event stream. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-032.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-032.03 — Implement the exact target behavior**
  - Action: Implement or reuse Kernel Log through the shared platform contract, delivering every part of: filtered/redacted persistent event stream. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-032.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-032.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-032.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Kernel Log.
- [ ] **T-CUR-032.05 — Qualify and retain this target's own result**
  - Action: Bind Kernel Log to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-032.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-033"></a>
## T-CUR-033 — Hex Viewer

**Original requirement:** huge-file paging/search, read-only default, guarded edit

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 79.

### Execution steps

- [ ] **T-CUR-033.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Hex Viewer to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-033.02 — Specify the complete target boundary**
  - Action: Hex Viewer must supply: huge-file paging/search, read-only default, guarded edit. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-033.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-033.03 — Implement the exact target behavior**
  - Action: Implement or reuse Hex Viewer through the shared platform contract, delivering every part of: huge-file paging/search, read-only default, guarded edit. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-033.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-033.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-033.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Hex Viewer.
- [ ] **T-CUR-033.05 — Qualify and retain this target's own result**
  - Action: Bind Hex Viewer to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-033.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-034"></a>
## T-CUR-034 — Console (tty1)

**Original requirement:** low-level recovery console, explicitly distinct from Terminal

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 80.

### Execution steps

- [ ] **T-CUR-034.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Console (tty1) to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-034.02 — Specify the complete target boundary**
  - Action: Console (tty1) must supply: low-level recovery console, explicitly distinct from Terminal. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-034.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-034.03 — Implement the exact target behavior**
  - Action: Implement or reuse Console (tty1) through the shared platform contract, delivering every part of: low-level recovery console, explicitly distinct from Terminal. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-034.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-034.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-034.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Console (tty1).
- [ ] **T-CUR-034.05 — Qualify and retain this target's own result**
  - Action: Bind Console (tty1) to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-034.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-035"></a>
## T-CUR-035 — Disk Usage

**Original requirement:** snapshot/cancellable storage analysis and permissions

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 81.

### Execution steps

- [ ] **T-CUR-035.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Disk Usage to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-035.02 — Specify the complete target boundary**
  - Action: Disk Usage must supply: snapshot/cancellable storage analysis and permissions. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-035.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-035.03 — Implement the exact target behavior**
  - Action: Implement or reuse Disk Usage through the shared platform contract, delivering every part of: snapshot/cancellable storage analysis and permissions. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-035.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-035.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-035.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Disk Usage.
- [ ] **T-CUR-035.05 — Qualify and retain this target's own result**
  - Action: Bind Disk Usage to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-035.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-036"></a>
## T-CUR-036 — Services

**Original requirement:** real supervisor state, health and authorized actions

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 82.

### Execution steps

- [ ] **T-CUR-036.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Services to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-036.02 — Specify the complete target boundary**
  - Action: Services must supply: real supervisor state, health and authorized actions. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-036.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-036.03 — Implement the exact target behavior**
  - Action: Implement or reuse Services through the shared platform contract, delivering every part of: real supervisor state, health and authorized actions. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-036.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-036.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-036.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Services.
- [ ] **T-CUR-036.05 — Qualify and retain this target's own result**
  - Action: Bind Services to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-036.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-037"></a>
## T-CUR-037 — Archive Manager

**Original requirement:** isolated parser and transactional extraction/creation

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 83.

### Execution steps

- [ ] **T-CUR-037.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Archive Manager to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-037.02 — Specify the complete target boundary**
  - Action: Archive Manager must supply: isolated parser and transactional extraction/creation. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-037.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-037.03 — Implement the exact target behavior**
  - Action: Implement or reuse Archive Manager through the shared platform contract, delivering every part of: isolated parser and transactional extraction/creation. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-037.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-037.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-037.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Archive Manager.
- [ ] **T-CUR-037.05 — Qualify and retain this target's own result**
  - Action: Bind Archive Manager to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-037.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-cur-038"></a>
## T-CUR-038 — Network

**Original requirement:** interfaces, routes, addresses, diagnostics and radios

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 84.

### Execution steps

- [ ] **T-CUR-038.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Network to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-CUR-038.02 — Specify the complete target boundary**
  - Action: Network must supply: interfaces, routes, addresses, diagnostics and radios. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-CUR-038.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-CUR-038.03 — Implement the exact target behavior**
  - Action: Implement or reuse Network through the shared platform contract, delivering every part of: interfaces, routes, addresses, diagnostics and radios. Do not fork a duplicate subsystem for this row.
  - Requires: T-CUR-038.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-CUR-038.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: bad or incompatible manifest; missing launch route; resource exhaustion; portal denial; cancelled request; crash-loop; service restart; interrupted update/uninstall; restore stale identity.
  - Requires: T-CUR-038.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Network.
- [ ] **T-CUR-038.05 — Qualify and retain this target's own result**
  - Action: Bind Network to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-CUR-038.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-001"></a>
## T-GAME-001 — Snake

**Original requirement:** grid, timing, keyboard, deterministic replay

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 94.

### Execution steps

- [ ] **T-GAME-001.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Snake to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-001.02 — Specify the complete target boundary**
  - Action: Snake must supply: grid, timing, keyboard, deterministic replay. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-001.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-001.03 — Implement the exact target behavior**
  - Action: Implement or reuse Snake through the shared platform contract, delivering every part of: grid, timing, keyboard, deterministic replay. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-001.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-001.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-001.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Snake.
- [ ] **T-GAME-001.05 — Qualify and retain this target's own result**
  - Action: Bind Snake to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-001.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-002"></a>
## T-GAME-002 — Word Guess

**Original requirement:** text input, localization and state

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 95.

### Execution steps

- [ ] **T-GAME-002.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Word Guess to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-002.02 — Specify the complete target boundary**
  - Action: Word Guess must supply: text input, localization and state. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-002.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-002.03 — Implement the exact target behavior**
  - Action: Implement or reuse Word Guess through the shared platform contract, delivering every part of: text input, localization and state. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-002.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-002.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-002.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Word Guess.
- [ ] **T-GAME-002.05 — Qualify and retain this target's own result**
  - Action: Bind Word Guess to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-002.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-003"></a>
## T-GAME-003 — Tic-Tac-Toe

**Original requirement:** pointer/keyboard focus and rules

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 96.

### Execution steps

- [ ] **T-GAME-003.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Tic-Tac-Toe to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-003.02 — Specify the complete target boundary**
  - Action: Tic-Tac-Toe must supply: pointer/keyboard focus and rules. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-003.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-003.03 — Implement the exact target behavior**
  - Action: Implement or reuse Tic-Tac-Toe through the shared platform contract, delivering every part of: pointer/keyboard focus and rules. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-003.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-003.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-003.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Tic-Tac-Toe.
- [ ] **T-GAME-003.05 — Qualify and retain this target's own result**
  - Action: Bind Tic-Tac-Toe to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-003.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-004"></a>
## T-GAME-004 — Nim

**Original requirement:** deterministic rules and accessibility

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 97.

### Execution steps

- [ ] **T-GAME-004.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Nim to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-004.02 — Specify the complete target boundary**
  - Action: Nim must supply: deterministic rules and accessibility. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-004.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-004.03 — Implement the exact target behavior**
  - Action: Implement or reuse Nim through the shared platform contract, delivering every part of: deterministic rules and accessibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-004.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-004.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-004.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Nim.
- [ ] **T-GAME-004.05 — Qualify and retain this target's own result**
  - Action: Bind Nim to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-004.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-005"></a>
## T-GAME-005 — Tower of Hanoi

**Original requirement:** drag/keyboard, animation and undo

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 98.

### Execution steps

- [ ] **T-GAME-005.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Tower of Hanoi to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-005.02 — Specify the complete target boundary**
  - Action: Tower of Hanoi must supply: drag/keyboard, animation and undo. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-005.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-005.03 — Implement the exact target behavior**
  - Action: Implement or reuse Tower of Hanoi through the shared platform contract, delivering every part of: drag/keyboard, animation and undo. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-005.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-005.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-005.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Tower of Hanoi.
- [ ] **T-GAME-005.05 — Qualify and retain this target's own result**
  - Action: Bind Tower of Hanoi to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-005.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-006"></a>
## T-GAME-006 — Lights Out

**Original requirement:** grid input, contrast and replay

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 99.

### Execution steps

- [ ] **T-GAME-006.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Lights Out to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-006.02 — Specify the complete target boundary**
  - Action: Lights Out must supply: grid input, contrast and replay. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-006.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-006.03 — Implement the exact target behavior**
  - Action: Implement or reuse Lights Out through the shared platform contract, delivering every part of: grid input, contrast and replay. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-006.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-006.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-006.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Lights Out.
- [ ] **T-GAME-006.05 — Qualify and retain this target's own result**
  - Action: Bind Lights Out to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-006.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-007"></a>
## T-GAME-007 — Connect Four

**Original requirement:** board, turn state and animations

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 100.

### Execution steps

- [ ] **T-GAME-007.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Connect Four to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-007.02 — Specify the complete target boundary**
  - Action: Connect Four must supply: board, turn state and animations. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-007.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-007.03 — Implement the exact target behavior**
  - Action: Implement or reuse Connect Four through the shared platform contract, delivering every part of: board, turn state and animations. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-007.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-007.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-007.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Connect Four.
- [ ] **T-GAME-007.05 — Qualify and retain this target's own result**
  - Action: Bind Connect Four to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-007.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-008"></a>
## T-GAME-008 — Maze

**Original requirement:** repair launch route; generation/pathfinding/input

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 101.

### Execution steps

- [ ] **T-GAME-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Maze to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-008.02 — Specify the complete target boundary**
  - Action: Maze must supply: repair launch route; generation/pathfinding/input. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse Maze through the shared platform contract, delivering every part of: repair launch route; generation/pathfinding/input. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-008.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Maze.
- [ ] **T-GAME-008.05 — Qualify and retain this target's own result**
  - Action: Bind Maze to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-009"></a>
## T-GAME-009 — Tetris

**Original requirement:** frame pacing, repeat, collision and persistence

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 102.

### Execution steps

- [ ] **T-GAME-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Tetris to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-009.02 — Specify the complete target boundary**
  - Action: Tetris must supply: frame pacing, repeat, collision and persistence. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse Tetris through the shared platform contract, delivering every part of: frame pacing, repeat, collision and persistence. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-009.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Tetris.
- [ ] **T-GAME-009.05 — Qualify and retain this target's own result**
  - Action: Bind Tetris to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-010"></a>
## T-GAME-010 — Pong

**Original requirement:** continuous input, collision and audio timing

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 103.

### Execution steps

- [ ] **T-GAME-010.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Pong to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-010.02 — Specify the complete target boundary**
  - Action: Pong must supply: continuous input, collision and audio timing. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-010.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-010.03 — Implement the exact target behavior**
  - Action: Implement or reuse Pong through the shared platform contract, delivering every part of: continuous input, collision and audio timing. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-010.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-010.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-010.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Pong.
- [ ] **T-GAME-010.05 — Qualify and retain this target's own result**
  - Action: Bind Pong to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-010.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-011"></a>
## T-GAME-011 — Breakout

**Original requirement:** damage, collision, particles and audio

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 104.

### Execution steps

- [ ] **T-GAME-011.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Breakout to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-011.02 — Specify the complete target boundary**
  - Action: Breakout must supply: damage, collision, particles and audio. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-011.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-011.03 — Implement the exact target behavior**
  - Action: Implement or reuse Breakout through the shared platform contract, delivering every part of: damage, collision, particles and audio. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-011.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-011.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-011.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Breakout.
- [ ] **T-GAME-011.05 — Qualify and retain this target's own result**
  - Action: Bind Breakout to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-011.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-012"></a>
## T-GAME-012 — Minesweeper

**Original requirement:** pointer/keyboard, grids and state restore

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 105.

### Execution steps

- [ ] **T-GAME-012.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Minesweeper to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-012.02 — Specify the complete target boundary**
  - Action: Minesweeper must supply: pointer/keyboard, grids and state restore. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-012.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-012.03 — Implement the exact target behavior**
  - Action: Implement or reuse Minesweeper through the shared platform contract, delivering every part of: pointer/keyboard, grids and state restore. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-012.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-012.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-012.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Minesweeper.
- [ ] **T-GAME-012.05 — Qualify and retain this target's own result**
  - Action: Bind Minesweeper to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-012.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-013"></a>
## T-GAME-013 — 2048

**Original requirement:** gestures/keyboard, animation and undo

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 106.

### Execution steps

- [ ] **T-GAME-013.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 2048 to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-013.02 — Specify the complete target boundary**
  - Action: 2048 must supply: gestures/keyboard, animation and undo. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-013.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-013.03 — Implement the exact target behavior**
  - Action: Implement or reuse 2048 through the shared platform contract, delivering every part of: gestures/keyboard, animation and undo. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-013.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-013.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-013.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 2048.
- [ ] **T-GAME-013.05 — Qualify and retain this target's own result**
  - Action: Bind 2048 to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-013.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-014"></a>
## T-GAME-014 — Conway's Life

**Original requirement:** large grid, simulation rate and pause

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 107.

### Execution steps

- [ ] **T-GAME-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Conway's Life to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-014.02 — Specify the complete target boundary**
  - Action: Conway's Life must supply: large grid, simulation rate and pause. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse Conway's Life through the shared platform contract, delivering every part of: large grid, simulation rate and pause. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-014.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Conway's Life.
- [ ] **T-GAME-014.05 — Qualify and retain this target's own result**
  - Action: Bind Conway's Life to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-015"></a>
## T-GAME-015 — Asteroids

**Original requirement:** vector rendering, rotation, collision and sound

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 108.

### Execution steps

- [ ] **T-GAME-015.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Asteroids to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-015.02 — Specify the complete target boundary**
  - Action: Asteroids must supply: vector rendering, rotation, collision and sound. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-015.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-015.03 — Implement the exact target behavior**
  - Action: Implement or reuse Asteroids through the shared platform contract, delivering every part of: vector rendering, rotation, collision and sound. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-015.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-015.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-015.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Asteroids.
- [ ] **T-GAME-015.05 — Qualify and retain this target's own result**
  - Action: Bind Asteroids to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-015.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-016"></a>
## T-GAME-016 — Invaders

**Original requirement:** sprites, pacing, collision and state

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 109.

### Execution steps

- [ ] **T-GAME-016.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Invaders to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-016.02 — Specify the complete target boundary**
  - Action: Invaders must supply: sprites, pacing, collision and state. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-016.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-016.03 — Implement the exact target behavior**
  - Action: Implement or reuse Invaders through the shared platform contract, delivering every part of: sprites, pacing, collision and state. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-016.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-016.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-016.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Invaders.
- [ ] **T-GAME-016.05 — Qualify and retain this target's own result**
  - Action: Bind Invaders to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-016.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-017"></a>
## T-GAME-017 — 15 Puzzle

**Original requirement:** grid, keyboard and solvable generation

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 110.

### Execution steps

- [ ] **T-GAME-017.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve 15 Puzzle to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-017.02 — Specify the complete target boundary**
  - Action: 15 Puzzle must supply: grid, keyboard and solvable generation. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-017.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-017.03 — Implement the exact target behavior**
  - Action: Implement or reuse 15 Puzzle through the shared platform contract, delivering every part of: grid, keyboard and solvable generation. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-017.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-017.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-017.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for 15 Puzzle.
- [ ] **T-GAME-017.05 — Qualify and retain this target's own result**
  - Action: Bind 15 Puzzle to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-017.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-018"></a>
## T-GAME-018 — Reversi

**Original requirement:** board rules and keyboard accessibility

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 111.

### Execution steps

- [ ] **T-GAME-018.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Reversi to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-018.02 — Specify the complete target boundary**
  - Action: Reversi must supply: board rules and keyboard accessibility. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-018.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-018.03 — Implement the exact target behavior**
  - Action: Implement or reuse Reversi through the shared platform contract, delivering every part of: board rules and keyboard accessibility. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-018.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-018.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-018.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Reversi.
- [ ] **T-GAME-018.05 — Qualify and retain this target's own result**
  - Action: Bind Reversi to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-018.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-019"></a>
## T-GAME-019 — Simon

**Original requirement:** audio/visual cues, timing and reduced-motion mode

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 112.

### Execution steps

- [ ] **T-GAME-019.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Simon to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-019.02 — Specify the complete target boundary**
  - Action: Simon must supply: audio/visual cues, timing and reduced-motion mode. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-019.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-019.03 — Implement the exact target behavior**
  - Action: Implement or reuse Simon through the shared platform contract, delivering every part of: audio/visual cues, timing and reduced-motion mode. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-019.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-019.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-019.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Simon.
- [ ] **T-GAME-019.05 — Qualify and retain this target's own result**
  - Action: Bind Simon to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-019.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-020"></a>
## T-GAME-020 — Sokoban

**Original requirement:** level data, undo and persistence

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 113.

### Execution steps

- [ ] **T-GAME-020.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Sokoban to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-020.02 — Specify the complete target boundary**
  - Action: Sokoban must supply: level data, undo and persistence. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-020.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-020.03 — Implement the exact target behavior**
  - Action: Implement or reuse Sokoban through the shared platform contract, delivering every part of: level data, undo and persistence. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-020.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-020.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-020.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Sokoban.
- [ ] **T-GAME-020.05 — Qualify and retain this target's own result**
  - Action: Bind Sokoban to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-020.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-021"></a>
## T-GAME-021 — Flappy

**Original requirement:** low-latency input, physics and replay

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 114.

### Execution steps

- [ ] **T-GAME-021.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Flappy to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-021.02 — Specify the complete target boundary**
  - Action: Flappy must supply: low-latency input, physics and replay. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-021.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-021.03 — Implement the exact target behavior**
  - Action: Implement or reuse Flappy through the shared platform contract, delivering every part of: low-latency input, physics and replay. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-021.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-021.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-021.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Flappy.
- [ ] **T-GAME-021.05 — Qualify and retain this target's own result**
  - Action: Bind Flappy to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-021.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-022"></a>
## T-GAME-022 — Missile Command

**Original requirement:** pointer, multiple objects and audio

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 115.

### Execution steps

- [ ] **T-GAME-022.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Missile Command to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-022.02 — Specify the complete target boundary**
  - Action: Missile Command must supply: pointer, multiple objects and audio. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-022.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-022.03 — Implement the exact target behavior**
  - Action: Implement or reuse Missile Command through the shared platform contract, delivering every part of: pointer, multiple objects and audio. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-022.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-022.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-022.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Missile Command.
- [ ] **T-GAME-022.05 — Qualify and retain this target's own result**
  - Action: Bind Missile Command to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-022.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-023"></a>
## T-GAME-023 — Blackjack

**Original requirement:** cards, deterministic randomness and state

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 116.

### Execution steps

- [ ] **T-GAME-023.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Blackjack to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-023.02 — Specify the complete target boundary**
  - Action: Blackjack must supply: cards, deterministic randomness and state. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-023.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-023.03 — Implement the exact target behavior**
  - Action: Implement or reuse Blackjack through the shared platform contract, delivering every part of: cards, deterministic randomness and state. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-023.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-023.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-023.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Blackjack.
- [ ] **T-GAME-023.05 — Qualify and retain this target's own result**
  - Action: Bind Blackjack to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-023.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-game-024"></a>
## T-GAME-024 — Frogger

**Original requirement:** moving hazards, pacing and collision

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 117.

### Execution steps

- [ ] **T-GAME-024.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Frogger to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-17, H-12.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-GAME-024.02 — Specify the complete target boundary**
  - Action: Frogger must supply: moving hazards, pacing and collision. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-GAME-024.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-GAME-024.03 — Implement the exact target behavior**
  - Action: Implement or reuse Frogger through the shared platform contract, delivering every part of: moving hazards, pacing and collision. Do not fork a duplicate subsystem for this row.
  - Requires: T-GAME-024.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-GAME-024.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: deterministic replay mismatch; focus/pause loss; key-repeat flood; invalid saved state; timing drift; resize; muted/disconnected audio; crash/restart; inaccessible control.
  - Requires: T-GAME-024.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Frogger.
- [ ] **T-GAME-024.05 — Qualify and retain this target's own result**
  - Action: Bind Frogger to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-GAME-024.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
