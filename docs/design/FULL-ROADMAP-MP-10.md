# MP-10: Finish the shared visual, accessibility and language system

Status: full-scope planning proposal, 2026-09-08. Nothing on this page is an implementation-completion claim.

[Master roadmap](FULL-SYSTEM-ROADMAP.md) · [Decisions](FULL-ROADMAP-DECISIONS.md) · [Machine-readable steps](FULL-SYSTEM-ROADMAP.json)

Owner scope to inspect: `kernel/src/graphics/ui/; kernel/src/graphics/fonts/; kernel/apps/; docs/design/`. Paths identify current areas, not invented future files.

## Bounded handoff and full completion

`H-10` exports: Usable semantic primitives, themes, text and input contracts consumed by the app SDK.

The handoff enables only its named subset. `CLOSE-10` requires every complete feature, target and source contract below. Prose dependencies must be bound to concrete providers before implementation. Shared work is implemented once and checked against each consuming contract.

## Ordered subsystem milestones

### M-10.01 — Freeze the selected visual direction and version semantic design tokens, assets and their licenses

Freeze the selected visual direction and version semantic design tokens, assets and their licenses.

**Requires:** `D-01`, `D-02`, `D-14`, `H-00`, `H-04`, `H-07`, `H-09`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.02 — Define every component state, keyboard operation, focus behavior, semantic role and error/recovery representation

Define every component state, keyboard operation, focus behavior, semantic role and error/recovery representation.

**Requires:** `M-10.01`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.03 — Implement atomic themes, scaling, layout constraints and high-contrast/reduced-motion behavior

Implement atomic themes, scaling, layout constraints and high-contrast/reduced-motion behavior.

**Requires:** `M-10.02`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.04 — Build text shaping, font fallback, grapheme handling, bidirectional layout and IME composition with bounded caches

Build text shaping, font fallback, grapheme handling, bidirectional layout and IME composition with bounded caches.

**Requires:** `M-10.03`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.05 — Define and implement the accessibility tree, action protocol, focus events and assistive-provider lifecycle

Define and implement the accessibility tree, action protocol, focus events and assistive-provider lifecycle.

**Requires:** `M-10.04`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### H-10 — Bounded development handoff: Finish the shared visual, accessibility and language system

Usable semantic primitives, themes, text and input contracts consumed by the app SDK.

**Requires:** `M-10.05`.

**Acceptance:** Name the exact exported subset, version, producer/consumer, bounds, failure/cleanup and passing proof. This does not mark the phase or its feature list complete.

### M-10.06 — Add screen-reader/speech/braille seams, magnification, filters, captions and switch/dwell input as separate targets

Add screen-reader/speech/braille seams, magnification, filters, captions and switch/dwell input as separate targets.

**Requires:** `M-10.05`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.07 — Implement locale, number/date/unit/timezone policy and translation catalogue tooling without changing monotonic deadlines

Implement locale, number/date/unit/timezone policy and translation catalogue tooling without changing monotonic deadlines.

**Requires:** `M-10.06`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.08 — Test components across theme/scale/locale/input/assistive states and attach real semantic and interaction oracles

Test components across theme/scale/locale/input/assistive states and attach real semantic and interaction oracles.

**Requires:** `M-10.07`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

### M-10.09 — Join speech/capture providers where needed while allowing non-audio design work to advance on explicit interface fixtures

Join speech/capture providers where needed while allowing non-audio design work to advance on explicit interface fixtures.

**Requires:** `M-10.08`.

**Acceptance:** Retain the concrete outcome and comparison/failure evidence named in this step. A milestone can reuse the matching feature/contract/target implementation and proof; do not implement it twice.

## Package index

| ID | Kind | Required outcome |
|---|---|---|
| [F-DS-001](#f-ds-001) | feature | named visual identity |
| [F-DS-002](#f-ds-002) | feature | semantic color tokens |
| [F-DS-003](#f-ds-003) | feature | typography tokens |
| [F-DS-004](#f-ds-004) | feature | spacing scale |
| [F-DS-005](#f-ds-005) | feature | sizing metrics |
| [F-DS-006](#f-ds-006) | feature | radii/shape grammar |
| [F-DS-007](#f-ds-007) | feature | border grammar |
| [F-DS-008](#f-ds-008) | feature | elevation/shadow grammar |
| [F-DS-009](#f-ds-009) | feature | motion tokens |
| [F-DS-010](#f-ds-010) | feature | icon system |
| [F-DS-011](#f-ds-011) | feature | illustration/wallpaper system |
| [F-DS-012](#f-ds-012) | feature | theme schema |
| [F-DS-013](#f-ds-013) | feature | light theme |
| [F-DS-014](#f-ds-014) | feature | dark theme |
| [F-DS-015](#f-ds-015) | feature | high-contrast themes |
| [F-DS-016](#f-ds-016) | feature | density modes |
| [F-DS-017](#f-ds-017) | feature | live theme preview |
| [F-DS-018](#f-ds-018) | feature | design-token registry |
| [F-DS-019](#f-ds-019) | feature | component registry |
| [F-DS-020](#f-ds-020) | feature | button family |
| [F-DS-021](#f-ds-021) | feature | text field family |
| [F-DS-022](#f-ds-022) | feature | selection controls |
| [F-DS-023](#f-ds-023) | feature | slider/spin control |
| [F-DS-024](#f-ds-024) | feature | menu family |
| [F-DS-025](#f-ds-025) | feature | toolbar family |
| [F-DS-026](#f-ds-026) | feature | tabs family |
| [F-DS-027](#f-ds-027) | feature | list/table/tree family |
| [F-DS-028](#f-ds-028) | feature | scroll container |
| [F-DS-029](#f-ds-029) | feature | dialog/modal family |
| [F-DS-030](#f-ds-030) | feature | toast/notification primitive |
| [F-DS-031](#f-ds-031) | feature | progress family |
| [F-DS-032](#f-ds-032) | feature | empty/error/loading states |
| [F-DS-033](#f-ds-033) | feature | card/panel/split-pane |
| [F-DS-034](#f-ds-034) | feature | window chrome |
| [F-DS-035](#f-ds-035) | feature | data visualization primitives |
| [F-DS-036](#f-ds-036) | feature | component gallery |
| [F-DS-037](#f-ds-037) | feature | declarative layout format |
| [F-DS-038](#f-ds-038) | feature | responsive breakpoints |
| [F-DS-039](#f-ds-039) | feature | content-writing rules |
| [F-DS-040](#f-ds-040) | feature | design-system versioning |
| [F-AX-001](#f-ax-001) | feature | semantic UI tree |
| [F-AX-002](#f-ax-002) | feature | semantic event stream |
| [F-AX-003](#f-ax-003) | feature | screen reader |
| [F-AX-004](#f-ax-004) | feature | braille provider |
| [F-AX-005](#f-ax-005) | feature | keyboard-complete navigation |
| [F-AX-006](#f-ax-006) | feature | focus appearance |
| [F-AX-007](#f-ax-007) | feature | focus order/groups |
| [F-AX-008](#f-ax-008) | feature | shortcuts/access keys |
| [F-AX-009](#f-ax-009) | feature | large text/system scale |
| [F-AX-010](#f-ax-010) | feature | magnifier |
| [F-AX-011](#f-ax-011) | feature | high contrast |
| [F-AX-012](#f-ax-012) | feature | color filters |
| [F-AX-013](#f-ax-013) | feature | contrast checker |
| [F-AX-014](#f-ax-014) | feature | reduced motion |
| [F-AX-015](#f-ax-015) | feature | captions |
| [F-AX-016](#f-ax-016) | feature | audio descriptions |
| [F-AX-017](#f-ax-017) | feature | mono/balance audio |
| [F-AX-018](#f-ax-018) | feature | sticky/filter/slow keys |
| [F-AX-019](#f-ax-019) | feature | mouse keys |
| [F-AX-020](#f-ax-020) | feature | switch control |
| [F-AX-021](#f-ax-021) | feature | dwell control |
| [F-AX-022](#f-ax-022) | feature | voice control |
| [F-AX-023](#f-ax-023) | feature | remapping |
| [F-AX-024](#f-ax-024) | feature | touch target policy |
| [F-AX-025](#f-ax-025) | feature | flashing/seizure safety |
| [F-AX-026](#f-ax-026) | feature | cognitive accessibility |
| [F-AX-027](#f-ax-027) | feature | Accessibility Center |
| [F-AX-028](#f-ax-028) | feature | locale framework |
| [F-AX-029](#f-ax-029) | feature | translation resource system |
| [F-AX-030](#f-ax-030) | feature | Unicode text |
| [F-AX-031](#f-ax-031) | feature | font fallback/shaping |
| [F-AX-032](#f-ax-032) | feature | right-to-left layout |
| [F-AX-033](#f-ax-033) | feature | timezone/calendar formats |
| [F-AX-034](#f-ax-034) | feature | units/number formats |
| [F-AX-035](#f-ax-035) | feature | pseudo-localization gate |
| [C-VX-02](#c-vx-02) | contract | Versioned semantic token schema |
| [C-VX-03](#c-vx-03) | contract | Atomic theme and appearance service |
| [C-VX-04](#c-vx-04) | contract | Scalable text, shaping and glyph cache |
| [C-VX-05](#c-vx-05) | contract | Semantic icon and asset pipeline |
| [C-VX-06](#c-vx-06) | contract | Motion engine and reduced-motion policy |
| [C-VX-07](#c-vx-07) | contract | Accessibility tree, assistive providers and control center |
| [C-VX-08](#c-vx-08) | contract | Primitive component library |
| [C-VX-09](#c-vx-09) | contract | Responsive layout, scale and localization |
| [C-VX-10](#c-vx-10) | contract | Focus, shortcuts and input ownership |
| [T-INPUT-014](#t-input-014) | target | switch/dwell/accessibility devices |
| [T-SVC-077](#t-svc-077) | target | Appearance/Theme Service |
| [T-SVC-078](#t-svc-078) | target | Font/Shaping Service |
| [T-SVC-079](#t-svc-079) | target | Icon/Asset Service |
| [T-SVC-080](#t-svc-080) | target | Localization Service |
| [T-SVC-081](#t-svc-081) | target | IME/Text Input Service |
| [T-SVC-082](#t-svc-082) | target | Accessibility Tree Broker |
| [T-SVC-083](#t-svc-083) | target | Screen Reader/Speech Service |
| [T-SVC-084](#t-svc-084) | target | Magnifier/Visual Assistance |
| [T-SVC-085](#t-svc-085) | target | Captions/Assistive Input Service |
| [T-SVC-086](#t-svc-086) | target | Color/Profile Service |
| [T-SVC-096](#t-svc-096) | target | Text-to-Speech Service |
| [T-APP-008](#t-app-008) | target | Accessibility Center |
| [T-APP-009](#t-app-009) | target | Appearance Center |
| [T-APP-107](#t-app-107) | target | Accessibility Tour |

<a id="f-ds-001"></a>
## F-DS-001 — named visual identity

**Original requirement:** current lime/graphite direction has documented principles and asset provenance

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-001.01 — Reconcile existing named visual identity**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for named visual identity. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: current lime/graphite direction has documented principles and asset provenance
- [ ] **F-DS-001.02 — Freeze the exact contract for named visual identity**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: current lime/graphite direction has documented principles and asset provenance. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-001.03 — Implement/prove: current lime/graphite direction has documented principles and asset provenance**
  - Action: For named visual identity, implement or reuse and verify this exact obligation: current lime/graphite direction has documented principles and asset provenance. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for current lime/graphite direction has documented principles and asset provenance; retain observable state/resource expectations.
- [ ] **F-DS-001.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to named visual identity: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-001.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-001.05 — Integrate into the real consumer and runtime route**
  - Action: Wire named visual identity into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-001.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-001.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for named visual identity as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-001.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-002"></a>
## F-DS-002 — semantic color tokens

**Original requirement:** canvas/surface/text/border/accent/status/selection/focus roles, not app hex copies

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-002.01 — Reconcile existing semantic color tokens**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for semantic color tokens. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: canvas/surface/text/border/accent/status/selection/focus roles, not app hex copies
- [ ] **F-DS-002.02 — Freeze the exact contract for semantic color tokens**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: canvas/surface/text/border/accent/status/selection/focus roles, not app hex copies. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-002.03 — Implement/prove: canvas/surface/text/border/accent/status/selection/focus roles**
  - Action: For semantic color tokens, implement or reuse and verify this exact obligation: canvas/surface/text/border/accent/status/selection/focus roles. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for canvas/surface/text/border/accent/status/selection/focus roles; retain observable state/resource expectations.
- [ ] **F-DS-002.04 — Implement/prove: not app hex copies**
  - Action: For semantic color tokens, implement or reuse and verify this exact obligation: not app hex copies. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-002.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not app hex copies; retain observable state/resource expectations.
- [ ] **F-DS-002.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to semantic color tokens: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-002.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-002.06 — Integrate into the real consumer and runtime route**
  - Action: Wire semantic color tokens into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-002.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-002.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for semantic color tokens as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-002.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-003"></a>
## F-DS-003 — typography tokens

**Original requirement:** display/title/body/label/code/caption roles with size/weight/line-height/fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-003.01 — Reconcile existing typography tokens**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for typography tokens. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: display/title/body/label/code/caption roles with size/weight/line-height/fallback
- [ ] **F-DS-003.02 — Freeze the exact contract for typography tokens**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: display/title/body/label/code/caption roles with size/weight/line-height/fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-003.03 — Implement/prove: display/title/body/label/code/caption roles with size/weight/line-height/fallback**
  - Action: For typography tokens, implement or reuse and verify this exact obligation: display/title/body/label/code/caption roles with size/weight/line-height/fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for display/title/body/label/code/caption roles with size/weight/line-height/fallback; retain observable state/resource expectations.
- [ ] **F-DS-003.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to typography tokens: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-003.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-003.05 — Integrate into the real consumer and runtime route**
  - Action: Wire typography tokens into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-003.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-003.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for typography tokens as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-003.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-004"></a>
## F-DS-004 — spacing scale

**Original requirement:** shared density-aware gaps/padding/insets and alignment rhythm

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-004.01 — Reconcile existing spacing scale**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for spacing scale. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: shared density-aware gaps/padding/insets and alignment rhythm
- [ ] **F-DS-004.02 — Freeze the exact contract for spacing scale**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: shared density-aware gaps/padding/insets and alignment rhythm. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-004.03 — Implement/prove: shared density-aware gaps/padding/insets and alignment rhythm**
  - Action: For spacing scale, implement or reuse and verify this exact obligation: shared density-aware gaps/padding/insets and alignment rhythm. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shared density-aware gaps/padding/insets and alignment rhythm; retain observable state/resource expectations.
- [ ] **F-DS-004.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to spacing scale: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-004.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-004.05 — Integrate into the real consumer and runtime route**
  - Action: Wire spacing scale into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-004.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-004.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for spacing scale as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-004.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-005"></a>
## F-DS-005 — sizing metrics

**Original requirement:** controls, rows, titlebars, panels, hit targets and minimum windows derive from tokens

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-005.01 — Reconcile existing sizing metrics**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sizing metrics. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: controls, rows, titlebars, panels, hit targets and minimum windows derive from tokens
- [ ] **F-DS-005.02 — Freeze the exact contract for sizing metrics**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: controls, rows, titlebars, panels, hit targets and minimum windows derive from tokens. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-005.03 — Implement/prove: controls**
  - Action: For sizing metrics, implement or reuse and verify this exact obligation: controls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controls; retain observable state/resource expectations.
- [ ] **F-DS-005.04 — Implement/prove: rows**
  - Action: For sizing metrics, implement or reuse and verify this exact obligation: rows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-005.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for rows; retain observable state/resource expectations.
- [ ] **F-DS-005.05 — Implement/prove: titlebars**
  - Action: For sizing metrics, implement or reuse and verify this exact obligation: titlebars. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-005.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for titlebars; retain observable state/resource expectations.
- [ ] **F-DS-005.06 — Implement/prove: panels**
  - Action: For sizing metrics, implement or reuse and verify this exact obligation: panels. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-005.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for panels; retain observable state/resource expectations.
- [ ] **F-DS-005.07 — Implement/prove: hit targets and minimum windows derive from tokens**
  - Action: For sizing metrics, implement or reuse and verify this exact obligation: hit targets and minimum windows derive from tokens. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-005.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hit targets and minimum windows derive from tokens; retain observable state/resource expectations.
- [ ] **F-DS-005.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sizing metrics: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-005.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-005.09 — Integrate into the real consumer and runtime route**
  - Action: Wire sizing metrics into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-005.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-005.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sizing metrics as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-005.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-006"></a>
## F-DS-006 — radii/shape grammar

**Original requirement:** consistent corner families, masks, hit tests and scale behavior

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-006.01 — Reconcile existing radii/shape grammar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for radii/shape grammar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: consistent corner families, masks, hit tests and scale behavior
- [ ] **F-DS-006.02 — Freeze the exact contract for radii/shape grammar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: consistent corner families, masks, hit tests and scale behavior. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-006.03 — Implement/prove: consistent corner families**
  - Action: For radii/shape grammar, implement or reuse and verify this exact obligation: consistent corner families. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consistent corner families; retain observable state/resource expectations.
- [ ] **F-DS-006.04 — Implement/prove: masks**
  - Action: For radii/shape grammar, implement or reuse and verify this exact obligation: masks. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-006.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for masks; retain observable state/resource expectations.
- [ ] **F-DS-006.05 — Implement/prove: hit tests and scale behavior**
  - Action: For radii/shape grammar, implement or reuse and verify this exact obligation: hit tests and scale behavior. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-006.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hit tests and scale behavior; retain observable state/resource expectations.
- [ ] **F-DS-006.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to radii/shape grammar: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-006.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-006.07 — Integrate into the real consumer and runtime route**
  - Action: Wire radii/shape grammar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-006.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-006.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for radii/shape grammar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-006.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-007"></a>
## F-DS-007 — border grammar

**Original requirement:** hierarchy/focus/separator/error roles and high-contrast alternatives

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-007.01 — Reconcile existing border grammar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for border grammar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: hierarchy/focus/separator/error roles and high-contrast alternatives
- [ ] **F-DS-007.02 — Freeze the exact contract for border grammar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: hierarchy/focus/separator/error roles and high-contrast alternatives. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-007.03 — Implement/prove: hierarchy/focus/separator/error roles and high-contrast alternatives**
  - Action: For border grammar, implement or reuse and verify this exact obligation: hierarchy/focus/separator/error roles and high-contrast alternatives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hierarchy/focus/separator/error roles and high-contrast alternatives; retain observable state/resource expectations.
- [ ] **F-DS-007.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to border grammar: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-007.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-007.05 — Integrate into the real consumer and runtime route**
  - Action: Wire border grammar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-007.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-007.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for border grammar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-007.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-008"></a>
## F-DS-008 — elevation/shadow grammar

**Original requirement:** semantic depth levels with bounded renderer and opaque fallback

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-008.01 — Reconcile existing elevation/shadow grammar**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for elevation/shadow grammar. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic depth levels with bounded renderer and opaque fallback
- [ ] **F-DS-008.02 — Freeze the exact contract for elevation/shadow grammar**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic depth levels with bounded renderer and opaque fallback. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-008.03 — Implement/prove: semantic depth levels with bounded renderer and opaque fallback**
  - Action: For elevation/shadow grammar, implement or reuse and verify this exact obligation: semantic depth levels with bounded renderer and opaque fallback. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic depth levels with bounded renderer and opaque fallback; retain observable state/resource expectations.
- [ ] **F-DS-008.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to elevation/shadow grammar: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-008.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-008.05 — Integrate into the real consumer and runtime route**
  - Action: Wire elevation/shadow grammar into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-008.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-008.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for elevation/shadow grammar as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-008.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-009"></a>
## F-DS-009 — motion tokens

**Original requirement:** durations/easing/distance/interruptibility/reduced-motion alternatives

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-009.01 — Reconcile existing motion tokens**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for motion tokens. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: durations/easing/distance/interruptibility/reduced-motion alternatives
- [ ] **F-DS-009.02 — Freeze the exact contract for motion tokens**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: durations/easing/distance/interruptibility/reduced-motion alternatives. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-009.03 — Implement/prove: durations/easing/distance/interruptibility/reduced-motion alternatives**
  - Action: For motion tokens, implement or reuse and verify this exact obligation: durations/easing/distance/interruptibility/reduced-motion alternatives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for durations/easing/distance/interruptibility/reduced-motion alternatives; retain observable state/resource expectations.
- [ ] **F-DS-009.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to motion tokens: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-009.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-009.05 — Integrate into the real consumer and runtime route**
  - Action: Wire motion tokens into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-009.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-009.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for motion tokens as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-009.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-010"></a>
## F-DS-010 — icon system

**Original requirement:** grid, stroke/fill, optical size, states, semantic names, fallback and licenses

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-010.01 — Reconcile existing icon system**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for icon system. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: grid, stroke/fill, optical size, states, semantic names, fallback and licenses
- [ ] **F-DS-010.02 — Freeze the exact contract for icon system**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: grid, stroke/fill, optical size, states, semantic names, fallback and licenses. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-010.03 — Implement/prove: grid**
  - Action: For icon system, implement or reuse and verify this exact obligation: grid. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grid; retain observable state/resource expectations.
- [ ] **F-DS-010.04 — Implement/prove: stroke/fill**
  - Action: For icon system, implement or reuse and verify this exact obligation: stroke/fill. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stroke/fill; retain observable state/resource expectations.
- [ ] **F-DS-010.05 — Implement/prove: optical size**
  - Action: For icon system, implement or reuse and verify this exact obligation: optical size. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-010.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for optical size; retain observable state/resource expectations.
- [ ] **F-DS-010.06 — Implement/prove: states**
  - Action: For icon system, implement or reuse and verify this exact obligation: states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-010.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for states; retain observable state/resource expectations.
- [ ] **F-DS-010.07 — Implement/prove: semantic names**
  - Action: For icon system, implement or reuse and verify this exact obligation: semantic names. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-010.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic names; retain observable state/resource expectations.
- [ ] **F-DS-010.08 — Implement/prove: fallback and licenses**
  - Action: For icon system, implement or reuse and verify this exact obligation: fallback and licenses. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-010.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback and licenses; retain observable state/resource expectations.
- [ ] **F-DS-010.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to icon system: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-010.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-010.10 — Integrate into the real consumer and runtime route**
  - Action: Wire icon system into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-010.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-010.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for icon system as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-010.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-011"></a>
## F-DS-011 — illustration/wallpaper system

**Original requirement:** coherent art direction, scale/crop/color, cache and attribution

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-011.01 — Reconcile existing illustration/wallpaper system**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for illustration/wallpaper system. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: coherent art direction, scale/crop/color, cache and attribution
- [ ] **F-DS-011.02 — Freeze the exact contract for illustration/wallpaper system**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: coherent art direction, scale/crop/color, cache and attribution. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-011.03 — Implement/prove: coherent art direction**
  - Action: For illustration/wallpaper system, implement or reuse and verify this exact obligation: coherent art direction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for coherent art direction; retain observable state/resource expectations.
- [ ] **F-DS-011.04 — Implement/prove: scale/crop/color**
  - Action: For illustration/wallpaper system, implement or reuse and verify this exact obligation: scale/crop/color. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-011.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scale/crop/color; retain observable state/resource expectations.
- [ ] **F-DS-011.05 — Implement/prove: cache and attribution**
  - Action: For illustration/wallpaper system, implement or reuse and verify this exact obligation: cache and attribution. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-011.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cache and attribution; retain observable state/resource expectations.
- [ ] **F-DS-011.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to illustration/wallpaper system: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-011.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-011.07 — Integrate into the real consumer and runtime route**
  - Action: Wire illustration/wallpaper system into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-011.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-011.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for illustration/wallpaper system as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-011.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-012"></a>
## F-DS-012 — theme schema

**Original requirement:** versioned required/optional tokens, validation, inheritance and unknown handling

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-012.01 — Reconcile existing theme schema**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for theme schema. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned required/optional tokens, validation, inheritance and unknown handling
- [ ] **F-DS-012.02 — Freeze the exact contract for theme schema**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned required/optional tokens, validation, inheritance and unknown handling. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-012.03 — Implement/prove: versioned required/optional tokens**
  - Action: For theme schema, implement or reuse and verify this exact obligation: versioned required/optional tokens. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned required/optional tokens; retain observable state/resource expectations.
- [ ] **F-DS-012.04 — Implement/prove: validation**
  - Action: For theme schema, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-012.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-DS-012.05 — Implement/prove: inheritance and unknown handling**
  - Action: For theme schema, implement or reuse and verify this exact obligation: inheritance and unknown handling. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-012.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for inheritance and unknown handling; retain observable state/resource expectations.
- [ ] **F-DS-012.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to theme schema: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-012.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-012.07 — Integrate into the real consumer and runtime route**
  - Action: Wire theme schema into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-012.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-012.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for theme schema as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-012.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-013"></a>
## F-DS-013 — light theme

**Original requirement:** complete component/app coverage and contrast proof, not palette inversion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-013.01 — Reconcile existing light theme**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for light theme. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: complete component/app coverage and contrast proof, not palette inversion
- [ ] **F-DS-013.02 — Freeze the exact contract for light theme**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: complete component/app coverage and contrast proof, not palette inversion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-013.03 — Implement/prove: complete component/app coverage and contrast proof**
  - Action: For light theme, implement or reuse and verify this exact obligation: complete component/app coverage and contrast proof. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete component/app coverage and contrast proof; retain observable state/resource expectations.
- [ ] **F-DS-013.04 — Implement/prove: not palette inversion**
  - Action: For light theme, implement or reuse and verify this exact obligation: not palette inversion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for not palette inversion; retain observable state/resource expectations.
- [ ] **F-DS-013.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to light theme: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-013.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-013.06 — Integrate into the real consumer and runtime route**
  - Action: Wire light theme into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-013.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-013.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for light theme as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-013.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-014"></a>
## F-DS-014 — dark theme

**Original requirement:** complete semantic coverage with current identity

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-014.01 — Reconcile existing dark theme**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for dark theme. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: complete semantic coverage with current identity
- [ ] **F-DS-014.02 — Freeze the exact contract for dark theme**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: complete semantic coverage with current identity. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-014.03 — Implement/prove: complete semantic coverage with current identity**
  - Action: For dark theme, implement or reuse and verify this exact obligation: complete semantic coverage with current identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for complete semantic coverage with current identity; retain observable state/resource expectations.
- [ ] **F-DS-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to dark theme: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire dark theme into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for dark theme as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-015"></a>
## F-DS-015 — high-contrast themes

**Original requirement:** non-color state cues and WCAG-like contrast across all primitives

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-015.01 — Reconcile existing high-contrast themes**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for high-contrast themes. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: non-color state cues and WCAG-like contrast across all primitives
- [ ] **F-DS-015.02 — Freeze the exact contract for high-contrast themes**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: non-color state cues and WCAG-like contrast across all primitives. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-015.03 — Implement/prove: non-color state cues and WCAG-like contrast across all primitives**
  - Action: For high-contrast themes, implement or reuse and verify this exact obligation: non-color state cues and WCAG-like contrast across all primitives. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for non-color state cues and WCAG-like contrast across all primitives; retain observable state/resource expectations.
- [ ] **F-DS-015.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to high-contrast themes: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-015.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-015.05 — Integrate into the real consumer and runtime route**
  - Action: Wire high-contrast themes into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-015.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-015.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for high-contrast themes as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-015.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-016"></a>
## F-DS-016 — density modes

**Original requirement:** compact/default/touch geometry without hard-coded app overrides

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-016.01 — Reconcile existing density modes**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for density modes. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: compact/default/touch geometry without hard-coded app overrides
- [ ] **F-DS-016.02 — Freeze the exact contract for density modes**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: compact/default/touch geometry without hard-coded app overrides. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-016.03 — Implement/prove: compact/default/touch geometry without hard-coded app overrides**
  - Action: For density modes, implement or reuse and verify this exact obligation: compact/default/touch geometry without hard-coded app overrides. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compact/default/touch geometry without hard-coded app overrides; retain observable state/resource expectations.
- [ ] **F-DS-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to density modes: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire density modes into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for density modes as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-017"></a>
## F-DS-017 — live theme preview

**Original requirement:** isolated preview, reversible apply, timeout rollback and persistence

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-017.01 — Reconcile existing live theme preview**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for live theme preview. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: isolated preview, reversible apply, timeout rollback and persistence
- [ ] **F-DS-017.02 — Freeze the exact contract for live theme preview**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: isolated preview, reversible apply, timeout rollback and persistence. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-017.03 — Implement/prove: isolated preview**
  - Action: For live theme preview, implement or reuse and verify this exact obligation: isolated preview. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for isolated preview; retain observable state/resource expectations.
- [ ] **F-DS-017.04 — Implement/prove: reversible apply**
  - Action: For live theme preview, implement or reuse and verify this exact obligation: reversible apply. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-017.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reversible apply; retain observable state/resource expectations.
- [ ] **F-DS-017.05 — Implement/prove: timeout rollback and persistence**
  - Action: For live theme preview, implement or reuse and verify this exact obligation: timeout rollback and persistence. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-017.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout rollback and persistence; retain observable state/resource expectations.
- [ ] **F-DS-017.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to live theme preview: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-017.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-017.07 — Integrate into the real consumer and runtime route**
  - Action: Wire live theme preview into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-017.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-017.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for live theme preview as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-017.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-018"></a>
## F-DS-018 — design-token registry

**Original requirement:** generated consumer map finds hard-coded duplicates and missing states

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-018.01 — Reconcile existing design-token registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for design-token registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: generated consumer map finds hard-coded duplicates and missing states
- [ ] **F-DS-018.02 — Freeze the exact contract for design-token registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: generated consumer map finds hard-coded duplicates and missing states. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-018.03 — Implement/prove: generated consumer map finds hard-coded duplicates and missing states**
  - Action: For design-token registry, implement or reuse and verify this exact obligation: generated consumer map finds hard-coded duplicates and missing states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for generated consumer map finds hard-coded duplicates and missing states; retain observable state/resource expectations.
- [ ] **F-DS-018.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to design-token registry: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-018.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-018.05 — Integrate into the real consumer and runtime route**
  - Action: Wire design-token registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-018.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-018.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for design-token registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-018.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-019"></a>
## F-DS-019 — component registry

**Original requirement:** one identity, variants, properties, states, semantics and ownership per primitive

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-019.01 — Reconcile existing component registry**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for component registry. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: one identity, variants, properties, states, semantics and ownership per primitive
- [ ] **F-DS-019.02 — Freeze the exact contract for component registry**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: one identity, variants, properties, states, semantics and ownership per primitive. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-019.03 — Implement/prove: one identity**
  - Action: For component registry, implement or reuse and verify this exact obligation: one identity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for one identity; retain observable state/resource expectations.
- [ ] **F-DS-019.04 — Implement/prove: variants**
  - Action: For component registry, implement or reuse and verify this exact obligation: variants. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for variants; retain observable state/resource expectations.
- [ ] **F-DS-019.05 — Implement/prove: properties**
  - Action: For component registry, implement or reuse and verify this exact obligation: properties. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-019.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for properties; retain observable state/resource expectations.
- [ ] **F-DS-019.06 — Implement/prove: states**
  - Action: For component registry, implement or reuse and verify this exact obligation: states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-019.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for states; retain observable state/resource expectations.
- [ ] **F-DS-019.07 — Implement/prove: semantics and ownership per primitive**
  - Action: For component registry, implement or reuse and verify this exact obligation: semantics and ownership per primitive. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-019.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantics and ownership per primitive; retain observable state/resource expectations.
- [ ] **F-DS-019.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to component registry: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-019.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-019.09 — Integrate into the real consumer and runtime route**
  - Action: Wire component registry into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-019.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-019.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for component registry as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-019.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-020"></a>
## F-DS-020 — button family

**Original requirement:** text/icon/toggle/destructive variants and full input/a11y states

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-020.01 — Reconcile existing button family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for button family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: text/icon/toggle/destructive variants and full input/a11y states
- [ ] **F-DS-020.02 — Freeze the exact contract for button family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: text/icon/toggle/destructive variants and full input/a11y states. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-020.03 — Implement/prove: text/icon/toggle/destructive variants and full input/a11y states**
  - Action: For button family, implement or reuse and verify this exact obligation: text/icon/toggle/destructive variants and full input/a11y states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for text/icon/toggle/destructive variants and full input/a11y states; retain observable state/resource expectations.
- [ ] **F-DS-020.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to button family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-020.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-020.05 — Integrate into the real consumer and runtime route**
  - Action: Wire button family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-020.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-020.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for button family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-020.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-021"></a>
## F-DS-021 — text field family

**Original requirement:** label/help/error, selection, IME, password, validation, undo and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-021.01 — Reconcile existing text field family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for text field family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: label/help/error, selection, IME, password, validation, undo and a11y
- [ ] **F-DS-021.02 — Freeze the exact contract for text field family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: label/help/error, selection, IME, password, validation, undo and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-021.03 — Implement/prove: label/help/error**
  - Action: For text field family, implement or reuse and verify this exact obligation: label/help/error. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for label/help/error; retain observable state/resource expectations.
- [ ] **F-DS-021.04 — Implement/prove: selection**
  - Action: For text field family, implement or reuse and verify this exact obligation: selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection; retain observable state/resource expectations.
- [ ] **F-DS-021.05 — Implement/prove: IME**
  - Action: For text field family, implement or reuse and verify this exact obligation: IME. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-021.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for IME; retain observable state/resource expectations.
- [ ] **F-DS-021.06 — Implement/prove: password**
  - Action: For text field family, implement or reuse and verify this exact obligation: password. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-021.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for password; retain observable state/resource expectations.
- [ ] **F-DS-021.07 — Implement/prove: validation**
  - Action: For text field family, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-021.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-DS-021.08 — Implement/prove: undo and a11y**
  - Action: For text field family, implement or reuse and verify this exact obligation: undo and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-021.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for undo and a11y; retain observable state/resource expectations.
- [ ] **F-DS-021.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to text field family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-021.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-021.10 — Integrate into the real consumer and runtime route**
  - Action: Wire text field family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-021.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-021.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for text field family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-021.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-022"></a>
## F-DS-022 — selection controls

**Original requirement:** checkbox/radio/switch with group, mixed, disabled, keyboard and semantic states

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-022.01 — Reconcile existing selection controls**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for selection controls. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: checkbox/radio/switch with group, mixed, disabled, keyboard and semantic states
- [ ] **F-DS-022.02 — Freeze the exact contract for selection controls**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: checkbox/radio/switch with group, mixed, disabled, keyboard and semantic states. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-022.03 — Implement/prove: checkbox/radio/switch with group**
  - Action: For selection controls, implement or reuse and verify this exact obligation: checkbox/radio/switch with group. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for checkbox/radio/switch with group; retain observable state/resource expectations.
- [ ] **F-DS-022.04 — Implement/prove: mixed**
  - Action: For selection controls, implement or reuse and verify this exact obligation: mixed. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for mixed; retain observable state/resource expectations.
- [ ] **F-DS-022.05 — Implement/prove: disabled**
  - Action: For selection controls, implement or reuse and verify this exact obligation: disabled. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for disabled; retain observable state/resource expectations.
- [ ] **F-DS-022.06 — Implement/prove: keyboard and semantic states**
  - Action: For selection controls, implement or reuse and verify this exact obligation: keyboard and semantic states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-022.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard and semantic states; retain observable state/resource expectations.
- [ ] **F-DS-022.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to selection controls: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-022.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-022.08 — Integrate into the real consumer and runtime route**
  - Action: Wire selection controls into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-022.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-022.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for selection controls as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-022.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-023"></a>
## F-DS-023 — slider/spin control

**Original requirement:** range/step/value format, keyboard, pointer, touch and validation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-023.01 — Reconcile existing slider/spin control**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for slider/spin control. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: range/step/value format, keyboard, pointer, touch and validation
- [ ] **F-DS-023.02 — Freeze the exact contract for slider/spin control**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: range/step/value format, keyboard, pointer, touch and validation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-023.03 — Implement/prove: range/step/value format**
  - Action: For slider/spin control, implement or reuse and verify this exact obligation: range/step/value format. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for range/step/value format; retain observable state/resource expectations.
- [ ] **F-DS-023.04 — Implement/prove: keyboard**
  - Action: For slider/spin control, implement or reuse and verify this exact obligation: keyboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-023.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard; retain observable state/resource expectations.
- [ ] **F-DS-023.05 — Implement/prove: pointer**
  - Action: For slider/spin control, implement or reuse and verify this exact obligation: pointer. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-023.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pointer; retain observable state/resource expectations.
- [ ] **F-DS-023.06 — Implement/prove: touch and validation**
  - Action: For slider/spin control, implement or reuse and verify this exact obligation: touch and validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-023.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for touch and validation; retain observable state/resource expectations.
- [ ] **F-DS-023.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to slider/spin control: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-023.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-023.08 — Integrate into the real consumer and runtime route**
  - Action: Wire slider/spin control into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-023.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-023.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for slider/spin control as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-023.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-024"></a>
## F-DS-024 — menu family

**Original requirement:** bar/context/popup/submenu, keyboard typeahead, bounds, dismissal and semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-024.01 — Reconcile existing menu family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for menu family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bar/context/popup/submenu, keyboard typeahead, bounds, dismissal and semantics
- [ ] **F-DS-024.02 — Freeze the exact contract for menu family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bar/context/popup/submenu, keyboard typeahead, bounds, dismissal and semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-024.03 — Implement/prove: bar/context/popup/submenu**
  - Action: For menu family, implement or reuse and verify this exact obligation: bar/context/popup/submenu. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bar/context/popup/submenu; retain observable state/resource expectations.
- [ ] **F-DS-024.04 — Implement/prove: keyboard typeahead**
  - Action: For menu family, implement or reuse and verify this exact obligation: keyboard typeahead. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard typeahead; retain observable state/resource expectations.
- [ ] **F-DS-024.05 — Implement/prove: bounds**
  - Action: For menu family, implement or reuse and verify this exact obligation: bounds. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-024.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bounds; retain observable state/resource expectations.
- [ ] **F-DS-024.06 — Implement/prove: dismissal and semantics**
  - Action: For menu family, implement or reuse and verify this exact obligation: dismissal and semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-024.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dismissal and semantics; retain observable state/resource expectations.
- [ ] **F-DS-024.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to menu family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-024.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-024.08 — Integrate into the real consumer and runtime route**
  - Action: Wire menu family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-024.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-024.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for menu family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-024.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-025"></a>
## F-DS-025 — toolbar family

**Original requirement:** grouping, overflow, labels/tooltips, shortcuts and adaptive density

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-025.01 — Reconcile existing toolbar family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for toolbar family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: grouping, overflow, labels/tooltips, shortcuts and adaptive density
- [ ] **F-DS-025.02 — Freeze the exact contract for toolbar family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: grouping, overflow, labels/tooltips, shortcuts and adaptive density. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-025.03 — Implement/prove: grouping**
  - Action: For toolbar family, implement or reuse and verify this exact obligation: grouping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grouping; retain observable state/resource expectations.
- [ ] **F-DS-025.04 — Implement/prove: overflow**
  - Action: For toolbar family, implement or reuse and verify this exact obligation: overflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-025.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow; retain observable state/resource expectations.
- [ ] **F-DS-025.05 — Implement/prove: labels/tooltips**
  - Action: For toolbar family, implement or reuse and verify this exact obligation: labels/tooltips. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-025.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for labels/tooltips; retain observable state/resource expectations.
- [ ] **F-DS-025.06 — Implement/prove: shortcuts and adaptive density**
  - Action: For toolbar family, implement or reuse and verify this exact obligation: shortcuts and adaptive density. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-025.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for shortcuts and adaptive density; retain observable state/resource expectations.
- [ ] **F-DS-025.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to toolbar family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-025.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-025.08 — Integrate into the real consumer and runtime route**
  - Action: Wire toolbar family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-025.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-025.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for toolbar family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-025.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-026"></a>
## F-DS-026 — tabs family

**Original requirement:** selection/focus separation, overflow, reorder/close and semantic relationships

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-026.01 — Reconcile existing tabs family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for tabs family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: selection/focus separation, overflow, reorder/close and semantic relationships
- [ ] **F-DS-026.02 — Freeze the exact contract for tabs family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: selection/focus separation, overflow, reorder/close and semantic relationships. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-026.03 — Implement/prove: selection/focus separation**
  - Action: For tabs family, implement or reuse and verify this exact obligation: selection/focus separation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection/focus separation; retain observable state/resource expectations.
- [ ] **F-DS-026.04 — Implement/prove: overflow**
  - Action: For tabs family, implement or reuse and verify this exact obligation: overflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for overflow; retain observable state/resource expectations.
- [ ] **F-DS-026.05 — Implement/prove: reorder/close and semantic relationships**
  - Action: For tabs family, implement or reuse and verify this exact obligation: reorder/close and semantic relationships. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for reorder/close and semantic relationships; retain observable state/resource expectations.
- [ ] **F-DS-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to tabs family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire tabs family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for tabs family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-027"></a>
## F-DS-027 — list/table/tree family

**Original requirement:** virtualization, selection, sort, hierarchy, resize and accessible semantics

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-027.01 — Reconcile existing list/table/tree family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for list/table/tree family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: virtualization, selection, sort, hierarchy, resize and accessible semantics
- [ ] **F-DS-027.02 — Freeze the exact contract for list/table/tree family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: virtualization, selection, sort, hierarchy, resize and accessible semantics. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-027.03 — Implement/prove: virtualization**
  - Action: For list/table/tree family, implement or reuse and verify this exact obligation: virtualization. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for virtualization; retain observable state/resource expectations.
- [ ] **F-DS-027.04 — Implement/prove: selection**
  - Action: For list/table/tree family, implement or reuse and verify this exact obligation: selection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-027.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for selection; retain observable state/resource expectations.
- [ ] **F-DS-027.05 — Implement/prove: sort**
  - Action: For list/table/tree family, implement or reuse and verify this exact obligation: sort. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-027.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for sort; retain observable state/resource expectations.
- [ ] **F-DS-027.06 — Implement/prove: hierarchy**
  - Action: For list/table/tree family, implement or reuse and verify this exact obligation: hierarchy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-027.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hierarchy; retain observable state/resource expectations.
- [ ] **F-DS-027.07 — Implement/prove: resize and accessible semantics**
  - Action: For list/table/tree family, implement or reuse and verify this exact obligation: resize and accessible semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-027.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize and accessible semantics; retain observable state/resource expectations.
- [ ] **F-DS-027.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to list/table/tree family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-027.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-027.09 — Integrate into the real consumer and runtime route**
  - Action: Wire list/table/tree family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-027.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-027.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for list/table/tree family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-027.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-028"></a>
## F-DS-028 — scroll container

**Original requirement:** clipping, bars, wheel/touch/keyboard, anchoring and reduced motion

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-028.01 — Reconcile existing scroll container**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for scroll container. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: clipping, bars, wheel/touch/keyboard, anchoring and reduced motion
- [ ] **F-DS-028.02 — Freeze the exact contract for scroll container**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: clipping, bars, wheel/touch/keyboard, anchoring and reduced motion. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-028.03 — Implement/prove: clipping**
  - Action: For scroll container, implement or reuse and verify this exact obligation: clipping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clipping; retain observable state/resource expectations.
- [ ] **F-DS-028.04 — Implement/prove: bars**
  - Action: For scroll container, implement or reuse and verify this exact obligation: bars. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bars; retain observable state/resource expectations.
- [ ] **F-DS-028.05 — Implement/prove: wheel/touch/keyboard**
  - Action: For scroll container, implement or reuse and verify this exact obligation: wheel/touch/keyboard. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for wheel/touch/keyboard; retain observable state/resource expectations.
- [ ] **F-DS-028.06 — Implement/prove: anchoring and reduced motion**
  - Action: For scroll container, implement or reuse and verify this exact obligation: anchoring and reduced motion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for anchoring and reduced motion; retain observable state/resource expectations.
- [ ] **F-DS-028.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to scroll container: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-028.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-028.08 — Integrate into the real consumer and runtime route**
  - Action: Wire scroll container into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-028.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-028.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for scroll container as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-028.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-029"></a>
## F-DS-029 — dialog/modal family

**Original requirement:** focus trap/restore, default/cancel, destructive confirmation and peer death

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-029.01 — Reconcile existing dialog/modal family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for dialog/modal family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: focus trap/restore, default/cancel, destructive confirmation and peer death
- [ ] **F-DS-029.02 — Freeze the exact contract for dialog/modal family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: focus trap/restore, default/cancel, destructive confirmation and peer death. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-029.03 — Implement/prove: focus trap/restore**
  - Action: For dialog/modal family, implement or reuse and verify this exact obligation: focus trap/restore. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus trap/restore; retain observable state/resource expectations.
- [ ] **F-DS-029.04 — Implement/prove: default/cancel**
  - Action: For dialog/modal family, implement or reuse and verify this exact obligation: default/cancel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for default/cancel; retain observable state/resource expectations.
- [ ] **F-DS-029.05 — Implement/prove: destructive confirmation and peer death**
  - Action: For dialog/modal family, implement or reuse and verify this exact obligation: destructive confirmation and peer death. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for destructive confirmation and peer death; retain observable state/resource expectations.
- [ ] **F-DS-029.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to dialog/modal family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-029.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-029.07 — Integrate into the real consumer and runtime route**
  - Action: Wire dialog/modal family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-029.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-029.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for dialog/modal family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-029.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-030"></a>
## F-DS-030 — toast/notification primitive

**Original requirement:** severity, action, timeout, pause, history and screen-reader announcement

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-030.01 — Reconcile existing toast/notification primitive**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for toast/notification primitive. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: severity, action, timeout, pause, history and screen-reader announcement
- [ ] **F-DS-030.02 — Freeze the exact contract for toast/notification primitive**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: severity, action, timeout, pause, history and screen-reader announcement. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-030.03 — Implement/prove: severity**
  - Action: For toast/notification primitive, implement or reuse and verify this exact obligation: severity. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for severity; retain observable state/resource expectations.
- [ ] **F-DS-030.04 — Implement/prove: action**
  - Action: For toast/notification primitive, implement or reuse and verify this exact obligation: action. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for action; retain observable state/resource expectations.
- [ ] **F-DS-030.05 — Implement/prove: timeout**
  - Action: For toast/notification primitive, implement or reuse and verify this exact obligation: timeout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-030.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timeout; retain observable state/resource expectations.
- [ ] **F-DS-030.06 — Implement/prove: pause**
  - Action: For toast/notification primitive, implement or reuse and verify this exact obligation: pause. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-030.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pause; retain observable state/resource expectations.
- [ ] **F-DS-030.07 — Implement/prove: history and screen-reader announcement**
  - Action: For toast/notification primitive, implement or reuse and verify this exact obligation: history and screen-reader announcement. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-030.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for history and screen-reader announcement; retain observable state/resource expectations.
- [ ] **F-DS-030.08 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to toast/notification primitive: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-030.07.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-030.09 — Integrate into the real consumer and runtime route**
  - Action: Wire toast/notification primitive into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-030.08.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-030.10 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for toast/notification primitive as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-030.09.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-031"></a>
## F-DS-031 — progress family

**Original requirement:** determinate/indeterminate/staged, cancel, error and truthful backend state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-031.01 — Reconcile existing progress family**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for progress family. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: determinate/indeterminate/staged, cancel, error and truthful backend state
- [ ] **F-DS-031.02 — Freeze the exact contract for progress family**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: determinate/indeterminate/staged, cancel, error and truthful backend state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-031.03 — Implement/prove: determinate/indeterminate/staged**
  - Action: For progress family, implement or reuse and verify this exact obligation: determinate/indeterminate/staged. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for determinate/indeterminate/staged; retain observable state/resource expectations.
- [ ] **F-DS-031.04 — Implement/prove: cancel**
  - Action: For progress family, implement or reuse and verify this exact obligation: cancel. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for cancel; retain observable state/resource expectations.
- [ ] **F-DS-031.05 — Implement/prove: error and truthful backend state**
  - Action: For progress family, implement or reuse and verify this exact obligation: error and truthful backend state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for error and truthful backend state; retain observable state/resource expectations.
- [ ] **F-DS-031.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to progress family: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-031.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-031.07 — Integrate into the real consumer and runtime route**
  - Action: Wire progress family into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-031.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-031.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for progress family as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-031.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-032"></a>
## F-DS-032 — empty/error/loading states

**Original requirement:** actionable truthful states in every app/service workflow

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-032.01 — Reconcile existing empty/error/loading states**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for empty/error/loading states. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: actionable truthful states in every app/service workflow
- [ ] **F-DS-032.02 — Freeze the exact contract for empty/error/loading states**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: actionable truthful states in every app/service workflow. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-032.03 — Implement/prove: actionable truthful states in every app/service workflow**
  - Action: For empty/error/loading states, implement or reuse and verify this exact obligation: actionable truthful states in every app/service workflow. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for actionable truthful states in every app/service workflow; retain observable state/resource expectations.
- [ ] **F-DS-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to empty/error/loading states: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire empty/error/loading states into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for empty/error/loading states as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-033"></a>
## F-DS-033 — card/panel/split-pane

**Original requirement:** semantic grouping, resize/collapse, keyboard and responsive layout

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-033.01 — Reconcile existing card/panel/split-pane**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for card/panel/split-pane. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic grouping, resize/collapse, keyboard and responsive layout
- [ ] **F-DS-033.02 — Freeze the exact contract for card/panel/split-pane**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic grouping, resize/collapse, keyboard and responsive layout. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-033.03 — Implement/prove: semantic grouping**
  - Action: For card/panel/split-pane, implement or reuse and verify this exact obligation: semantic grouping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic grouping; retain observable state/resource expectations.
- [ ] **F-DS-033.04 — Implement/prove: resize/collapse**
  - Action: For card/panel/split-pane, implement or reuse and verify this exact obligation: resize/collapse. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize/collapse; retain observable state/resource expectations.
- [ ] **F-DS-033.05 — Implement/prove: keyboard and responsive layout**
  - Action: For card/panel/split-pane, implement or reuse and verify this exact obligation: keyboard and responsive layout. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard and responsive layout; retain observable state/resource expectations.
- [ ] **F-DS-033.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to card/panel/split-pane: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-033.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-033.07 — Integrate into the real consumer and runtime route**
  - Action: Wire card/panel/split-pane into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-033.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-033.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for card/panel/split-pane as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-033.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-034"></a>
## F-DS-034 — window chrome

**Original requirement:** title/icon/status, drag, controls, resize, focus, maximize and a11y

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-034.01 — Reconcile existing window chrome**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for window chrome. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: title/icon/status, drag, controls, resize, focus, maximize and a11y
- [ ] **F-DS-034.02 — Freeze the exact contract for window chrome**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: title/icon/status, drag, controls, resize, focus, maximize and a11y. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-034.03 — Implement/prove: title/icon/status**
  - Action: For window chrome, implement or reuse and verify this exact obligation: title/icon/status. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for title/icon/status; retain observable state/resource expectations.
- [ ] **F-DS-034.04 — Implement/prove: drag**
  - Action: For window chrome, implement or reuse and verify this exact obligation: drag. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-034.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for drag; retain observable state/resource expectations.
- [ ] **F-DS-034.05 — Implement/prove: controls**
  - Action: For window chrome, implement or reuse and verify this exact obligation: controls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-034.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for controls; retain observable state/resource expectations.
- [ ] **F-DS-034.06 — Implement/prove: resize**
  - Action: For window chrome, implement or reuse and verify this exact obligation: resize. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-034.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for resize; retain observable state/resource expectations.
- [ ] **F-DS-034.07 — Implement/prove: focus**
  - Action: For window chrome, implement or reuse and verify this exact obligation: focus. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-034.06.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus; retain observable state/resource expectations.
- [ ] **F-DS-034.08 — Implement/prove: maximize and a11y**
  - Action: For window chrome, implement or reuse and verify this exact obligation: maximize and a11y. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-034.07.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for maximize and a11y; retain observable state/resource expectations.
- [ ] **F-DS-034.09 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to window chrome: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-034.08.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-034.10 — Integrate into the real consumer and runtime route**
  - Action: Wire window chrome into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-034.09.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-034.11 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for window chrome as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-034.10.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-035"></a>
## F-DS-035 — data visualization primitives

**Original requirement:** axes/units/legend/contrast/text alternative and streaming limits

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-035.01 — Reconcile existing data visualization primitives**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for data visualization primitives. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: axes/units/legend/contrast/text alternative and streaming limits
- [ ] **F-DS-035.02 — Freeze the exact contract for data visualization primitives**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: axes/units/legend/contrast/text alternative and streaming limits. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-035.03 — Implement/prove: axes/units/legend/contrast/text alternative and streaming limits**
  - Action: For data visualization primitives, implement or reuse and verify this exact obligation: axes/units/legend/contrast/text alternative and streaming limits. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for axes/units/legend/contrast/text alternative and streaming limits; retain observable state/resource expectations.
- [ ] **F-DS-035.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to data visualization primitives: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-035.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-035.05 — Integrate into the real consumer and runtime route**
  - Action: Wire data visualization primitives into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-035.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-035.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for data visualization primitives as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-035.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-036"></a>
## F-DS-036 — component gallery

**Original requirement:** interactive states, tokens, semantics, performance and visual-regression fixture

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-036.01 — Reconcile existing component gallery**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for component gallery. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: interactive states, tokens, semantics, performance and visual-regression fixture
- [ ] **F-DS-036.02 — Freeze the exact contract for component gallery**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: interactive states, tokens, semantics, performance and visual-regression fixture. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-036.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-036.03 — Implement/prove: interactive states**
  - Action: For component gallery, implement or reuse and verify this exact obligation: interactive states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-036.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for interactive states; retain observable state/resource expectations.
- [ ] **F-DS-036.04 — Implement/prove: tokens**
  - Action: For component gallery, implement or reuse and verify this exact obligation: tokens. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-036.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for tokens; retain observable state/resource expectations.
- [ ] **F-DS-036.05 — Implement/prove: semantics**
  - Action: For component gallery, implement or reuse and verify this exact obligation: semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-036.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantics; retain observable state/resource expectations.
- [ ] **F-DS-036.06 — Implement/prove: performance and visual-regression fixture**
  - Action: For component gallery, implement or reuse and verify this exact obligation: performance and visual-regression fixture. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-036.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for performance and visual-regression fixture; retain observable state/resource expectations.
- [ ] **F-DS-036.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to component gallery: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-036.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-036.08 — Integrate into the real consumer and runtime route**
  - Action: Wire component gallery into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-036.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-036.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for component gallery as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-036.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-037"></a>
## F-DS-037 — declarative layout format

**Original requirement:** versioned safe schema, validation, localization/scale and generated bindings

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-037.01 — Reconcile existing declarative layout format**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for declarative layout format. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: versioned safe schema, validation, localization/scale and generated bindings
- [ ] **F-DS-037.02 — Freeze the exact contract for declarative layout format**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: versioned safe schema, validation, localization/scale and generated bindings. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-037.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-037.03 — Implement/prove: versioned safe schema**
  - Action: For declarative layout format, implement or reuse and verify this exact obligation: versioned safe schema. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-037.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for versioned safe schema; retain observable state/resource expectations.
- [ ] **F-DS-037.04 — Implement/prove: validation**
  - Action: For declarative layout format, implement or reuse and verify this exact obligation: validation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-037.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for validation; retain observable state/resource expectations.
- [ ] **F-DS-037.05 — Implement/prove: localization/scale and generated bindings**
  - Action: For declarative layout format, implement or reuse and verify this exact obligation: localization/scale and generated bindings. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-037.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for localization/scale and generated bindings; retain observable state/resource expectations.
- [ ] **F-DS-037.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to declarative layout format: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-037.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-037.07 — Integrate into the real consumer and runtime route**
  - Action: Wire declarative layout format into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-037.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-037.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for declarative layout format as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-037.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-038"></a>
## F-DS-038 — responsive breakpoints

**Original requirement:** content-driven reflow for window, display, touch and remote viewport

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-038.01 — Reconcile existing responsive breakpoints**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for responsive breakpoints. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: content-driven reflow for window, display, touch and remote viewport
- [ ] **F-DS-038.02 — Freeze the exact contract for responsive breakpoints**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: content-driven reflow for window, display, touch and remote viewport. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-038.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-038.03 — Implement/prove: content-driven reflow for window**
  - Action: For responsive breakpoints, implement or reuse and verify this exact obligation: content-driven reflow for window. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-038.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for content-driven reflow for window; retain observable state/resource expectations.
- [ ] **F-DS-038.04 — Implement/prove: display**
  - Action: For responsive breakpoints, implement or reuse and verify this exact obligation: display. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-038.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for display; retain observable state/resource expectations.
- [ ] **F-DS-038.05 — Implement/prove: touch and remote viewport**
  - Action: For responsive breakpoints, implement or reuse and verify this exact obligation: touch and remote viewport. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-038.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for touch and remote viewport; retain observable state/resource expectations.
- [ ] **F-DS-038.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to responsive breakpoints: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-038.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-038.07 — Integrate into the real consumer and runtime route**
  - Action: Wire responsive breakpoints into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-038.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-038.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for responsive breakpoints as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-038.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-039"></a>
## F-DS-039 — content-writing rules

**Original requirement:** clear labels, errors, empty states, dangerous actions and terminology registry

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-039.01 — Reconcile existing content-writing rules**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for content-writing rules. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: clear labels, errors, empty states, dangerous actions and terminology registry
- [ ] **F-DS-039.02 — Freeze the exact contract for content-writing rules**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: clear labels, errors, empty states, dangerous actions and terminology registry. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-039.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-039.03 — Implement/prove: clear labels**
  - Action: For content-writing rules, implement or reuse and verify this exact obligation: clear labels. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-039.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for clear labels; retain observable state/resource expectations.
- [ ] **F-DS-039.04 — Implement/prove: errors**
  - Action: For content-writing rules, implement or reuse and verify this exact obligation: errors. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-039.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for errors; retain observable state/resource expectations.
- [ ] **F-DS-039.05 — Implement/prove: empty states**
  - Action: For content-writing rules, implement or reuse and verify this exact obligation: empty states. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-039.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for empty states; retain observable state/resource expectations.
- [ ] **F-DS-039.06 — Implement/prove: dangerous actions and terminology registry**
  - Action: For content-writing rules, implement or reuse and verify this exact obligation: dangerous actions and terminology registry. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-039.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for dangerous actions and terminology registry; retain observable state/resource expectations.
- [ ] **F-DS-039.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to content-writing rules: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-039.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-039.08 — Integrate into the real consumer and runtime route**
  - Action: Wire content-writing rules into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-039.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-039.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for content-writing rules as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-039.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ds-040"></a>
## F-DS-040 — design-system versioning

**Original requirement:** deprecations, migrations, compatibility ranges, consumers and release notes

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-DS-040.01 — Reconcile existing design-system versioning**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for design-system versioning. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: deprecations, migrations, compatibility ranges, consumers and release notes
- [ ] **F-DS-040.02 — Freeze the exact contract for design-system versioning**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: deprecations, migrations, compatibility ranges, consumers and release notes. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-DS-040.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-DS-040.03 — Implement/prove: deprecations**
  - Action: For design-system versioning, implement or reuse and verify this exact obligation: deprecations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-040.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for deprecations; retain observable state/resource expectations.
- [ ] **F-DS-040.04 — Implement/prove: migrations**
  - Action: For design-system versioning, implement or reuse and verify this exact obligation: migrations. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-040.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for migrations; retain observable state/resource expectations.
- [ ] **F-DS-040.05 — Implement/prove: compatibility ranges**
  - Action: For design-system versioning, implement or reuse and verify this exact obligation: compatibility ranges. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-040.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for compatibility ranges; retain observable state/resource expectations.
- [ ] **F-DS-040.06 — Implement/prove: consumers and release notes**
  - Action: For design-system versioning, implement or reuse and verify this exact obligation: consumers and release notes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-DS-040.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for consumers and release notes; retain observable state/resource expectations.
- [ ] **F-DS-040.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to design-system versioning: all component states; theme/scale transitions; font/glyph absence; layout overflow; interrupted settings commit; reduced motion; keyboard/focus and semantic mismatch. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-DS-040.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-DS-040.08 — Integrate into the real consumer and runtime route**
  - Action: Wire design-system versioning into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-DS-040.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-DS-040.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for design-system versioning as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-DS-040.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-001"></a>
## F-AX-001 — semantic UI tree

**Original requirement:** role/name/value/state/relations/actions/bounds with authenticated ownership

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-001.01 — Reconcile existing semantic UI tree**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for semantic UI tree. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: role/name/value/state/relations/actions/bounds with authenticated ownership
- [ ] **F-AX-001.02 — Freeze the exact contract for semantic UI tree**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: role/name/value/state/relations/actions/bounds with authenticated ownership. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-001.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-001.03 — Implement/prove: role/name/value/state/relations/actions/bounds with authenticated ownership**
  - Action: For semantic UI tree, implement or reuse and verify this exact obligation: role/name/value/state/relations/actions/bounds with authenticated ownership. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-001.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for role/name/value/state/relations/actions/bounds with authenticated ownership; retain observable state/resource expectations.
- [ ] **F-AX-001.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to semantic UI tree: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-001.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-001.05 — Integrate into the real consumer and runtime route**
  - Action: Wire semantic UI tree into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-001.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-001.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for semantic UI tree as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-001.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-002"></a>
## F-AX-002 — semantic event stream

**Original requirement:** focus/value/text/layout/live-region updates are bounded and generation-tagged

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-002.01 — Reconcile existing semantic event stream**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for semantic event stream. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: focus/value/text/layout/live-region updates are bounded and generation-tagged
- [ ] **F-AX-002.02 — Freeze the exact contract for semantic event stream**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: focus/value/text/layout/live-region updates are bounded and generation-tagged. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-002.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-002.03 — Implement/prove: focus/value/text/layout/live-region updates are bounded and generation-tagged**
  - Action: For semantic event stream, implement or reuse and verify this exact obligation: focus/value/text/layout/live-region updates are bounded and generation-tagged. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-002.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus/value/text/layout/live-region updates are bounded and generation-tagged; retain observable state/resource expectations.
- [ ] **F-AX-002.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to semantic event stream: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-002.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-002.05 — Integrate into the real consumer and runtime route**
  - Action: Wire semantic event stream into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-002.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-002.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for semantic event stream as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-002.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-003"></a>
## F-AX-003 — screen reader

**Original requirement:** navigation, speech output, app/shell/browser coverage and privacy controls

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-003.01 — Reconcile existing screen reader**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for screen reader. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: navigation, speech output, app/shell/browser coverage and privacy controls
- [ ] **F-AX-003.02 — Freeze the exact contract for screen reader**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: navigation, speech output, app/shell/browser coverage and privacy controls. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-003.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-003.03 — Implement/prove: navigation**
  - Action: For screen reader, implement or reuse and verify this exact obligation: navigation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-003.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for navigation; retain observable state/resource expectations.
- [ ] **F-AX-003.04 — Implement/prove: speech output**
  - Action: For screen reader, implement or reuse and verify this exact obligation: speech output. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-003.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for speech output; retain observable state/resource expectations.
- [ ] **F-AX-003.05 — Implement/prove: app/shell/browser coverage and privacy controls**
  - Action: For screen reader, implement or reuse and verify this exact obligation: app/shell/browser coverage and privacy controls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-003.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for app/shell/browser coverage and privacy controls; retain observable state/resource expectations.
- [ ] **F-AX-003.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to screen reader: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-003.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-003.07 — Integrate into the real consumer and runtime route**
  - Action: Wire screen reader into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-003.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-003.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for screen reader as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-003.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-004"></a>
## F-AX-004 — braille provider

**Original requirement:** contracted cells/routing keys, status and screen-reader integration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-004.01 — Reconcile existing braille provider**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for braille provider. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: contracted cells/routing keys, status and screen-reader integration
- [ ] **F-AX-004.02 — Freeze the exact contract for braille provider**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: contracted cells/routing keys, status and screen-reader integration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-004.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-004.03 — Implement/prove: contracted cells/routing keys**
  - Action: For braille provider, implement or reuse and verify this exact obligation: contracted cells/routing keys. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-004.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for contracted cells/routing keys; retain observable state/resource expectations.
- [ ] **F-AX-004.04 — Implement/prove: status and screen-reader integration**
  - Action: For braille provider, implement or reuse and verify this exact obligation: status and screen-reader integration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-004.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for status and screen-reader integration; retain observable state/resource expectations.
- [ ] **F-AX-004.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to braille provider: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-004.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-004.06 — Integrate into the real consumer and runtime route**
  - Action: Wire braille provider into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-004.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-004.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for braille provider as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-004.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-005"></a>
## F-AX-005 — keyboard-complete navigation

**Original requirement:** every route works without pointer with visible focus and logical order

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-005.01 — Reconcile existing keyboard-complete navigation**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for keyboard-complete navigation. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: every route works without pointer with visible focus and logical order
- [ ] **F-AX-005.02 — Freeze the exact contract for keyboard-complete navigation**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: every route works without pointer with visible focus and logical order. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-005.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-005.03 — Implement/prove: every route works without pointer with visible focus and logical order**
  - Action: For keyboard-complete navigation, implement or reuse and verify this exact obligation: every route works without pointer with visible focus and logical order. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-005.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for every route works without pointer with visible focus and logical order; retain observable state/resource expectations.
- [ ] **F-AX-005.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to keyboard-complete navigation: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-005.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-005.05 — Integrate into the real consumer and runtime route**
  - Action: Wire keyboard-complete navigation into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-005.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-005.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for keyboard-complete navigation as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-005.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-006"></a>
## F-AX-006 — focus appearance

**Original requirement:** persistent high-contrast non-color indication in every theme/state

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-006.01 — Reconcile existing focus appearance**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for focus appearance. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: persistent high-contrast non-color indication in every theme/state
- [ ] **F-AX-006.02 — Freeze the exact contract for focus appearance**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: persistent high-contrast non-color indication in every theme/state. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-006.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-006.03 — Implement/prove: persistent high-contrast non-color indication in every theme/state**
  - Action: For focus appearance, implement or reuse and verify this exact obligation: persistent high-contrast non-color indication in every theme/state. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-006.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistent high-contrast non-color indication in every theme/state; retain observable state/resource expectations.
- [ ] **F-AX-006.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to focus appearance: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-006.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-006.05 — Integrate into the real consumer and runtime route**
  - Action: Wire focus appearance into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-006.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-006.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for focus appearance as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-006.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-007"></a>
## F-AX-007 — focus order/groups

**Original requirement:** document/layout semantics, modal containment and restoration

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-007.01 — Reconcile existing focus order/groups**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for focus order/groups. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: document/layout semantics, modal containment and restoration
- [ ] **F-AX-007.02 — Freeze the exact contract for focus order/groups**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: document/layout semantics, modal containment and restoration. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-007.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-007.03 — Implement/prove: document/layout semantics**
  - Action: For focus order/groups, implement or reuse and verify this exact obligation: document/layout semantics. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-007.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for document/layout semantics; retain observable state/resource expectations.
- [ ] **F-AX-007.04 — Implement/prove: modal containment and restoration**
  - Action: For focus order/groups, implement or reuse and verify this exact obligation: modal containment and restoration. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-007.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for modal containment and restoration; retain observable state/resource expectations.
- [ ] **F-AX-007.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to focus order/groups: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-007.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-007.06 — Integrate into the real consumer and runtime route**
  - Action: Wire focus order/groups into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-007.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-007.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for focus order/groups as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-007.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-008"></a>
## F-AX-008 — shortcuts/access keys

**Original requirement:** discoverable, remappable, conflict-resolved and localized

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-008.01 — Reconcile existing shortcuts/access keys**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for shortcuts/access keys. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discoverable, remappable, conflict-resolved and localized
- [ ] **F-AX-008.02 — Freeze the exact contract for shortcuts/access keys**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discoverable, remappable, conflict-resolved and localized. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-008.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-008.03 — Implement/prove: discoverable**
  - Action: For shortcuts/access keys, implement or reuse and verify this exact obligation: discoverable. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-008.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discoverable; retain observable state/resource expectations.
- [ ] **F-AX-008.04 — Implement/prove: remappable**
  - Action: For shortcuts/access keys, implement or reuse and verify this exact obligation: remappable. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-008.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for remappable; retain observable state/resource expectations.
- [ ] **F-AX-008.05 — Implement/prove: conflict-resolved and localized**
  - Action: For shortcuts/access keys, implement or reuse and verify this exact obligation: conflict-resolved and localized. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-008.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for conflict-resolved and localized; retain observable state/resource expectations.
- [ ] **F-AX-008.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to shortcuts/access keys: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-008.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-008.07 — Integrate into the real consumer and runtime route**
  - Action: Wire shortcuts/access keys into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-008.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-008.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for shortcuts/access keys as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-008.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-009"></a>
## F-AX-009 — large text/system scale

**Original requirement:** semantic scale without clipping, hidden controls or fixed-layout failure

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-009.01 — Reconcile existing large text/system scale**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for large text/system scale. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: semantic scale without clipping, hidden controls or fixed-layout failure
- [ ] **F-AX-009.02 — Freeze the exact contract for large text/system scale**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: semantic scale without clipping, hidden controls or fixed-layout failure. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-009.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-009.03 — Implement/prove: semantic scale without clipping**
  - Action: For large text/system scale, implement or reuse and verify this exact obligation: semantic scale without clipping. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-009.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for semantic scale without clipping; retain observable state/resource expectations.
- [ ] **F-AX-009.04 — Implement/prove: hidden controls or fixed-layout failure**
  - Action: For large text/system scale, implement or reuse and verify this exact obligation: hidden controls or fixed-layout failure. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-009.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for hidden controls or fixed-layout failure; retain observable state/resource expectations.
- [ ] **F-AX-009.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to large text/system scale: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-009.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-009.06 — Integrate into the real consumer and runtime route**
  - Action: Wire large text/system scale into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-009.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-009.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for large text/system scale as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-009.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-010"></a>
## F-AX-010 — magnifier

**Original requirement:** full-screen/lens/docked modes, pointer/focus tracking and GPU/software paths

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-010.01 — Reconcile existing magnifier**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for magnifier. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: full-screen/lens/docked modes, pointer/focus tracking and GPU/software paths
- [ ] **F-AX-010.02 — Freeze the exact contract for magnifier**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: full-screen/lens/docked modes, pointer/focus tracking and GPU/software paths. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-010.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-010.03 — Implement/prove: full-screen/lens/docked modes**
  - Action: For magnifier, implement or reuse and verify this exact obligation: full-screen/lens/docked modes. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-010.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for full-screen/lens/docked modes; retain observable state/resource expectations.
- [ ] **F-AX-010.04 — Implement/prove: pointer/focus tracking and GPU/software paths**
  - Action: For magnifier, implement or reuse and verify this exact obligation: pointer/focus tracking and GPU/software paths. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-010.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for pointer/focus tracking and GPU/software paths; retain observable state/resource expectations.
- [ ] **F-AX-010.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to magnifier: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-010.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-010.06 — Integrate into the real consumer and runtime route**
  - Action: Wire magnifier into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-010.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-010.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for magnifier as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-010.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-011"></a>
## F-AX-011 — high contrast

**Original requirement:** theme/system override with image/icon/state treatment and regression proofs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-011.01 — Reconcile existing high contrast**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for high contrast. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: theme/system override with image/icon/state treatment and regression proofs
- [ ] **F-AX-011.02 — Freeze the exact contract for high contrast**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: theme/system override with image/icon/state treatment and regression proofs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-011.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-011.03 — Implement/prove: theme/system override with image/icon/state treatment and regression proofs**
  - Action: For high contrast, implement or reuse and verify this exact obligation: theme/system override with image/icon/state treatment and regression proofs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-011.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for theme/system override with image/icon/state treatment and regression proofs; retain observable state/resource expectations.
- [ ] **F-AX-011.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to high contrast: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-011.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-011.05 — Integrate into the real consumer and runtime route**
  - Action: Wire high contrast into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-011.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-011.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for high contrast as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-011.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-012"></a>
## F-AX-012 — color filters

**Original requirement:** grayscale/invert/color-deficiency transforms with capture/privacy policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-012.01 — Reconcile existing color filters**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for color filters. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: grayscale/invert/color-deficiency transforms with capture/privacy policy
- [ ] **F-AX-012.02 — Freeze the exact contract for color filters**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: grayscale/invert/color-deficiency transforms with capture/privacy policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-012.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-012.03 — Implement/prove: grayscale/invert/color-deficiency transforms with capture/privacy policy**
  - Action: For color filters, implement or reuse and verify this exact obligation: grayscale/invert/color-deficiency transforms with capture/privacy policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-012.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grayscale/invert/color-deficiency transforms with capture/privacy policy; retain observable state/resource expectations.
- [ ] **F-AX-012.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to color filters: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-012.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-012.05 — Integrate into the real consumer and runtime route**
  - Action: Wire color filters into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-012.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-012.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for color filters as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-012.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-013"></a>
## F-AX-013 — contrast checker

**Original requirement:** design/user inspection for text, icons, focus and state pairs

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-013.01 — Reconcile existing contrast checker**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for contrast checker. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: design/user inspection for text, icons, focus and state pairs
- [ ] **F-AX-013.02 — Freeze the exact contract for contrast checker**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: design/user inspection for text, icons, focus and state pairs. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-013.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-013.03 — Implement/prove: design/user inspection for text**
  - Action: For contrast checker, implement or reuse and verify this exact obligation: design/user inspection for text. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-013.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for design/user inspection for text; retain observable state/resource expectations.
- [ ] **F-AX-013.04 — Implement/prove: icons**
  - Action: For contrast checker, implement or reuse and verify this exact obligation: icons. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-013.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for icons; retain observable state/resource expectations.
- [ ] **F-AX-013.05 — Implement/prove: focus and state pairs**
  - Action: For contrast checker, implement or reuse and verify this exact obligation: focus and state pairs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-013.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for focus and state pairs; retain observable state/resource expectations.
- [ ] **F-AX-013.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to contrast checker: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-013.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-013.07 — Integrate into the real consumer and runtime route**
  - Action: Wire contrast checker into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-013.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-013.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for contrast checker as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-013.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-014"></a>
## F-AX-014 — reduced motion

**Original requirement:** animations disabled/reduced without losing causality or progress

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-014.01 — Reconcile existing reduced motion**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for reduced motion. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: animations disabled/reduced without losing causality or progress
- [ ] **F-AX-014.02 — Freeze the exact contract for reduced motion**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: animations disabled/reduced without losing causality or progress. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-014.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-014.03 — Implement/prove: animations disabled/reduced without losing causality or progress**
  - Action: For reduced motion, implement or reuse and verify this exact obligation: animations disabled/reduced without losing causality or progress. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-014.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for animations disabled/reduced without losing causality or progress; retain observable state/resource expectations.
- [ ] **F-AX-014.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to reduced motion: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-014.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-014.05 — Integrate into the real consumer and runtime route**
  - Action: Wire reduced motion into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-014.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-014.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for reduced motion as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-014.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-015"></a>
## F-AX-015 — captions

**Original requirement:** media/live system captions, language/style/position and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-015.01 — Reconcile existing captions**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for captions. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: media/live system captions, language/style/position and privacy
- [ ] **F-AX-015.02 — Freeze the exact contract for captions**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: media/live system captions, language/style/position and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-015.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-015.03 — Implement/prove: media/live system captions**
  - Action: For captions, implement or reuse and verify this exact obligation: media/live system captions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-015.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for media/live system captions; retain observable state/resource expectations.
- [ ] **F-AX-015.04 — Implement/prove: language/style/position and privacy**
  - Action: For captions, implement or reuse and verify this exact obligation: language/style/position and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-015.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for language/style/position and privacy; retain observable state/resource expectations.
- [ ] **F-AX-015.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to captions: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-015.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-015.06 — Integrate into the real consumer and runtime route**
  - Action: Wire captions into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-015.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-015.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for captions as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-015.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-016"></a>
## F-AX-016 — audio descriptions

**Original requirement:** alternate media track selection and controls

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-016.01 — Reconcile existing audio descriptions**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for audio descriptions. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: alternate media track selection and controls
- [ ] **F-AX-016.02 — Freeze the exact contract for audio descriptions**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: alternate media track selection and controls. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-016.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-016.03 — Implement/prove: alternate media track selection and controls**
  - Action: For audio descriptions, implement or reuse and verify this exact obligation: alternate media track selection and controls. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-016.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for alternate media track selection and controls; retain observable state/resource expectations.
- [ ] **F-AX-016.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to audio descriptions: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-016.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-016.05 — Integrate into the real consumer and runtime route**
  - Action: Wire audio descriptions into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-016.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-016.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for audio descriptions as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-016.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-017"></a>
## F-AX-017 — mono/balance audio

**Original requirement:** system accessibility routing and test tone

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-017.01 — Reconcile existing mono/balance audio**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mono/balance audio. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: system accessibility routing and test tone
- [ ] **F-AX-017.02 — Freeze the exact contract for mono/balance audio**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: system accessibility routing and test tone. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-017.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-017.03 — Implement/prove: system accessibility routing and test tone**
  - Action: For mono/balance audio, implement or reuse and verify this exact obligation: system accessibility routing and test tone. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-017.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for system accessibility routing and test tone; retain observable state/resource expectations.
- [ ] **F-AX-017.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mono/balance audio: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-017.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-017.05 — Integrate into the real consumer and runtime route**
  - Action: Wire mono/balance audio into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-017.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-017.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mono/balance audio as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-017.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-018"></a>
## F-AX-018 — sticky/filter/slow keys

**Original requirement:** configurable keyboard transforms, indicators, persistence and escape

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-018.01 — Reconcile existing sticky/filter/slow keys**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for sticky/filter/slow keys. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: configurable keyboard transforms, indicators, persistence and escape
- [ ] **F-AX-018.02 — Freeze the exact contract for sticky/filter/slow keys**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: configurable keyboard transforms, indicators, persistence and escape. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-018.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-018.03 — Implement/prove: configurable keyboard transforms**
  - Action: For sticky/filter/slow keys, implement or reuse and verify this exact obligation: configurable keyboard transforms. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-018.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configurable keyboard transforms; retain observable state/resource expectations.
- [ ] **F-AX-018.04 — Implement/prove: indicators**
  - Action: For sticky/filter/slow keys, implement or reuse and verify this exact obligation: indicators. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-018.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for indicators; retain observable state/resource expectations.
- [ ] **F-AX-018.05 — Implement/prove: persistence and escape**
  - Action: For sticky/filter/slow keys, implement or reuse and verify this exact obligation: persistence and escape. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-018.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for persistence and escape; retain observable state/resource expectations.
- [ ] **F-AX-018.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to sticky/filter/slow keys: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-018.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-018.07 — Integrate into the real consumer and runtime route**
  - Action: Wire sticky/filter/slow keys into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-018.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-018.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for sticky/filter/slow keys as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-018.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-019"></a>
## F-AX-019 — mouse keys

**Original requirement:** keyboard pointer movement, acceleration and buttons

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-019.01 — Reconcile existing mouse keys**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for mouse keys. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: keyboard pointer movement, acceleration and buttons
- [ ] **F-AX-019.02 — Freeze the exact contract for mouse keys**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: keyboard pointer movement, acceleration and buttons. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-019.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-019.03 — Implement/prove: keyboard pointer movement**
  - Action: For mouse keys, implement or reuse and verify this exact obligation: keyboard pointer movement. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-019.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keyboard pointer movement; retain observable state/resource expectations.
- [ ] **F-AX-019.04 — Implement/prove: acceleration and buttons**
  - Action: For mouse keys, implement or reuse and verify this exact obligation: acceleration and buttons. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-019.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for acceleration and buttons; retain observable state/resource expectations.
- [ ] **F-AX-019.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to mouse keys: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-019.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-019.06 — Integrate into the real consumer and runtime route**
  - Action: Wire mouse keys into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-019.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-019.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for mouse keys as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-019.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-020"></a>
## F-AX-020 — switch control

**Original requirement:** scanning groups/actions, timing, device input and safe escape

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-020.01 — Reconcile existing switch control**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for switch control. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: scanning groups/actions, timing, device input and safe escape
- [ ] **F-AX-020.02 — Freeze the exact contract for switch control**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: scanning groups/actions, timing, device input and safe escape. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-020.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-020.03 — Implement/prove: scanning groups/actions**
  - Action: For switch control, implement or reuse and verify this exact obligation: scanning groups/actions. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-020.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for scanning groups/actions; retain observable state/resource expectations.
- [ ] **F-AX-020.04 — Implement/prove: timing**
  - Action: For switch control, implement or reuse and verify this exact obligation: timing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-020.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for timing; retain observable state/resource expectations.
- [ ] **F-AX-020.05 — Implement/prove: device input and safe escape**
  - Action: For switch control, implement or reuse and verify this exact obligation: device input and safe escape. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-020.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for device input and safe escape; retain observable state/resource expectations.
- [ ] **F-AX-020.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to switch control: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-020.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-020.07 — Integrate into the real consumer and runtime route**
  - Action: Wire switch control into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-020.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-020.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for switch control as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-020.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-021"></a>
## F-AX-021 — dwell control

**Original requirement:** configurable dwell targets/timing, feedback and cancellation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-021.01 — Reconcile existing dwell control**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for dwell control. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: configurable dwell targets/timing, feedback and cancellation
- [ ] **F-AX-021.02 — Freeze the exact contract for dwell control**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: configurable dwell targets/timing, feedback and cancellation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-021.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-021.03 — Implement/prove: configurable dwell targets/timing**
  - Action: For dwell control, implement or reuse and verify this exact obligation: configurable dwell targets/timing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-021.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for configurable dwell targets/timing; retain observable state/resource expectations.
- [ ] **F-AX-021.04 — Implement/prove: feedback and cancellation**
  - Action: For dwell control, implement or reuse and verify this exact obligation: feedback and cancellation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-021.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for feedback and cancellation; retain observable state/resource expectations.
- [ ] **F-AX-021.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to dwell control: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-021.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-021.06 — Integrate into the real consumer and runtime route**
  - Action: Wire dwell control into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-021.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-021.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for dwell control as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-021.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-022"></a>
## F-AX-022 — voice control

**Original requirement:** on-device/remote policy, command grammar, confirmation and privacy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-022.01 — Reconcile existing voice control**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for voice control. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: on-device/remote policy, command grammar, confirmation and privacy
- [ ] **F-AX-022.02 — Freeze the exact contract for voice control**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: on-device/remote policy, command grammar, confirmation and privacy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-022.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-022.03 — Implement/prove: on-device/remote policy**
  - Action: For voice control, implement or reuse and verify this exact obligation: on-device/remote policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-022.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for on-device/remote policy; retain observable state/resource expectations.
- [ ] **F-AX-022.04 — Implement/prove: command grammar**
  - Action: For voice control, implement or reuse and verify this exact obligation: command grammar. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-022.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for command grammar; retain observable state/resource expectations.
- [ ] **F-AX-022.05 — Implement/prove: confirmation and privacy**
  - Action: For voice control, implement or reuse and verify this exact obligation: confirmation and privacy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-022.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for confirmation and privacy; retain observable state/resource expectations.
- [ ] **F-AX-022.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to voice control: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-022.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-022.07 — Integrate into the real consumer and runtime route**
  - Action: Wire voice control into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-022.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-022.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for voice control as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-022.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-023"></a>
## F-AX-023 — remapping

**Original requirement:** keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-023.01 — Reconcile existing remapping**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for remapping. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints
- [ ] **F-AX-023.02 — Freeze the exact contract for remapping**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-023.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-023.03 — Implement/prove: keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints**
  - Action: For remapping, implement or reuse and verify this exact obligation: keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-023.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for keys/buttons/gamepad actions mapped per user/app with secure-shortcut constraints; retain observable state/resource expectations.
- [ ] **F-AX-023.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to remapping: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-023.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-023.05 — Integrate into the real consumer and runtime route**
  - Action: Wire remapping into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-023.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-023.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for remapping as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-023.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-024"></a>
## F-AX-024 — touch target policy

**Original requirement:** minimum target, spacing and coarse-pointer adaptation

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-024.01 — Reconcile existing touch target policy**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for touch target policy. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: minimum target, spacing and coarse-pointer adaptation
- [ ] **F-AX-024.02 — Freeze the exact contract for touch target policy**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: minimum target, spacing and coarse-pointer adaptation. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-024.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-024.03 — Implement/prove: minimum target**
  - Action: For touch target policy, implement or reuse and verify this exact obligation: minimum target. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-024.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for minimum target; retain observable state/resource expectations.
- [ ] **F-AX-024.04 — Implement/prove: spacing and coarse-pointer adaptation**
  - Action: For touch target policy, implement or reuse and verify this exact obligation: spacing and coarse-pointer adaptation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-024.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for spacing and coarse-pointer adaptation; retain observable state/resource expectations.
- [ ] **F-AX-024.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to touch target policy: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-024.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-024.06 — Integrate into the real consumer and runtime route**
  - Action: Wire touch target policy into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-024.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-024.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for touch target policy as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-024.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-025"></a>
## F-AX-025 — flashing/seizure safety

**Original requirement:** frequency/luminance limits and reduced-effects enforcement

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-025.01 — Reconcile existing flashing/seizure safety**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for flashing/seizure safety. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: frequency/luminance limits and reduced-effects enforcement
- [ ] **F-AX-025.02 — Freeze the exact contract for flashing/seizure safety**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: frequency/luminance limits and reduced-effects enforcement. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-025.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-025.03 — Implement/prove: frequency/luminance limits and reduced-effects enforcement**
  - Action: For flashing/seizure safety, implement or reuse and verify this exact obligation: frequency/luminance limits and reduced-effects enforcement. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-025.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for frequency/luminance limits and reduced-effects enforcement; retain observable state/resource expectations.
- [ ] **F-AX-025.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to flashing/seizure safety: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-025.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-025.05 — Integrate into the real consumer and runtime route**
  - Action: Wire flashing/seizure safety into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-025.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-025.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for flashing/seizure safety as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-025.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-026"></a>
## F-AX-026 — cognitive accessibility

**Original requirement:** plain language, predictable navigation, time extensions and recoverable workflows

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-026.01 — Reconcile existing cognitive accessibility**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for cognitive accessibility. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: plain language, predictable navigation, time extensions and recoverable workflows
- [ ] **F-AX-026.02 — Freeze the exact contract for cognitive accessibility**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: plain language, predictable navigation, time extensions and recoverable workflows. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-026.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-026.03 — Implement/prove: plain language**
  - Action: For cognitive accessibility, implement or reuse and verify this exact obligation: plain language. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-026.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for plain language; retain observable state/resource expectations.
- [ ] **F-AX-026.04 — Implement/prove: predictable navigation**
  - Action: For cognitive accessibility, implement or reuse and verify this exact obligation: predictable navigation. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-026.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for predictable navigation; retain observable state/resource expectations.
- [ ] **F-AX-026.05 — Implement/prove: time extensions and recoverable workflows**
  - Action: For cognitive accessibility, implement or reuse and verify this exact obligation: time extensions and recoverable workflows. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-026.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for time extensions and recoverable workflows; retain observable state/resource expectations.
- [ ] **F-AX-026.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to cognitive accessibility: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-026.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-026.07 — Integrate into the real consumer and runtime route**
  - Action: Wire cognitive accessibility into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-026.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-026.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for cognitive accessibility as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-026.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-027"></a>
## F-AX-027 — Accessibility Center

**Original requirement:** discover/test/preview/apply/rollback every assistive setting/provider

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-027.01 — Reconcile existing Accessibility Center**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Accessibility Center. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: discover/test/preview/apply/rollback every assistive setting/provider
- [ ] **F-AX-027.02 — Freeze the exact contract for Accessibility Center**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: discover/test/preview/apply/rollback every assistive setting/provider. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-027.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-027.03 — Implement/prove: discover/test/preview/apply/rollback every assistive setting/provider**
  - Action: For Accessibility Center, implement or reuse and verify this exact obligation: discover/test/preview/apply/rollback every assistive setting/provider. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-027.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for discover/test/preview/apply/rollback every assistive setting/provider; retain observable state/resource expectations.
- [ ] **F-AX-027.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Accessibility Center: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-027.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-027.05 — Integrate into the real consumer and runtime route**
  - Action: Wire Accessibility Center into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-027.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-027.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Accessibility Center as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-027.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-028"></a>
## F-AX-028 — locale framework

**Original requirement:** locale data, formats, plural rules, fallback and per-user choice

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-028.01 — Reconcile existing locale framework**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for locale framework. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: locale data, formats, plural rules, fallback and per-user choice
- [ ] **F-AX-028.02 — Freeze the exact contract for locale framework**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: locale data, formats, plural rules, fallback and per-user choice. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-028.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-028.03 — Implement/prove: locale data**
  - Action: For locale framework, implement or reuse and verify this exact obligation: locale data. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-028.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale data; retain observable state/resource expectations.
- [ ] **F-AX-028.04 — Implement/prove: formats**
  - Action: For locale framework, implement or reuse and verify this exact obligation: formats. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-028.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for formats; retain observable state/resource expectations.
- [ ] **F-AX-028.05 — Implement/prove: plural rules**
  - Action: For locale framework, implement or reuse and verify this exact obligation: plural rules. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-028.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for plural rules; retain observable state/resource expectations.
- [ ] **F-AX-028.06 — Implement/prove: fallback and per-user choice**
  - Action: For locale framework, implement or reuse and verify this exact obligation: fallback and per-user choice. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-028.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback and per-user choice; retain observable state/resource expectations.
- [ ] **F-AX-028.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to locale framework: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-028.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-028.08 — Integrate into the real consumer and runtime route**
  - Action: Wire locale framework into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-028.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-028.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for locale framework as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-028.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-029"></a>
## F-AX-029 — translation resource system

**Original requirement:** stable message IDs, extraction, completeness, fallback and pseudo-locales

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-029.01 — Reconcile existing translation resource system**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for translation resource system. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: stable message IDs, extraction, completeness, fallback and pseudo-locales
- [ ] **F-AX-029.02 — Freeze the exact contract for translation resource system**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: stable message IDs, extraction, completeness, fallback and pseudo-locales. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-029.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-029.03 — Implement/prove: stable message IDs**
  - Action: For translation resource system, implement or reuse and verify this exact obligation: stable message IDs. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-029.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for stable message IDs; retain observable state/resource expectations.
- [ ] **F-AX-029.04 — Implement/prove: extraction**
  - Action: For translation resource system, implement or reuse and verify this exact obligation: extraction. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-029.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for extraction; retain observable state/resource expectations.
- [ ] **F-AX-029.05 — Implement/prove: completeness**
  - Action: For translation resource system, implement or reuse and verify this exact obligation: completeness. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-029.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for completeness; retain observable state/resource expectations.
- [ ] **F-AX-029.06 — Implement/prove: fallback and pseudo-locales**
  - Action: For translation resource system, implement or reuse and verify this exact obligation: fallback and pseudo-locales. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-029.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for fallback and pseudo-locales; retain observable state/resource expectations.
- [ ] **F-AX-029.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to translation resource system: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-029.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-029.08 — Integrate into the real consumer and runtime route**
  - Action: Wire translation resource system into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-029.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-029.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for translation resource system as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-029.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-030"></a>
## F-AX-030 — Unicode text

**Original requirement:** UTF-8, grapheme/bidi/line break/normalization and malformed input policy

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-030.01 — Reconcile existing Unicode text**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for Unicode text. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: UTF-8, grapheme/bidi/line break/normalization and malformed input policy
- [ ] **F-AX-030.02 — Freeze the exact contract for Unicode text**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: UTF-8, grapheme/bidi/line break/normalization and malformed input policy. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-030.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-030.03 — Implement/prove: UTF-8**
  - Action: For Unicode text, implement or reuse and verify this exact obligation: UTF-8. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-030.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for UTF-8; retain observable state/resource expectations.
- [ ] **F-AX-030.04 — Implement/prove: grapheme/bidi/line break/normalization and malformed input policy**
  - Action: For Unicode text, implement or reuse and verify this exact obligation: grapheme/bidi/line break/normalization and malformed input policy. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-030.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for grapheme/bidi/line break/normalization and malformed input policy; retain observable state/resource expectations.
- [ ] **F-AX-030.05 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to Unicode text: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-030.04.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-030.06 — Integrate into the real consumer and runtime route**
  - Action: Wire Unicode text into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-030.05.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-030.07 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for Unicode text as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-030.06.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-031"></a>
## F-AX-031 — font fallback/shaping

**Original requirement:** script/language coverage, emoji, missing-glyph diagnostics and cache keys

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-031.01 — Reconcile existing font fallback/shaping**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for font fallback/shaping. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: script/language coverage, emoji, missing-glyph diagnostics and cache keys
- [ ] **F-AX-031.02 — Freeze the exact contract for font fallback/shaping**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: script/language coverage, emoji, missing-glyph diagnostics and cache keys. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-031.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-031.03 — Implement/prove: script/language coverage**
  - Action: For font fallback/shaping, implement or reuse and verify this exact obligation: script/language coverage. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-031.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for script/language coverage; retain observable state/resource expectations.
- [ ] **F-AX-031.04 — Implement/prove: emoji**
  - Action: For font fallback/shaping, implement or reuse and verify this exact obligation: emoji. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-031.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for emoji; retain observable state/resource expectations.
- [ ] **F-AX-031.05 — Implement/prove: missing-glyph diagnostics and cache keys**
  - Action: For font fallback/shaping, implement or reuse and verify this exact obligation: missing-glyph diagnostics and cache keys. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-031.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for missing-glyph diagnostics and cache keys; retain observable state/resource expectations.
- [ ] **F-AX-031.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to font fallback/shaping: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-031.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-031.07 — Integrate into the real consumer and runtime route**
  - Action: Wire font fallback/shaping into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-031.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-031.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for font fallback/shaping as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-031.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-032"></a>
## F-AX-032 — right-to-left layout

**Original requirement:** bidi content plus mirrored shell/widgets and mixed-direction tests

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-032.01 — Reconcile existing right-to-left layout**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for right-to-left layout. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: bidi content plus mirrored shell/widgets and mixed-direction tests
- [ ] **F-AX-032.02 — Freeze the exact contract for right-to-left layout**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: bidi content plus mirrored shell/widgets and mixed-direction tests. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-032.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-032.03 — Implement/prove: bidi content plus mirrored shell/widgets and mixed-direction tests**
  - Action: For right-to-left layout, implement or reuse and verify this exact obligation: bidi content plus mirrored shell/widgets and mixed-direction tests. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-032.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bidi content plus mirrored shell/widgets and mixed-direction tests; retain observable state/resource expectations.
- [ ] **F-AX-032.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to right-to-left layout: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-032.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-032.05 — Integrate into the real consumer and runtime route**
  - Action: Wire right-to-left layout into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-032.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-032.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for right-to-left layout as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-032.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-033"></a>
## F-AX-033 — timezone/calendar formats

**Original requirement:** locale-aware wall time, calendars, first day, DST and user override

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-033.01 — Reconcile existing timezone/calendar formats**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for timezone/calendar formats. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: locale-aware wall time, calendars, first day, DST and user override
- [ ] **F-AX-033.02 — Freeze the exact contract for timezone/calendar formats**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: locale-aware wall time, calendars, first day, DST and user override. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-033.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-033.03 — Implement/prove: locale-aware wall time**
  - Action: For timezone/calendar formats, implement or reuse and verify this exact obligation: locale-aware wall time. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-033.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for locale-aware wall time; retain observable state/resource expectations.
- [ ] **F-AX-033.04 — Implement/prove: calendars**
  - Action: For timezone/calendar formats, implement or reuse and verify this exact obligation: calendars. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-033.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for calendars; retain observable state/resource expectations.
- [ ] **F-AX-033.05 — Implement/prove: first day**
  - Action: For timezone/calendar formats, implement or reuse and verify this exact obligation: first day. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-033.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for first day; retain observable state/resource expectations.
- [ ] **F-AX-033.06 — Implement/prove: DST and user override**
  - Action: For timezone/calendar formats, implement or reuse and verify this exact obligation: DST and user override. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-033.05.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for DST and user override; retain observable state/resource expectations.
- [ ] **F-AX-033.07 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to timezone/calendar formats: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-033.06.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-033.08 — Integrate into the real consumer and runtime route**
  - Action: Wire timezone/calendar formats into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-033.07.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-033.09 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for timezone/calendar formats as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-033.08.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-034"></a>
## F-AX-034 — units/number formats

**Original requirement:** decimal/grouping/currency/measurement preferences and round-trip parsing

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-034.01 — Reconcile existing units/number formats**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for units/number formats. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: decimal/grouping/currency/measurement preferences and round-trip parsing
- [ ] **F-AX-034.02 — Freeze the exact contract for units/number formats**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: decimal/grouping/currency/measurement preferences and round-trip parsing. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-034.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-034.03 — Implement/prove: decimal/grouping/currency/measurement preferences and round-trip parsing**
  - Action: For units/number formats, implement or reuse and verify this exact obligation: decimal/grouping/currency/measurement preferences and round-trip parsing. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-034.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for decimal/grouping/currency/measurement preferences and round-trip parsing; retain observable state/resource expectations.
- [ ] **F-AX-034.04 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to units/number formats: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-034.03.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-034.05 — Integrate into the real consumer and runtime route**
  - Action: Wire units/number formats into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-034.04.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-034.06 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for units/number formats as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-034.05.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="f-ax-035"></a>
## F-AX-035 — pseudo-localization gate

**Original requirement:** expansion, bidi, missing translation and hard-coded string detection

**Source:** [docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md](../../docs/program/research/CANONICAL_COMPLETE_PRODUCT_FEATURE_CATALOG_2026-08-22.md).

**Baseline evidence:** `PLANNED_UNPROVED` from run `34014513857`; this is a dated evidence label, not an instruction to rebuild working code.

**Recorded gaps:** no feature-specific implementation receipt is joined.

### Execution steps

- [ ] **F-AX-035.01 — Reconcile existing pseudo-localization gate**
  - Action: Trace the actual producer, consumer, build/image edge and available tests for pseudo-localization gate. Reuse matching behavior; record exactly what is absent, partial, unreachable or already proved. Baseline evidence label: PLANNED_UNPROVED.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-and-evidence comparison against the complete requirement: expansion, bidi, missing translation and hard-coded string detection
- [ ] **F-AX-035.02 — Freeze the exact contract for pseudo-localization gate**
  - Action: Specify input/output types, versions, state transitions, owner, resource bounds, required dependencies and cleanup for: expansion, bidi, missing translation and hard-coded string detection. Resolve referenced research contracts and provider versions before code; unresolved dependencies keep this package blocked.
  - Requires: F-AX-035.01.
  - Acceptance: A reviewed producer/consumer contract and positive/negative scenario per acceptance clause. Do not substitute a generic checklist for the feature semantics.
- [ ] **F-AX-035.03 — Implement/prove: expansion**
  - Action: For pseudo-localization gate, implement or reuse and verify this exact obligation: expansion. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-035.02.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for expansion; retain observable state/resource expectations.
- [ ] **F-AX-035.04 — Implement/prove: bidi**
  - Action: For pseudo-localization gate, implement or reuse and verify this exact obligation: bidi. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-035.03.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for bidi; retain observable state/resource expectations.
- [ ] **F-AX-035.05 — Implement/prove: missing translation and hard-coded string detection**
  - Action: For pseudo-localization gate, implement or reuse and verify this exact obligation: missing translation and hard-coded string detection. Preserve the complete parent requirement; use the owning phase recipe and original research contracts for the detailed behavior.
  - Requires: F-AX-035.04.
  - Acceptance: A runnable positive case and a failing-before/fixed-after or planted-negative case for missing translation and hard-coded string detection; retain observable state/resource expectations.
- [ ] **F-AX-035.06 — Refute failure and boundary behavior**
  - Action: Apply the relevant cases from this domain matrix to pseudo-localization gate: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion. Explain exclusions by scope; add feature-specific cases from its original contract and current gaps.
  - Requires: F-AX-035.05.
  - Acceptance: Each applicable failure has an explicit expected outcome, bounded resource use and independent post-state/cleanup check.
- [ ] **F-AX-035.07 — Integrate into the real consumer and runtime route**
  - Action: Wire pseudo-localization gate into its declared caller, source/build manifest, ABI/IDL/registry and real user or kernel route. Keep proven fallback behavior until replacement conditions are met.
  - Requires: F-AX-035.06.
  - Acceptance: The shipped artifact actually reaches the behavior. A helper, isolated demo or source file alone is insufficient.
- [ ] **F-AX-035.08 — Qualify the declared profiles and retain rollback proof**
  - Action: Run the required host/build/QEMU/physical/interaction/performance lanes for pseudo-localization gate as specified by its contract and the programme proof rules. Name exact applicable profiles, image/source/toolchain identities, skips and recovery/removal conditions.
  - Requires: F-AX-035.07.
  - Acceptance: Promote only the observed scope; required physical proof cannot be replaced by simulation. Reuse unchanged valid evidence; do not rerun unrelated gates merely for a documentation change.

<a id="c-vx-02"></a>
## C-VX-02 — Versioned semantic token schema

**Original requirement:** Versioned semantic token schema

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 111.

### Preserved original contract

**Depends on:** VX-00.

**Deliver:** generated color/type/spacing/radius/elevation/motion/layer/density
schema with defaults, ranges, fallback and semantic relationships.

**Invariants:** raw literals are confined to token artifacts; unknown mandatory
fields reject; semantic state colors do not silently follow theme accent;
focus/selection/error roles remain distinguishable.

**Proof:** source scan, schema round-trip, missing/duplicate/wrong-type/unknown-
required cases, every accent contrast pair and one-literal mutation.

### Execution steps

- [ ] **C-VX-02.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-02.02 — Resolve this contract's exact dependencies**
  - Action: VX-00. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-02.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-02.03 — Deliver — Versioned semantic token schema**
  - Action: generated color/type/spacing/radius/elevation/motion/layer/density schema with defaults, ranges, fallback and semantic relationships.
  - Requires: C-VX-02.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-02.
- [ ] **C-VX-02.04 — Invariants — Versioned semantic token schema**
  - Action: raw literals are confined to token artifacts; unknown mandatory fields reject; semantic state colors do not silently follow theme accent; focus/selection/error roles remain distinguishable.
  - Requires: C-VX-02.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-02.
- [ ] **C-VX-02.05 — Proof — Versioned semantic token schema**
  - Action: source scan, schema round-trip, missing/duplicate/wrong-type/unknown- required cases, every accent contrast pair and one-literal mutation.
  - Requires: C-VX-02.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-02.
- [ ] **C-VX-02.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-02. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-02.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-03"></a>
## C-VX-03 — Atomic theme and appearance service

**Original requirement:** Atomic theme and appearance service

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 125.

### Preserved original contract

**Depends on:** VX-02.

**Deliver:** per-user theme profile, accent, contrast, transparency, density,
wallpaper and scale settings; preview/apply/revert; signed asset manifest;
atomic persistence and rollback.

**Invariants:** partial theme never publishes; failed preview expires; broken
theme falls back without losing user data; lock/recovery has safe built-in
theme.

**Proof:** failure-inject every write and publish step; reboot during apply;
malformed assets; contrast-invalid theme; provider death during preview.

### Execution steps

- [ ] **C-VX-03.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-03.02 — Resolve this contract's exact dependencies**
  - Action: VX-02. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-03.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-03.03 — Deliver — Atomic theme and appearance service**
  - Action: per-user theme profile, accent, contrast, transparency, density, wallpaper and scale settings; preview/apply/revert; signed asset manifest; atomic persistence and rollback.
  - Requires: C-VX-03.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-03.
- [ ] **C-VX-03.04 — Invariants — Atomic theme and appearance service**
  - Action: partial theme never publishes; failed preview expires; broken theme falls back without losing user data; lock/recovery has safe built-in theme.
  - Requires: C-VX-03.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-03.
- [ ] **C-VX-03.05 — Proof — Atomic theme and appearance service**
  - Action: failure-inject every write and publish step; reboot during apply; malformed assets; contrast-invalid theme; provider death during preview.
  - Requires: C-VX-03.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-03.
- [ ] **C-VX-03.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-03. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-03.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-04"></a>
## C-VX-04 — Scalable text, shaping and glyph cache

**Original requirement:** Scalable text, shaping and glyph cache

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 140.

### Preserved original contract

**Depends on:** VX-02.

**Deliver:** UI/reading/code/terminal roles, scalable rasterization, metrics,
fallback, shaping, bidirectionality, combining marks, selection/caret hit tests,
bounded cache and deterministic eviction.

**Invariants:** no zero/negative metrics; terminal monospace stays exact;
fallback cannot recurse forever; cache exhaustion degrades, never corrupts;
text scale is independent of display scale.

**Proof:** multilingual corpus, RTL, combining/emoji fallback, huge glyph churn,
200%/300% text, cache failure injection, browser/editor/terminal differential.

### Execution steps

- [ ] **C-VX-04.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-04.02 — Resolve this contract's exact dependencies**
  - Action: VX-02. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-04.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-04.03 — Deliver — Scalable text, shaping and glyph cache**
  - Action: UI/reading/code/terminal roles, scalable rasterization, metrics, fallback, shaping, bidirectionality, combining marks, selection/caret hit tests, bounded cache and deterministic eviction.
  - Requires: C-VX-04.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-04.
- [ ] **C-VX-04.04 — Invariants — Scalable text, shaping and glyph cache**
  - Action: no zero/negative metrics; terminal monospace stays exact; fallback cannot recurse forever; cache exhaustion degrades, never corrupts; text scale is independent of display scale.
  - Requires: C-VX-04.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-04.
- [ ] **C-VX-04.05 — Proof — Scalable text, shaping and glyph cache**
  - Action: multilingual corpus, RTL, combining/emoji fallback, huge glyph churn, 200%/300% text, cache failure injection, browser/editor/terminal differential.
  - Requires: C-VX-04.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-04.
- [ ] **C-VX-04.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-04. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-04.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-05"></a>
## C-VX-05 — Semantic icon and asset pipeline

**Original requirement:** Semantic icon and asset pipeline

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 155.

### Preserved original contract

**Depends on:** VX-02, VX-03.

**Deliver:** named icons with optical sizes/states/directionality, bounded build
artifact, signed app assets and decoder-worker boundary.

**Invariants:** atlas position is not API; missing icons get explicit fallback;
untrusted images never parse in shell; selected/disabled/attention states keep
meaning at every scale.

**Proof:** missing/duplicate name, oversized/decompression bomb, RTL icon,
1x/1.25x/1.5x/2x/3x snapshots, decoder crash and fallback rendering.

### Execution steps

- [ ] **C-VX-05.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-05.02 — Resolve this contract's exact dependencies**
  - Action: VX-02, VX-03. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-05.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-05.03 — Deliver — Semantic icon and asset pipeline**
  - Action: named icons with optical sizes/states/directionality, bounded build artifact, signed app assets and decoder-worker boundary.
  - Requires: C-VX-05.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-05.
- [ ] **C-VX-05.04 — Invariants — Semantic icon and asset pipeline**
  - Action: atlas position is not API; missing icons get explicit fallback; untrusted images never parse in shell; selected/disabled/attention states keep meaning at every scale.
  - Requires: C-VX-05.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-05.
- [ ] **C-VX-05.05 — Proof — Semantic icon and asset pipeline**
  - Action: missing/duplicate name, oversized/decompression bomb, RTL icon, 1x/1.25x/1.5x/2x/3x snapshots, decoder crash and fallback rendering.
  - Requires: C-VX-05.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-05.
- [ ] **C-VX-05.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-05. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-05.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-06"></a>
## C-VX-06 — Motion engine and reduced-motion policy

**Original requirement:** Motion engine and reduced-motion policy

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 169.

### Preserved original contract

**Depends on:** VX-02.

**Deliver:** named curves/durations, monotonic clock scheduling, interrupt/
reverse/retarget behavior, damage generation and static/reduced alternatives.

**Invariants:** PIT/frequency changes do not alter duration; final state is exact;
interruption cannot leak grabs/modal/focus; no infinite attention animation;
reduced motion removes nonessential travel.

**Proof:** fixed tick vectors, dropped-frame simulation, mid-animation reverse,
window death, reduced-motion traces and damage-mutation check.

### Execution steps

- [ ] **C-VX-06.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-06.02 — Resolve this contract's exact dependencies**
  - Action: VX-02. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-06.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-06.03 — Deliver — Motion engine and reduced-motion policy**
  - Action: named curves/durations, monotonic clock scheduling, interrupt/ reverse/retarget behavior, damage generation and static/reduced alternatives.
  - Requires: C-VX-06.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-06.
- [ ] **C-VX-06.04 — Invariants — Motion engine and reduced-motion policy**
  - Action: PIT/frequency changes do not alter duration; final state is exact; interruption cannot leak grabs/modal/focus; no infinite attention animation; reduced motion removes nonessential travel.
  - Requires: C-VX-06.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-06.
- [ ] **C-VX-06.05 — Proof — Motion engine and reduced-motion policy**
  - Action: fixed tick vectors, dropped-frame simulation, mid-animation reverse, window death, reduced-motion traces and damage-mutation check.
  - Requires: C-VX-06.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-06.
- [ ] **C-VX-06.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-06. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-06.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-07"></a>
## C-VX-07 — Accessibility tree, assistive providers and control center

**Original requirement:** Accessibility tree, assistive providers and control center

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 185.

### Preserved original contract

**Depends on:** VX-02; process/IPC handle foundation; `DA-09` display provider
for magnification/color transforms, `DA-10` input provider for remapping and
alternative input, `DA-20C` settings service for profiles, and `DA-14`
AudioServer/provider seam for speech and caption-audio integration. Early host
proof may use deterministic fake providers, but target completion requires the
named canonical contracts.

**Deliver:** versioned tree updates with stable IDs, roles, names, descriptions,
states, values, bounds, text ranges, actions and privacy/redaction markers;
screen-reader output with speech and braille provider interfaces; magnifier,
cursor scale, large-text, high-contrast and color-filter services; caption and
transcript routing; sticky/filter/slow keys, remapping, switch control and dwell
input; and an Accessibility Center that previews, persists, reverts and reports
availability of every feature.

**Invariants:** tree is acyclic and owned; updates are bounded/atomic; peer death
removes subtree; action checks current handle and generation; protected content
is not exposed; assistive-provider death leaves keyboard recovery and visible
fallback; lock/session/privacy policy redacts speech, braille, captions and
history; input transformations are ordered, reversible and never trap the user
without an emergency reset path.

**Proof:** cycle/alias/stale ID, oversized update, peer death, focus/action replay,
redaction, a foundation semantic-node fixture, full screen-reader action
journey, speech/braille provider crash, magnifier and color-filter screenshots,
live captions, switch/dwell control, sticky/filter/slow-key timing vectors,
remapping conflict, lock privacy, reboot persistence, failed preview rollback
and emergency recovery with no pointer device.

### Execution steps

- [ ] **C-VX-07.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-07.02 — Resolve this contract's exact dependencies**
  - Action: VX-02; process/IPC handle foundation; `DA-09` display provider for magnification/color transforms, `DA-10` input provider for remapping and alternative input, `DA-20C` settings service for profiles, and `DA-14` AudioServer/provider seam for speech and caption-audio integration. Early host proof may use deterministic fake providers, but target completion requires the named canonical contracts. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-07.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-07.03 — Deliver — Accessibility tree, assistive providers and control center**
  - Action: versioned tree updates with stable IDs, roles, names, descriptions, states, values, bounds, text ranges, actions and privacy/redaction markers; screen-reader output with speech and braille provider interfaces; magnifier, cursor scale, large-text, high-contrast and color-filter services; caption and transcript routing; sticky/filter/slow keys, remapping, switch control and dwell input; and an Accessibility Center that previews, persists, reverts and reports availability of every feature.
  - Requires: C-VX-07.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-07.
- [ ] **C-VX-07.04 — Invariants — Accessibility tree, assistive providers and control center**
  - Action: tree is acyclic and owned; updates are bounded/atomic; peer death removes subtree; action checks current handle and generation; protected content is not exposed; assistive-provider death leaves keyboard recovery and visible fallback; lock/session/privacy policy redacts speech, braille, captions and history; input transformations are ordered, reversible and never trap the user without an emergency reset path.
  - Requires: C-VX-07.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-07.
- [ ] **C-VX-07.05 — Proof — Accessibility tree, assistive providers and control center**
  - Action: cycle/alias/stale ID, oversized update, peer death, focus/action replay, redaction, a foundation semantic-node fixture, full screen-reader action journey, speech/braille provider crash, magnifier and color-filter screenshots, live captions, switch/dwell control, sticky/filter/slow-key timing vectors, remapping conflict, lock privacy, reboot persistence, failed preview rollback and emergency recovery with no pointer device.
  - Requires: C-VX-07.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-07.
- [ ] **C-VX-07.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-07. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-07.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-08"></a>
## C-VX-08 — Primitive component library

**Original requirement:** Primitive component library

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 216.

### Preserved original contract

**Depends on:** VX-02, VX-04, VX-05, VX-07.

**Deliver:** the complete primitive set named in the clean-room plan, each with
layout, rendering, events, semantics and state model.

**Invariants:** disabled cannot fire; busy actions deduplicate or reject;
focus-visible differs from pointer hover; invalid state is named and announced;
minimum touch target and clipping rules hold.

**Proof:** state matrix screenshot and semantic snapshot for every primitive;
screen-reader action coverage over that snapshot; keyboard/pointer/touch
actions; hostile labels; locale expansion; mutation of every required state.

### Execution steps

- [ ] **C-VX-08.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-08.02 — Resolve this contract's exact dependencies**
  - Action: VX-02, VX-04, VX-05, VX-07. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-08.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-08.03 — Deliver — Primitive component library**
  - Action: the complete primitive set named in the clean-room plan, each with layout, rendering, events, semantics and state model.
  - Requires: C-VX-08.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-08.
- [ ] **C-VX-08.04 — Invariants — Primitive component library**
  - Action: disabled cannot fire; busy actions deduplicate or reject; focus-visible differs from pointer hover; invalid state is named and announced; minimum touch target and clipping rules hold.
  - Requires: C-VX-08.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-08.
- [ ] **C-VX-08.05 — Proof — Primitive component library**
  - Action: state matrix screenshot and semantic snapshot for every primitive; screen-reader action coverage over that snapshot; keyboard/pointer/touch actions; hostile labels; locale expansion; mutation of every required state.
  - Requires: C-VX-08.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-08.
- [ ] **C-VX-08.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-08. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-08.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-09"></a>
## C-VX-09 — Responsive layout, scale and localization

**Original requirement:** Responsive layout, scale and localization

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 231.

### Preserved original contract

**Depends on:** VX-04, VX-08.

**Deliver:** constraints, intrinsic sizing, wrapping, scroll/virtualization,
per-monitor fractional scale, RTL mirroring and locale/time/number/unit services.

**Invariants:** checked geometry; no critical action clips at 200% text;
resizing never yields negative/overflow dimensions; focus order follows logical
order; virtualization preserves semantic identity.

**Proof:** narrow/wide/tall/4K matrices, 30–100% longer strings, RTL, rapid scale
and monitor transitions, huge list and integer-boundary fuzzing.

### Execution steps

- [ ] **C-VX-09.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-09.02 — Resolve this contract's exact dependencies**
  - Action: VX-04, VX-08. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-09.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-09.03 — Deliver — Responsive layout, scale and localization**
  - Action: constraints, intrinsic sizing, wrapping, scroll/virtualization, per-monitor fractional scale, RTL mirroring and locale/time/number/unit services.
  - Requires: C-VX-09.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-09.
- [ ] **C-VX-09.04 — Invariants — Responsive layout, scale and localization**
  - Action: checked geometry; no critical action clips at 200% text; resizing never yields negative/overflow dimensions; focus order follows logical order; virtualization preserves semantic identity.
  - Requires: C-VX-09.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-09.
- [ ] **C-VX-09.05 — Proof — Responsive layout, scale and localization**
  - Action: narrow/wide/tall/4K matrices, 30–100% longer strings, RTL, rapid scale and monitor transitions, huge list and integer-boundary fuzzing.
  - Requires: C-VX-09.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-09.
- [ ] **C-VX-09.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-09. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-09.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="c-vx-10"></a>
## C-VX-10 — Focus, shortcuts and input ownership

**Original requirement:** Focus, shortcuts and input ownership

**Source:** [docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md](../../docs/program/research/VISUAL_BROWSER_AND_APP_EXPERIENCE_CONTRACT_BACKLOG_2026-08-21.md), line 245.

### Preserved original contract

**Depends on:** VX-07, VX-08, input service.

**Deliver:** keyboard traversal, focus scopes, modal trap/escape, accelerators,
global shortcuts, pointer capture, drag ownership, IME and shortcut discovery.

**Invariants:** one focus owner per seat; only session policy owns global keys;
peer death releases capture; hidden/disabled controls cannot focus; signal/input
events are bounded and loss-accounted.

**Proof:** focus-order replay, conflicting shortcut, modal nesting, app crash
while grabbed, device disconnect, IME composition and keyboard-only journeys.

### Execution steps

- [ ] **C-VX-10.01 — Reconcile existing behavior with the original contract**
  - Action: Read the complete source contract and its document-level rules; trace its existing producer, consumer and retained evidence. Record the exact unmet obligations before changing code.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A source-bound comparison of every original obligation, preserving working implementations and unresolved proof gaps.
- [ ] **C-VX-10.02 — Resolve this contract's exact dependencies**
  - Action: VX-07, VX-08, input service. Bind every named/narrative prerequisite to an exported interface and proof. Preserve explicit source requirements; if a bootstrap cycle requires splitting a contract, resolve and record that split under D-02 before implementation.
  - Requires: C-VX-10.01.
  - Acceptance: All prerequisite versions, providers, ownership and evidence are identified. A complete source prerequisite cannot silently be downgraded to a mock.
- [ ] **C-VX-10.03 — Deliver — Focus, shortcuts and input ownership**
  - Action: keyboard traversal, focus scopes, modal trap/escape, accelerators, global shortcuts, pointer capture, drag ownership, IME and shortcut discovery.
  - Requires: C-VX-10.02.
  - Acceptance: Satisfy every obligation in the preserved Deliver field for VX-10.
- [ ] **C-VX-10.04 — Invariants — Focus, shortcuts and input ownership**
  - Action: one focus owner per seat; only session policy owns global keys; peer death releases capture; hidden/disabled controls cannot focus; signal/input events are bounded and loss-accounted.
  - Requires: C-VX-10.03.
  - Acceptance: Satisfy every obligation in the preserved Invariants field for VX-10.
- [ ] **C-VX-10.05 — Proof — Focus, shortcuts and input ownership**
  - Action: focus-order replay, conflicting shortcut, modal nesting, app crash while grabbed, device disconnect, IME composition and keyboard-only journeys.
  - Requires: C-VX-10.04.
  - Acceptance: Satisfy every obligation in the preserved Proof field for VX-10.
- [ ] **C-VX-10.06 — Close the complete contract with bounded claims**
  - Action: Join the exact implementation, deterministic/target proof and rollback/removal obligations for VX-10. Shared work may satisfy multiple packages only when each complete acceptance contract is checked.
  - Requires: C-VX-10.05.
  - Acceptance: All original fields below are satisfied; preserve every unresolved physical, independent-review and policy boundary.

<a id="t-input-014"></a>
## T-INPUT-014 — switch/dwell/accessibility devices

**Original requirement:** semantic actions, timing and remapping

**Source:** [docs/program/DRIVERS.md](../../docs/program/DRIVERS.md), line 168.

### Execution steps

- [ ] **T-INPUT-014.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve switch/dwell/accessibility devices to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-INPUT-014.02 — Specify the complete target boundary**
  - Action: switch/dwell/accessibility devices must supply: semantic actions, timing and remapping. Define identify/match/admit/reserve/start/online/quiesce/reset/recover/remove; exact register/DMA/IRQ/resource and firmware ownership. Bind each prerequisite provider and contract before implementation.
  - Requires: T-INPUT-014.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-INPUT-014.03 — Implement the exact target behavior**
  - Action: Implement or reuse switch/dwell/accessibility devices through the shared platform contract, delivering every part of: semantic actions, timing and remapping. Do not fork a duplicate subsystem for this row.
  - Requires: T-INPUT-014.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-INPUT-014.04 — Prove target-specific failure and recovery**
  - Action: Require a pinned primary specification, fake-device or simulator transcript, every resource-acquisition failure, reset/removal recovery and exact physical-device qualification where support is claimed. Relevant domain cases: multi-device interleaving; lost release/modifier state; repeat and queue overflow; wrong focus/grab owner; disconnect mid-event; malformed HID reports; denied global capture.
  - Requires: T-INPUT-014.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for switch/dwell/accessibility devices.
- [ ] **T-INPUT-014.05 — Qualify and retain this target's own result**
  - Action: Bind switch/dwell/accessibility devices to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-INPUT-014.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-077"></a>
## T-SVC-077 — Appearance/Theme Service

**Original requirement:** semantic tokens, preview, atomic apply and rollback

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 97.

### Execution steps

- [ ] **T-SVC-077.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Appearance/Theme Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-077.02 — Specify the complete target boundary**
  - Action: Appearance/Theme Service must supply: semantic tokens, preview, atomic apply and rollback. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-077.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-077.03 — Implement the exact target behavior**
  - Action: Implement or reuse Appearance/Theme Service through the shared platform contract, delivering every part of: semantic tokens, preview, atomic apply and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-077.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-077.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-077.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Appearance/Theme Service.
- [ ] **T-SVC-077.05 — Qualify and retain this target's own result**
  - Action: Bind Appearance/Theme Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-077.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-078"></a>
## T-SVC-078 — Font/Shaping Service

**Original requirement:** font discovery, shaping, fallback, caches and licensing

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 98.

### Execution steps

- [ ] **T-SVC-078.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Font/Shaping Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-078.02 — Specify the complete target boundary**
  - Action: Font/Shaping Service must supply: font discovery, shaping, fallback, caches and licensing. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-078.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-078.03 — Implement the exact target behavior**
  - Action: Implement or reuse Font/Shaping Service through the shared platform contract, delivering every part of: font discovery, shaping, fallback, caches and licensing. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-078.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-078.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-078.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Font/Shaping Service.
- [ ] **T-SVC-078.05 — Qualify and retain this target's own result**
  - Action: Bind Font/Shaping Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-078.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-079"></a>
## T-SVC-079 — Icon/Asset Service

**Original requirement:** semantic/versioned assets, scale/theme variants

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 99.

### Execution steps

- [ ] **T-SVC-079.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Icon/Asset Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-079.02 — Specify the complete target boundary**
  - Action: Icon/Asset Service must supply: semantic/versioned assets, scale/theme variants. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-079.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-079.03 — Implement the exact target behavior**
  - Action: Implement or reuse Icon/Asset Service through the shared platform contract, delivering every part of: semantic/versioned assets, scale/theme variants. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-079.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-079.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-079.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Icon/Asset Service.
- [ ] **T-SVC-079.05 — Qualify and retain this target's own result**
  - Action: Bind Icon/Asset Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-079.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-080"></a>
## T-SVC-080 — Localization Service

**Original requirement:** catalogues, locale formats, fallback and live updates

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 100.

### Execution steps

- [ ] **T-SVC-080.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Localization Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-080.02 — Specify the complete target boundary**
  - Action: Localization Service must supply: catalogues, locale formats, fallback and live updates. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-080.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-080.03 — Implement the exact target behavior**
  - Action: Implement or reuse Localization Service through the shared platform contract, delivering every part of: catalogues, locale formats, fallback and live updates. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-080.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-080.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-080.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Localization Service.
- [ ] **T-SVC-080.05 — Qualify and retain this target's own result**
  - Action: Bind Localization Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-080.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-081"></a>
## T-SVC-081 — IME/Text Input Service

**Original requirement:** composition, candidates, secure fields and locale

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 101.

### Execution steps

- [ ] **T-SVC-081.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve IME/Text Input Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-081.02 — Specify the complete target boundary**
  - Action: IME/Text Input Service must supply: composition, candidates, secure fields and locale. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-081.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-081.03 — Implement the exact target behavior**
  - Action: Implement or reuse IME/Text Input Service through the shared platform contract, delivering every part of: composition, candidates, secure fields and locale. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-081.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-081.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-081.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for IME/Text Input Service.
- [ ] **T-SVC-081.05 — Qualify and retain this target's own result**
  - Action: Bind IME/Text Input Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-081.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-082"></a>
## T-SVC-082 — Accessibility Tree Broker

**Original requirement:** semantic snapshots/events/actions and privacy

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 102.

### Execution steps

- [ ] **T-SVC-082.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Accessibility Tree Broker to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-082.02 — Specify the complete target boundary**
  - Action: Accessibility Tree Broker must supply: semantic snapshots/events/actions and privacy. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-082.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-082.03 — Implement the exact target behavior**
  - Action: Implement or reuse Accessibility Tree Broker through the shared platform contract, delivering every part of: semantic snapshots/events/actions and privacy. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-082.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-082.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-082.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Accessibility Tree Broker.
- [ ] **T-SVC-082.05 — Qualify and retain this target's own result**
  - Action: Bind Accessibility Tree Broker to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-082.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-083"></a>
## T-SVC-083 — Screen Reader/Speech Service

**Original requirement:** focus/navigation/speech/braille providers

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 103.

### Execution steps

- [ ] **T-SVC-083.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Screen Reader/Speech Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-083.02 — Specify the complete target boundary**
  - Action: Screen Reader/Speech Service must supply: focus/navigation/speech/braille providers. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-083.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-083.03 — Implement the exact target behavior**
  - Action: Implement or reuse Screen Reader/Speech Service through the shared platform contract, delivering every part of: focus/navigation/speech/braille providers. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-083.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-083.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-083.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Screen Reader/Speech Service.
- [ ] **T-SVC-083.05 — Qualify and retain this target's own result**
  - Action: Bind Screen Reader/Speech Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-083.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-084"></a>
## T-SVC-084 — Magnifier/Visual Assistance

**Original requirement:** zoom, cursor, filters, large text and multi-display

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 104.

### Execution steps

- [ ] **T-SVC-084.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Magnifier/Visual Assistance to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-084.02 — Specify the complete target boundary**
  - Action: Magnifier/Visual Assistance must supply: zoom, cursor, filters, large text and multi-display. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-084.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-084.03 — Implement the exact target behavior**
  - Action: Implement or reuse Magnifier/Visual Assistance through the shared platform contract, delivering every part of: zoom, cursor, filters, large text and multi-display. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-084.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-084.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-084.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Magnifier/Visual Assistance.
- [ ] **T-SVC-084.05 — Qualify and retain this target's own result**
  - Action: Bind Magnifier/Visual Assistance to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-084.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-085"></a>
## T-SVC-085 — Captions/Assistive Input Service

**Original requirement:** captions, switch/dwell, sticky/filter/slow keys

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 105.

### Execution steps

- [ ] **T-SVC-085.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Captions/Assistive Input Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-085.02 — Specify the complete target boundary**
  - Action: Captions/Assistive Input Service must supply: captions, switch/dwell, sticky/filter/slow keys. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-085.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-085.03 — Implement the exact target behavior**
  - Action: Implement or reuse Captions/Assistive Input Service through the shared platform contract, delivering every part of: captions, switch/dwell, sticky/filter/slow keys. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-085.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-085.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-085.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Captions/Assistive Input Service.
- [ ] **T-SVC-085.05 — Qualify and retain this target's own result**
  - Action: Bind Captions/Assistive Input Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-085.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-086"></a>
## T-SVC-086 — Color/Profile Service

**Original requirement:** display profiles, contrast, filters and calibration

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 106.

### Execution steps

- [ ] **T-SVC-086.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Color/Profile Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-086.02 — Specify the complete target boundary**
  - Action: Color/Profile Service must supply: display profiles, contrast, filters and calibration. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-086.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-086.03 — Implement the exact target behavior**
  - Action: Implement or reuse Color/Profile Service through the shared platform contract, delivering every part of: display profiles, contrast, filters and calibration. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-086.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-086.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-086.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Color/Profile Service.
- [ ] **T-SVC-086.05 — Qualify and retain this target's own result**
  - Action: Bind Color/Profile Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-086.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-svc-096"></a>
## T-SVC-096 — Text-to-Speech Service

**Original requirement:** voices, queue, interruption and accessibility binding

**Source:** [docs/program/SERVICES.md](../../docs/program/SERVICES.md), line 118.

### Execution steps

- [ ] **T-SVC-096.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Text-to-Speech Service to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-SVC-096.02 — Specify the complete target boundary**
  - Action: Text-to-Speech Service must supply: voices, queue, interruption and accessibility binding. Define descriptor/admission/dependency-ready/start/ready/degraded/recover/quarantine/stop; authenticated versioned endpoints, quotas and reverse teardown. Bind each prerequisite provider and contract before implementation.
  - Requires: T-SVC-096.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-SVC-096.03 — Implement the exact target behavior**
  - Action: Implement or reuse Text-to-Speech Service through the shared platform contract, delivering every part of: voices, queue, interruption and accessibility binding. Do not fork a duplicate subsystem for this row.
  - Requires: T-SVC-096.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-SVC-096.04 — Prove target-specific failure and recovery**
  - Action: Require protocol and state-machine tests with unknown-required messages, full queues, deadlines/cancel, peer death and no leaked handles or transactions. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-SVC-096.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Text-to-Speech Service.
- [ ] **T-SVC-096.05 — Qualify and retain this target's own result**
  - Action: Bind Text-to-Speech Service to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-SVC-096.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-008"></a>
## T-APP-008 — Accessibility Center

**Original requirement:** live preview/control for every assistive feature

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 135.

### Execution steps

- [ ] **T-APP-008.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Accessibility Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-008.02 — Specify the complete target boundary**
  - Action: Accessibility Center must supply: live preview/control for every assistive feature. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-008.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-008.03 — Implement the exact target behavior**
  - Action: Implement or reuse Accessibility Center through the shared platform contract, delivering every part of: live preview/control for every assistive feature. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-008.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-008.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-APP-008.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Accessibility Center.
- [ ] **T-APP-008.05 — Qualify and retain this target's own result**
  - Action: Bind Accessibility Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-008.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-009"></a>
## T-APP-009 — Appearance Center

**Original requirement:** theme/type/scale/motion/color preview and rollback

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 136.

### Execution steps

- [ ] **T-APP-009.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Appearance Center to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-009.02 — Specify the complete target boundary**
  - Action: Appearance Center must supply: theme/type/scale/motion/color preview and rollback. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-009.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-009.03 — Implement the exact target behavior**
  - Action: Implement or reuse Appearance Center through the shared platform contract, delivering every part of: theme/type/scale/motion/color preview and rollback. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-009.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-009.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-APP-009.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Appearance Center.
- [ ] **T-APP-009.05 — Qualify and retain this target's own result**
  - Action: Bind Appearance Center to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-009.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.

<a id="t-app-107"></a>
## T-APP-107 — Accessibility Tour

**Original requirement:** learn/test screen reader, magnifier, captions and input aids

**Source:** [docs/program/APPLICATIONS.md](../../docs/program/APPLICATIONS.md), line 235.

### Execution steps

- [ ] **T-APP-107.01 — Identify the exact target and reuse its existing owner**
  - Action: Resolve Accessibility Tour to its current implementation or required new provider/package, consumed protocols and exact source/hardware/format/version profile. Preserve this target even when no device or implementation is currently available.
  - Requires: D-01, D-02, D-14, H-10.
  - Acceptance: A named owner, input/source/specification inventory, dependency list and baseline evidence or explicit absence.
- [ ] **T-APP-107.02 — Specify the complete target boundary**
  - Action: Accessibility Tour must supply: learn/test screen reader, magnifier, captions and input aids. Define admitted install/launch/ready/use/save or action/close/reopen/crash/restart/update/remove; keyboard, semantics, locale and scaling. Bind each prerequisite provider and contract before implementation.
  - Requires: T-APP-107.01.
  - Acceptance: Complete state, input/output, failure, resource, authority, version and recovery definitions for this exact target.
- [ ] **T-APP-107.03 — Implement the exact target behavior**
  - Action: Implement or reuse Accessibility Tour through the shared platform contract, delivering every part of: learn/test screen reader, magnifier, captions and input aids. Do not fork a duplicate subsystem for this row.
  - Requires: T-APP-107.02.
  - Acceptance: The target fulfills its own boundary through a real consumer; shared implementation evidence is accepted only after the target-specific contract is checked.
- [ ] **T-APP-107.04 — Prove target-specific failure and recovery**
  - Action: Require a real user journey with independent inspection of saved/exported/external effects, plus malformed input, denied grants, dependency loss and crash recovery. Relevant domain cases: keyboard-only completion; missing/duplicated semantic node; focus loss; bidi/IME composition; extreme scale; disabled assistive provider; high contrast; locale expansion.
  - Requires: T-APP-107.03.
  - Acceptance: A positive workload, all applicable hostile/failure cases and exact resource/durable-state recovery for Accessibility Tour.
- [ ] **T-APP-107.05 — Qualify and retain this target's own result**
  - Action: Bind Accessibility Tour to its exact package/image/architecture/device/firmware/profile evidence, accessibility/performance obligations where applicable, and rollback/removal conditions. Unavailable hardware remains a named qualification task.
  - Requires: T-APP-107.04.
  - Acceptance: No family-wide, source-present, screenshot-only or simulated result is promoted to unsupported target scope.
